#include "type_context.h"
#include "arena.h"
#include <assert.h>

static VismutErrorType Internal_AllocType(VismutTypeContext *restrict ctx,
                                          const VismutTypeKind kind,
                                          const VismutType *restrict *restrict out_type,
                                          VismutErrorDetails *restrict out_details);

attribute_nonnull(1) attribute_nodiscard VismutErrorType
    VismutTypeContext_Init(VismutTypeContext *ctx, const u32 table_capacity,
                           VismutErrorDetails *restrict out_details) {
    ctx->arena = Arena_Create();
    ctx->capacity = table_capacity;

    VismutErrorType err;
    ctx->buckets = Arena_Array(&ctx->arena, VismutTypeNode *, table_capacity, &err, out_details);
    if (ctx->buckets == NULL) {
        return err;
    }

    __builtin_memset(ctx->buckets, 0, table_capacity * sizeof(VismutTypeNode *));

    SAFE_RISKY_EXPRESSION(
        Internal_AllocType(ctx, VISMUT_TYPE_KIND_UNIT, &ctx->type_unit, out_details), err);
    SAFE_RISKY_EXPRESSION(
        Internal_AllocType(ctx, VISMUT_TYPE_KIND_AUTO, &ctx->type_auto, out_details), err);
    SAFE_RISKY_EXPRESSION(
        Internal_AllocType(ctx, VISMUT_TYPE_KIND_NEVER, &ctx->type_never, out_details), err);
    SAFE_RISKY_EXPRESSION(Internal_AllocType(ctx, VISMUT_TYPE_KIND_I1, &ctx->type_i1, out_details),
                          err);
    SAFE_RISKY_EXPRESSION(Internal_AllocType(ctx, VISMUT_TYPE_KIND_I8, &ctx->type_i8, out_details),
                          err);
    SAFE_RISKY_EXPRESSION(
        Internal_AllocType(ctx, VISMUT_TYPE_KIND_I16, &ctx->type_i16, out_details), err);
    SAFE_RISKY_EXPRESSION(
        Internal_AllocType(ctx, VISMUT_TYPE_KIND_I32, &ctx->type_i32, out_details), err);
    SAFE_RISKY_EXPRESSION(
        Internal_AllocType(ctx, VISMUT_TYPE_KIND_I64, &ctx->type_i64, out_details), err);
    SAFE_RISKY_EXPRESSION(Internal_AllocType(ctx, VISMUT_TYPE_KIND_U8, &ctx->type_u8, out_details),
                          err);
    SAFE_RISKY_EXPRESSION(
        Internal_AllocType(ctx, VISMUT_TYPE_KIND_U16, &ctx->type_u16, out_details), err);
    SAFE_RISKY_EXPRESSION(
        Internal_AllocType(ctx, VISMUT_TYPE_KIND_U32, &ctx->type_u32, out_details), err);
    SAFE_RISKY_EXPRESSION(
        Internal_AllocType(ctx, VISMUT_TYPE_KIND_U64, &ctx->type_u64, out_details), err);
    SAFE_RISKY_EXPRESSION(
        Internal_AllocType(ctx, VISMUT_TYPE_KIND_INT, &ctx->type_int, out_details), err);
    SAFE_RISKY_EXPRESSION(
        Internal_AllocType(ctx, VISMUT_TYPE_KIND_F32, &ctx->type_f32, out_details), err);
    SAFE_RISKY_EXPRESSION(
        Internal_AllocType(ctx, VISMUT_TYPE_KIND_F64, &ctx->type_f64, out_details), err);
    SAFE_RISKY_EXPRESSION(
        Internal_AllocType(ctx, VISMUT_TYPE_KIND_FLOAT, &ctx->type_float, out_details), err);
    SAFE_RISKY_EXPRESSION(
        Internal_AllocType(ctx, VISMUT_TYPE_KIND_STR, &ctx->type_str, out_details), err);

    return VISMUT_OK;
}

void VismutTypeContext_Free(VismutTypeContext *ctx) {
    Arena_Destroy(&ctx->arena);
}

