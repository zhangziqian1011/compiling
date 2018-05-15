#ifndef __PARTREE_H__
#define __PARTREE_H__

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdarg.h>

typedef struct treeNode
{
	int intvalue;
	double floatvalue;
	char *stringvalue;
	char *name;
	int lineno;
	struct treeNode *first;
	struct treeNode *next;
}TreeNode;

TreeNode *buildTree(char *,int,...);
TreeNode *newNode();
void addChild(TreeNode *,TreeNode *);
void printTree(TreeNode *,int);

#endif
