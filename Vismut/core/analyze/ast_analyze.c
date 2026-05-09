#include "ast_analyze.h"
#include "../convert/literal.h"
#include "../memory/arena.h"
#include "../memory/modules_table.h"
#include "../utils/ast_utils.h"
#include "../utils/types.h"
#include "ast_typing.h"
#include <assert.h>
#include <stdio.h>

ASTAnalyzer ASTAnalyzer_Create(ASTBuilder *restrict builder, Arena *restrict arena,
                               VismutTypeContext *restrict type_context,
                               SymbolsTable *restrict symbols_table,
                               VismutModulesTable *restrict modules,
                               VismutErrorInfo *restrict error_info, const StringView source,
                               const ASTNodeIdx module) {
    return (ASTAnalyzer){
        .builder = builder,
        .arena = arena,
        .type_context = type_context,
        .module = module,
        .modules = modules,
        .symbols_table = symbols_table,
        .error_info = error_info,
        .source = source,
        .state =
            {
                .help_type = type_context->type_auto,
                .expected_return_type = NULL,
                .current_scope = NULL,
                .in_block = 0,
                .in_function = 0,
                .is_exported_decl = 0,
            },
    };
}

attribute_nodiscard static VismutErrorType
ASTAnalyzer_Analyze(ASTAnalyzer *restrict ctx, ASTNodeIdx idx,
                    const VismutType **restrict out_type);

attribute_nodiscard VismutErrorType ASTAnalyzer_TypeAnalyze(ASTAnalyzer *ctx) {

    const VismutType *type;
    return ASTAnalyzer_Analyze(ctx, ctx->module, &type);
}

static ASTNode *node_at(ASTAnalyzer *ctx, const ASTNodeIdx idx) {
    return ASTBuilder_NodeAt(ctx->builder, idx);
}

// return 0 if type conv. not needed, 1 if needed
attribute_nodiscard static int get_narrowed_literal_type(const VismutTypeContext *restrict ctx,
                                                         const VismutType *restrict literal_type,
                                                         const VismutType *restrict helping_type,
                                                         const VismutType **restrict out_type) {
    if (literal_type != ctx->type_int && literal_type != ctx->type_float) {
        *out_type = literal_type;
        return 0;
    }
    if (literal_type == ctx->type_int) {
        if (VismutTypeKind_IsInt(helping_type->kind)) {
            *out_type = helping_type;
            return 1;
        } else if (helping_type == ctx->type_auto) {
            *out_type = ctx->type_i32; // default int type!
            return 1;
        } else {
            *out_type = literal_type;
            return 0;
        }
    } else { // type_float
        if (VismutTypeKind_IsFloat(helping_type->kind)) {
            *out_type = helping_type;
            return 1;
        } else if (helping_type == ctx->type_auto) {
            *out_type = ctx->type_f64; // default float type!
            return 1;
        } else {
            *out_type = literal_type;
            return 0;
        }
    }
}

static void ASTAnalyzer_SetErrorInfo(ASTAnalyzer *ctx, const VismutErrorType type,
                                     const Position pos, const VismutErrorDetails details) {
    ctx->error_info->type = type;
    ctx->error_info->pos = pos;
    ctx->error_info->details = details;
    ctx->error_info->source = ctx->source;
}

static VismutErrorType find_symbol_from_scope(ASTAnalyzer *restrict ctx, const StringView name,
                                              VismutSymbol **out_symbol,
                                              VismutErrorDetails *out_details) {
    if (ctx->state.current_scope != NULL) {
        VismutScopeNode *found_node = VismutScope_Find(ctx->state.current_scope, name);
        if (found_node != NULL) {
            *out_symbol = found_node->symbol;
            return VISMUT_OK;
        }
    }

    const u32 name_hash = StringView_Hash(name);

    VismutSymbol *symbol = SymbolsTable_FindSymbol(ctx->symbols_table, name, name_hash);
    if (unlikely(symbol == NULL)) {
        *out_details = (VismutErrorDetails){
            .symbol_name = name,
        };
        return VISMUT_ERR_NAME_NOT_FOUND;
    }

    *out_symbol = symbol;
    return VISMUT_OK;
}

