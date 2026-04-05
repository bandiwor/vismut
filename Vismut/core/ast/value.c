#include "value.h"

const u8 *VismutTypeKind_String(const VismutTypeKind type) {
#define X(name, text) [name] = (const u8 *)text,
    static const u8 *const codes_table[] = {X_VISMUT_TYPE_KIND(X)};
#undef X

    if (unlikely(type >= VISMUT_TYPE_KIND_COUNT || type < 0)) {
        return codes_table[VISMUT_TYPE_KIND_UNKNOWN];
    }

    return codes_table[type];
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
    if (kind == VISMUT_TYPE_KIND_POINTER) {
        return a->pointer.element_type == element;
    }
    if (kind == VISMUT_TYPE_KIND_ARRAY) {
        return a->array.element_type == element && a->array.size == size;
    }
    if (kind == VISMUT_TYPE_KIND_VECTOR) {
        return a->vector.element_type == element;
    }
    return 1;
}

static const VismutType *Internal_AllocPrimitive(VismutTypeContext *ctx,
                                                 const VismutTypeKind kind) {
    VismutType *t = (VismutType *)Arena_Type(&ctx->arena, VismutType);
    *t = (VismutType){
        .kind = kind,
    };
    return (const VismutType *)t;
}

attribute_nonnull(1) attribute_nodiscard VismutErrorType
    VismutTypeContext_Init(VismutTypeContext *ctx, const u32 table_capacity) {
    ctx->arena = Arena_Create();
    ctx->capacity = table_capacity;
    ctx->buckets = Arena_Array(&ctx->arena, VismutTypeNode *, table_capacity);
    if (ctx->buckets == NULL) {
        return VISMUT_ERROR_OUT_OF_MEMORY;
    }

    __builtin_memset(ctx->buckets, 0, table_capacity * sizeof(VismutTypeNode *));

    ctx->type_void = Internal_AllocPrimitive(ctx, VISMUT_TYPE_KIND_VOID);
    ctx->type_auto = Internal_AllocPrimitive(ctx, VISMUT_TYPE_KIND_AUTO);
    ctx->type_i1 = Internal_AllocPrimitive(ctx, VISMUT_TYPE_KIND_I1);
    ctx->type_i8 = Internal_AllocPrimitive(ctx, VISMUT_TYPE_KIND_I8);
    ctx->type_i16 = Internal_AllocPrimitive(ctx, VISMUT_TYPE_KIND_I16);
    ctx->type_i32 = Internal_AllocPrimitive(ctx, VISMUT_TYPE_KIND_I32);
    ctx->type_i64 = Internal_AllocPrimitive(ctx, VISMUT_TYPE_KIND_I64);
    ctx->type_u8 = Internal_AllocPrimitive(ctx, VISMUT_TYPE_KIND_U8);
    ctx->type_u16 = Internal_AllocPrimitive(ctx, VISMUT_TYPE_KIND_U16);
    ctx->type_u32 = Internal_AllocPrimitive(ctx, VISMUT_TYPE_KIND_U32);
    ctx->type_u64 = Internal_AllocPrimitive(ctx, VISMUT_TYPE_KIND_U64);
    ctx->type_int = Internal_AllocPrimitive(ctx, VISMUT_TYPE_KIND_INT);
    ctx->type_f32 = Internal_AllocPrimitive(ctx, VISMUT_TYPE_KIND_F32);
    ctx->type_f64 = Internal_AllocPrimitive(ctx, VISMUT_TYPE_KIND_F64);
    ctx->type_float = Internal_AllocPrimitive(ctx, VISMUT_TYPE_KIND_FLOAT);
    ctx->type_str = Internal_AllocPrimitive(ctx, VISMUT_TYPE_KIND_STR);

    return VISMUT_ERROR_OK;
}

attribute_nonnull(1, 2, 3) attribute_nodiscard VismutErrorType
    VismutTypeContext_GetPointer(VismutTypeContext *restrict ctx,
                                 const VismutType *restrict element_type,
                                 const VismutType *restrict *restrict out_type) {
    const u32 hash = HashType(VISMUT_TYPE_KIND_POINTER, element_type, 0);
    const u32 index = hash % ctx->capacity;

    VismutTypeNode *node = ctx->buckets[index];
    while (node) {
        if (TypesAreMatching(node->type, VISMUT_TYPE_KIND_POINTER, element_type, 0)) {
            *out_type = node->type;
            return VISMUT_ERROR_OK;
        }
        node = node->next;
    }

    VismutType *new_type = (VismutType *)Arena_Type(&ctx->arena, VismutType);
    if (!new_type)
        return VISMUT_ERROR_OUT_OF_MEMORY;

    *new_type = (VismutType){
        .kind = VISMUT_TYPE_KIND_POINTER,
        .pointer =
            {
                .element_type = element_type,
            },
    };

    VismutTypeNode *new_node = (VismutTypeNode *)Arena_Type(&ctx->arena, VismutTypeNode);
    if (!new_node)
        return VISMUT_ERROR_OUT_OF_MEMORY;

    *new_node = (VismutTypeNode){
        .type = new_type,
        .next = ctx->buckets[index],
    };

    ctx->buckets[index] = new_node;

    *out_type = new_type;
    return VISMUT_ERROR_OK;
}

