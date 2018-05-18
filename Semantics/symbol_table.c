#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "data_type.h"

int compare_argv_argv(struct argv* argv1, struct argv* argv2);
void free_symbol(struct Symbol* symbol);
void free_func(struct Symbol_function* func);
void free_argv(struct argv* argv1);
void free_fieldList(FieldList fieldList);
void free_type(Type type);

// this class contains all the services provided by hash_table

// the hash function recommended in the homework
unsigned int get_hash(char* name){
  unsigned int val = 0, i;
  for(; *name; ++name){
    val = (val << 2) + *name;
    if( i = val & ~hash_size) val = (val ^ (i >> 12)) & hash_size;
  }
  return val;
}

// create a hash table
struct Hash_table* create_table(){
  struct Hash_table* result = (struct Hash_table*)malloc(sizeof(struct Hash_table));
  memset(result, 0, sizeof(struct Hash_table));
  return result;
}

// free all the symbols and a stack in the table
void free_table(struct Hash_table* hash_table){
  // free the stack
  struct Stack_node* p = hash_table->stack_head;
  while(p!=NULL){
    free_symbol(p->down);
    struct Stack_node* temp = p;
    p = p->left;
    free(temp);
  }
  // free the table
  free(hash_table);

}

// free a symbol iteratively
void free_symbol(struct Symbol* symbol){
  if(symbol==NULL){
    // do nothing
  }else{
    if((symbol->kind==STRUCT)||(symbol->kind==VARIABLE)){
      free_type(symbol->type);
    }else if(symbol->kind==FUNC){
      free_func(symbol->function);
    }
    if(symbol->down!=NULL)
      free_symbol(symbol->down);
    free(symbol);
  }
}

// free a func iteratively
void free_func(struct Symbol_function* func){
  if(func==NULL){
    // do nothing
  }else{
    free_type(func->return_type);
    free_argv(func->argv1);
    free(func);
  }
}

// free the argv list iteratively
void free_argv(struct argv* argv1) {
  if (argv1 == NULL) {
    // do nothing
  } else {
    free_argv(argv1->next);
    free_type(argv1->type);
    free(argv1);
  }
}

// free the fieldList iteratively
void free_fieldList(FieldList fieldList) {
  if (fieldList == NULL) {
    // do nothing
  } else {
    free_type(fieldList->type);
    free_fieldList(fieldList->tail);
    free(fieldList);
  }
}

// free the type
void free_type(Type type) {
  if (type == NULL) {
    // do nothing
  } else {
    if (type->kind == BASIC) {
      // do nothing
    } else if (type->kind == ARRAY) {
      free_type(type->u.array.elem);
    } else { // structure
      free_fieldList(type->u.structure);
    }
    free(type);
  }
}

// push_env makes a new environment, such as for statements in a {}
void push_env(struct Hash_table* hash_table) {
  struct Stack_node *new_stack_node = (struct Stack_node *) malloc(sizeof(struct Stack_node));
  memset(new_stack_node, 0, sizeof(struct Stack_node));
  if (hash_table->stack_head == NULL) {
    new_stack_node->depth = 0;
    new_stack_node->left = NULL;
    hash_table->stack_head = new_stack_node;
  } else {
    new_stack_node->left = hash_table->stack_head;
    new_stack_node->depth = ((hash_table->stack_head->depth) + 1);
    hash_table->stack_head = new_stack_node;
  }
}

// pop_env deletes the symbols contained in the current environment
void pop_env(struct Hash_table* hash_table) {
  struct Stack_node *head_node = hash_table->stack_head;
  // delete symbols
  struct Symbol *symbol = head_node->down;
  while (symbol != NULL) {
    unsigned int index = symbol->index;
    hash_table->hash_table[index] = symbol->right;
    struct Symbol *temp = symbol;
    symbol = symbol->down;
    free(temp);
  }
  // pop the last stack_node
  hash_table->stack_head = head_node->left;
  free(head_node);
}

