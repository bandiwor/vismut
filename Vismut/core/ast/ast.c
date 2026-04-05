#include "ast.h"

attribute_const const u8 *ASTNodeType_String(const ASTNodeType type) {
#define X(name, text) [name] = (const u8 *)text,
    static const u8 *const codes_table[] = {X_VISMUT_AST_NODES(X)};
#undef X

    if (unlikely(type >= VISMUT_AST_COUNT || type < 0)) {
        return codes_table[VISMUT_AST_UNKNOWN];
    }

    return codes_table[type];
}

attribute_const const u8 *ASTBinaryNodeType_String(const ASTBinaryNodeType type) {
#define X(name, text) [name] = (const u8 *)text,
    static const u8 *const codes_table[] = {X_VISMUT_AST_BINARY_NODES(X)};
#undef X

    if (unlikely(type >= VISMUT_AST_BINARY_COUNT || type < 0)) {
        return codes_table[VISMUT_AST_BINARY_UNKNOWN];
    }

    return codes_table[type];
}

attribute_const const u8 *ASTUnaryNodeType_String(const ASTUnaryNodeType type) {
#define X(name, text) [name] = (const u8 *)text,
    static const u8 *const codes_table[] = {X_VISMUT_AST_UNARY_NODES(X)};
#undef X

    if (unlikely(type >= VISMUT_AST_UNARY_COUNT || type < 0)) {
        return codes_table[VISMUT_AST_UNARY_UNKNOWN];
    }

    return codes_table[type];
}

ASTNode ASTNode_CreateModule(StringNode *name, const ASTNodeIdx first_expression,
                             const ASTNodeIdx first_function_declaration) {
    return (ASTNode){
        .type = VISMUT_AST_MODULE,
        .pos = {0},
        .module = {.name = name,
                   .first_expression = first_expression,
                   .first_function_declaration = first_function_declaration},
    };
}

ASTNode ASTNode_CreateExpression(const Position pos, const VismutType *type,
                                 const ASTNodeIdx expr) {
    return (ASTNode){.type = VISMUT_AST_EXPRESSION,
                     .pos = pos,
                     .expression = {
                         .type = type,
                         .expr = expr,
                         .next_expr = ASTNodeIdx_None,
                     }};
}

ASTNode ASTNode_CreateLiteral(Position pos, const VismutType *type, const VismutSimpleValue value) {
    return (ASTNode){
        .type = VISMUT_AST_LITERAL,
        .pos = pos,
        .literal =
            {
                .type = type,
                .value = value,
            },
    };
}

ASTNode ASTNode_CreateBinary(const Position pos, const VismutType *type, const ASTNodeIdx left,
                             const ASTNodeIdx right, const ASTBinaryNodeType op) {
    return (ASTNode){
        .type = VISMUT_AST_BINARY,
        .pos = pos,
        .binary =
            {
                .left = left,
                .right = right,
                .op = op,
                .type = type,
            },
    };
}

ASTNode ASTNode_CreateUnary(const Position pos, const VismutType *type, const ASTNodeIdx right,
                            const ASTUnaryNodeType op) {
    return (ASTNode){
        .type = VISMUT_AST_UNARY,
        .pos = pos,
        .unary =
            {
                .right = right,
                .op = op,
                .type = type,
            },
    };
}

ASTNode ASTNode_CreateIdentifier(const Position pos, const VismutType *type, StringNode *name) {
    return (ASTNode){
        .type = VISMUT_AST_IDENTIFIER,
        .pos = pos,
        .identifier =
            {
                .name = name,
                .type = type,
            },
    };
}

ASTNode ASTNode_CreateCondition(const Position pos, const VismutType *type,
                                const ASTNodeIdx condition, const ASTNodeIdx then,
                                const ASTNodeIdx else_) {
    return (ASTNode){
        .type = VISMUT_AST_CONDITION,
        .pos = pos,
        .condition =
            {
                .condition = condition,
                .then = then,
                .else_ = else_,
                .type = type,
            },
    };
}

ASTNode ASTNode_CreateTypeCast(const Position pos, const VismutType *from_type,
                               const VismutType *to_type, const ASTNodeIdx argument) {
    return (ASTNode){
        .type = VISMUT_AST_TYPE_CAST,
        .pos = pos,
        .type_cast =
            {
                .from_type = from_type,
                .to_type = to_type,
                .argument = argument,
            },
    };
}

ASTNode ASTNode_CreateFnCall(const Position pos, StringNode *restrict name,
                             const VismutType *restrict fn_signature,
                             ASTNodeIdx *restrict arguments, const u64 arguments_count) {
    return (ASTNode){
        .type = VISMUT_AST_FN_CALL,
        .pos = pos,
        .fn_call =
            {
                .fn_signature = fn_signature,
                .name = name,
                .arguments = arguments,
                .arguments_count = arguments_count,
            },
    };
}

ASTNode ASTNode_CreateVarDeclaration(const Position pos, StringNode *name, const VismutType *type,
                                     const ASTNodeIdx init, const int is_mutable) {
    return (ASTNode){
        .type = VISMUT_AST_VAR_DECLARATION,
        .pos = pos,
        .var_declaration =
            {
                .name = name,
                .type = type,
                .init = init,
                .is_mutable = is_mutable,
            },
    };
}

ASTNode ASTNode_CreateBlock(const Position pos, const ASTNodeIdx first_expression,
                            const VismutType *type) {
    return (ASTNode){
        .type = VISMUT_AST_BLOCK,
        .pos = pos,
        .block =
            {
                .first_expression = first_expression,
                .type = type,
                .scope = NULL,
            },
    };
}
