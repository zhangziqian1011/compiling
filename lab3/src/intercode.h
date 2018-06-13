#ifndef __INTERCODE_H__
#define __INTERCODE_H__

typedef enum { VARIABLE, CONSTANT, LABEL, FUNCTION, TEMP, } operandKind;
typedef enum { DEFINE_LABEL, DEFINE_FUNCTION, 
		   ASSIGN, ADD, SUB, MUL, DIV_, 
		   ASSIGN_ADDR, ASSIGN_VAL, VAL_ASSIGN, 
		   GOTO, GOTO_COND, 
		   RETURN_, DEC, ARG, CALL, PARAM,
		   READ, WRITE, } interCodeKind;
		   
typedef struct Operand {
	operandKind kind;
	union {
		char *name;
		int id;
		int num;
	}; 
} Operand;

typedef struct InterCode {
	interCodeKind kind;
	union {
		struct { Operand *single; } value;
		struct { Operand *right, *left; } assign;
		struct { Operand *result, *op1, *op2; } binop;
	};
	union {
		char *relop;
		int size;
	};
	int num;
} InterCode;

typedef struct InterCodes {
	InterCode *code;
	struct InterCodes *prev, *next;
} InterCodes;


typedef struct OperandList {
	Operand *operand;
	struct OperandList *prev, *next;
} OperandList;

typedef struct Tnode
{
	OperandList *argList;
	InterCodes *interCodes;
} Tnode;

void interCodeInsertBefore(InterCodes*, InterCodes*);
void interCodeDeleteBefore(InterCodes*);
void interCodeBind(InterCodes*, InterCodes*);
InterCodes* newInterCodes1(interCodeKind, Operand*)
InterCodes* newInterCodes2(interCodeKind, Operand*, Operand*);
InterCodes* newInterCodes3(interCodeKind, Operand*, Operand*, Operand*);
int getInterCodesId(InterCodes*);
void interCodesPrint(InterCodes*);

#endif
