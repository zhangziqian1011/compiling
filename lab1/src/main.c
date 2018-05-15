#include "syntax.tab.h"
#include "partree.h"

extern int yyrestart(FILE *f1);
extern int yyparse();

int main(int argc,char **argv)
{
	if(argc<2)
	{
		return 1;
	}
	else
	{
		int i=1;
		while(i<argc)
		{
			FILE *f=fopen(argv[i],"r");
			if(!f)
			{
				perror(argv[1]);
				return 1;
			}
			else
			{
				yyrestart(f);
				yyparse();
				fclose(f);
			}
			i++;
		}
		return 0;
	}
}
