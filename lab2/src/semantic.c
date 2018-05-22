# include <string.h>
# include "semantic.h"

const char *semanticError[] = {"","Undefined variable \"%s\".\n",
							 "Undefined function \"%s\".\n", 
							 "Redefined variable \"%s\".\n", 
							 "Redefined function \"%s\".\n", 
							 "Type mismatched for assignment.\n", 
							 "The left-hand side of an assignment must be a variable.\n", 
							 "Type mismatched for operands.\n", 
							 "Type mismatched for return.\n",
							 "Function \"%s(%s)\" is not applicable for arguments \"(%s)\".\n", 
							 "\"%s\" is not an array.\n", 
							 "\"%s\" is not a function.\n", 
							 "\"%f\" is not an integer.\n", 
							 "Illegal use of \".\".\n", 
							 "Non-existent field \"%s\".\n", 
							 "Redefined field \"%s\".\n", 
							 "Duplicated name \"%s\".\n", 
							 "Undefined structure \"%s\".\n",
							 "Undefined function \"%s\".\n",
							 "Inconsistent declaration of function \"%s\".\n"};

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

void analyseOptTag(TreeNode *node, Type *type)
{
	if (node == NULL) return;
	if (strcmp(node->name, "OptTag") != 0) return;
	
	TreeNode *id = node->firstchild;
	Symbol *symbol = (Symbol*)malloc(sizeof(Symbol));
	symbol->kind = STRUCTS;
	symbol->type = type;
	
	if (id == NULL) {
		char name[50] = "0";
		int num = noStructName, i = 1;
		while (num > 0) {
			name[i] = num % 10;
			i++; num/=10;
		}
		name[i] = '\0';
		symbol->name = (char*)malloc(strlen(name) + 1);
		strcpy(symbol->name, name);
	}
	else if (id != NULL && strcmp(id->name, "ID") == 0) {
		symbol->name = (char*)malloc(strlen(id->morpheme) + 1);
		strcpy(symbol->name, id->morpheme);
	}
	
	if (!symbolTableInsert(symbol)) {
			printf("Error type 16 at Line %d: ", id->lineno);
			printf(semanticError[16], symbol->name);
	}
}

Type* analyseTag(TreeNode *node) 
{
	if (node == NULL) return NULL;
	if (strcmp(node->name, "Tag") != 0) return NULL;
	
	TreeNode *id = node->firstchild;
	if (id != NULL && strcmp(id->name, "ID") == 0) {
		Symbol *symbol = findSymbol(id->morpheme);
		if (symbol == NULL || symbol->kind != STRUCTS) {
			printf("Error type 17 at Line %d: ", id->lineno);
			printf(semanticError[17], id->morpheme);
			return NULL;
		}
		return symbol->type;
	}
	return NULL;
}

FieldList* analyseVarDec(TreeNode *node, Type *type)
{
	TreeNode *first = node->firstchild;
	if (first != NULL && strcmp(first->name, "ID") == 0) {
		FieldList *var = (FieldList*)malloc(sizeof(FieldList));
		var->name = (char*)malloc(strlen(first->morpheme) + 1);
		strcpy(var->name, first->morpheme);
		var->type = type;
		var->tail = NULL;
		return var;
	} else if (first != NULL && strcmp(first->name, "VarDec") == 0) {
		TreeNode *second = first->nextsibling;
		TreeNode *size = second->nextsibling;
		Type *newType = (Type*)malloc(sizeof(Type));
		newType->kind = ARRAY;
		newType->array.elem = type;
		newType->array.size = size->intvalue;
		return analyseVarDec(first, newType);
	} else return NULL;
}

Func* analyseFunDec(TreeNode *node, Type *type, int isDefined)
{
	if (node == NULL) return NULL;
	if (strcmp(node->name, "FunDec") != 0) return NULL;
	
	TreeNode *id = node->firstchild;
	TreeNode *second = id->nextsibling;
	
	if (id == NULL) return NULL;
	if (id->nextsibling = NULL) return NULL;
	
	TreeNode *third = second->nextsibling;
	
	Func *func = (Func*)malloc(sizeof(Func));
	func->returnType = type;
	func->isDefined = 0;
	func->arg = NULL;
	
	Symbol *symbol = findSymbol(id->morpheme);
	
	if (symbol != NULL && (symbol->kind != FUNC || (isDefined && symbol->func->isDefined))) {
		printf("Error type 4 at Line %d: ", id->lineno);
		printf(semanticError[4], symbol->name);
	} else {
		if (third != NULL && strcmp(third->name, "VarList") == 0)
			 func->arg = analyseVarList(third, func->arg);
			 
		if (symbol == NULL) { //该函数名未定义或声明过
			symbol = (Symbol*)malloc(sizeof(Symbol));
			symbol->kind = FUNC;
			symbol->func = func;
			symbol->name = (char*)malloc(strlen(id->morpheme) + 1);
			strcpy(symbol->name, id->morpheme);
			
			if (symbolTableInsert(symbol)) {
				SymbolList *funSymbol = (SymbolList*)malloc(sizeof(SymbolList));
				funSymbol->symbol = symbol;
				funSymbol->lineno = node->lineno;
				funSymbol->next = funSymbolList;
				funSymbolList = funSymbol;
			}
			return func;
		} else if (!isTypeEqual(func->returnType, symbol->func->returnType) || !isArgEqual(func->arg, symbol->func->arg)) {
			FieldList *field;
			
			symbol->func->isDefined = isDefined;
			func->isDefined = symbol->func->isDefined;
			printf("Error type 19 at Line %d: ", id->lineno);
			printf(semanticError[19], symbol->name);
		} else {
			symbol->func->isDefined = isDefined;
			func->isDefined = symbol->func->isDefined;
			return func;
		}
	}
	return NULL;
}

FieldList* analyseVarList(TreeNode *node, FieldList *args)
{
	if (node == NULL) return;
	if (strcmp(node->name, "VarList") != 0) return;
	
	TreeNode *first = node->firstchild;
	TreeNode *last = first;
	for (; last->nextsibling != NULL; last = last->nextsibling);
	
	FieldList *arg = analyseParamDec(first);
	arg->tail = args;
	args = arg;
	if (last != NULL && strcmp(last->name, "VarList") == 0) return analyseVarList(last, args);
	else return args;
}

FieldList* analyseParamDec(TreeNode *node)
{
	if (node == NULL) return;
	if (strcmp(node->name, "ParamDec") != 0) return;
	
	TreeNode *specifier = node->firstchild;
	TreeNode *varDec = specifier->nextsibling;
	Type *type = analyseSpecifier(specifier);
	return analyseVarDec(varDec, type);
}
