# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include "symbol.h"
# include "intercode.h"
# include "translate.h"
# include "partree.h"

Operand *CONST_ZERO, *CONST_ONE;

void initOperandBasic()
{
	CONST_ZERO= (Operand*)malloc(sizeof(Operand));
	CONST_ZERO->kind = CONSTANT;
	CONST_ZERO->num = 0;

	CONST_ONE= (Operand*)malloc(sizeof(Operand));
	CONST_ONE->kind = CONSTANT;
	CONST_ONE->num = 1;
}

int typeSize(Type *type)
{
	int size = 0;
	if (type->kind == BASIC) return 4;
	else if (type->kind ==  ARRAY) return typeSize(type->array.elem) * (type->array.size);
	else {
		FieldList *field = type->structure;
		for (; field != NULL; field = field->tail)size += typeSize(field->type);
		return size;
	}
}

InterCodes* translateCompst(TreeNode *node, Func *func)
{
	////////printf("%s\n", node->name);

	TreeNode *first = node->firstchild;
	TreeNode *defList = first->nextsibling;
	TreeNode *stmtList = defList->nextsibling;

	InterCodes *interCodes = (InterCodes*)malloc(sizeof(InterCodes));
	interCodes->code = interCodes->prev = interCodes->next = interCodes;

	if (func != NULL) {
		FieldList *arg = func->arg;
		for (; arg != NULL; arg = arg->tail) {
			char *name = (char*)malloc(strlen(arg->name) + 1);
			strcpy(name, arg->name);

			InterCodes *ir = newInterCodes1(PARAM, newVarOperand());
			interCodeInsertBefore(interCodes, ir);

			Symbol *symbol = findSymbol(name);
			symbol->id = getInterCodesId(ir);
		}
	}

	if (defList != NULL && strcmp(defList->name, "DefList") == 0)
		interCodeBind(interCodes, translateDefList(defList));
		if (stmtList != NULL && strcmp(stmtList->name, "DefList") == 0)
			interCodeBind(interCodes, translateDefList(stmtList));
	if (stmtList != NULL && strcmp(stmtList->name, "StmtList") == 0)
		interCodeBind(interCodes, translateStmtList(stmtList));
		if (defList != NULL && strcmp(defList->name, "StmtList") == 0)
			interCodeBind(interCodes, translateStmtList(defList));

	return interCodes;
}

InterCodes* translateDefList(TreeNode *node)
{
	//////printf("%s\n", node->name);

	TreeNode *def = node->firstchild;
	InterCodes *interCodes = translateDef(def);

	TreeNode *defList = def->nextsibling;
	if (defList != NULL && strcmp(defList->name, "DefList") == 0)
		interCodeBind(interCodes, translateDefList(defList));

	return interCodes;
}

InterCodes* translateDef(TreeNode *node)
{
	//////printf("%s\n", node->name);

	TreeNode *specifier = node->firstchild;
	TreeNode *decList = specifier->nextsibling;

	InterCodes *interCodes = translateDecList(decList);

	return interCodes;
}

InterCodes* translateDecList(TreeNode *node)
{
	//////printf("%s\n", node->name);

	TreeNode *dec = node->firstchild;
	InterCodes *interCodes = translateDec(dec);

	if (dec->nextsibling != NULL) {
		TreeNode *decList = dec->nextsibling->nextsibling;
		if (decList != NULL && strcmp(decList->name, "DecList") == 0)
				interCodeBind(interCodes, translateDecList(decList));
	}

	return interCodes;
}
