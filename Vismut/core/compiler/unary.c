#include "unary.h"

static inline i1 IsFloatType(const VismutTypeKind kind) {
    return kind == VISMUT_TYPE_KIND_F32 || kind == VISMUT_TYPE_KIND_F64 ||
           kind == VISMUT_TYPE_KIND_FLOAT;
}

static inline i1 IsSignedIntType(const VismutTypeKind kind) {
    return kind == VISMUT_TYPE_KIND_I8 || kind == VISMUT_TYPE_KIND_I16 ||
           kind == VISMUT_TYPE_KIND_I32 || kind == VISMUT_TYPE_KIND_I64 ||
           kind == VISMUT_TYPE_KIND_INT;
}

static inline i1 IsUnsignedIntType(const VismutTypeKind kind) {
    return kind == VISMUT_TYPE_KIND_U8 || kind == VISMUT_TYPE_KIND_U16 ||
           kind == VISMUT_TYPE_KIND_U32 || kind == VISMUT_TYPE_KIND_U64;
}

static inline i1 IsAnyIntType(const VismutTypeKind kind) {
    return IsSignedIntType(kind) || IsUnsignedIntType(kind);
}

static inline i1 IsBoolType(const VismutTypeKind kind) {
    return kind == VISMUT_TYPE_KIND_I1;
}

VismutErrorType VismutCompiler_GetUnaryInstruction(const ASTUnaryNodeType op,
                                                   const VismutType *restrict right_t,
                                                   VismutOpcode *restrict out_opcode,
                                                   VismutErrorDetails *restrict out_details) {
    const i1 is_i1 = IsBoolType(right_t->kind);
    const i1 is_int = IsAnyIntType(right_t->kind);
    const i1 is_signed_int = IsSignedIntType(right_t->kind);
    const i1 is_float = IsFloatType(right_t->kind);

    switch (op) {
    case VISMUT_AST_UNARY_LOGICAL_NOT:
        if (!is_i1) {
            *out_details = (VismutErrorDetails){.unary = {
                                                    .right = right_t->kind,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_UNARY;
        }
        *out_opcode = OP_NOT;
        break;

    case VISMUT_AST_UNARY_MINUS:
        if (is_signed_int) {
            *out_opcode = OP_NEG_I;
        } else if (is_float) {
            *out_opcode = OP_NEG_F;
        } else {
            *out_details = (VismutErrorDetails){.unary = {
                                                    .right = right_t->kind,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_UNARY;
        }
        break;

    case VISMUT_AST_UNARY_PLUS:
        return VISMUT_ERR_UNSUPPORTED_UNARY;

    case VISMUT_AST_UNARY_BITWISE_NOT:
        if (!is_int) {
            *out_details = (VismutErrorDetails){.unary = {
                                                    .right = right_t->kind,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_UNARY;
        }
        *out_opcode = OP_BNOT;
        break;

    default:
        return VISMUT_ERR_UNREACHABLE;
    }

    return VISMUT_OK;
}
