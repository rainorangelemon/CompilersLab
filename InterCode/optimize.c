//
// Created by rainorangelemon on 6/11/18.
//

#include "intercode.h"

typedef struct Changes_* Changes;

struct Changes_{
  char* name;
  int change_time;
  Changes next;
  Changes prev;
};

void free_code(InterCode interCode);

void remove_InterCode(InterCodes interCodes, InterCode interCode){
  InterCodes temp = interCodes;
  while(temp!=NULL){
    if(temp->code==interCode){
      if(temp->next!=NULL) {
        temp->next->prev = temp->prev;
      }
      if(temp->prev!=NULL) {
        temp->prev->next = temp->next;
      }
      free_code(temp->code);
      free(temp);
    }
  }
}

void free_code(InterCode interCode) {
  if ((interCode->kind == ARG)
      || (interCode->kind == LABEL)
      || (interCode->kind == FUNCTION)
      || (interCode->kind == GOTO)
      || (interCode->kind == RETURN)
      || (interCode->kind == PARAM)
      || (interCode->kind == WRITE)
      || (interCode->kind == READ)) {
    free(interCode->u.label.label);
  } else if (interCode->kind == MATHOP) {
    free(interCode->u.binop.op2);
    free(interCode->u.binop.op1);
    free(interCode->u.binop.mathop);
    free(interCode->u.binop.result);
  } else if (interCode->kind == RELOP) {
    free(interCode->u.goto_con.label);
    free(interCode->u.goto_con.left);
    free(interCode->u.goto_con.relop);
    free(interCode->u.goto_con.right);
  } else if(interCode->kind != EMPTY){
    free(interCode->u.assign.left);
    free(interCode->u.assign.right);
  }
}

void refresh_list(Changes changes, Operand operand, int time_step){
  if(operand->kind!=CONSTANT){
    Changes temp = changes;
    while(temp!=NULL){
      if((temp->name!=NULL)&&(strcmp(temp->name, operand->u.name)==0)){
        temp->change_time = time_step;
        return;
      }else{
        if(temp->next!=NULL) {
          temp = temp->next;
        }else{
          break;
        }
      }
    }
    Changes newChange = (Changes) malloc(sizeof(struct Changes_));
    memset(newChange, 0, sizeof(struct Changes_));
    newChange->change_time = time_step;
    newChange->prev = temp;
    newChange->name = operand->u.name;
    temp->next = newChange;
  }
}

void build_change_list(Changes changes, InterCodes interCodes) {
  InterCodes temp = interCodes;
  int time = 1;
  while (temp != NULL) {
    InterCode interCode = temp->code;
    if (interCode->kind == MATHOP) {
      printf("MATHOP\n");
      refresh_list(changes, interCode->u.binop.result, time);
    } else if (interCode->kind == RELOP) {
      printf("RELOP\n");
      // do nothing
    } else if ((interCode->kind == ARG)
               || (interCode->kind == LABEL)
               || (interCode->kind == FUNCTION)
               || (interCode->kind == GOTO)
               || (interCode->kind == RETURN)
               || (interCode->kind == PARAM)
               || (interCode->kind == WRITE)
               || (interCode->kind == READ)) {
      if (interCode->kind == READ) {
        printf("READ\n");
        refresh_list(changes, interCode->u.label.label, time);
      }
    } else if ((interCode->kind != DEC)&&(interCode->kind!=EMPTY)) {
      printf("ASSIGN\n");
      refresh_list(changes, interCode->u.assign.left, time);
    }
    temp = temp->next;
    time++;
  }
}

void delete_zero(InterCodes interCodes){
  InterCodes temp = interCodes;
  while(temp!=NULL){
    if(temp->code->kind == MATHOP){
      Operand result = temp->code->u.binop.result;
      Operand left = temp->code->u.binop.op1;
      Operand right = temp->code->u.binop.op2;
      if(((*temp->code->u.binop.mathop->u.name)=='-')||((*temp->code->u.binop.mathop->u.name)=='+')){
        if((left->kind==CONSTANT)&&(left->u.value==0)){
          temp->code->kind = ASSIGN;
          temp->code->u.assign.left = result;
          temp->code->u.assign.right = right;
        }else if((right->kind==CONSTANT)&&(right->u.value==0)){
          temp->code->kind = ASSIGN;
          temp->code->u.assign.left = result;
          temp->code->u.assign.right = left;
        }
      }else if((*temp->code->u.binop.mathop->u.name)=='*'){
        if((left->kind==CONSTANT)&&(left->u.value==1)){
          temp->code->kind = ASSIGN;
          temp->code->u.assign.left = result;
          temp->code->u.assign.right = right;
        }else if((right->kind==CONSTANT)&&(right->u.value==1)){
          temp->code->kind = ASSIGN;
          temp->code->u.assign.left = result;
          temp->code->u.assign.right = left;
        }
      }else{
        if((right->kind==CONSTANT)&&(right->u.value==1)){
          temp->code->kind = ASSIGN;
          temp->code->u.assign.left = result;
          temp->code->u.assign.right = left;
        }
      }
    }
    temp = temp->next;
  }
}

