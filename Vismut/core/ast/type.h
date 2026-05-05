#ifndef VISMUT_CORE_AST_TYPE_H
#define VISMUT_CORE_AST_TYPE_H
#include "../types.h"

#define X_VISMUT_TYPE_KIND(X)                                                                      \
    X(VISMUT_TYPE_KIND_UNIT, "unit")                                                               \
    X(VISMUT_TYPE_KIND_AUTO, "auto")                                                               \
    X(VISMUT_TYPE_KIND_NEVER, "never")                                                             \
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
    X(VISMUT_TYPE_KIND_ARRAY, "array")                                                             \
    X(VISMUT_TYPE_KIND_VECTOR, "vector")                                                           \
    X(VISMUT_TYPE_KIND_TUPLE, "tuple")                                                             \
    X(VISMUT_TYPE_KIND_STRUCTURE, "struct")                                                        \
    X(VISMUT_TYPE_KIND_STRUCTURE_INSTANCE, "struct-of")                                            \
    X(VISMUT_TYPE_KIND_FUNCTION, "func")                                                           \
    X(VISMUT_TYPE_KIND_MODULE_REF, "module")                                                       \
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

        struct {
            VismutModuleIdx idx;
        } module_ref;
    };
};

#endif
