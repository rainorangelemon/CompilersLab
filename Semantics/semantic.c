#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Lex/tree.h"
#include "data_type.h"

extern Node* root;
struct Hash_table* hash_table;

int error_line = -1;


// check whether the expression, name, matches with tree_root
int compareSubExpression(Node* tree_root, char* name){
//  printf("tree_root:%s, name:%s\n", tree_root->name, name); // just for debug
  if(tree_root->son==NULL){
    return -1;
  }else{
    Node* son = tree_root->son;
    char* subname = strtok(name, " ");
    while (subname != NULL)
    {
      if(son==NULL){
        return -1;
      }else if(strcmp(son->name, subname)!=0){
        return -1;
      }else{
        subname = strtok(NULL, " ");
        son = son->bro;
      }
    }
    if(son!=NULL){
      return -1;
    }
    return 1;
  }
}

// High-level Definitions

void check_error(Node* tree_root) {
  char rule1[] = "Program";
  if (strcmp(tree_root->name, rule1) != 0) {
    return;
  }
  hash_table = create_table();
  push_env(hash_table);
  check_error_ExtDefList(root->son);
  // check error 18
  struct Symbol *test_symbol = hash_table->stack_head->down;
  while (test_symbol != NULL) {
    if ((test_symbol->kind == FUNC) && (test_symbol->function->hasCompSt == 0)) {
      print_error(18, test_symbol->name, test_symbol->function->define_lineno);
    }
    test_symbol = test_symbol->down;
  }
  // pop env
  pop_env(hash_table);
  free_table(hash_table);
}

void check_error_ExtDefList(Node* ExtDefList) {
  if (ExtDefList->son == NULL) {   // is rule 2
    // do nothing
  } else {
    check_error_ExtDef(ExtDefList->son);
    check_error_ExtDefList(ExtDefList->son->bro);
  }
}

void check_error_ExtDef(Node* ExtDef) {
  char rule1[] = "Specifier ExtDecList SEMI";
  char rule2[] = "Specifier SEMI";
  char rule3[] = "Specifier FunDec CompSt";
  Type specifier_type = createType_Specifier(ExtDef->son);
  if (compareSubExpression(ExtDef, rule1) == 1) {
    valueType_ExtDecList(ExtDef->son->bro, specifier_type);
  } else if (compareSubExpression(ExtDef, rule2) == 1) {
    // do nothing
  } else if (compareSubExpression(ExtDef, rule3) == 1){
    createSymbol_function_FunDec(ExtDef->son->bro, 0, specifier_type);
    check_error_CompSt(ExtDef->son->bro->bro, specifier_type);
    pop_env(hash_table);
  } else {
    createSymbol_function_FunDec(ExtDef->son->bro, 1, specifier_type);
    pop_env(hash_table);
  }
}

Type valueType_ExtDecList(Node* ExtDecList, Type type) {
  char rule2[] = "VarDec COMMA ExtDecList";
  int isRule2 = compareSubExpression(ExtDecList, rule2);
  if (isRule2 != 1) {
    createType_VarDec(ExtDecList->son, type, 0, NULL);
  } else {
    createType_VarDec(ExtDecList->son, type, 0, NULL);
    valueType_ExtDecList(ExtDecList->son->bro->bro, type);
  }
  return type;
}

// Specifiers

Type createType_Specifier(Node* Specifier) {
  char rule1[] = "TYPE";
  char rule2[] = "StructSpecifier";
  int isRule1 = compareSubExpression(Specifier, rule1);
  if (isRule1 == 1) {
    Type result = (Type) malloc(sizeof(struct Type_));
    memset(result, 0, sizeof(struct Type_));
    if (strcmp(Specifier->son->value, "int") == 0) {
      result->kind = BASIC;
      result->u.basic = INT;
    } else {
      result->kind = BASIC;
      result->u.basic = FLOAT;
    }
    return result;
  } else {
    return createType_StructSpecifier(Specifier->son);
  }
}

