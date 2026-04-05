#ifndef VISMUT_CORE_AST_AST_BUILDER_H
#define VISMUT_CORE_AST_AST_BUILDER_H
#include "../tokenizer/tokenizer.h"
#include "ast.h"

typedef struct {
    ASTNode *nodes;
    VismutTokenizer *tokenizer;
    Arena *arena;
    u32 nodes_length;
    u32 nodes_capacity;
} ASTBuilder;

ASTBuilder ASTBuilder_Create(VismutTokenizer *);

attribute_nodiscard VismutErrorType ASTBuilder_PushNode(ASTBuilder *restrict builder,
                                                        const ASTNode *restrict node);

attribute_nodiscard VismutErrorType ASTBuilder_NextToken(ASTBuilder *restrict builder,
                                                         VismutToken *restrict out_token);

void ASTBuilder_LinkNodes(ASTBuilder *builder, ASTNodeIdx prev, ASTNodeIdx next);

#endif
