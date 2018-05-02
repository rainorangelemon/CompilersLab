#include <stdio.h>
#include <stdlib.h>
#include "tree.h"

extern int yylineno;
extern Node* root;
extern int errorFlag;

Node* createNode(char* name, char* value){
//    printf("%s %s\n", name, value);
    Node* result = (Node*)malloc(sizeof(Node));
    result -> lineno = yylineno;
    strcpy(result->name, name);
    strcpy(result->value, value);
    result -> son = NULL;
    result -> bro = NULL;
    return result;
}

/*正序插入*/
void addSon(Node* father, Node* son){
    if(father!=NULL && son!=NULL){
        if(father->son!=NULL){
            Node* p = father->son;
            while(p->bro!=NULL){
                p = p->bro;
            }
            p->bro = son;
        }else{
            father->son = son;
        }
	father->lineno = son->lineno;
    }
}

/* print the tree*/
void printTree(Node* root, int n){
    for(int i=0; i<n; i++)
        printf("·");
    if(root==NULL){
	return;
    } 
    if(root->son==NULL){
	if(strcmp(root->name, "FLOAT") == 0){
	    printf("%s: %f\n", root->name, atof(root->value));
	}else if(strcmp(root->name, "INT") == 0){
	    printf("%s: %ld\n", root->name, strtol(root->value, NULL, 0));
	}else if((strcmp(root->name, "TYPE") == 0)||(strcmp(root->name, "ID") == 0)){
	    printf("%s: %s\n", root->name, root->value);
	}else{
	    printf("%s\n", root->name);
	}
    }else{
    	printf("%s (%d)\n", root->name, root->lineno);
    	Node* p = root->son;
    	while(p != NULL){
        	printTree(p, n+1);
        	p = p -> bro;
    	}
    }
}

/* delete the tree*/
void deleteTree(Node* root){
    if(root==NULL){
        return;
    }else{
        deleteTree(root->son);
        deleteTree(root->bro);
        free(root);
    }
}

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
//  if(errorFlag == 0){
	printTree(root, 0);
//  }
  deleteTree(root);
  return 0;
}
