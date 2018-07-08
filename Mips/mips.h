//
// Created by rainorangelemon on 7/5/18.
//

#include <stdio.h>
#include <string.h>
#include "../InterCode/intercode.h"

#ifndef COMPILERSLAB_MIPS_H
#define COMPILERSLAB_MIPS_H

typedef struct VarIndex_* VarIndex;
typedef struct VarIndexes_* VarIndexes;
typedef struct Bit_* Bit;
typedef struct Bits_* Bits;
typedef struct Basic_* Basic;
typedef struct RegStatus_* RegStatus;

struct Reg{
  int hasReg;
  int isS;
  int index;
};

struct VarAddr{
  int hasAddr;
  int fpIndex;
  int off2Fp;
  int size;
  int newest;
};

struct VarIndex_{
  Operand operand;
  int index;
  int isSpilled;
  struct Reg reg;
  struct VarAddr decAddr, spiltAddr;
};

struct VarIndexes_{
  VarIndex varIndex;
  VarIndexes next;
};

struct Succ{
  int succ1;
  int succ2;
};

struct Basic_{
  int* in;
  int* out;
  int* def;
  int* use;
  struct Succ* succ;
};

struct RegStatus_{
  int isS;
  int index;
  int variables;
};

#endif //COMPILERSLAB_MIPS_H
