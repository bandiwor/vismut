#include "binary.h"
#include <assert.h>

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

VismutErrorType VismutCompiler_GetBinaryInstruction(const ASTBinaryNodeType op,
                                                    const VismutType *restrict left_t,
                                                    const VismutType *restrict right_t,
                                                    VismutOpcode *restrict out_opcode,
                                                    VismutErrorDetails *restrict out_details) {
    if (op == VISMUT_AST_BINARY_POW) {
        if (left_t->kind == VISMUT_TYPE_KIND_F64 && right_t->kind == VISMUT_TYPE_KIND_I32) {
            *out_opcode = OP_POW_F_I;
            return VISMUT_OK;
        }
    }

    if (op == VISMUT_AST_BINARY_POW) {
        if (IsFloatType(left_t->kind) && IsAnyIntType(right_t->kind)) {
            *out_opcode = OP_POW_F_I;
            return VISMUT_OK;
        }
    }

    if (left_t->kind != right_t->kind) {
        *out_details = (VismutErrorDetails){.binary = {
                                                .left = left_t->kind,
                                                .right = right_t->kind,
                                                .op = op,
                                            }};
        return VISMUT_ERR_UNSUPPORTED_BINARY;
    }

    const VismutTypeKind k = left_t->kind;

    switch (op) {
    case VISMUT_AST_BINARY_ADD:
        if (IsAnyIntType(k))
            *out_opcode = OP_ADD_I;
        else if (IsFloatType(k))
            *out_opcode = OP_ADD_F;
        else {
            *out_details = (VismutErrorDetails){.binary = {
                                                    .left = k,
                                                    .right = k,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_BINARY;
        }
        break;

    case VISMUT_AST_BINARY_SUB:
        if (IsAnyIntType(k))
            *out_opcode = OP_SUB_I;
        else if (IsFloatType(k))
            *out_opcode = OP_SUB_F;
        else {
            *out_details = (VismutErrorDetails){.binary = {
                                                    .left = k,
                                                    .right = k,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_BINARY;
        }
        break;

    case VISMUT_AST_BINARY_MUL:
        if (IsAnyIntType(k))
            *out_opcode = OP_MUL_I;
        else if (IsFloatType(k))
            *out_opcode = OP_MUL_F;
        else {
            *out_details = (VismutErrorDetails){.binary = {
                                                    .left = k,
                                                    .right = k,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_BINARY;
        }
        break;

    case VISMUT_AST_BINARY_POW:
        if (IsAnyIntType(k))
            *out_opcode = OP_POW_I;
        else if (IsFloatType(k))
            *out_opcode = OP_POW_F;
        else {
            *out_details = (VismutErrorDetails){.binary = {
                                                    .left = k,
                                                    .right = k,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_BINARY;
        }
        break;

    case VISMUT_AST_BINARY_DIV:
        if (IsSignedIntType(k))
            *out_opcode = OP_DIV_I;
        else if (IsUnsignedIntType(k))
            *out_opcode = OP_DIV_U;
        else if (IsFloatType(k))
            *out_opcode = OP_DIV_F;
        else {
            *out_details = (VismutErrorDetails){.binary = {
                                                    .left = k,
                                                    .right = k,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_BINARY;
        }
        break;

    case VISMUT_AST_BINARY_REM_DIV:
        if (IsSignedIntType(k))
            *out_opcode = OP_REM_I;
        else if (IsUnsignedIntType(k))
            *out_opcode = OP_REM_U;
        else if (IsFloatType(k))
            *out_opcode = OP_REM_F;
        else {
            *out_details = (VismutErrorDetails){.binary = {
                                                    .left = k,
                                                    .right = k,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_BINARY;
        }
        break;

    case VISMUT_AST_BINARY_LOGICAL_OR:
        if (IsBoolType(k))
            *out_opcode = OP_OR;
        else {
            *out_details = (VismutErrorDetails){.binary = {
                                                    .left = k,
                                                    .right = k,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_BINARY;
        }
        break;

    case VISMUT_AST_BINARY_LOGICAL_AND:
        if (IsBoolType(k))
            *out_opcode = OP_AND;
        else {
            *out_details = (VismutErrorDetails){.binary = {
                                                    .left = k,
                                                    .right = k,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_BINARY;
        }
        break;

    case VISMUT_AST_BINARY_BITWISE_OR:
        if (IsAnyIntType(k))
            *out_opcode = OP_BOR;
        else {
            *out_details = (VismutErrorDetails){.binary = {
                                                    .left = k,
                                                    .right = k,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_BINARY;
        }
        break;

    case VISMUT_AST_BINARY_BITWISE_AND:
        if (IsAnyIntType(k))
            *out_opcode = OP_BAND;
        else {
            *out_details = (VismutErrorDetails){.binary = {
                                                    .left = k,
                                                    .right = k,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_BINARY;
        }
        break;

    case VISMUT_AST_BINARY_BITWISE_XOR:
        if (IsAnyIntType(k))
            *out_opcode = OP_BXOR;
        else {
            *out_details = (VismutErrorDetails){.binary = {
                                                    .left = k,
                                                    .right = k,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_BINARY;
        }
        break;

    case VISMUT_AST_BINARY_SHIFT_LEFT:
        if (IsAnyIntType(k))
            *out_opcode = OP_SHL;
        else {
            *out_details = (VismutErrorDetails){.binary = {
                                                    .left = k,
                                                    .right = k,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_BINARY;
        }
        break;

    case VISMUT_AST_BINARY_SHIFT_RIGHT:
        if (IsSignedIntType(k))
            *out_opcode = OP_SHR_I;
        else if (IsUnsignedIntType(k))
            *out_opcode = OP_SHR_U;
        else {
            *out_details = (VismutErrorDetails){.binary = {
                                                    .left = k,
                                                    .right = k,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_BINARY;
        }
        break;

    case VISMUT_AST_BINARY_EQUALS:
        if (IsAnyIntType(k) || IsBoolType(k))
            *out_opcode = OP_EQ_I;
        else if (IsFloatType(k))
            *out_opcode = OP_EQ_F;
        else {
            *out_details = (VismutErrorDetails){.binary = {
                                                    .left = k,
                                                    .right = k,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_BINARY;
        }
        break;

    case VISMUT_AST_BINARY_NOT_EQUALS:
        if (IsAnyIntType(k) || IsBoolType(k))
            *out_opcode = OP_NEQ_I;
        else if (IsFloatType(k))
            *out_opcode = OP_NEQ_F;
        else {
            *out_details = (VismutErrorDetails){.binary = {
                                                    .left = k,
                                                    .right = k,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_BINARY;
        }
        break;

    case VISMUT_AST_BINARY_LESS_THAN:
        if (IsSignedIntType(k))
            *out_opcode = OP_LESS_I;
        else if (IsUnsignedIntType(k))
            *out_opcode = OP_LESS_U;
        else if (IsFloatType(k))
            *out_opcode = OP_LESS_F;
        else {
            *out_details = (VismutErrorDetails){.binary = {
                                                    .left = k,
                                                    .right = k,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_BINARY;
        }
        break;

    case VISMUT_AST_BINARY_LESS_THAN_OR_EQUAL:
        if (IsSignedIntType(k))
            *out_opcode = OP_LEQ_I;
        else if (IsUnsignedIntType(k))
            *out_opcode = OP_LEQ_U;
        else if (IsFloatType(k))
            *out_opcode = OP_LEQ_F;
        else {
            *out_details = (VismutErrorDetails){.binary = {
                                                    .left = k,
                                                    .right = k,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_BINARY;
        }
        break;

    case VISMUT_AST_BINARY_GREATER_THAN:
        if (IsSignedIntType(k))
            *out_opcode = OP_GREATER_I;
        else if (IsUnsignedIntType(k))
            *out_opcode = OP_GREATER_U;
        else if (IsFloatType(k))
            *out_opcode = OP_GREATER_F;
        else {
            *out_details = (VismutErrorDetails){.binary = {
                                                    .left = k,
                                                    .right = k,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_BINARY;
        }
        break;

    case VISMUT_AST_BINARY_GREATER_THAN_OR_EQUAL:
        if (IsSignedIntType(k))
            *out_opcode = OP_GEQ_I;
        else if (IsUnsignedIntType(k))
            *out_opcode = OP_GEQ_U;
        else if (IsFloatType(k))
            *out_opcode = OP_GEQ_F;
        else {
            *out_details = (VismutErrorDetails){.binary = {
                                                    .left = k,
                                                    .right = k,
                                                    .op = op,
                                                }};
            return VISMUT_ERR_UNSUPPORTED_BINARY;
        }
        break;

    default:
        assert(0 && "Unreachable!\n");
        return VISMUT_ERR_UNREACHABLE;
    }

    return VISMUT_OK;
}
