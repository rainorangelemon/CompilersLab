//
// Created by rainorangelemon on 7/5/18.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mips.h"

void print_codes_status(int* in, int* out, int* def, int* use, struct Succ* succ, int numInterCodes, InterCodes head);
void getVariables(VarIndexes varIndexes, InterCodes head);
void calculateBasic(InterCodes head, VarIndexes varIndexes);
void print_binary(int bits);

void createMips(InterCodes head){
  VarIndexes varIndexes = (VarIndexes)malloc(sizeof(struct VarIndexes_));
  memset(varIndexes, 0, sizeof(struct VarIndexes_));
  getVariables(varIndexes, head);
  Basic basic = calculateBasic(head, varIndexes);

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
    return -1;
  }
}

void addVariableIndex(VarIndexes varIndexes, Operand operand){
  if(find_varIndex(varIndexes, operand)<0){
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
    } else if (interCode->kind == MATHOP) {
      addVariableIndex(varIndexes, interCode->u.binop.result);
    } else if (interCode->kind == RIGHT_ADDR) {
      addVariableIndex(varIndexes, interCode->u.assign.left);
    } else if (interCode->kind == RIGHT_STAR) {
      addVariableIndex(varIndexes, interCode->u.assign.left);
    } else if (interCode->kind == LEFT_STAR) {
      // do nothing
    } else if (interCode->kind == GOTO) {
      // do nothing
    } else if (interCode->kind == RELOP) {
      // do nothing
    } else if (interCode->kind == RETURN) {
      // do nothing
    } else if (interCode->kind == DEC) {
      // do nothing
    } else if (interCode->kind == ARG) {
      // do nothing
    } else if (interCode->kind == CALL) {
      addVariableIndex(varIndexes, interCode->u.assign.left);
    } else if (interCode->kind == PARAM) {
      // do nothing
    } else if (interCode->kind == READ) {
      addVariableIndex(varIndexes, interCode->u.label.label);
    } else if (interCode->kind == WRITE) {
      // do nothing
    } else {
      // do nothing
    }
    temp = temp->next;
  }
  VarIndexes varIndexes1 = varIndexes;
  while(varIndexes1!=NULL){
    printf("%d: %s\n", varIndexes1->varIndex->index, print_operand(varIndexes1->varIndex->operand, 1));
    varIndexes1 = varIndexes1->next;
  }
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

Basic calculateBasic(InterCodes head, VarIndexes varIndexes){
  int numInterCodes=1;
  InterCodes temp = head;
  while(temp!=NULL){
    numInterCodes += 1;
    temp = temp->next;
  }

  printf("control flow!\n");

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

  printf("succ, def and use!\n");

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
      *tempUse = *tempUse | (1 << find_varIndex(varIndexes, interCode->u.assign.right));
    } else if (interCode->kind == MATHOP) {
      // def
      *tempDef = *tempDef | (1 << find_varIndex(varIndexes, interCode->u.binop.result));
      // use
      *tempUse = *tempUse | (1 << find_varIndex(varIndexes, interCode->u.binop.op1));
      *tempUse = *tempUse | (1 << find_varIndex(varIndexes, interCode->u.binop.op2));
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
    } else if (interCode->kind == GOTO) {
      // do nothing
    } else if (interCode->kind == RELOP) {
      // no def
      // use
      *tempUse = *tempUse | (1 << find_varIndex(varIndexes, interCode->u.goto_con.left));
      *tempUse = *tempUse | (1 << find_varIndex(varIndexes, interCode->u.goto_con.right));
    } else if (interCode->kind == RETURN) {
      *tempUse = *tempUse | (1 << find_varIndex(varIndexes, interCode->u.label.label));
    } else if (interCode->kind == DEC) {
      // do nothing
    } else if (interCode->kind == ARG) {
      // do nothing
    } else if (interCode->kind == CALL) {
      // def
      *tempDef = *tempDef | (1 << find_varIndex(varIndexes, interCode->u.assign.left));
    } else if (interCode->kind == PARAM) {
      // do nothing
    } else if (interCode->kind == READ) {
      // def
      *tempDef = *tempDef | (1 << find_varIndex(varIndexes, interCode->u.label.label));
    } else if (interCode->kind == WRITE) {
      // use
      *tempUse = *tempUse | (1 << find_varIndex(varIndexes, interCode->u.label.label));
    } else {
      // do nothing
    }

    // go to next instr
    temp = temp->next;
  }

  printf("in and out!\n");

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

  Basic result = (Basic)malloc(sizeof(struct Basic_));
  result.def = def;
  result.in = in;
  result.out = out;
  result.succ = succ;
  result.use = use;
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