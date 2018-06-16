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

char* getVarDecName(TreeNode *node)
{
	TreeNode *first = node->firstchild;
	if (first != NULL && strcmp(first->name, "ID") == 0) return first->morpheme;
    else if (first != NULL && strcmp(first->name, "VarDec") == 0) return getVarDecName(first);
	else return NULL;
}

InterCodes* translateDec(TreeNode *node)
{
	//////printf("%s\n", node->name);

	TreeNode *varDec = node->firstchild;
	TreeNode *last = varDec;
	for (; last->nextsibling != NULL; last = last->nextsibling);

	Symbol *symbol = findSymbol(getVarDecName(varDec));
	Operand *var = (Operand*)malloc(sizeof(Operand));

	if (symbol->id == -1) symbol->id = varOperandId();
	var->kind = VARIABLE;
	var->id = symbol->id;

	InterCodes *interCodes = (InterCodes*)malloc(sizeof(InterCodes));
	interCodes->code = interCodes->prev = interCodes->next = interCodes;

	if (symbol->type->kind != BASIC) {
		InterCodes *ir1 = newInterCodes1(DEC, var);
		Type *type = symbol->type;
		ir1->code->size = typeSize(type);
		////////printf("\n%d\n\n", ir1->code->size);
		interCodeInsertBefore(interCodes, ir1);
	}

	if (last != NULL && strcmp(last->name, "Exp") == 0) {
		Operand *op = newTempOperand();
		InterCodes *ir2 = newInterCodes2(ASSIGN, var, op);

		interCodeBind(interCodes, translateExp(last, op));
		interCodeInsertBefore(interCodes, ir2);
	}

	return interCodes;
}

InterCodes* translateStmtList(TreeNode *node)
{
	//////printf("%s\n", node->name);

	TreeNode *stmt = node->firstchild;
	TreeNode *stmtList = stmt->nextsibling;

	InterCodes *interCodes = translateStmt(stmt);
	if (stmtList != NULL && strcmp(stmtList->name, "StmtList") == 0)
		interCodeBind(interCodes, translateStmtList(stmtList));

	return interCodes;
}

