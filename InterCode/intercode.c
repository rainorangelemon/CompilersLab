//
// Created by rainorangelemon on 6/9/18.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "intercode.h"
#include "../Semantics/data_type.h"

int tempVariableNumber = 0;
int lableNumber = 0;

void addCode(InterCodes codes, Node newCode){
  InterCodes temp  = codes;
  while(temp->next!=NULL){
    temp = temp->next;
  }
  InterCodes newInterCode;
  if(strcmp(temp->code->name, "") == 0){
    newInterCode->code = newCode;
  }else{
    newInterCode = (InterCodes) malloc(sizeof(struct InterCodes_));
    newInterCode->prev = temp;
    newInterCode->code = newCode;
    temp->next = newInterCode;
  }
}

char* new_temp(){
  tempVariableNumber++;
  char *result = malloc(30*sizeof(char));
  sprintf(result, "t%d", tempVariableNumber);
  return result;
}

char* new_label(){
  lableNumber++;
  char *result = malloc(30*sizeof(char));
  sprintf(result, "label%d", lableNumber);
  return result;
}

int get_value(struct Node* INT){
  int size = strtol(INT->value, NULL, 0);
  return size;
}

Operand create_operand(int kind, int value, char* name){
  Operand result = (Operand)malloc(sizeof(struct Operand_));
  result->kind = kind;
  result->u.name = name;
  result->u.value = value;
  return result;
}

InterCode create_code_label(int kind, Operand label){
  InterCode result = (InterCode)malloc(sizeof(struct InterCode_));
  result->kind = kind;
  result->u.label = label;
  return result;
}

InterCode create_code_assign(int kind, Operand left, Operand right){
  InterCode result = (InterCode)malloc(sizeof(struct InterCode_));
  result->kind = kind;
  result->u.assign.left = left;
  result->u.assign.right = right;
  return result;
}

InterCode create_code_binop(int kind, Operand result, Operand op1, Operand mathop, Operand op2){
  InterCode interCode = (InterCode)malloc(sizeof(struct InterCode_));
  interCode->kind = kind;
  interCode->u.binop.result = result;
  interCode->u.binop.op1 = op1;
  interCode->u.binop.op2 = op2;
  interCode->u.binop.mathop = mathop;
  return interCode;
}

InterCode create_code_goto_con(int kind, Operand left, Operand relop, Operand right, Operand label){
  InterCode interCode = (InterCode)malloc(sizeof(struct InterCode_));
  interCode->kind = kind;
  interCode->u.goto_con.left = left;
  interCode->u.goto_con.relop = relop;
  interCode->u.goto_con.right = right;
  interCode->u.goto_con.label = label;
  return interCode;
}

char* lookup_symbols(char* name) {
  struct Symbol *symbol = find_symbol(hash_table, name, VARIABLE);
  if (symbol->name == NULL) {
    char* new_name = new_temp();
    symbol->name = new_name;
  }
  return symbol->name;
}

InterCodes translate_root(Node* tree_root) {
  char rule1[] = "Program";
  if (strcmp(tree_root->name, rule1) != 0) {
    return;
  }
  hash_table = create_table();
  struct InterCodes* head = (InterCodes) malloc(sizeof(struct InterCodes_));
  tanslate_ExtDefList(root->son, head);
  free_table(hash_table);
  return head;
}

void tanslate_ExtDefList(Node* ExtDefList, InterCodes head) {
  if (ExtDefList->son == NULL) {   // is rule 2
    // do nothing
  } else {
    translate_ExtDef(ExtDefList->son);
    tanslate_ExtDefList(ExtDefList->son->bro);
  }
}

void translate_ExtDef(Node* ExtDef, InterCodes head) {
  char rule1[] = "Specifier ExtDecList SEMI";
  char rule2[] = "Specifier SEMI";
  char rule3[] = "Specifier FunDec CompSt";
  Type specifier_type = createType_Specifier(ExtDef->son);
  if (compareSubExpression(ExtDef, rule1) == 1) {
    // do nothing, since no global variable will be used
  } else if (compareSubExpression(ExtDef, rule2) == 1) {
    // do nothing
  } else {
    translate_FunDec(ExtDef->son->bro, head);
    translate_CompSt(ExtDef->son->bro->bro, head);
  }
}

