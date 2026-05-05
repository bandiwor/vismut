#include "types.h"

const VismutType *VismutType_FromTypeToken(const VismutTypeContext *ctx,
                                           const VismutTokenType type) {
    switch (type) {
    case VISMUT_TOKEN_I8_TYPE:
        return ctx->type_i8;
    case VISMUT_TOKEN_I16_TYPE:
        return ctx->type_i16;
    case VISMUT_TOKEN_I32_TYPE:
        return ctx->type_i32;
    case VISMUT_TOKEN_I64_TYPE:
        return ctx->type_i64;
    case VISMUT_TOKEN_U8_TYPE:
        return ctx->type_u8;
    case VISMUT_TOKEN_U16_TYPE:
        return ctx->type_u16;
    case VISMUT_TOKEN_U32_TYPE:
        return ctx->type_u32;
    case VISMUT_TOKEN_U64_TYPE:
        return ctx->type_i64;
    case VISMUT_TOKEN_STR_TYPE:
        return ctx->type_str;
    case VISMUT_TOKEN_F32_TYPE:
        return ctx->type_f32;
    case VISMUT_TOKEN_F64_TYPE:
        return ctx->type_f64;
    default:
        return NULL;
    }
}

attribute_const int VismutTypeKind_IsInt(const VismutTypeKind kind) {
    switch (kind) {
    case VISMUT_TYPE_KIND_I1:
    case VISMUT_TYPE_KIND_I8:
    case VISMUT_TYPE_KIND_I16:
    case VISMUT_TYPE_KIND_I32:
    case VISMUT_TYPE_KIND_I64:
    case VISMUT_TYPE_KIND_U8:
    case VISMUT_TYPE_KIND_U16:
    case VISMUT_TYPE_KIND_U32:
    case VISMUT_TYPE_KIND_U64:
        return 1;
    default:
        return 0;
    }
}

attribute_const int VismutTypeKind_IsUInt(const VismutTypeKind kind) {
    switch (kind) {
    case VISMUT_TYPE_KIND_U8:
    case VISMUT_TYPE_KIND_U16:
    case VISMUT_TYPE_KIND_U32:
    case VISMUT_TYPE_KIND_U64:
        return 1;
    default:
        return 0;
    }
}

attribute_const int VismutTypeKind_IsFloat(const VismutTypeKind kind) {
    return kind == VISMUT_TYPE_KIND_F32 || kind == VISMUT_TYPE_KIND_F64;
}

attribute_const VismutTypeKind VismutTypeKind_FromIntSuffix(const VismutIntSuffix suffix) {
    switch (suffix) {
    case VISMUT_INT_SUFFIX_I1:
        return VISMUT_TYPE_KIND_I1;
    case VISMUT_INT_SUFFIX_I8:
        return VISMUT_TYPE_KIND_I8;
    case VISMUT_INT_SUFFIX_I16:
        return VISMUT_TYPE_KIND_I16;
    case VISMUT_INT_SUFFIX_I32:
        return VISMUT_TYPE_KIND_I32;
    case VISMUT_INT_SUFFIX_I64:
        return VISMUT_TYPE_KIND_I64;
    case VISMUT_INT_SUFFIX_U8:
        return VISMUT_TYPE_KIND_U8;
    case VISMUT_INT_SUFFIX_U16:
        return VISMUT_TYPE_KIND_U16;
    case VISMUT_INT_SUFFIX_U32:
        return VISMUT_TYPE_KIND_U32;
    case VISMUT_INT_SUFFIX_U64:
        return VISMUT_TYPE_KIND_U64;
    default:
        return VISMUT_TYPE_KIND_UNKNOWN;
    }
}