InterCodes* translateStmt(TreeNode *node)
{
	////////printf("%s\n", node->name);

	TreeNode *first = node->firstchild;
	if (first != NULL && strcmp(first->name, "Exp") == 0) {
		//puts("11");
		InterCodes *interCodes = translateExp(first, NULL);
		return interCodes;
	} else if (first != NULL && strcmp(first->name, "CompSt") == 0) {
		//puts("22");
		InterCodes *interCodes = translateCompst(first, NULL);
		return interCodes;
	} else if (first != NULL && strcmp(first->name, "RETURN") == 0) {
		//puts("33");
		TreeNode *exp = first->nextsibling;
		Operand *t1 = newTempOperand();

		InterCodes *interCodes = translateExp(exp, t1);

		InterCodes *ir = newInterCodes1(RETURN_, t1);
		//puts("%%%%%%%");
		interCodeInsertBefore(interCodes, ir);

		return interCodes;
	} else if (first != NULL && strcmp(first->name, "IF") == 0) {
		//puts("44");
		TreeNode *exp = first->nextsibling->nextsibling;
		TreeNode *stmt1 = exp->nextsibling->nextsibling;
		TreeNode *stmt2 = stmt1;
		for (; stmt2->nextsibling != NULL; stmt2 = stmt2->nextsibling);

		int isElse = (stmt1->nextsibling != NULL);

		////puts("++++++");
		////////printf("%d %d %d\n", exp == NULL, stmt1 == NULL, stmt2 == NULL);

		Operand *label1 = newLabelOperand();
		Operand *label2 = newLabelOperand();
		Operand *label3 = NULL;

		//puts("------");
		InterCodes *interCodes = translateCond(exp, label1, label2);

		InterCodes *ir1 = newInterCodes1(DEFINE_LABEL, label1);
		interCodeInsertBefore(interCodes, ir1);
		interCodeBind(interCodes, translateStmt(stmt1));


		if (isElse) {
			label3 = newLabelOperand();
			InterCodes *ir2 = newInterCodes1(GOTO, label3);
			interCodeInsertBefore(interCodes, ir2);
		}

		InterCodes *ir3 = newInterCodes1(DEFINE_LABEL, label2);
		interCodeInsertBefore(interCodes, ir3);

		if (isElse) {
			interCodeBind(interCodes, translateStmt(stmt2));
			InterCodes *ir4 = newInterCodes1(DEFINE_LABEL, label3);
			interCodeInsertBefore(interCodes, ir4);
		}

		return interCodes;
	} else {
		//puts("55");
		TreeNode *exp = first->nextsibling->nextsibling;
		TreeNode *stmt = exp;
		for (; stmt->nextsibling != NULL; stmt = stmt->nextsibling);

		Operand *label1 = newLabelOperand();
		Operand *label2 = newLabelOperand();
		Operand *label3 = newLabelOperand();

		InterCodes *interCodes = (InterCodes*)malloc(sizeof(InterCodes));
		interCodes->code = interCodes->prev = interCodes->next = interCodes;


		InterCodes *ir1 = newInterCodes1(DEFINE_LABEL, label1);
		interCodeInsertBefore(interCodes, ir1);
		//puts("===========1");

		////////printf("\n%s\n\n", exp->name);
		interCodeBind(interCodes, translateCond(exp, label2, label3));
		//puts("===========2");

		InterCodes *ir2 = newInterCodes1(DEFINE_LABEL, label2);
		interCodeInsertBefore(interCodes, ir2);

		interCodeBind(interCodes, translateStmt(stmt));
		//puts("===========3");

		InterCodes *ir3 = newInterCodes1(GOTO, label1);
		interCodeInsertBefore(interCodes, ir3);
		InterCodes *ir4 = newInterCodes1(DEFINE_LABEL, label3);
		interCodeInsertBefore(interCodes, ir4);

		return interCodes;
	}
}

