#ifndef NODE_H
#define NODE_H

#include <stdio.h>
#include <string.h>
typedef struct Node{
    char name[16];
    char value[32];
    int lineno;
    struct Node* son;
    struct Node* bro;
}Node;

Node* createNode(char* name, char* value);
void addSon(Node* father, Node* son);
void printTree(Node* root, int n);
void deleteTree(Node* root);

#endif
