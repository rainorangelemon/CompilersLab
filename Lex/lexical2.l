%{
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
{ERROR_COMMENT} {errorFlag=1; printf("Error type A at line %d: Illegal comments.\n",yylineno);}
{LINE_COMMENT} {;}
{BLOCK_COMMENT} {;}
{SPACES} {;}
\n {;}
{FLOAT} { printf("%s, FLOAT\n", yytext);}
{FLOAT_ERR} { printf("%s, Wrong FLOAT\n", yytext); errorFlag=1; printf("Error type A at line %d: Illegal floating number \"%s\".\n",yylineno, yytext);}
{INT} {  printf("%s, INT\n", yytext);}
{INT8_ERR} { printf("%s, Wrong INT8\n", yytext); errorFlag=1; printf("Error type A at line %d: Illegal octal number \"%s\".\n",yylineno, yytext);}
{INT16_ERR} { printf("%s, Wrong INT16\n", yytext); errorFlag=1; printf("Error type A at line %d: Illegal hexadecimal number \"%s\".\n",yylineno, yytext);}
{SEMI} {  printf("%s, SEMI\n", yytext);}
{COMMA} {  printf("%s, COMMA\n", yytext);}
{ASSIGNOP} {  printf("%s, ASSIGNOP\n", yytext);}
{RELOP} {  printf("%s, RELOP\n", yytext);}
{PLUS} {  printf("%s, PLUS\n", yytext);}
{MINUS} {  printf("%s, MINUS\n", yytext);}
{STAR} {  printf("%s, STAR\n", yytext);}
{DIV} {  printf("%s, DIV\n", yytext);}
{AND} {  printf("%s, AND\n", yytext);}
{OR} {  printf("%s, OR\n", yytext);}
{DOT} {  printf("%s, DOT\n", yytext);}
{NOT} {  printf("%s, NOT\n", yytext);}
{TYPE} {  printf("%s, TYPE\n", yytext);}
{LP} {  printf("%s, LP\n", yytext);}
{RP} {  printf("%s, RP\n", yytext);}
{LB} {  printf("%s, LB\n", yytext);}
{RB} {  printf("%s, RB\n", yytext);}
{LC} {  printf("%s, LC\n", yytext);}
{RC} {  printf("%s, RC\n", yytext);}
{STRUCT} {  printf("%s, STRUCT\n", yytext); } 
{RETURN} {  printf("%s, RETURN\n", yytext); } 
{IF} {  printf("%s, IF\n", yytext);} 
{ELSE} {  printf("%s, ELSE\n", yytext);} 
{WHILE} {  printf("%s, WHILE\n", yytext);}
{ID} {  printf("%s, ID\n", yytext);}
. {errorFlag=1; printf("Error type A at line %d: Mysterious character '%s'\n",yylineno,yytext);}
%%
int main(int argc, char** argv){
  yyin = fopen(argv[1], "r");
  yylex();
  return 0;
}
