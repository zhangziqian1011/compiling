#ifndef __PARTREE_H__
#define __PARTREE_H__

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdarg.h>

typedef struct tnode
{
	int intvalue;
	double doublevalue;
	char *stringvalue;
	char *name;
	int lineno;
	struct node *first;
	struct node *next;
}node;

node *build(char *,int,...);
node *newnode();
void addnode(node *,node *);
void printtree(node *,int);

#endif
