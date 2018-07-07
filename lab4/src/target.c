#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "intercode.h"

const char *beginCodes = ".data\n_prompt: .asciiz \"Enter an integer:\"\n_ret: .asciiz \"\\n\"\n.globl main\n.text\nread:\n\tli $v0, 4\n\tla $a0, _prompt\n\tsyscall\n\tli $v0, 5\n\tsyscall\n\tjr $ra\n\nwrite:\n\tli $v0, 1\n\tsyscall\n\tli $v0, 4\n\tla $a0, _ret\n\tsyscall\n\tmove $v0, $0\n\tjr $ra\n\n";

int Nt=0;
int Nv=0;
int tBase=0;

void printBeginCodes(FILE *file)
{
	fprintf(file, "%s",beginCodes);
}

int countInterCodes(InterCodes* interCodes)
{
	int cnt = 0;
	InterCodes *ir = interCodes->prev;
	for(; ir != interCodes; ir = ir->prev) cnt++;
	return cnt;
}

void changeOperandId(Operand** opp, int tIds[500],int vIds[100])
{
	Operand *op = *opp;
	if(op==NULL) return;
	if(op->kind==VARIABLE)
	{
		if(vIds[op->id]==-1)
			vIds[op->id]=Nv++;
		//fprintf(file, "%d %d\n",op->id,vIds[op->id]);
		Operand *newop=(Operand *)malloc(sizeof(Operand));
		memcpy(newop,op,sizeof(Operand));
		newop->id=vIds[op->id];
		*opp=newop;
	}else if(op->kind==TEMP)
	{
		if(tIds[op->id]==-1)
			tIds[op->id]=Nt++;
		//fprintf(file, "%d %d\n",op->id,tIds[op->id]);
		Operand *newop=(Operand *)malloc(sizeof(Operand));
		memcpy(newop,op,sizeof(Operand));
		newop->id=tIds[op->id];
		*opp=newop;
	}
}

void recountId(InterCodes* interCodes)
{
	int tIds[500];
	int vIds[100];
	int i;
	for(i=0;i<500;i++) tIds[i]=-1;
	for(i=0;i<100;i++) vIds[i]=-1;
	InterCodes *ir = interCodes->prev;
	for(;ir != interCodes; ir = ir->prev)
	{
		InterCode *code = ir->code;
		if (code->num == 3)
		{
			changeOperandId(&code->binop.result,tIds,vIds);
			changeOperandId(&code->binop.op1,tIds,vIds);
			changeOperandId(&code->binop.op2,tIds,vIds);
		}else if(code->num == 2)
		{
			changeOperandId(&code->assign.right,tIds,vIds);
			changeOperandId(&code->assign.left,tIds,vIds);	
		}else
		{
			changeOperandId(&code->value.single,tIds,vIds);
		}
	}
}

int *getOffsets(InterCodes* interCodes)
{
	Nv=Nt=tBase=0;
	recountId(interCodes);
	int *vBases=(int *)malloc(Nv*sizeof(int));
	int *vLens=(int *)malloc(Nv*sizeof(int));
	int i;
	for(i=0;i<Nv;i++){ vLens[i]=4; vBases[i]=0;}
	InterCodes *ir = interCodes->prev;
	for(;ir != interCodes; ir = ir->prev)
		if(ir->code->kind==DEC)
			vLens[ir->code->value.single->id]=ir->code->size;
	for(i=1;i<Nv;i++)
		vBases[i]=vBases[i-1]+vLens[i-1];
	tBase=vBases[Nv-1]+vLens[Nv-1];
	free(vLens);
	return vBases;
}

int getBase(Operand *op, int *vBases)
{
	if(op->kind==VARIABLE)
		return vBases[op->id];
	else
		return 4*(op->id)+tBase;
}

void loadVariable(Operand *op, int *vBases, int ri, FILE *file)
{
	if(op->kind==CONSTANT)
	{
		fprintf(file, "\tli $t%d, %d\n",ri,op->num);
	}else
	{
		int b=getBase(op,vBases);
		fprintf(file, "\tlw $t%d, -%d($fp)\n",ri,b);
	}
}

void storeVariable(Operand *op, int *vBases, int ri, FILE* file)
{
	int b=getBase(op,vBases);
	fprintf(file, "\tsw $t%d, -%d($fp)\n",ri,b);
}

