#include "scope.h"

VismutScope VismutScope_Create(VismutScope *parent) {
    return (VismutScope){
        .parent = parent,
        .node = NULL,
    };
}

attribute_pure VismutScopeNode *VismutScope_Find(VismutScope *scope, const StringView name) {
    for (const VismutScope *current_scope = scope; current_scope != NULL;
         current_scope = current_scope->parent) {
        for (VismutScopeNode *current_node = current_scope->node; current_node != NULL;
             current_node = current_node->prev) {
            if (StringView_Equals(name, current_node->symbol->name)) {
                return current_node;
            }
        }
    }

    return NULL;
}

void VismutScope_InsertNode(VismutScope *scope, VismutScopeNode *node) {
    node->prev = scope->node;
    scope->node = node;
}

VismutScopeNode VismutScopeNode_Create(VismutSymbol *symbol) {
    return (VismutScopeNode){
        .symbol = symbol,
        .prev = NULL,
    };
}
