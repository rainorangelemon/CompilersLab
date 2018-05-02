%{
  #include "tree.h"
  #include "../Syntax/syntax.tab.h"
  int errorFlag = 0;
%}

%option yylineno

INT10 ([1-9][0-9]*)|0
INT8 0[0-7]*
INT8_ERR 0([0-7]*[8-9]+)*
INT16 ("0x"|"0X")[0-9A-Fa-f]+
INT16_ERR ("0x"|"0X")([0-9A-Fa-f]*[g-zG-Z]+)*
INT {INT10}|{INT8}|{INT16}

FLOAT1 [0-9]+"."[0-9]+
FLOAT1_ERR ({INT}".")|("."{INT})
FLOAT2 (([0-9]+"."[0-9]*)|([0-9]*"."[0-9]+))[Ee][+-]?[0-9]+
FLOAT2_ERR [0-9]*"."[0-9]*[Ee][+-]?[0-9]*
FLOAT {FLOAT1}|{FLOAT2}
FLOAT_ERR {FLOAT1_ERR}|{FLOAT2_ERR}

ID [a-zA-Z_][0-9a-zA-Z_]*

SPACES [ \t]+

LINE_COMMENT "//"[^\n]*
BLOCK_COMMENT "/*"(([^\*]*(\*[^\/])?)*)"*/"
ERROR_COMMENT "/*"(([^\*]*(\*[^\/])?)*)({BLOCK_COMMENT}(([^\*]*(\*[^\/])?)*))+"*/"

SEMI ";"
COMMA ","
ASSIGNOP "="
RELOP ">"|"<"|">="|"<="|"=="|"!="
PLUS "+"
MINUS "-"
STAR "*"
DIV "/"
AND "&&"
OR "||"
DOT "."
NOT "!"
TYPE "int"|"float"
LP "("
RP ")"
LB "["
RB "]"
LC "{"
RC "}"
STRUCT "struct"
RETURN "return"
IF "if"
ELSE "else"
WHILE "while"


%%
{ERROR_COMMENT}	{errorFlag=1; printf("Error type A at line %d: Illegal comments.\n",yylineno);}
{LINE_COMMENT}	{;}
{BLOCK_COMMENT}	{;}
{SPACES}	{;}
\n		{;}
{FLOAT}		{yylval.node=createNode("FLOAT", yytext); return FLOAT;}
{FLOAT_ERR}	{yylval.node=createNode("Wrong FLOAT", yytext); errorFlag=1; printf("Error type A at line %d: Illegal floating number \"%s\".\n",yylineno, yytext); return FLOAT;}
{INT}		{yylval.node=createNode("INT", yytext); return INT;}
{INT8_ERR}	{yylval.node=createNode("Wrong INT8", yytext); errorFlag=1; printf("Error type A at line %d: Illegal octal number \"%s\".\n",yylineno, yytext); return INT;}
{INT16_ERR}	{yylval.node=createNode("Wrong INT16", yytext); errorFlag=1; printf("Error type A at line %d: Illegal hexadecimal number \"%s\".\n",yylineno, yytext); return INT;}
{SEMI}		{ yylval.node=createNode("SEMI", yytext); return SEMI;}
{COMMA}		{ yylval.node=createNode("COMMA", yytext); return COMMA;}
{ASSIGNOP}	{ yylval.node=createNode("ASSIGNOP", yytext); return ASSIGNOP;}
{RELOP}		{ yylval.node=createNode("RELOP", yytext); return RELOP;} 
{PLUS}		{yylval.node=createNode("PLUS", yytext); return PLUS;} 
{MINUS}		{ yylval.node=createNode("MINUS", yytext); return MINUS;} 
{STAR} 		{ yylval.node=createNode("STAR", yytext); return STAR;} 
{DIV}		{ yylval.node=createNode("DIV", yytext); return DIV;} 
{AND}		{ yylval.node=createNode("AND", yytext); return AND;} 
{OR} 		{ yylval.node=createNode("OR", yytext); return OR;} 
{DOT}		{yylval.node=createNode("DOT", yytext); return DOT;} 
{NOT}		{yylval.node=createNode("NOT", yytext); return NOT;} 
{TYPE}		{yylval.node=createNode("TYPE", yytext); return TYPE;} 
{LP}		{yylval.node=createNode("LP", yytext); return LP;} 
{RP}		{yylval.node=createNode("RP", yytext); return RP;} 
{LB}		{yylval.node=createNode("LB", yytext); return LB;} 
{RB}		{yylval.node=createNode("RB", yytext); return RB;} 
{LC}		{yylval.node=createNode("LC", yytext); return LC;} 
{RC} 		{yylval.node=createNode("RC", yytext); return RC;} 
{STRUCT} 	{yylval.node=createNode("STRUCT", yytext); return STRUCT;} 
{RETURN} 	{yylval.node=createNode("RETURN", yytext); return RETURN;} 
{IF} 		{yylval.node=createNode("IF", yytext); return IF;} 
{ELSE} 		{yylval.node=createNode("ELSE", yytext); return ELSE;} 
{WHILE} 	{ yylval.node=createNode("WHILE", yytext); return WHILE;} 
{ID}		{yylval.node=createNode("ID", yytext); return ID;} 
. 		{errorFlag=1; printf("Error type A at line %d: Mysterious character '%s'\n",yylineno,yytext);}
%%
