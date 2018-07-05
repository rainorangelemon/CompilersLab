//
// Created by rainorangelemon on 7/5/18.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mips.h"

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

void getVariables(InterCodes head){
  VarIndexes varIndexes = (VarIndexes)malloc(sizeof(struct VarIndexes_));
  memset(varIndexes, 0, sizeof(struct VarIndexes_));
  InterCodes temp = head;
  while(temp!=NULL) {
    InterCode interCode = temp->code;
    if (interCode->kind == LABEL) {
      // do nothing
    } else if (interCode->kind == FUNCTION) {
      // do nothing
    } else if (interCode->kind == ASSIGN) {
      addVariableIndex(varIndexes, interCode->u.assign.left);
      addVariableIndex(varIndexes, interCode->u.assign.right);
    } else if (interCode->kind == MATHOP) {
      addVariableIndex(varIndexes, interCode->u.binop.result);
      addVariableIndex(varIndexes, interCode->u.binop.op1);
      addVariableIndex(varIndexes, interCode->u.binop.op2);
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
      // do nothing
    } else if (interCode->kind == DEC) {
      addVariableIndex(varIndexes, interCode->u.assign.left);
      addVariableIndex(varIndexes, interCode->u.assign.right);
    } else if (interCode->kind == ARG) {
      addVariableIndex(varIndexes, interCode->u.label.label);
    } else if (interCode->kind == CALL) {
      addVariableIndex(varIndexes, interCode->u.assign.left);
    } else if (interCode->kind == PARAM) {
      addVariableIndex(varIndexes, interCode->u.label.label);
    } else if (interCode->kind == READ) {
      addVariableIndex(varIndexes, interCode->u.label.label);
    } else if (interCode->kind == WRITE) {
      addVariableIndex(varIndexes, interCode->u.label.label);
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
  }
  return -1;
}

Basic calculateBasic(InterCodes head, VarIndexes varIndexes){
  int numInterCodes=1;
  InterCodes temp = head;
  while(temp!=NULL){
    numInterCodes += 1;
  }
  int* in = (int*)malloc(sizeof(int)*numInterCodes);
  int* out = (int*)malloc(sizeof(int)*numInterCodes);
  int* def = (int*)malloc(sizeof(int)*numInterCodes);
  int* use = (int*)malloc(sizeof(int)*numInterCodes);
  struct Succ* succ = (Succ*)malloc(sizeof(struct Succ)*numInterCodes);
  memset(in, 0, sizeof(int)*numInterCodes);
  memset(out, 0, sizeof(int)*numInterCodes);
  memset(def, 0, sizeof(int)*numInterCodes);
  memset(use, 0, sizeof(int)*numInterCodes);
  memset(succ, 0, sizeof(struct Succ)*numInterCodes);
  // define succ
  int codeIndex = 0;
  InterCodes temp = head;
  while(temp!=NULL){
    codeIndex += 1;
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
    } else if ((interCode->kind != EMPTY)&&(codeIndex<numInterCodes)){
      tempSucc->succ1 = codeIndex+1;
      tempSucc->succ2 = 0;
    } else {
      tempSucc->succ1 = 0;
      tempSucc->succ2 = 0;
    }
    temp = temp->next;
  }
}