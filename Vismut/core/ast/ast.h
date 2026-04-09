#ifndef VISMUT_CORE_AST_AST_H
#define VISMUT_CORE_AST_AST_H
#include "../defines.h"
#include "../types.h"
#include "scope.h"
#include "value.h"

#define X_VISMUT_AST_NODES(X)                                                                      \
    X(VISMUT_AST_MODULE, "module")                                                                 \
    X(VISMUT_AST_EXPRESSION, "expr")                                                               \
    X(VISMUT_AST_LITERAL, "literal")                                                               \
    X(VISMUT_AST_IDENTIFIER, "identifier")                                                         \
    X(VISMUT_AST_BINARY, "bin-op")                                                                 \
    X(VISMUT_AST_UNARY, "un-op")                                                                   \
    X(VISMUT_AST_VAR_DECLARATION, "var-decl")                                                      \
    X(VISMUT_AST_TYPE_CAST, "type-cast")                                                           \
    X(VISMUT_AST_CONDITION, "condition")                                                           \
    X(VISMUT_AST_BLOCK, "block")                                                                   \
    X(VISMUT_AST_FN_CALL, "fn-call")                                                               \
    X(VISMUT_AST_UNIT, "unit")                                                                     \
    X(VISMUT_AST_TUPLE, "tuple")                                                                   \
    X(VISMUT_AST_UNKNOWN, "unknown")

#define X_VISMUT_AST_BINARY_NODES(X)                                                               \
    X(VISMUT_AST_BINARY_ADD, "+")                                                                  \
    X(VISMUT_AST_BINARY_SUB, "-")                                                                  \
    X(VISMUT_AST_BINARY_MUL, "*")                                                                  \
    X(VISMUT_AST_BINARY_DIV, "/")                                                                  \
    X(VISMUT_AST_BINARY_REM_DIV, "%")                                                              \
    X(VISMUT_AST_BINARY_POW, "**")                                                                 \
    X(VISMUT_AST_BINARY_LOGICAL_OR, "||")                                                          \
    X(VISMUT_AST_BINARY_LOGICAL_AND, "&&")                                                         \
    X(VISMUT_AST_BINARY_BITWISE_OR, "|")                                                           \
    X(VISMUT_AST_BINARY_BITWISE_AND, "&")                                                          \
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
    X(VISMUT_AST_UNARY_REFERENCE, "&")                                                             \
    X(VISMUT_AST_UNARY_DEREFERENCE, "*")                                                           \
    X(VISMUT_AST_UNARY_UNKNOWN, "unknown")

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

attribute_const const u8 *ASTNodeType_String(ASTNodeType);

attribute_const const u8 *ASTBinaryNodeType_String(ASTBinaryNodeType);

attribute_const const u8 *ASTUnaryNodeType_String(ASTUnaryNodeType);

typedef i32 ASTNodeIdx;

typedef struct {
    ASTNodeType type;
    Position pos;

    union {
        struct {
            StringNode *name;
            VismutScope *scope;
            ASTNodeIdx first_expression;
            ASTNodeIdx first_function_declaration;
        } module;

        struct {
            ASTNodeIdx expr;
            ASTNodeIdx next_expr;
            const VismutType *type;
        } expression;

        struct {
            ASTNodeIdx left;
            ASTNodeIdx right;
            ASTBinaryNodeType op;
            const VismutType *type;
        } binary;

        struct {
            ASTNodeIdx right;
            ASTUnaryNodeType op;
            const VismutType *type;
        } unary;

        struct {
            const VismutType *type;
            VismutSimpleValue value;
        } literal;

        struct {
            ASTNodeIdx condition;
            ASTNodeIdx then;
            ASTNodeIdx else_;
            const VismutType *type;
        } condition;

        struct {
            const VismutType *type;
            StringNode *name;
        } identifier;

        struct {
            const VismutType *type;
            VismutScope *scope;
            ASTNodeIdx first_expression;
        } block;

        struct {
            StringNode *name;
            VismutScope *scope;
            const VismutType *signature;
            ASTNodeIdx body_or_expression;
        } fn_declaration;

        struct {
            StringNode *name;
            const VismutType *fn_signature;
            ASTNodeIdx *arguments;
            u64 arguments_count;
        } fn_call;

        struct {
            const VismutType *from_type;
            const VismutType *to_type;
            ASTNodeIdx argument;
        } type_cast;

        struct {
            StringNode *name;
            const VismutType *type;
            ASTNodeIdx init;
            int is_mutable;
        } var_declaration;

        struct {
            u32 fields_count;
            ASTNodeIdx *fields;
            VismutSimpleValue *values;
            const VismutType *type;
        } tuple;
    };
} attribute_aligned(16) ASTNode;

ASTNode ASTNode_CreateModule(StringNode *name, ASTNodeIdx first_expression,
                             ASTNodeIdx first_function_declaration);

ASTNode ASTNode_CreateExpression(Position pos, const VismutType *type, ASTNodeIdx expr);

ASTNode ASTNode_CreateLiteral(Position pos, const VismutType *type, VismutSimpleValue value);

ASTNode ASTNode_CreateTuple(Position pos, ASTNodeIdx *fields, u32 fields_count,
                            const VismutType *type);

ASTNode ASTNode_CreateUnit(Position pos);

ASTNode ASTNode_CreateIdentifier(Position pos, const VismutType *type, StringNode *name);

ASTNode ASTNode_CreateBinary(Position pos, const VismutType *type, ASTNodeIdx left,
                             ASTNodeIdx right, ASTBinaryNodeType op);

ASTNode ASTNode_CreateUnary(Position pos, const VismutType *type, ASTNodeIdx right,
                            ASTUnaryNodeType op);

ASTNode ASTNode_CreateCondition(Position pos, const VismutType *type, ASTNodeIdx condition,
                                ASTNodeIdx then, ASTNodeIdx else_);

ASTNode ASTNode_CreateTypeCast(Position pos, const VismutType *from_type, const VismutType *to_type,
                               ASTNodeIdx argument);

ASTNode ASTNode_CreateFnCall(Position pos, StringNode *restrict name,
                             const VismutType *restrict fn_signature,
                             ASTNodeIdx *restrict arguments, const u64 arguments_count);

ASTNode ASTNode_CreateVarDeclaration(Position pos, StringNode *name, const VismutType *type,
                                     ASTNodeIdx init, int is_mutable);

ASTNode ASTNode_CreateBlock(Position pos, ASTNodeIdx first_expression, const VismutType *type);

#define ASTNodeIdx_None (-1)
#define ASTNodeIdx_IsNone(node_idx) (ASTNodeIdx_None == (node_idx))

#endif
