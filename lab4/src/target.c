#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "intercode.h"

const char *beginCodes = ".data\n_prompt: .asciiz \"Enter an integer:\"\n_ret: .asciiz \"\\n\"\n.globl main\n.text\nread:\n\tli $v0, 4\n\tla $a0, _prompt\n\tsyscall\n\tli $v0, 5\n\tsyscall\n\tjr $ra\n\nwrite:\n\tli $v0, 1\n\tsyscall\n\tli $v0, 4\n\tla $a0, _ret\n\tsyscall\n\tmove $v0, $0\n\tjr $ra\n\n";

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

