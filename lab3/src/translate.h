#ifndef __TRANSLATE_H__
#define __TRANSLATE_H__

# include "symbol.h"
# include "intercode.h"
# include "partree.h"

void initOperandBasic();
int typeSize(Type*);
InterCodes* translateCompst(TreeNode*, Func*);
InterCodes* translateDefList(TreeNode*);
InterCodes* translateDef(TreeNode*);
InterCodes* translateDecList(TreeNode*);
