#include "ast_type.h"

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

attribute_const const u8 *ASTAssignNodeType_String(const ASTAssignNodeType type) {
#define X(name, text) [name] = (const u8 *)text,
    static const u8 *const codes_table[] = {X_VISMUT_AST_ASSIGN_NODES(X)};
#undef X

    if (unlikely(type >= VISMUT_AST_ASSIGN_COUNT || type < 0)) {
        return codes_table[VISMUT_AST_ASSIGN_UNKNOWN];
    }

    return codes_table[type];
}
