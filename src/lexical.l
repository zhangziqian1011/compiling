%{
#define YYSTYPE TreeNode*

#include "partree.h"
#include "syntax.tab.h"

extern int isError;



void getToken(char *name)//处理输入
{
    TreeNode *p = newNode();
    p->name = (char *)malloc(strlen(name) + 1);
    strcpy(p->name, name);
    p->stringvalue = (char *)malloc(strlen(yytext) + 1);
    strcpy(p->stringvalue, yytext);
    p->lineno = yylineno;
    yylval = p;
}

int isOct()
{
	int i;
	for (i = 0; i < yyleng; i++) 
	{
		if (yytext[i] == '8' || yytext[i] == '9')
		{
			printf("Error type A at Line %d: Illegal octal number \'%s\'\n", yylineno, yytext);
			isError = 1;
			return 0;
		}
	}
	return 1;
}
int isHex()
{
	int i;
	for (i = 2; i < yyleng; i++)
	{
		if ((yytext[i]>'f' && yytext[i]<='z') || (yytext[i]>'F' && yytext[i]<='Z')) {
			printf("Error type A at Line %d: Illegal hexadecimal number \'%s\'\n", yylineno, yytext);
			isError = 1;
			return 0;
		}
	}
	return 1;
}



%}

digit [0-9]
decNum 0|[1-9]{digit}*
octNum 0{digit}+
hexNum 0[xX][0-9a-zA-Z]+
letter [_a-zA-Z]
space [ \t\n\v\f\r]
int {decNum}|{octNum}|{hexNum}
float {digit}+{exp}|{base}{exp}?
wrongfloat {base}[eE]([-\+]?{base})?
exp [eE][-\+]?{digit}+
base {digit}+\.{digit}*|\.{digit}+
id {letter}({letter}|{digit})*
%option yylineno


%%

; {getToken("SEMI"); return SEMI;}
, {getToken("COMMA"); return COMMA;}
= {getToken("ASSIGNOP"); return ASSIGNOP;}
>|<|>=|<=|==|!= {getToken("RELOP"); return RELOP;}
\+ {getToken("PLUS"); return PLUS;}
- {getToken("MINUS"); return MINUS;}
\* {getToken("STAR"); return STAR;}
\/ {getToken("DIV"); return DIV;}
&& {getToken("AND"); return AND;}
"||" {getToken("OR"); return OR;}
\. {getToken("DOT"); return DOT;}
! {getToken("NOT"); return NOT;}
int|float {getToken("TYPE"); return TYPE;}
"(" {getToken("LP"); return LP;}
")" {getToken("RP"); return RP;}
"[" {getToken("LB"); return LB;}
"]" {getToken("RB"); return RB;}
"{" {getToken("LC"); return LC;}
"}" {getToken("RC"); return RC;}
struct {getToken("STRUCT"); return STRUCT;}
return {getToken("RETURN"); return RETURN;}
if {getToken("IF"); return IF;}
else {getToken("ELSE"); return ELSE;}
while {getToken("WHILE"); return WHILE;}
{id} {getToken("ID"); return ID;}
{space} { }
"//".* {}
"/*" {
    char a = 'x', b = input();
    while (a != '*' || b != '/') 
    {
        a = b;
        b = input();
        if (b == EOF)
            break;
    }
}
. {
    isError = 1;
    printf("Error type A at Line %d: Mysterious characters \"%s\"\n", yylineno, yytext); 
}

{decNum} {
    getToken("INT");
    yylval->intvalue = atoi(yytext);
    return INT;
}

{octNum} {
	getToken("INT");
	if (isOct())
		yylval->intvalue = strtol(yytext, NULL, 8);
	return INT;
}
{hexNum} {
    getToken("INT");
	if (isHex())
		yylval->intvalue = strtol(yytext, NULL, 16);
    return INT;
}

{wrongfloat} {
	getToken("FLOAT");
	printf("Error type A at Line %d: Illegal float number \'%s\'\n", yylineno, yytext);
	isError = 1;
	yylval->floatvalue = .0;
	return FLOAT;
}
{float} {
    getToken("FLOAT");
    yylval->floatvalue = atof(yytext);
    return FLOAT;
}

%%