Changes find_changes(Changes changes, char* name){
  Changes temp =changes;
  while(temp!=NULL){
    if((temp->name!=NULL)&&(strcmp(temp->name, name)==0)){
      return temp;
    }else{
      temp = temp->next;
    }
  }
  return NULL;
}

int find_changes_time(Changes changes, Operand operand){
  if(operand->kind==CONSTANT){
    return 0;
  }else{
    Changes target = find_changes(changes, operand->u.name);
    if(target!=NULL){
      return target->change_time;
    }else{
      return 1000000; // very big
    }
  }
}

int before_change(int change_left, int change_right){
  if(change_left < change_right){
    return 1;
  }else{
    return 0;
  }
}

void replace_variable_on_operand(Operand target, Operand oldOperand, Operand newOperand){
  if(((target->kind==oldOperand->kind)&&(oldOperand->kind==CONSTANT)&&(oldOperand->u.value==target->u.value))
      ||((target->kind==oldOperand->kind)&&(oldOperand->kind!=CONSTANT)&&(strcmp(oldOperand->u.name, target->u.name)==0))){
    if(newOperand->kind!=CONSTANT){
      target->kind = newOperand->kind;
      target->u.name = newOperand->u.name;
    }else{
      target->kind = newOperand->kind;
      target->u.value = newOperand->u.value;
    }
  }
}

void replace_variable(InterCodes interCodes, Operand oldOperand, Operand newOperand){
  InterCodes temp = interCodes;
  while(temp!=NULL){
    InterCode interCode = temp->code;
    if ((interCode->kind == ARG)
        || (interCode->kind == LABEL)
        || (interCode->kind == FUNCTION)
        || (interCode->kind == GOTO)
        || (interCode->kind == RETURN)
        || (interCode->kind == PARAM)
        || (interCode->kind == WRITE)
        || (interCode->kind == READ)) {
      if ((interCode->kind == ARG)
          || (interCode->kind == RETURN)
          || (interCode->kind == PARAM)
          || (interCode->kind == WRITE)
          || (interCode->kind == READ)) {
        replace_variable_on_operand(interCode->u.label.label, oldOperand, newOperand);
      }
    } else if (interCode->kind == MATHOP) {
      replace_variable_on_operand(interCode->u.binop.op2, oldOperand, newOperand);
      replace_variable_on_operand(interCode->u.binop.op1, oldOperand, newOperand);
      replace_variable_on_operand(interCode->u.binop.result, oldOperand, newOperand);
    } else if (interCode->kind == RELOP) {
      replace_variable_on_operand(interCode->u.goto_con.left, oldOperand, newOperand);
      replace_variable_on_operand(interCode->u.goto_con.right, oldOperand, newOperand);
    } else if(interCode->kind!=EMPTY){
      replace_variable_on_operand(interCode->u.assign.left, oldOperand, newOperand);
      replace_variable_on_operand(interCode->u.assign.right, oldOperand, newOperand);
    }
    temp = temp->next;
  }
}

void replace_label(InterCodes interCodes, Operand oldOperand, Operand newOperand){
  InterCodes temp = interCodes;
  while(temp!=NULL){
    InterCode interCode = temp->code;
    if ((interCode->kind == LABEL)||(interCode->kind == GOTO)){
      if(strcmp(interCode->u.label.label->u.name, oldOperand->u.name)==0){
        interCode->u.label.label->u.name = newOperand->u.name;
      }
    }else if(interCode->kind == RELOP){
      printf("RELOP label: %s %s\n", interCode->u.goto_con.label->u.name, oldOperand->u.name);
      if(strcmp(interCode->u.goto_con.label->u.name, oldOperand->u.name)==0){
        interCode->u.goto_con.label->u.name = newOperand->u.name;
      }
    }
    temp = temp->next;
  }
}

void cover_assign_temps(InterCodes interCodes){
  Changes changes = (Changes) malloc(sizeof(struct Changes_));
  memset(changes, 0, sizeof(struct Changes_));
  build_change_list(changes, interCodes);
  InterCodes temp = interCodes;
  while(temp!=NULL){
    if(temp->code->kind == ASSIGN){
      int change_left = find_changes_time(changes, temp->code->u.assign.left);
      int change_right = find_changes_time(changes, temp->code->u.assign.right);
      // TODO: 仅更新基本块内的临时变量
      if((before_change(change_right, change_left)==1)&&((*temp->code->u.assign.left->u.name)!='v')){
        temp->code->kind = EMPTY;
        replace_variable(interCodes, temp->code->u.assign.left, temp->code->u.assign.right);
      }
    }
    temp = temp->next;
  }
}

