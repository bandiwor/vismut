#include "compiler.h"
#include "binary.h"
#include "op_codes.h"
#include "unary.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    VismutModuleEntry *module;
    ASTNodeIdx node_idx;
} DefferedFunction;

typedef struct {
    u32 instruction_index;
    VismutSymbol *target_func;
} CallPatch;

VismutCompiler VismutCompiler_Create(VismutModulesTable *modules_table,
                                     VismutModuleIdx *modules_order, u32 modules_order_size,
                                     VismutTypeContext *type_context, ConstantPool *constant_pool,
                                     VismutErrorInfo *error_info) {
    return (VismutCompiler){
        .contant_pool = constant_pool,
        .type_context = type_context,
        .modules_table = modules_table,
        .modules_order = modules_order,
        .modules_order_size = modules_order_size,
        .deffered_functions = RawVector_Create(),
        .call_patches = RawVector_Create(),
        .error_info = error_info,
        .bytecode = RawVector_Create(),
        .ctx =
            {
                .next_free_reg = 0,
                .next_global_slot = 0,
                .current_module_entry = NULL,
            },
    };
}

VismutErrorType VismutCompiler_Init(VismutCompiler *restrict compiler,
                                    VismutErrorDetails *restrict out_details) {
    VismutErrorType err;
    SAFE_RISKY_EXPRESSION(RawVector_Init(&compiler->bytecode, out_details), err);
    SAFE_RISKY_EXPRESSION(RawVector_Init(&compiler->call_patches, out_details), err);
    SAFE_RISKY_EXPRESSION(RawVector_Init(&compiler->deffered_functions, out_details), err);
    return VISMUT_OK;
}

void VismutCompiler_Destroy(VismutCompiler *compiler) {
    RawVector_Free(&compiler->bytecode);
    RawVector_Free(&compiler->call_patches);
    RawVector_Free(&compiler->deffered_functions);
}

static void VismutCompiler_SetErrorInfo(VismutCompiler *compiler, const VismutErrorType type,
                                        const Position pos, VismutErrorDetails details) {
    compiler->error_info->type = type;
    compiler->error_info->pos = pos;
    compiler->error_info->details = details;
}

static VismutErrorType emit(VismutCompiler *compiler, const VismutInstruction inst) {
    VismutErrorType err;
    VismutErrorDetails details = {0};

    RawVector_Push(&compiler->bytecode, VismutInstruction, inst, err, &details);
    if (err != VISMUT_OK) {
        VismutCompiler_SetErrorInfo(compiler, err, Position_Create(0, 0), details);
        return err;
    }

    return VISMUT_OK;
}

static VismutErrorType emit_wide(VismutCompiler *compiler, const VismutWideInstruction inst) {
    VismutErrorType err;
    VismutErrorDetails details = {0};

    const VismutInstruction word1 = (VismutInstruction)(inst & 0xFFFFFFFF);
    const VismutInstruction word2 = (VismutInstruction)(inst >> 32);

    RawVector_Push(&compiler->bytecode, VismutInstruction, word1, err, &details);
    if (err != VISMUT_OK) {
        VismutCompiler_SetErrorInfo(compiler, err, Position_Create(0, 0), details);
        return err;
    }

    RawVector_Push(&compiler->bytecode, VismutInstruction, word2, err, &details);
    if (err != VISMUT_OK) {
        VismutCompiler_SetErrorInfo(compiler, err, Position_Create(0, 0), details);
        return err;
    }

    return VISMUT_OK;
}

#define at(idx) ASTBuilder_NodeAt(&ctx->ctx.current_module_entry->ast_builder, idx)

static u32 get_next_reg(VismutCompiler *ctx) {
    const u32 reg = ctx->ctx.next_free_reg;
    ++ctx->ctx.next_free_reg;
    return reg;
}

