#include "ast_builder.h"
#include "ast.h"
#include <assert.h>

ASTBuilder ASTBuilder_Create(VismutTokenizer *tokenizer) {
    return (ASTBuilder){
        .tokenizer = tokenizer,
        .nodes_vector = RawVector_Create(),
    };
}

#define SYSTEM_PAGE_SIZE_BYTES 4096

attribute_nodiscard VismutErrorType ASTBuilder_PushNode(ASTBuilder *restrict builder,
                                                        const ASTNode *restrict node,
                                                        ASTNodeIdx *restrict out_idx,
                                                        VismutErrorDetails *restrict out_details) {
    VismutErrorType err;
    ASTNodeIdx new_idx = RawVector_Count(&builder->nodes_vector, ASTNode);

    RawVector_Push(&builder->nodes_vector, ASTNode, *node, err, out_details);

    *out_idx = new_idx;
    return VISMUT_OK;
}

attribute_nodiscard VismutErrorType ASTBuilder_NextToken(ASTBuilder *restrict builder,
                                                         VismutToken *restrict out_token) {
    return VismutTokenizer_Next(builder->tokenizer, out_token);
}

void ASTBuilder_LinkNodes(ASTBuilder *builder, const ASTNodeIdx prev, const ASTNodeIdx next) {
    ASTNode *prev_node = ASTBuilder_NodeAt(builder, prev);
    switch (prev_node->type) {
    case VISMUT_AST_EXPRESSION:
        prev_node->expression.next = next;
        break;
    case VISMUT_AST_DECLARATION:
        prev_node->declaration.next = next;
        break;
    default:
        assert(0 && "This node type cannot be linked as a sequence");
    }
}