static VismutErrorType declare_variable(ASTAnalyzer *restrict ctx, const StringView name,
                                        const VismutType *type, i1 is_mutable, i1 is_exported,
                                        VismutSymbol **out_symbol,
                                        VismutErrorDetails *restrict out_details) {
    VismutErrorType err;

    if (ctx->state.current_scope == NULL) {
        SAFE_RISKY_EXPRESSION(SymbolsTable_InsertSymbol(ctx->symbols_table,
                                                        SymbolEntry_CreateGlobalVariable(
                                                            name, type, is_exported, 0, is_mutable),
                                                        out_symbol, out_details),
                              err);
    } else {
        VismutSymbol *new_symbol;
        SAFE_RISKY_EXPRESSION(SymbolsTable_AllocateLocalSymbol(
                                  ctx->symbols_table,
                                  SymbolEntry_CreateLocalVariable(name, type, 0, is_mutable),
                                  &new_symbol, out_details),
                              err);

        VismutScopeNode *node = Arena_Type(ctx->arena, VismutScopeNode, &err, out_details);
        if (unlikely(err != VISMUT_OK)) {
            return err;
        }

        *node = VismutScopeNode_Create(new_symbol);

        VismutScope_InsertNode(ctx->state.current_scope, node);

        *out_symbol = new_symbol;
    }

    return VISMUT_OK;
}

