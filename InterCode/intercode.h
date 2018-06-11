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
    LABEL=9,
    VARIABLE=6,
    CONSTANT=10,
    MATHOP=11,
    ADDRESS=12,
    RELOP=13,
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
  enum{LABEL=9, FUNCTION=7, ASSIGN=14, MATHOP=11,
       RIGHT_ADDR=15, RIGHT_STAR=16, LEFT_STAR=17, GOTO=18, RELOP=13, RETURN=19, DEC=20, ARG=21,
       CALL=22, PARAM=23, READ=24, WRITE=25, EMPTY=26} kind;
  union{
    struct {Operand left, right;} assign;
    struct {Operand result, op1, mathop, op2;} binop;
    struct {Operand left, relop, right, label;} goto_con;
    struct {Operand label;} label;
  }u;
};

void insert_symbol_intercodes(struct Hash_table* hash_table, int isAddress, char* name, int kind, Type type, struct Symbol_function* function);
struct Symbol* find_symbol_intercodes(struct Hash_table* hash_table, char* name, int kind);
char* printCodes(InterCode interCode);
void optimize_InterCodes(InterCodes interCodes);
#endif
