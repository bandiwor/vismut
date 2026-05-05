#include "ast.h"
#include "ast_type.h"
#include "scope.h"
#include "symbol.h"
#include <assert.h>

const VismutType *ASTNode_GetType(const VismutTypeContext *restrict type_ctx,
                                  const ASTNode *restrict node) {
    switch (node->type) {
    case VISMUT_AST_MODULE:
        return type_ctx->type_unit;
    case VISMUT_AST_EXPRESSION:
        return node->expression.type;
    case VISMUT_AST_DECLARATION:
        return type_ctx->type_unit;
    case VISMUT_AST_LITERAL:
        return node->literal.type;
    case VISMUT_AST_IDENTIFIER:
        return node->identifier.type;
    case VISMUT_AST_ASSIGNMENT:
        return type_ctx->type_unit;
    case VISMUT_AST_BINARY:
        return node->binary.type;
    case VISMUT_AST_UNARY:
        return node->unary.type;
    case VISMUT_AST_VAR_DECLARATION:
        return type_ctx->type_unit;
    case VISMUT_AST_TYPE_CAST:
        return node->type_cast.to_type;
    case VISMUT_AST_CONDITION:
        return node->condition.type;
    case VISMUT_AST_LOOP:
        return type_ctx->type_unit;
    case VISMUT_AST_BLOCK:
        return node->block.type;
    case VISMUT_AST_FN_DECLARATION:
        return type_ctx->type_unit;
    case VISMUT_AST_CALL:
        return node->call.type;
    case VISMUT_AST_UNIT:
        return type_ctx->type_unit;
    case VISMUT_AST_TUPLE:
        return node->tuple.type;
    case VISMUT_AST_RETURN:
        return type_ctx->type_never;
    default:
        return type_ctx->type_unit;
    }
}

ASTNode ASTNode_CreateModule(const ASTNodeIdx first_statement) {
    return (ASTNode){
        .type = VISMUT_AST_MODULE,
        .pos = {0},
        .module =
            {
                .first_statement = first_statement,
            },
    };
}

ASTNode ASTNode_CreateExpression(const Position pos, const VismutType *type,
                                 const ASTNodeIdx expr) {
    return (ASTNode){.type = VISMUT_AST_EXPRESSION,
                     .pos = pos,
                     .expression = {
                         .type = type,
                         .expr = expr,
                         .next = ASTNodeIdx_None,
                     }};
}

ASTNode ASTNode_CreateDeclaration(const Position pos, const ASTNodeIdx decl, const int is_export) {
    return (ASTNode){.type = VISMUT_AST_DECLARATION,
                     .pos = pos,
                     .declaration = {
                         .decl = decl,
                         .next = ASTNodeIdx_None,
                         .is_export = is_export,
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

ASTNode ASTNode_CreateIdentifier(const Position pos, const VismutType *type, StringView name) {
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

ASTNode ASTNode_CreateVarDeclaration(const Position pos, StringView name, const VismutType *type,
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

ASTNode ASTNode_CreateBlock(const Position pos, const ASTNodeIdx first_statement,
                            const VismutType *type) {
    return (ASTNode){
        .type = VISMUT_AST_BLOCK,
        .pos = pos,
        .block =
            {
                .first_statement = first_statement,
                .type = type,
                .scope = VismutScope_Create(NULL),
            },
    };
}

ASTNode ASTNode_CreateTuple(const Position pos, ASTNodeIdx *fields, const u32 fields_count,
                            const VismutType *type) {
    assert(fields_count > 0);
    return (ASTNode){
        .type = VISMUT_AST_TUPLE,
        .pos = pos,
        .tuple =
            {
                .fields_count = fields_count,
                .fields = fields,
                .type = type,
            },
    };
}

ASTNode ASTNode_CreateUnit(const Position pos) {
    return (ASTNode){
        .type = VISMUT_AST_UNIT,
        .pos = pos,
    };
}

ASTNode ASTNode_CreateFnDeclaration(const Position pos, StringView name,
                                    const VismutType *restrict signature,
                                    StringView *restrict param_names, VismutSymbol *resolved_symbol,
                                    const ASTNodeIdx body) {
    return (ASTNode){
        .type = VISMUT_AST_FN_DECLARATION,
        .pos = pos,
        .fn_declaration =
            {
                .name = name,
                .signature = signature,
                .param_names = param_names,
                .resolved_symbol = resolved_symbol,
                .scope = VismutScope_Create(NULL),
                .body = body,
            },
    };
}

ASTNode ASTNode_CreateReturn(const Position pos, const ASTNodeIdx expression) {
    return (ASTNode){.type = VISMUT_AST_RETURN,
                     .pos = pos,
                     .ret = {
                         .expression = expression,
                     }};
}

ASTNode ASTNode_CreateLoop(const Position pos, const ASTNodeIdx condition, const ASTNodeIdx body) {
    return (ASTNode){
        .type = VISMUT_AST_LOOP,
        .pos = pos,
        .loop =
            {
                .condition = condition,
                .body = body,
            },
    };
}

ASTNode ASTNode_CreateAssignment(const Position pos, const ASTNodeIdx target,
                                 const ASTNodeIdx value, const ASTAssignNodeType operation) {
    return (ASTNode){
        .type = VISMUT_AST_ASSIGNMENT,
        .pos = pos,
        .assignment =
            {
                .target = target,
                .value = value,
                .operation = operation,
            },
    };
}

ASTNode ASTNode_CreateImport(const Position pos, StringView module, StringView alias,
                             const VismutModuleIdx module_idx, VismutSymbol *resolved_symbol) {
    return (ASTNode){.type = VISMUT_AST_IMPORT,
                     .pos = pos,
                     .import = {
                         .module = module,
                         .module_idx = module_idx,
                         .alias = alias,
                         .resolved_symbol = resolved_symbol,
                     }};
}

ASTNode ASTNode_CreateDot(const Position pos, const VismutType *type, const ASTNodeIdx object,
                          StringView member) {
    return (ASTNode){
        .type = VISMUT_AST_DOT,
        .pos = pos,
        .dot =
            {
                .type = type,
                .object = object,
                .member = member,
            },
    };
}

ASTNode ASTNode_CreateCall(Position pos, const VismutType *type, const ASTNodeIdx object,
                           ASTNodeIdx *restrict arguments, const u32 arguments_count) {
    return (ASTNode){
        .type = VISMUT_AST_CALL,
        .pos = pos,
        .call =
            {
                .type = type,
                .object = object,
                .arguments = arguments,
                .arguments_count = arguments_count,
            },
    };
}
