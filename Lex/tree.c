#include "tree.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
extern int yylineno;

Node* createNode(char* name, char* value){
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
            son->bro = father->son;
            father->son = son;
            father->lineno = son->lineno;
        }
    }
}

/* print the tree*/
void printTree(Node* root, int n){
    for(int i=0; i<n; i++)
        printf(" ");
    if(root->son == NULL){
        
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