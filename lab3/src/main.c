#include "syntax.tab.h"
#include "partree.h"
#include "symbol.h"
#include "semantic.h"
#include "intercode.h"

extern TreeNode *root = NULL;
int main(int argc, char **argv)
{
    if (argc <= 1) return 1;
	int i = 1;
	for (i = 1 ; i < argc; i++)
	{
		FILE* f = fopen(argv[i], "r");
	    if (!f)//file cannot open
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
			analyseProgram(root);
		}
		fclose(f);
	}
    return 0;
}