// compare an argv with another argv
int compare_argv_argv(struct argv* argv1, struct argv* argv2) {
  if (((argv1 == NULL) && (argv2 != NULL)) || ((argv1 != NULL) && (argv2 == NULL))) {
    return -1;
  } else if ((argv1 == NULL) && (argv2 == NULL)) {
    return 1;
  } else {
    if (compare_type_type(argv1->type, argv2->type) != 1) {
      return -1;
    } else {
      return compare_argv_argv(argv1->next, argv2->next);
    }
  }
}

// insert a symbol into hash table, and check whether this symbol already exists
void insert_symbol(struct Hash_table* hash_table, char* name, int kind, Type type, struct Symbol_function* function, int lineno) {
//  printf("I am searched! lineno: %d, name: %s, kind: %d\n", lineno, name, kind);
  // check collision
  if (kind == FUNC) {
    struct Symbol *temp = find_symbol(hash_table, name, kind);
//  printf("FUNC is searched! name: %s, kind: %d, isExist: %d\n", name, kind, (temp==NULL));
    if ((temp != NULL) && (temp->depth == current_depth(hash_table))) {
      if ((temp->kind != FUNC) ||
          ((temp->kind == FUNC) && (temp->function->hasCompSt == 1) && (function->hasCompSt == 1))) {
        print_error(4, name, lineno);
        return;
      } else {
        if (compare_type_type(temp->function->return_type, function->return_type) != 1) {
          print_error(19, name, lineno);
          return;
        } else if (temp->function->argc != function->argc) {
          print_error(19, name, lineno);
          return;
        } else if (compare_argv_argv(temp->function->argv1, function->argv1) != 1) {
          print_error(19, name, lineno);
          return;
        } else {
          if (temp->function->hasCompSt == 0) {
            temp->function->hasCompSt = function->hasCompSt;
          }
          return;
        }
      }
    }
  } else if (kind == VARIABLE) {
    struct Symbol *temp = find_symbol(hash_table, name, VARIABLE);
    if ((temp != NULL) && (temp->depth == current_depth(hash_table))) {
      print_error(3, name, lineno);
      return;
    }
    temp = find_symbol(hash_table, name, STRUCT);
    if ((temp != NULL) && (temp->depth == current_depth(hash_table))) {
      print_error(3, name, lineno);
      return;
    }
  } else if (kind == STRUCT) {
    struct Symbol *temp = find_symbol(hash_table, name, VARIABLE);
    if ((temp != NULL) && (temp->depth == current_depth(hash_table))) {
      print_error(16, name, lineno);
      return;
    }
    temp = find_symbol(hash_table, name, STRUCT);
    if ((temp != NULL) && (temp->depth == current_depth(hash_table))) {
      print_error(16, name, lineno);
      return;
    }
  }


  // give value to new symbol
  struct Symbol *new_symbol = (struct Symbol *) malloc(sizeof(struct Symbol));
  memset(new_symbol, 0, sizeof(struct Symbol));
  // get the index of new symbol
  unsigned int index = get_hash(name);
  new_symbol->index = index;
  new_symbol->kind = kind;
  new_symbol->name = name;
  if (kind != FUNC) {
    new_symbol->depth = hash_table->stack_head->depth;
    new_symbol->down = hash_table->stack_head->down;
    hash_table->stack_head->down = new_symbol;
    new_symbol->type = type;
    // insert the symbol
    new_symbol->right = hash_table->hash_table[index];
    hash_table->hash_table[index] = new_symbol;
  } else {
    new_symbol->depth = hash_table->stack_head->depth;
    struct Stack_node* stack_node = hash_table->stack_head;
    while(stack_node->left!=NULL){
      stack_node = stack_node->left;
    }
    new_symbol->down = stack_node->down;
    stack_node->down = new_symbol;
    new_symbol->function = function;
    // insert the symbol
    new_symbol->right = hash_table->hash_table[index];
    hash_table->hash_table[index] = new_symbol;
  }

}

// find a symbol in the hash table
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

// get the current depth of the codes
int current_depth(struct Hash_table* hash_table){
  return hash_table->stack_head->depth;
}