Type createType_StructSpecifier(Node* StructSpecifier) {
  char rule1[] = "STRUCT OptTag LC DefList RC";
  int isRule1 = compareSubExpression(StructSpecifier, rule1);
  if (isRule1 == 1) {
    Type result = (Type) malloc(sizeof(struct Type_));
    memset(result, 0, sizeof(struct Type_));
    Node *OptTag = StructSpecifier->son->bro;
    push_env(hash_table);
    FieldList structure = (FieldList) malloc(sizeof(struct FieldList_));
    memset(structure, 0, sizeof(struct FieldList_));
    structure->name = NULL;
    createFieldList_DefList(OptTag->bro->bro, structure, 1);
    pop_env(hash_table);
    result->u.structure = structure;
    result->kind = STRUCTURE;
    if (OptTag->son != NULL) {
      insert_symbol(hash_table, OptTag->son->value, STRUCT, result, NULL, OptTag->son->lineno);
    }
    return result;
  } else {
    Node *Tag = StructSpecifier->son->bro;
    struct Symbol temp;
    struct Symbol *structure = find_symbol(hash_table, Tag->son->value, STRUCT);
    if (structure == NULL) {
      print_error(17, Tag->son->value, Tag->lineno);
      return NULL;
    }
    return structure->type;
  }
}

// Declarators

Type createType_VarDec(Node* VarDec, Type type, int insideStruct, FieldList fieldList) {
  char rule1[] = "ID";
  int isRule1 = compareSubExpression(VarDec, rule1);
  if (isRule1 == 1) {
    if (insideStruct != 1) {
      insert_symbol(hash_table, VarDec->son->value, VARIABLE, type, NULL, VarDec->son->lineno);
    } else {
      if (fieldList->name == NULL) {
        fieldList->name = VarDec->son->value;
        fieldList->type = type;
      } else {
        char *name = VarDec->son->value;
        // begin to check the name redundancy
        FieldList temp = fieldList;
        while (temp->tail != NULL) {
          if (strcmp(temp->name, name) == 0) {
            print_error(15, name, VarDec->son->lineno);
            return type;
          }
          temp = temp->tail;
        }
        // no name redundancy
        FieldList new_field = (FieldList) malloc(sizeof(struct FieldList_));
        memset(new_field, 0, sizeof(struct FieldList_));
        new_field->name = name;
        new_field->type = type;
        temp->tail = new_field;
      }
    }
    return type;
  } else {
    int size = strtol(VarDec->son->bro->bro->value, NULL, 0);
    Type result = (Type) malloc(sizeof(struct Type_));
    memset(result, 0, sizeof(struct Type_));
    result->kind = ARRAY;
    result->u.array.size = size;
    result->u.array.elem = type;
    createType_VarDec(VarDec->son, result, insideStruct, fieldList);
    return result;
  }
}

void createSymbol_function_FunDec(struct Node* FunDec, int isDeclaration, Type returnType) {
  char rule1[] = "ID LP VarList RP";
  int isRule1 = compareSubExpression(FunDec, rule1);
  struct Symbol_function *symbol_function = (struct Symbol_function *) malloc(sizeof(struct Symbol_function));
  memset(symbol_function, 0, sizeof(struct Symbol_function));
  symbol_function->argc = 0;
  symbol_function->argv1 = NULL;
  symbol_function->return_type = returnType;
  symbol_function->define_lineno = FunDec->son->lineno;
  symbol_function->hasCompSt = (1-isDeclaration);
  push_env(hash_table);
  if (isRule1 == 1) {
    valueSymbol_function_VarList(FunDec->son->bro->bro, symbol_function);
  }
  insert_symbol(hash_table, FunDec->son->value, FUNC, NULL, symbol_function, FunDec->son->lineno);
}

