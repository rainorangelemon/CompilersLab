#include <stdio.h>
#include <stdlib.h>

extern FILE* yyin;

int main(int argc, char* argv[])
{
  //if(argc<=1) return 1;
  if (!(yyin=fopen(argv[1], "r")))
  {
    perror(argv[1]);
    return 1;
  }
  yylex();
  return 0;
}
