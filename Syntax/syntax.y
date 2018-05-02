%{
  #include <stdio.h>
  #include "../Lex/tree.h"
  #include "../Lex/lex.yy.c"
  Node* root;
  int yylex();
  char missingChar='?';
%}

%union {
    Node* node;
};

%token <node> INT FLOAT ID SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR DOT NOT TYPE
%token <node> LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE

%type <node> PROGRAM ExtDefList ExtDef ExtDecList
%type <node> Specifier StructSpecifier OptTag Tag 
%type <node> VarDec FunDec VarList ParamDec
%type <node> CompSt StmtList Stmt 
%type <node> DefList Def DecList Dec
%type <node> Exp Args

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
	| /*empty*/				{$$=NULL;}
	;
ExtDef : Specifier ExtDecList SEMI		{$$=createNode("ExtDef", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| Specifier SEMI			{$$=createNode("ExtDef", ""); addSon($$, $1); addSon($$, $2);}
	| Specifier FunDec CompSt		{$$=createNode("ExtDef", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	;
ExtDecList : VarDec				{$$=createNode("ExtDecList", ""); addSon($$, $1);}
	| VarDec COMMA ExtDecList		{$$=createNode("ExtDecList", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	;

/*Specifiers*/
Specifier : TYPE				{$$=createNode("Specifier", ""); addSon($$, $1);}
	| StructSpecifier			{$$=createNode("Specifier", ""); addSon($$, $1);}
	;
StructSpecifier : STRUCT OptTag LC DefList RC	{$$=createNode("StructSpecifier", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3); addSon($$, $4); addSon($$, $5);}
	| STRUCT Tag				{$$=createNode("StructSpecifier", ""); addSon($$, $1); addSon($$, $2);}
	;
OptTag : ID					{$$=createNode("OptTag", ""); addSon($$, $1);}
	| /*empty*/				{$$=NULL;}
	;
Tag : ID					{$$=createNode("Tag", ""); addSon($$, $1);}
	;

/*Declarators*/
VarDec : ID					{$$=createNode("VarDec", ""); addSon($$, $1);}
	| VarDec LB INT RB			{$$=createNode("VarDec", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3); addSon($$, $4);}
	;
FunDec : ID LP VarList RP			{$$=createNode("FunDec", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3); addSon($$, $4);}
	| ID LP RP				{$$=createNode("FunDec", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	;
VarList : ParamDec COMMA VarList		{$$=createNode("VarList", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| ParamDec				{$$=createNode("VarList", ""); addSon($$, $1);}
	;
ParamDec : Specifier VarDec			{$$=createNode("ParamDec", ""); addSon($$, $1); addSon($$, $2);}
	;

/*Statements*/
CompSt : LC DefList StmtList RC			{$$=createNode("CompSt", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3); addSon($$, $4);}
	;
StmtList : Stmt StmtList			{$$=createNode("StmtList", ""); addSon($$, $1); addSon($$, $2);}
	| /*empty*/				{$$=NULL;}
	;
Stmt : Exp SEMI					{$$=createNode("Stmt", ""); addSon($$, $1); addSon($$, $2);}
	| CompSt				{$$=createNode("Stmt", ""); addSon($$, $1);}
	| RETURN Exp SEMI			{$$=createNode("Stmt", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| IF LP Exp RP Stmt %prec LOWER_THAN_ELSE	{$$=createNode("Stmt", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3); addSon($$, $4); addSon($$, $5);}
	| IF LP Exp RP Stmt ELSE Stmt		{$$=createNode("Stmt", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3); addSon($$, $4); addSon($$, $5); addSon($$, $6); addSon($$, $7);}
	| WHILE LP Exp RP Stmt			{$$=createNode("Stmt", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3); addSon($$, $4); addSon($$, $5);}
	| error SEMI				{printf("Here is an error!\n"); errorFlag=1;}
	;

/*Local Definitions*/
DefList : Def DefList				{$$=createNode("DefList", ""); addSon($$, $1); addSon($$, $2);}
	| /*empty*/				{$$=NULL;}
	;
Def : Specifier DecList SEMI			{$$=createNode("Def", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
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
	;
Args : Exp COMMA Args 				{$$=createNode("Args", ""); addSon($$, $1); addSon($$, $2); addSon($$, $3);}
	| Exp					{$$=createNode("Args", ""); addSon($$, $1);}
	;

%%
yyerror(char* msg){
	fprintf(stderr,"Error type B at line %d: %s.  (unexpected near '%s', Missing %c)\n", yylineno, msg, ((Node*)yylval.node)->value, missingChar);
	missingChar = '?';
}
