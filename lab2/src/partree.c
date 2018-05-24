#include "partree.h"

TreeNode *buildTree(char *name, int narg, ...)
{
    va_list ap;//use variable-length argument list to store all the chlidren
    va_start(ap, narg);
    TreeNode *root = newNode();
    root->name = (char *)malloc(strlen(name) + 1);
    strcpy(root->name, name);
    TreeNode *child;
    int i;
    for (i = 0; i < narg; i++)
    {
        child = va_arg(ap, TreeNode *);
        if (i == 0)//the lineno of root equals to that of its first child
            root->lineno = child->lineno;
        addChild(root, child);
    }
    va_end(ap);

    return root;
}

TreeNode *newNode()
{
    TreeNode *p = (TreeNode *)malloc(sizeof(TreeNode));
    p->firstchild = p->nextsibling = NULL;
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

void printTree(TreeNode *root, int depth)
{
    int i;
    for (i = 0; i < 2 * depth; i++) printf(" ");//spaces in front of each line
    printf("%s", root->name);
    if (root->firstchild == NULL)//leaves
    {
        if (strcmp(root->name, "INT") == 0)
            printf(": %d", root->intvalue);
        else if (strcmp(root->name, "FLOAT") == 0)
            printf(": %lf", root->floatvalue);
        else if (strcmp(root->name, "ID") == 0 || strcmp(root->name, "TYPE") == 0)
            printf(": %s", root->morpheme);
    }else
        printf(" (%d)", root->lineno);
    printf("\n");

    TreeNode *p = root->firstchild;
    while(p)//preorder traversal
    {
        printTree(p, depth + 1);
        p = p->nextsibling;
    }
}
