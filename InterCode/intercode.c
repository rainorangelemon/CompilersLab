//
// Created by rainorangelemon on 6/9/18.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "intercode.h"

int tempVariableNumber = 0;
int variableNumber = 0;
int lableNumber = 0;
extern struct Hash_table* hash_table;

void addCode(InterCodes codes, InterCode newCode);
char* new_temp();
char* new_label();
int get_value(struct Node* INT);
Operand create_operand(int kind, int value, char* name);
InterCode create_code_label(int kind, Operand label);
InterCode create_code_assign(int kind, Operand left, Operand right);
InterCode create_code_binop(int kind, Operand result, Operand op1, Operand mathop, Operand op2);
InterCode create_code_goto_con(int kind, Operand left, Operand relop, Operand right, Operand label);
char* lookup_symbols(char* name);
void translate_root(Node* tree_root, char* path);
void translate_ExtDefList(Node* ExtDefList, InterCodes head);
void translate_ExtDef(Node* ExtDef, InterCodes head);
Type getType_Specifier(Node* Specifier);
Type getType_StructSpecifier(Node* StructSpecifier);
Type getType_VarDec(Node* VarDec, Type type, int insideStruct, int funcParam, FieldList fieldList, InterCodes head);
void translate_FunDec(struct Node* FunDec, InterCodes head);
void translate_VarList(struct Node* FunDec, InterCodes head);
void translate_ParamDec(struct Node* ParamDec, InterCodes head);
void translate_CompSt(struct Node* CompSt, InterCodes head);
void translate_StmtList(struct Node* StmtList, InterCodes head);
void translate_Stmt(struct Node* Stmt, InterCodes head);
void translate_DefList(struct Node* DefList, FieldList result, int insideStruct, InterCodes head);
void translate_Def(struct Node* Def, FieldList result, int insideStruct, InterCodes head);
void translate_DecList(struct Node* DecList, Type type, FieldList fieldList, int insideStruct, InterCodes head);
void translate_Dec(struct Node* Dec, Type type, FieldList fieldList, int insideStruct, InterCodes head);
void translate_Cond(struct Node* Exp, char* label_true, char* label_false, InterCodes head);
Type translate_Exp(struct Node* Exp, InterCodes head, char* place);
void translate_Args(struct Node* Args, InterCodes head, Arg_list arg_list);
int calculate_space(Type type);
int calculate_offset_in_struct(FieldList fieldList, char* name);
Type get_type_in_struct(FieldList fieldList, char* name);
char* printCodes(InterCode interCode);