ASTNodeIdx list_next(const ASTNode *node) {
    switch (node->type) {
    case VISMUT_AST_EXPRESSION:
        return node->expression.next;
    case VISMUT_AST_DECLARATION:
        return node->declaration.next;
    default:
        return ASTNodeIdx_None;
    }
}

ASTNodeIdx list_node_payload(const ASTNode *node) {
    switch (node->type) {
    case VISMUT_AST_EXPRESSION:
        return node->expression.expr;
    case VISMUT_AST_DECLARATION:
        return node->declaration.decl;
    default:
        return ASTNodeIdx_None;
    }
}

VismutErrorType VismutCompiler_InternLiteral(VismutCompiler *restrict ctx,
                                             ASTNode *restrict literal_node,
                                             ConstantPoolIdx *restrict out_idx) {
    VismutErrorType err;
    VismutErrorDetails details;

    switch (literal_node->literal.type->kind) {
    case VISMUT_TYPE_KIND_I1:
    case VISMUT_TYPE_KIND_I8:
    case VISMUT_TYPE_KIND_I16:
    case VISMUT_TYPE_KIND_I32:
    case VISMUT_TYPE_KIND_I64: {
        err = ConstantPool_InternInt(ctx->contant_pool, literal_node->literal.value.i, out_idx,
                                     &details);
        if (err != VISMUT_OK) {
            VismutCompiler_SetErrorInfo(ctx, err, literal_node->pos, details);
        }
    } break;
    case VISMUT_TYPE_KIND_U8:
    case VISMUT_TYPE_KIND_U16:
    case VISMUT_TYPE_KIND_U32:
    case VISMUT_TYPE_KIND_U64: {
        err = ConstantPool_InternUInt(ctx->contant_pool, literal_node->literal.value.u, out_idx,
                                      &details);
        if (err != VISMUT_OK) {
            VismutCompiler_SetErrorInfo(ctx, err, literal_node->pos, details);
        }
    } break;
    case VISMUT_TYPE_KIND_F32:
    case VISMUT_TYPE_KIND_F64: {
        err = ConstantPool_InternFloat(ctx->contant_pool, literal_node->literal.value.f, out_idx,
                                       &details);
        if (err != VISMUT_OK) {
            VismutCompiler_SetErrorInfo(ctx, err, literal_node->pos, details);
        }
    } break;
    case VISMUT_TYPE_KIND_STR: {
        err = ConstantPool_InternString(ctx->contant_pool, literal_node->literal.value.str, out_idx,
                                        &details);
        if (err != VISMUT_OK) {
            VismutCompiler_SetErrorInfo(ctx, err, literal_node->pos, details);
        }
    } break;
    default:
        assert(0 && "Unreachable!\n");
        return VISMUT_ERR_UNREACHABLE;
    }

    return VISMUT_OK;
}

VismutErrorType VismutCompiler_CompileExpression(VismutCompiler *ctx, const ASTNodeIdx idx,
                                                 u8 *restrict out_reg);

VismutErrorType VismutCompiler_CompileBinary(VismutCompiler *ctx, const VismutOpcode opcode,
                                             const ASTNodeIdx left, const ASTNodeIdx right,
                                             u8 *restrict out_reg) {
    VismutErrorType err;

    const u8 old_next_free_reg = ctx->ctx.next_free_reg;

    u8 reg_left, reg_right;
    SAFE_RISKY_EXPRESSION(VismutCompiler_CompileExpression(ctx, left, &reg_left), err);
    SAFE_RISKY_EXPRESSION(VismutCompiler_CompileExpression(ctx, right, &reg_right), err);
    ctx->ctx.next_free_reg = old_next_free_reg;

    const u8 result_reg = get_next_reg(ctx);
    SAFE_RISKY_EXPRESSION(emit(ctx, VismutOpcode_ABC(opcode, result_reg, reg_left, reg_right)),
                          err);

    *out_reg = result_reg;
    return VISMUT_OK;
}

