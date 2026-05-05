#ifndef VISMUT_CORE_AST_AST_BUILDER_H
#define VISMUT_CORE_AST_AST_BUILDER_H
#include "../memory/raw_vector.h"
#include "../tokenizer/tokenizer.h"
#include "ast.h"

typedef struct {
    VismutTokenizer *tokenizer;
    RawVector nodes_vector;
} ASTBuilder;

ASTBuilder ASTBuilder_Create(VismutTokenizer *);

attribute_nodiscard VismutErrorType ASTBuilder_PushNode(ASTBuilder *restrict builder,
                                                        const ASTNode *restrict node,
                                                        ASTNodeIdx *restrict out_node,
                                                        VismutErrorDetails *restrict out_details);

attribute_nodiscard VismutErrorType ASTBuilder_NextToken(ASTBuilder *restrict builder,
                                                         VismutToken *restrict out_token);

void ASTBuilder_LinkNodes(ASTBuilder *builder, ASTNodeIdx prev, ASTNodeIdx next);

attribute_aligned(_Alignof(ASTNode)) attribute_returns_nonnull attribute_always_inline
    static inline ASTNode *ASTBuilder_NodeAt(ASTBuilder *restrict builder, ASTNodeIdx index) {
    ASTNode *nodes = builder->nodes_vector.memory;
    return &nodes[index];
}

attribute_aligned(_Alignof(ASTNode)) attribute_returns_nonnull attribute_always_inline
    static inline const ASTNode *ASTBuilder_ConstNodeAt(const ASTBuilder *restrict builder,
                                                        ASTNodeIdx index) {
    const ASTNode *nodes = builder->nodes_vector.memory;
    return &nodes[index];
}

#endif
