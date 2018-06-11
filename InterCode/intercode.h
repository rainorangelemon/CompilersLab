//
// Created by rainorangelemon on 6/7/18.
//
#include <stdio.h>
#include <string.h>
#include "../Semantics/data_type.h"

#ifndef INTER_CODE_H
#define INTER_CODE_H

typedef struct Operand_* Operand;
typedef struct InterCode_* InterCode;
typedef struct InterCodes_* InterCodes;
typedef struct Arg_list_* Arg_list;

struct InterCodes_{
  InterCode code;
  InterCodes prev;
  InterCodes next;
};

struct Operand_{
  enum{
    VARIABLE,
    CONSTANT,
    LABEL,
    RELOP,
    MATHOP
  } kind;
  union{
    int value;
    char* name;
  }u;
};

struct Arg_list_{
  char* arg;
  Arg_list next;
};

struct InterCode_{
  enum{LABEL, FUNCTION, ASSIGN, MATHOP,
       RIGHT_ADDR, RIGHT_STAR, LEFT_STAR, GOTO, RELOP, RETURN, DEC, ARG,
       CALL, PARAM, READ, WRITE} kind;
  union{
    struct {Operand left, right;} assign;
    struct {Operand result, op1, mathop, op2;} binop;
    struct {Operand left, relop, right, label;} goto_con;
    struct {Operand label;} label;
  }u;
};

void insert_symbol_intercodes(struct Hash_table* hash_table, char* name, int kind, Type type, struct Symbol_function* function);
struct Symbol* find_symbol_intercodes(struct Hash_table* hash_table, char* name, int kind);
#endif
