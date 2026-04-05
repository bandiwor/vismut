#ifndef VISMUT_CORE_AST_AST_UTILS_H
#define VISMUT_CORE_AST_AST_UTILS_H
#include "../defines.h"
#include "../tokenizer/token.h"
#include "ast.h"

typedef enum {
    PRECEDENCE_NONE,
    PRECEDENCE_MINIMAL,
    PRECEDENCE_LOGICAL_OR,
    PRECEDENCE_LOGICAL_AND,
    PRECEDENCE_EQUALITY,
    PRECEDENCE_RELATIONAL,
    PRECEDENCE_ADDITIVE,
    PRECEDENCE_MULTIPLICATIVE,
    PRECEDENCE_UNARY,
    PRECEDENCE_PRIMARY,
} OperatorPrecedence;

attribute_const OperatorPrecedence GetPrecedence(VismutTokenType);

attribute_const ASTBinaryNodeType GetBinaryType(VismutTokenType);

attribute_const ASTUnaryNodeType GetUnaryType(VismutTokenType);

attribute_pure const VismutType *VismutTypeTokenToType(const VismutTokenType type,
                                                       const VismutTypeContext *ctx);

#endif
