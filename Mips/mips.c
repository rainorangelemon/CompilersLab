//
// Created by rainorangelemon on 7/5/18.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mips.h"

void print_codes_status(int* in, int* out, int* def, int* use, struct Succ* succ, int numInterCodes, InterCodes head);
void getVariables(VarIndexes varIndexes, InterCodes head);
Basic calculateBasic(InterCodes head, VarIndexes varIndexes);
void print_binary(int bits);
void allocateReg(InterCodes head, Basic basic, VarIndexes varIndexes);
void generateObj(InterCodes head, VarIndexes varIndexes);

const int numS = 6;
const int numT = 10;
const int numSpiltReg = 2;
const int numReg = 10 + 8 - numSpiltReg; // save a reg s7 for spilled var
FILE * fp=NULL;

extern char* sFile;

void createMips(InterCodes head){

  if(sFile!=NULL) {
    fp = fopen(sFile, "w+");
  }

  VarIndexes varIndexes = (VarIndexes)malloc(sizeof(struct VarIndexes_));
  memset(varIndexes, 0, sizeof(struct VarIndexes_));
  getVariables(varIndexes, head);
  Basic basic = calculateBasic(head, varIndexes);
  allocateReg(head, basic, varIndexes);
  generateObj(head, varIndexes);

  if(sFile!=NULL) {
    fclose(fp);
  }

}

VarIndexes find_varIndexes(VarIndexes varIndexes, Operand operand){
  VarIndexes temp = varIndexes;
  while(temp!=NULL){
    if(temp->varIndex!=NULL) {
      if (compare_operand_with_operand(temp->varIndex->operand, operand)==1){
        return temp;
      }
    }
    temp = temp->next;
  }
  return NULL;
}

int find_varIndex(VarIndexes varIndexes, Operand operand){
  VarIndexes temp = varIndexes;
  while(temp!=NULL){
    if(temp->varIndex!=NULL) {
      if (compare_operand_with_operand(temp->varIndex->operand, operand)==1){
        return temp->varIndex->index;
      }
    }
    temp = temp->next;
  }
  if((operand->kind==CONSTANT)&&(operand->u.value==0)) {
    return 0;
  }else{
    return 32;
  }
}

void addVariableIndex(VarIndexes varIndexes, Operand operand){
  if(find_varIndex(varIndexes, operand)>31){
    int index = 0;
    VarIndexes temp = varIndexes;
    while(temp->varIndex!=NULL){
      index = temp->varIndex->index;
      if(temp->next!=NULL) {
        temp = temp->next;
      }else{
        break;
      }
    }
    index += 1;
    VarIndex newIndex = (VarIndex)malloc(sizeof(struct VarIndex_));
    memset(newIndex, 0, sizeof(struct VarIndex_));
    newIndex->index = index;
    newIndex->operand = operand;
    if(temp->varIndex==NULL){
      temp->varIndex = newIndex;
    }else{
      VarIndexes newVarIndexes = (VarIndexes)malloc(sizeof(struct VarIndexes_));
      memset(newVarIndexes, 0, sizeof(struct VarIndexes_));
      newVarIndexes->varIndex = newIndex;
      newVarIndexes->next = NULL;
      temp->next = newVarIndexes;
    }
  }else{
    // do nothing
  }
}

void addConstantIndex(VarIndexes varIndexes, Operand operand){
  if(operand->kind==CONSTANT){
    addVariableIndex(varIndexes, operand);
  }
}

void getVariables(VarIndexes varIndexes, InterCodes head){
  InterCodes temp = head;
  while(temp!=NULL) {
    InterCode interCode = temp->code;
    if (interCode->kind == LABEL) {
      // do nothing
    } else if (interCode->kind == FUNCTION) {
      // do nothing
    } else if (interCode->kind == ASSIGN) {
      addVariableIndex(varIndexes, interCode->u.assign.left);
      if(interCode->u.assign.right->kind!=CONSTANT) {
        addVariableIndex(varIndexes, interCode->u.assign.right);
      }
    } else if (interCode->kind == MATHOP) {
      addVariableIndex(varIndexes, interCode->u.binop.result);
      if(((*(interCode->u.binop.mathop->u.name))=='*')||
         ((*(interCode->u.binop.mathop->u.name))=='/')||
         (interCode->u.binop.op1->kind!=CONSTANT)) {
        addVariableIndex(varIndexes, interCode->u.binop.op1);
      }
      if(((*(interCode->u.binop.mathop->u.name))=='*')||
         ((*(interCode->u.binop.mathop->u.name))=='/')||
         (interCode->u.binop.op2->kind!=CONSTANT)) {
        addVariableIndex(varIndexes, interCode->u.binop.op2);
      }
    } else if (interCode->kind == RIGHT_ADDR) {
      addVariableIndex(varIndexes, interCode->u.assign.left);
      addVariableIndex(varIndexes, interCode->u.assign.right);
    } else if (interCode->kind == RIGHT_STAR) {
      addVariableIndex(varIndexes, interCode->u.assign.left);
      addVariableIndex(varIndexes, interCode->u.assign.right);
    } else if (interCode->kind == LEFT_STAR) {
      addVariableIndex(varIndexes, interCode->u.assign.left);
      addVariableIndex(varIndexes, interCode->u.assign.right);
    } else if (interCode->kind == GOTO) {
      // do nothing
    } else if (interCode->kind == RELOP) {
      addVariableIndex(varIndexes, interCode->u.goto_con.left);
      addVariableIndex(varIndexes, interCode->u.goto_con.right);
    } else if (interCode->kind == RETURN) {
      addVariableIndex(varIndexes, interCode->u.label.label);
    } else if (interCode->kind == DEC) {
      // do nothing
    } else if (interCode->kind == ARG) {
      addVariableIndex(varIndexes, interCode->u.label.label);
    } else if (interCode->kind == CALL) {
      addVariableIndex(varIndexes, interCode->u.assign.left);
    } else if (interCode->kind == PARAM) {
      // do nothing
    } else if (interCode->kind == READ) {
      addVariableIndex(varIndexes, interCode->u.label.label);
    } else if (interCode->kind == WRITE) {
      addVariableIndex(varIndexes, interCode->u.label.label);
    } else {
      // do nothing
    }
    temp = temp->next;
  }

//  for debug
//  VarIndexes varIndexes1 = varIndexes;
//  while(varIndexes1!=NULL){
//    printf("%d: %s\n", varIndexes1->varIndex->index, print_operand(varIndexes1->varIndex->operand, 1));
//    varIndexes1 = varIndexes1->next;
//  }
}

