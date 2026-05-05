#ifndef VISMUT_CORE_AST_SCOPE_H
#define VISMUT_CORE_AST_SCOPE_H
#include "symbol.h"

typedef struct VismutScope VismutScope;

typedef struct VismutScopeNode VismutScopeNode;

struct VismutScopeNode {
    VismutSymbol *symbol;
    VismutScopeNode *prev;
};

struct VismutScope {
    VismutScope *parent;
    VismutScopeNode *node;
};

VismutScope VismutScope_Create(VismutScope *parent);

VismutScopeNode VismutScopeNode_Create(VismutSymbol *symbol);

void VismutScope_InsertNode(VismutScope *scope, VismutScopeNode *node);

attribute_pure VismutScopeNode *VismutScope_Find(VismutScope *scope, StringView name);

#endif
