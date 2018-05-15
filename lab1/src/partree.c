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
	p->first=p->next=NULL;//对新节点初始化
	return p;
}

void addChild(TreeNode *p1,TreeNode *p2)
{
	if(p1->first!=NULL)//判断是否有子节点
	{
		TreeNode *temp=p1->first;
		while(temp->next!=NULL)
		{
			temp=temp->next;
		}
		temp->next=p2;
	}
	else
	{
		p1->first=p2;
	}
}

void printTree(TreeNode *root,int n)
{
	for(int i=0;i<2*n;i++)
	{
		printf(" ");//凑格式
	}
	printf("%s",root->name);
	if(root->first==NULL)
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
			printf(": %s",root->stringvalue);
		}
	}
	else
	{
		printf(" (%d)",root->lineno);
	}
	printf("\n");
	TreeNode *p=root->first;
	while(p)
	{
		printTree(p,n+1);
		p=p->next;
	}
}
