#include "partree.h"

TreeNode *buildTree(char *name,int narg,...)
{
	int i=0;
	TreeNode *root=newNode();//创建树的根节点
	root->name=(char *)malloc(strlen(name)+1);
	strcpy(root->name,name);
	va_list p;//开始对可变参数进行处理
	va_start(p,narg);
	TreeNode *p1;
	while(i<narg)//依次获取各个可变参数
	{
		p1=va_arg(p,TreeNode *);
		if(i==0)
		{
			root->lineno=p1->lineno;
		}
		addChild(root,p1);
		i++;
	}
	va_end(p);
	return root;
}

TreeNode *newNode()
{
	TreeNode *p=(TreeNode *)malloc(sizeof(TreeNode));
	p->firstchild=p->nextsibling=NULL;//对新节点初始化
	return p;
}

void addChild(TreeNode *p1,TreeNode *p2)
{
	if(p1->firstchild!=NULL)//判断是否有子节点
	{
		TreeNode *temp=p1->firstchild;
		while(temp->nextsibling!=NULL)
		{
			temp=temp->nextsibling;
		}
		temp->nextsibling=p2;
	}
	else
	{
		p1->firstchild=p2;
	}
}

void printTree(TreeNode *root,int n)
{
	for(int i=0;i<2*n;i++)
	{
		printf(" ");//凑格式
	}
	printf("%s",root->name);
	if(root->firstchild==NULL)
	{
		if(strcmp(root->name,"INT")==0)
		{
			printf(": %d",root->intvalue);
		}
		else if(strcmp(root->name,"FLOAT")==0)
		{
			printf(": %lf",root->floatvalue);
		}
		else if(strcmp(root->name,"ID")==0||strcmp(root->name,"TYPE")==0)
		{
			printf(": %s",root->morpheme);
		}
	}
	else
	{
		printf(" (%d)",root->lineno);
	}
	printf("\n");
	TreeNode *p=root->firstchild;
	while(p)
	{
		printTree(p,n+1);
		p=p->nextsibling;
	}
}
