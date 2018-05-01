#include "partree.h"

node *build(char *key,int narg,...)
{
	int i=0;
	node *root=newnode();//创建树的根节点
	root->name=(char *)malloc(strlen(key)+1);
	strcpy(root->name.key);
	va_list p;//开始对可变参数进行处理
	va_start(p,narg);
	node *p1;
	while(i<narg)//依次获取各个可变参数
	{
		p1=va_arg(p,node *);
		if(i==0)
		{
			root->lineno=p1->lineno;
		}
		addnode(root,p1);
		i++;
	}
	va_end(p);
	return root;
}

node *newnode()
{
	node *p=(node *)malloc(sizeof(node));
	p->first=p->next=NULL;//对新节点初始化
	return p;
}

void addnode(node *p1,node *p2)
{
	if(p1->first!=NULL)//判断是否有子节点
	{
		node *temp=p1->first;
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

void printtree(node *root,int n)
{
	for(int=0;i<2*n;i++)
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
	node *p=root->first;
	while(p)
	{
		printtree(p,depth+1);
		p=p->next;
	}
}