Type valueSymbol_function_VarList(struct Node* VarList, struct Symbol_function* function) {
  char rule1[] = "ParamDec COMMA VarList";
  int isRule1 = compareSubExpression(VarList, rule1);
  struct argv *p = function->argv1;
  Type paramDec_type = createType_ParamDec(VarList->son);
  struct argv *temp = (struct argv *) malloc(sizeof(struct argv));
  memset(temp, 0, sizeof(struct argv));
  temp->type = paramDec_type;
  temp->next = NULL;
  if (p == NULL) {
    function->argv1 = temp;
  } else {
    while (p->next != NULL) {
      p = p->next;
    }
    p->next = temp;
  }
  function->argc = function->argc + 1;
  if (isRule1 == 1) {
    valueSymbol_function_VarList(VarList->son->bro->bro, function);
  }
  return NULL;
}

Type createType_ParamDec(struct Node* ParamDec) {
  Type type = createType_Specifier(ParamDec->son);
  createType_VarDec(ParamDec->son->bro, type, 0, NULL);
  return type;
}

// Statements

void check_error_CompSt(struct Node* CompSt, Type returnType) {
  push_env(hash_table);
  createFieldList_DefList(CompSt->son->bro, NULL, 0);
  check_error_StmtList(CompSt->son->bro->bro, returnType);
  pop_env(hash_table);
}

void check_error_StmtList(struct Node* StmtList, Type returnType) {
  if (StmtList->son != NULL) {
    check_error_Stmt(StmtList->son, returnType);
    check_error_StmtList(StmtList->son->bro, returnType);
  }
}

void check_error_Stmt(struct Node* Stmt, Type returnType) {
  char rule1[] = "Exp SEMI";
  char rule2[] = "CompSt";
  char rule3[] = "RETURN Exp SEMI";
  char rule4[] = "IF LP Exp RP Stmt";
  char rule5[] = "IF LP Exp RP Stmt ELSE Stmt";
  char rule6[] = "WHILE LP Exp RP Stmt";
  if (compareSubExpression(Stmt, rule1) == 1) {
    getType_Exp(Stmt->son);
  } else if (compareSubExpression(Stmt, rule2) == 1) {
    check_error_CompSt(Stmt->son, returnType);
  } else if (compareSubExpression(Stmt, rule3) == 1) {
    if (returnType != NULL) {
      if (compare_type_type(getType_Exp(Stmt->son->bro), returnType) != 1) {
        print_error(8, NULL, Stmt->son->lineno);
      }
    }
  } else if (compareSubExpression(Stmt, rule4) == 1) {
    if (compare_type_kind(getType_Exp(Stmt->son->bro->bro), INT) != 1) {
      print_error(7, NULL, Stmt->son->lineno);
    }
    check_error_Stmt(Stmt->son->bro->bro->bro->bro, returnType);
  } else if (compareSubExpression(Stmt, rule5) == 1) {
    if (compare_type_kind(getType_Exp(Stmt->son->bro->bro), INT) != 1) {
      print_error(7, NULL, Stmt->son->lineno);
    }
    check_error_Stmt(Stmt->son->bro->bro->bro->bro, returnType);
    check_error_Stmt(Stmt->son->bro->bro->bro->bro->bro->bro, returnType);
  } else if (compareSubExpression(Stmt, rule6) == 1) {
    if (compare_type_kind(getType_Exp(Stmt->son->bro->bro), INT) != 1) {
      print_error(7, NULL, Stmt->son->lineno);
    }
    check_error_Stmt(Stmt->son->bro->bro->bro->bro, returnType);
  }
}

// Local Definitions

FieldList createFieldList_DefList(struct Node* DefList, FieldList result, int insideStruct) {
  if (DefList->son != NULL) {
    createFieldList_Def(DefList->son, result, insideStruct);
    createFieldList_DefList(DefList->son->bro, result, insideStruct);
    return result;
  } else {
    return NULL;
  }
}

FieldList createFieldList_Def(struct Node* Def, FieldList result, int insideStruct) {
  Type type = createType_Specifier(Def->son);
  createFieldList_DecList(Def->son->bro, type, result, insideStruct);
  return result;
}

void createFieldList_DecList(struct Node* DecList, Type type, FieldList fieldList, int insideStruct) {
  createFieldList_Dec(DecList->son, type, fieldList, insideStruct);
  char rule1[] = "Dec";
  int isRule1 = compareSubExpression(DecList, rule1);
  if (isRule1 != 1) {
    createFieldList_DecList(DecList->son->bro->bro, type, fieldList, insideStruct);
  }
}

