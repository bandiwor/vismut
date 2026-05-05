#ifndef VISMUT_COR_AST_AST_PARSER_H
#define VISMUT_COR_AST_AST_PARSER_H
#include "../ast/ast.h"
#include "../ast/ast_builder.h"
#include "../memory/modules_table.h"
#include "../memory/symbol_table.h"

typedef struct {
    i8 is_in_function_declaration;
    i8 is_exported_decl;
} ASTParserContext;

typedef struct {
    ASTBuilder *builder;
    Arena *arena;
    VismutTypeContext *type_ctx;
    SymbolsTable *symbols_table;
    VismutModulesTable *modules_table;
    VismutModuleIdx current_module_idx;
    VismutErrorInfo *error_info;
    ASTNodeIdx module_node;
    VismutToken current_token;
    ASTParserContext ctx;
} ASTParser;

ASTParser ASTParser_Create(ASTBuilder *restrict builder, VismutTypeContext *restrict type_ctx,
                           SymbolsTable *restrict symbols_table,
                           VismutModulesTable *restrict modules_table,
                           VismutModuleIdx current_module_idx);

attribute_nodiscard attribute_nonnull(1) VismutErrorType ASTParser_Parse(ASTParser *);

#endif