void interCodes2Target(InterCodes* interCodes, FILE *file)
{
	int Nitc = countInterCodes(interCodes);
	int *vBases = getOffsets(interCodes);
	InterCodes *ir=interCodes->prev;
	fprintf(file, "\tsw $fp, -4($sp)\n\taddi $sp, -8\n\tmove $fp $sp\n\taddi $sp, -%d\n",tBase+(Nt-1)*4);
	for(;ir!=interCodes;ir=ir->prev)
	{
		InterCode *code=ir->code;
		switch(code->kind)
		{
			int i,br,b;
			Operand *r;
			Operand *l;
			Operand *result;
			Operand *op1;
			Operand *op2;
			case DEFINE_LABEL:
				fprintf(file, "label%d:\n",code->value.single->id);
				break;
			case DEFINE_FUNCTION:
				fprintf(file, "\t%s:\n",code->value.single->name);
				break;
			case ASSIGN:
				r=code->assign.right;
				l=code->assign.left;
				if (r->kind==CONSTANT){
					int lb = getBase(l,vBases);
					fprintf(file, "\tli $t0, %d\n",r->num);
					fprintf(file, "\tsw $t0, -%d($fp)\n",lb);
				}else{
					int rb = getBase(r,vBases);
					int lb = getBase(l,vBases);
					fprintf(file, "\tlw $t0, -%d($fp)\n",rb);
					fprintf(file, "\tsw $t0, -%d($fp)\n",lb);
				}
				break;
			case ADD:
				op1=code->binop.op1;
				op2=code->binop.op2;
				result=code->binop.result;
				if(op1->kind==CONSTANT)
				{
					fprintf(file, "\tli $t0, %d\n",op1->num);
					if(op2->kind==CONSTANT)
						fprintf(file, "\taddi $t0, $t0, %d\n",op2->num);
					else{
						int b2 = getBase(op2,vBases);
						fprintf(file, "lw $t1, -%d($fp)\n",b2);
						fprintf(file, "add $t0, $t0, $t1\n");
					}
				}else if(op2->kind==CONSTANT)
				{
					int b1 = getBase(op1,vBases);
					fprintf(file, "\tlw $t0, -%d($fp)\n",b1);
					fprintf(file, "\taddi $t0, $t0, %d\n",op2->num);
				}else
				{
					int b1 = getBase(op1,vBases);
					int b2 = getBase(op2,vBases);
					fprintf(file, "\tlw $t0, -%d($fp)\n",b1);
					fprintf(file, "\tlw $t1, -%d($fp)\n",b2);
					fprintf(file, "\tadd $t0, $t0, $t1\n");
				}
				br = getBase(result,vBases);
				fprintf(file, "\tsw $t0, -%d($fp)\n",br);
				break;
			case SUB:
				op1=code->binop.op1;
				op2=code->binop.op2;
				result=code->binop.result;
				if(op1->kind==CONSTANT)
				{
					fprintf(file, "\tli $t0, %d\n",op1->num);
					if(op2->kind==CONSTANT)
						fprintf(file, "\taddi $t0, $t0, -%d\n",op2->num);
					else{
						int b2 = getBase(op2,vBases);
						fprintf(file, "\tlw $t1, -%d($fp)\n",b2);
						fprintf(file, "\tsub $t0, $t0, $t1\n");
					}
				}else if(op2->kind==CONSTANT)
				{
					int b1 = getBase(op1,vBases);
					fprintf(file, "\tlw $t0, -%d($fp)\n",b1);
					fprintf(file, "\taddi $t0, $t0, -%d\n",op2->num);
				}else
				{
					int b1 = getBase(op1,vBases);
					int b2 = getBase(op2,vBases);
					fprintf(file, "\tlw $t0, -%d($fp)\n",b1);
					fprintf(file, "\tlw $t1, -%d($fp)\n",b2);
					fprintf(file, "\tsub $t0, $t0, $t1\n");
				}
				br = getBase(result,vBases);
				fprintf(file, "\tsw $t0, -%d($fp)\n",br);
				break;
			case MUL:
				op1=code->binop.op1;
				op2=code->binop.op2;
				result=code->binop.result;
				loadVariable(op1,vBases,0,file);
				loadVariable(op2,vBases,1,file);
				fprintf(file, "\tmul $t0, $t0, $t1\n");
				storeVariable(result,vBases,0,file);
				break;
			case DIV_:
				op1=code->binop.op1;
				op2=code->binop.op2;
				result=code->binop.result;
				loadVariable(op1,vBases,0,file);
				loadVariable(op2,vBases,1,file);
				fprintf(file, "\tdiv $t0, $t1\n\tmflo $t0\n");
				storeVariable(result,vBases,0,file);
				break;
			case ASSIGN_ADDR:
				l=code->assign.left;
				r=code->assign.right;
				fprintf(file, "\tmove $t0, $fp\n");
				int rb=getBase(r,vBases);
				fprintf(file, "\taddi $t0, -%d\n",rb);
				storeVariable(l,vBases,0,file);
				break;
			case ASSIGN_VAL:
				l=code->assign.left;
				r=code->assign.right;
				loadVariable(r,vBases,1,file);
				fprintf(file, "\tlw $t0, 0($t1)\n");
				storeVariable(l,vBases,0,file);
				break;
			case VAL_ASSIGN:
				l=code->assign.left;
				r=code->assign.right;
				loadVariable(l,vBases,1,file);
				loadVariable(r,vBases,0,file);
				fprintf(file, "\tsw $t0, 0($t1)\n");
				break;
			case GOTO:
				fprintf(file, "\tj label%d\n",code->value.single->id);
				break;
			case GOTO_COND:
				op1=code->binop.op1;
				op2=code->binop.op2;
				result=code->binop.result;
				loadVariable(op1,vBases,0,file);
				loadVariable(op2,vBases,1,file);
				if(strcmp(code->relop,"==")==0)
					fprintf(file, "\tbeq");
				if(strcmp(code->relop,"!=")==0)
					fprintf(file, "\tbne");
				if(strcmp(code->relop,">")==0)
					fprintf(file, "\tbgt");
				if(strcmp(code->relop,"<")==0)
					fprintf(file, "\tblt");
				if(strcmp(code->relop,">=")==0)
					fprintf(file, "\tbge");
				if(strcmp(code->relop,"<=")==0)
					fprintf(file, "\tble");
				fprintf(file, " $t0, $t1, label%d\n",result->id);
				break;
			case RETURN_:
				b = getBase(code->value.single,vBases);
				fprintf(file, "\tlw $v0, -%d($fp)\n",b);
				fprintf(file, "\tmove $sp, $fp\n\taddi $sp, 8\n\tlw $fp, 4($fp)\n");
				fprintf(file, "\tjr $ra\n",b);
				break;
			case DEC:
				break;
			case ARG:
				i=0;
				for(;ir->code->kind==ARG;ir=ir->prev)
				{
					fprintf(file, "\tlw $a%d -%d($fp)\n",i,getBase(ir->code->value.single,vBases));
					i++;
				}
				ir=ir->next;
				break;
			case CALL:
				l=code->assign.left;
				r=code->assign.right;
				fprintf(file, "\taddi $sp, $sp, -4\n\tsw $ra, 0($sp)\n\tjal %s\n\tlw $ra, 0($sp)\n\taddi $sp, $sp, 4\n",r->name);
				if(l!=NULL)
				{
					int lb = getBase(l,vBases);
					fprintf(file, "\tsw $v0 -%d($fp)\n",lb);
				}
				break;
			case PARAM:
				i=0;
				for(;ir->prev->code->kind==PARAM;ir=ir->prev) ;
				for(;ir->code->kind==PARAM;ir=ir->next)
				{
					fprintf(file, "\tsw $a%d -%d($fp)\n",i,getBase(ir->code->value.single,vBases));
					i++;
				}
				ir=ir->prev;
				for(;ir->prev->code->kind==PARAM;ir=ir->prev) ;
				break;
			case READ:
				fprintf(file, "\taddi $sp, $sp, -4\n\tsw $ra, 0($sp)\n\tjal read\n\tlw $ra, 0($sp)\n\taddi $sp, $sp, 4\n");
				fprintf(file, "\tsw $v0 -%d($fp)\n",getBase(code->value.single,vBases));
				break;
			case WRITE:
				if(code->value.single->kind==CONSTANT)
					fprintf(file, "\tli $a0, %d\n",code->value.single->num,vBases);
				else
					fprintf(file, "\tlw $a0 -%d($fp)\n",getBase(code->value.single,vBases));
				fprintf(file, "\taddi $sp, $sp, -4\n\tsw $ra, 0($sp)\n\tjal write\n\tlw $ra, 0($sp)\n\taddi $sp, $sp, 4\n");
				break;
				
		}
	}
	free(vBases);
}