InterCodes* translateExp(TreeNode *node, Operand *result)
{
	////////printf("%s\n", node->name);

	TreeNode *first = node->firstchild;
	TreeNode *second = first->nextsibling;

	static Type *type = NULL;
	InterCodes *interCodes = (InterCodes*)malloc(sizeof(InterCodes));
	interCodes->code = interCodes->prev = interCodes->next = interCodes;

	if (first != NULL && strcmp(first->name, "INT") == 0) {
		//puts("1");
		if (result == NULL) return interCodes;
		*result = *newConstOperand(first->intvalue);
		type = TYPE_INT;
	} else if (first != NULL && strcmp(first->name, "ID") == 0) {
		//puts("2");
		Symbol *symbol = findSymbol(first->morpheme);
		if (second == NULL) {
			//puts("1:");
			if (result == NULL) return interCodes;

			Operand *op = (Operand*)malloc(sizeof(Operand));
			if (symbol->id == -1) symbol->id = varOperandId();
			op->kind = VARIABLE;
			op->id = symbol->id;

			type = symbol->type;

			if (result->id == -1 && type->kind != BASIC) {
				result->id = tempOperandId();
				if (symbol->isRef) {
					InterCodes *ir = newInterCodes2(ASSIGN, result, op);
					interCodeInsertBefore(interCodes, ir);
				} else {
					InterCodes *ir = newInterCodes2(ASSIGN_ADDR, result, op);
					interCodeInsertBefore(interCodes, ir);
				}
			} else {
				//puts("============");
				*result = *op;
			}
		} else {
			//puts("2:");
			TreeNode *third = second->nextsibling;
			OperandList *argOperand = (OperandList*)malloc(sizeof(OperandList));
			argOperand->prev = argOperand->next = argOperand;

			if (third != NULL && strcmp(third->name, "Args") == 0) {
				FieldList *argList = symbol->func->arg;
				interCodeBind(interCodes, translateArgs(third, argList, argOperand));
				////////printf("%d\n", argOperand->next == argOperand);
			}
			if (strcmp(symbol->name, "read") == 0) {
				if (result == NULL) result = newTempOperand();
				InterCodes *ir = newInterCodes1(READ, result);
				interCodeInsertBefore(interCodes, ir);
				//operandPrint(ir->code->value.single);

			} else if (strcmp(symbol->name, "write") == 0) {
				//puts("!!!5!!!");
				//////////printf("%d\n", argOperand == NULL);
				Operand *op = argOperand->prev->operand;
				//puts("!!!50!!!");
				InterCodes *ir = newInterCodes1(WRITE, op);
				interCodeInsertBefore(interCodes, ir);

				if (result != NULL) {
					Operand *op1 = newConstOperand(0);
					InterCodes *ir1 = newInterCodes2(ASSIGN, result, op);
					interCodeInsertBefore(interCodes, ir1);
				}
			} else {
				OperandList *oplist = argOperand->next;
				for (; oplist != argOperand; oplist = oplist->next) {
					Operand *op = oplist->operand;
					InterCodes *ir = newInterCodes1(ARG, op);
					interCodeInsertBefore(interCodes, ir);
				}

				if (result == NULL) result = newTempOperand();
				Operand *op = newFunctionOperand(first->morpheme);
			//	//////printf("\n%s\n\n", op->name);
				InterCodes *ir = newInterCodes2(CALL, result, op);
				interCodeInsertBefore(interCodes, ir);
			}

			type = symbol->func->returnType;
		}
	} else if (second != NULL && strcmp(second->name, "ASSIGNOP") == 0) {
		//puts("3");
		Operand *op1 = (Operand*)malloc(sizeof(Operand));
		op1->kind = TEMP;
		op1->id = -1;
		Operand *op2 = newTempOperand();


		InterCodes *ir1 = translateExp(first, op1);
		InterCodes *ir2 = translateExp(second->nextsibling, op2);

		interCodeBind(interCodes, ir1);
		interCodeBind(interCodes, ir2);

		InterCodes *ir;
		//////////printf("&&&[  %d, %d  ]&&&:\n", op1->id, op2->id);

		if (op1->kind == VARIABLE) ir = newInterCodes2(ASSIGN, op1, op2);
		else ir = newInterCodes2(VAL_ASSIGN, op1, op2);
		interCodeInsertBefore(interCodes, ir);

		if (result == NULL) return interCodes;

		//puts("####");
		//////////printf("%d\n", result ==NULL);

		if (result->id == -1) *result = *op1;
		else {
			InterCodes *ir3 = newInterCodes2(ASSIGN_VAL, result, op1);
			//puts("=============1");
		//	//////printf("%s %s\n", operandPrint(result), operandPrint(op1));
			interCodeInsertBefore(interCodes, ir3);
		}
	} else if (first != NULL && strcmp(first->name, "MINUS") == 0) {
	//	puts("4");
		Operand *op = newTempOperand();
		InterCodes *ir1 = translateExp(second, op);
		interCodeBind(interCodes, ir1);

		if (result == NULL) return interCodes;

		InterCodes *ir2 = newInterCodes3(SUB, result, CONST_ZERO, op);
		interCodeInsertBefore(interCodes, ir2);
	} else if ((first != NULL && strcmp(first->name, "NOT") == 0) || (second != NULL &&
		(strcmp(second->name, "RELOP") == 0 || strcmp(second->name, "AND") == 0 || strcmp(second->name, "OR") == 0))) {
	//		puts("5");
		Operand *label1 = newLabelOperand();
		Operand *label2 = newLabelOperand();

		InterCodes *ir1 = translateCond(node, label1, label2);

		if (result != NULL) result = newTempOperand();

		InterCodes *ir2 = newInterCodes2(ASSIGN, result, CONST_ZERO);
		interCodeInsertBefore(interCodes, ir2);
		interCodeBind(interCodes, ir1);

		InterCodes *ir3 = newInterCodes1(DEFINE_LABEL, label1);
		interCodeInsertBefore(interCodes, ir3);
		//////////printf("$$$$   %d    $$$$\n",label1->id);

		InterCodes *ir4 = newInterCodes2(ASSIGN, result, CONST_ONE);
		interCodeInsertBefore(interCodes, ir4);

		InterCodes *ir5 = newInterCodes1(DEFINE_LABEL, label2);
		interCodeInsertBefore(interCodes, ir5);

		type = TYPE_INT;
	} else if (first != NULL && strcmp(first->name, "LP") == 0) {
	//	puts("6");
		free(interCodes);
		return translateExp(second, result);
	} else if (second != NULL && strcmp(second->name, "LB") == 0) {
	//	puts("7");
		TreeNode *exp = second->nextsibling;
		Operand *op1 = newTempOperand();
		interCodeBind(interCodes, translateExp(exp, op1));

		Operand *base = (Operand*)malloc(sizeof(Operand));
		base->kind = TEMP;
		base->id = -1;
		interCodeBind(interCodes, translateExp(first, base));

		type = type->array.elem;

		Operand *offset = newTempOperand();
		Operand *size = newConstOperand(typeSize(type));
		InterCodes *ir1 = newInterCodes3(MUL, offset, op1, size);
		interCodeInsertBefore(interCodes, ir1);

		if (result->id == -1) {
			result->id = tempOperandId();
			InterCodes *ir2 = newInterCodes3(ADD, result, base, offset);
			interCodeInsertBefore(interCodes, ir2);
		} else {
			Operand *op2 = newTempOperand();
			InterCodes *ir2 = newInterCodes3(ADD, op2, base, offset);
			interCodeInsertBefore(interCodes, ir2);
			InterCodes *ir3 = newInterCodes2(ASSIGN_VAL, result, op2);
			//puts("==============2:");
			////////printf("%s %s\n", operandPrint(base), operandPrint(offset));
			interCodeInsertBefore(interCodes, ir3);
		}
	} else if (second != NULL && strcmp(second->name, "DOT") == 0) {
		//puts("8");
		Operand *base = (Operand*)malloc(sizeof(Operand));
		base->kind = TEMP;
		base->id = -1;
		interCodeBind(interCodes, translateExp(first, base));

		TreeNode *id = second->nextsibling;
		char *name = (char*)malloc(id->morpheme + 1);
		strcpy(name, id->morpheme);
		
		int size = 0,  all = 0;
		FieldList *field = type->structure;
		for (; field != NULL && strcmp(field->name, name) != 0; field = field->tail) 			
			size += typeSize(field->type); 		
		size += typeSize(field->type); 		
		
		FieldList *allFiled = type->structure;
		for (; allFiled != NULL; allFiled = allFiled->tail)
			all += typeSize(allFiled->type);
		
		if (size == 0) size = -1;
		size = all - size;

		Operand *offset = newConstOperand(size);
		type = field->type;

		if (result->id == -1) {
			result->id = tempOperandId();
			InterCodes *ir = newInterCodes3(ADD, result, base, offset);
			interCodeInsertBefore(interCodes, ir);
		} else {
			Operand *op = newTempOperand();
			InterCodes *ir1 = newInterCodes3(ADD, op, base, offset);
			interCodeInsertBefore(interCodes, ir1);
			InterCodes *ir2 = newInterCodes2(ASSIGN_VAL, result, op);
			//puts("=============3:");
			////////printf("%s %s\n", operandPrint(base), operandPrint(offset));
			interCodeInsertBefore(interCodes, ir2);
		}
	} else {
		//puts("9");
		Operand *t1 = newTempOperand();
		Operand *t2 = newTempOperand();
		InterCodes *ir1 = translateExp(first, t1);
		InterCodes *ir2 = translateExp(second->nextsibling, t2);

		interCodeBind(interCodes, ir1);
		interCodeBind(interCodes, ir2);

		if (result == NULL) return interCodes;

		interCodeKind kind ;
		//puts(second->name);
		////////printf("2333   %d\n",strcmp(second->name, "MINUS"));
		if ((second != NULL) && strcmp(second->name, "PLUS")==0) {kind = ADD;}
		else if ((second != NULL) && strcmp(second->name, "MINUS")==0) {kind = SUB;}
		else if ((second != NULL) && strcmp(second->name, "STAR")==0) {kind = MUL;}
		else {kind = DIV_;}

		InterCodes *ir = newInterCodes3(kind, result, t1, t2);
		interCodeInsertBefore(interCodes, ir);
	}
	return interCodes;
}

