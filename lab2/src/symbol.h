#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include <stdio.h>
#include <stdlib.h>

typedef struct SymbolNode {
	Symbol *symbol;
	struct SymbolNode *preHashList, *preStack;
	struct SymbolNode *nextHashList, *nextStack;
} SymbolNode;

void symbolInit();
unsigned int hash_pjw(char*);
void insertBefore(SymbolNode*, SymbolNode*,SymbolNode*);
void deleteBefore(SymbolNode*);
