# include <stdio.h>
# include <string.h>
# include "intercode.h"

const char *INTER_CODE[] = {
		"LABEL %s :\n",
		"FUNCTION %s :\n",
		"%s := %s\n",
		"%s := %s + %s\n",
		"%s := %s - %s\n",
		"%s := %s * %s\n",
		"%s := %s / %s\n",
		"%s := &%s\n",
		"%s := *%s\n",
		"*%s := %s\n",
		"GOTO %s\n",
		"IF %s %s %s GOTO %s\n",
		"RETURN %s\n",
		"DEC %s %d\n",
		"ARG %s\n",
		"%s := CALL %s\n",
		"PARAM %s\n",
		"READ %s\n",
		"WRITE %s\n",
};

void interCodeInsertBefore(InterCodes *interCodes, InterCodes *data)
{
	data->next = interCodes->next;
	data->prev = interCodes;
	interCodes->next->prev = data;
	interCodes->next = data;
	//data->next = interCodes;
	//data->prev = interCodes->prev;
	//interCodes->prev = data;
	//data->prev->next = data;
}

void interCodeDeleteBefore(InterCodes *data)
{
	data->prev->next = data->next;
	data->next->prev = data->prev;
}

void interCodeBind(InterCodes *interCodes, InterCodes *ir)
{
	if (ir->next == ir) return;

	InterCodes *first = ir->next;
	InterCodes *last = ir->prev;

	interCodes->next->prev = last;
	last->next = interCodes->next;
	interCodes->next = first;
	first->prev = interCodes;
}

InterCodes* newInterCodes1(interCodeKind kind, Operand *op)
{
	InterCodes *ir = (InterCodes*)malloc(sizeof(InterCodes));
	ir->next = ir->prev = ir;
	ir->code = (InterCode*)malloc(sizeof(InterCode));
	ir->code->kind = kind;
	ir->code->value.single = op;
	ir->code->num = 1;
	return ir;
}

InterCodes* newInterCodes2(interCodeKind kind, Operand *op1, Operand *op2)
{
	InterCodes *ir = (InterCodes*)malloc(sizeof(InterCodes));
	ir->next = ir->prev = ir;
	ir->code = (InterCode*)malloc(sizeof(InterCode));
	ir->code->kind = kind;
	ir->code->assign.left = op1;
	ir->code->assign.right = op2;
	
//	printf("&&&[  %d, %s  ]&&&:\n", ir->code->assign.left->id, ir->code->assign.right->name);
	
	ir->code->num = 2;
	return ir;
}

InterCodes* newInterCodes3(interCodeKind kind, Operand *op1, Operand *op2, Operand *op3)
{
	InterCodes *ir = (InterCodes*)malloc(sizeof(InterCodes));
	ir->next = ir->prev = ir;
	ir->code = (InterCode*)malloc(sizeof(InterCode));
	ir->code->kind = kind;

	ir->code->binop.result = op1;
	ir->code->binop.op1 = op2;
	ir->code->binop.op2 = op3;
	ir->code->num = 3;
	return ir;
}

int getInterCodesId(InterCodes *ir)
{
	if (ir->code->num == 1) {
		Operand *op = ir->code->value.single;
		return op->id;
	} else if (ir->code->num == 2) {
		Operand *op = ir->code->assign.left;
		return op->id;
	} else {
		Operand *op = ir->code->binop.result;
		return op->id;
	}
}

void interCodesPrint(InterCodes *interCodes)
{
	InterCodes *ir = interCodes->prev;
	for (; ir != interCodes; ir = ir->prev) {
		InterCode *code = ir->code;
		if (code->kind == GOTO_COND) {
			printf(INTER_CODE[code->kind], operandPrint(code->binop.op1), code->relop, operandPrint(code->binop.op2), operandPrint(code->binop.result));
		} else if (code->kind == DEC) {
			printf(INTER_CODE[code->kind], operandPrint(code->value.single), code->size);
		} else if (code->num == 3) {
			//printf("%s\n",operandPrint(code->binop.result));
			//printf("%s\n",operandPrint(code->binop.op1));
			//printf("%s\n",operandPrint(code->binop.op2));
			printf(INTER_CODE[code->kind], operandPrint(code->binop.result), operandPrint(code->binop.op1), operandPrint(code->binop.op2));
		} else if (code->num == 2) {
			printf(INTER_CODE[code->kind], operandPrint(code->assign.left), operandPrint(code->assign.right));
		} else {
			printf(INTER_CODE[code->kind], operandPrint(code->value.single));
		}
	}
	printf("\n");
}

void operandInsertBefore(OperandList *operandList, OperandList *data)
{
	data->next = operandList;
	data->prev = operandList->prev;
	operandList->prev = data;
	data->prev->next = data;
}

void operandDeleteBefore(OperandList *data)
{
	data->prev->next = data->next;
	data->next->prev = data->prev;
}

int varOperandId()
{
	static int vId = 0;
	return ++vId;
}
int tempOperandId()
{
	static int tId = 0;
	return ++tId;
}
int labelOperandId()
{
	static int lId = 0;
	return ++lId;
}

Operand* newVarOperand()
{
	Operand *op = (Operand*)malloc(sizeof(Operand));
	op->kind = VARIABLE;
	op->id = varOperandId();
	return op;
}

Operand* newTempOperand()
{
	Operand *op = (Operand*)malloc(sizeof(Operand));
	op->kind = TEMP;
	op->id = tempOperandId();
	return op;
}

Operand* newLabelOperand()
{
	Operand *op = (Operand*)malloc(sizeof(Operand));
	op->kind = LABEL;
	op->id = varOperandId();
	return op;
}

Operand* newConstOperand(int num)
{
	Operand *op = (Operand*)malloc(sizeof(Operand));
	op->kind = CONSTANT;
	op->num = num;
	return op;
}

Operand* newFunctionOperand(char *name)
{
	Operand *op = (Operand*)malloc(sizeof(Operand));
	op->kind = FUNCTION;
	op->name = (char*)malloc(name + 1);
	strcpy(op->name, name);
	return op;
}

char* operandPrint(Operand *op) 
{
	char buf[100];
	char *name;
	if (op->kind == TEMP) {
		sprintf(buf, "t%d", op->id);
	} else if (op->kind == VARIABLE) {
		sprintf(buf, "v%d", op->id);
	} else if (op->kind == CONSTANT) {
		sprintf(buf, "#%d", op->num);
	} else if (op->kind == LABEL) {
		sprintf(buf, "label%d", op->id);
	} else return op->name;

	name = malloc(strlen(buf) + 1);
	strcpy(name, buf);
	return name;
}