InterCodes* translateCond(TreeNode *node, Operand *labelTrue, Operand *labelFalse)
{
	////////printf("%s\n", node->name);

	TreeNode *first = node->firstchild;
	TreeNode *second = first->nextsibling;

	if (second != NULL && strcmp(second->name, "RELOP") == 0) {
		//puts("-___________");
		Operand *t1 = newTempOperand();
		Operand *t2 = newTempOperand();
		//puts("=========");
		InterCodes *interCodes = translateExp(first, t1);
		//puts("-----------");
		InterCodes *ir1 = translateExp(second->nextsibling, t2);

		interCodeBind(interCodes, ir1);

		InterCodes *ir2 = newInterCodes3(GOTO_COND, labelTrue, t1, t2);
		ir2->code->relop = second->morpheme;
		interCodeInsertBefore(interCodes, ir2);

		InterCodes *ir3 = newInterCodes1(GOTO, labelFalse);
		interCodeInsertBefore(interCodes, ir3);

		return interCodes;
	} else if (first != NULL && strcmp(first->name, "NOT") == 0) {
			return translateCond(second, labelTrue, labelFalse);
	} else if (second != NULL && strcmp(second->name, "AND") == 0) {
		Operand *label1 = newLabelOperand();
		InterCodes *interCodes = translateCond(first, label1, labelFalse);

		InterCodes *ir1 = newInterCodes1(DEFINE_LABEL, label1);
		interCodeInsertBefore(interCodes, ir1);
		interCodeBind(interCodes, translateCond(second->nextsibling, labelTrue, labelFalse));

		return interCodes;
	} else if (second != NULL && strcmp(second->name, "OR") == 0) {
		Operand *label1 = newTempOperand();
		InterCodes *interCodes = translateCond(first, labelTrue, label1);

		InterCodes *ir1 = newInterCodes1(DEFINE_LABEL, label1);
		interCodeInsertBefore(interCodes, ir1);
		interCodeBind(interCodes, translateCond(second->nextsibling, labelTrue, labelFalse));

		return interCodes;
	} else {
		Operand *t1 = newTempOperand();
		InterCodes *interCodes = translateExp(node, t1);

		InterCodes *ir1 = newInterCodes3(GOTO_COND, labelTrue, t1, CONST_ZERO);
		ir1->code->relop = (char*)malloc(2 + 1);
		strcpy(ir1->code->relop, "!=");
		interCodeInsertBefore(interCodes, ir1);

		InterCodes *ir2 = newInterCodes1(GOTO, labelFalse);
		interCodeInsertBefore(interCodes, ir2);

		return interCodes;
	}
}

InterCodes* translateArgs(TreeNode *node, FieldList *arg, OperandList *argOperand)
{
	////////printf("%s\n", node->name);

	TreeNode *first = node->firstchild;
	TreeNode *last = first;
	for (; last->nextsibling != NULL; last = last->nextsibling);

	Operand *t1;
	if (arg->type->kind == BASIC) t1 = newTempOperand();
	else {
		t1 = (Operand*)malloc(sizeof(Operand));
		t1->id = -1;
	}
	InterCodes *interCodes = translateExp(first, t1);

	OperandList *op = (OperandList*)malloc(sizeof(OperandList));
	op->operand = t1;
	op->next = NULL;

	operandInsertBefore(argOperand, op);

	if (last != NULL && strcmp(last->name, "Args") == 0)
		interCodeBind(interCodes, translateArgs(last, arg->tail, argOperand));

	return interCodes;
}
