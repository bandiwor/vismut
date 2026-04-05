#ifndef VISMUT_CORE_AST_VALUE_H
#define VISMUT_CORE_AST_VALUE_H
#include "../errors/errors.h"
#include "../memory/arena.h"
#include "../memory/string_pool.h"
#include "../types.h"

#define X_VISMUT_TYPE_KIND(X)                                                                      \
    X(VISMUT_TYPE_KIND_VOID, "void")                                                               \
    X(VISMUT_TYPE_KIND_AUTO, "auto")                                                               \
    X(VISMUT_TYPE_KIND_I1, "i1")                                                                   \
    X(VISMUT_TYPE_KIND_I8, "i8")                                                                   \
    X(VISMUT_TYPE_KIND_I16, "i16")                                                                 \
    X(VISMUT_TYPE_KIND_I32, "i32")                                                                 \
    X(VISMUT_TYPE_KIND_I64, "i64")                                                                 \
    X(VISMUT_TYPE_KIND_U8, "u8")                                                                   \
    X(VISMUT_TYPE_KIND_U16, "u16")                                                                 \
    X(VISMUT_TYPE_KIND_U32, "u32")                                                                 \
    X(VISMUT_TYPE_KIND_U64, "u64")                                                                 \
    X(VISMUT_TYPE_KIND_F32, "f32")                                                                 \
    X(VISMUT_TYPE_KIND_F64, "f64")                                                                 \
    X(VISMUT_TYPE_KIND_STR, "str")                                                                 \
    X(VISMUT_TYPE_KIND_INT, "int")                                                                 \
    X(VISMUT_TYPE_KIND_FLOAT, "float")                                                             \
    X(VISMUT_TYPE_KIND_POINTER, "ptr")                                                             \
    X(VISMUT_TYPE_KIND_ARRAY, "array")                                                             \
    X(VISMUT_TYPE_KIND_VECTOR, "vector")                                                           \
    X(VISMUT_TYPE_KIND_TUPLE, "tuple")                                                             \
    X(VISMUT_TYPE_KIND_STRUCTURE, "struct")                                                        \
    X(VISMUT_TYPE_KIND_STRUCTURE_INSTANCE, "struct-of")                                            \
    X(VISMUT_TYPE_KIND_FUNCTION, "func")                                                           \
    X(VISMUT_TYPE_KIND_UNKNOWN, "unknown")

typedef enum {
#define X(name, text) name,
    X_VISMUT_TYPE_KIND(X)
#undef X
        VISMUT_TYPE_KIND_COUNT
} VismutTypeKind;

const u8 *VismutTypeKind_String(VismutTypeKind);

typedef struct VismutType VismutType;

struct VismutType {
    VismutTypeKind kind;
    union {
        struct {
            const VismutType *element_type;
        } pointer;

        struct {
            u32 size;
            const VismutType *element_type;
        } array;

        struct {
            const VismutType *element_type;
        } vector;

        struct {
            u32 fields_count;
            const VismutType **fields;
        } tuple;

        struct {
            u32 param_count;
            const VismutType *return_type;
            const VismutType **param_types;
        } function;
    };
};

typedef union {
    i64 i;
    u64 u;
    double f;
    StringNode *str;
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

    // pre-allocated types
    const VismutType *type_void;
    const VismutType *type_auto;
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
    VismutTypeContext_Init(VismutTypeContext *ctx, u32 table_capacity);

attribute_nonnull(1, 2, 3) attribute_nodiscard VismutErrorType
    VismutTypeContext_GetPointer(VismutTypeContext *restrict ctx,
                                 const VismutType *restrict element_type,
                                 const VismutType *restrict *restrict out_type);

attribute_nonnull(1, 2, 4) attribute_nodiscard VismutErrorType
    VismutTypeContext_GetArray(VismutTypeContext *restrict ctx,
                               const VismutType *restrict element_type, u32 size,
                               const VismutType *restrict *restrict out_type);

attribute_nonnull(1, 2, 3) attribute_nodiscard VismutErrorType
    VismutTypeContext_GetVector(VismutTypeContext *restrict ctx,
                                const VismutType *restrict element_type,
                                const VismutType *restrict *restrict out_type);

attribute_nonnull(1, 2, 3, 5) attribute_nodiscard VismutErrorType
    VismutTypeContext_GetFunction(VismutTypeContext *restrict ctx,
                                  const VismutType *restrict return_type,
                                  const VismutType *restrict *restrict params, u32 param_count,
                                  const VismutType *restrict *restrict out_type);

attribute_nodiscard VismutErrorType VismutTypeContext_GetTuple(
    VismutTypeContext *restrict ctx, const VismutType *restrict *restrict fields, u32 fields_count,
    const VismutType *restrict *restrict out_type);

#endif
