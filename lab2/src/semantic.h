#ifndef __SEMANTIC_H__
#define __SEMANTIC_H__

# include "tree.h"
# include "symbol.h"

typedef struct SymbolList {
	Symbol* symbol;
	struct SymbolList *next;
	int lineno;
} SymbolList;


int isTypeEqual(Type*, Type*);
int isArgEqual(FieldList*, FieldList*);
void initBasicType();

void analyseProgram(TreeNode*);
void analyseExtDefList(TreeNode*);
void analyseExtDef(TreeNode*);
void analyseExtDecList(TreeNode*, Type*);
Type* analyseSpecifier(TreeNode*);
Type* analyseStructSpecifier(TreeNode*);