attribute_nonnull(1, 2, 4) attribute_nodiscard VismutErrorType
    VismutTypeContext_GetArray(VismutTypeContext *restrict ctx,
                               const VismutType *restrict element_type, const u32 size,
                               const VismutType *restrict *restrict out_type) {
    const u32 hash = HashType(VISMUT_TYPE_KIND_ARRAY, element_type, size);
    const u32 index = hash % ctx->capacity;

    VismutTypeNode *node = ctx->buckets[index];
    while (node) {
        if (TypesAreMatching(node->type, VISMUT_TYPE_KIND_ARRAY, element_type, size)) {
            *out_type = node->type;
            return VISMUT_ERROR_OK;
        }
        node = node->next;
    }

    VismutType *new_type = (VismutType *)Arena_Type(&ctx->arena, VismutType);
    if (!new_type)
        return VISMUT_ERROR_OUT_OF_MEMORY;

    *new_type = (VismutType){
        .kind = VISMUT_TYPE_KIND_ARRAY,
        .array =
            {
                .element_type = element_type,
                .size = size,
            },
    };

    VismutTypeNode *new_node = (VismutTypeNode *)Arena_Type(&ctx->arena, VismutTypeNode);
    *new_node = (VismutTypeNode){
        .type = new_type,
        .next = ctx->buckets[index],
    };

    ctx->buckets[index] = new_node;

    *out_type = new_type;
    return VISMUT_ERROR_OK;
}

attribute_nonnull(1, 2, 3) attribute_nodiscard VismutErrorType
    VismutTypeContext_GetVector(VismutTypeContext *restrict ctx,
                                const VismutType *restrict element_type,
                                const VismutType *restrict *restrict out_type) {
    const u32 hash = HashType(VISMUT_TYPE_KIND_VECTOR, element_type, 0);
    const u32 index = hash % ctx->capacity;

    VismutTypeNode *node = ctx->buckets[index];
    while (node) {
        if (TypesAreMatching(node->type, VISMUT_TYPE_KIND_VECTOR, element_type, 0)) {
            *out_type = node->type;
            return VISMUT_ERROR_OK;
        }
        node = node->next;
    }

    VismutType *new_type = (VismutType *)Arena_Type(&ctx->arena, VismutType);
    if (!new_type)
        return VISMUT_ERROR_OUT_OF_MEMORY;

    *new_type = (VismutType){
        .kind = VISMUT_TYPE_KIND_VECTOR,
        .vector =
            {
                .element_type = element_type,
            },
    };

    VismutTypeNode *new_node = (VismutTypeNode *)Arena_Type(&ctx->arena, VismutTypeNode);
    *new_node = (VismutTypeNode){
        .type = new_type,
        .next = ctx->buckets[index],
    };

    ctx->buckets[index] = new_node;

    *out_type = new_type;
    return VISMUT_ERROR_OK;
}

attribute_nonnull(1, 2, 3, 5) attribute_nodiscard VismutErrorType
    VismutTypeContext_GetFunction(VismutTypeContext *restrict ctx,
                                  const VismutType *restrict return_type,
                                  const VismutType *restrict *restrict params,
                                  const u32 param_count,
                                  const VismutType *restrict *restrict out_type) {
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
                return VISMUT_ERROR_OK;
            }
        }
        node = node->next;
    }

    VismutType *new_func = Arena_Type(&ctx->arena, VismutType);
    if (!new_func)
        return VISMUT_ERROR_OUT_OF_MEMORY;

    const VismutTypeKind kind = VISMUT_TYPE_KIND_FUNCTION;
    const VismutType **param_types = NULL;

    if (param_count > 0) {
        const u64 params_size = sizeof(VismutType *) * param_count;
        const VismutType **allocated_params =
            Arena_Array(&ctx->arena, const VismutType *, params_size);
        if (allocated_params == NULL) {
            return VISMUT_ERROR_OUT_OF_MEMORY;
        }

        __builtin_memcpy(allocated_params, (const void *restrict)params, params_size);

        param_types = allocated_params;
    }

    *new_func = (VismutType){
        .kind = kind,
        .function =
            {
                .return_type = return_type,
                .param_count = param_count,
                .param_types = param_types,
            },
    };

    VismutTypeNode *new_node = Arena_Type(&ctx->arena, VismutTypeNode);
    *new_node = (VismutTypeNode){
        .type = new_func,
        .next = ctx->buckets[index],
    };
    ctx->buckets[index] = new_node;

    *out_type = new_func;
    return VISMUT_ERROR_OK;
}

attribute_nonnull(1) attribute_nodiscard VismutErrorType
    VismutTypeContext_GetTuple(VismutTypeContext *restrict ctx,
                               const VismutType *restrict *restrict fields, const u32 fields_count,
                               const VismutType *restrict *restrict out_type) {
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
                return VISMUT_ERROR_OK;
            }
        }
        node = node->next;
    }

    const VismutType **allocated_fields = NULL;
    if (fields_count > 0) {
        const u64 fields_size = sizeof(VismutType *) * fields_count;
        allocated_fields = Arena_Array(&ctx->arena, const VismutType *, fields_size);

        if (!allocated_fields) {
            return VISMUT_ERROR_OUT_OF_MEMORY;
        }

        __builtin_memcpy(allocated_fields, (const void *restrict)fields, fields_size);
    }

    VismutType *new_tuple = Arena_Type(&ctx->arena, VismutType);
    if (new_tuple == NULL) {
        return VISMUT_ERROR_OUT_OF_MEMORY;
    }

    *new_tuple = (VismutType){
        .kind = VISMUT_TYPE_KIND_TUPLE,
        .tuple =
            {
                .fields_count = fields_count,
                .fields = allocated_fields,
            },
    };

    VismutTypeNode *new_node = Arena_Type(&ctx->arena, VismutTypeNode);
    if (!new_node) {
        return VISMUT_ERROR_OUT_OF_MEMORY;
    }

    *new_node = (VismutTypeNode){
        .type = new_tuple,
        .next = ctx->buckets[index],
    };
    ctx->buckets[index] = new_node;

    *out_type = new_tuple;
    return VISMUT_ERROR_OK;
}
