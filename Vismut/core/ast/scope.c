#include "scope.h"

attribute_const VismutScope VismutScope_Create(VismutScope *parent) {
    return (VismutScope){
        .parent = parent,
        .node = NULL,
    };
}

attribute_pure attribute_nonnull(1, 2) VismutScopeNode *VismutScope_Find(const VismutScope *scope,
                                                                         const StringNode *name) {
    for (const VismutScope *current_scope = scope; current_scope != NULL;
         current_scope = current_scope->parent) {
        for (VismutScopeNode *current_node = scope->node; current_node != NULL;
             current_node = current_node->prev) {
            if (current_node->name == name) {
                return current_node;
            }
        }
    }

    return NULL;
}

attribute_nonnull(1, 2) void VismutScope_InsertNode(VismutScope *scope, VismutScopeNode *node) {
    node->prev = scope->node;
    scope->node = node;
}

attribute_nonnull(1, 2) VismutScopeNode VismutScopeNode_Create(StringNode *name, VismutType *type) {
    return (VismutScopeNode){
        .name = name,
        .type = type,
        .prev = NULL,
    };
}