VismutErrorType VismutCompiler_CompileUnary(VismutCompiler *ctx, const VismutOpcode opcode,
                                            const ASTNodeIdx right, u8 *restrict out_reg) {
    VismutErrorType err;

    const u8 old_next_free_reg = ctx->ctx.next_free_reg;

    u8 reg_right;
    SAFE_RISKY_EXPRESSION(VismutCompiler_CompileExpression(ctx, right, &reg_right), err);
    ctx->ctx.next_free_reg = old_next_free_reg;

    const u8 result_reg = get_next_reg(ctx);
    SAFE_RISKY_EXPRESSION(emit(ctx, VismutOpcode_AB(opcode, result_reg, reg_right)), err);

    *out_reg = result_reg;
    return VISMUT_OK;
}

VismutErrorType VismutCompiler_CompileStatement(VismutCompiler *ctx, const ASTNodeIdx idx);

VismutErrorType VismutCompiler_CompileExpression(VismutCompiler *ctx, const ASTNodeIdx idx,
                                                 u8 *restrict out_reg) {
    VismutErrorType err;
    switch (at(idx)->type) {
    case VISMUT_AST_LITERAL: {
        ConstantPoolIdx cp_idx;
        SAFE_RISKY_EXPRESSION(VismutCompiler_InternLiteral(ctx, at(idx), &cp_idx), err);

        const u8 target_reg = get_next_reg(ctx);
        SAFE_RISKY_EXPRESSION(emit(ctx, VismutOpcode_ABX(OP_LOAD_CONST, target_reg, cp_idx)), err)

        *out_reg = target_reg;
    }
        return VISMUT_OK;

    case VISMUT_AST_UNARY: {
        VismutErrorDetails details;
        const ASTNodeIdx right_node = at(idx)->unary.right;
        const VismutType *right_t = ASTNode_GetType(ctx->type_context, at(right_node));

        if (at(idx)->unary.op == VISMUT_AST_UNARY_PLUS) {
            return VismutCompiler_CompileExpression(ctx, at(idx)->unary.right, out_reg);
        }

        VismutOpcode op_code;
        err = VismutCompiler_GetUnaryInstruction(at(idx)->unary.op, right_t, &op_code, &details);
        if (err != VISMUT_OK) {
            VismutCompiler_SetErrorInfo(ctx, err, at(idx)->pos, details);
            return err;
        }

        return VismutCompiler_CompileUnary(ctx, op_code, right_node, out_reg);
    }
        return VISMUT_OK;

    case VISMUT_AST_BINARY: {
        VismutErrorDetails details;
        const ASTNodeIdx left_node = at(idx)->binary.left;
        const ASTNodeIdx right_node = at(idx)->binary.right;
        const VismutType *left_t = ASTNode_GetType(ctx->type_context, at(left_node));
        const VismutType *right_t = ASTNode_GetType(ctx->type_context, at(right_node));

        VismutOpcode op_code;
        err = VismutCompiler_GetBinaryInstruction(at(idx)->binary.op, left_t, right_t, &op_code,
                                                  &details);
        if (err != VISMUT_OK) {
            VismutCompiler_SetErrorInfo(ctx, err, at(idx)->pos, details);
            return err;
        }

        return VismutCompiler_CompileBinary(ctx, op_code, left_node, right_node, out_reg);
    };

    case VISMUT_AST_IDENTIFIER: {
        VismutSymbol *sym = at(idx)->identifier.resolved_symbol;

        if (sym->kind == VISMUT_SYMBOL_GLOBAL_VAR) {
            const u8 target_reg = get_next_reg(ctx);
            SAFE_RISKY_EXPRESSION(emit(ctx, VismutOpcode_ABX(OP_LOAD_GLOBAL, target_reg,
                                                             sym->as.global_var.global_index)),
                                  err);
            *out_reg = target_reg;
        } else if (sym->kind == VISMUT_SYMBOL_LOCAL_VAR) {
            *out_reg = sym->as.local_var.register_index;
        }

        return VISMUT_OK;
    }

    case VISMUT_AST_CALL: {
        VismutErrorDetails details;
        ASTNode *call_node = at(idx);
        u32 arg_count = call_node->call.arguments_count;

        u8 temp_arg_regs[256];
        for (u32 i = 0; i < arg_count; ++i) {
            SAFE_RISKY_EXPRESSION(VismutCompiler_CompileExpression(
                                      ctx, call_node->call.arguments[i], &temp_arg_regs[i]),
                                  err);
        }

        const u8 args_start = ctx->ctx.next_free_reg;
        ctx->ctx.next_free_reg += (arg_count > 0 ? arg_count : 1);

        for (u32 i = 0; i < arg_count; ++i) {
            SAFE_RISKY_EXPRESSION(
                emit(ctx, VismutOpcode_ABC(OP_MOVE, args_start + i, temp_arg_regs[i], 0)), err);
        }

        ASTNode *target_node = at(call_node->call.object);
        VismutSymbol *target_symbol = NULL;
        if (target_node->type == VISMUT_AST_IDENTIFIER) {
            target_symbol = target_node->identifier.resolved_symbol;
        } else if (target_node->type == VISMUT_AST_DOT) {
            target_symbol = target_node->dot.resolved_symbol;
        }

        const u32 patch_index = RawVector_Count(&ctx->bytecode, VismutInstruction);
        VismutWideInstruction call_inst =
            VismutOpcode_ABC_WIDE(OP_CALL, args_start, arg_count, 0, 0xFFFFFFFF);
        SAFE_RISKY_EXPRESSION(emit_wide(ctx, call_inst), err);

        CallPatch patch = {.instruction_index = patch_index, .target_func = target_symbol};
        RawVector_Push(&ctx->call_patches, CallPatch, patch, err, &details);
        *out_reg = args_start;
    }
        return VISMUT_OK;

    case VISMUT_AST_BLOCK: {
        ASTNodeIdx current = at(idx)->block.first_statement;
        u8 result_reg = 0;

        while (!ASTNodeIdx_IsNone(current)) {
            ASTNode *stmt = at(current);

            if (stmt->type == VISMUT_AST_EXPRESSION && ASTNodeIdx_IsNone(stmt->expression.next)) {
                SAFE_RISKY_EXPRESSION(
                    VismutCompiler_CompileExpression(ctx, stmt->expression.expr, &result_reg), err);
            } else {
                SAFE_RISKY_EXPRESSION(VismutCompiler_CompileStatement(ctx, current), err);
            }

            current = list_next(stmt);
        }

        *out_reg = result_reg; // Отдаем регистр наверх!
    }
        return VISMUT_OK;

    case VISMUT_AST_RETURN: {
        ASTNodeIdx expr_idx = at(idx)->ret.expression;

        if (!ASTNodeIdx_IsNone(expr_idx)) {
            u8 ret_reg;

            SAFE_RISKY_EXPRESSION(VismutCompiler_CompileExpression(ctx, expr_idx, &ret_reg), err);
            SAFE_RISKY_EXPRESSION(emit(ctx, VismutOpcode_A(OP_RET, ret_reg)), err);
        } else {
            SAFE_RISKY_EXPRESSION(emit(ctx, VismutOpcode_A(OP_RET, 0)), err);
        }
    }
        return VISMUT_OK;

    case VISMUT_AST_CONDITION: {
        u8 cond_reg;
        SAFE_RISKY_EXPRESSION(
            VismutCompiler_CompileExpression(ctx, at(idx)->condition.condition, &cond_reg), err);

        u32 jmp_if_false_idx = RawVector_Count(&ctx->bytecode, VismutInstruction);

        SAFE_RISKY_EXPRESSION(emit(ctx, VismutOpcode_ABX(OP_JMP_IF_FALSE, cond_reg, 0)),
                              err); // 0 - пока заглушка

        // 3. Компилируем блок THEN
        SAFE_RISKY_EXPRESSION(VismutCompiler_CompileStatement(ctx, at(idx)->condition.then), err);

        // Если есть ELSE, нам нужен еще один прыжок (чтобы после THEN не провалиться в ELSE)
        u32 jmp_end_idx = 0;
        i1 has_else = !ASTNodeIdx_IsNone(at(idx)->condition.else_);

        if (has_else) {
            jmp_end_idx = RawVector_Count(&ctx->bytecode, VismutInstruction);
            // Допустим, OP_JMP использует свободный формат (например, ABX, где A=0, BX=offset)
            SAFE_RISKY_EXPRESSION(emit(ctx, VismutOpcode_ABX(OP_JMP, 0, 0)), err); // заглушка
        }

        // 4. Патчим первый прыжок (JMP_IF_FALSE)
        // Теперь мы знаем, где начинается ELSE (или конец IF, если ELSE нет)
        u32 else_start_idx = RawVector_Count(&ctx->bytecode, VismutInstruction);

        VismutInstruction *instructions = (VismutInstruction *)ctx->bytecode.memory;
        instructions[jmp_if_false_idx] =
            VismutOpcode_ABX(OP_JMP_IF_FALSE, cond_reg, else_start_idx);

        // 5. Компилируем блок ELSE (если он есть) и патчим JMP в конце THEN
        if (has_else) {
            SAFE_RISKY_EXPRESSION(VismutCompiler_CompileStatement(ctx, at(idx)->condition.else_),
                                  err);

            u32 end_idx = RawVector_Count(&ctx->bytecode, VismutInstruction);
            instructions[jmp_end_idx] = VismutOpcode_ABX(OP_JMP, 0, end_idx);
        }

        break;
    }

    default:
        printf("%s\n", ASTNodeType_String(at(idx)->type));
        assert(0 && "Unreachable!\n");
        return VISMUT_ERR_UNREACHABLE;
    }
}

