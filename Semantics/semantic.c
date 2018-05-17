#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Lex/tree.h"
#include "data_type.h"

extern Node* root;
extern struct Hash_table* hash_table;

void check_error(Node* tree_root);
Type createType_Specifier(Node* Specifier);
Type createType_StructSpecifier(Node* StructSpecifier);
Type valueType_ExtDecList(Node* ExtDecList, Type type);
Type createType_VarDec(Node* VarDec, Type type, int insideStruct, FieldList fieldList);
void createSymbol_function_FunDec(struct Node* FunDec, Type returnType);

Type valueSymbol_function_VarList(struct Node* VarList, struct Symbol_function* function);
Type createType_ParamDec(struct Node* ParamDec);

void check_error_CompSt(struct Node* CompSt, int rightAfterFunc, Type returnType);
void check_error_StmtList(struct Node* StmtList, Type returnType);
void check_error_Stmt(struct Node* Stmt, Type returnType);

FieldList createFieldList_DefList(Node* DefList, int insideStruct);
FieldList createFieldList_Def(struct Node* DefList, int insideStruct);

Type getType_Exp(struct Node* Exp);
int compare_type_type(Type type_a, Type type_b);
int compare_type_kind(Type type, int kind);
void print_error(int error_type, int lineno);

int compareSubExpression(Node* tree_root, char* name){
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

void check_error(Node* tree_root){
  char str[] = "Specifier FunDec CompSt";
  int result = compareSubExpression(tree_root, str);
  if(result==1){
    printf("%s %s %d\n", tree_root->name, tree_root->value, tree_root->lineno);
  }
  if(tree_root->son!=NULL){
    check_error(tree_root->son);
  }
}

Type createType_Specifier(Node* Specifier){
  char rule1[] = "TYPE";
  char rule2[] = "StructSpecifier";
  int isRule1 = compareSubExpression(Specifier, rule1);
  if(isRule1==1){
    Type result = (Type)malloc(sizeof(struct Type_));
    if(strcmp(Specifier->son->value, "int")==0) {
      result->kind = BASIC;
      result->u.basic = INT;
    }else{
      result->kind = BASIC;
      result->u.basic = FLOAT;
    }
    return result;
  }else{
    return createType_StructSpecifier(Specifier->son);
  }
}

Type createType_StructSpecifier(Node* StructSpecifier){
  char rule1[] = "STRUCT OptTag LC DefList RC";
  int isRule1 = compareSubExpression(StructSpecifier, rule1);
  if(isRule1==1){
    Type result = (Type)malloc(sizeof(struct Type_));
    Node* OptTag = StructSpecifier->son->bro;
    FieldList structure;
    push_env(hash_table);
    structure = createFieldList_DefList(OptTag->bro->bro, 1);
    pop_env(hash_table);
    result->u.structure = structure;
    result->kind = STRUCTURE;
    if(OptTag->son!=NULL){
      struct Symbol symbol;
      insert_symbol(hash_table, OptTag->son->value, STRUCT, result, NULL, OptTag->son->lineno);
    }
    return result;
  }else{
    Node* Tag = StructSpecifier->son->bro;
    struct Symbol temp;
    struct Symbol* structure = find_symbol(hash_table, Tag->value, STRUCT);
    if(structure == NULL){
      print_error(17, Tag->lineno);
    }
    return structure->type;
  }
}

Type valueType_ExtDecList(Node* ExtDecList, Type type){
  char rule2[] = "VarDec COMMA ExtDecList";
  int isRule2 = compareSubExpression(ExtDecList, rule2);
  if(isRule2!=1){
    createType_VarDec(ExtDecList->son, type, 0, NULL);
  }else{
    createType_VarDec(ExtDecList->son, type, 0, NULL);
    valueType_ExtDecList(ExtDecList->son->bro->bro, type);
  }
  return type;
}

Type createType_VarDec(Node* VarDec, Type type, int insideStruct, FieldList fieldList){
  char rule1[] = "ID";
  int isRule1 = compareSubExpression(VarDec, rule1);
  if(isRule1==1) {
    if(insideStruct != 1){
    insert_symbol(hash_table, VarDec->son->value, VARIABLE, type, NULL, VarDec->son->lineno);
    }else{
      if(fieldList->name==NULL){
        fieldList->name = VarDec->son->value;
        fieldList->type = type;
      }else {
        char* name = VarDec->son->value;
        // begin to check the name redundancy
        FieldList temp = fieldList;
        while (temp->tail != NULL) {
          if(strcmp(temp->name, name)==0){
            print_error(15, VarDec->son->lineno);
            return type;
          }
          temp = temp->tail;
        }
        // no name redundancy
        FieldList new_field = (FieldList) malloc(sizeof(FieldList_));
        new_field->name = name;
        new_field->type = type;
        temp->tail = new_field;
      }
    }
    return type;
  }else{
    int size = strtol(VarDec->son->bro->bro->value, NULL, 0);
    Type result = (Type)malloc(sizeof(struct Type_));
    result->kind = ARRAY;
    result->u.array.size = size;
    result->u.array.elem = type;
    createType_VarDec(VarDec->son, result, insideStruct, fieldList);
    return result;
  }
}

void createSymbol_function_FunDec(struct Node* FunDec, Type returnType){
  char rule1[] = "ID LP VarList RP";
  int isRule1 = compareSubExpression(FunDec, rule1);
  struct Symbol_function* symbol_function = (struct Symbol_function*)malloc(sizeof(struct Symbol_function));
  symbol_function->argc = 0;
  symbol_function->return_type = returnType;
  push_env(hash_table);
  if(isRule1==1){
    valueSymbol_function_VarList(FunDec->son->bro->bro, symbol_function);
  }
  insert_symbol(hash_table, FunDec->son->value, FUNC, NULL, symbol_function, FunDec->son->lineno);
}

Type valueSymbol_function_VarList(struct Node* VarList, struct Symbol_function* function){
  char rule1[] = "ParamDec COMMA VarList";
  int isRule1 = compareSubExpression(VarList, rule1);
  struct argv* p = function->argv1;
  Type paramDec_type = createType_ParamDec(VarList->son);
  struct argv* temp = (struct argv*)malloc(sizeof(struct argv));
  temp->type = paramDec_type;
  temp->next = NULL;
  if(p==NULL){
    function->argv1 = temp;
  }else{
    while(p->next!=NULL){
      p = p->next;
    }
    p->next = temp;
  }
  function->argc = function->argc + 1;
  if(isRule1==1){
    valueSymbol_function_VarList(VarList->son->bro->bro, function);
  }
  return NULL;
}

Type createType_ParamDec(struct Node* ParamDec){
  Type type = createType_Specifier(ParamDec->son);
  insert_symbol(hash_table, ParamDec->son->bro->value, VARIABLE, type, NULL, ParamDec->son->bro->lineno);
  return type;
}

// Statements

void check_error_CompSt(struct Node* CompSt, int rightAfterFunc, Type returnType){
  push_env(hash_table);
  createFieldList_DefList(CompSt->son->bro, 0);
  check_error_StmtList(CompSt->son->bro->bro, returnType);
  pop_env(hash_table);
  if(rightAfterFunc){
    pop_env(hash_table);
  }
}

void check_error_StmtList(struct Node* StmtList, Type returnType){
  if(StmtList->son!=NULL){
    check_error_Stmt(StmtList->son, returnType);
    check_error_StmtList(StmtList->son->bro, returnType);
  }
}

void check_error_Stmt(struct Node* Stmt, Type returnType){
  char rule1[] = "Exp SEMI";
  char rule2[] = "CompSt";
  char rule3[] = "RETURN Exp SEMI";
  char rule4[] = "IF LP Exp RP Stmt";
  char rule5[] = "IF LP Exp RP Stmt ELSE Stmt";
  char rule6[] = "WHILE LP Exp RP Stmt";
  if(compareSubExpression(Stmt, rule1)==1){
    getType_Exp(Stmt->son);
  }else if(compareSubExpression(Stmt, rule2)==1){
    check_error_CompSt(Stmt->son, 0, returnType);
  }else if(compareSubExpression(Stmt, rule3)==1){
    if(returnType!=NULL){
      if(compare_type_type(getType_Exp(Stmt->son->bro), returnType)!=1){
        print_error(8, Stmt->son->lineno);
      }
    }
  }else if(compareSubExpression(Stmt, rule4)==1){
    if(compare_type_kind(getType_Exp(Stmt->son->bro->bro), INT)!=1){
      print_error(7, Stmt->son->lineno);
    }
    check_error_Stmt(Stmt->son->bro->bro->bro->bro, returnType);
  }else if(compareSubExpression(Stmt, rule5)==1){
    if(compare_type_kind(getType_Exp(Stmt->son->bro->bro), INT)!=1){
      print_error(7, Stmt->son->lineno);
    }
    check_error_Stmt(Stmt->son->bro->bro->bro->bro, returnType);
    check_error_Stmt(Stmt->son->bro->bro->bro->bro->bro->bro, returnType);
  }else if(compareSubExpression(Stmt, rule6)==1){
    if(compare_type_kind(getType_Exp(Stmt->son->bro->bro), INT)!=1){
      print_error(7, Stmt->son->lineno);
    }
    check_error_Stmt(Stmt->son->bro->bro->bro->bro, returnType);
  }
}

// LOCAL Definitions

FieldList createFieldList_DefList(struct Node* DefList, int insideStruct){
  if(DefList->son!=NULL){
    FieldList head = createFieldList_Def(DefList->son, insideStruct);
    FieldList tail = createFieldList_DefList(DefList->son->bro, insideStruct);
    FieldList p = head;
    while(p->tail!=NULL){
      p = p->tail;
    }
    p->tail = tail;
    return head;
  }else{
    return NULL;
  }
}

FieldList createFieldList_Def(struct Node* Def, int insideStruct){
  Type type = createType_Specifier(Def->son);
  FieldList result = (FieldList)malloc(FieldList_);
  result->name = NULL;
  createFieldList_DecList(Def->son->bro, type, result, insideStruct);
  return result;
}

void createFieldList_DecList(struct Node* DecList, Type type, FieldList fieldList, int insideStruct){
  createFieldList_Dec(DecList->son, type, fieldList, insideStruct);
  char rule1[] = "Dec";
  int isRule1 = compareSubExpression(DecList, rule1);
  if(isRule1!=1){
    createFieldList_DecList(DecList->son->bro->bro, type, fieldList, insideStruct);
  }
}

void createFieldList_Dec(struct Node* Dec, Type type, FieldList fieldList, int insideStruct){
  char rule1[] = "VarDec";
  int isRule1 = compareSubExpression(Dec, rule1);
  if(isRule1!=1){
    if(insideStruct==1){
      print_error(15, Dec->son->lineno);
    }
    if(!(((compare_type_kind(type, INT)==1)&&(compare_type_kind(getType_Exp(Dec->son->bro->bro), INT)==1))|| \
      ((compare_type_kind(type, FLOAT)==1)&&(compare_type_kind(getType_Exp(Dec->son->bro->bro), FLOAT)==1)))){
      print_error(5, Dec->son->lineno);
    }
  }
  createType_VarDec(Dec->son, type, insideStruct, fieldList);
}

Type getType_Exp(struct Node* Exp){
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
  if(compareSubExpression(Exp, rule1)){

  }else if(compareSubExpression(Exp, rule2)){

  }else if(compareSubExpression(Exp, rule3)){

  }else if(compareSubExpression(Exp, rule4)){

  }else if(compareSubExpression(Exp, rule5)){

  }else if(compareSubExpression(Exp, rule6)){

  }else if(compareSubExpression(Exp, rule7)){

  }else if(compareSubExpression(Exp, rule8)){

  }else if(compareSubExpression(Exp, rule9)){

  }else if(compareSubExpression(Exp, rule10)){

  }else if(compareSubExpression(Exp, rule11)){

  }else if(compareSubExpression(Exp, rule12)){

  }else if(compareSubExpression(Exp, rule13)){
    //check whether the function exists in symbol

  }else if(compareSubExpression(Exp, rule14)) {
    // check whether the first Exp is an array
    Type first_exp = getType_Exp(Exp->son);
    if((first_exp==NULL)||(first_exp->kind!=ARRAY)){
      print_error(10, Exp->son->lineno);
    }else{
      // check whether the index is an integer
      Type index = getType_Exp(Exp->son->bro->bro);
      if ((index == NULL) || (index->kind != BASIC) || (index->u.basic != INT)) {
        print_error(12, Exp->son->lineno);
      } else {
        result = first_exp->u.array.elem;
      }
    }
  }else if(compareSubExpression(Exp, rule15)){
    Type first_exp_type = getType_Exp(Exp->son);
    if((first_exp_type==NULL)||(first_exp_type->kind!=STRUCTURE)){
      print_error(13, Exp->son->lineno);
    }else{
      //check whether there is a matching ID in Exp.type
      FieldList temp = first_exp_type->u.structure;
      while (temp!= NULL) {
        if(strcmp(temp->name, Exp->son->bro->bro->value)==0){
          result = temp->type;
          break;
        }
        temp = temp->tail;
      }
      if(temp==NULL){
        print_error(14, Exp->son->lineno);
      }
    }
  }else if(compareSubExpression(Exp, rule16)){
    struct Symbol* symbol = find_symbol(hash_table, Exp->son->value, VARIABLE);
    if(symbol==NULL){
      print_error(1, Exp->son->lineno);
    }else {
      result = symbol->type;
    }
  }else if(compareSubExpression(Exp, rule17)){
    result = (Type)malloc(sizeof(Type_));
    result->kind = BASIC;
    result->u.basic = INT;
  }else if(compareSubExpression(Exp, rule18)){
    result = (Type)malloc(sizeof(Type_));
    result->kind = BASIC;
    result->u.basic = FLOAT;
  }
  return result;
}

void print_error(int error_type, int lineno){
  printf("Error type %d at Line %d: .", error_type, lineno);
}

int compare_type_kind(Type type, int kind){
  if(type==NULL){
    return -1;
  }
  //TODO: implement the rest comparison
  return -1;
}

int compare_type_type(Type type_a, Type type_b){
  if((type_a==NULL)||(type_b==NULL)){
    return -1;
  }
  //TODO: implement the rest comparison
  return -1;
}