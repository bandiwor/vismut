#ifndef VISMUT_COMPILE_COMPILE_CONTEXT_H
#define VISMUT_COMPILE_COMPILE_CONTEXT_H

#include "../args/parse_args.h"
#include "../core/memory/constant_pool.h"
#include "../core/memory/modules_table.h"
#include "../core/memory/type_context.h"

typedef struct {
    VismutModulesTable modules_table;
    VismutTypeContext type_context;
    ConstantPool constant_pool;
    Arena arena;
    VismutErrorInfo *error_info;
    StringView input_file;
    bool dump_sources;
    bool dump_ast;
    bool dump_bytecode;
} VismutCompileContext;

VismutErrorType VismutCompileContext_Init(VismutCompileContext *restrict ctx,
                                          const VismutCompileArgs *restrict args,
                                          VismutErrorInfo *restrict error_info,
                                          VismutErrorDetails *restrict out_details);

VismutErrorType VismutCompileContext_Compile(VismutCompileContext *ctx);

void VismutCompileContext_Free(VismutCompileContext *ctx);

#endif
