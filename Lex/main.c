#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "../Semantics/data_type.h"

extern int yylineno;
extern Node* root;
extern int errorFlag;

struct Hash_table* hash_table;

int main(int argc, char* argv[])
{
  if(argc<=1) return 1;
  FILE* f =fopen(argv[1], "r");
  if (!f)
  {
    perror(argv[1]);
    return 1;
  }
  root = NULL;
  yylineno = 1;
  yyrestart(f);
  yyparse();
  check_error(root);
  if((errorFlag == 0)||((argc>=3)&&(strcmp(argv[2], "-debug")==0))){
	  printTree(root, 0);
  }
  deleteTree(root);
  return 0;
}
