#ifndef VISMUT_CORE_AST_AST_H
#define VISMUT_CORE_AST_AST_H
#include "../defines.h"
#include "../memory/type_context.h"
#include "../types.h"
#include "ast_type.h"
#include "scope.h"
#include "symbol.h"
#include <stdint.h>

typedef struct {
    ASTNodeType type;
    Position pos;

    union {
        struct {
            ASTNodeIdx first_statement;
        } module;

        struct {
            ASTNodeIdx expr;
            ASTNodeIdx next;
            const VismutType *type;
        } expression;

        struct {
            ASTNodeIdx decl;
            ASTNodeIdx next;
            i8 is_export;
        } declaration;

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
            ASTNodeIdx condition;
            ASTNodeIdx body;
        } loop;

        struct {
            const VismutType *type;
            VismutSymbol *resolved_symbol;
            StringView name;
        } identifier;

        struct {
            ASTNodeIdx target;
            ASTNodeIdx value;
            ASTAssignNodeType operation;
        } assignment;

        struct {
            const VismutType *type;
            VismutScope scope;
            ASTNodeIdx first_statement;
        } block;

        struct {
            StringView name;
            StringView *param_names;
            VismutScope scope;
            const VismutType *signature;
            VismutSymbol *resolved_symbol;
            ASTNodeIdx body;
        } fn_declaration;

        struct {
            const VismutType *from_type;
            const VismutType *to_type;
            ASTNodeIdx argument;
        } type_cast;

        struct {
            StringView name;
            const VismutType *type;
            VismutSymbol *resolved_symbol;
            ASTNodeIdx init;
            i8 is_mutable;
        } var_declaration;

        struct {
            u32 fields_count;
            ASTNodeIdx *fields;
            VismutSimpleValue *values;
            const VismutType *type;
        } tuple;

        struct {
            ASTNodeIdx expression;
        } ret;

        struct {
            StringView module;
            StringView alias;
            VismutModuleIdx module_idx;
            VismutSymbol *resolved_symbol;
        } import;

        struct {
            const VismutType *type;
            VismutSymbol *resolved_symbol;
            ASTNodeIdx object;
            StringView member;
        } dot;

        struct {
            const VismutType *type;
            ASTNodeIdx object;
            u32 arguments_count;
            ASTNodeIdx *arguments;
        } call;
    };
} attribute_aligned(8) ASTNode;

const VismutType *ASTNode_GetType(const VismutTypeContext *restrict type_ctx,
                                  const ASTNode *restrict node);

ASTNode ASTNode_CreateModule(ASTNodeIdx first_statement);

ASTNode ASTNode_CreateExpression(Position pos, const VismutType *type, ASTNodeIdx expr);

ASTNode ASTNode_CreateDeclaration(Position pos, ASTNodeIdx decl, int is_export);

ASTNode ASTNode_CreateLiteral(Position pos, const VismutType *type, VismutSimpleValue value);

ASTNode ASTNode_CreateTuple(Position pos, ASTNodeIdx *fields, u32 fields_count,
                            const VismutType *type);

ASTNode ASTNode_CreateUnit(Position pos);

ASTNode ASTNode_CreateIdentifier(Position pos, const VismutType *type, StringView name);

ASTNode ASTNode_CreateBinary(Position pos, const VismutType *type, ASTNodeIdx left,
                             ASTNodeIdx right, ASTBinaryNodeType op);

ASTNode ASTNode_CreateUnary(Position pos, const VismutType *type, ASTNodeIdx right,
                            ASTUnaryNodeType op);

ASTNode ASTNode_CreateCondition(Position pos, const VismutType *type, ASTNodeIdx condition,
                                ASTNodeIdx then, ASTNodeIdx else_);

ASTNode ASTNode_CreateTypeCast(Position pos, const VismutType *from_type, const VismutType *to_type,
                               ASTNodeIdx argument);

ASTNode ASTNode_CreateFnCall(Position pos, StringView name, const VismutType *restrict fn_signature,
                             ASTNodeIdx *restrict arguments, const u64 arguments_count);

ASTNode ASTNode_CreateVarDeclaration(Position pos, StringView name, const VismutType *type,
                                     ASTNodeIdx init, int is_mutable);

ASTNode ASTNode_CreateBlock(Position pos, ASTNodeIdx first_statement, const VismutType *type);

ASTNode ASTNode_CreateFnDeclaration(const Position pos, StringView name,
                                    const VismutType *restrict signature,
                                    StringView *restrict param_names, VismutSymbol *resolved_symbol,
                                    const ASTNodeIdx body);

ASTNode ASTNode_CreateReturn(Position pos, ASTNodeIdx expression);

ASTNode ASTNode_CreateLoop(Position pos, ASTNodeIdx condition, ASTNodeIdx body);

ASTNode ASTNode_CreateAssignment(Position pos, ASTNodeIdx target, ASTNodeIdx value,
                                 ASTAssignNodeType operation);

ASTNode ASTNode_CreateImport(Position pos, StringView module, StringView alias,
                             VismutModuleIdx module_idx, VismutSymbol *resolved_symbol);

ASTNode ASTNode_CreateDot(Position pos, const VismutType *type, ASTNodeIdx object,
                          StringView member);

ASTNode ASTNode_CreateCall(Position pos, const VismutType *type, ASTNodeIdx object,
                           ASTNodeIdx *restrict arguments, u32 arguments_count);

#endif