int find_label_index(InterCodes head, Operand label){
  InterCodes temp = head;
  int numIndex = 0;
  while(temp!=NULL){
    numIndex += 1;
    if(temp->code->kind==LABEL){
      if(compare_operand_with_operand(temp->code->u.label.label, label)==1){
        return numIndex;
      }
    }
    temp = temp->next;
  }
  return -1;
}

void addDef(VarIndexes varIndexes, Operand operand, int* def){
  if(operand->kind==CONSTANT){
//    *def  = *def | (1 << find_varIndex(varIndexes, operand));
  }
}

Basic calculateBasic(InterCodes head, VarIndexes varIndexes){
  int numInterCodes=1;
  InterCodes temp = head;
  while(temp!=NULL){
    numInterCodes += 1;
    temp = temp->next;
  }

  // control flow

  int* in = (int*)malloc(sizeof(int)*numInterCodes);
  int* out = (int*)malloc(sizeof(int)*numInterCodes);
  int* def = (int*)malloc(sizeof(int)*numInterCodes);
  int* use = (int*)malloc(sizeof(int)*numInterCodes);
  struct Succ* succ = (struct Succ*)malloc(sizeof(struct Succ)*numInterCodes);
  memset(in, 0, sizeof(int)*numInterCodes);
  memset(out, 0, sizeof(int)*numInterCodes);
  memset(def, 0, sizeof(int)*numInterCodes);
  memset(use, 0, sizeof(int)*numInterCodes);
  memset(succ, 0, sizeof(struct Succ)*numInterCodes);

  int codeIndex = 0;
  temp = head;

  // define succ, def, use
  while(temp!=NULL){
    codeIndex += 1;

    // define succ
    struct Succ* tempSucc = succ+codeIndex;
    InterCode interCode = temp->code;
    if (interCode->kind == GOTO) {
      tempSucc->succ2 = find_label_index(head, interCode->u.label.label);
    } else if (interCode->kind == RELOP) {
      tempSucc->succ1 = codeIndex+1;
      tempSucc->succ2 = find_label_index(head, interCode->u.goto_con.label);
    } else if (interCode->kind == RETURN) {
      tempSucc->succ1 = 0;
      tempSucc->succ2 = 0;
    } else if ((interCode->kind != EMPTY)&&(codeIndex<(numInterCodes-1))){
      tempSucc->succ1 = codeIndex+1;
      tempSucc->succ2 = 0;
    } else {
      tempSucc->succ1 = 0;
      tempSucc->succ2 = 0;
    }

    // define use and def
    int* tempUse = use+codeIndex;
    int* tempDef = def+codeIndex;
    if (interCode->kind == LABEL) {
      // do nothing
    } else if (interCode->kind == FUNCTION) {
      // do nothing
    } else if (interCode->kind == ASSIGN) {
      // def
      *tempDef = *tempDef | (1 << find_varIndex(varIndexes, interCode->u.assign.left));
      // use
      if(interCode->u.assign.right->kind!=CONSTANT) {
        *tempUse = *tempUse | (1 << find_varIndex(varIndexes, interCode->u.assign.right));
        addDef(varIndexes, interCode->u.assign.right, tempDef);
      }
    } else if (interCode->kind == MATHOP) {
      // def
      *tempDef = *tempDef | (1 << find_varIndex(varIndexes, interCode->u.binop.result));
      // use
      if(((*(interCode->u.binop.mathop->u.name))=='*')||
         ((*(interCode->u.binop.mathop->u.name))=='/')||
         (interCode->u.binop.op1->kind!=CONSTANT)) {
        *tempUse = *tempUse | (1 << find_varIndex(varIndexes, interCode->u.binop.op1));
      }
      if(((*(interCode->u.binop.mathop->u.name))=='*')||
         ((*(interCode->u.binop.mathop->u.name))=='/')||
         (interCode->u.binop.op2->kind!=CONSTANT)) {
        *tempUse = *tempUse | (1 << find_varIndex(varIndexes, interCode->u.binop.op2));
      }
      addDef(varIndexes, interCode->u.binop.op1, tempDef);
      addDef(varIndexes, interCode->u.binop.op2, tempDef);
    } else if (interCode->kind == RIGHT_ADDR) {
      // def
      *tempDef = *tempDef | (1 << find_varIndex(varIndexes, interCode->u.assign.left));
      // use
      *tempUse = *tempUse | (1 << find_varIndex(varIndexes, interCode->u.assign.right));
    } else if (interCode->kind == RIGHT_STAR) {
      // def
      *tempDef = *tempDef | (1 << find_varIndex(varIndexes, interCode->u.assign.left));
      // use
      *tempUse = *tempUse | (1 << find_varIndex(varIndexes, interCode->u.assign.right));
    } else if (interCode->kind == LEFT_STAR) {
      // no def
      // use
      *tempUse = *tempUse | (1 << find_varIndex(varIndexes, interCode->u.assign.left));
      *tempUse = *tempUse | (1 << find_varIndex(varIndexes, interCode->u.assign.right));
      addDef(varIndexes, interCode->u.assign.right, tempDef);
    } else if (interCode->kind == GOTO) {
      // do nothing
    } else if (interCode->kind == RELOP) {
      // no def
      // use
      *tempUse = *tempUse | (1 << find_varIndex(varIndexes, interCode->u.goto_con.left));
      *tempUse = *tempUse | (1 << find_varIndex(varIndexes, interCode->u.goto_con.right));
      addDef(varIndexes, interCode->u.goto_con.left, tempDef);
      addDef(varIndexes, interCode->u.goto_con.right, tempDef);
    } else if (interCode->kind == RETURN) {
      *tempUse = *tempUse | (1 << find_varIndex(varIndexes, interCode->u.label.label));
      addDef(varIndexes, interCode->u.label.label, tempDef);
    } else if (interCode->kind == DEC) {
      // do nothing
    } else if (interCode->kind == ARG) {
      addDef(varIndexes, interCode->u.label.label, tempDef);
    } else if (interCode->kind == CALL) {
      // def
      *tempDef = *tempDef | (1 << find_varIndex(varIndexes, interCode->u.assign.left));
    } else if (interCode->kind == PARAM) {
      *tempDef = *tempDef | (1 << find_varIndex(varIndexes, interCode->u.label.label));
    } else if (interCode->kind == READ) {
      // def
      *tempDef = *tempDef | (1 << find_varIndex(varIndexes, interCode->u.label.label));
    } else if (interCode->kind == WRITE) {
      // use
      *tempUse = *tempUse | (1 << find_varIndex(varIndexes, interCode->u.label.label));
      addDef(varIndexes, interCode->u.label.label, tempDef);
    } else {
      // do nothing
    }

    // go to next instr
    temp = temp->next;
  }


  // define in and out
  int different = 1;
  int flag = 0;
  while((different!=0)||(flag==0)){
    if(different==0){
      flag = 1;
    }
    different = 0;
    for(int i =1; i < numInterCodes; i++){
      // save previous in
      int oldIn = *(in + i);
      // calculate out
      int currentSucc1 = (*(succ +i)).succ1;
      int currentSucc2 = (*(succ +i)).succ2;
      *(out + i) = (*(in + currentSucc1)) | (*(in + currentSucc2));
      // calculate in
      *(in + i) = (*(use + i)) | ((*(out + i)) & (~(*(def + i))));
      // calculate different
      different |= (oldIn & (~(*(in + i)))) | ((~oldIn) & (*(in + i)));
    }
  }

//  print_codes_status(in, out, def, use, succ, numInterCodes, head);

  Basic result = (Basic)malloc(sizeof(struct Basic_));
  result->def = def;
  result->in = in;
  result->out = out;
  result->succ = succ;
  result->use = use;
  return result;
}