InterCodes get_next_code(InterCodes interCodes){
  InterCodes temp = interCodes->next;
  while((temp!=NULL)){
    if(temp->code->kind!=EMPTY){
      return temp;
    }else{
      temp = temp->next;
    }
  }
  return NULL;
}

void merge_labels(InterCodes interCodes){
  InterCodes temp = interCodes;
  while(temp!=NULL){
    InterCodes nextCode = get_next_code(temp);
    if(temp->code->kind == LABEL){
      if(nextCode!=NULL){
        if(nextCode->code->kind == LABEL){
            temp->code->kind = EMPTY;
            printf("begin replacing\n");
            replace_label(interCodes, temp->code->u.label.label, nextCode->code->u.label.label);
            printf("end replacing\n");
        }
      }
    }
    temp = nextCode;
  }
}

char* replace_relop(char* origin) {
  char relop1[] = ">",
    relop2[] = "<=",
    relop3[] = "==",
    relop4[] = "!=",
    relop5[] = "<",
    relop6[] = ">=";
  char *result = (char *) malloc(3 * sizeof(char));
  memset(result, 0, 3 * sizeof(char));
  if (strcmp(origin, relop1)==0) {
    sprintf(result, "<=");
  }else if (strcmp(origin, relop2)==0){
    sprintf(result, ">");
  }else if (strcmp(origin, relop3)==0){
    sprintf(result, "!=");
  }else if (strcmp(origin, relop4)==0){
    sprintf(result, "==");
  }else if (strcmp(origin, relop5)==0){
    sprintf(result, ">=");
  }else if (strcmp(origin, relop6)==0){
    sprintf(result, "<");
  }
  return result;
}

void reduce_goto(InterCodes interCodes) {
  InterCodes temp = interCodes;
  while (temp != NULL) {
    InterCodes nextCode = get_next_code(temp);
    if (temp->code->kind == RELOP) {
      if (nextCode != NULL) {
        InterCodes nextNextCode = get_next_code(nextCode);
        if (nextNextCode != NULL) {
          if ((nextCode->code->kind == GOTO)
              && (nextNextCode->code->kind == LABEL)
              && (strcmp(temp->code->u.goto_con.label->u.name, nextNextCode->code->u.label.label->u.name) == 0)) {
            temp->code->u.goto_con.label->u.name = nextCode->code->u.label.label->u.name;
            nextCode->code->kind = EMPTY;
            nextNextCode->code->kind = EMPTY;
            temp->code->u.goto_con.relop->u.name = replace_relop(temp->code->u.goto_con.relop->u.name);
          }
        }
      }
    }
    temp = nextCode;
  }
}

void calculate_constant(InterCodes interCodes) {
  InterCodes temp = interCodes;
  while (temp != NULL) {
    InterCodes nextCode = get_next_code(temp);
    if (temp->code->kind == MATHOP) {
      if((temp->code->u.binop.op1->kind==CONSTANT)&&(temp->code->u.binop.op2->kind==CONSTANT)){
        char relop1[] = "+",
          relop2[] = "-",
          relop3[] = "*",
          relop4[] = "/";
        int value = 0;
        char* origin = temp->code->u.binop.mathop->u.name;
        if (strcmp(origin, relop1)==0) {
          value = temp->code->u.binop.op1->u.value + temp->code->u.binop.op2->u.value;
        }else if (strcmp(origin, relop2)==0){
          value = temp->code->u.binop.op1->u.value - temp->code->u.binop.op2->u.value;
        }else if (strcmp(origin, relop3)==0){
          value = temp->code->u.binop.op1->u.value * temp->code->u.binop.op2->u.value;
        }else{
          value = temp->code->u.binop.op1->u.value / temp->code->u.binop.op2->u.value;
        }
        temp->code->kind = ASSIGN;
        Operand left = temp->code->u.binop.result;
        Operand right = temp->code->u.binop.op1;
        right->u.value = value;
        free(temp->code->u.binop.op2);
        temp->code->u.assign.left = left;
        temp->code->u.assign.right = right;
      }
    }
    temp = nextCode;
  }
}

//  LABEL, FUNCTION, ASSIGN, MATHOP,
//  RIGHT_ADDR, RIGHT_STAR, LEFT_STAR, GOTO, RELOP, RETURN, DEC, ARG,
//  CALL, PARAM, READ, WRITE

void optimize_InterCodes(InterCodes interCodes){
  for(int i=0; i<2; i++) {
    delete_zero(interCodes);
    printf("here\n");
    merge_labels(interCodes);
    printf("reduce_goto\n");
    reduce_goto(interCodes);
//    cover_assign_temps(interCodes);
    calculate_constant(interCodes);
    printf("leaving\n");
  }
}