void createFieldList_Dec(struct Node* Dec, Type type, FieldList fieldList, int insideStruct) {
  createType_VarDec(Dec->son, type, insideStruct, fieldList);
  char rule1[] = "VarDec";
  int isRule1 = compareSubExpression(Dec, rule1);
  if (isRule1 != 1) {
    if (insideStruct == 1) {
      print_error(15, NULL, Dec->son->lineno);
    }
    Type exp_type = getType_Exp(Dec->son->bro->bro);
    if (compare_type_type(type, exp_type) != 1) {
      print_error(5, NULL, Dec->son->lineno);
    }
  }
}

// Expressions

Type getType_Exp(struct Node* Exp) {
  char
    rule1[] = "Exp ASSIGNOP Exp",
    rule2[] = "Exp AND Exp",
    rule3[] = "Exp OR Exp",
    rule4[] = "Exp RELOP Exp",
    rule5[] = "Exp PLUS Exp",
    rule6[] = "Exp MINUS Exp",
    rule7[] = "Exp STAR Exp",
    rule8[] = "Exp DIV Exp",
    rule9[] = "LP Exp RP",
    rule10[] = "MINUS Exp",
    rule11[] = "NOT Exp",
    rule12[] = "ID LP Args RP",
    rule13[] = "ID LP RP",
    rule14[] = "Exp LB Exp RB",
    rule15[] = "Exp DOT ID",
    rule16[] = "ID",
    rule17[] = "INT",
    rule18[] = "FLOAT";
  Type result = NULL;
  if (compareSubExpression(Exp, rule1) == 1) {
    // check whether the value on the left of ASSIGNOP is a left value
    if (!((compareSubExpression(Exp->son, rule14) == 1) || (compareSubExpression(Exp->son, rule15) == 1) ||
          (compareSubExpression(Exp->son, rule16) == 1))) {
      print_error(6, NULL, Exp->son->lineno);
    } else {
      // check whether the types on both sides are equal
      Type type1 = getType_Exp(Exp->son);
      Type type2 = getType_Exp(Exp->son->bro->bro);
      if (compare_type_type(type1, type2) != 1) {
        print_error(5, NULL, Exp->son->lineno);
      }
    }
    result = (Type) malloc(sizeof(struct Type_));
    memset(result, 0, sizeof(struct Type_));
    result->kind = BASIC;
    result->u.basic = INT;
  } else if ((compareSubExpression(Exp, rule2) == 1) || (compareSubExpression(Exp, rule3) == 1)) {
    Type type1 = getType_Exp(Exp->son);
    Type type2 = getType_Exp(Exp->son->bro->bro);
    if (!((compare_type_kind(type1, INT) == 1) && (compare_type_kind(type2, INT) == 1))) {
      print_error(7, NULL, Exp->son->lineno);
    }
    result = (Type) malloc(sizeof(struct Type_));
    result->kind = BASIC;
    memset(result, 0, sizeof(struct Type_));
    result->u.basic = INT;
  } else if ((compareSubExpression(Exp, rule4) == 1) || (compareSubExpression(Exp, rule5) == 1) ||
             (compareSubExpression(Exp, rule6) == 1) || (compareSubExpression(Exp, rule7) == 1) ||
             (compareSubExpression(Exp, rule8) == 1)) {
    Type type1 = getType_Exp(Exp->son);
    Type type2 = getType_Exp(Exp->son->bro->bro);
    if (!(((compare_type_kind(type1, INT) == 1) && (compare_type_kind(type2, INT) == 1)) ||
      ((compare_type_kind(type1, FLOAT) == 1) && (compare_type_kind(type2, FLOAT) == 1)))) {
      print_error(7, NULL, Exp->son->lineno);
    } else {
      result = type1;
    }
  } else if (compareSubExpression(Exp, rule9) == 1) {
    result = getType_Exp(Exp->son->bro);
  } else if (compareSubExpression(Exp, rule10) == 1) {
    if (compare_type_kind(getType_Exp(Exp->son->bro), INT) == 1) {
      result = (Type) malloc(sizeof(struct Type_));
      memset(result, 0, sizeof(struct Type_));
      result->kind = BASIC;
      result->u.basic = INT;
    } else if (compare_type_kind(getType_Exp(Exp->son->bro), FLOAT) == 1) {
      result = (Type) malloc(sizeof(struct Type_));
      memset(result, 0, sizeof(struct Type_));
      result->kind = BASIC;
      result->u.basic = FLOAT;
    } else {
      print_error(7, NULL, Exp->son->bro->lineno);
    }
  } else if (compareSubExpression(Exp, rule11) == 1) {
    if (compare_type_kind(getType_Exp(Exp->son->bro), INT) != 1) {
      print_error(7, NULL, Exp->son->bro->lineno);
    } else {
      result = (Type) malloc(sizeof(struct Type_));
      memset(result, 0, sizeof(struct Type_));
      result->kind = BASIC;
      result->u.basic = INT;
    }
  } else if (compareSubExpression(Exp, rule12) == 1) {
    //check whether the function exists in symbol
    struct Symbol *func = find_symbol(hash_table, Exp->son->value, FUNC);
    if (func == NULL) {
      struct Symbol *variable = find_symbol(hash_table, Exp->son->value, VARIABLE);
      if (variable == NULL) {
        print_error(2, Exp->son->value, Exp->son->lineno);
      } else {
        print_error(11, Exp->son->value, Exp->son->lineno);
      }
    } else {
      //check length of param of func
      if (compareArgv_args(Exp->son->bro->bro, func->function->argv1) != 1) {
        print_error(9, Exp->son->value, Exp->son->lineno);
      } else {
        result = func->function->return_type;
      }
    }
  } else if (compareSubExpression(Exp, rule13) == 1) {
    //check whether the function exists in symbol
    struct Symbol *func = find_symbol(hash_table, Exp->son->value, FUNC);
    if (func == NULL) {
      struct Symbol *variable = find_symbol(hash_table, Exp->son->value, VARIABLE);
      if (variable == NULL) {
        print_error(2, Exp->son->value, Exp->son->lineno);
      } else {
        print_error(11, Exp->son->value, Exp->son->lineno);
      }
    } else {
      //check length of param of func
      if (func->function->argc != 0) {
        print_error(9, Exp->son->value, Exp->son->lineno);
      } else {
        result = func->function->return_type;
      }
    }
  } else if (compareSubExpression(Exp, rule14) == 1) {
    // check whether the first Exp is an array
    Type first_exp = getType_Exp(Exp->son);
    if ((first_exp == NULL) || (first_exp->kind != ARRAY)) {
      print_error(10, NULL, Exp->son->lineno);
    } else {
      // check whether the index is an integer
      Type index = getType_Exp(Exp->son->bro->bro);
      if ((index == NULL) || (index->kind != BASIC) || (index->u.basic != INT)) {
        print_error(12, NULL, Exp->son->lineno);
      } else {
        result = first_exp->u.array.elem;
      }
    }
  } else if (compareSubExpression(Exp, rule15) == 1) {
    Type first_exp_type = getType_Exp(Exp->son);
    if ((first_exp_type == NULL) || (first_exp_type->kind != STRUCTURE)) {
      print_error(13, NULL, Exp->son->lineno);
    } else {
      //check whether there is a matching ID in Exp.type
      FieldList temp = first_exp_type->u.structure;
      while (temp != NULL) {
        if (strcmp(temp->name, Exp->son->bro->bro->value) == 0) {
          result = temp->type;
          break;
        }
        temp = temp->tail;
      }
      if (temp == NULL) {
        print_error(14, Exp->son->bro->bro->value, Exp->son->lineno);
      }
    }
  } else if (compareSubExpression(Exp, rule16) == 1) {
    struct Symbol *symbol = find_symbol(hash_table, Exp->son->value, VARIABLE);
    if (symbol == NULL) {
      print_error(1, Exp->son->value, Exp->son->lineno);
    } else {
      result = symbol->type;
    }
  } else if (compareSubExpression(Exp, rule17) == 1) {
    result = (Type) malloc(sizeof(struct Type_));
    memset(result, 0, sizeof(struct Type_));
    result->kind = BASIC;
    result->u.basic = INT;
  } else if (compareSubExpression(Exp, rule18) == 1) {
    result = (Type) malloc(sizeof(struct Type_));
    memset(result, 0, sizeof(struct Type_));
    result->kind = BASIC;
    result->u.basic = FLOAT;
  }
  return result;
}

