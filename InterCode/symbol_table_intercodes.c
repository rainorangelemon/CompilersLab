//
// Created by rainorangelemon on 6/7/18.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../Semantics/data_type.h"

void insert_symbol_intercodes(struct Hash_table* hash_table, char* name, int kind, Type type, struct Symbol_function* function){
  struct Symbol *new_symbol = (struct Symbol *) malloc(sizeof(struct Symbol));
  memset(new_symbol, 0, sizeof(struct Symbol));
  // get the index of new symbol
  unsigned int index = get_hash(name);
  new_symbol->index = index;
  new_symbol->kind = kind;
  new_symbol->name = name;
  new_symbol->code_name = NULL;
  if (kind != FUNC) {
    new_symbol->type = type;
    // insert the symbol
    new_symbol->right = hash_table->hash_table[index];
    hash_table->hash_table[index] = new_symbol;
  } else {
    new_symbol->function = function;
    // insert the symbol
    new_symbol->right = hash_table->hash_table[index];
    hash_table->hash_table[index] = new_symbol;
  }
}

struct Symbol* find_symbol_intercodes(struct Hash_table* hash_table, char* name, int kind){
  unsigned int index = get_hash(name);
  struct Symbol* temp = hash_table->hash_table[index];
  while(temp!=NULL){
    if((strcmp(name, temp->name)==0)&&(temp->kind==kind)){
      break;
    }else{
      temp = temp->right;
    }
  }
  return temp;
}