Type createType_Specifier(Node* Specifier) {
  char rule1[] = "TYPE";
  char rule2[] = "StructSpecifier";
  int isRule1 = compareSubExpression(Specifier, rule1);
  if (isRule1 == 1) {
    Type result = (Type) malloc(sizeof(struct Type_));
    memset(result, 0, sizeof(struct Type_));
    if (strcmp(Specifier->son->value, "int") == 0) {
      result->kind = BASIC;
      result->u.basic = INT;
    } else {
      result->kind = BASIC;
      result->u.basic = FLOAT;
    }
    return result;
  } else {
    return createType_StructSpecifier(Specifier->son);
  }
}

Type createType_StructSpecifier(Node* StructSpecifier) {
  char rule1[] = "STRUCT OptTag LC DefList RC";
  int isRule1 = compareSubExpression(StructSpecifier, rule1);
  if (isRule1 == 1) {
    Type result = (Type) malloc(sizeof(struct Type_));
    memset(result, 0, sizeof(struct Type_));
    Node *OptTag = StructSpecifier->son->bro;
    FieldList structure = (FieldList) malloc(sizeof(struct FieldList_));
    memset(structure, 0, sizeof(struct FieldList_));
    structure->name = NULL;
    translate_DefList(OptTag->bro->bro, structure, 1, NULL);
    result->u.structure = structure;
    result->kind = STRUCTURE;
    if (OptTag->son != NULL) {
      insert_symbol_intercodes(hash_table, OptTag->son->value, NULL, STRUCT, result, NULL);
    }
    return result;
  } else {
    Node *Tag = StructSpecifier->son->bro;
    struct Symbol temp;
    struct Symbol *structure = find_symbol_intercodes(hash_table, Tag->son->value, STRUCT);
    return structure->type;
  }
}

// Declarators

Type createType_VarDec(Node* VarDec, Type type, int insideStruct, int funcParam, FieldList fieldList, InterCodes head) {
  char rule1[] = "ID";
  int isRule1 = compareSubExpression(VarDec, rule1);
  if (isRule1 == 1) {
    if (insideStruct != 1) {
      if(funcParam==1){
        char* name = VarDec->son->value;
        insert_symbol_intercodes(hash_table, VarDec->son->value, name, VARIABLE, type, NULL);
        // add new code of PARAM
        InterCode newCode = create_code_label(PARAM, create_operand(LABEL, NULL, VarDec->son->value));
        addCode(head, newCode);
        // malloc space for symbols which are not basic types
        if(type->kind!=BASIC){
          // add new code: DEC / malloc space
          int size = calculate_size(type);
          addCode(head, create_code_assign(DEC,
                                           create_operand(VARIABLE, NULL, lookup_symbols(VarDec->son->value)),
                                           create_operand(CONSTANT, size, NULL)));
        }
      }else {
        insert_symbol_intercodes(hash_table, VarDec->son->value, NULL, VARIABLE, type, NULL);
      }
    } else {
      if (fieldList->name == NULL) {
        fieldList->name = VarDec->son->value;
        fieldList->type = type;
      } else {
        char *name = VarDec->son->value;
        // insert the new field
        FieldList temp = fieldList;
        while (temp->tail != NULL) {
          temp = temp->tail;
        }
        FieldList new_field = (FieldList) malloc(sizeof(struct FieldList_));
        memset(new_field, 0, sizeof(struct FieldList_));
        new_field->name = name;
        new_field->type = type;
        temp->tail = new_field;
      }
    }
    return type;
  } else {
    // an array
    int size = strtol(VarDec->son->bro->bro->value, NULL, 0);
    Type result = (Type) malloc(sizeof(struct Type_));
    memset(result, 0, sizeof(struct Type_));
    result->kind = ARRAY;
    result->u.array.size = size;
    result->u.array.elem = type;
    createType_VarDec(VarDec->son, result, insideStruct, funcParam, fieldList, head);
    return result;
  }
}

