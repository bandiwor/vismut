#ifndef VISMUT_CORE_AST_AST_UTILS_H
#define VISMUT_CORE_AST_AST_UTILS_H
#include "../ast/ast.h"
#include "../defines.h"
#include "../tokenizer/token.h"

typedef enum {
    PRECEDENCE_NONE,
    PRECEDENCE_MINIMAL,
    PRECEDENCE_LOGICAL_OR,
    PRECEDENCE_LOGICAL_AND,
    PRECEDENCE_BITWISE_OR,
    PRECEDENCE_BITWISE_XOR,
    PRECEDENCE_BITWISE_AND,
    PRECEDENCE_EQUALITY,
    PRECEDENCE_RELATIONAL,
    PRECEDENCE_SHIFT,
    PRECEDENCE_ADDITIVE,
    PRECEDENCE_MULTIPLICATIVE,
    PRECEDENCE_UNARY,
    PRECEDENCE_PRIMARY,
} OperatorPrecedence;

attribute_const OperatorPrecedence GetPrecedence(ASTBinaryNodeType);

attribute_const ASTBinaryNodeType GetBinaryType(VismutTokenType);

attribute_const ASTUnaryNodeType GetUnaryType(VismutTokenType);

attribute_pure const VismutType *VismutTypeTokenToType(const VismutTokenType type,
                                                       const VismutTypeContext *ctx);

attribute_pure ASTNodeIdx ASTNode_GetNextStatement(const ASTNode *node);

#endif
