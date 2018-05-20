# include <string.h>
# include "semantic.h"

SymbolList *funSymbolList;
FieldList *argList;				 
Type *TYPE_INT, *TYPE_FLOAT, *returnType;
int noStructName;

int isTypeEqual(Type *type1, Type *type2)
{
	if (type1->kind != type2->kind) return 0;
	if (type1->kind == BASIC) 
		return type1->basic == type2->basic;
	if (type1->kind == ARRAY) {
		Type *newType1 = type1->array.elem;
		Type *newType2 = type2->array.elem;
		return isTypeEqual(newType1, newType2) && type1->array.size == type2->array.size;
	}
	if (type1->kind == STRUCTURE) {
		FieldList *struct1 = type1->structure;
		FieldList *struct2 = type2->structure;
		for (; struct1 != NULL && struct2 != NULL; struct1 = struct1->tail, struct2 = struct2->tail) {
			if (!isTypeEqual(struct1->type, struct2->type)) return 0;
		}
		return struct1 == NULL && struct2 == NULL;
	}
	return 0;
}

int isArgEqual(FieldList *arg1, FieldList *arg2)
{
	for (; arg1 != NULL && arg2 !=  NULL; arg1 = arg1->tail, arg2 = arg2->tail) {
		if (!isTypeEqual(arg1->type, arg2->type)) return 0;
	}
	if (arg1 == NULL && arg2 == NULL) return 1;
	else return 0;
}

void initBasicType() {
	TYPE_INT = (Type*)malloc(sizeof(Type));
	TYPE_INT->kind = BASIC;
	TYPE_INT->basic = 0;
	TYPE_FLOAT = (Type*)malloc(sizeof(Type));
	TYPE_FLOAT->kind = BASIC;
	TYPE_FLOAT->basic = 1;
}

void analyseProgram(TreeNode *node)
{
	if (node == NULL) return;
	if (strcmp(node->name, "Program") != 0) return;
	
	TreeNode *extDefList = node->firstchild;
	funSymbolList = NULL;
	argList = NULL;
	noStructName = 0;
	analyseExtDefList(extDefList);
	for (; funSymbolList != NULL;) {
		Symbol* symbol = funSymbolList->symbol;

		if (!symbol->func->isDefined) {
			printf("Error type 18 at Line %d: ", funSymbolList->lineno);
			printf(semanticError[18], symbol->name);
		}
		SymbolList* p = funSymbolList;
		funSymbolList = funSymbolList->next;
		free(p);
	}
}

void analyseExtDefList(TreeNode *node)
{
	if (node == NULL) return;
	if (strcmp(node->name, "ExtDefList") != 0) return;
	
	TreeNode *extDef = node->firstchild;
	analyseExtDef(extDef);
	
	if (extDef == NULL) return;
	TreeNode *extDefList = extDef->nextsibling;
	if (extDefList != NULL && strcmp(extDefList->name, "ExtDefList") == 0) 
		analyseExtDefList(extDefList);
}

void analyseExtDef(TreeNode *node)
{
	if (node == NULL) return;
	if (strcmp(node->name, "ExtDef") != 0) return;
	TreeNode *specifier = node->firstchild;
	Type *type = analyseSpecifier(specifier);
	if (specifier == NULL) return;
	TreeNode *second = specifier->nextsibling;
	if (second != NULL && strcmp(second->name, "ExtDecList") == 0) 
		analyseExtDecList(second, type);
	else if (second != NULL && strcmp(second->name, "FunDec") == 0) {
		TreeNode *third = second->nextsibling;
		int isDefined = (third != NULL && strcmp(third->name, "CompSt") == 0);
		Func *func = analyseFunDec(second, type, isDefined);
		if (func == NULL) return;
		returnType = func->returnType;
		if (isDefined) {
			analyseCompSt(third, func);
			func->isDefined = 1;
		}
	}
}

void analyseExtDecList(TreeNode *node, Type *type)
{
	if (node == NULL) return;
	if (strcmp(node->name, "DecList") != 0) return;
	
	TreeNode *varDec = node->firstchild;
	if (varDec == NULL) return;
	TreeNode *last = varDec;
	for (; last->nextsibling != NULL; last = last->nextsibling);
	FieldList * var = analyseVarDec(varDec, type);
	Symbol *symbol = (Symbol*)malloc(sizeof(Symbol));
	symbol->kind = VAR;
	symbol->type = var->type;
	symbol->name = (char*)malloc(strlen(varDec->morpheme) + 1);
	strcpy(symbol->name, varDec->morpheme);

	if (!symbolTableInsert(symbol)) {
		printf("Error type 3 at Line %d: ", varDec->lineno);
		printf(semanticError[3], symbol->name);
	}
	if (last != NULL && strcmp(last->name, "ExtDecList") == 0) {
		analyseExtDecList(last, type);
	}
}

Type* analyseSpecifier(TreeNode *node) 
{
	if (node == NULL) return NULL;
	if (strcmp(node->name, "Specifier") != 0) return NULL;
	
	TreeNode *first = node->firstchild;

	if (first != NULL && strcmp(first->name, "TYPE") == 0) {
		if (strcmp(first->morpheme, "int") == 0) return TYPE_INT;
		else return TYPE_FLOAT;
	} else if (first != NULL && strcmp(first->name, "StructSpecifier") == 0) 
				return analyseStructSpecifier(first);
			
}

Type* analyseStructSpecifier(TreeNode *node) 
{
	if (node == NULL) return NULL;
	if (strcmp(node->name, "StructSpecifier") != 0) return NULL;
	
	TreeNode *first = node->firstchild;
	if (first != NULL && strcmp(first->name, "STRUCT") == 0) {
		TreeNode *second = first->nextsibling;
		if (second != NULL && strcmp(second->name, "OptTag") == 0) {
			if (second->nextsibling == NULL) return NULL;
			TreeNode *defList = second->nextsibling->nextsibling;
			Type *type = (Type*)malloc(sizeof(Type));
			type->kind = STRUCTURE;
			type->structure = analyseDefList(defList, NULL, 1);
			analyseOptTag(second, type);
			return type;
		} else if (second != NULL && strcmp(second->name, "Tag") == 0) 
				return analyseTag(second);
		  else if (second != NULL && strcmp(second->name, "LC") == 0) {
			  TreeNode *defList = second->nextsibling;
			  Type *type = (Type*)malloc(sizeof(Type));
			  type->kind = STRUCTURE;
			  type->structure = analyseDefList(defList, NULL, 1);
			  analyseOptTag(NULL, type);
			  return type;
		  }
	}
}
