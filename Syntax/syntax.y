%{
  #include <stdio.h>
  #include <string.h>
  #include "../Lex/tree.h"
  #include "../Lex/lex.yy.c"
  Node* root;
  int yylex();
  char hint[100]="";
  yydebug = 1;
%}

%union {
    Node* node;
};

%token <node> INT FLOAT ID SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR DOT NOT TYPE
%token <node> LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE

/*
%type <node> PROGRAM ExtDefList ExtDef ExtDecList
%type <node> Specifier StructSpecifier OptTag Tag 
%type <node> VarDec FunDec VarList ParamDec
%type <node> CompSt StmtList Stmt 
%type <node> DefList Def DecList Dec
%type <node> Exp Args
*/

%type <node> PROGRAM ExtDefList ExtDef ExtDecList Specifier
%type <node> StructSpecifier OptTag Tag VarDec FunDec VarList
%type <node> ParamDec CompSt StmtList Stmt DefList Def DecList
%type <node> Dec Exp Args

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left DOT LB RB LP RP

%nonassoc LOWER_THAN_ELSE /*for modifying the priority of ELSE*/
%nonassoc ELSE

%%
/* High-level Definitions*/
PROGRAM : ExtDefList 				{$$=createNode("Program", ""); addSon($$, $1); root=$$;}
	;
ExtDefList : ExtDef ExtDefList			{$$=createNode("ExtDefList", ""); addSon($$, $1); addSon($$, $2);}
	| /*empty*/				{$$=createNode("ExtDefList", "");}
	;
