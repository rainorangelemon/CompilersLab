//
// Created by rainorangelemon on 6/11/18.
//

#include "intercode.h"

typedef struct Changes_* Changes;
typedef struct Valid_Block_* Valid_Block;

struct Changes_{
  char* name;
  int earliest_change_time;
  int latest_change_time;
  Changes next;
  Changes prev;
};

struct Valid_Block_{
  int block_begin;
  int block_end;
  Valid_Block prev;
  Valid_Block next;
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
        temp->earliest_change_time = (temp->earliest_change_time>time_step)?time_step:temp->earliest_change_time;
        temp->latest_change_time = (temp->latest_change_time<time_step)?time_step:temp->latest_change_time;
        return;
      }else{
        if(temp->next!=NULL) {
          temp = temp->next;
        }else if(temp->name==NULL){
          temp->name = operand->u.name;
          temp->earliest_change_time = 10000;
          temp->latest_change_time = 0;
          temp->earliest_change_time = (temp->earliest_change_time>time_step)?time_step:temp->earliest_change_time;
          temp->latest_change_time = (temp->latest_change_time<time_step)?time_step:temp->latest_change_time;
          return;
        }else{
          break;
        }
      }
    }
    Changes newChange = (Changes) malloc(sizeof(struct Changes_));
    memset(newChange, 0, sizeof(struct Changes_));
    newChange->prev = temp;
    newChange->name = operand->u.name;
    temp->next = newChange;
    newChange->earliest_change_time = 10000;
    newChange->latest_change_time = 0;

    newChange->earliest_change_time = (newChange->earliest_change_time>time_step)?time_step:newChange->earliest_change_time;
    newChange->latest_change_time = (newChange->latest_change_time<time_step)?time_step:newChange->latest_change_time;
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
      if ((interCode->kind == READ)||(interCode->kind == PARAM)) {
        printf("READ or PARAM\n");
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

void build_valid_block(Valid_Block valid_block, InterCodes interCodes){
  InterCodes temp = interCodes;
  Valid_Block current_block = valid_block;
  int i = 1;
  while(1){
    if(temp==NULL){
      if(current_block->block_end==0){
        current_block->block_end = i-1;
      }
      break;
    }
    if((temp->code->kind==LABEL)||(temp->code->kind==FUNCTION)){
      if(i!=1) {
        if(current_block->block_begin!=i) {
          if (current_block->block_end == 0) {
            current_block->block_end = i - 1;
          }
          Valid_Block temp_block = (Valid_Block) malloc(sizeof(struct Valid_Block_));
          memset(temp_block, 0, sizeof(struct Valid_Block_));
          temp_block->block_begin = i;
          current_block->next = temp_block;
          temp_block->prev = current_block;
          current_block = temp_block;
        }
      }
    }else if((temp->code->kind==CALL)||(temp->code->kind==GOTO)||(temp->code->kind==RELOP)){
      if(current_block->block_end==0){
        current_block->block_end = i;
      }
      Valid_Block temp_block = (Valid_Block) malloc(sizeof(struct Valid_Block_));
      memset(temp_block, 0, sizeof(struct Valid_Block_));
      temp_block->block_begin = i+1;
      current_block->next = temp_block;
      temp_block->prev = current_block;
      current_block = temp_block;
    }else{
      if(current_block->block_begin==0){
        current_block->block_begin = i;
      }
    }
    i++;
    temp = temp->next;
  }
}

void delete_zero(InterCodes interCodes){
  InterCodes temp = interCodes;
  while(temp!=NULL){
    if(temp->code->kind == MATHOP){
      Operand result = temp->code->u.binop.result;
      Operand left = temp->code->u.binop.op1;
      Operand right = temp->code->u.binop.op2;
      if((*temp->code->u.binop.mathop->u.name)=='+'){
        if((left->kind==CONSTANT)&&(left->u.value==0)){
          temp->code->kind = ASSIGN;
          temp->code->u.assign.left = result;
          temp->code->u.assign.right = right;
        }else if((right->kind==CONSTANT)&&(right->u.value==0)){
          temp->code->kind = ASSIGN;
          temp->code->u.assign.left = result;
          temp->code->u.assign.right = left;
        }
      }else if((*temp->code->u.binop.mathop->u.name)=='-'){
        if((left->kind==CONSTANT)&&(left->u.value==0)){
          // do nothing
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

int find_earliest_changes_time(Changes changes, Operand operand){
  if(operand->kind==CONSTANT){
    return 0;
  }else{
    Changes target = find_changes(changes, operand->u.name);
    if(target!=NULL){
      return target->earliest_change_time;
    }else{
      return 0; // very small
    }
  }
}

int find_latest_changes_time(Changes changes, Operand operand){
  if(operand->kind==CONSTANT){
    return 0;
  }else{
    Changes target = find_changes(changes, operand->u.name);
    if(target!=NULL){
      return target->latest_change_time;
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

int no_right_star_or_address(InterCodes interCodes, Operand operand){
  InterCodes temp = interCodes;
  while(temp!=NULL){
    if((temp->code->kind==RIGHT_STAR)||(temp->code->kind==RIGHT_ADDR)){
      if((operand->kind!=CONSTANT)
         &&(temp->code->u.assign.right->kind!=CONSTANT)
         &&(strcmp(operand->u.name, temp->code->u.assign.right->u.name)==0)){
        return 0;
      }
    }
    temp = temp->next;
  }
  return 1;
}

void replace_variable_on_operand(Operand target, Operand oldOperand, Operand newOperand){
  if(((target->kind==CONSTANT)&&(oldOperand->kind==CONSTANT)&&(oldOperand->u.value==target->u.value))
      ||((target->kind!=CONSTANT)&&(oldOperand->kind!=CONSTANT)&&(strcmp(oldOperand->u.name, target->u.name)==0))){
    if(newOperand->kind!=CONSTANT){
      target->kind = newOperand->kind;
      target->u.name = newOperand->u.name;
    }else{
      target->kind = newOperand->kind;
      target->u.value = newOperand->u.value;
    }
  }
}

void replace_variable_on_intercode(InterCode interCode, Operand oldOperand, Operand newOperand){
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
}

void replace_variable(InterCodes interCodes, Operand oldOperand, Operand newOperand){
  InterCodes temp = interCodes;
  while(temp!=NULL){
    replace_variable_on_intercode(temp->code, oldOperand, newOperand);
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

int compare_operand_with_operand(Operand operand1, Operand operand2){
  if(((operand1->kind==CONSTANT)&&(operand2->kind==CONSTANT))
     ||((operand1->kind!=CONSTANT)&&(operand2->kind!=CONSTANT))){
    if (operand1->kind == CONSTANT){
      return (operand1->u.value==operand2->u.value);
    }else{
      if(strcmp(operand1->u.name, operand2->u.name)==0){
        return 1;
      }else{
        return 0;
      }
    }
  }else{
    return 0;
  }
}

void change_temp_operand(InterCodes interCodes, int current_index, int block_end, Operand oldOperand, Operand newOperand){
  InterCodes temp = interCodes;
  while((temp!=NULL)&&(current_index<=block_end)){
    InterCode interCode = temp->code;
    if (interCode->kind == MATHOP) {
      if(compare_operand_with_operand(interCode->u.binop.result, newOperand)){
        printf("return: %d\n", current_index);
        return;
      }else{
        replace_variable_on_intercode(interCode, oldOperand, newOperand);
      }
    } else if (interCode->kind == RELOP) {
      replace_variable_on_intercode(interCode, oldOperand, newOperand);
    } else if ((interCode->kind == ARG)
               || (interCode->kind == LABEL)
               || (interCode->kind == FUNCTION)
               || (interCode->kind == GOTO)
               || (interCode->kind == RETURN)
               || (interCode->kind == PARAM)
               || (interCode->kind == WRITE)
               || (interCode->kind == READ)) {
      if (((interCode->kind == READ)||(interCode->kind == PARAM))&&(compare_operand_with_operand(interCode->u.label.label, newOperand))){
        printf("return: %d\n", current_index);
        return;
      }else{
        replace_variable_on_intercode(interCode, oldOperand, newOperand);
      }
    } else if ((interCode->kind != DEC)&&(interCode->kind!=EMPTY)) {
      if(compare_operand_with_operand(interCode->u.assign.left, newOperand)){
        printf("return: %d\n", current_index);
        return;
      }else{
        replace_variable_on_intercode(interCode, oldOperand, newOperand);
      }
    }
    current_index++;
    temp = temp->next;
  }
}

int visit_value(InterCodes interCodes, Operand operand){
  InterCodes temp = interCodes;
  while(temp!=NULL){
    InterCode interCode = temp->code;
    if (interCode->kind == MATHOP) {
      if(compare_operand_with_operand(interCode->u.binop.op1, operand)
         ||compare_operand_with_operand(interCode->u.binop.op2, operand)){
        return 1;
      }
    } else if (interCode->kind == RELOP) {
      if(compare_operand_with_operand(interCode->u.goto_con.left, operand)
         ||compare_operand_with_operand(interCode->u.goto_con.right, operand)){
        return 1;
      }
    } else if ((interCode->kind == ARG)
               || (interCode->kind == LABEL)
               || (interCode->kind == FUNCTION)
               || (interCode->kind == GOTO)
               || (interCode->kind == RETURN)
               || (interCode->kind == PARAM)
               || (interCode->kind == WRITE)
               || (interCode->kind == READ)) {
      if (compare_operand_with_operand(interCode->u.label.label, operand)) {
        return 1;
      }
    } else if ((interCode->kind != DEC)&&(interCode->kind!=EMPTY)) {
      if(compare_operand_with_operand(interCode->u.assign.right, operand)){
        return 1;
      }
    }
    temp = temp->next;
  }
  return 0;
}

void cover_assign_temps(InterCodes interCodes){
  Changes changes = (Changes) malloc(sizeof(struct Changes_));
  memset(changes, 0, sizeof(struct Changes_));
  build_change_list(changes, interCodes);
  Valid_Block valid_block = (Valid_Block) malloc(sizeof(struct Valid_Block_));
  memset(valid_block, 0, sizeof(struct Valid_Block_));
  valid_block->block_begin = 1;
  build_valid_block(valid_block, interCodes);
  Valid_Block current_block = valid_block;
  int current_code_index = 1;
  InterCodes temp = interCodes;
  while(temp!=NULL){
    if(temp->code->kind == ASSIGN){
      int change_left_early = find_earliest_changes_time(changes, temp->code->u.assign.left);
      int change_left_late = find_latest_changes_time(changes, temp->code->u.assign.left);
      int change_right_early = find_earliest_changes_time(changes, temp->code->u.assign.right);
      int change_right_late = find_latest_changes_time(changes, temp->code->u.assign.right);

      // 仅更新基本块内的临时变量
      if((change_left_early==change_left_late)
         &&(change_right_late<change_left_early)
         &&
         (((change_right_early>=current_block->block_begin)
         &&(change_right_late<=current_block->block_end))
         ||
         (temp->code->u.assign.right->kind==CONSTANT))){
        if((temp->code->u.assign.right->kind!=ADDRESS)||(no_right_star_or_address(interCodes, temp->code->u.assign.left)==1)) {
          temp->code->kind = EMPTY;
          replace_variable(interCodes, temp->code->u.assign.left, temp->code->u.assign.right);
        }
      }else if(change_left_early==change_left_late){
        if((temp->code->u.assign.right->kind!=ADDRESS)||(no_right_star_or_address(interCodes, temp->code->u.assign.left)==1)) {
          change_temp_operand(temp->next, current_code_index + 1, current_block->block_end, temp->code->u.assign.left,
                              temp->code->u.assign.right);
          if (!visit_value(interCodes, temp->code->u.assign.left)) {
            temp->code->kind = EMPTY;
          }
        }
      }
    }
    else if(temp->code->kind == RIGHT_ADDR){
      int change_left_early = find_earliest_changes_time(changes, temp->code->u.assign.left);
      int change_left_late = find_latest_changes_time(changes, temp->code->u.assign.left);

      // 仅更新基本块内的临时变量
      if((change_left_early==change_left_late)
         &&(no_right_star_or_address(interCodes, temp->code->u.assign.left)==1)){
        temp->code->kind = EMPTY;
        temp->code->u.assign.right->kind = ADDRESS;
        replace_variable(interCodes, temp->code->u.assign.left, temp->code->u.assign.right);
      }
    }
    temp = temp->next;
    current_code_index++;
    if(current_block->block_end<current_code_index){
      current_block = current_block->next;
    }
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
  InterCodes temp;
  for(int i=0; i<5; i++) {
    delete_zero(interCodes);
    printf("here\n");
    merge_labels(interCodes);
    printf("reduce_goto\n");
    reduce_goto(interCodes);
    temp = interCodes;
    while (temp != NULL) {
      if (temp->code->kind != EMPTY) {
        printf("%s\n", printCodes(temp->code));
      }else{
        printf("\n", printCodes(temp->code));
      }
      temp = temp->next;
    }
    cover_assign_temps(interCodes);
    temp = interCodes;
    while (temp != NULL) {
      if (temp->code->kind != EMPTY) {
        printf("%s\n", printCodes(temp->code));
      }else{
        printf("\n", printCodes(temp->code));
      }
      temp = temp->next;
    }
    calculate_constant(interCodes);
    printf("leaving\n");
  }
}