int compareArgv_args(struct Node* Args, struct argv* function_argv) {
  if (((Args == NULL) && (function_argv != NULL)) || ((Args != NULL) && (function_argv == NULL))) {
    return -1;
  } else if ((Args == NULL) && (function_argv == NULL)) {
    return 1;
  }
  Type exp_type = getType_Exp(Args->son);
  if (compare_type_type(function_argv->type, exp_type) != 1) {
    return -1;
  }
  char rule1[] = "Exp COMMA Args";
  if (compareSubExpression(Args, rule1) == 1) {
    return compareArgv_args(Args->son->bro->bro, function_argv->next);
  } else {
    return compareArgv_args(NULL, function_argv->next);
  }
}

// to print error
void print_error(int error_type, char* target, int lineno){
  char *information[20] ={"", // 0
                          "Undefined variable", // 1
                          "Undefined function", // 2
                          "Redefined variable", // 3
                          "Redefined function", // 4
                          "Type mismatched for assignment", // 5
                          "The left-hand side of an assignment must be a variable", // 6
                          "Type mismatched for operands", // 7
                          "Type mismatched for return", // 8
                          "Function's arguments is not applicable", // 9
                          "The first Exp is not an array", // 10
                          "It is not a function", // 11
                          "The Exp between [ and ] is not an integer", // 12
                          "Illegal use of \".\"", // 13
                          "Non-existent field", // 14
                          "Redefined field, or tried to initialize the field", // 15
                          "Duplicated name", // 16
                          "Undefined structure", // 17
                          "Undefined function", // 18
                          "Inconsistent declaration of function" //19
  };
  if((error_line!=lineno)&&(error_type<=19)&&(error_type>=1)){  //because every line has only one error
    if(target!=NULL) {
      printf("Error type %d at Line %d: \"%s\": %s.\n", error_type, lineno, target, information[error_type]);
    }else{
      printf("Error type %d at Line %d: %s.\n", error_type, lineno, information[error_type]);
    }
    error_line = lineno;
  }
}

