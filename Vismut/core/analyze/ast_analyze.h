#ifndef VISMUT_CORE_ANALYZE_AST_ANALYZE_H
#define VISMUT_CORE_ANALYZE_AST_ANALYZE_H
#include "../ast/ast_builder.h"
#include "../defines.h"
#include "../errors/errors.h"
#include "../memory/modules_table.h"
#include "../memory/symbol_table.h"

typedef struct {
    const VismutType *help_type;
    VismutScope *current_scope;
    const VismutType *expected_return_type;
    i8 in_function;
    i8 in_block;
    i8 is_exported_decl;
} ASTAnalyzerState;

typedef struct {
    StringView source;
    ASTBuilder *builder;
    Arena *arena;
    VismutTypeContext *type_context;
    SymbolsTable *symbols_table;
    VismutModulesTable *modules;
    VismutErrorInfo *error_info;
    ASTNodeIdx module;
    ASTAnalyzerState state;
} ASTAnalyzer;

ASTAnalyzer ASTAnalyzer_Create(ASTBuilder *restrict builder, Arena *restrict arena,
                               VismutTypeContext *restrict type_context,
                               SymbolsTable *restrict symbols_table, VismutModulesTable *modules,
                               VismutErrorInfo *restrict error_info, StringView source,
                               ASTNodeIdx module);

attribute_nodiscard VismutErrorType ASTAnalyzer_TypeAnalyze(ASTAnalyzer *ctx);

#endif