// check whether the expression, name, matches with tree_root
int compareExpression(Node* tree_root, char* name){
  printf("tree_root:%s, name:%s, lineno: %d\n", tree_root->name, name, tree_root->lineno); // just for debug
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

void addCode(InterCodes codes, InterCode newCode){
  InterCodes temp  = codes;
  while(temp->next!=NULL){
    temp = temp->next;
  }
  if(temp->code==NULL){
    temp->code = newCode;
  }else{
    InterCodes newInterCode = (InterCodes) malloc(sizeof(struct InterCodes_));
    memset(newInterCode, 0, sizeof(struct InterCodes_));
    newInterCode->prev = temp;
    newInterCode->code = newCode;
    temp->next = newInterCode;
  }
}

char* new_temp(){
  tempVariableNumber++;
  char *result = malloc(30*sizeof(char));
  memset(result, 0, 30*sizeof(char));
  sprintf(result, "t%d", tempVariableNumber);
  return result;
}

char* new_variable(){
  variableNumber++;
  char *result = malloc(30*sizeof(char));
  memset(result, 0, 30*sizeof(char));
  sprintf(result, "v%d", variableNumber);
  return result;
}

char* new_label(){
  lableNumber++;
  char *result = malloc(30*sizeof(char));
  memset(result, 0, 30*sizeof(char));
  sprintf(result, "label%d", lableNumber);
  return result;
}

int get_value(struct Node* INT){
  int size = strtol(INT->value, NULL, 0);
  return size;
}

Operand create_operand(int kind, int value, char* name){
  Operand result = (Operand)malloc(sizeof(struct Operand_));
  memset(result, 0, sizeof(struct Operand_));
  result->kind = kind;
  if(kind!=CONSTANT) {
    result->u.name = name;
  }else {
    result->u.value = value;
  }
  return result;
}

char* print_operand(Operand operand, int addHashTag){
  if(operand->kind!=CONSTANT){
    return operand->u.name;
  }else {
    char* result = (char*)malloc(20*sizeof(char));
    memset(result, 0, 20*sizeof(char));
    if(addHashTag==1){
      sprintf(result, "#%d", operand->u.value);
    }else{
      sprintf(result, "%d", operand->u.value);
    }
    return result;
  }
}

InterCode create_code_label(int kind, Operand label){
  InterCode result = (InterCode)malloc(sizeof(struct InterCode_));
  memset(result, 0, sizeof(struct InterCodes_));
  result->kind = kind;
  result->u.label.label = label;
  return result;
}

InterCode create_code_assign(int kind, Operand left, Operand right){
  InterCode result = (InterCode)malloc(sizeof(struct InterCode_));
  memset(result, 0, sizeof(struct InterCodes_));
  result->kind = kind;
  result->u.assign.left = left;
  result->u.assign.right = right;
  return result;
}

InterCode create_code_binop(int kind, Operand result, Operand op1, Operand mathop, Operand op2){
  InterCode interCode = (InterCode)malloc(sizeof(struct InterCode_));
  memset(interCode, 0, sizeof(struct InterCodes_));
  interCode->kind = kind;
  interCode->u.binop.result = result;
  interCode->u.binop.op1 = op1;
  interCode->u.binop.op2 = op2;
  interCode->u.binop.mathop = mathop;
  return interCode;
}

InterCode create_code_goto_con(int kind, Operand left, Operand relop, Operand right, Operand label){
  InterCode interCode = (InterCode)malloc(sizeof(struct InterCode_));
  memset(interCode, 0, sizeof(struct InterCodes_));
  interCode->kind = kind;
  interCode->u.goto_con.left = left;
  interCode->u.goto_con.relop = relop;
  interCode->u.goto_con.right = right;
  interCode->u.goto_con.label = label;
  return interCode;
}

char* lookup_symbols(char* name) {
  struct Symbol *symbol = find_symbol(hash_table, name, VARIABLE);
  if (symbol->code_name == NULL) {
    char* new_name = new_variable();
    symbol->code_name = new_name;
  }
  return symbol->code_name;
}

int isAddress(char* name){
  struct Symbol *symbol = find_symbol(hash_table, name, VARIABLE);
  return symbol->isAddress;
}

void translate_root(Node* tree_root, char* path) {
  char rule1[] = "Program";
  if (strcmp(tree_root->name, rule1) != 0) {
    return;
  }
  hash_table = create_table();
  InterCodes head = (InterCodes) malloc(sizeof(struct InterCodes_));
  memset(head, 0, sizeof(struct InterCodes_));
  translate_ExtDefList(tree_root->son, head);
  InterCodes temp = head;
  while(temp!=NULL){
    printf("%s\n", printCodes(temp->code));
    temp = temp->next;
  }
  // TODO: write into file
//  FILE * fp;
//  char *path = malloc(60*sizeof(char));
//  memset(path, 0, 60*sizeof(char));
//  sprintf(path, "../%s", path);
//  fp = fopen(path,"w+");
//  // TODO: write into file
//  fclose(fp);
  free_table(hash_table);
}

void translate_ExtDefList(Node* ExtDefList, InterCodes head) {
  if (ExtDefList->son == NULL) {   // is rule 2
    // do nothing
  } else {
    translate_ExtDef(ExtDefList->son, head);
    translate_ExtDefList(ExtDefList->son->bro, head);
  }
}

void translate_ExtDef(Node* ExtDef, InterCodes head) {
  char rule1[] = "Specifier ExtDecList SEMI";
  char rule2[] = "Specifier SEMI";
  char rule3[] = "Specifier FunDec CompSt";
  Type specifier_type = getType_Specifier(ExtDef->son);
  if (compareExpression(ExtDef, rule3) == 1) {
    translate_FunDec(ExtDef->son->bro, head);
    translate_CompSt(ExtDef->son->bro->bro, head);
  }
}

Type getType_Specifier(Node* Specifier) {
  char rule1[] = "TYPE";
  char rule2[] = "StructSpecifier";
  if (compareExpression(Specifier, rule1) == 1) {
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
    return getType_StructSpecifier(Specifier->son);
  }
}

Type getType_StructSpecifier(Node* StructSpecifier) {
  char rule1[] = "STRUCT OptTag LC DefList RC";
  int isRule1 = compareExpression(StructSpecifier, rule1);
  if (isRule1 == 1) {
    Type result = (Type) malloc(sizeof(struct Type_));
    memset(result, 0, sizeof(struct Type_));
    Node *OptTag = StructSpecifier->son->bro;
    FieldList structure = (FieldList) malloc(sizeof(struct FieldList_));
    memset(structure, 0, sizeof(struct FieldList_));
    structure->name = NULL;
    translate_DefList(OptTag->bro->bro, structure, 1, NULL);
    result->u.structure = structure;
    result->kind = STRUCTURE;
    if (OptTag->son != NULL) {
      insert_symbol_intercodes(hash_table, 0, OptTag->son->value, STRUCT, result, NULL);
    }
    return result;
  } else {
    Node *Tag = StructSpecifier->son->bro;
    struct Symbol temp;
    struct Symbol* structure = find_symbol_intercodes(hash_table, Tag->son->value, STRUCT);
    return structure->type;
  }
}

// Declarators

Type getType_VarDec(Node* VarDec, Type type, int insideStruct, int funcParam, FieldList fieldList, InterCodes head) {
  char rule1[] = "ID";
  int isRule1 = compareExpression(VarDec, rule1);
  if (isRule1 == 1) {
    if (insideStruct != 1) {
      if(funcParam==1){
        char* name = VarDec->son->value;
        if(type->kind!=BASIC) {
          insert_symbol_intercodes(hash_table, 1, VarDec->son->value, VARIABLE, type, NULL);
        }else{
          insert_symbol_intercodes(hash_table, 0, VarDec->son->value, VARIABLE, type, NULL);
        }
        // add new code of PARAM
        InterCode newCode = create_code_label(PARAM, create_operand(LABEL, 0, lookup_symbols(VarDec->son->value)));
        addCode(head, newCode);
      }else {
        insert_symbol_intercodes(hash_table, 0, VarDec->son->value, VARIABLE, type, NULL);
        // malloc space for symbols which are not basic types
        if(type->kind!=BASIC){
          // add new code: DEC / malloc space
          int size = calculate_space(type);
          addCode(head, create_code_assign(DEC,
                                           create_operand(VARIABLE, 0, lookup_symbols(VarDec->son->value)),
                                           create_operand(CONSTANT, size, NULL)));
        }
      }
    } else {
      if (fieldList->name == NULL) {
        fieldList->name = VarDec->son->value;
        fieldList->type = type;
      } else {
        char *name = VarDec->son->value;
        // insert the new field
        FieldList temp = fieldList;
        while (temp->tail != NULL) {
          temp = temp->tail;
        }
        FieldList new_field = (FieldList) malloc(sizeof(struct FieldList_));
        memset(new_field, 0, sizeof(struct FieldList_));
        new_field->name = name;
        new_field->type = type;
        temp->tail = new_field;
      }
    }
    return type;
  } else {
    // an array
    int size = strtol(VarDec->son->bro->bro->value, NULL, 0);
    Type result = (Type) malloc(sizeof(struct Type_));
    memset(result, 0, sizeof(struct Type_));
    result->kind = ARRAY;
    result->u.array.size = size;
    result->u.array.elem = type;
    getType_VarDec(VarDec->son, result, insideStruct, funcParam, fieldList, head);
    return result;
  }
}

void translate_FunDec(struct Node* FunDec, InterCodes head) {
  // add new code of FUNCTION
  InterCode newCode = create_code_label(FUNCTION, create_operand(LABEL, 0, FunDec->son->value));
  addCode(head, newCode);
  // check VarList
  char rule1[] = "ID LP VarList RP";
  int isRule1 = compareExpression(FunDec, rule1);
  if (isRule1 == 1) {
    translate_VarList(FunDec->son->bro->bro, head);
  }else{
    // do nothing
  }
}

void translate_VarList(struct Node* VarList, InterCodes head){
  translate_ParamDec(VarList->son, head);
  char rule1[] = "ParamDec COMMA VarList";
  int isRule1 = compareExpression(VarList, rule1);
  if (isRule1 == 1) {
    translate_VarList(VarList->son->bro->bro, head);
  }
}

void translate_ParamDec(struct Node* ParamDec, InterCodes head) {
  Type type = getType_Specifier(ParamDec->son);
  getType_VarDec(ParamDec->son->bro, type, 0, 1, NULL, head);
}

void translate_CompSt(struct Node* CompSt, InterCodes head){
  translate_DefList(CompSt->son->bro, NULL, 0, head);
  translate_StmtList(CompSt->son->bro->bro, head);
}

void translate_StmtList(struct Node* StmtList, InterCodes head) {
  if (StmtList->son != NULL) {
    translate_Stmt(StmtList->son, head);
    translate_StmtList(StmtList->son->bro, head);
  }
}

void translate_Stmt(struct Node* Stmt, InterCodes head) {
  char rule1[] = "Exp SEMI";
  char rule2[] = "CompSt";
  char rule3[] = "RETURN Exp SEMI";
  char rule4[] = "IF LP Exp RP Stmt";
  char rule5[] = "IF LP Exp RP Stmt ELSE Stmt";
  char rule6[] = "WHILE LP Exp RP Stmt";
  if (compareExpression(Stmt, rule1) == 1) {
    translate_Exp(Stmt->son, head, NULL);
  } else if (compareExpression(Stmt, rule2) == 1) {
    translate_CompSt(Stmt->son, head);
  } else if (compareExpression(Stmt, rule3) == 1) {
    char* t1 = new_temp();
    translate_Exp(Stmt->son->bro, head, t1);
    // add new code of RETURN
    InterCode newCode = create_code_label(RETURN, create_operand(VARIABLE, 0, t1));
    addCode(head, newCode);
  } else if (compareExpression(Stmt, rule4) == 1) {
    char* label1 = new_label();
    char* label2 = new_label();
    translate_Cond(Stmt->son->bro->bro, label1, label2, head);
    // add new code of LABEL
    addCode(head, create_code_label(LABEL, create_operand(LABEL, 0, label1)));
    translate_Stmt(Stmt->son->bro->bro->bro->bro, head);
    // add new code of LABEL
    addCode(head, create_code_label(LABEL, create_operand(LABEL, 0, label2)));
  } else if (compareExpression(Stmt, rule5) == 1) {
    char* label1 = new_label();
    char* label2 = new_label();
    char* label3 = new_label();
    translate_Cond(Stmt->son->bro->bro, label1, label2, head);
    // add new code of LABEL
    addCode(head, create_code_label(LABEL, create_operand(LABEL, 0, label1)));
    translate_Stmt(Stmt->son->bro->bro->bro->bro, head);
    // add new code of GOTO
    addCode(head, create_code_label(GOTO, create_operand(LABEL, 0, label3)));
    // add new code of LABEL
    addCode(head, create_code_label(LABEL, create_operand(LABEL, 0, label2)));
    translate_Stmt(Stmt->son->bro->bro->bro->bro->bro->bro, head);
    // add new code of LABEL
    addCode(head, create_code_label(LABEL, create_operand(LABEL, 0, label3)));
  } else if (compareExpression(Stmt, rule6) == 1) {
    char* label1 = new_label();
    char* label2 = new_label();
    char* label3 = new_label();
    // add new code: LABEL label1
    addCode(head, create_code_label(LABEL, create_operand(LABEL, 0, label1)));
    translate_Cond(Stmt->son->bro->bro, label2, label3, head);
    // add new code: LABEL label2
    addCode(head, create_code_label(LABEL, create_operand(LABEL, 0, label2)));
    translate_Stmt(Stmt->son->bro->bro->bro->bro, head);
    // add new code: GOTO label1
    addCode(head, create_code_label(GOTO, create_operand(LABEL, 0, label1)));
    // add new code: LABEL label3
    addCode(head, create_code_label(LABEL, create_operand(LABEL, 0, label3)));
  }
}

void translate_DefList(struct Node* DefList, FieldList result, int insideStruct, InterCodes head) {
  if (DefList->son != NULL) {
    translate_Def(DefList->son, result, insideStruct, head);
    translate_DefList(DefList->son->bro, result, insideStruct, head);
  }
}

void translate_Def(struct Node* Def, FieldList result, int insideStruct, InterCodes head) {
  Type type = getType_Specifier(Def->son);
  translate_DecList(Def->son->bro, type, result, insideStruct, head);
}

void translate_DecList(struct Node* DecList, Type type, FieldList fieldList, int insideStruct, InterCodes head) {
  translate_Dec(DecList->son, type, fieldList, insideStruct, head);
  char rule1[] = "Dec";
  if (compareExpression(DecList, rule1)!= 1) {
    translate_DecList(DecList->son->bro->bro, type, fieldList, insideStruct, head);
  }
}

void translate_Dec(struct Node* Dec, Type type, FieldList fieldList, int insideStruct, InterCodes head) {

  char rule1[] = "VarDec";
  if (compareExpression(Dec, rule1) != 1) {
    getType_VarDec(Dec->son, type, insideStruct, 0, fieldList, head);
    char* t1 = new_temp();
    translate_Exp(Dec->son->bro->bro, head, t1);
    char* place = lookup_symbols(Dec->son->son->value);
    addCode(head, create_code_assign(ASSIGN,
                                     create_operand(VARIABLE, 0, place),
                                     create_operand(VARIABLE, 0, t1)));
  }else{
    getType_VarDec(Dec->son, type, insideStruct, 0, fieldList, head);
  }
}

void translate_Cond(struct Node* Exp, char* label_true, char* label_false, InterCodes head) {
  char rule1[] = "Exp RELOP Exp";
  char rule2[] = "NOT Exp";
  char rule3[] = "Exp AND Exp";
  char rule4[] = "Exp OR Exp";
  if (compareExpression(Exp, rule1) == 1) {
    char *t1 = new_temp();
    char *t2 = new_temp();
    translate_Exp(Exp->son, head, t1);
    translate_Exp(Exp->son->bro->bro, head, t2);
    addCode(head, create_code_goto_con(RELOP,
                                       create_operand(VARIABLE, 0, t1),
                                       create_operand(RELOP, 0, Exp->son->bro->value),
                                       create_operand(VARIABLE, 0, t2),
                                       create_operand(LABEL, 0, label_true)));
    addCode(head, create_code_label(GOTO, create_operand(LABEL, 0, label_false)));
  } else if (compareExpression(Exp, rule2) == 1) {
    translate_Cond(Exp->son->bro, label_false, label_true, head);
  } else if (compareExpression(Exp, rule3) == 1) {
    char *label1 = new_label();
    translate_Cond(Exp->son, label1, label_false, head);
    addCode(head, create_code_label(LABEL, create_operand(LABEL, 0, label1)));
    translate_Cond(Exp->son->bro->bro, label_true, label_false, head);
  } else if (compareExpression(Exp, rule4) == 1) {
    char *label1 = new_label();
    translate_Cond(Exp->son, label_true, label1, head);
    addCode(head, create_code_label(LABEL, create_operand(LABEL, 0, label1)));
    translate_Cond(Exp->son->bro->bro, label_true, label_false, head);
  } else {
    char *t1 = new_temp();
    translate_Exp(Exp, head, t1);
    char relop[] = "!=";
    addCode(head, create_code_goto_con(RELOP,
                                       create_operand(VARIABLE, 0, t1),
                                       create_operand(RELOP, 0, relop),
                                       create_operand(CONSTANT, 0, NULL),
                                       create_operand(LABEL, 0, label_true)));
    addCode(head, create_code_label(GOTO, create_operand(LABEL, 0, label_false)));
  }
}

Type translate_Exp(struct Node* Exp, InterCodes head, char* place) {
  char
    rule1[30] = "Exp ASSIGNOP Exp", // included in this func
    rule2[30] = "Exp AND Exp",
    rule3[30] = "Exp OR Exp",
    rule4[30] = "Exp RELOP Exp",
    rule5[30] = "Exp PLUS Exp",
    rule6[30] = "Exp MINUS Exp",
    rule7[30] = "Exp STAR Exp",
    rule8[30] = "Exp DIV Exp",
    rule9[30] = "LP Exp RP",
    rule10[30] = "MINUS Exp",
    rule11[30] = "NOT Exp",
    rule12[30] = "ID LP Args RP",
    rule13[30] = "ID LP RP",
    rule14[30] = "Exp LB Exp RB",
    rule15[30] = "Exp DOT ID",
    rule16[30] = "ID",
    rule17[30] = "INT",
    rule18[30] = "FLOAT"; // this will not appear
  Type result = NULL;
  if (compareExpression(Exp, rule17) == 1) {
    if (place != NULL) {
      addCode(head, create_code_assign(ASSIGN,
                                       create_operand(VARIABLE, 0, place),
                                       create_operand(CONSTANT, get_value(Exp->son), NULL)));
    }
    result = (Type) malloc(sizeof(struct Type_));
    memset(result, 0, sizeof(struct Type_));
    result->kind = BASIC;
    result->u.basic = INT;
  } else if (compareExpression(Exp, rule16) == 1) {
    struct Symbol *symbol = find_symbol_intercodes(hash_table, Exp->son->value, VARIABLE);
    if (place != NULL) {
      if (symbol->type->kind == BASIC) {
        addCode(head, create_code_assign(ASSIGN,
                                         create_operand(VARIABLE, 0, place),
                                         create_operand(VARIABLE, 0, lookup_symbols(Exp->son->value))));
      } else {
        if(isAddress(Exp->son->value)!=1) {
          addCode(head, create_code_assign(RIGHT_ADDR,
                                           create_operand(VARIABLE, 0, place),
                                           create_operand(VARIABLE, 0, lookup_symbols(Exp->son->value))));
        }else{
          addCode(head, create_code_assign(ASSIGN,
                                           create_operand(VARIABLE, 0, place),
                                           create_operand(VARIABLE, 0, lookup_symbols(Exp->son->value))));
        }
      }
    }
    result = symbol->type;
  } else if ((compareExpression(Exp, rule5) == 1)
             || (compareExpression(Exp, rule6) == 1)
             || (compareExpression(Exp, rule7) == 1)
             || (compareExpression(Exp, rule8) == 1)) {
    char *t1 = new_temp();
    char *t2 = new_temp();
    translate_Exp(Exp->son, head, t1);
    translate_Exp(Exp->son->bro->bro, head, t2);
    if(place!=NULL) {
      addCode(head, create_code_binop(MATHOP,
                                      create_operand(VARIABLE, 0, place),
                                      create_operand(VARIABLE, 0, t1),
                                      create_operand(MATHOP, 0, Exp->son->bro->value),
                                      create_operand(VARIABLE, 0, t2)));
    }
    result = (Type) malloc(sizeof(struct Type_));
    memset(result, 0, sizeof(struct Type_));
    result->kind = BASIC;
    result->u.basic = INT;
  } else if (compareExpression(Exp, rule9) == 1) {
    result = translate_Exp(Exp->son->bro, head, place);
  } else if (compareExpression(Exp, rule10) == 1) {
    char *t1 = new_temp();
    translate_Exp(Exp->son->bro, head, t1);
    if(place!=NULL) {
      char *mathop = malloc(2 * sizeof(char));
      sprintf(mathop, "-");
      addCode(head, create_code_binop(MATHOP,
                                      create_operand(VARIABLE, 0, place),
                                      create_operand(CONSTANT, 0, NULL),
                                      create_operand(MATHOP, 0, mathop),
                                      create_operand(VARIABLE, 0, t1)));
    }
  } else if ((compareExpression(Exp, rule2) == 1)
             || (compareExpression(Exp, rule3) == 1)
             || (compareExpression(Exp, rule4) == 1)
             || (compareExpression(Exp, rule11) == 1)) {
    if(place!=NULL) {
      char *label1 = new_label();
      char *label2 = new_label();
      addCode(head, create_code_assign(ASSIGN,
                                       create_operand(VARIABLE, 0, place),
                                       create_operand(CONSTANT, 0, NULL)));
      translate_Cond(Exp, label1, label2, head);
      addCode(head, create_code_label(LABEL, create_operand(LABEL, 0, label1)));
      addCode(head, create_code_assign(ASSIGN,
                                       create_operand(VARIABLE, 0, place),
                                       create_operand(CONSTANT, 1, NULL)));
      addCode(head, create_code_label(LABEL, create_operand(LABEL, 0, label2)));
    }else{
      char *label1 = new_label();
      char *label2 = new_label();
      translate_Cond(Exp, label1, label2, head);
    }
    result = (Type) malloc(sizeof(struct Type_));
    memset(result, 0, sizeof(struct Type_));
    result->kind = BASIC;
    result->u.basic = INT;
  } else if (compareExpression(Exp, rule13) == 1) {
    char read[] = "read";
    if(place!=NULL) {
      if (strcmp(Exp->son->value, read) == 0) {
        addCode(head, create_code_label(READ,
                                        create_operand(VARIABLE, 0, place)));
      } else {
        addCode(head, create_code_assign(CALL,
                                         create_operand(VARIABLE, 0, place),
                                         create_operand(VARIABLE, 0, Exp->son->value)));
      }
    }
    result = (Type) malloc(sizeof(struct Type_));
    memset(result, 0, sizeof(struct Type_));
    result->kind = BASIC;
    result->u.basic = INT;
  } else if (compareExpression(Exp, rule12) == 1) {
    Arg_list arg_list = (Arg_list) malloc(sizeof(struct Arg_list_));
    memset(arg_list, 0, sizeof(struct Arg_list_));
    translate_Args(Exp->son->bro->bro, head, arg_list);
    char write[] = "write";
    if (strcmp(Exp->son->value, write)==0) {
      addCode(head, create_code_label(WRITE,
                                      create_operand(VARIABLE, 0, arg_list->next->arg)));
    } else {
      Arg_list temp = arg_list->next;
      while (temp != NULL) {
        addCode(head, create_code_label(ARG,
                                        create_operand(VARIABLE, 0, temp->arg)));
        temp = temp->next;
      }
      if(place!=NULL) {
        addCode(head, create_code_assign(CALL,
                                         create_operand(VARIABLE, 0, place),
                                         create_operand(VARIABLE, 0, Exp->son->value)));
      }
    }
    result = (Type) malloc(sizeof(struct Type_));
    memset(result, 0, sizeof(struct Type_));
    result->kind = BASIC;
    result->u.basic = INT;
  } else if (compareExpression(Exp, rule14) == 1) {
    char *t1 = new_temp();
    result = translate_Exp(Exp->son, head, t1); // basic
    char *t2 = new_temp();
    translate_Exp(Exp->son->bro->bro, head, t2); // offset
    result = result->u.array.elem;
    if(place!=NULL) {
      int size = calculate_space(result);
      char *mathop1 = (char *) malloc(2 * sizeof(char));
      sprintf(mathop1, "*");
      addCode(head, create_code_binop(MATHOP,
                                      create_operand(VARIABLE, 0, place),
                                      create_operand(CONSTANT, size, NULL),
                                      create_operand(MATHOP, 0, mathop1),
                                      create_operand(VARIABLE, 0, t2)));
      char *mathop2 = (char *) malloc(2 * sizeof(char));
      sprintf(mathop2, "+");
      addCode(head, create_code_binop(MATHOP,
                                      create_operand(VARIABLE, 0, place),
                                      create_operand(VARIABLE, 0, place),
                                      create_operand(MATHOP, 0, mathop2),
                                      create_operand(VARIABLE, 0, t1)));
      if (result->kind == BASIC) {
        addCode(head, create_code_assign(RIGHT_STAR,
                                         create_operand(VARIABLE, 0, place),
                                         create_operand(VARIABLE, 0, place)));
      }
    }
  } else if (compareExpression(Exp, rule15) == 1) {
    char *t1 = new_temp();
    result = translate_Exp(Exp->son, head, t1);
    int size = calculate_offset_in_struct(result->u.structure, Exp->son->bro->bro->value);
    result = get_type_in_struct(result->u.structure, Exp->son->bro->bro->value);
    if(place!=NULL) {
      char *mathop = (char *) malloc(2 * sizeof(char));
      sprintf(mathop, "+");
      addCode(head, create_code_binop(MATHOP,
                                      create_operand(VARIABLE, 0, place),
                                      create_operand(CONSTANT, size, NULL),
                                      create_operand(MATHOP, 0, mathop),
                                      create_operand(VARIABLE, 0, t1)));
      if (result->kind == BASIC) {
        addCode(head, create_code_assign(RIGHT_STAR,
                                         create_operand(VARIABLE, 0, place),
                                         create_operand(VARIABLE, 0, place)));
      }
    }
  } else if (compareExpression(Exp, rule1) == 1) {
    struct Node *Exp1 = Exp->son;
    printf("here11!\n");
    printf("%s!\n", Exp1->son->name);
    printf("%s!\n", rule13);
    if (compareExpression(Exp1, rule16) == 1) {  // ID
      printf("here3!\n");
      char *variable = lookup_symbols(Exp1->son->value);
      char *t1 = new_temp();
      result = translate_Exp(Exp1->bro->bro, head, t1);
      if (result->kind == BASIC) {
        addCode(head, create_code_assign(ASSIGN,
                                         create_operand(VARIABLE, 0, variable),
                                         create_operand(VARIABLE, 0, t1)));
        if(place!=NULL) {
          addCode(head, create_code_assign(ASSIGN,
                                           create_operand(VARIABLE, 0, place),
                                           create_operand(VARIABLE, 0, variable)));
        }
      } else {
        char *t2 = new_temp();
        addCode(head, create_code_assign(RIGHT_STAR,
                                         create_operand(VARIABLE, 0, t2),
                                         create_operand(VARIABLE, 0, t1)));
        if(isAddress(Exp1->son->value)==1) {
          addCode(head, create_code_assign(LEFT_STAR,
                                           create_operand(VARIABLE, 0, variable),
                                           create_operand(VARIABLE, 0, t2)));
        }else{
          char *t3 = new_temp();
          addCode(head, create_code_assign(RIGHT_ADDR,
                                           create_operand(VARIABLE, 0, t3),
                                           create_operand(VARIABLE, 0, variable)));
          addCode(head, create_code_assign(LEFT_STAR,
                                           create_operand(VARIABLE, 0, t3),
                                           create_operand(VARIABLE, 0, t2)));
        }
        if(place!=NULL) {
          addCode(head, create_code_assign(ASSIGN,
                                           create_operand(VARIABLE, 0, place),
                                           create_operand(VARIABLE, 0, t1)));
        }
      }
    } else if (compareExpression(Exp1, rule14) == 1) {  // Exp LB Exp RB
      printf("here2!\n");
      char *t1 = new_temp();
      result = translate_Exp(Exp1->son, head, t1); // basic
      char *t2 = new_temp();
      translate_Exp(Exp1->son->bro->bro, head, t2); // offset
      result = result->u.array.elem;
      int size = calculate_space(result);
      char* mathop1 = (char*)malloc(2*sizeof(char));
      sprintf(mathop1, "*");
      char* address = new_temp();
      addCode(head, create_code_binop(MATHOP,
                                      create_operand(VARIABLE, 0, address),
                                      create_operand(CONSTANT, size, NULL),
                                      create_operand(MATHOP, 0, mathop1),
                                      create_operand(VARIABLE, 0, t2)));
      char* mathop2 = (char*)malloc(2*sizeof(char));
      sprintf(mathop2, "+");
      addCode(head, create_code_binop(MATHOP,
                                      create_operand(VARIABLE, 0, address),
                                      create_operand(VARIABLE, 0, address),
                                      create_operand(MATHOP, 0, mathop2),
                                      create_operand(VARIABLE, 0, t1)));
      char *t3 = new_temp();
      result = translate_Exp(Exp1->bro->bro, head, t3);
      if (result->kind == BASIC) {
        addCode(head, create_code_assign(LEFT_STAR,
                                         create_operand(VARIABLE, 0, address),
                                         create_operand(VARIABLE, 0, t3)));
        if(place!=NULL) {
          addCode(head, create_code_assign(ASSIGN,
                                           create_operand(VARIABLE, 0, place),
                                           create_operand(VARIABLE, 0, t3)));
        }
      } else {
        char *t4 = new_temp();
        addCode(head, create_code_assign(RIGHT_STAR,
                                         create_operand(VARIABLE, 0, t4),
                                         create_operand(VARIABLE, 0, t3)));
        addCode(head, create_code_assign(LEFT_STAR,
                                         create_operand(VARIABLE, 0, address),
                                         create_operand(VARIABLE, 0, t4)));
        if(place!=NULL) {
          addCode(head, create_code_assign(ASSIGN,
                                           create_operand(VARIABLE, 0, place),
                                           create_operand(VARIABLE, 0, address)));
        }
      }
    } else { //  Exp DOT ID
      printf("here123!\n");
      char *t1 = new_temp();
      char* address = new_temp();
      result = translate_Exp(Exp1->son, head, t1);
      int size = calculate_offset_in_struct(result->u.structure, Exp1->son->bro->bro->value);
      result = get_type_in_struct(result->u.structure, Exp1->son->bro->bro->value);
      char* mathop = (char*)malloc(2*sizeof(char));
      sprintf(mathop, "+");
      addCode(head, create_code_binop(MATHOP,
                                      create_operand(VARIABLE, 0, address),
                                      create_operand(CONSTANT, size, NULL),
                                      create_operand(MATHOP, 0, mathop),
                                      create_operand(VARIABLE, 0, t1)));
      char *t3 = new_temp();
      result = translate_Exp(Exp1->bro->bro, head, t3);
      if (result->kind == BASIC) {
        addCode(head, create_code_assign(LEFT_STAR,
                                         create_operand(VARIABLE, 0, address),
                                         create_operand(VARIABLE, 0, t3)));
        if(place!=NULL) {
          addCode(head, create_code_assign(ASSIGN,
                                           create_operand(VARIABLE, 0, place),
                                           create_operand(VARIABLE, 0, t3)));
        }
      } else {
        char *t4 = new_temp();
        addCode(head, create_code_assign(RIGHT_STAR,
                                         create_operand(VARIABLE, 0, t4),
                                         create_operand(VARIABLE, 0, t3)));
        addCode(head, create_code_assign(LEFT_STAR,
                                         create_operand(VARIABLE, 0, address),
                                         create_operand(VARIABLE, 0, t4)));
        if(place!=NULL) {
          addCode(head, create_code_assign(ASSIGN,
                                           create_operand(VARIABLE, 0, place),
                                           create_operand(VARIABLE, 0, address)));
        }
      }
    }
  }
  return result;
}

void translate_Args(struct Node* Args, InterCodes head, Arg_list arg_list) {
  char rule2[] = "Exp COMMA Args";
  if (compareExpression(Args, rule2) != 1) {
    char *t1 = new_temp();
    translate_Exp(Args->son, head, t1);
    Arg_list newArgs = (Arg_list) malloc(sizeof(struct Arg_list_));
    memset(newArgs, 0, sizeof(struct Arg_list_));
    newArgs->next = arg_list->next;
    arg_list->next = newArgs;
    newArgs->arg = t1;
  } else {
    char *t1 = new_temp();
    translate_Exp(Args->son, head, t1);
    Arg_list newArgs = (Arg_list) malloc(sizeof(struct Arg_list_));
    memset(newArgs, 0, sizeof(struct Arg_list_));
    newArgs->next = arg_list->next;
    arg_list->next = newArgs;
    newArgs->arg = t1;
    translate_Args(Args->son->bro->bro, head, arg_list);
  }
}

int calculate_space(Type type) {
  if (type->kind == BASIC) {
    if (type->u.basic == INT) {
      return 4;
    } else if (type->u.basic == FLOAT) {
      return 8;
    }
  } else if (type->kind == ARRAY) {
    return (type->u.array.size * calculate_space(type->u.array.elem));
  } else if (type->kind == STRUCTURE) {
    int size = 0;
    FieldList temp = type->u.structure;
    while ((temp != NULL) && (temp->name != NULL) && (temp->type != NULL)) {
      size += calculate_space(temp->type);
      temp = temp->tail;
    }
    return size;
  }
  return 0;
}

int calculate_offset_in_struct(FieldList fieldList, char* name) {
  int size = 0;
  FieldList temp = fieldList;
  while ((temp != NULL) && (temp->name != NULL) && (temp->type != NULL)) {
    if (strcmp(name, temp->name) != 0) {
      size += calculate_space(temp->type);
      temp = temp->tail;
    } else {
      break;
    }
  }
  return size;
}

Type get_type_in_struct(FieldList fieldList, char* name) {
  FieldList temp = fieldList;
  while ((temp != NULL) && (temp->name != NULL) && (temp->type != NULL)) {
    if (strcmp(name, temp->name) != 0) {
      temp = temp->tail;
    } else {
      return temp->type;
    }
  }
  return NULL;
}

//LABEL, FUNCTION, ASSIGN, MATHOP,
//       RIGHT_ADDR, RIGHT_STAR, LEFT_STAR, GOTO, RELOP, RETURN, DEC, ARG,
//       CALL, PARAM, READ, WRITE

char* printCodes(InterCode interCode) {
  char* code=(char*)malloc(50*sizeof(char));
  memset(code, 0, 50*sizeof(char));
  if (interCode->kind == LABEL) {
    sprintf(code, "LABEL %s :", print_operand(interCode->u.label.label, 0));
  } else if (interCode->kind == FUNCTION) {
    sprintf(code, "FUNCTION %s :", print_operand(interCode->u.label.label, 0));
  } else if (interCode->kind == ASSIGN) {
    sprintf(code, "%s := %s",
            print_operand(interCode->u.assign.left, 1),
            print_operand(interCode->u.assign.right, 1));
  } else if (interCode->kind == MATHOP) {
    sprintf(code, "%s := %s %s %s",
            print_operand(interCode->u.binop.result, 1),
            print_operand(interCode->u.binop.op1, 1),
            print_operand(interCode->u.binop.mathop, 1),
            print_operand(interCode->u.binop.op2, 1));
  } else if (interCode->kind == RIGHT_ADDR) {
    sprintf(code, "%s := &%s",
            print_operand(interCode->u.assign.left, 1),
            print_operand(interCode->u.assign.right, 1));
  } else if (interCode->kind == RIGHT_STAR) {
    sprintf(code, "%s := *%s",
            print_operand(interCode->u.assign.left, 1),
            print_operand(interCode->u.assign.right, 1));
  } else if (interCode->kind == LEFT_STAR) {
    sprintf(code, "*%s := %s",
            print_operand(interCode->u.assign.left, 1),
            print_operand(interCode->u.assign.right, 1));
  } else if (interCode->kind == GOTO) {
    sprintf(code, "GOTO %s", print_operand(interCode->u.label.label, 0));
  } else if (interCode->kind == RELOP) {
    sprintf(code, "IF %s %s %s GOTO %s",
            print_operand(interCode->u.goto_con.left, 1),
            print_operand(interCode->u.goto_con.relop, 1),
            print_operand(interCode->u.goto_con.right, 1),
            print_operand(interCode->u.goto_con.label, 1));
  } else if (interCode->kind == RETURN) {
    sprintf(code, "RETURN %s", print_operand(interCode->u.label.label, 1));
  } else if (interCode->kind == DEC) {
    sprintf(code, "DEC %s %s",
            print_operand(interCode->u.assign.left, 1),
            print_operand(interCode->u.assign.right, 0));
  } else if (interCode->kind == ARG) {
    sprintf(code, "ARG %s", print_operand(interCode->u.label.label, 1));
  } else if (interCode->kind == CALL) {
    sprintf(code, "%s := CALL %s",
            print_operand(interCode->u.assign.left, 1),
            print_operand(interCode->u.assign.right, 1));
  } else if (interCode->kind == PARAM) {
    sprintf(code, "PARAM %s", print_operand(interCode->u.label.label, 1));
  } else if (interCode->kind == READ) {
    sprintf(code, "READ %s", print_operand(interCode->u.label.label, 1));
  } else if (interCode->kind == WRITE) {
    sprintf(code, "WRITE %s", print_operand(interCode->u.label.label, 1));
  }
  return code;
}