void translate_FunDec(struct Node* FunDec, InterCodes head) {
  // add new code of FUNCTION
  InterCode newCode = create_code_label(FUNCTION, create_operand(LABEL, NULL, FunDec->son->value));
  addCode(head, newCode);
  // check VarList
  char rule1[] = "ID LP VarList RP";
  int isRule1 = compareSubExpression(FunDec, rule1);
  if (isRule1 == 1) {
    translate_VarList(FunDec->son->bro->bro, head);
  }else{
    // do nothing
  }
}

void translate_VarList(struct Node* FunDec, InterCodes head){
  Type paramDec_type = translate_ParamDec(VarList->son, head);
  char rule1[] = "ParamDec COMMA VarList";
  int isRule1 = compareSubExpression(VarList, rule1);
  if (isRule1 == 1) {
    translate_VarList(VarList->son->bro->bro, head);
  }
}

void translate_ParamDec(struct Node* ParamDec, InterCodes head) {
  Type type = createType_Specifier(ParamDec->son);
  createType_VarDec(ParamDec->son->bro, type, 0, 1, NULL, head);
}

void translate_CompSt(struct Node* CompSt, InterCodes head){
  translate_DefList(CompSt->son->bro, NULL, 0, head);
  translate_StmtList(CompSt->son->bro->bro, head);
}

void translate_StmtList(struct Node* StmtList, InterCodes head) {
  if (StmtList->son != NULL) {
    translate_Stmt(StmtList->son, head);
    translate_StmtList(StmtList->son->bro, head);
  }
}

void translate_Stmt(struct Node* Stmt, InterCodes head) {
  char rule1[] = "Exp SEMI";
  char rule2[] = "CompSt";
  char rule3[] = "RETURN Exp SEMI";
  char rule4[] = "IF LP Exp RP Stmt";
  char rule5[] = "IF LP Exp RP Stmt ELSE Stmt";
  char rule6[] = "WHILE LP Exp RP Stmt";
  if (compareSubExpression(Stmt, rule1) == 1) {
    translate_Exp(Stmt->son, head, NULL);
  } else if (compareSubExpression(Stmt, rule2) == 1) {
    translate_CompSt(Stmt->son, head);
  } else if (compareSubExpression(Stmt, rule3) == 1) {
    char* t1 = new_temp();
    translate_Exp(Stmt->son->bro, head, t1);
    // add new code of RETURN
    InterCode newCode = create_code_label(RETURN, create_operand(VARIABLE, NULL, t1));
    addCode(head, newCode);
  } else if (compareSubExpression(Stmt, rule4) == 1) {
    char* label1 = new_label();
    char* label2 = new_label();
    translate_Cond(Stmt->son->bro->bro, label1, label2, head);
    // add new code of LABEL
    InterCode newCode = create_code_label(LABEL, create_operand(LABEL, NULL, label1));
    addCode(head, newCode);
    translate_Stmt(Stmt->son->bro->bro->bro->bro, head);
    // add new code of LABEL
    InterCode newCode = create_code_label(LABEL, create_operand(LABEL, NULL, label2));
    addCode(head, newCode);
  } else if (compareSubExpression(Stmt, rule5) == 1) {
    char* label1 = new_label();
    char* label2 = new_label();
    char* label3 = new_label();
    translate_Cond(Stmt->son->bro->bro, label1, label2, head);
    // add new code of LABEL
    addCode(head, create_code_label(LABEL, create_operand(LABEL, NULL, label1)));
    translate_Stmt(Stmt->son->bro->bro->bro->bro, head);
    // add new code of GOTO
    addCode(head, create_code_label(GOTO, create_operand(LABEL, NULL, label3)));
    // add new code of LABEL
    addCode(head, create_code_label(LABEL, create_operand(LABEL, NULL, label2)));
    translate_Stmt(Stmt->son->bro->bro->bro->bro->bro->bro, head);
    // add new code of LABEL
    addCode(head, create_code_label(LABEL, create_operand(LABEL, NULL, label3)));
  } else if (compareSubExpression(Stmt, rule6) == 1) {
    char* label1 = new_label();
    char* label2 = new_label();
    char* label3 = new_label();
    // add new code: LABEL label1
    addCode(head, create_code_label(LABEL, create_operand(LABEL, NULL, label1)));
    translate_Cond(Stmt->son->bro->bro, label2, label3, head);
    // add new code: LABEL label2
    addCode(head, create_code_label(LABEL, create_operand(LABEL, NULL, label2)));
    translate_Stmt(Stmt->son->bro->bro->bro->bro, head);
    // add new code: GOTO label1
    addCode(head, create_code_label(GOTO, create_operand(LABEL, NULL, label1)));
    // add new code: LABEL label3
    addCode(head, create_code_label(LABEL, create_operand(LABEL, NULL, label3)));
  }
}

