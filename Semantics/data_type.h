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

const int hash_size = 0x3fff;

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
    struct argv* next;
  }argv1;
};

struct Symbol{
  char* name;
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
void push_env(struct Hash_table* hash_table);
void pop_env(struct Hash_table* hash_table);
void insert_symbol(struct Hash_table* hash_table, char* name, Type type, struct Symbol_function* function);
struct Symbol* find_variable(struct Hash_table* hash_table, char* name);
struct Symbol* find_function(struct Hash_table* hash_table, char* name);
int current_depth(struct Hash_table* hash_table);

void check_error(Node* tree_root);

#endif