// compare type with kind
int compare_type_kind(Type type, int kind){
  if(type==NULL){
    return -1;
  }
  if((type->kind==BASIC)&&((kind==INT)||(kind==FLOAT))&&(type->u.basic==kind)){
    return 1;
  }else if(type->kind==kind){
    return 1;
  }
  return -1;
}

// compare type with another type
int compare_type_type(Type type_a, Type type_b){
  if((type_a==NULL)||(type_b==NULL)){
    return -1;
  }
  if((type_a->kind==BASIC)&&(type_b->kind==BASIC)){
    if(type_a->u.basic==type_b->u.basic){
      return 1;
    }
  }else if((type_a->kind==ARRAY)&&(type_b->kind==ARRAY)){
    return compare_type_type(type_a->u.array.elem, type_b->u.array.elem);
  }else if((type_a->kind==STRUCTURE)&&(type_b->kind==STRUCTURE)){
    FieldList temp_a = type_a->u.structure;
    FieldList temp_b = type_b->u.structure;
    int same;
    while((temp_a!=NULL)||(temp_b!=NULL)){
      if(((temp_a==NULL)&&(temp_b!=NULL))||((temp_a!=NULL)&&(temp_b==NULL))){
        return -1;
      }
      same = compare_type_type(temp_a->type, temp_b->type);
      if(same!=1){
        return -1;
      }else{
        temp_a = temp_a->tail;
        temp_b = temp_b->tail;
      }
    }
    return 1;
  }
  return -1;
}