void translate_DefList(struct Node* DefList, FieldList result, int insideStruct, InterCodes head) {
  if (DefList->son != NULL) {
    translate_Def(DefList->son, result, insideStruct, head);
    translate_DefList(DefList->son->bro, result, insideStruct, head);
  }
}

void translate_Def(struct Node* Def, FieldList result, int insideStruct, InterCodes head) {
  Type type = createType_Specifier(Def->son);
  translate_DecList(Def->son->bro, type, result, insideStruct, head);
  return result;
}

void translate_DecList(struct Node* DecList, Type type, FieldList fieldList, int insideStruct, InterCodes head) {
  translate_Dec(DecList->son, type, fieldList, insideStruct, head);
  char rule1[] = "Dec";
  if (compareSubExpression(DecList, rule1)!= 1) {
    translate_DecList(DecList->son->bro->bro, type, fieldList, insideStruct, head);
  }
}

void translate_Dec(struct Node* Dec, Type type, FieldList fieldList, int insideStruct, InterCodes head) {

  char rule1[] = "VarDec";
  if (compareSubExpression(Dec, rule1) != 1) {
    createType_VarDec(Dec->son, type, insideStruct, 0, fieldList, head);
    char* t1 = new_temp();
    translate_Exp(Dec->son->bro->bro, head, t1);
    char* place = lookup_symbols(Dec->son->son->value);
    addCode(head, create_code_assign(ASSIGN,
                                     create_operand(VARIABLE, NULL, place),
                                     create_operand(VARIABLE, NULL, t1)));
  }else{
    createType_VarDec(Dec->son, type, insideStruct, 0, fieldList, head);
  }
}

void translate_Cond(struct Node* Exp, char* label_true, char* label_false, InterCodes head) {
  char rule1[] = "Exp RELOP Exp";
  char rule2[] = "NOT Exp";
  char rule3[] = "Exp AND Exp";
  char rule4[] = "Exp OR Exp";
  if (compareSubExpression(Exp, rule1) == 1) {
    char *t1 = new_temp();
    char *t2 = new_temp();
    translate_Exp(Exp->son, head, t1);
    translate_Exp(Exp->son, head, t2);
    addCode(head, create_code_goto_con(RELOP,
                                       create_operand(VARIABLE, NULL, t1),
                                       create_operand(RELOP, NULL, Exp->son->bro->value),
                                       create_operand(VARIABLE, NULL, t2),
                                       create_operand(LABEL, NULL, label_true)));
    addCode(head, create_code_label(GOTO, create_operand(LABEL, NULL, label_false)));
  } else if (compareSubExpression(Exp, rule2) == 1) {
    translate_Cond(Exp->son->bro, label_false, label_true, head);
  } else if (compareSubExpression(Exp, rule3) == 1) {
    char *label1 = new_label();
    translate_Cond(Exp->son, label1, label_false, head);
    addCode(head, create_code_label(LABEL, create_operand(LABEL, NULL, label1)));
    translate_Cond(Exp->son->bro->bro, label_true, label_false, head);
  } else if (compareSubExpression(Exp, rule4) == 1) {
    char *label1 = new_label();
    translate_Cond(Exp->son, label_true, label1, head);
    addCode(head, create_code_label(LABEL, create_operand(LABEL, NULL, label1)));
    translate_Cond(Exp->son->bro->bro, label_true, label_false, head);
  } else {
    char *t1 = new_temp();
    translate_Exp(Exp, head, t1);
    char relop[] = "!=";
    addCode(head, create_code_goto_con(RELOP,
                                       create_operand(VARIABLE, NULL, t1),
                                       create_operand(RELOP, NULL, relop),
                                       create_operand(CONSTANT, 0, NULL),
                                       create_operand(LABEL, NULL, label_true)));
    addCode(head, create_code_label(GOTO, create_operand(LABEL, NULL, label_false)));
  }
}

