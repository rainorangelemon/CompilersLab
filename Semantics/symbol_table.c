#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "data_type.h"

unsigned int get_hash(char* name){
  unsigned int val = 0, i;
  for(; *name; ++name){
    val = (val << 2) + *name;
    if( i = val & ~hash_size) val = (val ^ (i >> 12)) & hash_size;
  }
  return val;
}

struct Hash_table* create_table(){
  struct Hash_table* result = (struct Hash_table*)malloc(sizeof(struct Hash_table));
  memset(result, 0, sizeof(struct Hash_table));
  return result;
}

void push_env(struct Hash_table* hash_table) {
  struct Stack_node* new_stack_node = (struct Stack_node*)malloc(sizeof(struct Stack_node));
  if(hash_table->stack_head==NULL){
    new_stack_node->depth=0;
    hash_table->stack_head = new_stack_node;
  }else{
    new_stack_node->left=hash_table->stack_head;
    new_stack_node->depth=((hash_table->stack_head->depth)+1);
    hash_table->stack_head = new_stack_node;
  }
}

void pop_env(struct Hash_table* hash_table){
  struct Stack_node* head_node = hash_table->stack_head;
  // delete symbols
  struct Symbol* symbol = head_node->down;
  while(symbol!=NULL){
    unsigned int index = symbol->index;
    hash_table->hash_table[index]=symbol->right;
    struct Symbol *temp = symbol->down;
    free(symbol);
    symbol = temp;
  }
  // pop the last stack_node
  struct Stack_node *temp = head_node->left;
  free(head_node);
  hash_table->stack_head = temp;
}

void insert_symbol(struct Hash_table* hash_table, char* name, int kind, Type type, struct Symbol_function* function, int lineno){
  // check collision
  if(kind==FUNC){
    struct Symbol* temp = find_symbol(hash_table, name, kind);
    if((temp!=NULL)&&(temp->depth==current_depth(hash_table))){
      print_error(4, lineno);
      return;
    }
  }else if(kind==VARIABLE){
    struct Symbol* temp = find_symbol(hash_table, name, VARIABLE);
    if((temp!=NULL)&&(temp->depth==current_depth(hash_table))){
      print_error(3, lineno);
      return;
    }
    temp = find_symbol(hash_table, name, STRUCT);
    if((temp!=NULL)&&(temp->depth==current_depth(hash_table))){
      print_error(3, lineno);
      return;
    }
  }else if(kind==STRUCT){
    struct Symbol* temp = find_symbol(hash_table, name, VARIABLE);
    if((temp!=NULL)&&(temp->depth==current_depth(hash_table))){
      print_error(16, lineno);
      return;
    }
    temp = find_symbol(hash_table, name, STRUCT);
    if((temp!=NULL)&&(temp->depth==current_depth(hash_table))){
      print_error(16, lineno);
      return;
    }
  }


  // give value to new symbol
  struct Symbol* new_symbol = (struct Symbol*)malloc(sizeof(struct Symbol));
  new_symbol->depth = hash_table->stack_head->depth;
  new_symbol->kind = kind;
  if(kind != FUNC){
    new_symbol->type = type;
  } else {
    new_symbol->function = function;
  }
  new_symbol->name = name;
  new_symbol->down = hash_table->stack_head->down;
  hash_table->stack_head->down = new_symbol;
  // get the index of new symbol
  unsigned int index = get_hash(new_symbol->name);
  new_symbol->index = index;
  // insert the symbol
  if(hash_table->hash_table[index]==NULL){
    hash_table->hash_table[index] = new_symbol;
  }else{
    new_symbol->right=hash_table->hash_table[index];
    hash_table->hash_table[index]=new_symbol;
  }
}

struct Symbol* find_symbol(struct Hash_table* hash_table, char* name, int kind){
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

int current_depth(struct Hash_table* hash_table){
  return hash_table->stack_head->depth;
}