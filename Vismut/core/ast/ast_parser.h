#ifndef VISMUT_COR_AST_AST_PARSER_H
#define VISMUT_COR_AST_AST_PARSER_H
#include "ast.h"
#include "ast_builder.h"

typedef struct {
    i8 is_in_function_declaration;
} ASTParserContext;

typedef struct {
    ASTBuilder *builder;
    StringPool *string_pool;
    VismutTypeContext *type_ctx;
    ASTNodeIdx module_node;
    VismutToken current_token;
    ASTParserContext ctx;
} ASTParser;

ASTParser ASTParser_Create(ASTBuilder *restrict builder, StringPool *restrict string_pool,
                           VismutTypeContext *restrict type_ctx);

attribute_nodiscard attribute_nonnull(1) VismutErrorType ASTParser_Parse(ASTParser *);

#endif
