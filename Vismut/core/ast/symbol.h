#ifndef VISMUT_CORE_AST_SYMBOL_H
#define VISMUT_CORE_AST_SYMBOL_H
#include "../types.h"
#include "type.h"

typedef enum {
    VISMUT_SYMBOL_UNKNOWN = 0,
    VISMUT_SYMBOL_FUNCTION,
    VISMUT_SYMBOL_GLOBAL_VAR,
    VISMUT_SYMBOL_LOCAL_VAR,
    VISMUT_SYMBOL_CONSTANT,
    VISMUT_SYMBOL_MODULE_REF,
} SymbolKind;

typedef struct {
    StringView name;
    SymbolKind kind;
    const VismutType *type;
    i8 is_exported;

    union {
        struct {
            ASTNodeIdx declaration_node;
            u32 bytecode_offset;
            u8 registers_needed;
        } func;

        struct {
            ConstantPoolIdx cp_idx;
        } constant;

        struct {
            VismutModuleIdx idx;
        } module_ref;

        struct {
            u16 global_index;
            i1 is_mutable;
        } global_var;

        struct {
            u8 register_index;
            i1 is_mutable;
        } local_var;
    } as;
} VismutSymbol;

#endif