VismutErrorType VismutCompiler_CompileStatement(VismutCompiler *ctx, const ASTNodeIdx idx) {
    VismutErrorType err;

    switch (at(idx)->type) {
    case VISMUT_AST_MODULE: {
        ASTNodeIdx current = at(idx)->module.first_statement;
        while (!ASTNodeIdx_IsNone(current)) {
            SAFE_RISKY_EXPRESSION(VismutCompiler_CompileStatement(ctx, current), err);
            current = list_next(at(current));
        }
    } break;

    case VISMUT_AST_EXPRESSION: {
        const u32 old_free_reg = ctx->ctx.next_free_reg;
        u8 reg;
        SAFE_RISKY_EXPRESSION(VismutCompiler_CompileExpression(ctx, at(idx)->expression.expr, &reg),
                              err);
        ctx->ctx.next_free_reg = old_free_reg;
    } break;

    case VISMUT_AST_DECLARATION: {
        VismutErrorDetails details;
        ASTNodeIdx decl_idx = list_node_payload(at(idx));
        ASTNode *decl = at(decl_idx);

        if (decl->type == VISMUT_AST_FN_DECLARATION) {
            DefferedFunction def_fn = {.module = ctx->ctx.current_module_entry,
                                       .node_idx = decl_idx};
            RawVector_Push(&ctx->deffered_functions, DefferedFunction, def_fn, err, &details);
            if (err != VISMUT_OK) {
                VismutCompiler_SetErrorInfo(ctx, err, decl->pos, details);
                return err;
            }
        } else if (decl->type == VISMUT_AST_VAR_DECLARATION) {
            const u32 old_free_reg = ctx->ctx.next_free_reg;
            u8 reg;
            SAFE_RISKY_EXPRESSION(
                VismutCompiler_CompileExpression(ctx, decl->var_declaration.init, &reg), err);

            VismutSymbol *sym = decl->var_declaration.resolved_symbol;

            if (sym->kind == VISMUT_SYMBOL_GLOBAL_VAR) {
                sym->as.global_var.global_index = ctx->ctx.next_global_slot++;

                SAFE_RISKY_EXPRESSION(emit(ctx, VismutOpcode_ABX(OP_STORE_GLOBAL, reg,
                                                                 sym->as.global_var.global_index)),
                                      err);
                ctx->ctx.next_free_reg = old_free_reg;
            } else if (sym->kind == VISMUT_SYMBOL_LOCAL_VAR) {
                sym->as.local_var.register_index = reg;
            }
        }
    } break;

    default:
        break;
    }

    return VISMUT_OK;
}

