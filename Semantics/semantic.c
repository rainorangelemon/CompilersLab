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
FieldList createFieldList_DefList(Node* DefList, int insideStruct);
int compareType(struct Symbol* symbol_a, struct Symbol* symbol_b);
Type valueType_ExtDecList(Node* ExtDecList, Type type);
Type createType_VarDec(Node* VarDec, Type type);
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
    if(strcmp(OptTag->value, "")!=0){
      struct Symbol symbol;
      insert_symbol(hash_table, OptTag->name, STRUCT, result, NULL);
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

FieldList createFieldList_DefList(Node* DefList, int insideStruct){

}

int compareType(struct Symbol* symbol_a, struct Symbol* symbol_b){
  //TODO: implement the rest comparison
  return -1;
}

Type valueType_ExtDecList(Node* ExtDecList, Type type){
  char rule2[] = "VarDec COMMA ExtDecList";
  int isRule2 = compareSubExpression(ExtDecList, rule2);
  if(isRule2!=1){
    createType_VarDec(ExtDecList->son, type);
  }else{
    createType_VarDec(ExtDecList->son, type);
    valueType_ExtDecList(ExtDecList->son->bro->bro, type);
  }
  return type;
}

Type createType_VarDec(Node* VarDec, Type type){
  char rule1[] = "ID";
  int isRule1 = compareSubExpression(VarDec, rule1);
  if(isRule1==1){
    struct Symbol temp;
    insert_symbol(hash_table, VarDec->son->value, VARIABLE, type, NULL);
    return type;
  }else{
    int size = strtol(VarDec->son->bro->bro->value, NULL, 0);
    Type result = (Type)malloc(sizeof(struct Type_));
    result->kind = ARRAY;
    result->u.array.size = size;
    result->u.array.elem = type;
    createType_VarDec(VarDec->son, result);
    return result;
  }
}



void print_error(int error_type, int lineno){
  printf("Error type %d at Line %d: .", error_type, lineno);
}