#include <stdio.h>
#include <stdlib.h>
#include "symbol.h"

#define SIZE 0x4000

SymbolNode table[SIZE], stack[SIZE];
int top;

void symbolInit()
{
	int i;
	for (i = 0; i < SIZE; i++) {
		table[i].preHashList = table[i].nextHashList = NULL;
		table[i].preStack = table[i].nextStack = NULL;
		table[i].symbol = NULL;
	}
	top = 0;

	Type *TYPE_INT = (Type *)malloc(sizeof(Type));
	TYPE_INT->kind = BASIC;
	TYPE_INT->basic = 0;

	Func *readFunc = (Func *)malloc(sizeof(Func));
	readFunc->returnType = TYPE_INT;
	readFunc->arg = NULL;
	readFunc->isDefined = 0;
	Symbol *read = (Symbol *)malloc(sizeof(Symbol));
	read->kind = FUNC;
	read->func = readFunc;
	read->id = -1;
	read->name = (char *)malloc(5);
	strcpy(read->name,"read");
	symbolTableInsert(read);

	Func *writeFunc = (Func *)malloc(sizeof(Func));
	writeFunc->returnType = TYPE_INT;
	writeFunc->arg = (FieldList *)malloc(sizeof(FieldList));
	writeFunc->arg->type = TYPE_INT;
	writeFunc->isDefined = 0;
	Symbol *write = (Symbol *)malloc(sizeof(Symbol));
	write->kind = FUNC;
	write->func = writeFunc;
	write->id = -1;
	write->name = (char *)malloc(6);
	strcpy(write->name,"write");
	symbolTableInsert(write);
}
unsigned int hash_pjw(char *name)
{
	unsigned int val = 0, i;
	
	for (; *name; name++) {
		val = (val << 2) + *name;
		if (i = val & ~0x3fff) val = (val ^ (i >> 12)) & 0x3fff;
	}
	return val;
}
void insertBefore(SymbolNode *hashIndex, SymbolNode *stackIndex,SymbolNode *data)
{
	if (hashIndex->nextHashList == NULL) {
		hashIndex->nextHashList = data;
		data->preHashList = hashIndex;
	}
	else {
		hashIndex->nextHashList->preHashList = data;
		data->nextHashList = hashIndex->nextHashList;
		hashIndex->nextHashList = data;
		data->preHashList = hashIndex;
	}
	
	if (stackIndex->nextStack == NULL) {
		stackIndex->nextStack = data;
		data->preStack = stackIndex;
	} else {
		stackIndex->nextStack->preStack = data;
		data->nextStack = stackIndex->nextStack;
		stackIndex->nextStack = data;
		data->preStack = stackIndex;
	}
}
void deleteBefore(SymbolNode *data)
{
	data->preHashList->nextHashList = data->nextHashList;
	if (data->nextHashList != NULL)
		data->nextHashList->preHashList = data->preHashList;
	
	data->preStack->nextStack = data->nextStack;
	if (data->nextStack != NULL)
		data->nextStack->preStack = data->preStack;
}

Symbol* findSymbol(char *name) {
	int index = hash_pjw(name);
	SymbolNode *node = table + index;

	for (node = node->nextHashList; node != NULL; node = node->nextHashList) {
		Symbol* symbol = node->symbol;
		if (symbol == NULL || strcmp(symbol->name, name) == 0) return symbol;
	}
	return NULL;
}

int symbolIsDefined(char *name)
{
	Symbol* symbol = findSymbol(name);
	if (symbol == NULL) return 0;
	if (symbol->depth == top) return 1;
	return 0;
}

int symbolTableInsert(Symbol* symbol)
{
	if (symbolIsDefined(symbol->name)) return 0;
	SymbolNode *node = (SymbolNode*)malloc(sizeof(SymbolNode));
	int index = hash_pjw(symbol->name);
	symbol->depth = top;
	node->nextHashList = node->preHashList = NULL;
	node->preStack = node->nextStack = NULL;
	node->symbol = symbol;
	insertBefore(table + index, stack + top, node);
	return 1;
}

void stackPush()
{
	top++;
}

void stackPop() 
{
	SymbolNode *p, *node = stack + top;
	for (node = node->nextStack; node != NULL;) {
		deleteBefore(node); 
		p = node;
		node = node->nextStack;
		Symbol *symbol = p->symbol;
		if (symbol != NULL) {
			Type *type = symbol->type;
			if (type != NULL) {
				if ((symbol->kind == VAR && type->kind == ARRAY) || symbol->kind == STRUCTS) {
					releaseType(type);
				}
			}
			free(symbol->name);
			free(symbol);
		}	
		free(p);
	}
}

void releaseType(Type* type)
{
	if (type->kind == BASIC) free(type);
	else if (type->kind == ARRAY) {
		Type *newType = type->array.elem;
		if (newType->kind == ARRAY) releaseType(newType);
		free(type);
	} else {
		FieldList* node = type->structure;
		for (; node != NULL; node = node->tail) {
			FieldList* p = node;
			releaseType(p->type);
			free(p->name);
			free(p->type);
		}
		
		free(type);
	}
}