void print_codes_status(int* in, int* out, int* def, int* use, struct Succ* succ, int numInterCodes, InterCodes head){
  int index = 0;
  InterCodes temp = head;
  while(temp!=NULL) {
    index += 1;

    printf("index: %d,\t", index);
    print_binary(*(in+index));
    print_binary(*(out+index));
    print_binary(*(def+index));
    print_binary(*(use+index));
    printf("succ1: %d, ", (*(succ+index)).succ1);
    printf("succ2: %d, ", (*(succ+index)).succ2);
    printf("code: %s\n", printCodes(temp->code));
    temp = temp->next;
  }
}

void print_binary(int bits){
  for(int i=31; i>=0; i--) {
    if(((bits>>i)&0x1)==1) {
      printf("1");
    }else{
      printf("0");
    }
  }
  printf(", ");
}

int count_one(int bits){
  int ones = 0;
  for(int i=31; i>=1; i--) {
    if(((bits>>i)&0x1)==1) {
      ones += 1;
    }
  }
  return ones;
}

void color(VarIndexes varIndexes, int varIndex, char regName, int regIndex){
  VarIndexes temp = varIndexes;
  while(temp!=NULL){
    if(temp->varIndex->index == varIndex){
      if(regName == '0') {
        temp->varIndex->isSpilled = 1;
        temp->varIndex->reg.hasReg = 0;
      }else {
        temp->varIndex->isSpilled = 0;
        temp->varIndex->reg.hasReg = 1;
        temp->varIndex->reg.index = regIndex;
        if (regName == 's') {
          temp->varIndex->reg.isS = 1;
        } else {
          temp->varIndex->reg.isS = 0;
        }
      }
      break;
    }
    temp = temp->next;
  }
}

int auto_color(VarIndexes varIndexes, int varIndex, int done, int neighbours, char regName){
  int doneNeighbour = (neighbours & done);
  if((regName=='a')||(regName=='t')){
    int t = 0;
    VarIndexes temp = varIndexes;
    while(temp!=NULL){
      if(((1 << (temp->varIndex->index))&doneNeighbour)!=0){
        if((temp->varIndex->reg.hasReg==1)&&(temp->varIndex->reg.isS==0)){
          t |= (1<<(temp->varIndex->reg.index));
        }
      }
      temp = temp->next;
    }
    int remain = ((1<<numT)-1) & (~t);
    if(remain!=0){
      for(int i=0; i<numT; i++){
        if(((remain>>i)&0x1)!=0){
          color(varIndexes, varIndex, 't', i);
          return 0;
        }
      }
    }
  }
  if((regName=='a')||(regName=='s')){
    int s = 0;
    VarIndexes temp = varIndexes;
    while(temp!=NULL){
      if(((1 << (temp->varIndex->index))&doneNeighbour)!=0){
        if((temp->varIndex->reg.hasReg==1)&&(temp->varIndex->reg.isS==1)){
          s |= (1<<(temp->varIndex->reg.index));
        }
      }
      temp = temp->next;
    }
    int remain = ((1<<numS)-1) & (~s);
    if(remain!=0){
      for(int i=0; i<numS; i++){
        if(((remain>>i)&0x1)!=0){
          color(varIndexes, varIndex, 's', i);
          return 0;
        }
      }
    }
  }
  color(varIndexes, varIndex, '0', -1);
  return 1;
}

void print_color(VarIndexes varIndexes){
  VarIndexes temp = varIndexes;
  while(temp!=NULL){
    if(temp->varIndex->isSpilled==1) {
      printf("%d: spilled\n", temp->varIndex->index);
    }else{
      printf("%d: isS: %d, index: %d\n", temp->varIndex->index, temp->varIndex->reg.isS, temp->varIndex->reg.index);
    }
    temp = temp->next;
  }
}

