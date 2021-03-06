#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "../Semantics/data_type.h"
#include "../InterCode/intercode.h"

extern int yylineno;
extern Node* root;
extern int errorFlag;
extern int error_line;

void yyrestart ( FILE *input_file  );
int yyparse (void);

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
  if((errorFlag == 0)||((argc>=3)&&(strcmp(argv[2], "-debug")==0))){
//    printTree(root, 0);
    check_error(root);
  }
  if(((errorFlag==0)&&(error_line==-1))||((argc>=3)&&(strcmp(argv[2], "-debug")==0))){
    if(argc>=3) {
      translate_root(root, argv[2]);
    }else{
      printTree(root, 0);
    }
  }
  if((argc>=3)&&(strcmp(argv[2], "-debug")==0)){
    printTree(root, 0);
  }
  deleteTree(root);
  return 0;
}