attribute_nodiscard static VismutErrorType
ASTAnalyzer_Analyze(ASTAnalyzer *restrict ctx, const ASTNodeIdx idx,
                    const VismutType **restrict out_type) {
    VismutErrorType err;
    VismutErrorDetails details;

    switch (node_at(ctx, idx)->type) {
    case VISMUT_AST_MODULE: {
        ASTNodeIdx statement_idx = node_at(ctx, idx)->module.first_statement;
        const VismutType *type;
        while (!ASTNodeIdx_IsNone(statement_idx)) {
            SAFE_RISKY_EXPRESSION(ASTAnalyzer_Analyze(ctx, statement_idx, &type), err);
            statement_idx = ASTNode_GetNextStatement(node_at(ctx, statement_idx));
        }
    } break;

    case VISMUT_AST_EXPRESSION: {
        const VismutType *sub_expression_type;
        SAFE_RISKY_EXPRESSION(
            ASTAnalyzer_Analyze(ctx, node_at(ctx, idx)->expression.expr, &sub_expression_type),
            err);

        if (sub_expression_type == ctx->type_context->type_never) {
            node_at(ctx, idx)->expression.type = ctx->type_context->type_never;
            *out_type = ctx->type_context->type_never;
            break;
        }

        node_at(ctx, idx)->expression.type =
            node_at(ctx, idx)->expression.type == ctx->type_context->type_auto
                ? sub_expression_type
                : node_at(ctx, idx)->expression.type;

        if (node_at(ctx, idx)->expression.type != ctx->type_context->type_unit &&
            !ctx->state.in_block) {
            ASTAnalyzer_SetErrorInfo(ctx, VISMUT_ERR_NON_UNIT_TYPE_AT_TOP_LEVEL,
                                     node_at(ctx, idx)->pos, (VismutErrorDetails){0});
            return VISMUT_ERR_NON_UNIT_TYPE_AT_TOP_LEVEL;
        }

        *out_type = node_at(ctx, idx)->expression.type;
    } break;

    case VISMUT_AST_DECLARATION: {
        const i1 prev_exported = ctx->state.is_exported_decl;
        ctx->state.is_exported_decl = node_at(ctx, idx)->declaration.is_export;
        const VismutType *declaration_type;
        SAFE_RISKY_EXPRESSION(
            ASTAnalyzer_Analyze(ctx, node_at(ctx, idx)->declaration.decl, &declaration_type), err);

        ctx->state.is_exported_decl = prev_exported;
        *out_type = ctx->type_context->type_unit;
    } break;

    case VISMUT_AST_FN_DECLARATION: {
        const VismutType *signature = node_at(ctx, idx)->fn_declaration.signature;

        if (ctx->state.current_scope != NULL) {
            ASTAnalyzer_SetErrorInfo(ctx, VISMUT_ERR_NESTED_FN, node_at(ctx, idx)->pos,
                                     (VismutErrorDetails){0});
            return VISMUT_ERR_NESTED_FN;
        }

        const i8 old_in_function = ctx->state.in_function;
        ctx->state.in_function = 1;

        node_at(ctx, idx)->fn_declaration.scope = VismutScope_Create(NULL);
        ctx->state.current_scope = &node_at(ctx, idx)->fn_declaration.scope;

        const u32 param_count = signature->function.param_count;
        for (u32 i = 0; i < param_count; ++i) {
            const StringView param_name = node_at(ctx, idx)->fn_declaration.param_names[i];
            const VismutType *param_type = signature->function.param_types[i];

            VismutSymbol *param_symbol = NULL;
            err = SymbolsTable_AllocateLocalSymbol(
                ctx->symbols_table, SymbolEntry_CreateLocalVariable(param_name, param_type, i, 0),
                &param_symbol, &details);

            if (err != VISMUT_OK) {
                ASTAnalyzer_SetErrorInfo(ctx, err, Position_FromSubView(ctx->source, param_name),
                                         details);
                return err;
            }

            VismutScopeNode *param_node = Arena_Type(ctx->arena, VismutScopeNode, &err, &details);
            if (unlikely(err != VISMUT_OK)) {
                return err;
            }

            *param_node = VismutScopeNode_Create(param_symbol);
            VismutScope_InsertNode(ctx->state.current_scope, param_node);
        }

        const VismutType *old_expected_return = ctx->state.expected_return_type;
        ctx->state.expected_return_type = signature->function.return_type;

        const VismutType *restore_help_type = ctx->state.help_type;
        ctx->state.help_type = signature->function.return_type;

        const VismutType *body_type;
        SAFE_RISKY_EXPRESSION(
            ASTAnalyzer_Analyze(ctx, node_at(ctx, idx)->fn_declaration.body, &body_type), err);

        ctx->state.expected_return_type = old_expected_return;
        ctx->state.help_type = restore_help_type;

        const VismutType *expected_ret_type = signature->function.return_type;
        if (expected_ret_type != ctx->type_context->type_auto &&
            expected_ret_type != ctx->type_context->type_unit) {
            if (body_type != expected_ret_type && body_type != ctx->type_context->type_never) {
                ASTAnalyzer_SetErrorInfo(
                    ctx, VISMUT_ERR_UNEXPECTED_TYPE, node_at(ctx, idx)->pos,
                    (VismutErrorDetails){.type_kind = expected_ret_type->kind});
                return VISMUT_ERR_UNEXPECTED_TYPE;
            }
        }

        ctx->state.current_scope = NULL;
        ctx->state.in_function = old_in_function;

        *out_type = ctx->type_context->type_unit;
    } break;

    case VISMUT_AST_CALL: {
        const VismutType *callee_type;
        SAFE_RISKY_EXPRESSION(
            ASTAnalyzer_Analyze(ctx, node_at(ctx, idx)->call.object, &callee_type), err);

        if (callee_type->kind != VISMUT_TYPE_KIND_FUNCTION) {
            ASTAnalyzer_SetErrorInfo(ctx, VISMUT_ERR_CALL_NOT_A_FN, node_at(ctx, idx)->pos,
                                     (VismutErrorDetails){0});
            return VISMUT_ERR_CALL_NOT_A_FN;
        }

        node_at(ctx, idx)->call.type = callee_type;

        const u32 expected_args = callee_type->function.param_count;
        const u32 provided_args = node_at(ctx, idx)->call.arguments_count;

        if (expected_args != provided_args) {
            ASTAnalyzer_SetErrorInfo(ctx, VISMUT_ERR_CALL_ARITY, node_at(ctx, idx)->pos,
                                     (VismutErrorDetails){0});
            return VISMUT_ERR_CALL_ARITY;
        }

        for (u32 i = 0; i < expected_args; ++i) {
            const VismutType *old_help_type = ctx->state.help_type;
            ctx->state.help_type = callee_type->function.param_types[i];
            const ASTNodeIdx arg_node_idx = node_at(ctx, idx)->call.arguments[i];

            const VismutType *arg_type;
            SAFE_RISKY_EXPRESSION(ASTAnalyzer_Analyze(ctx, arg_node_idx, &arg_type), err);

            const VismutType *expected_type = callee_type->function.param_types[i];
            ctx->state.help_type = old_help_type;

            if (arg_type != expected_type) {
                ASTAnalyzer_SetErrorInfo(ctx, VISMUT_ERR_UNEXPECTED_TYPE,
                                         node_at(ctx, arg_node_idx)->pos,
                                         (VismutErrorDetails){.type_kind = expected_type->kind});
                return VISMUT_ERR_UNEXPECTED_TYPE;
            }
        }

        *out_type = callee_type->function.return_type;
    } break;

    case VISMUT_AST_RETURN: {
        if (!ctx->state.in_function) {
            ASTAnalyzer_SetErrorInfo(ctx, VISMUT_ERR_RETURN_OUTSIDE_FN, node_at(ctx, idx)->pos,
                                     (VismutErrorDetails){0});
            return VISMUT_ERR_RETURN_OUTSIDE_FN;
        }

        const VismutType *expr_type = ctx->type_context->type_unit;
        if (!ASTNodeIdx_IsNone(node_at(ctx, idx)->ret.expression)) {
            SAFE_RISKY_EXPRESSION(
                ASTAnalyzer_Analyze(ctx, node_at(ctx, idx)->ret.expression, &expr_type), err);
        }

        if (expr_type != ctx->type_context->type_never &&
            expr_type != ctx->state.expected_return_type) {
            ASTAnalyzer_SetErrorInfo(
                ctx, VISMUT_ERR_UNEXPECTED_TYPE, node_at(ctx, idx)->pos,
                (VismutErrorDetails){.type_kind = ctx->state.expected_return_type->kind});
            return VISMUT_ERR_UNEXPECTED_TYPE;
        }

        *out_type = ctx->type_context->type_never;
    } break;

    case VISMUT_AST_VAR_DECLARATION: {
        const VismutType *annototaion_type = node_at(ctx, idx)->var_declaration.type;
        const VismutType *restore_help_type = ctx->state.help_type;
        ctx->state.help_type = annototaion_type;

        const VismutType *init_expression_type;
        SAFE_RISKY_EXPRESSION(ASTAnalyzer_Analyze(ctx, node_at(ctx, idx)->var_declaration.init,
                                                  &init_expression_type),
                              err);
        ctx->state.help_type = restore_help_type;

        const VismutType *result_type = annototaion_type == ctx->type_context->type_auto
                                            ? init_expression_type
                                            : annototaion_type;
        node_at(ctx, idx)->var_declaration.type = result_type;

        VismutSymbol *declared_symbol = NULL;
        err = declare_variable(ctx, node_at(ctx, idx)->var_declaration.name, result_type,
                               node_at(ctx, idx)->var_declaration.is_mutable,
                               ctx->state.is_exported_decl, &declared_symbol, &details);
        if (err != VISMUT_OK) {
            ASTAnalyzer_SetErrorInfo(ctx, err, node_at(ctx, idx)->pos, details);
            return err;
        }

        node_at(ctx, idx)->var_declaration.resolved_symbol = declared_symbol;

        *out_type = ctx->type_context->type_unit;
    } break;

    case VISMUT_AST_CONDITION: {
        const VismutType *condition_type;
        SAFE_RISKY_EXPRESSION(
            ASTAnalyzer_Analyze(ctx, node_at(ctx, idx)->condition.condition, &condition_type), err);

        if (condition_type != ctx->type_context->type_i1) {
            ASTAnalyzer_SetErrorInfo(
                ctx, VISMUT_ERR_UNEXPECTED_TYPE, node_at(ctx, idx)->pos,
                (VismutErrorDetails){.type_kind = ctx->type_context->type_i1->kind});
            return VISMUT_ERR_UNEXPECTED_TYPE;
        }

        const VismutType *then_type;
        SAFE_RISKY_EXPRESSION(
            ASTAnalyzer_Analyze(ctx, node_at(ctx, idx)->condition.then, &then_type), err);

        const VismutType *else_type = ctx->type_context->type_unit;
        if (!ASTNodeIdx_IsNone(node_at(ctx, idx)->condition.else_)) {
            SAFE_RISKY_EXPRESSION(
                ASTAnalyzer_Analyze(ctx, node_at(ctx, idx)->condition.else_, &else_type), err);
        }
        if (else_type != ctx->type_context->type_unit && then_type != else_type) {
            ASTAnalyzer_SetErrorInfo(ctx, VISMUT_ERR_UNEXPECTED_TYPE, node_at(ctx, idx)->pos,
                                     (VismutErrorDetails){.type_kind = else_type->kind});
            return VISMUT_ERR_UNEXPECTED_TYPE;
        }

        const VismutType *result_type;
        if (then_type == ctx->type_context->type_never) {
            result_type = else_type;
        } else if (else_type == ctx->type_context->type_never) {
            result_type = then_type;
        } else if (then_type == else_type) {
            result_type = then_type;
        } else {
            ASTAnalyzer_SetErrorInfo(ctx, VISMUT_ERR_TYPE, node_at(ctx, idx)->pos,
                                     (VismutErrorDetails){0});
            return VISMUT_ERR_TYPE;
        }

        node_at(ctx, idx)->condition.type = result_type;
        *out_type = result_type;
    } break;

    case VISMUT_AST_LOOP: {
        const VismutType *type;
        SAFE_RISKY_EXPRESSION(ASTAnalyzer_Analyze(ctx, node_at(ctx, idx)->loop.condition, &type),
                              err);
        SAFE_RISKY_EXPRESSION(ASTAnalyzer_Analyze(ctx, node_at(ctx, idx)->loop.body, &type), err);
        *out_type = ctx->type_context->type_unit;
    } break;

    case VISMUT_AST_BLOCK: {
        const i8 old_in_block = ctx->state.in_block;
        ctx->state.in_block = 1;

        VismutScope *parent_scope = ctx->state.current_scope;
        node_at(ctx, idx)->block.scope = VismutScope_Create(parent_scope);
        ctx->state.current_scope = &node_at(ctx, idx)->block.scope;

        const VismutType *last_statement_type = ctx->type_context->type_unit;
        ASTNodeIdx statement_idx = node_at(ctx, idx)->block.first_statement;

        while (!ASTNodeIdx_IsNone(statement_idx)) {
            const VismutType *stmt_type;
            SAFE_RISKY_EXPRESSION(ASTAnalyzer_Analyze(ctx, statement_idx, &stmt_type), err);

            if (stmt_type == ctx->type_context->type_never) {
                last_statement_type = ctx->type_context->type_never;
            } else if (last_statement_type != ctx->type_context->type_never) {
                last_statement_type = stmt_type;
            }

            statement_idx = ASTNode_GetNextStatement(node_at(ctx, statement_idx));
        }

        node_at(ctx, idx)->block.type = last_statement_type;
        *out_type = last_statement_type;

        ctx->state.current_scope = parent_scope;
        ctx->state.in_block = old_in_block;
    } break;

    case VISMUT_AST_BINARY: {
        const VismutType *left_type;
        SAFE_RISKY_EXPRESSION(ASTAnalyzer_Analyze(ctx, node_at(ctx, idx)->binary.left, &left_type),
                              err);
        const VismutType *right_type;
        SAFE_RISKY_EXPRESSION(
            ASTAnalyzer_Analyze(ctx, node_at(ctx, idx)->binary.right, &right_type), err);
        const ASTBinaryNodeType operation = node_at(ctx, idx)->binary.op;
        const VismutType *result_type;
        err = GetBinaryOpResult(ctx->type_context, left_type, right_type, &result_type, operation,
                                &details);
        if (err != VISMUT_OK) {
            ASTAnalyzer_SetErrorInfo(ctx, err, node_at(ctx, idx)->pos, details);
            return err;
        }

        node_at(ctx, idx)->binary.type = result_type;
        *out_type = result_type;
    } break;

    case VISMUT_AST_UNARY: {
        const VismutType *right_type;
        SAFE_RISKY_EXPRESSION(ASTAnalyzer_Analyze(ctx, node_at(ctx, idx)->unary.right, &right_type),
                              err);
        const ASTUnaryNodeType operation = node_at(ctx, idx)->unary.op;
        const VismutType *result_type = NULL;
        err = GetUnaryOpResult(ctx->type_context, right_type, operation, &result_type, &details);
        if (err != VISMUT_OK) {
            ASTAnalyzer_SetErrorInfo(ctx, err, node_at(ctx, idx)->pos, details);
            return err;
        }

        node_at(ctx, idx)->unary.type = result_type;
        *out_type = result_type;
    } break;

    case VISMUT_AST_IDENTIFIER: {
        VismutSymbol *symbol = NULL;
        err = find_symbol_from_scope(ctx, node_at(ctx, idx)->identifier.name, &symbol, &details);
        if (unlikely(err != VISMUT_OK)) {
            ASTAnalyzer_SetErrorInfo(ctx, err, node_at(ctx, idx)->pos, details);
            return err;
        }

        node_at(ctx, idx)->identifier.resolved_symbol = symbol;
        node_at(ctx, idx)->identifier.type = symbol->type;

        *out_type = symbol->type;
    } break;

    case VISMUT_AST_LITERAL: {
        const VismutType *new_literal_type;
        const int convert =
            get_narrowed_literal_type(ctx->type_context, node_at(ctx, idx)->literal.type,
                                      ctx->state.help_type, &new_literal_type);
        if (convert) {
            VismutSimpleValue new_value = {0};
            SAFE_RISKY_EXPRESSION(narrow_literal(node_at(ctx, idx)->literal.value,
                                                 node_at(ctx, idx)->literal.type->kind,
                                                 new_literal_type->kind, &new_value),
                                  err);
            node_at(ctx, idx)->literal.value = new_value;
            node_at(ctx, idx)->literal.type = new_literal_type;
        }
        *out_type = new_literal_type;
    } break;

    case VISMUT_AST_ASSIGNMENT: {
        if (node_at(ctx, node_at(ctx, idx)->assignment.target)->type == VISMUT_AST_IDENTIFIER) {
            const StringView name =
                node_at(ctx, node_at(ctx, idx)->assignment.target)->identifier.name;

            VismutSymbol *symbol = NULL;
            err = find_symbol_from_scope(ctx, name, &symbol, &details);
            if (err != VISMUT_OK) {
                ASTAnalyzer_SetErrorInfo(ctx, err, node_at(ctx, idx)->pos, details);
                return err;
            }

            int is_mutable = 0;
            if (symbol->kind == VISMUT_SYMBOL_LOCAL_VAR) {
                is_mutable = symbol->as.local_var.is_mutable;
            } else {
                is_mutable = symbol->as.global_var.is_mutable;
            }

            if (!is_mutable) {
                ASTAnalyzer_SetErrorInfo(ctx, VISMUT_ERR_CONST_MUTATION, node_at(ctx, idx)->pos,
                                         (VismutErrorDetails){0});
                return VISMUT_ERR_CONST_MUTATION;
            }
        }

        const VismutType *target_type;
        SAFE_RISKY_EXPRESSION(
            ASTAnalyzer_Analyze(ctx, node_at(ctx, idx)->assignment.target, &target_type), err);

        const VismutType *value_type;
        SAFE_RISKY_EXPRESSION(
            ASTAnalyzer_Analyze(ctx, node_at(ctx, idx)->assignment.value, &value_type), err);

        err = CheckAssignment(ctx->type_context, target_type, value_type,
                              node_at(ctx, idx)->assignment.operation, &details);
        if (err != VISMUT_OK) {
            ASTAnalyzer_SetErrorInfo(ctx, err, node_at(ctx, idx)->pos, details);
            return err;
        }

        *out_type = ctx->type_context->type_unit;
    } break;

    case VISMUT_AST_TYPE_CAST: {
        const VismutType *restore_help_type = ctx->state.help_type;
        ctx->state.help_type = node_at(ctx, idx)->type_cast.to_type;

        const VismutType *from_type;
        SAFE_RISKY_EXPRESSION(
            ASTAnalyzer_Analyze(ctx, node_at(ctx, idx)->type_cast.argument, &from_type), err);

        node_at(ctx, idx)->type_cast.from_type = from_type;
        ctx->state.help_type = restore_help_type;
        *out_type = node_at(ctx, idx)->type_cast.to_type;
    } break;

    case VISMUT_AST_IMPORT: {
        *out_type = ctx->type_context->type_unit;
    } break;

    case VISMUT_AST_DOT: {
        const VismutType *left_type;
        SAFE_RISKY_EXPRESSION(ASTAnalyzer_Analyze(ctx, node_at(ctx, idx)->dot.object, &left_type),
                              err);

        if (left_type->kind != VISMUT_TYPE_KIND_MODULE_REF) {
            ASTAnalyzer_SetErrorInfo(ctx, VISMUT_ERR_DOT_ON_NON_MODULE, node_at(ctx, idx)->pos,
                                     (VismutErrorDetails){0});
            return VISMUT_ERR_DOT_ON_NON_MODULE;
        }

        const VismutModuleIdx target_module_idx = left_type->module_ref.idx;
        const StringView member_name = node_at(ctx, idx)->dot.member;

        VismutModuleEntry *target_module =
            VismutModulesTable_ModuleAt(ctx->modules, target_module_idx);

        const u32 name_hash = StringView_Hash(member_name);

        VismutSymbol *member_symbol =
            SymbolsTable_FindSymbol(&target_module->symbols, member_name, name_hash);

        if (unlikely(member_symbol == NULL)) {
            ASTAnalyzer_SetErrorInfo(ctx, VISMUT_ERR_NAME_NOT_FOUND, node_at(ctx, idx)->pos,
                                     (VismutErrorDetails){0});
            return VISMUT_ERR_NAME_NOT_FOUND;
        }

        if (!member_symbol->is_exported) {
            ASTAnalyzer_SetErrorInfo(ctx, VISMUT_ERR_PRIVATE_MEMBER, node_at(ctx, idx)->pos,
                                     (VismutErrorDetails){0});
            return VISMUT_ERR_PRIVATE_MEMBER;
        }

        node_at(ctx, idx)->dot.resolved_symbol = member_symbol;
        node_at(ctx, idx)->dot.type = member_symbol->type;

        *out_type = member_symbol->type;
    } break;

    default:
        assert(0 && "Unreachable!");
        return VISMUT_ERR_UNREACHABLE;
    }

    return VISMUT_OK;
}