void allocateReg(InterCodes head, Basic basic, VarIndexes varIndexes){
  // 数变量个数
  int varNum=0;
  VarIndexes tempVarIndexes = varIndexes;
  while(tempVarIndexes!=NULL){
    varNum += 1;
    tempVarIndexes = tempVarIndexes->next;
  }

  // 数中间代码个数
  int varCodes=0;
  InterCodes interCodes = head;
  while(interCodes!=NULL){
    varCodes += 1;
    interCodes = interCodes->next;
  }

  int* out = basic->out;
  int* in = basic->in;
  int* def = basic->def;

  // 建立干涉图
  int* inferG = (int*)malloc(sizeof(int)*(varNum+1));
  memset(inferG, 0, sizeof(int)*(varNum+1));


  interCodes = head;
  for(int codeIndex=1; codeIndex<=varCodes; codeIndex++){
    for(int x=1; x<=varNum; x++){
      for(int y=1; y<=varNum; y++) {
        if(y!=x){
          if((((*(out+codeIndex))&(1<<x))!=0)&&(((*(out+codeIndex))&(1<<y))!=0)){
            *(inferG + x) = (*(inferG+x)) | (1<<y);
            *(inferG + y) = (*(inferG+y)) | (1<<x);
          }else if((interCodes->code->kind!=ASSIGN)||(interCodes->code->u.assign.right->kind==ADDRESS)){
            if((((*(def+codeIndex))&(1<<x))!=0)&&(((*(out+codeIndex))&(1<<y))!=0)){
              *(inferG + x) = (*(inferG+x)) | (1<<y);
              *(inferG + y) = (*(inferG+y)) | (1<<x);
            }
          }
        }
      }
    }
    interCodes = interCodes->next;
  }

  // 记录哪些变量在 CALL 语句时有效
  interCodes = head;
  int callLive = 0;
  int codeIndex=0;
  while(interCodes!=NULL){
    codeIndex += 1;
    if((interCodes->code->kind==CALL)||(interCodes->code->kind==READ)||(interCodes->code->kind==WRITE)){
      int inVar = *(in + codeIndex);
      int outVar = *(out + codeIndex);
      callLive |= (inVar & outVar);
    }
    interCodes = interCodes->next;
  }


  // 染色
  int oldStack = -1;
  int stack = 0;
  while(oldStack!=stack){
    oldStack = stack;
    for(int x=1; x<=varNum; x++) {
      if((((oldStack>>x)&(0x1))==0)&&(count_one((*(inferG+x))&(~oldStack))<=(numReg-1))){ // save a reg for spilled var
        stack |= (1<<x);
      }
    }
  }
  // 检查剩下来的顶点
  int remainVer = 0;
  int spilt = 0;
  for(int x=1; x<=varNum; x++){
    if(((stack>>x)&(0x1))==0){
      remainVer += 1;
    }
    if(remainVer > (numReg - 1)){
      stack |= (1<<x);
      spilt |= (1<<x);
    }
  }

  // 先给小于k个节点染色，done意味着已完成染色的节点
  int done = 0;
  // 优先给干涉图中在call处活跃的节点染色
  int sNum = 0;
  for(int x=1; x<=varNum; x++){
    if(((stack>>x)&(0x1))==0){
      if(((callLive>>x)&(0x1))==1){
        if(sNum<numS) {
          color(varIndexes, x, 's', sNum);
          sNum += 1;
          done |= (1<<x);
        }else{
          stack |= (1<<x);
          spilt |= (1<<x);
        }
      }
    }
  }
  //给干涉图中不在call处活跃的节点染色
  for(int x=1; x<=varNum; x++){
    if((((stack>>x)&(0x1))==0)&&(((done>>x)&(0x1))==0)){
      auto_color(varIndexes, x, done, (*(inferG+x)), 'a');
      done |= (1<<x);
    }
  }

  // 给stack中非溢出节点染色
  for(int x=1; x<=varNum; x++){
    if((((stack>>x)&(0x1))==1)&&(((done>>x)&(0x1))==0)&&(((spilt>>x)&(0x1))==0)){
      int spill = 0;
      if(((callLive>>x)&(0x1))==1){
        spill = auto_color(varIndexes, x, done, (*(inferG + x)), 's');
      }else{
        spill = auto_color(varIndexes, x, done, (*(inferG + x)), 'a');
      }
      done |= (1<<x);
      stack &= (~(1<<x));
      if(spill==1){
        spilt |= (1<<x);
      }
    }
  }


  // 给stack中剩余节点染色
  while((stack&(~(0x1)))!=0) {
    for (int x = 1; x <= varNum; x++) {
      if ((((stack >> x) & (0x1)) == 1) && (((done >> x) & (0x1)) == 0)) {
        int spill = 0;
        if(((callLive>>x)&(0x1))==1) {
          spill = auto_color(varIndexes, x, done, (*(inferG + x)), 's');
        }else{
          spill = auto_color(varIndexes, x, done, (*(inferG + x)), 'a');
        }
        done |= 1<<x;
        stack &= (~(1<<x));
        if(spill==1){
          spilt |= 1<<x;
        }
      }
    }
  }

  // 尝试输出节点 for debug only
//  print_color(varIndexes);
}

void printObj(char* obj){
  if(sFile!=NULL){
    fprintf(fp, "%s\n", obj);
  }
}

int offsetFp(Operand operand, VarIndexes varIndexes, int isDec){
  VarIndexes temp = varIndexes;
  while(temp!=NULL){
    if(compare_operand_with_operand(operand, temp->varIndex->operand)==1){
      if(isDec==1) {
        return temp->varIndex->decAddr.off2Fp;
      }else if((temp->varIndex->isSpilled==1)&&(temp->varIndex->spiltAddr.hasAddr==1)){
        return temp->varIndex->spiltAddr.off2Fp;
      }
      break;
    }
    temp = temp->next;
  }
  return -1;
}

void saveDecAddr(RegStatus regStatus, Operand operand, VarIndexes varIndexes, int fpIndex, int size, int* fpSize){
  *fpSize = *fpSize + size;
  VarIndexes temp = find_varIndexes(varIndexes, operand);
  if(temp!=NULL){
    temp->varIndex->decAddr.hasAddr = 1;
    temp->varIndex->decAddr.off2Fp = -(*fpSize);
    temp->varIndex->decAddr.size = size;
    temp->varIndex->decAddr.fpIndex = fpIndex;
    temp->varIndex->decAddr.newest = 1;
  }
}

