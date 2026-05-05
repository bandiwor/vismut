#ifndef VISMUT_CORE_MEMORY_TYPE_CONTEXT_H
#define VISMUT_CORE_MEMORY_TYPE_CONTEXT_H
#include "../ast/type.h"
#include "../errors/errors.h"
#include "../types.h"
#include "arena.h"

typedef union {
    i64 i;
    u64 u;
    f64 f;
    StringView str;
} VismutSimpleValue;

typedef struct VismutTypeNode VismutTypeNode;

struct VismutTypeNode {
    VismutType *type;
    VismutTypeNode *next;
};

typedef struct {
    Arena arena;
    VismutTypeNode **buckets;
    u32 capacity;

    const VismutType *type_unit;
    const VismutType *type_auto;
    const VismutType *type_never;
    const VismutType *type_i1;
    const VismutType *type_i8;
    const VismutType *type_i16;
    const VismutType *type_i32;
    const VismutType *type_i64;
    const VismutType *type_u8;
    const VismutType *type_u16;
    const VismutType *type_u32;
    const VismutType *type_u64;
    const VismutType *type_int;
    const VismutType *type_f32;
    const VismutType *type_f64;
    const VismutType *type_float;
    const VismutType *type_str;
} VismutTypeContext;

attribute_nonnull(1) attribute_nodiscard VismutErrorType
    VismutTypeContext_Init(VismutTypeContext *restrict ctx, u32 table_capacity,
                           VismutErrorDetails *restrict out_details);

void VismutTypeContext_Free(VismutTypeContext *ctx);

attribute_nonnull(1, 2, 4) attribute_nodiscard VismutErrorType
    VismutTypeContext_GetArray(VismutTypeContext *restrict ctx,
                               const VismutType *restrict element_type, u32 size,
                               const VismutType *restrict *restrict out_type,
                               VismutErrorDetails *restrict out_details);

attribute_nonnull(1, 2, 3) attribute_nodiscard VismutErrorType
    VismutTypeContext_GetVector(VismutTypeContext *restrict ctx,
                                const VismutType *restrict element_type,
                                const VismutType *restrict *restrict out_type,
                                VismutErrorDetails *restrict out_details);

attribute_nonnull(1, 2, 3, 5) attribute_nodiscard VismutErrorType
    VismutTypeContext_GetFunction(VismutTypeContext *restrict ctx,
                                  const VismutType *restrict return_type,
                                  const VismutType *restrict *restrict params, u32 param_count,
                                  const VismutType *restrict *restrict out_type,
                                  VismutErrorDetails *restrict out_details);

attribute_nonnull(1, 2, 4) attribute_nodiscard VismutErrorType
    VismutTypeContext_GetTuple(VismutTypeContext *restrict ctx,
                               const VismutType *restrict *restrict fields, u32 fields_count,
                               const VismutType *restrict *restrict out_type,
                               VismutErrorDetails *restrict out_details);

attribute_nonnull(1, 3, 4) attribute_nodiscard VismutErrorType
    VismutTypeContext_GetModuleRef(VismutTypeContext *restrict ctx, VismutModuleIdx module_idx,
                                   const VismutType *restrict *restrict out_type,
                                   VismutErrorDetails *restrict out_details);

#endif
