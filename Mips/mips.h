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

struct Reg{
  int hasReg;
  int isS;
  int index;
};

struct VarIndex_{
  Operand operand;
  int index;
  int isSpilled;
  struct Reg reg;
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

#endif //COMPILERSLAB_MIPS_H
