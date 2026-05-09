#include "compile_context.h"
#include "../core/analyze/ast_analyze.h"
#include "../core/compiler/compiler.h"
#include "../core/printer/ast_printer.h"
#include "../core/printer/instructions_printer.h"
#include <stdio.h>

VismutErrorType VismutCompileContext_Init(VismutCompileContext *restrict ctx,
                                          const VismutCompileArgs *restrict args,
                                          VismutErrorInfo *restrict error_info,
                                          VismutErrorDetails *restrict out_details) {
    VismutErrorType err;

    __builtin_memset(ctx, 0, sizeof(VismutCompileContext));

    ctx->input_file = args->input_file;
    ctx->error_info = error_info;
    ctx->dump_sources = !args->no_print_inputs;
    ctx->dump_ast = !args->no_print_ast;
    ctx->dump_bytecode = true;

    ctx->arena = Arena_Create();

    SAFE_RISKY_EXPRESSION(VismutTypeContext_Init(&ctx->type_context, 512, out_details), err);

    ctx->constant_pool = ConstantPool_Create();
    SAFE_RISKY_EXPRESSION(ConstantPool_Init(&ctx->constant_pool, out_details), err);

    ctx->modules_table = VismutModulesTable_Create();
    SAFE_RISKY_EXPRESSION(VismutModulesTable_Init(&ctx->modules_table, out_details), err);

    return VISMUT_OK;
}

VismutErrorType VismutCompileContext_Compile(VismutCompileContext *ctx) {
    VismutErrorType err;
    VismutErrorDetails details = {0};
    VismutModuleIdx main_idx;

    SAFE_RISKY_EXPRESSION(VismutModulesTable_FindOrLoad(
                              &ctx->modules_table, StringView_FromCStr("main"), ctx->input_file,
                              &ctx->type_context, ctx->error_info, &main_idx, &details),
                          err);

    VismutModuleIdx *ordered_modules = NULL;
    u32 order_count = 0;

    SAFE_RISKY_EXPRESSION(VismutModulesTable_GetInitOrder(&ctx->modules_table, main_idx,
                                                          &ordered_modules, &order_count, &details),
                          err);

    for (u32 i = 0; i < order_count; i++) {
        VismutModuleIdx mod_idx = ordered_modules[i];
        VismutModuleEntry *mod = VismutModulesTable_ModuleAt(&ctx->modules_table, mod_idx);

        ASTAnalyzer analyzer =
            ASTAnalyzer_Create(&mod->ast_builder, &mod->arena, &ctx->type_context, &mod->symbols,
                               &ctx->modules_table, ctx->error_info, mod->source, mod->root_idx);

        SAFE_RISKY_EXPRESSION(ASTAnalyzer_TypeAnalyze(&analyzer), err);

        mod->state = VISMUT_MODULE_STATE_ANALYZED;

        if (ctx->dump_ast) {
            printf("\n--- AST for Module: %.*s [%u] ---\n", (int)mod->name.length, mod->name.data,
                   mod_idx);
            ASTPrinter printer = ASTPrinter_Create(&mod->ast_builder, stdout, 1);
            ASTPrinter_Print(&printer, mod->root_idx);
        }
    }

    VismutCompiler compiler =
        VismutCompiler_Create(&ctx->modules_table, ordered_modules, order_count, &ctx->type_context,
                              &ctx->constant_pool, ctx->error_info);

    SAFE_RISKY_EXPRESSION(VismutCompiler_Init(&compiler, &details), err);
    SAFE_RISKY_EXPRESSION(VismutCompiler_Compile(&compiler), err);

    InstructionsPrinter printer = InstructionsPrinter_Create(
        RawVector_Elements(&compiler.bytecode, VismutInstruction),
        RawVector_Count(&compiler.bytecode, VismutInstruction), &ctx->constant_pool);

    InstructionsPrinter_Print(&printer);

    return VISMUT_OK;
}

void VismutCompileContext_Free(VismutCompileContext *ctx) {
    VismutModulesTable_Destroy(&ctx->modules_table);
    VismutTypeContext_Free(&ctx->type_context);
    Arena_Destroy(&ctx->arena);
}
