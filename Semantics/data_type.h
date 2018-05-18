#ifndef DATATYPE_H
#define DATATYPE_H

#include "../Lex/tree.h"

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;

struct Type_
{
  enum { BASIC=1, ARRAY=2, STRUCTURE=3 } kind;
  union
  {
    // basic type
    enum Type_TYPE{ INT=4, FLOAT=5 } basic;
    // array type
    struct { Type elem; int size; } array;
    FieldList structure;
  } u;
};

const static int hash_size = 0x3fff;

struct FieldList_
{
  char* name; // name of field
  Type type; // type of field
  FieldList tail; // next field
};

struct argv{
  Type type;
  struct argv* next;
};


struct Symbol_function{
  Type return_type;
  int argc;
  struct argv* argv1;
  int hasCompSt;
  int define_lineno;
};

struct Symbol{
  char* name;
  enum Symbol_TYPE{ UNKNOWN = 0, VARIABLE=6, FUNC=7, STRUCT=8 } kind;
  union {
    Type type;
    struct Symbol_function* function;
  };
  unsigned int index;
  int depth;
  struct Symbol* right;
  struct Symbol* down;
};

struct Stack_node{
  int depth;
  struct Stack_node* left;
  struct Symbol* down;
};

struct Hash_table{
  struct Stack_node* stack_head;
  struct Symbol* hash_table[hash_size];
};

struct Hash_table* create_table();
void free_table(struct Hash_table* hash_table);
void push_env(struct Hash_table* hash_table);
void pop_env(struct Hash_table* hash_table);
void insert_symbol(struct Hash_table* hash_table, char* name, int kind, Type type, struct Symbol_function* function, int lineno);
struct Symbol* find_symbol(struct Hash_table* hash_table, char* name, int kind);
int current_depth(struct Hash_table* hash_table);
void check_error(Node* tree_root);
void print_error(int error_type, int lineno);

int compare_type_type(Type type_a, Type type_b);

#endif