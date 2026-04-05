#include "ast_utils.h"
#include "value.h"
#include <assert.h>

attribute_const OperatorPrecedence GetPrecedence(const VismutTokenType token) {
    switch (token) {
    case VISMUT_TOKEN_LOGICAL_OR:
        return PRECEDENCE_LOGICAL_OR;
    case VISMUT_TOKEN_AMPERSAND_AMPERSAND: // logical and
        return PRECEDENCE_LOGICAL_AND;
    case VISMUT_TOKEN_EQUAL_EQUAL:
    case VISMUT_TOKEN_NOT_EQUALS:
        return PRECEDENCE_EQUALITY;
    case VISMUT_TOKEN_LANGLE:
    case VISMUT_TOKEN_LESS_THAN_OR_EQUALS:
    case VISMUT_TOKEN_RANGLE:
    case VISMUT_TOKEN_GREATER_THAN_OR_EQUALS:
        return PRECEDENCE_RELATIONAL;
    case VISMUT_TOKEN_PLUS:
    case VISMUT_TOKEN_MINUS:
        return PRECEDENCE_ADDITIVE;
    case VISMUT_TOKEN_STAR:
    case VISMUT_TOKEN_SLASH:
        return PRECEDENCE_MULTIPLICATIVE;
    case VISMUT_TOKEN_STAR_STAR:
        return PRECEDENCE_UNARY;
    default:
        return PRECEDENCE_NONE;
    }
}

attribute_const ASTBinaryNodeType GetBinaryType(const VismutTokenType token) {
    switch (token) {
    case VISMUT_TOKEN_PLUS:
        return VISMUT_AST_BINARY_ADD;
    case VISMUT_TOKEN_MINUS:
        return VISMUT_AST_BINARY_SUB;
    case VISMUT_TOKEN_STAR:
        return VISMUT_AST_BINARY_MUL;
    case VISMUT_TOKEN_SLASH:
        return VISMUT_AST_BINARY_DIV;
    case VISMUT_TOKEN_PERCENT:
        return VISMUT_AST_BINARY_REM_DIV;
    case VISMUT_TOKEN_STAR_STAR:
        return VISMUT_AST_BINARY_POW;
    case VISMUT_TOKEN_EQUAL_EQUAL:
        return VISMUT_AST_BINARY_EQUALS;
    case VISMUT_TOKEN_NOT_EQUALS:
        return VISMUT_AST_BINARY_NOT_EQUALS;
    case VISMUT_TOKEN_RANGLE:
        return VISMUT_AST_BINARY_GREATER_THAN;
    case VISMUT_TOKEN_GREATER_THAN_OR_EQUALS:
        return VISMUT_AST_BINARY_GREATER_THAN_OR_EQUAL;
    case VISMUT_TOKEN_LANGLE:
        return VISMUT_AST_BINARY_LESS_THAN;
    case VISMUT_TOKEN_LESS_THAN_OR_EQUALS:
        return VISMUT_AST_BINARY_LESS_THAN_OR_EQUAL;
    default:
        return VISMUT_AST_BINARY_UNKNOWN;
    }
}

attribute_const ASTUnaryNodeType GetUnaryType(const VismutTokenType token) {
    switch (token) {
    case VISMUT_TOKEN_MINUS:
        return VISMUT_AST_UNARY_MINUS;
    case VISMUT_TOKEN_PLUS:
        return VISMUT_AST_UNARY_PLUS;
    case VISMUT_TOKEN_EXCLAMATION_MARK:
        return VISMUT_AST_UNARY_LOGICAL_NOT;
    case VISMUT_TOKEN_TILDA:
        return VISMUT_AST_UNARY_BITWISE_NOT;
    case VISMUT_TOKEN_AMPERSAND:
        return VISMUT_AST_UNARY_REFERENCE;
    case VISMUT_TOKEN_STAR:
        return VISMUT_AST_UNARY_DEREFERENCE;
    default:
        return VISMUT_AST_UNARY_UNKNOWN;
    }
}

attribute_pure const VismutType *VismutTypeTokenToType(const VismutTokenType type,
                                                       const VismutTypeContext *ctx) {
    switch (type) {
    case VISMUT_TOKEN_I1_TYPE:
        return ctx->type_i1;
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
        return ctx->type_u64;
    case VISMUT_TOKEN_F32_TYPE:
        return ctx->type_f32;
    case VISMUT_TOKEN_F64_TYPE:
        return ctx->type_f64;
    default:
        assert(0 && "Unreachable!");
        return ctx->type_void;
    }
}