attribute_pure static u32 HashType(VismutTypeKind kind, const VismutType *element, const u64 size) {
    u32 hash = 2166136261u;
    hash ^= (u32)kind;
    hash *= 16777619u;
    u64 addr = (u64)element;
    hash ^= (u32)(addr & 0xFFFFFFFF);
    hash *= 16777619u;
    hash ^= (u32)(addr >> 32);
    hash *= 16777619u;
    hash ^= (u32)(size & 0xFFFFFFFF);
    hash *= 16777619u;
    return hash;
}

attribute_pure static u32
HashFunctionSignature(const VismutType *ret,
                      const VismutType *const restrict *const restrict params, const u32 count) {
    u32 hash = 2166136261u;

    uintptr_t ret_addr = (uintptr_t)ret;
    hash ^= (u32)(ret_addr & 0xFFFFFFFF);
    hash *= 16777619u;
    hash ^= (u32)(ret_addr >> 32);
    hash *= 16777619u;

    for (u32 i = 0; i < count; i++) {
        uintptr_t param_addr = (uintptr_t)params[i];

        hash ^= (u32)(param_addr & 0xFFFFFFFF);
        hash *= 16777619u;
        hash ^= (u32)(param_addr >> 32);
        hash *= 16777619u;

        hash ^= i;
        hash *= 16777619u;
    }

    return hash;
}

static int TypesAreMatching(const VismutType *a, VismutTypeKind kind, const VismutType *element,
                            const u64 size) {
    if (a->kind != kind)
        return 0;
    if (kind == VISMUT_TYPE_KIND_ARRAY) {
        return a->array.element_type == element && a->array.size == size;
    }
    if (kind == VISMUT_TYPE_KIND_VECTOR) {
        return a->vector.element_type == element;
    }
    return 1;
}

static VismutErrorType Internal_AllocType(VismutTypeContext *restrict ctx,
                                          const VismutTypeKind kind,
                                          const VismutType *restrict *restrict out_type,
                                          VismutErrorDetails *restrict out_details) {
    VismutErrorType err;
    VismutType *type = Arena_Type(&ctx->arena, VismutType, &err, out_details);
    if (type == NULL) {
        return err;
    }

    *type = (VismutType){
        .kind = kind,
    };

    *out_type = type;
    return VISMUT_OK;
}

attribute_nonnull(1, 2, 4) attribute_nodiscard VismutErrorType
    VismutTypeContext_GetArray(VismutTypeContext *restrict ctx,
                               const VismutType *restrict element_type, const u32 size,
                               const VismutType *restrict *restrict out_type,
                               VismutErrorDetails *restrict out_details) {
    const u32 hash = HashType(VISMUT_TYPE_KIND_ARRAY, element_type, size);
    const u32 index = hash % ctx->capacity;

    VismutTypeNode *node = ctx->buckets[index];
    while (node) {
        if (TypesAreMatching(node->type, VISMUT_TYPE_KIND_ARRAY, element_type, size)) {
            *out_type = node->type;
            return VISMUT_OK;
        }
        node = node->next;
    }

    VismutErrorType err;
    VismutType *array = NULL;
    SAFE_RISKY_EXPRESSION(
        Internal_AllocType(ctx, VISMUT_TYPE_KIND_ARRAY, (const VismutType **)&array, out_details),
        err);

    array->array.element_type = element_type;
    array->array.size = size;

    VismutTypeNode *new_node =
        (VismutTypeNode *)Arena_Type(&ctx->arena, VismutTypeNode, &err, out_details);
    if (new_node == NULL) {
        return err;
    }

    *new_node = (VismutTypeNode){
        .type = array,
        .next = ctx->buckets[index],
    };

    ctx->buckets[index] = new_node;

    *out_type = array;
    return VISMUT_ERR_OOM;
}