VismutErrorType VismutCompiler_Compile(VismutCompiler *ctx) {
    VismutErrorType err;

    for (u32 i = 0; i < ctx->modules_order_size; ++i) {
        VismutModuleIdx mod_idx = ctx->modules_order[i];

        ctx->ctx.current_module_entry =
            &RawVector_At(&ctx->modules_table->entries, VismutModuleEntry, mod_idx);

        ASTNodeIdx root_module_node = ctx->ctx.current_module_entry->root_idx;
        SAFE_RISKY_EXPRESSION(VismutCompiler_CompileStatement(ctx, root_module_node), err);
    }

    SAFE_RISKY_EXPRESSION(emit(ctx, VismutOpcode_None(OP_HALT)), err);

    const u32 funcs_count = RawVector_Count(&ctx->deffered_functions, DefferedFunction);
    DefferedFunction *funcs = (DefferedFunction *)ctx->deffered_functions.memory;

    for (u32 i = 0; i < funcs_count; ++i) {
        DefferedFunction def_fn = funcs[i];

        ASTNode *fn_node = at(def_fn.node_idx);
        VismutSymbol *fn_symbol = fn_node->fn_declaration.resolved_symbol;

        const u32 args_count = fn_node->fn_declaration.signature->function.param_count;

        ctx->ctx.current_module_entry = def_fn.module;
        ctx->ctx.next_free_reg = args_count;

        fn_symbol->as.func.bytecode_offset = RawVector_Count(&ctx->bytecode, VismutInstruction);

        ASTNodeIdx current_stmt = fn_node->fn_declaration.body;
        u8 reg;
        SAFE_RISKY_EXPRESSION(VismutCompiler_CompileExpression(ctx, current_stmt, &reg), err);

        fn_symbol->as.func.registers_needed = ctx->ctx.next_free_reg;
        SAFE_RISKY_EXPRESSION(emit(ctx, VismutOpcode_A(OP_RET, reg)), err);
    }

    const u32 patches_count = RawVector_Count(&ctx->call_patches, CallPatch);
    CallPatch *patches = (CallPatch *)ctx->call_patches.memory;
    VismutInstruction *instructions = (VismutInstruction *)ctx->bytecode.memory;

    for (u32 i = 0; i < patches_count; ++i) {
        CallPatch patch = patches[i];
        u32 patch_idx = patch.instruction_index;

        VismutWideInstruction old_wide =
            VismutOpCode_MakeWide(instructions[patch_idx], instructions[patch_idx + 1]);

        VismutDecodedABCWide decoded = VismutInstruction_DecodeABCWide(old_wide);

        u32 real_offset = patch.target_func->as.func.bytecode_offset;
        u8 regs_needed = patch.target_func->as.func.registers_needed;

        VismutWideInstruction patched_wide =
            VismutOpcode_ABC_WIDE(decoded.type, decoded.a, decoded.b, regs_needed, real_offset);

        instructions[patch_idx] = (u32)(patched_wide & 0xFFFFFFFF);
        instructions[patch_idx + 1] = (u32)(patched_wide >> 32);
    }

    return VISMUT_OK;
}
