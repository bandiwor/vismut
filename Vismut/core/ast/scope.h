#ifndef VISMUT_CORE_AST_SCOPE_H
#define VISMUT_CORE_AST_SCOPE_H
#include "../memory/string_pool.h"
#include "value.h"

typedef struct VismutScope VismutScope;

typedef struct VismutScopeNode VismutScopeNode;

struct VismutScopeNode {
    StringNode *name;
    VismutType *type;
    VismutScopeNode *prev;
};

struct VismutScope {
    VismutScope *parent;
    VismutScopeNode *node;
};

attribute_const VismutScope VismutScope_Create(VismutScope *parent);

attribute_nonnull(1, 2) VismutScopeNode VismutScopeNode_Create(StringNode *name, VismutType *type);

attribute_nonnull(1, 2) void VismutScope_InsertNode(VismutScope *scope, VismutScopeNode *node);

attribute_pure attribute_nonnull(1, 2) VismutScopeNode *VismutScope_Find(const VismutScope *scope,
                                                                         const StringNode *name);

#endif