attribute_nonnull(1, 2, 3) attribute_nodiscard VismutErrorType
    VismutTypeContext_GetVector(VismutTypeContext *restrict ctx,
                                const VismutType *restrict element_type,
                                const VismutType *restrict *restrict out_type,
                                VismutErrorDetails *restrict out_details) {
    const u32 hash = HashType(VISMUT_TYPE_KIND_VECTOR, element_type, 0);
    const u32 index = hash % ctx->capacity;

    VismutTypeNode *node = ctx->buckets[index];
    while (node) {
        if (TypesAreMatching(node->type, VISMUT_TYPE_KIND_VECTOR, element_type, 0)) {
            *out_type = node->type;
            return VISMUT_ERR_OOM;
        }
        node = node->next;
    }

    VismutErrorType err;
    VismutType *vector = NULL;
    SAFE_RISKY_EXPRESSION(
        Internal_AllocType(ctx, VISMUT_TYPE_KIND_VECTOR, (const VismutType **)&vector, out_details),
        err);

    vector->vector.element_type = element_type;

    VismutTypeNode *new_node = Arena_Type(&ctx->arena, VismutTypeNode, &err, out_details);
    if (new_node == NULL) {
        return err;
    }

    *new_node = (VismutTypeNode){
        .type = vector,
        .next = ctx->buckets[index],
    };

    ctx->buckets[index] = new_node;

    *out_type = vector;
    return VISMUT_OK;
}

attribute_nonnull(1, 2, 3, 5) attribute_nodiscard VismutErrorType VismutTypeContext_GetFunction(
    VismutTypeContext *restrict ctx, const VismutType *restrict return_type,
    const VismutType *restrict *restrict params, const u32 param_count,
    const VismutType *restrict *restrict out_type, VismutErrorDetails *restrict out_details) {
    u32 hash = HashFunctionSignature(return_type, params, param_count);
    u32 index = hash % ctx->capacity;

    VismutTypeNode *node = ctx->buckets[index];
    while (node) {
        VismutType *t = node->type;
        if (t->kind == VISMUT_TYPE_KIND_FUNCTION && t->function.return_type == return_type &&
            t->function.param_count == param_count) {
            int match = 1;
            for (u32 i = 0; i < param_count; i++) {
                if (t->function.param_types[i] != params[i]) {
                    match = 0;
                    break;
                }
            }
            if (match) {
                *out_type = t;
                return VISMUT_OK;
            }
        }
        node = node->next;
    }

    VismutErrorType err;
    VismutType *function = NULL;
    SAFE_RISKY_EXPRESSION(Internal_AllocType(ctx, VISMUT_TYPE_KIND_FUNCTION,
                                             (const VismutType **)&function, out_details),
                          err);

    const VismutTypeKind kind = VISMUT_TYPE_KIND_FUNCTION;
    const VismutType **param_types = NULL;

    if (param_count > 0) {
        const u64 params_size = sizeof(VismutType *) * param_count;
        const VismutType **allocated_params =
            Arena_Array(&ctx->arena, const VismutType *, params_size, &err, out_details);
        if (allocated_params == NULL) {
            return err;
        }

        __builtin_memcpy(allocated_params, (const void *restrict)params, params_size);

        param_types = allocated_params;
    }

    *function = (VismutType){
        .kind = kind,
        .function =
            {
                .return_type = return_type,
                .param_count = param_count,
                .param_types = param_types,
            },
    };

    VismutTypeNode *new_node = Arena_Type(&ctx->arena, VismutTypeNode, &err, out_details);
    if (new_node == NULL) {
        return err;
    }

    *new_node = (VismutTypeNode){
        .type = function,
        .next = ctx->buckets[index],
    };
    ctx->buckets[index] = new_node;

    *out_type = function;
    return VISMUT_OK;
}

