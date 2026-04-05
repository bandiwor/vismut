#include "ast_builder.h"
#include "ast.h"
#include <assert.h>

ASTBuilder ASTBuilder_Create(VismutTokenizer *tokenizer) {
    return (ASTBuilder){
        .tokenizer = tokenizer,
        .arena = tokenizer->arena,
        .nodes = NULL,
        .nodes_capacity = 0,
        .nodes_length = 0,
    };
}

static u32 ASTBuilder_CalculateNodesCapacity(const u32 old_capacity) {
    if (old_capacity <= 32) {
        return 32;
    } else {
        return old_capacity * 2;
    }
}

attribute_nodiscard static VismutErrorType ASTBuilder_MemoryExtend(ASTBuilder *builder) {
    const u32 new_capacity = ASTBuilder_CalculateNodesCapacity(builder->nodes_capacity);
    ASTNode *new_nodes = __builtin_realloc(builder->nodes, new_capacity);
    if (new_nodes == NULL) {
        return VISMUT_ERROR_OUT_OF_MEMORY;
    }
    builder->nodes_capacity = new_capacity;
    builder->nodes = new_nodes;

    return VISMUT_ERROR_OK;
}

attribute_nodiscard VismutErrorType ASTBuilder_PushNode(ASTBuilder *restrict builder,
                                                        const ASTNode *restrict node) {
    VismutErrorType err;
    if (unlikely(builder->nodes_capacity <= builder->nodes_length)) {
        SAFE_RISKY_EXPRESSION(ASTBuilder_MemoryExtend(builder), err);
    }

    builder->nodes[builder->nodes_length] = *node;
    ++builder->nodes_length;

    return VISMUT_ERROR_OK;
}

attribute_nodiscard VismutErrorType ASTBuilder_NextToken(ASTBuilder *restrict builder,
                                                         VismutToken *restrict out_token) {
    return VismutTokenizer_Next(builder->tokenizer, out_token);
}

void ASTBuilder_LinkNodes(ASTBuilder *builder, const ASTNodeIdx prev, const ASTNodeIdx next) {
    ASTNode *prev_node = &builder->nodes[prev];

    switch (prev_node->type) {
    case VISMUT_AST_EXPRESSION:
        prev_node->expression.next_expr = next;
        break;
    default:
        assert(0 && "This node type cannot be linked as a sequence");
    }
}
