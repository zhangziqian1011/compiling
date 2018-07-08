#ifndef __SEMANTIC_H__
#define __SEMANTIC_H__

# include "partree.h"
# include "symbol.h"

typedef struct SymbolList {
	Symbol* symbol;
	struct SymbolList *next;
	int lineno;
} SymbolList;


int isTypeEqual(Type*, Type*);
int isArgEqual(FieldList*, FieldList*);
void initBasicType();

void analyseProgram(TreeNode*,FILE*);
void analyseExtDefList(TreeNode*,FILE*);
void analyseExtDef(TreeNode*,FILE*);
void analyseExtDecList(TreeNode*, Type*);
Type* analyseSpecifier(TreeNode*);
Type* analyseStructSpecifier(TreeNode*);
void analyseOptTag(TreeNode*, Type*);
Type* analyseTag(TreeNode*);
FieldList* analyseVarDec(TreeNode*, Type*);
Func* analyseFunDec(TreeNode*, Type*, int);
FieldList* analyseVarList(TreeNode*, FieldList*);
FieldList* analyseParamDec(TreeNode*);
void analyseCompSt(TreeNode*, Func*,FILE*);
void analyseStmtList(TreeNode*);
void analyseStmt(TreeNode*);
FieldList* analyseDefList(TreeNode*, FieldList*, int);
FieldList* analyseDef(TreeNode*, FieldList*, int);
FieldList* analyseDecList(TreeNode*, Type*, FieldList*, int);
FieldList* analyseDec(TreeNode*, Type*, FieldList*, int);
void argString(FieldList*, char*);
Symbol* analyseExp(TreeNode*);
FieldList* analyseArgs(TreeNode*, FieldList*);

#endif
