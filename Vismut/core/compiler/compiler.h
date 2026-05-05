#ifndef VISMUT_CORE_COMPILER_COMPILER_H
#define VISMUT_CORE_COMPILER_COMPILER_H
#include "../memory/constant_pool.h"
#include "../memory/modules_table.h"
#include "../memory/type_context.h"

typedef struct {
    VismutModuleEntry *current_module_entry;
    u16 next_global_slot;
    u8 next_free_reg;
} VismutCompilerContext;

typedef struct {
    VismutModulesTable *modules_table;
    ConstantPool *contant_pool;
    VismutTypeContext *type_context;
    VismutModuleIdx *modules_order;
    RawVector bytecode;
    RawVector deffered_functions;
    RawVector call_patches;
    VismutErrorInfo *error_info;
    u32 modules_order_size;
    VismutCompilerContext ctx;
} VismutCompiler;

VismutCompiler VismutCompiler_Create(VismutModulesTable *modules_table,
                                     VismutModuleIdx *modules_order, u32 modules_order_size,
                                     VismutTypeContext *type_context, ConstantPool *constant_pool,
                                     VismutErrorInfo *error_info);

VismutErrorType VismutCompiler_Init(VismutCompiler *restrict compiler,
                                    VismutErrorDetails *restrict out_details);

void VismutCompiler_Destroy(VismutCompiler *compiler);

VismutErrorType VismutCompiler_Compile(VismutCompiler *compiler);

#endif
