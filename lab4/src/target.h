#ifndef __TARGET_H__
#define __TARGET_H__

#include "intercode.h"

void printBeginCodes(FILE *);
int countInterCodes(InterCodes*);
void changeOperandId(Operand**, int [], int []);
void recountId(InterCodes*);
int *getOffsets(InterCodes*);
int getBase(Operand *);
void loadVariable(Operand *, int *, int, FILE *);
void storeVariable(Operand *, int *, int, FILE *);
void interCodes2Target(InterCodes*, FILE *);


#endif
