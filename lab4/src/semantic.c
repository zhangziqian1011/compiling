# include <string.h>
# include "semantic.h"
# include "translate.h"
# include "target.h"

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

Type *copyType(Type *type)
{
	Type *newType = malloc(sizeof(Type));
	newType->kind = type->kind;
	if (type->kind == BASIC) {
		newType->basic = type->basic;
	}else if (type->kind == ARRAY) {
		(newType->array).elem = copyType(type->array.elem);
		(newType->array).size = (type->array).size;
	}else if (type->kind == STRUCTURE) {
		FieldList *struct1 = type->structure;
		FieldList *p = NULL;
		newType->structure = NULL;
		for (;struct1 != NULL; struct1 = struct1->tail) {
			FieldList *newStruct = malloc(sizeof(FieldList));
			newStruct->name = malloc(strlen(struct1->name) + 1);
			strcpy(newStruct->name, struct1->name);
			newStruct->type = copyType(struct1->type);
			newStruct->tail = NULL;
			if (newType->structure == NULL)
				newType->structure = newStruct;
			if (p != NULL)
				p->tail = newStruct;
			p = newStruct;
		}

	}
	return newType;
}

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

void analyseProgram(TreeNode *node, FILE *file)
{
	if (node == NULL) return;
	if (strcmp(node->name, "Program") != 0) return;
	
	TreeNode *extDefList = node->firstchild;
	funSymbolList = NULL;
	argList = NULL;
	noStructName = 0;
	analyseExtDefList(extDefList, file);
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

void analyseExtDefList(TreeNode *node, FILE *file)
{
	if (node == NULL) return;
	if (strcmp(node->name, "ExtDefList") != 0) return;
	
	TreeNode *extDef = node->firstchild;
	analyseExtDef(extDef, file);
	
	if (extDef == NULL) return;
	TreeNode *extDefList = extDef->nextsibling;
	if (extDefList != NULL && strcmp(extDefList->name, "ExtDefList") == 0) 
		analyseExtDefList(extDefList, file);
}

void analyseExtDef(TreeNode *node, FILE *file)
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
			//printf("FUNCTION %s :\n",(second->firstchild)->morpheme);
			fprintf(file,"\n%s:\n",(second->firstchild)->morpheme);
			analyseCompSt(third, func, file);
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
	symbol->id = -1;
	symbol->isRef = 0;
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
	symbol->id = -1;
	symbol->isRef = 0;
	
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
			symbol->id = -1;
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

void analyseCompSt(TreeNode *node, Func *func, FILE *file)
{
	if (node == NULL) return;
	if (strcmp(node->name, "CompSt") != 0) return;
	
	stackPush();
	
	TreeNode *first = node->firstchild;
	if (first == NULL) return;
	TreeNode *defList = first->nextsibling;
	if (defList == NULL) return;
	TreeNode *stmtList = defList->nextsibling;
	
	if (func != NULL) {
		FieldList *arg = func->arg;
		for (; arg != NULL; arg = arg->tail) {
			Symbol *symbol = (Symbol*)malloc(sizeof(Symbol));
			symbol->kind = VAR;
			symbol->type = copyType(arg->type);
			symbol->id = -1;
			symbol->isRef = 0;
			//symbol->type = arg->type;
			symbol->name = (char*)malloc(strlen(arg->name) + 1);
			strcpy(symbol->name, arg->name);
			if (arg->type->kind != BASIC) symbol->isRef = 1;
			
			symbolTableInsert(symbol);
		}
	}
	if (defList != NULL && strcmp(defList->name, "DefList") == 0) analyseDefList(defList, NULL, 0);
	if (defList != NULL && strcmp(defList->name, "StmtList") == 0) analyseStmtList(defList);
	if (stmtList != NULL && strcmp(stmtList->name, "StmtList") == 0) analyseStmtList(stmtList);
	
	if(func != NULL) {
		InterCodes *interCodes = translateCompst(node, func);
		//interCodesPrint(interCodes);
		interCodes2Target(interCodes, file);
		//interCodesPrint(interCodes);
	}

	stackPop();
}

void analyseStmtList(TreeNode *node)
{
	if (node == NULL) return;
	if (strcmp(node->name, "StmtList") != 0) return;
	
	TreeNode *stmt = node->firstchild;
	if (stmt == NULL) return;
	
	analyseStmt(stmt);
	TreeNode *stmtList = stmt->nextsibling;
	if (stmtList != NULL && strcmp(stmtList->name, "StmtList") == 0) analyseStmtList(stmtList);	
}

void analyseStmt(TreeNode *node)
{
	if (node == NULL) return;
	if (strcmp(node->name, "Stmt") != 0) return;
	
	TreeNode *first = node->firstchild;
	if (first != NULL && strcmp(first->name, "Exp") == 0) analyseExp(first);
	else if (first != NULL && strcmp(first->name, "CompSt") == 0) {
		analyseCompSt(first, NULL, NULL);
	} else if (first != NULL && strcmp(first->name, "RETURN") == 0) {
		TreeNode *exp = first->nextsibling;
		if (exp != NULL && strcmp(exp->name, "Exp") == 0) {
			Symbol *symbol = analyseExp(exp);
			if (symbol != NULL) {
				Type *type = symbol->type;
				if (type != NULL && !isTypeEqual(type, returnType)) {
					printf("Error type 8 at Line %d: ", exp->lineno);
					printf("%s", semanticError[8]);
				}
			}
		}
	} else {
		TreeNode *p = first->nextsibling;
		for (; p != NULL; p = p->nextsibling) {
			
			if (p != NULL && strcmp(p->name, "Exp") == 0) {
				Symbol *symbol = analyseExp(p);
				if (symbol != NULL) {
					Type *type = symbol->type;
					if (type != NULL && !isTypeEqual(type, TYPE_INT)) {
						printf("Error type 7 at Line %d: ", p->lineno);
						printf("%s", semanticError[7]);
					}
				}
			} else if (p != NULL && strcmp(p->name, "Stmt") == 0) analyseStmt(p);
		}
	}
}

FieldList* analyseDefList(TreeNode *node, FieldList *structure, int isStruct)
{
	if (node == NULL) return;
	if (strcmp(node->name, "DefList") != 0) return;
	
	TreeNode *def = node->firstchild;
	if (def != NULL && strcmp(def->name, "Def") == 0) {
		structure = analyseDef(def, structure, isStruct);
		
		
		TreeNode *defList = def->nextsibling;
		if (defList != NULL && strcmp(defList->name, "DefList") == 0) return analyseDefList(defList, structure, isStruct);
		else return structure;
	}
}

FieldList* analyseDef(TreeNode *node, FieldList *structure, int isStruct) 
{
	if (node == NULL) return;
	if (strcmp(node->name, "Def") != 0) return;
	
	TreeNode *specifier = node->firstchild;
	if (specifier != NULL && strcmp(specifier->name, "Specifier") == 0) {
		TreeNode *decList = specifier->nextsibling;
		if (decList != NULL && strcmp(decList->name, "DecList") == 0) {
			Type *type = analyseSpecifier(specifier);
			return analyseDecList(decList, type, structure, isStruct);
		}
	}
}

FieldList* analyseDecList(TreeNode *node, Type *type, FieldList *structure, int isStruct)
{
	if (node == NULL) return;
	if (strcmp(node->name, "DecList") != 0) return;
	
	TreeNode *dec = node->firstchild;
	
	if (dec != NULL && strcmp(dec->name, "Dec") == 0) {
		structure = analyseDec(dec, type, structure, isStruct);
		

		if (dec->nextsibling != NULL) {
			TreeNode *decList = dec->nextsibling->nextsibling;
			if (decList != NULL && strcmp(decList->name, "DecList") == 0) {
				return analyseDecList(decList, type, structure, isStruct);
			}
		} else return structure;
		
	}
}

FieldList* analyseDec(TreeNode *node, Type *type, FieldList *structure, int isStruct)
{
	if (node == NULL) return;
	if (strcmp(node->name, "Dec") != 0) return;
	
	TreeNode *varDec = node->firstchild;
	TreeNode *last = varDec;
	for (; last->nextsibling != NULL; last = last->nextsibling);
	
	FieldList *var = analyseVarDec(varDec, type);
	
	if (isStruct) {
		FieldList *field;
		for (field = structure; field != NULL; field = field->tail) {
			if (strcmp(field->name, var->name) == 0) break;}

		if (field != NULL) {
			printf("Error type 15 at Line %d: ", node->lineno);
			printf(semanticError[15], var->name);
		} else {
			var->tail = structure;
			structure = var;
		}
		
		if (last != NULL && strcmp(last->name, "Exp") == 0) {
			printf("Error type 15 at Line %d: ", node->lineno);
			printf(semanticError[15], var->name);
		}
		return structure;
	} else {
		Symbol *symbol = (Symbol*)malloc(sizeof(Symbol));
		symbol->kind = VAR;
		symbol->type = var->type;
		symbol->name = var->name;
		symbol->id = -1;
		symbol->isRef = 0;
		symbol->name = (char*)malloc(strlen(var->name) + 1);
		strcpy(symbol->name, var->name);
		
		if (!symbolTableInsert(symbol)) {
			printf("Error type 3 at Line %d: ", node->lineno);
			printf(semanticError[3], var->name);
		} else if (last != NULL && strcmp(last->name, "Exp") == 0) {
			Symbol *symbol1 = analyseExp(last);
			if (symbol1 != NULL) {
				Type *type = symbol1->type;
				if (type != NULL && !isTypeEqual(type, var->type)) {
					printf("Error type 5 at Line %d: ", node->lineno);
					printf("%s",semanticError[5]);
				}
			}
		}
		return NULL;
	}
}

void argString(FieldList *arg, char* str) {
	for (; arg != NULL; arg = arg->tail) {
		if (isTypeEqual(arg->type, TYPE_INT)) {
			strcpy(str, "int");
			str = str + strlen(str);
		}
		else if (isTypeEqual(arg->type, TYPE_FLOAT)) {
			strcpy(str, "float");
			str = str + strlen(str);
		}
		else {
			Type *type = arg->type;
			if (type->kind = STRUCTURE) {
				strcpy(str, "struct");
				str = str + strlen(str);
			}
			else {
				Type *type = arg->type;
				for (; type != NULL && type->kind == ARRAY; type = type->array.elem);
				if (isTypeEqual(type, TYPE_INT)) strcpy(str, "int");
				else if (isTypeEqual(type, TYPE_FLOAT)) strcpy(str, "float");
				else if (type->kind = STRUCTURE) strcpy(str, "struct");
			
				for (type = arg->type; type != NULL && type->kind == ARRAY; type = type->array.elem) {
					sprintf(str, "[%d]", type->array.size);
					str = str + strlen(str);
				}
			}
		}
		if (arg->tail != NULL) {
			sprintf(str, ", ");
			str = str + 2;
		}
	}
}

Symbol* analyseExp(TreeNode *node)
{
	if (node == NULL) return;
	if (strcmp(node->name, "Exp") != 0) return;
	
	TreeNode *first = node->firstchild;
	
	if (first == NULL) return;
	TreeNode *second = first->nextsibling;
	
	if (second != NULL && strcmp(second->name, "ASSIGNOP") == 0) {
		TreeNode *third = second->nextsibling;
		Symbol *left = analyseExp(first);
		if (third != NULL && strcmp(third->name, "Exp") == 0) {
			Symbol *right = analyseExp(third);
			if (left != NULL && (left->kind != VAR || (left->kind == VAR && left->name == NULL))) {
				printf("Error type 6 at Line %d: ", node->lineno);
				printf("%s",semanticError[6]);
			} else if (left != NULL && right != NULL && !isTypeEqual(left->type, right->type)) {
				printf("Error type 5 at Line %d: ", node->lineno);
				printf("%s",semanticError[5]);
			}
		}
		
		
		return left;
	} else if (second != NULL && (strcmp(second->name, "AND") == 0 || strcmp(second->name, "OR") == 0)) {
		TreeNode *third = second->nextsibling;
		Symbol *left = analyseExp(first);
		if (third != NULL && strcmp(third->name, "Exp") == 0) {
			Symbol *right = analyseExp(third);
			if (left != NULL && right != NULL && !(isTypeEqual(left->type, TYPE_INT) && isTypeEqual(right->type, TYPE_INT))) {
				printf("Error type 7 at Line %d: ", node->lineno);
				printf("%s",semanticError[7]);
			}
		}
		return left;
	} else if (first != NULL && strcmp(first->name, "MINUS") == 0) {
		if (second != NULL && strcmp(second->name, "Exp") == 0) {
			Symbol *left = analyseExp(second);
			if (left == NULL || !(isTypeEqual(left->type, TYPE_INT) || isTypeEqual(left->type, TYPE_FLOAT))) {
				printf("Error type 7 at Line %d: ", node->lineno);
				printf("%s",semanticError[7]);
			}
			return left;
		}
	} else if (first != NULL && strcmp(first->name, "NOT") == 0) {
		if (second != NULL && strcmp(second->name, "Exp") == 0) {
			Symbol *left = analyseExp(first);
			if (left == NULL || !isTypeEqual(left->type, TYPE_INT)) {
				printf("Error type 7 at Line %d: ", node->lineno);
				printf("%s",semanticError[7]);
			}
			return left;
		}
	} else if (first != NULL && strcmp(first->name, "ID") == 0) {
		if (second != NULL && strcmp(second->name, "LP") == 0) {
			Symbol *symbol = findSymbol(first->morpheme);
			if (symbol == NULL) {
				printf("Error type 2 at Line %d: ", node->lineno);
				printf(semanticError[2], first->morpheme);
			} else if (symbol->kind != FUNC) {
				printf("Error type 11 at Line %d: ", node->lineno);
				printf(semanticError[11], first->morpheme);
			} else {
				FieldList *arg1 = NULL, *arg2 = symbol->func->arg;
				TreeNode *args = second->nextsibling;
				if (args != NULL && strcmp(args->name, "Args") == 0) {
					
					arg1 = analyseArgs(args, arg1);
					char str1[50],str2[50];
					if (!isArgEqual(arg1, arg2)) {
						argString(arg1, str1);
						argString(arg2, str2);
						printf("Error type 9 at Line %d: ", first->lineno);
						printf(semanticError[9], symbol->name, str1, str2);
					}	
				}
				Symbol *newSymbol = (Symbol*)malloc(sizeof(Symbol));;
				newSymbol->kind = symbol->kind;
				newSymbol->type = symbol->func->returnType;
				newSymbol->depth = symbol->depth;
				symbol->id = -1;
				newSymbol->name = (char*)malloc(strlen(symbol->name) + 1);
				strcpy(newSymbol->name, symbol->name);

				return newSymbol;
			}
			return symbol;
		} else {
			Symbol *symbol = findSymbol(first->morpheme);
			if (symbol == NULL) {
				printf("Error type 1 at Line %d: ", first->lineno);
				printf(semanticError[1], first->morpheme);
			}
		
			return symbol;
		}
	} else if (second != NULL && strcmp(second->name, "LB") == 0) {
		TreeNode *third = second->nextsibling;
		Symbol *left = analyseExp(first);
		
		Type *type = left->type;
		if (left == NULL || type->kind != ARRAY) {
			printf("Error type 10 at Line %d: ", first->lineno);
			printf(semanticError[10], left->name);
			return left;
		}
		if (third != NULL) {
			Symbol *right = analyseExp(third);
			if (right == NULL || !isTypeEqual(right->type, TYPE_INT)) {
				printf("Error type 12 at Line %d: ", third->lineno);
				printf(semanticError[12], third->firstchild->floatvalue);
			}
		}
		Symbol *newSymbol = (Symbol*)malloc(sizeof(Symbol));;
		newSymbol->kind = left->kind;
		newSymbol->type = type->array.elem;
		newSymbol->depth = left->depth;
		newSymbol->id = -1;
		newSymbol->name = (char*)malloc(strlen(left->name) + 1);
		strcpy(newSymbol->name, left->name);

		return newSymbol;
		
	} else if (second != NULL && strcmp(second->name, "DOT") == 0) {
		TreeNode *third = second->nextsibling;
		Symbol *left = analyseExp(first);
		Type *type = left->type;
		
		if (left == NULL || type->kind != STRUCTURE) {
			printf("Error type 13 at Line %d: ", first->lineno);
			printf("%s", semanticError[13]);
			return NULL;
		} else if (third != NULL) {
			FieldList *field = type->structure;
			for (; field != NULL; field = field->tail)
				if (strcmp(field->name, third->morpheme) == 0) break;
			if (field == NULL) {
				printf("Error type 14 at Line %d: ", first->lineno);
				printf(semanticError[14], third->morpheme);
				return NULL;
			}
			Symbol *newSymbol = (Symbol*)malloc(sizeof(Symbol));;
			newSymbol->kind = left->kind;
			newSymbol->type = field->type;
			newSymbol->depth = left->depth;
			newSymbol->id = -1;
			newSymbol->name = (char*)malloc(strlen(left->name) + 1);
			strcpy(newSymbol->name, left->name);
			
			return newSymbol;
		}
		
	} else if (first != NULL && strcmp(first->name, "Exp") == 0) {
		TreeNode *third = second->nextsibling;
		Symbol *left = analyseExp(first);
		if (third != NULL && strcmp(third->name, "Exp") == 0) {
			Symbol *right = analyseExp(third);
			if (left != NULL && right != NULL && !isTypeEqual(left->type, right->type)) {
				printf("Error type 7 at Line %d: ", node->lineno);
				printf("%s",semanticError[7]);
			}
		}
		return left;
	}else if (first !=NULL && strcmp(first->name, "LP") == 0) {
		if (second != NULL && strcmp(second->name, "Exp") == 0) {
			return analyseExp(second);
		}
	} else if (first != NULL && strcmp(first->name, "INT") == 0) {
		Symbol *symbol = (Symbol*)malloc(sizeof(Symbol));
		symbol->kind = VAR;
		symbol->type = TYPE_INT;
		symbol->id = -1;
		symbol->isRef = 0;
		symbol->name = NULL;
		return symbol;
	} else {
		Symbol *symbol = (Symbol*)malloc(sizeof(Symbol));
		symbol->kind = VAR;
		symbol->type = TYPE_FLOAT;
		symbol->id = -1;
		symbol->isRef = 0;
		symbol->name = NULL;
		return symbol;
	}
}

FieldList* analyseArgs(TreeNode *node, FieldList *args)
{
	if (node == NULL) return;
	if (strcmp(node->name, "Args") != 0) return;
	
	TreeNode *exp = node->firstchild;
	TreeNode *last = exp;
	for (; last->nextsibling != NULL; last = last->nextsibling);
	
	if (exp != NULL && strcmp(exp->name, "Exp") == 0) {
		FieldList *arg = (FieldList*)malloc(sizeof(FieldList));
		Symbol *symbol = analyseExp(exp);
		
		arg->name = NULL;
		arg->type = symbol->type;
		arg->tail = args;
		args = arg;
		if (last != NULL && strcmp(last->name, "Args") == 0) return analyseArgs(last, args);
		else return args;
	}
}
