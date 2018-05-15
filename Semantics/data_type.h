#ifndef DATA_TYPE_H
#define DATA_TYPE_H

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;

struct Type_
{
  enum { BASIC, ARRAY, STRUCTURE } kind;
  union
  {
    // basic type
    int basic;
    // array type
    struct { Type elem; int size; } array;
    FieldList structure;
  } u;
};

constant int hash_size = 0x3fff;

struct FieldList_
{
  char* name; // name of field
  Type type; // type of field
  FieldList tail; // next field
};

struct Symbol_function{
  Type return_type;
  int argc;
  struct argv{
    Type type;
    argv* next;
  }argv1;
};

struct Symbol{
  char* name;
  union {
    Type type=NULL;
    Symbol_function* function=NULL;
  };
  unsigned int index;
  int depth = 0;
  Symbol* right=NULL;
  Symbol* down=NULL;
};

struct Stack_node{
  int depth = 0;
  Stack_node* left=NULL;
  Symbol* down=NULL;
};

struct Hash_table{
  Stack_node* stack_head;
  Symbol* hash_table[hash_size];
};

Hash_table* create_table();
void push_env(Hash_table* hash_table);
void pop_env(Hash_table* hash_table);
void insert_symbol(Hash_table* hash_table, char* name, Type type, Symbol_fuction* function);
Symbol* find_variable(Hash_table* hash_table, char* name);
Symbol* find_function(Hash_table* hash_table, char* name);
int current_depth(Hash_table* hash_table);

#endif