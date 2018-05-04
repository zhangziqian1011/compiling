%{
#include "tree.h"
#include "lex.yy.c"
#include <stdarg.h>
//#define YYERROR_VERBOSE
#define YYSTYPE TreeNode*

int isError = 0;

/*YYSTYPE reduction(char *name, int narg, ...)
{
    return buildTree(name, narg, __VA_ARGS__);
}*/
#define reduction buildTree

%}

/* All Tokens */
%token INT FLOAT ID SEMI COMMA LC RC
%left ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT
%left DOT LP RP LB RB
%token TYPE STRUCT RETURN IF WHILE
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%
/*High Level Definitions*/
Program : ExtDefList {
        $$ = reduction("Program", 1, $1);
        if (!isError) printTree($$, 0);
    }
    ;
ExtDefList : ExtDef ExtDefList {$$ = reduction("ExtDefList", 2, $1, $2);}
    | {$$ = NULL;}/*empty*/
    ;
ExtDef : Specifier ExtDecList SEMI {$$ = reduction("ExtDef", 3, $1, $2 ,$3);}
    | Specifier SEMI {$$ = reduction("ExtDef", 2, $1, $2);}
    | Specifier FunDec CompSt {$$ = reduction("ExtDef", 3, $1, $2, $3);}
    ;
ExtDecList : VarDec {$$ = reduction("ExtDecList", 1, $1);}
    | VarDec COMMA ExtDecList {$$ = reduction("ExtDecList", 3, $1, $2, $3);}
    ;

/*Specifier*/
Specifier : TYPE {$$ = reduction("Specifier", 1, $1);}
    | StructSpecifier {$$ = reduction("Specifier", 1, $1);}
    ;
StructSpecifier : STRUCT OptTag LC DefList RC {$$ = reduction("StructSpecifier", 5, $1, $2, $3, $4, $5);}
    | STRUCT Tag {$$ = reduction("StructSpecifier", 2, $1, $2);}
    ;
OptTag : ID {$$ = reduction("OptTag", 1, $1);}
    | {$$ = NULL;}/*empty*/
    ;
Tag : ID {$$ = reduction("Tag", 1, $1);}
    ;

/*Declarators*/
VarDec : ID {$$ = reduction("VarDec", 1, $1);}
    | VarDec LB INT RB {$$ = reduction("VarDec", 4, $1, $2, $3, $4);}
    ;
FunDec : ID LP VarList RP {$$ = reduction("FunDec", 4, $1, $2, $3, $4);}
    | ID LP RP {$$ = reduction("FunDec", 3, $1, $2, $3);}
    ;
VarList : ParamDec COMMA VarList {$$ = reduction("VarList", 3, $1, $2, $3);}
    | ParamDec {$$ = reduction("VarList", 1, $1);}
    ;
ParamDec : Specifier VarDec {$$ = reduction("ParamDec", 2, $1, $2);}
    ;

/*Statements*/
CompSt : LC DefList StmtList RC {$$ = reduction("CompSt", 4, $1, $2, $3, $4);}
    | error RC {yyerrok;}
    ;
StmtList : Stmt StmtList {$$ = reduction("StmtList", 2, $1, $2);}
    | {$$ = NULL;}/*empty*/
    ;
Stmt : Exp SEMI {$$ = reduction("Stmt", 2, $1, $2);}
    | CompSt {$$ = reduction("Stmt", 1, $1);}
    | RETURN Exp SEMI {$$ = reduction("Stmt", 3, $1, $2, $3);}
    | IF LP Exp RP Stmt {$$ = reduction("Stmt", 5, $1, $2, $3, $4, $5);} %prec LOWER_THAN_ELSE
    | IF LP Exp RP Stmt ELSE Stmt {$$ = reduction("Stmt", 7, $1, $2, $3, $4, $5, $6, $7);}
    | WHILE LP Exp RP Stmt {$$ = reduction("Stmt", 5, $1, $2, $3, $4, $5);}
    | error SEMI {yyerrok;}
	;



%%

yyerror(char *msg)
{
    isError = 1;
    printf("Error type B at Line %d: Syntax Error- %s\n", yylineno, msg);
}