Type translate_Exp(struct Node* Exp, InterCodes head, char* place) {
  char
    rule1[] = "Exp ASSIGNOP Exp", // included in this func
    rule2[] = "Exp AND Exp",
    rule3[] = "Exp OR Exp",
    rule4[] = "Exp RELOP Exp",
    rule5[] = "Exp PLUS Exp",
    rule6[] = "Exp MINUS Exp",
    rule7[] = "Exp STAR Exp",
    rule8[] = "Exp DIV Exp",
    rule9[] = "LP Exp RP",
    rule10[] = "MINUS Exp",
    rule11[] = "NOT Exp",
    rule12[] = "ID LP Args RP",
    rule13[] = "ID LP RP",
    rule14[] = "Exp LB Exp RB",
    rule15[] = "Exp DOT ID",
    rule16[] = "ID",
    rule17[] = "INT",
    rule18[] = "FLOAT"; // this will not appear
  Type result = NULL;
  if (compareSubExpression(Exp, rule17) == 1) {
    if (place != NULL) {
      addCode(head, create_code_assign(ASSIGN,
                                       create_operand(VARIABLE, NULL, place),
                                       create_operand(CONSTANT, get_value(Exp->son), NULL)));
    }
    result = (Type) malloc(sizeof(struct Type_));
    memset(result, 0, sizeof(struct Type_));
    result->kind = BASIC;
    result->u.basic = INT;
  } else if (compareSubExpression(Exp, rule16) == 1) {
    struct Symbol *symbol = find_symbol_intercodes(hash_table, Exp->son->value, VARIABLE);
    if (place != NULL) {
      if (symbol->kind == BASIC) {
        addCode(head, create_code_assign(ASSIGN,
                                         create_operand(VARIABLE, NULL, place),
                                         create_operand(VARIABLE, NULL, lookup_symbols(Exp->son->value))));
      } else {
        addCode(head, create_code_assign(RIGHT_ADDRESS,
                                         create_operand(VARIABLE, NULL, place),
                                         create_operand(VARIABLE, NULL, lookup_symbols(Exp->son->value))));
      }
    }
    result = symbol->type;
  } else if ((compareSubExpression(Exp, rule5) == 1)
             || (compareSubExpression(Exp, rule6) == 1)
             || (compareSubExpression(Exp, rule7) == 1)
             || (compareSubExpression(Exp, rule8) == 1)) {
    char *t1 = new_temp();
    char *t2 = new_temp();
    translate_Exp(Exp->son, head, t1);
    translate_Exp(Exp->son->bro->bro, head, t2);
    addCode(head, create_code_binop(MATHOP,
                                    create_operand(VARIABLE, NULL, place),
                                    create_operand(VARIABLE, NULL, t1),
                                    create_operand(MATHOP, NULL, Exp->son->bro->value),
                                    create_operand(VARIABLE, NULL, t2)));
    result = (Type) malloc(sizeof(struct Type_));
    memset(result, 0, sizeof(struct Type_));
    result->kind = BASIC;
    result->u.basic = INT;
  } else if (compareSubExpression(Exp, rule9) == 1) {
    result = translate_Exp(Exp->son->bro, head, place);
  } else if (compareSubExpression(Exp, rule10) == 1) {
    char *t1 = new_temp();
    result = translate_Exp(Exp->son->bro, head, t1);
    char mathop[] = "-";
    addCode(head, create_code_binop(MATHOP,
                                    create_operand(VARIABLE, NULL, place),
                                    create_operand(CONSTANT, 0, NULL),
                                    create_operand(MATHOP, NULL, mathop),
                                    create_operand(VARIABLE, NULL, t1)));
  } else if ((compareSubExpression(Exp, rule2) == 1)
             || (compareSubExpression(Exp, rule3) == 1)
             || (compareSubExpression(Exp, rule4) == 1)
             || (compareSubExpression(Exp, rule11) == 1)) {
    char *label1 = new_label();
    char *label2 = new_label();
    addCode(head, create_code_assign(ASSIGN,
                                     create_operand(VARIABLE, NULL, place),
                                     create_operand(CONSTANT, 0, NULL)));
    translate_Cond(Exp, label1, label2, head);
    addCode(head, create_code_label(LABEL, create_operand(LABEL, NULL, label1)));
    addCode(head, create_code_assign(ASSIGN,
                                     create_operand(VARIABLE, NULL, place),
                                     create_operand(CONSTANT, 1, NULL)));
    addCode(head, create_code_label(LABEL, create_operand(LABEL, NULL, label2)));
    result = (Type) malloc(sizeof(struct Type_));
    memset(result, 0, sizeof(struct Type_));
    result->kind = BASIC;
    result->u.basic = INT;
  } else if (compareSubExpression(Exp, rule13) == 1) {
    char read[] = "read";
    if (strcmp(Exp->son->value, read) == 0) {
      addCode(head, create_code_label(READ,
                                      create_operand(VARIABLE, NULL, place)));
    } else {
      addCode(head, create_code_assign(CALL,
                                       create_operand(VARIABLE, NULL, place),
                                       create_operand(VARIABLE, NULL, Exp->son->value)));
    }
    result = (Type) malloc(sizeof(struct Type_));
    memset(result, 0, sizeof(struct Type_));
    result->kind = BASIC;
    result->u.basic = INT;
  } else if (compareSubExpression(Exp, rule12) == 1) {
    function = lookup(sym_table, ID);
    Arg_list arg_list = (Arg_list) malloc(sizeof(struct Arg_list_));
    memset(arg_list, 0, sizeof(struct Arg_list_));
    translate_Args(Exp->son->bro->bro, sym_table, arg_list);
    if (function.name == "write") {
      addCode(head, create_code_label(WRITE,
                                      create_operand(VARIABLE, NULL, arg_list->arg)));
    } else {
      Arg_list temp = arg_list->next;
      while (temp != NULL) {
        addCode(head, create_code_label(ARG,
                                        create_operand(VARIABLE, NULL, temp->arg)));
        temp = temp->next;
      }
      addCode(head, create_code_assign(CALL,
                                       create_operand(VARIABLE, NULL, place),
                                       create_operand(VARIABLE, NULL, Exp->son->value)));
    }
    result = (Type) malloc(sizeof(struct Type_));
    memset(result, 0, sizeof(struct Type_));
    result->kind = BASIC;
    result->u.basic = INT;
  } else if (compareSubExpression(Exp, rule14) == 1) {
    char *t1 = new_temp();
    result = translate_Exp(Exp->son, head, t1); // basic
    char *t2 = new_temp();
    translate_Exp(Exp->son->bro->bro, head, t2); // offset
    result = result->u.array.elem;
    int size = calculate_space(result);
    char mathop1[] = "*";
    addCode(head, create_code_binop(MATHOP,
                                    create_operand(VARIABLE, NULL, place),
                                    create_operand(CONSTANT, size, NULL),
                                    create_operand(MATHOP, NULL, mathop1),
                                    create_operand(VARIABLE, NULL, t2)));
    char mathop2[] = "+";
    addCode(head, create_code_binop(MATHOP,
                                    create_operand(VARIABLE, NULL, place),
                                    create_operand(VARIABLE, NULL, place),
                                    create_operand(MATHOP, NULL, mathop2),
                                    create_operand(VARIABLE, NULL, t1)));
    if (result->kind == BASIC) {
      addCode(head, create_code_assign(RIGHT_STAR,
                                       create_operand(VARIABLE, NULL, place),
                                       create_operand(VARIABLE, NULL, place)));
    }
  } else if (compareSubExpression(Exp, rule15) == 1) {
    char *t1 = new_temp();
    result = translate_Exp(Exp->son, head, t1);
    int size = calculate_offset_in_struct(result->u.structure, Exp->son->bro->bro->value);
    result = get_type_in_struct(result->u.structure, Exp->son->bro->bro->value);
    char mathop[] = "+";
    addCode(head, create_code_binop(MATHOP,
                                    create_operand(VARIABLE, NULL, place),
                                    create_operand(CONSTANT, size, NULL),
                                    create_operand(MATHOP, NULL, mathop),
                                    create_operand(VARIABLE, NULL, t1)));
    if (result->kind == BASIC) {
      addCode(head, create_code_assign(RIGHT_STAR,
                                       create_operand(VARIABLE, NULL, place),
                                       create_operand(VARIABLE, NULL, place)));
    }
  } else if (compareSubExpression(Exp, rule1) == 1) {
    struct Node *Exp1 = Exp->son;
    if (compareSubExpression(Exp1, rule16) == 1) {  // ID
      char *variable = lookup_symbols(Exp1->son->value);
      char *t1 = new_temp();
      result = translate_Exp(Exp1->bro->bro, head, t1);
      if (result->kind == BASIC) {
        addCode(head, create_code_assign(ASSIGN,
                                         create_operand(VARIABLE, NULL, variable),
                                         create_operand(VARIABLE, NULL, t1)));
        addCode(head, create_code_assign(ASSIGN,
                                         create_operand(VARIABLE, NULL, place),
                                         create_operand(VARIABLE, NULL, variable)));
      } else {
        char *t2 = new_temp();
        addCode(head, create_code_assign(RIGHT_STAR,
                                         create_operand(VARIABLE, NULL, t2),
                                         create_operand(VARIABLE, NULL, t1)));
        addCode(head, create_code_assign(LEFT_STAR,
                                         create_operand(VARIABLE, NULL, variable),
                                         create_operand(VARIABLE, NULL, t2)));
        addCode(head, create_code_assign(ASSIGN,
                                         create_operand(VARIABLE, NULL, place),
                                         create_operand(VARIABLE, NULL, t1)));
      }
    } else if (compareSubExpression(Exp1, rule14) == 1) {  // Exp LB Exp RB
      char *t1 = new_temp();
      result = translate_Exp(Exp1->son, head, t1); // basic
      char *t2 = new_temp();
      translate_Exp(Exp1->son->bro->bro, head, t2); // offset
      result = result->u.array.elem;
      int size = calculate_space(result);
      char mathop1[] = "*";
      addCode(head, create_code_binop(MATHOP,
                                      create_operand(VARIABLE, NULL, place),
                                      create_operand(CONSTANT, size, NULL),
                                      create_operand(MATHOP, NULL, mathop1),
                                      create_operand(VARIABLE, NULL, t2)));
      char mathop2[] = "+";
      addCode(head, create_code_binop(MATHOP,
                                      create_operand(VARIABLE, NULL, place),
                                      create_operand(VARIABLE, NULL, place),
                                      create_operand(MATHOP, NULL, mathop2),
                                      create_operand(VARIABLE, NULL, t1)));
      char *t3 = new_temp();
      result = translate_Exp(Exp1->bro->bro, head, t3);
      if (result->kind == BASIC) {
        addCode(head, create_code_assign(LEFT_STAR,
                                         create_operand(VARIABLE, NULL, place),
                                         create_operand(VARIABLE, NULL, t3)));
        addCode(head, create_code_assign(ASSIGN,
                                         create_operand(VARIABLE, NULL, place),
                                         create_operand(VARIABLE, NULL, t3)));
      } else {
        char *t4 = new_temp();
        addCode(head, create_code_assign(RIGHT_STAR,
                                         create_operand(VARIABLE, NULL, t4),
                                         create_operand(VARIABLE, NULL, t3)));
        addCode(head, create_code_assign(LEFT_STAR,
                                         create_operand(VARIABLE, NULL, place),
                                         create_operand(VARIABLE, NULL, t4)));
      }
    } else { //  Exp DOT ID
      char *t1 = new_temp();
      result = translate_Exp(Exp1->son, head, t1);
      int size = calculate_offset_in_struct(result->u.structure, Exp1->son->bro->bro->value);
      result = get_type_in_struct(result->u.structure, Exp1->son->bro->bro->value);
      char mathop[] = "+";
      addCode(head, create_code_binop(MATHOP,
                                      create_operand(VARIABLE, NULL, place),
                                      create_operand(CONSTANT, size, NULL),
                                      create_operand(MATHOP, NULL, mathop),
                                      create_operand(VARIABLE, NULL, t1)));
      char *t3 = new_temp();
      result = translate_Exp(Exp1->bro->bro, head, t3);
      if (result->kind == BASIC) {
        addCode(head, create_code_assign(LEFT_STAR,
                                         create_operand(VARIABLE, NULL, place),
                                         create_operand(VARIABLE, NULL, t3)));
        addCode(head, create_code_assign(ASSIGN,
                                         create_operand(VARIABLE, NULL, place),
                                         create_operand(VARIABLE, NULL, t3)));
      } else {
        char *t4 = new_temp();
        addCode(head, create_code_assign(RIGHT_STAR,
                                         create_operand(VARIABLE, NULL, t4),
                                         create_operand(VARIABLE, NULL, t3)));
        addCode(head, create_code_assign(LEFT_STAR,
                                         create_operand(VARIABLE, NULL, place),
                                         create_operand(VARIABLE, NULL, t4)));
      }
    }
  }
  return result;
}

