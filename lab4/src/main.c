#include "partree.h"
#include "syntax.tab.h"
#include "symbol.h"
#include "intercode.h"
#include "semantic.h"
#include "target.h"


extern TreeNode *root = NULL;

int main(int argc, char **argv)
{
	if (argc <= 1) return 1;
	FILE* f = fopen(argv[1], "r");
	FILE* fout = fopen(argv[2],"w");
    if (!f||!fout)//file cannot open
	{
		perror(argv[1]);
		return 1;
	}
	/*parsing*/
	yyrestart(f);
	yyparse();
	if (root != NULL)
	{
		symbolInit();
		initBasicType();
		initOperandBasic();
		printBeginCodes(fout);
		analyseProgram(root,fout);
	}
	fclose(f);
	fclose(fout);
    return 0;
}