attribute_nonnull(1) attribute_nodiscard VismutErrorType
    VismutTypeContext_GetTuple(VismutTypeContext *restrict ctx,
                               const VismutType *restrict *restrict fields, const u32 fields_count,
                               const VismutType *restrict *restrict out_type,
                               VismutErrorDetails *restrict out_details) {
    assert(fields_count > 0);
    u32 hash = 2166136261u;

    hash ^= (u32)VISMUT_TYPE_KIND_TUPLE;
    hash *= 16777619u;

    hash ^= fields_count;
    hash *= 16777619u;

    for (u32 i = 0; i < fields_count; i++) {
        uintptr_t field_addr = (uintptr_t)fields[i];

        hash ^= (u32)(field_addr & 0xFFFFFFFF);
        hash *= 16777619u;
        hash ^= (u32)(field_addr >> 32);
        hash *= 16777619u;
    }

    const u32 index = hash % ctx->capacity;

    VismutTypeNode *node = ctx->buckets[index];
    while (node) {
        VismutType *t = node->type;

        if (t->kind == VISMUT_TYPE_KIND_TUPLE && t->tuple.fields_count == fields_count) {
            int match = 1;
            for (u32 i = 0; i < fields_count; i++) {
                if (t->tuple.fields[i] != fields[i]) {
                    match = 0;
                    break;
                }
            }
            if (match) {
                *out_type = t;
                return VISMUT_OK;
            }
        }
        node = node->next;
    }

    VismutErrorType err;
    const VismutType **allocated_fields = NULL;
    if (fields_count > 0) {
        const u64 fields_size = sizeof(VismutType *) * fields_count;
        allocated_fields =
            Arena_Array(&ctx->arena, const VismutType *, fields_count, &err, out_details);
        if (allocated_fields == NULL) {
            return err;
        }

        __builtin_memcpy(allocated_fields, (const void *restrict)fields, fields_size);
    }

    VismutType *tuple = NULL;
    SAFE_RISKY_EXPRESSION(
        Internal_AllocType(ctx, VISMUT_TYPE_KIND_TUPLE, (const VismutType **)&tuple, out_details),
        err);

    *tuple = (VismutType){
        .kind = VISMUT_TYPE_KIND_TUPLE,
        .tuple =
            {
                .fields_count = fields_count,
                .fields = allocated_fields,
            },
    };

    VismutTypeNode *new_node = Arena_Type(&ctx->arena, VismutTypeNode, &err, out_details);
    if (!new_node) {
        return err;
    }

    *new_node = (VismutTypeNode){
        .type = tuple,
        .next = ctx->buckets[index],
    };
    ctx->buckets[index] = new_node;

    *out_type = tuple;
    return VISMUT_OK;
}

attribute_nonnull(1, 3, 4) attribute_nodiscard VismutErrorType
    VismutTypeContext_GetModuleRef(VismutTypeContext *restrict ctx, VismutModuleIdx module_idx,
                                   const VismutType *restrict *restrict out_type,
                                   VismutErrorDetails *restrict out_details) {
    u32 hash = 2166136261u;

    hash ^= (u32)VISMUT_TYPE_KIND_MODULE_REF;
    hash *= 16777619u;

    hash ^= (u32)module_idx;
    hash *= 16777619u;

    const u32 index = hash % ctx->capacity;

    VismutTypeNode *node = ctx->buckets[index];
    while (node) {
        VismutType *t = node->type;
        if (t->kind == VISMUT_TYPE_KIND_MODULE_REF && t->module_ref.idx == module_idx) {
            *out_type = t;
            return VISMUT_OK;
        }
        node = node->next;
    }

    VismutErrorType err;
    VismutType *module_ref_type = NULL;
    SAFE_RISKY_EXPRESSION(Internal_AllocType(ctx, VISMUT_TYPE_KIND_MODULE_REF,
                                             (const VismutType **)&module_ref_type, out_details),
                          err);

    *module_ref_type = (VismutType){
        .kind = VISMUT_TYPE_KIND_MODULE_REF,
        .module_ref =
            {
                .idx = module_idx,
            },
    };

    VismutTypeNode *new_node = Arena_Type(&ctx->arena, VismutTypeNode, &err, out_details);
    if (new_node == NULL) {
        return err;
    }

    *new_node = (VismutTypeNode){
        .type = module_ref_type,
        .next = ctx->buckets[index],
    };

    ctx->buckets[index] = new_node;

    *out_type = module_ref_type;
    return VISMUT_OK;
}