void translate_Args(struct Node* Args, InterCodes head, Arg_list arg_list) {
  char rule2[] = "Exp COMMA Args";
  if (compareSubExpression(Args, rule2) != 1) {
    char *t1 = new_temp();
    translate_Exp(Exp, head, t1);
    Arg_list newArgs = (Args_list) malloc(sizeof(struct Args_list_));
    newArgs->next = arg_list->next;
    arg_list->next = newArgs;
    newArgs->arg = t1;
  } else {
    char *t1 = new_temp();
    translate_Exp(Exp, head, t1);
    Arg_list newArgs = (Args_list) malloc(sizeof(struct Args_list_));
    newArgs->next = arg_list->next;
    arg_list->next = newArgs;
    newArgs->arg = t1;
    translate_Args(Args->son->bro->bro, head, arg_list);
  }
}

int calculate_space(Type type) {
  if (type->kind == BASIC) {
    if (type->u.basic == INT) {
      return 4;
    } else if (type->u.basic == FLOAT) {
      return 8;
    }
  } else if (type->kind == ARRAY) {
    return (type->u.array.size * calculate_space(type->u.array.elem));
  } else if (type->kind == STRUCTURE) {
    int size = 0;
    FieldList temp = type->u.structure;
    while ((temp != NULL) && (temp->name != NULL) && (temp->type != NULL)) {
      size += calculate_space(temp->type);
      temp = temp->tail;
    }
    return size;
  }
}

int calculate_offset_in_struct(FieldList fieldList, char* name) {
  int size = 0;
  FieldList temp = type->u.structure;
  while ((temp != NULL) && (temp->name != NULL) && (temp->type != NULL)) {
    if (strcmp(name, temp->name) != 0) {
      size += calculate_space(temp->type);
      temp = temp->tail;
    } else {
      break;
    }
  }
  return size;
}

Type get_type_in_struct(FieldList fieldList, char* name) {
  FieldList temp = type->u.structure;
  while ((temp != NULL) && (temp->name != NULL) && (temp->type != NULL)) {
    if (strcmp(name, temp->name) != 0) {
      temp = temp->tail;
    } else {
      return temp->type;
    }
  }
  return NULL;
}