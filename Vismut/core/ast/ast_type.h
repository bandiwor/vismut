#ifndef VISMUT_CORE_AST_AST_TYPE_H
#define VISMUT_CORE_AST_AST_TYPE_H
#include "../defines.h"
#include "../types.h"

#define X_VISMUT_AST_NODES(X)                                                                      \
    X(VISMUT_AST_MODULE, "module")                                                                 \
    X(VISMUT_AST_EXPRESSION, "expr")                                                               \
    X(VISMUT_AST_DECLARATION, "decl")                                                              \
    X(VISMUT_AST_LITERAL, "literal")                                                               \
    X(VISMUT_AST_IDENTIFIER, "identifier")                                                         \
    X(VISMUT_AST_ASSIGNMENT, "assign")                                                             \
    X(VISMUT_AST_BINARY, "bin-op")                                                                 \
    X(VISMUT_AST_UNARY, "un-op")                                                                   \
    X(VISMUT_AST_VAR_DECLARATION, "var-decl")                                                      \
    X(VISMUT_AST_TYPE_CAST, "type-cast")                                                           \
    X(VISMUT_AST_CONDITION, "condition")                                                           \
    X(VISMUT_AST_LOOP, "loop")                                                                     \
    X(VISMUT_AST_BLOCK, "block")                                                                   \
    X(VISMUT_AST_FN_DECLARATION, "fn-decl")                                                        \
    X(VISMUT_AST_CALL, "call")                                                                     \
    X(VISMUT_AST_UNIT, "unit")                                                                     \
    X(VISMUT_AST_TUPLE, "tuple")                                                                   \
    X(VISMUT_AST_RETURN, "ret")                                                                    \
    X(VISMUT_AST_IMPORT, "import")                                                                 \
    X(VISMUT_AST_UNKNOWN, "unknown")                                                               \
    X(VISMUT_AST_DOT, "dot")

#define X_VISMUT_AST_BINARY_NODES(X)                                                               \
    X(VISMUT_AST_BINARY_ADD, "+")                                                                  \
    X(VISMUT_AST_BINARY_SUB, "-")                                                                  \
    X(VISMUT_AST_BINARY_MUL, "*")                                                                  \
    X(VISMUT_AST_BINARY_DIV, "/")                                                                  \
    X(VISMUT_AST_BINARY_REM_DIV, "%")                                                              \
    X(VISMUT_AST_BINARY_POW, "**")                                                                 \
    X(VISMUT_AST_BINARY_SHIFT_LEFT, "<<")                                                          \
    X(VISMUT_AST_BINARY_SHIFT_RIGHT, ">>")                                                         \
    X(VISMUT_AST_BINARY_LOGICAL_OR, "||")                                                          \
    X(VISMUT_AST_BINARY_LOGICAL_AND, "&&")                                                         \
    X(VISMUT_AST_BINARY_BITWISE_OR, "|")                                                           \
    X(VISMUT_AST_BINARY_BITWISE_AND, "&")                                                          \
    X(VISMUT_AST_BINARY_BITWISE_XOR, "^")                                                          \
    X(VISMUT_AST_BINARY_EQUALS, "==")                                                              \
    X(VISMUT_AST_BINARY_NOT_EQUALS, "!=")                                                          \
    X(VISMUT_AST_BINARY_LESS_THAN, "<")                                                            \
    X(VISMUT_AST_BINARY_LESS_THAN_OR_EQUAL, "<=")                                                  \
    X(VISMUT_AST_BINARY_GREATER_THAN, ">")                                                         \
    X(VISMUT_AST_BINARY_GREATER_THAN_OR_EQUAL, ">=")                                               \
    X(VISMUT_AST_BINARY_UNKNOWN, "unknown")

#define X_VISMUT_AST_UNARY_NODES(X)                                                                \
    X(VISMUT_AST_UNARY_PLUS, "+")                                                                  \
    X(VISMUT_AST_UNARY_MINUS, "-")                                                                 \
    X(VISMUT_AST_UNARY_LOGICAL_NOT, "!")                                                           \
    X(VISMUT_AST_UNARY_BITWISE_NOT, "~")                                                           \
    X(VISMUT_AST_UNARY_UNKNOWN, "unknown")

#define X_VISMUT_AST_ASSIGN_NODES(X)                                                               \
    X(VISMUT_AST_ASSIGN_EQUAL, "=")                                                                \
    X(VISMUT_AST_ASSIGN_UNKNOWN, "unknown")

typedef enum {
#define X(name, text) name,
    X_VISMUT_AST_NODES(X)
#undef X
        VISMUT_AST_COUNT
} ASTNodeType;

typedef enum {
#define X(name, text) name,
    X_VISMUT_AST_BINARY_NODES(X)
#undef X
        VISMUT_AST_BINARY_COUNT
} ASTBinaryNodeType;

typedef enum {
#define X(name, text) name,
    X_VISMUT_AST_UNARY_NODES(X)
#undef X
        VISMUT_AST_UNARY_COUNT
} ASTUnaryNodeType;

typedef enum {
#define X(name, text) name,
    X_VISMUT_AST_ASSIGN_NODES(X)
#undef X
        VISMUT_AST_ASSIGN_COUNT
} ASTAssignNodeType;

attribute_const const u8 *ASTNodeType_String(ASTNodeType);

attribute_const const u8 *ASTBinaryNodeType_String(ASTBinaryNodeType);

attribute_const const u8 *ASTUnaryNodeType_String(ASTUnaryNodeType);

attribute_const const u8 *ASTAssignNodeType_String(ASTAssignNodeType);

#endif