ExtDef : Specifier ExtDecList SEMI		{$$=createNode("ExtDef", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| Specifier SEMI			{$$=createNode("ExtDef", ""); addSon($$, $1); addSon($$, $2);}
	| Specifier FunDec CompSt		{$$=createNode("ExtDef", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| Specifier error SEMI			{$$=createNode("ExtDef", ""); errorFlag=1; strcpy(hint, "Wrong Dec BETWEEN Specifier and SEMI."); printf(" %s\n", hint);}
	;
ExtDecList : VarDec				{$$=createNode("ExtDecList", ""); addSon($$, $1);}
	| VarDec COMMA ExtDecList		{$$=createNode("ExtDecList", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| error COMMA ExtDecList		{$$=createNode("ExtDecList", ""); errorFlag=1; strcpy(hint, "Wrong VarDec before COMMA ExtDecList"); printf(" %s\n", hint);}
	;

/*Specifiers*/
Specifier : TYPE				{$$=createNode("Specifier", ""); addSon($$, $1);}
	| StructSpecifier			{$$=createNode("Specifier", ""); addSon($$, $1);}
	;
StructSpecifier : STRUCT OptTag LC DefList RC	{$$=createNode("StructSpecifier", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3); addSon($$, $4); addSon($$, $5);}
//	| STRUCT OptTag LC error RC		{$$=createNode("StructSpecifier", "");  strcpy(hint, "Wrong DefList between LC and RC"); addSon($$, $1); addSon($$, $2); addSon($$, $3); addSon($$, createNode("DefList", "")); addSon($$, $5); errorFlag=1; printf(" %s\n", hint);}
	| STRUCT Tag				{$$=createNode("StructSpecifier", ""); addSon($$, $1); addSon($$, $2);}
	;
OptTag : ID					{$$=createNode("OptTag", ""); addSon($$, $1);}
	| /*empty*/				{$$=createNode("OptTag", "");}
	;
Tag : ID					{$$=createNode("Tag", ""); addSon($$, $1);}
	;

/*Declarators*/
VarDec : ID					{$$=createNode("VarDec", ""); addSon($$, $1);}
	| VarDec LB INT RB			{$$=createNode("VarDec", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3); addSon($$, $4);}
	| VarDec LB error RB			{$$=createNode("VarDec", ""); errorFlag=1; strcpy(hint, "Wrong INT between [ and ]."); printf(" %s\n", hint);}
	;
FunDec : ID LP VarList RP			{$$=createNode("FunDec", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3); addSon($$, $4);}
	| ID LP RP				{$$=createNode("FunDec", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| ID LP error RP			{strcpy(hint, "Wrong VarList between ID ( and )."); $$=createNode("FunDec", ""); addSon($$, $1); addSon($$, $2); addSon($$, $4); errorFlag=1; printf(" %s\n", hint);}
	| ID error RP				{strcpy(hint, "Wrong VarList between ID and )."); $$=createNode("FunDec", ""); addSon($$, $1); addSon($$, createNode("LP", "")); addSon($$, $3); errorFlag=1; printf(" %s\n", hint);}
	;
VarList : ParamDec COMMA VarList		{$$=createNode("VarList", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| ParamDec				{$$=createNode("VarList", ""); addSon($$, $1);}
	| error COMMA VarList			{$$=createNode("VarList", ""); errorFlag=1; strcpy(hint, "Wrong ParamDec before COMMA"); printf(" %s\n", hint);}
	;
ParamDec : Specifier VarDec			{$$=createNode("ParamDec", ""); addSon($$, $1); addSon($$, $2);}
	;

/*Statements*/
CompSt : LC DefList StmtList RC			{$$=createNode("CompSt", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3); addSon($$, $4);}
	| LC DefList error RC			{$$=createNode("CompSt", ""); strcpy(hint, "Wrong StmtList between DefList and }."); addSon($$, $1); addSon($$, $2); addSon($$, createNode("StmtList", "")); addSon($$, $4); errorFlag=1; printf(" %s\n", hint);}
	| LC error StmtList RC			{$$=createNode("CompSt", ""); strcpy(hint, "Wrong StmtList between DefList and }."); addSon($$, $1); addSon($$, createNode("DefList", "")); addSon($$, $3); addSon($$, $4); errorFlag=1; printf(" %s\n", hint);}
	;
StmtList : Stmt StmtList			{$$=createNode("StmtList", ""); addSon($$, $1); addSon($$, $2);}
	| /*empty*/				{$$=createNode("StmtList", "");}
	;
Stmt : Exp SEMI					{$$=createNode("Stmt", ""); addSon($$, $1); addSon($$, $2);}
	| CompSt				{$$=createNode("Stmt", ""); addSon($$, $1);}
	| RETURN Exp SEMI			{$$=createNode("Stmt", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| IF LP Exp RP Stmt %prec LOWER_THAN_ELSE	{$$=createNode("Stmt", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3); addSon($$, $4); addSon($$, $5);}
	| IF LP error SEMI			{$$=createNode("Stmt", ""); errorFlag=1; strcpy(hint, "Wrong Exp after if (."); printf(" %s\n", hint);}
	| IF LP Exp RP Stmt ELSE Stmt		{$$=createNode("Stmt", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3); addSon($$, $4); addSon($$, $5); addSon($$, $6); addSon($$, $7);}
	| IF LP Exp RP error ELSE Stmt		{$$=createNode("Stmt", ""); errorFlag=1; strcpy(hint, "Wrong Stmt between ) and ELSE."); printf(" %s\n", hint);}
	| WHILE LP Exp RP Stmt			{$$=createNode("Stmt", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3); addSon($$, $4); addSon($$, $5);}
	| WHILE LP error RP Stmt		{$$=createNode("Stmt", ""); errorFlag=1; strcpy(hint, "Wrong Exp between while ( and )."); printf(" %s\n", hint);}
	| WHILE error SEMI			{$$=createNode("Stmt", ""); errorFlag=1; strcpy(hint, "Wrong Stmt between while and SEMI."); printf(" %s\n", hint);}
	| error SEMI				{errorFlag=1; strcpy(hint, "Wrong Stmt."); printf(" %s\n", hint);}	;

/*Local Definitions*/
DefList : Def DefList				{$$=createNode("DefList", ""); addSon($$, $1); addSon($$, $2);}
	| /*empty*/				{$$=createNode("DefList", "");}
	;
Def : Specifier DecList SEMI			{$$=createNode("Def", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| Specifier error SEMI			{$$=createNode("Def", ""); errorFlag=1; printf(" %s\n", hint);}
	;
DecList : Dec					{$$=createNode("DecList", ""); addSon($$, $1);}
	| Dec COMMA DecList			{$$=createNode("DecList", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	;
Dec : VarDec					{$$=createNode("Dec", ""); addSon($$, $1);}
	| VarDec ASSIGNOP Exp			{$$=createNode("Dec", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	;

/*Expressions*/
Exp : Exp ASSIGNOP Exp 				{$$=createNode("Exp", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| Exp AND Exp				{$$=createNode("Exp", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| Exp OR Exp				{$$=createNode("Exp", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| Exp RELOP Exp				{$$=createNode("Exp", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| Exp PLUS Exp				{$$=createNode("Exp", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| Exp MINUS Exp				{$$=createNode("Exp", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| Exp STAR Exp				{$$=createNode("Exp", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| Exp DIV Exp				{$$=createNode("Exp", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| LP Exp RP				{$$=createNode("Exp", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| MINUS Exp				{$$=createNode("Exp", ""); addSon($$, $1); addSon($$, $2);}
	| NOT Exp				{$$=createNode("Exp", ""); addSon($$, $1); addSon($$, $2);}
	| ID LP Args RP				{$$=createNode("Exp", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3); addSon($$, $4);}
	| ID LP RP				{$$=createNode("Exp", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| Exp LB Exp RB				{$$=createNode("Exp", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3); addSon($$, $4);}
	| Exp DOT ID				{$$=createNode("Exp", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| ID					{$$=createNode("Exp", ""); addSon($$, $1);}
	| INT					{$$=createNode("Exp", ""); addSon($$, $1);}
	| FLOAT					{$$=createNode("Exp", ""); addSon($$, $1);}
	| LP error RP				{$$=createNode("Exp", ""); errorFlag=1; strcpy(hint, "Wrong Exp between ( and ).");printf(" %s\n", hint);}
	| Exp LB error RB			{$$=createNode("Exp", ""); errorFlag=1; strcpy(hint, "Wrong Exp between [ and ].");printf(" %s\n", hint);}
	;
Args : Exp COMMA Args 				{$$=createNode("Args", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| Exp					{$$=createNode("Args", ""); addSon($$, $1);}
	;


%%
yyerror(char* msg){
	strcpy(hint, " ");
	fprintf(stderr,"Error type B at line %d: %s. Unexpected near '%s', ", yylineno, msg, ((Node*)yylval.node)->value);

}
