%{
  #include<stdio.h>
%}

%token INT
%%
Term: INT
  ;
%%
#include "../Lex/lex.yy.c"
yyerror(char* msg){
}
