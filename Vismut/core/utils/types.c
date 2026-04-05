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