void saveParamAddr(RegStatus regStatus, Operand operand, VarIndexes varIndexes, int fpIndex, int offset){
  VarIndexes temp = find_varIndexes(varIndexes, operand);
  if(temp!=NULL){
    temp->varIndex->decAddr.hasAddr = 1;
    temp->varIndex->decAddr.off2Fp = offset;
    temp->varIndex->decAddr.size = 4;
    temp->varIndex->decAddr.fpIndex = fpIndex;
    temp->varIndex->decAddr.newest = 1;
  }
}

void freeAllReg(RegStatus regStatus){
  for(int i=0; i<numReg+numSpiltReg;i++){
    (regStatus + i)->variables = 0;
  }
}

void freeSplitReg(RegStatus regStatus, char* regName){
  if(regName==NULL) {
    for (int i = 0; i < numSpiltReg; i++) {
      (regStatus + numReg + i)->variables = 0;
    }
  }else{
    for (int i=numS; i<numS+numSpiltReg; i++){
      char* currentName=(char*)malloc(50*sizeof(char));
      memset(currentName, 0, 50*sizeof(char));
      sprintf(currentName, "s%d", i);
      if(strcmp(regName, currentName)==0){
        (regStatus + numReg + i - numS)->variables = 0;
        free(currentName);
        return;
      }
      free(currentName);
    }
  }
}

void saveSpiltVar(int* fpSize, RegStatus regStatus, Operand operand, char* regName, VarIndexes varIndexes, int fpIndex){
  VarIndexes temp = find_varIndexes(varIndexes, operand);
  if(temp==NULL){
    return;
  }else if(temp->varIndex->isSpilled==0) {      // check whether regName is spiltReg
    return;
  }else {
    int offset = offsetFp(operand, varIndexes, 0);
    if (offset != -1) {
      char *code = (char *) malloc(50 * sizeof(char));
      memset(code, 0, 50 * sizeof(char));
      sprintf(code, "sw %s, %d($fp)", regName, offset);
      printObj(code);
    } else {
      *fpSize = *fpSize + 4;
      offset = -(*fpSize);
      saveParamAddr(regStatus, operand, varIndexes, fpIndex, offset);
      char *code = (char *) malloc(50 * sizeof(char));
      memset(code, 0, 50 * sizeof(char));
      sprintf(code, "sw %s, %d($fp)\nsubu $sp, $sp, 4", regName, offset);
      printObj(code);
    }
    freeSplitReg(regStatus, regName);
  }
}

// 所有getReg的调用者必须看情况使用cleanReg
char* getReg(RegStatus regStatus, Operand operand, VarIndexes varIndexes){
  char* regName=(char*)malloc(50*sizeof(char));
  memset(regName, 0, 50*sizeof(char));
  // v0
  if((operand->kind==CONSTANT)&&(operand->u.value==0)){
    sprintf(regName, "$0");
    return regName;
  }else{
    VarIndexes temp = find_varIndexes(varIndexes, operand);
    if(temp==NULL){
      return NULL;
    }else if(temp->varIndex->isSpilled==1){

      // find spilt reg
      int spiltRegIndex = numS+numT;
      for(int i = numS+numT; i<numT+numS+numSpiltReg; i++){
        if((regStatus+i)->variables==0){
          spiltRegIndex = i;
          break;
        }
      }
      sprintf(regName, "$s%d", spiltRegIndex);
      (regStatus+spiltRegIndex)->variables |= (1<<(temp->varIndex->index));

      // if the reg's value exist, load it
      if(operand->kind!=CONSTANT){
        int offset = offsetFp(operand, varIndexes, 0);
        if(offset!=-1) {
          char *code = (char *) malloc(50 * sizeof(char));
          memset(code, 0, 50 * sizeof(char));
          sprintf(code, "lw %s, %d($fp)", regName, offset);
          printObj(code);
        }
      }else{
        char *code = (char *) malloc(50 * sizeof(char));
        memset(code, 0, 50 * sizeof(char));
        sprintf(code, "li %s, %d", regName, operand->u.value);
        printObj(code);
      }
      return regName;

    }else {
      // get the reg name
      if(temp->varIndex->reg.isS==1){
        sprintf(regName, "$s%d", temp->varIndex->reg.index);

        if(((((regStatus+temp->varIndex->reg.index+numT)->variables>>(temp->varIndex->index))&0x1)==0)
           &&(operand->kind==CONSTANT)){
          char *code = (char *) malloc(50 * sizeof(char));
          memset(code, 0, 50 * sizeof(char));
          sprintf(code, "li %s, %d", regName, operand->u.value);
          printObj(code);
        }

        (regStatus+temp->varIndex->reg.index+numT)->variables |= (1<<(temp->varIndex->index));
      }else{
        sprintf(regName, "$t%d", temp->varIndex->reg.index);

        if(((((regStatus+temp->varIndex->reg.index)->variables>>(temp->varIndex->index))&0x1)==0)
           &&(operand->kind==CONSTANT)){
          char *code = (char *) malloc(50 * sizeof(char));
          memset(code, 0, 50 * sizeof(char));
          sprintf(code, "li %s, %d", regName, operand->u.value);
          printObj(code);
        }

        (regStatus+temp->varIndex->reg.index)->variables |= (1<<(temp->varIndex->index));
      }
      return regName;
    }
  }
}

int count_s_regs(VarIndexes varIndexes){
  int result = 0;
  VarIndexes temp = varIndexes;
  while(temp!=NULL){
    if(temp->varIndex!=NULL) {
      if((temp->varIndex->isSpilled==0)&&(temp->varIndex->reg.isS==1)){
        result |= (1<<temp->varIndex->reg.index);
      }
    }
    temp = temp->next;
  }
  return result;
}

