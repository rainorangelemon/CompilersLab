%{
%}

%option yylineno

INT10 ([1-9][0-9]*)|0
INT8 0[0-7]*
INT16 ("0x"|"0X")[0-9A-Fa-f]+
INT {INT10}|{INT8}|{INT16}

FLOAT1 [0-9]+.[0-9]+
FLOAT2 (([0-9]+"."[0-9]*)|([0-9]*"."[0-9]+))[Ee][+-]?[0-9]+
FLOAT {FLOAT1}|{FLOAT2}

ID [a-zA-Z_][0-9a-zA-Z_]*

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
\n {;}
{INT} { printf("INT: %d %s\n", yylineno, yytext); }
{FLOAT} { printf("FLOAT: %d %s\n", yylineno, yytext); }
{SEMI} { printf("SEMI: %d %s\n", yylineno, yytext); }
{COMMA} { printf("COMMA: %d %s\n", yylineno, yytext); }
{ASSIGNOP} { printf("ASSIGNOP: %d %s\n", yylineno, yytext); }
{RELOP} { printf("ID: %d %s\n", yylineno, yytext); } 
{PLUS} { printf("PLUS: %d %s\n", yylineno, yytext); }
{MINUS} { printf("MINUS: %d %s\n", yylineno, yytext); }
{STAR} { printf("STAR: %d %s\n", yylineno, yytext); }
{DIV} { printf("DIV: %d %s\n", yylineno, yytext); }
{AND} { printf("AND: %d %s\n", yylineno, yytext); }
{OR} { printf("OR: %d %s\n", yylineno, yytext); }
{DOT} { printf("DOT: %d %s\n", yylineno, yytext); }
{NOT} { printf("NOT: %d %s\n", yylineno, yytext); }
{TYPE} { printf("TYPE: %d %s\n", yylineno, yytext); }
{LP} { printf("LP: %d %s\n", yylineno, yytext); }
{RP} { printf("RP: %d %s\n", yylineno, yytext); }
{LB} { printf("LB: %d %s\n", yylineno, yytext); }
{RB} { printf("RB: %d %s\n", yylineno, yytext); }
{LC} { printf("LC: %d %s\n", yylineno, yytext); }
{RC} { printf("RC: %d %s\n", yylineno, yytext); }
{STRUCT} { printf("STRUCT: %d %s\n", yylineno, yytext); }
{RETURN} { printf("RETURN: %d %s\n", yylineno, yytext); }
{IF} { printf("IF: %d %s\n", yylineno, yytext); }
{ELSE} { printf("ELSE: %d %s\n", yylineno, yytext); }
{WHILE} { printf("WHILE: %d %s\n", yylineno, yytext); }
{ID} { printf("ID: %d %s\n", yylineno, yytext); } 
. {printf("Error type A at line %d: Mysterious character '%s'\n",yylineno,yytext);}
%%
