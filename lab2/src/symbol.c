#include <stdio.h>
#include <stdlib.h>
#include "symbol.h"



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