void print_begin_code(VarIndexes varIndexes){
  char* code=(char*)malloc(50*sizeof(char));
  memset(code, 0, 50*sizeof(char));
  sprintf(code, ".data");
  printObj(code);
  memset(code, 0, 50*sizeof(char));
  sprintf(code, "_prompt: .asciiz \"Can I have an integer: \"");
  printObj(code);
  memset(code, 0, 50*sizeof(char));
  sprintf(code, "_ret: .asciiz \"\\n\"\n.globl main\n.text");
  printObj(code);
  memset(code, 0, 50*sizeof(char));

  // read
  sprintf(code, "\nread:");
  printObj(code);
  memset(code, 0, 50*sizeof(char));

  sprintf(code, "subu $sp, $sp, %d", 8);
  printObj(code);
  memset(code, 0, 50*sizeof(char));
  sprintf(code, "sw $ra, %d($sp)\nsw $fp, %d($sp)", 4, 0);
  printObj(code);
  memset(code, 0, 50*sizeof(char));
  sprintf(code, "addi $fp, $sp, %d", 8);
  printObj(code);
  memset(code, 0, 50*sizeof(char));

  sprintf(code, "li $v0, 4\nla $a0, _prompt\nsyscall"); // 提醒用户输入
  printObj(code);
  memset(code, 0, 50*sizeof(char));
  sprintf(code, "li $v0, 5\nsyscall"); // 读入
  printObj(code);
  memset(code, 0, 50*sizeof(char));

  // 开始恢复寄存器
  sprintf(code, "move $sp, $fp");
  printObj(code);
  memset(code, 0, 50*sizeof(char));
  sprintf(code, "lw $ra, %d($sp)\nlw $fp, %d($sp)", -4, -8);
  printObj(code);
  memset(code, 0, 50*sizeof(char));
  sprintf(code, "jr $ra"); // 结果已在v0中，无需改变存放的寄存器
  printObj(code);
  memset(code, 0, 50*sizeof(char));

  // write
  sprintf(code, "\nwrite:");
  printObj(code);
  memset(code, 0, 50*sizeof(char));

  sprintf(code, "subu $sp, $sp, %d", 8);
  printObj(code);
  memset(code, 0, 50*sizeof(char));
  sprintf(code, "sw $ra, %d($sp)\nsw $fp, %d($sp)", 4, 0);
  printObj(code);
  memset(code, 0, 50*sizeof(char));
  sprintf(code, "addi $fp, $sp, %d", 8);
  printObj(code);
  memset(code, 0, 50*sizeof(char));


  sprintf(code, "li $v0, 1\nsyscall\nli $v0, 4"); // 输出
  printObj(code);
  memset(code, 0, 50*sizeof(char));
  sprintf(code, "la $a0, _ret\nsyscall\nmove $v0, $0"); // 换个行
  printObj(code);
  memset(code, 0, 50*sizeof(char));

  // 开始恢复寄存器
  sprintf(code, "move $sp, $fp");
  printObj(code);
  memset(code, 0, 50*sizeof(char));
  sprintf(code, "lw $ra, %d($sp)\nlw $fp, %d($sp)", -4, -8);
  printObj(code);
  memset(code, 0, 50*sizeof(char));
  sprintf(code, "jr $ra"); // 结果已在v0中，无需改变存放的寄存器
  printObj(code);
  memset(code, 0, 50*sizeof(char));

}

char* bigLetter(char* name){
  char mainFunc[] = "main";
  if((((*name)>='a')&&((*name)<='z'))&&(strcmp(name, mainFunc)!=0)){
    *name = *name - ('a' -'A');
  }
  return name;
}

void generateObj(InterCodes head, VarIndexes varIndexes){
  RegStatus regStatus = (RegStatus)malloc(sizeof(struct RegStatus_)*(numReg+numSpiltReg));
  memset(regStatus, 0, sizeof(struct RegStatus_)*(numReg+numSpiltReg));
  for(int i=0; i<numReg+numSpiltReg; i++){
    RegStatus currentReg = regStatus+i;
    if(i<numT){
      currentReg->isS = 0;
      currentReg->index = i;
    }else{
      currentReg->isS = 1;
      currentReg->index = i-numT;
    }
  }

  int fpIndex = 0;
  int paramIndex = 0;
  int* fpSize = malloc(sizeof(int));
  memset(fpSize, 0, sizeof(int));

  print_begin_code(varIndexes);

  InterCodes tempCodes = head;
  while(tempCodes!=NULL){
    InterCode interCode = tempCodes->code;
    char* code=(char*)malloc(50*sizeof(char));
    memset(code, 0, 50*sizeof(char));
    if (interCode->kind == LABEL) {

      sprintf(code, "%s:", bigLetter(interCode->u.label.label->u.name));

    } else if ((interCode->kind == ASSIGN)&&(interCode->u.assign.right->kind!=ADDRESS)) {

      char *regX;
      if(interCode->u.assign.right->kind!=CONSTANT) {
        char *regY = getReg(regStatus, interCode->u.assign.right, varIndexes);
        regX = getReg(regStatus, interCode->u.assign.left, varIndexes);
        sprintf(code, "move %s, %s", regX, regY);
      }else{
        regX = getReg(regStatus, interCode->u.assign.left, varIndexes);
        sprintf(code, "li %s, %d", regX, interCode->u.assign.right->u.value);
      }
      // save the spilt reg of result
      saveSpiltVar(fpSize, regStatus, interCode->u.assign.left, regX, varIndexes, fpIndex);
      freeSplitReg(regStatus, NULL);

    } else if (interCode->kind == MATHOP) {

      char* regX;
      if((*(interCode->u.binop.mathop->u.name))=='+'){
        if(interCode->u.binop.op1->kind==CONSTANT){
          // 加法表达式中不可能同时出现两个常量（不然会被优化掉）
          char *regY = getReg(regStatus, interCode->u.binop.op2, varIndexes);
          regX = getReg(regStatus, interCode->u.binop.result, varIndexes);
          sprintf(code, "addi %s, %s, %d", regX, regY, interCode->u.binop.op1->u.value);
        }else if(interCode->u.binop.op2->kind==CONSTANT){
          // 加法表达式中不可能同时出现两个常量（不然会被优化掉）
          char *regY = getReg(regStatus, interCode->u.binop.op1, varIndexes);
          regX = getReg(regStatus, interCode->u.binop.result, varIndexes);
          sprintf(code, "addi %s, %s, %d", regX, regY, interCode->u.binop.op2->u.value);
        }else{
          char *regZ = getReg(regStatus, interCode->u.binop.op2, varIndexes);
          char *regY = getReg(regStatus, interCode->u.binop.op1, varIndexes);
          regX = getReg(regStatus, interCode->u.binop.result, varIndexes);
          sprintf(code, "add %s, %s, %s", regX, regY, regZ);
        }
      }else if((*(interCode->u.binop.mathop->u.name))=='-'){
        if(interCode->u.binop.op1->kind==CONSTANT){
          // 减法表达式中不可能同时出现两个常量（不然会被优化掉）
          char *regY = getReg(regStatus, interCode->u.binop.op2, varIndexes);
          regX = getReg(regStatus, interCode->u.binop.result, varIndexes);
          sprintf(code, "addi %s, %s, %d", regX, regY, -(interCode->u.binop.op1->u.value));
        }else if(interCode->u.binop.op2->kind==CONSTANT){
          // 减法表达式中不可能同时出现两个常量（不然会被优化掉）
          char *regY = getReg(regStatus, interCode->u.binop.op1, varIndexes);
          regX = getReg(regStatus, interCode->u.binop.result, varIndexes);
          sprintf(code, "addi %s, %s, %d", regX, regY, -(interCode->u.binop.op2->u.value));
        }else{
          char *regZ = getReg(regStatus, interCode->u.binop.op2, varIndexes);
          char *regY = getReg(regStatus, interCode->u.binop.op1, varIndexes);
          regX = getReg(regStatus, interCode->u.binop.result, varIndexes);
          sprintf(code, "sub %s, %s, %s", regX, regY, regZ);
        }
      }else if((*(interCode->u.binop.mathop->u.name))=='*'){
        char *regZ = getReg(regStatus, interCode->u.binop.op2, varIndexes);
        char *regY = getReg(regStatus, interCode->u.binop.op1, varIndexes);
        regX = getReg(regStatus, interCode->u.binop.result, varIndexes);
        sprintf(code, "mul %s, %s, %s", regX, regY, regZ);
      }else if((*(interCode->u.binop.mathop->u.name))=='/'){
        char *regZ = getReg(regStatus, interCode->u.binop.op2, varIndexes);
        char *regY = getReg(regStatus, interCode->u.binop.op1, varIndexes);
        regX = getReg(regStatus, interCode->u.binop.result, varIndexes);
        sprintf(code, "div %s, %s\nmflo %s", regY, regZ, regX);
      }
      // save the spilt reg of result
      saveSpiltVar(fpSize, regStatus, interCode->u.binop.result, regX, varIndexes, fpIndex);
      freeSplitReg(regStatus, NULL);

    } else if (((interCode->kind == ASSIGN)&&(interCode->u.assign.right->kind == ADDRESS))||(interCode->kind == RIGHT_ADDR)) {

      char *regY = getReg(regStatus, interCode->u.assign.right, varIndexes);
      char *regX = getReg(regStatus, interCode->u.assign.left, varIndexes);
      sprintf(code, "move %s, %s", regX, regY);
      // save the spilt reg of result
      saveSpiltVar(fpSize, regStatus, interCode->u.assign.left, regX, varIndexes, fpIndex);
      freeSplitReg(regStatus, NULL);

    } else if (interCode->kind == RIGHT_STAR) {

      char *regY = getReg(regStatus, interCode->u.assign.right, varIndexes);
      char *regX = getReg(regStatus, interCode->u.assign.left, varIndexes);
      sprintf(code, "lw %s, 0(%s)", regX, regY);
      // save the spilt reg of result
      saveSpiltVar(fpSize, regStatus, interCode->u.assign.left, regX, varIndexes, fpIndex);
      freeSplitReg(regStatus, NULL);

    } else if (interCode->kind == LEFT_STAR) {

      char *regY = getReg(regStatus, interCode->u.assign.right, varIndexes);
      char *regX = getReg(regStatus, interCode->u.assign.left, varIndexes);
      sprintf(code, "sw %s, 0(%s)", regY, regX);
      freeSplitReg(regStatus, NULL);

    } else if (interCode->kind == GOTO) {

      sprintf(code, "j %s", bigLetter(interCode->u.label.label->u.name));

    } else if (interCode->kind == RELOP) {

      char relop1[] = ">",
        relop2[] = "<=",
        relop3[] = "==",
        relop4[] = "!=",
        relop5[] = "<",
        relop6[] = ">=";
      char* origin = interCode->u.goto_con.relop->u.name;
      char *regX = getReg(regStatus, interCode->u.goto_con.left, varIndexes);
      char *regY = getReg(regStatus, interCode->u.goto_con.right, varIndexes);
      char* Z = bigLetter(interCode->u.goto_con.label->u.name);
      if (strcmp(origin, relop1)==0) {
        sprintf(code, "bgt %s, %s, %s", regX, regY, Z);
      }else if (strcmp(origin, relop2)==0){
        sprintf(code, "ble %s, %s, %s", regX, regY, Z);
      }else if (strcmp(origin, relop3)==0){
        sprintf(code, "beq %s, %s, %s", regX, regY, Z);
      }else if (strcmp(origin, relop4)==0){
        sprintf(code, "bne %s, %s, %s", regX, regY, Z);
      }else if (strcmp(origin, relop5)==0){
        sprintf(code, "blt %s, %s, %s", regX, regY, Z);
      }else if (strcmp(origin, relop6)==0){
        sprintf(code, "bge %s, %s, %s", regX, regY, Z);
      }
      freeSplitReg(regStatus, NULL);

    } else if (interCode->kind == ARG) {

      int argNum = 0;
      InterCodes currentCodes = tempCodes;
      while((currentCodes!=NULL)&&(currentCodes->code->kind==ARG)){
        argNum += 1;
        currentCodes = currentCodes->next;
      }
      int spSize = (0>(4*(argNum-5)))?0:(4*(argNum-5));
      if(spSize!=0) {
        sprintf(code, "subu $sp, $sp, %d", spSize);
      }
      printObj(code);
      memset(code, 0, 50*sizeof(char));
      currentCodes = tempCodes;
      for(int currentArg = argNum; currentArg >=1; currentArg--){
        char *arg = getReg(regStatus, currentCodes->code->u.label.label, varIndexes);
        if(currentArg>=5){
          int offset = 4*(currentArg-5);
          sprintf(code, "sw %s, %d($sp)", arg, offset);
          printObj(code);
          memset(code, 0, 50*sizeof(char));
        }else{
          sprintf(code, "move $a%d, %s", currentArg-1, arg);
          printObj(code);
          memset(code, 0, 50*sizeof(char));
        }
        freeSplitReg(regStatus, NULL);
        currentCodes = currentCodes->next;
      }

      if(currentCodes->code->kind==CALL) {
        char* result = getReg(regStatus, currentCodes->code->u.assign.left, varIndexes);
        if(spSize!=0) {
          sprintf(code, "jal %s\nmove %s, $v0\naddi $sp, $sp, %d", bigLetter(currentCodes->code->u.assign.right->u.name), result, spSize);
        }else{
          sprintf(code, "jal %s\nmove %s, $v0", bigLetter(currentCodes->code->u.assign.right->u.name), result);
        }
        saveSpiltVar(fpSize, regStatus, currentCodes->code->u.assign.left, result, varIndexes, fpIndex);
        freeSplitReg(regStatus, NULL);
      }else{
        printf("wrong!!!!!!!!!!!\n");
      }

      tempCodes = currentCodes;

    } else if (interCode->kind == CALL) {

      char* result = getReg(regStatus, interCode->u.assign.left, varIndexes);
      sprintf(code, "jal %s\nmove %s, $v0", interCode->u.assign.right->u.name, result);
      saveSpiltVar(fpSize, regStatus, interCode->u.assign.left, result, varIndexes, fpIndex);
      freeSplitReg(regStatus, NULL);

    } else if (interCode->kind == FUNCTION) {

      fpIndex += 1;
      *fpSize = 0;
      paramIndex = 0;
      sprintf(code, "\n%s:", bigLetter(interCode->u.label.label->u.name));
      printObj(code);
      memset(code, 0, 50*sizeof(char));

      int paramNum = 0;
      InterCodes currentCodes = tempCodes->next;
      while(tempCodes->code->kind==PARAM){
        paramNum += 1;
        tempCodes = tempCodes->next;
      }

      int numLiveS=0;
      int s_regs = count_s_regs(varIndexes);
      for(int i=0;i<numS;i++){
        if(((s_regs>>i)&0x1)==1){
          numLiveS += 1;
        }
      }

      int spOff = 4*(numLiveS)+8;
      sprintf(code, "subu $sp, $sp, %d", spOff);
      printObj(code);
      memset(code, 0, 50*sizeof(char));
      sprintf(code, "sw $ra, %d($sp)\nsw $fp, %d($sp)", spOff-4, spOff-8);
      printObj(code);
      memset(code, 0, 50*sizeof(char));
      sprintf(code, "addi $fp, $sp, %d", spOff);
      printObj(code);
      memset(code, 0, 50*sizeof(char));

      int offReg = 8;
      for(int i=0;i<numS;i++){
        if(((s_regs>>i)&0x1)==1){
          offReg += 4;
          sprintf(code, "sw $s%d, %d($fp)", i, -offReg);
          printObj(code);
          memset(code, 0, 50*sizeof(char));
        }
      }

      *fpSize = *fpSize + spOff;
      freeAllReg(regStatus);


    } else if (interCode->kind == PARAM) {

      paramIndex += 1;
      if(find_varIndex(varIndexes, interCode->u.label.label)>31){
        // do nothing
      }else{
        char* param = getReg(regStatus, interCode->u.label.label, varIndexes);
        if(paramIndex>=5){
          int offset = 4*(paramIndex-5);
          sprintf(code, "lw %s, %d($fp)", param, offset);
          saveParamAddr(regStatus, interCode->u.label.label, varIndexes, fpIndex, offset);
        }else{
          sprintf(code, "move %s, $a%d", param, paramIndex-1);
        }
        saveSpiltVar(fpSize, regStatus, interCode->u.label.label, param, varIndexes, fpIndex);
        freeSplitReg(regStatus, NULL);
      }

    } else if (interCode->kind == RETURN) {

      int s_regs = count_s_regs(varIndexes);
      int offReg = 8;
      for(int i=0;i<numS;i++){
        if(((s_regs>>i)&0x1)==1){
          offReg += 4;
          sprintf(code, "lw $s%d, %d($fp)", i, -offReg);
          printObj(code);
          memset(code, 0, 50*sizeof(char));
        }
      }

      // TODO: still have doubt here: is having lw $fp, -8($fp) good ?
      sprintf(code, "move $sp, $fp");
      printObj(code);
      memset(code, 0, 50*sizeof(char));
      sprintf(code, "lw $ra, -4($fp)");
      printObj(code);
      memset(code, 0, 50*sizeof(char));
      sprintf(code, "lw $fp, -8($fp)");
      printObj(code);
      memset(code, 0, 50*sizeof(char));

      char* result = getReg(regStatus, interCode->u.label.label, varIndexes);
      sprintf(code, "move $v0, %s\njr $ra", result);

    } else if (interCode->kind == DEC) {

      saveDecAddr(regStatus, interCode->u.assign.left, varIndexes, fpIndex, interCode->u.assign.right->u.value, fpSize);
      sprintf(code, "subu $sp, $sp, %d", interCode->u.assign.right->u.value);
      printObj(code);
      memset(code, 0, 50*sizeof(char));
      int offset = offsetFp(interCode->u.assign.left, varIndexes, 1);
      char *regX = getReg(regStatus, interCode->u.assign.left, varIndexes);
      if(regX!=NULL) {
        sprintf(code, "addi %s, $fp, %d", regX, offset);
        saveSpiltVar(fpSize, regStatus, interCode->u.assign.left, regX, varIndexes, fpIndex);
        freeSplitReg(regStatus, NULL);
      }

    } else if (interCode->kind == READ) {

      char* result = getReg(regStatus, interCode->u.label.label, varIndexes);
      sprintf(code, "jal read\nmove %s, $v0", result);
      saveSpiltVar(fpSize, regStatus, interCode->u.label.label, result, varIndexes, fpIndex);
      freeSplitReg(regStatus, NULL);

    } else if (interCode->kind == WRITE) {

      char* output = getReg(regStatus, interCode->u.label.label, varIndexes);
      sprintf(code, "move $a0, %s\njal write", output);
      freeSplitReg(regStatus, NULL);

    } else {
      // do nothing
    }
    printObj(code);
    tempCodes = tempCodes->next;
  }
}