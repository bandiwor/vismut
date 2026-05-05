#include "ast_parser.h"
#include "../memory/type_context.h"
#include "../utils/ast_utils.h"
#include "../utils/overflow.h"
#include <assert.h>
#include <stdatomic.h>

ASTParser ASTParser_Create(ASTBuilder *restrict builder, VismutTypeContext *restrict type_ctx,
                           SymbolsTable *restrict symbols_table,
                           VismutModulesTable *restrict modules_table,
                           const VismutModuleIdx current_module_idx) {
    return (ASTParser){
        .module_node = ASTNodeIdx_None,
        .builder = builder,
        .arena = builder->tokenizer->arena,
        .type_ctx = type_ctx,
        .symbols_table = symbols_table,
        .error_info = builder->tokenizer->error_info,
        .modules_table = modules_table,
        .current_module_idx = current_module_idx,
        .current_token = {0},
        .ctx =
            {
                .is_in_function_declaration = 0,
                .is_exported_decl = 0,
            },
    };
}

static void ASTParser_SetErrorInfo(ASTParser *parser, const VismutErrorType type,
                                   const Position pos, const VismutErrorDetails details) {
    parser->error_info->type = type;
    parser->error_info->pos = pos;
    parser->error_info->details = details;
    parser->error_info->source = (StringView){.data = (u8 *)parser->builder->tokenizer->start,
                                              .length = parser->builder->tokenizer->limit -
                                                        parser->builder->tokenizer->start};
}

attribute_nodiscard static inline VismutErrorType
ASTParser_PushNode(const ASTParser *parser, const ASTNode node, ASTNodeIdx *out_idx,
                   VismutErrorDetails *out_details) {
    return ASTBuilder_PushNode(parser->builder, &node, out_idx, out_details);
}

static VismutToken ASTParser_Peek(const ASTParser *parser) {
    return parser->current_token;
}

attribute_nodiscard static VismutErrorType ASTParser_Next(ASTParser *parser) {
    return ASTBuilder_NextToken(parser->builder, &parser->current_token);
}

attribute_nodiscard static VismutErrorType ASTParser_AssertTokenType(ASTParser *restrict parser,
                                                                     const VismutTokenType type) {
    if (unlikely(ASTParser_Peek(parser).type != type)) {
        ASTParser_SetErrorInfo(parser, VISMUT_ERR_UNEXPECTED_TOKEN, ASTParser_Peek(parser).position,
                               (VismutErrorDetails){.token_type = type});
        return VISMUT_ERR_UNEXPECTED_TOKEN;
    }
    return VISMUT_OK;
}

attribute_nodiscard static VismutErrorType ASTParser_NextTokenExcept(ASTParser *restrict parser,
                                                                     const VismutTokenType type) {
    VismutErrorType err;
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
    return ASTParser_AssertTokenType(parser, type);
}

attribute_nodiscard attribute_always_inline static inline ASTNode *
ASTParser_NodeAt(ASTParser *parser, const ASTNodeIdx idx) {
    return ASTBuilder_NodeAt(parser->builder, idx);
}

attribute_nodiscard static VismutErrorType ASTParser_ParseStatement(ASTParser *restrict parser,
                                                                    ASTNodeIdx *restrict out_idx);

attribute_nodiscard attribute_nonnull(1) VismutErrorType ASTParser_Parse(ASTParser *parser) {
    VismutErrorType err;
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    ASTNodeIdx first_expression = ASTNodeIdx_None;
    ASTNodeIdx last_expression = ASTNodeIdx_None;

    while (ASTParser_Peek(parser).type != VISMUT_TOKEN_EOF) {
        ASTNodeIdx current_expression;
        SAFE_RISKY_EXPRESSION(ASTParser_ParseStatement(parser, &current_expression), err);

        if (ASTNodeIdx_IsNone(first_expression)) {
            first_expression = current_expression;
        } else {
            ASTBuilder_LinkNodes(parser->builder, last_expression, current_expression);
        }

        last_expression = current_expression;
    }

    err = ASTParser_PushNode(parser, ASTNode_CreateModule(first_expression), &parser->module_node,
                             &parser->error_info->details);
    if (err != VISMUT_OK) {
        parser->error_info->type = err;
        parser->error_info->pos = (Position){0, 0};
        return err;
    }

    return VISMUT_OK;
}

attribute_nodiscard static VismutErrorType ASTParser_ParseExpression(ASTParser *restrict parser,
                                                                     ASTNodeIdx *restrict out_idx);

attribute_nodiscard static VismutErrorType
ASTParser_ParseNoWrappedExpression(ASTParser *restrict parser, ASTNodeIdx *restrict out_idx);

attribute_nodiscard static VismutErrorType
ASTParser_ParseSubExpression(ASTParser *restrict parser, ASTNodeIdx *restrict out_idx);

attribute_nodiscard static VismutErrorType
ASTParser_ParseType(ASTParser *restrict parser, const VismutType *restrict *restrict out_type) {
    VismutErrorType err;
    VismutErrorDetails details;

    const VismutTokenType token_type = ASTParser_Peek(parser).type;

    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    switch (token_type) {
    case VISMUT_TOKEN_I8_TYPE:
        *out_type = parser->type_ctx->type_i8;
        return VISMUT_OK;
    case VISMUT_TOKEN_I16_TYPE:
        *out_type = parser->type_ctx->type_i16;
        return VISMUT_OK;
    case VISMUT_TOKEN_I32_TYPE:
        *out_type = parser->type_ctx->type_i32;
        return VISMUT_OK;
    case VISMUT_TOKEN_I64_TYPE:
        *out_type = parser->type_ctx->type_i64;
        return VISMUT_OK;
    case VISMUT_TOKEN_U8_TYPE:
        *out_type = parser->type_ctx->type_u8;
        return VISMUT_OK;
    case VISMUT_TOKEN_U16_TYPE:
        *out_type = parser->type_ctx->type_u16;
        return VISMUT_OK;
    case VISMUT_TOKEN_U32_TYPE:
        *out_type = parser->type_ctx->type_u32;
        return VISMUT_OK;
    case VISMUT_TOKEN_U64_TYPE:
        *out_type = parser->type_ctx->type_u64;
        return VISMUT_OK;
    case VISMUT_TOKEN_F32_TYPE:
        *out_type = parser->type_ctx->type_f32;
        return VISMUT_OK;
    case VISMUT_TOKEN_F64_TYPE:
        *out_type = parser->type_ctx->type_f64;
        return VISMUT_OK;
    case VISMUT_TOKEN_STR_TYPE:
        *out_type = parser->type_ctx->type_str;
        return VISMUT_OK;
    case VISMUT_TOKEN_LANGLE: {
        const VismutType *vector_type;
        SAFE_RISKY_EXPRESSION(ASTParser_ParseType(parser, &vector_type), err);

        SAFE_RISKY_EXPRESSION(ASTParser_AssertTokenType(parser, VISMUT_TOKEN_RANGLE), err);

        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

        err = VismutTypeContext_GetVector(parser->type_ctx, vector_type, out_type, &details);
        if (err != VISMUT_OK) {
            ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
            return err;
        }
        return VISMUT_OK;
    }
    case VISMUT_TOKEN_LBRACKET: {
        const VismutType *array_type;
        SAFE_RISKY_EXPRESSION(ASTParser_ParseType(parser, &array_type), err);

        SAFE_RISKY_EXPRESSION(ASTParser_AssertTokenType(parser, VISMUT_TOKEN_SEMICOLON), err);

        SAFE_RISKY_EXPRESSION(ASTParser_NextTokenExcept(parser, VISMUT_TOKEN_INT_LITERAL), err);

        const i64 array_length = ASTParser_Peek(parser).data.i.value;
        if (array_length <= 0) {
            ASTParser_SetErrorInfo(parser, VISMUT_ERR_ARRAY_LENGTH, ASTParser_Peek(parser).position,
                                   (VismutErrorDetails){0});
            return VISMUT_ERR_ARRAY_LENGTH;
        }
        SAFE_RISKY_EXPRESSION(ASTParser_NextTokenExcept(parser, VISMUT_TOKEN_RBRACKET), err);
        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

        err = VismutTypeContext_GetArray(parser->type_ctx, array_type, (u64)array_length, out_type,
                                         &details);
        if (err != VISMUT_OK) {
            ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
            return err;
        }
        return VISMUT_OK;
    }
    case VISMUT_TOKEN_LPAREN: { // (t1, ...) -> r
        const VismutType *types[16];
        u32 types_count = 0;
        const u32 max_types_count = COUNTOF(types);
        while (ASTParser_Peek(parser).type != VISMUT_TOKEN_RPAREN) {
            if (types_count >= max_types_count) {
                ASTParser_SetErrorInfo(parser, VISMUT_ERR_ARRAY_LENGTH,
                                       ASTParser_Peek(parser).position, (VismutErrorDetails){0});
                return VISMUT_ERR_TOO_MANY_PARAMS;
            }
            const VismutType *type;
            SAFE_RISKY_EXPRESSION(ASTParser_ParseType(parser, &type), err);
            types[types_count++] = type;
            if (ASTParser_Peek(parser).type == VISMUT_TOKEN_COMMA) {
                SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
            }
        }
        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
        if (ASTParser_Peek(parser).type != VISMUT_TOKEN_ARROW_RIGHT) { // parse Tuple
            if (types_count == 0) {
                *out_type = parser->type_ctx->type_unit;
                return VISMUT_OK;
            }
            err = VismutTypeContext_GetTuple(parser->type_ctx, types, types_count, out_type,
                                             &details);
            if (err != VISMUT_OK) {
                ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
                return err;
            }
            return VISMUT_OK;
        }
        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
        const VismutType *return_type;
        SAFE_RISKY_EXPRESSION(ASTParser_ParseType(parser, &return_type), err);

        err = VismutTypeContext_GetFunction(parser->type_ctx, return_type, types, types_count,
                                            out_type, &details);
        if (err != VISMUT_OK) {
            ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
            return err;
        }
        return VISMUT_OK;
    }

    default:
        ASTParser_SetErrorInfo(parser, VISMUT_ERR_TYPE_SYNTAX, ASTParser_Peek(parser).position,
                               (VismutErrorDetails){0});
        return VISMUT_ERR_TYPE_SYNTAX;
    }
}

attribute_nodiscard static VismutErrorType ASTParser_ParseBlock(ASTParser *restrict parser,
                                                                ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;
    VismutErrorDetails details;
    const Position pos = ASTParser_Peek(parser).position;

    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    ASTNodeIdx first_expression = ASTNodeIdx_None;
    ASTNodeIdx last_expression = ASTNodeIdx_None;

    int is_void_type_block = 1;
    int expressions_count = 0;
    while (ASTParser_Peek(parser).type != VISMUT_TOKEN_RBRACE) {
        ++expressions_count;
        if (ASTParser_Peek(parser).type == VISMUT_TOKEN_EOF) {
            break;
        }
        ASTNodeIdx current_expression;
        SAFE_RISKY_EXPRESSION(ASTParser_ParseStatement(parser, &current_expression), err);

        if (ASTNodeIdx_IsNone(first_expression)) {
            first_expression = current_expression;
        } else {
            ASTBuilder_LinkNodes(parser->builder, last_expression, current_expression);
        }

        last_expression = current_expression;

        if (unlikely(ASTParser_NodeAt(parser, current_expression)->type == VISMUT_AST_EXPRESSION &&
                     ASTParser_NodeAt(parser, current_expression)->expression.type !=
                         parser->type_ctx->type_unit)) {
            is_void_type_block = 0;
            break;
        }
    }

    SAFE_RISKY_EXPRESSION(ASTParser_AssertTokenType(parser, VISMUT_TOKEN_RBRACE), err);

    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    if (expressions_count == 0) {
        err = ASTParser_PushNode(parser, ASTNode_CreateUnit(pos), out_idx, &details);
        if (err != VISMUT_OK) {
            ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
            return err;
        }
        return VISMUT_OK;
    }

    if (expressions_count == 1) {
        const ASTNodeIdx sub_exp_node = ASTParser_NodeAt(parser, last_expression)->expression.expr;
        const ASTNodeType sub_exp_node_type = ASTParser_NodeAt(parser, sub_exp_node)->type;
        if (sub_exp_node_type != VISMUT_AST_FN_DECLARATION &&
            sub_exp_node_type != VISMUT_AST_VAR_DECLARATION) {
            *out_idx = sub_exp_node;
            return VISMUT_OK;
        }
    }

    err = ASTParser_PushNode(parser,
                             ASTNode_CreateBlock(pos, first_expression,
                                                 is_void_type_block ? parser->type_ctx->type_unit
                                                                    : parser->type_ctx->type_auto),
                             out_idx, &details);
    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
        return err;
    }
    return VISMUT_OK;
}

attribute_nodiscard static VismutErrorType
ASTParser_ParseFunctionDeclaration(ASTParser *restrict parser, ASTNodeIdx *restrict out_idx,
                                   const Position pos, StringView name) {
    VismutErrorType err;
    VismutErrorDetails details;

    if (parser->ctx.is_in_function_declaration) {
        ASTParser_SetErrorInfo(parser, VISMUT_ERR_NESTED_FN, ASTParser_Peek(parser).position,
                               (VismutErrorDetails){0});
        return VISMUT_ERR_NESTED_FN;
    }

    // skip '('
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    const VismutType *param_types[16];
    const u32 max_params_count = COUNTOF(param_types);
    StringView param_names[max_params_count];
    u32 params_count = 0;

    while (ASTParser_Peek(parser).type != VISMUT_TOKEN_RPAREN) {
        if (params_count >= max_params_count) {
            ASTParser_SetErrorInfo(parser, VISMUT_ERR_TOO_MANY_PARAMS,
                                   ASTParser_Peek(parser).position, (VismutErrorDetails){0});
            return VISMUT_ERR_TOO_MANY_PARAMS;
        }

        SAFE_RISKY_EXPRESSION(ASTParser_AssertTokenType(parser, VISMUT_TOKEN_IDENTIFIER), err);
        StringView param_name = ASTParser_Peek(parser).data.str;

        SAFE_RISKY_EXPRESSION(ASTParser_NextTokenExcept(parser, VISMUT_TOKEN_COLON), err);

        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

        const VismutType *param_type;
        SAFE_RISKY_EXPRESSION(ASTParser_ParseType(parser, &param_type), err);

        param_names[params_count] = param_name;
        param_types[params_count] = param_type;
        ++params_count;

        if (ASTParser_Peek(parser).type == VISMUT_TOKEN_COMMA) {
            SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
        }
    }
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    const VismutType *return_type = parser->type_ctx->type_unit;
    if (ASTParser_Peek(parser).type == VISMUT_TOKEN_ARROW_RIGHT) {
        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
        SAFE_RISKY_EXPRESSION(ASTParser_ParseType(parser, &return_type), err);
    }

    StringView *heap_param_names = NULL;
    if (params_count > 0) {
        heap_param_names = Arena_Array(parser->arena, StringView, params_count, &err, &details);
        if (err != VISMUT_OK) {
            ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
            return err;
        }
        __builtin_memcpy(heap_param_names, param_names, sizeof(StringView) * params_count);
    }

    const VismutType *signature;
    err = VismutTypeContext_GetFunction(parser->type_ctx, return_type, param_types, params_count,
                                        &signature, &details);
    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
        return err;
    }

    SAFE_RISKY_EXPRESSION(ASTParser_AssertTokenType(parser, VISMUT_TOKEN_LBRACE), err);

    parser->ctx.is_in_function_declaration = 1;

    ASTNodeIdx body_idx;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseBlock(parser, &body_idx), err);

    parser->ctx.is_in_function_declaration = 0;

    VismutSymbol *function_symbol;
    err = SymbolsTable_InsertSymbol(
        parser->symbols_table,
        SymbolEntry_CreateFunction(name, signature, parser->ctx.is_exported_decl, *out_idx, 0, 0),
        &function_symbol, &details);

    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
        return err;
    }

    err = ASTParser_PushNode(parser,
                             ASTNode_CreateFnDeclaration(pos, name, signature, heap_param_names,
                                                         function_symbol, body_idx),
                             out_idx, &details);
    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
        return err;
    }

    return VISMUT_OK;
}

attribute_nodiscard static VismutErrorType
ASTParser_ParseNameDeclaration(ASTParser *restrict parser, ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;
    VismutErrorDetails details;

    const Position pos = ASTParser_Peek(parser).position;

    SAFE_RISKY_EXPRESSION(ASTParser_NextTokenExcept(parser, VISMUT_TOKEN_IDENTIFIER), err);

    const StringView name = ASTParser_Peek(parser).data.str;

    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    int is_mutable = 0;
    if (ASTParser_Peek(parser).type == VISMUT_TOKEN_LPAREN) {
        return ASTParser_ParseFunctionDeclaration(parser, out_idx, pos, name);
    }

    if (ASTParser_Peek(parser).type == VISMUT_TOKEN_EXCLAMATION_MARK) {
        is_mutable = 1;
        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
    }

    const VismutType *type = parser->type_ctx->type_auto;
    if (ASTParser_Peek(parser).type == VISMUT_TOKEN_COLON) {
        // skip ':'
        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
        SAFE_RISKY_EXPRESSION(ASTParser_ParseType(parser, &type), err);
    }

    SAFE_RISKY_EXPRESSION(ASTParser_AssertTokenType(parser, VISMUT_TOKEN_EQUAL), err);
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    ASTNodeIdx init_node;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseSubExpression(parser, &init_node), err);

    SAFE_RISKY_EXPRESSION(ASTParser_AssertTokenType(parser, VISMUT_TOKEN_SEMICOLON), err);

    err = ASTParser_PushNode(parser,
                             ASTNode_CreateVarDeclaration(pos, name, type, init_node, is_mutable),
                             out_idx, &details);
    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
        return err;
    }

    return ASTParser_Next(parser);
}

attribute_nodiscard static VismutErrorType ASTParser_ParseCondition(ASTParser *restrict parser,
                                                                    ASTNodeIdx *restrict out_idx);

attribute_nodiscard static VismutErrorType ASTParser_ParseCondition(ASTParser *restrict parser,
                                                                    ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;
    VismutErrorDetails details;

    const Position pos = ASTParser_Peek(parser).position;

    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    ASTNodeIdx condition;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseSubExpression(parser, &condition), err);

    ASTNodeIdx then;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseSubExpression(parser, &then), err);

    ASTNodeIdx else_ = ASTNodeIdx_None;
    if (ASTParser_Peek(parser).type == VISMUT_TOKEN_EXCLAMATION_MARK) {
        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
        SAFE_RISKY_EXPRESSION(ASTParser_ParseSubExpression(parser, &else_), err);
    } else if (ASTParser_Peek(parser).type == VISMUT_TOKEN_CONDITION_ELSE_IF) {
        SAFE_RISKY_EXPRESSION(ASTParser_ParseCondition(parser, &else_), err);
    }

    const VismutType *node_type =
        ASTParser_NodeAt(parser, then)->expression.type == parser->type_ctx->type_unit ||
                ASTNodeIdx_IsNone(else_) ||
                ASTParser_NodeAt(parser, else_)->expression.type == parser->type_ctx->type_unit
            ? parser->type_ctx->type_unit
            : parser->type_ctx->type_auto;

    err = ASTParser_PushNode(
        parser, ASTNode_CreateCondition(pos, node_type, condition, then, else_), out_idx, &details);
    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
        return err;
    }

    return VISMUT_OK;
}

attribute_const static VismutTypeKind int_suffix_to_vismut_type_kind(const VismutIntSuffix suffix) {
    switch (suffix) {
    case VISMUT_INT_SUFFIX_I1:
        return VISMUT_TYPE_KIND_I1;
    case VISMUT_INT_SUFFIX_I8:
        return VISMUT_TYPE_KIND_I8;
    case VISMUT_INT_SUFFIX_I16:
        return VISMUT_TYPE_KIND_I16;
    case VISMUT_INT_SUFFIX_I32:
        return VISMUT_TYPE_KIND_I32;
    case VISMUT_INT_SUFFIX_I64:
        return VISMUT_TYPE_KIND_I64;
    case VISMUT_INT_SUFFIX_U8:
        return VISMUT_TYPE_KIND_U8;
    case VISMUT_INT_SUFFIX_U16:
        return VISMUT_TYPE_KIND_U16;
    case VISMUT_INT_SUFFIX_U32:
        return VISMUT_TYPE_KIND_U32;
    case VISMUT_INT_SUFFIX_U64:
        return VISMUT_TYPE_KIND_U64;
    default:
        return VISMUT_TYPE_KIND_UNKNOWN;
    }
}

attribute_nodiscard static VismutErrorType ASTParser_ParseLiteral(ASTParser *restrict parser,
                                                                  ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;
    VismutErrorDetails details;

    VismutToken token = ASTParser_Peek(parser);
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    VismutSimpleValue value = {0};
    const VismutType *type = NULL;
    switch (token.type) {
    case VISMUT_TOKEN_INT_LITERAL: {
        const VismutIntSuffix suffix = token.data.i.suffix;
        value.u = token.data.i.value;
        switch (token.data.i.suffix) {
        case VISMUT_INT_SUFFIX_I1:
            type = parser->type_ctx->type_i1;
            break;
        case VISMUT_INT_SUFFIX_I8:
            type = parser->type_ctx->type_i8;
            break;
        case VISMUT_INT_SUFFIX_I16:
            type = parser->type_ctx->type_i16;
            break;
        case VISMUT_INT_SUFFIX_I32:
            type = parser->type_ctx->type_i32;
            break;
        case VISMUT_INT_SUFFIX_I64:
            type = parser->type_ctx->type_i64;
            break;
        case VISMUT_INT_SUFFIX_U8:
            type = parser->type_ctx->type_u8;
            break;
        case VISMUT_INT_SUFFIX_U16:
            type = parser->type_ctx->type_u16;
            break;
        case VISMUT_INT_SUFFIX_U32:
            type = parser->type_ctx->type_u32;
            break;
        case VISMUT_INT_SUFFIX_U64:
            type = parser->type_ctx->type_u64;
            break;
        case VISMUT_INT_SUFFIX_NONE:
            type = parser->type_ctx->type_int;
            break;
        default:
            assert(0 && "Unreachable!");
            return VISMUT_ERR_UNREACHABLE;
        }
        if (int_suffix_to_vismut_type_kind(suffix) != VISMUT_TYPE_KIND_UNKNOWN &&
            IsNumberOverflowed(value, type->kind)) {
            ASTParser_SetErrorInfo(parser, VISMUT_ERR_NUM_OVERFLOW, ASTParser_Peek(parser).position,
                                   (VismutErrorDetails){0});
            return VISMUT_ERR_NUM_OVERFLOW;
        }
    } break;
    case VISMUT_TOKEN_FLOAT_LITERAL:
        value.f = token.data.f.value;
        switch (token.data.f.suffix) {
        case VISMUT_FLOAT_SUFFIX_F32:
            type = parser->type_ctx->type_f32;
            break;
        case VISMUT_FLOAT_SUFFIX_F64:
            type = parser->type_ctx->type_f64;
            break;
        case VISMUT_FLOAT_SUFFIX_NONE:
            type = parser->type_ctx->type_float;
            break;
        default:
            assert(0 && "Unreachable!");
            return VISMUT_ERR_UNREACHABLE;
        }
        break;
    case VISMUT_TOKEN_STR_LITERAL:
        value.str = token.data.str;
        type = parser->type_ctx->type_str;
        break;
    default:
        assert(0 && "Unreachable!");
        return VISMUT_ERR_UNREACHABLE;
    }

    err = ASTParser_PushNode(parser, ASTNode_CreateLiteral(token.position, type, value), out_idx,
                             &details);
    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
        return err;
    }

    return VISMUT_OK;
}

attribute_nodiscard static VismutErrorType ASTParser_ParseBinary(ASTParser *restrict parser,
                                                                 ASTNodeIdx *restrict out_idx);

attribute_nodiscard static VismutErrorType ASTParser_ParseTuple(ASTParser *restrict parser,
                                                                ASTNodeIdx *restrict out_idx,
                                                                const ASTNodeIdx first_field,
                                                                const Position pos) {
    VismutErrorType err;
    VismutErrorDetails details;

    ASTNodeIdx fields[16] = {[0] = first_field};
    const u32 max_fields_count = COUNTOF(fields);
    u32 fields_count = 1;

    while (ASTParser_Peek(parser).type != VISMUT_TOKEN_RPAREN) {
        if (fields_count >= max_fields_count) {
            return VISMUT_ERR_TOO_MANY_FIELDS;
        }

        ASTNodeIdx field;
        SAFE_RISKY_EXPRESSION(ASTParser_ParseBinary(parser, &field), err);

        fields[fields_count++] = field;

        if (ASTParser_Peek(parser).type == VISMUT_TOKEN_COMMA) {
            SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
        }
    }
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    ASTNodeIdx *heap_fields = Arena_Array(parser->arena, ASTNodeIdx, fields_count, &err, &details);
    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
        return err;
    }

    __builtin_memcpy(heap_fields, fields, fields_count * sizeof(ASTNodeIdx));

    err = ASTParser_PushNode(
        parser, ASTNode_CreateTuple(pos, heap_fields, fields_count, parser->type_ctx->type_auto),
        out_idx, &details);
    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
    }
    return VISMUT_OK;
}

attribute_nodiscard static VismutErrorType
ASTParser_ParseParenthesizedExpression(ASTParser *restrict parser, ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;
    VismutErrorDetails details;

    const Position pos = ASTParser_Peek(parser).position;

    // skip '('
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
    if (ASTParser_Peek(parser).type == VISMUT_TOKEN_COMMA) {
        SAFE_RISKY_EXPRESSION(ASTParser_NextTokenExcept(parser, VISMUT_TOKEN_RPAREN), err);
        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
        err = ASTParser_PushNode(parser, ASTNode_CreateUnit(pos), out_idx, &details);
        if (err != VISMUT_OK) {
            ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
            return err;
        }
        return VISMUT_OK;
    }

    // parse (...)
    ASTNodeIdx binary_idx;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseBinary(parser, &binary_idx), err);
    if (ASTParser_Peek(parser).type == VISMUT_TOKEN_COMMA) {
        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
        return ASTParser_ParseTuple(parser, out_idx, binary_idx, pos);
    }

    *out_idx = binary_idx;
    SAFE_RISKY_EXPRESSION(ASTParser_AssertTokenType(parser, VISMUT_TOKEN_RPAREN), err);

    return ASTParser_Next(parser);
}

attribute_nodiscard static VismutErrorType
ASTParser_ParseCallArguments(ASTParser *restrict parser,
                             ASTNodeIdx *restrict *restrict out_arguments,
                             u32 *restrict out_arguments_count) {
    VismutErrorType err;
    VismutErrorDetails details;

    ASTNodeIdx arguments[16];
    const u32 max_arguments_count = COUNTOF(arguments);
    u32 arguments_count = 0;

    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    while (ASTParser_Peek(parser).type != VISMUT_TOKEN_RPAREN) {
        ASTNodeIdx argument_idx;
        SAFE_RISKY_EXPRESSION(ASTParser_ParseBinary(parser, &argument_idx), err);

        if (arguments_count >= max_arguments_count) {
            return VISMUT_ERR_TOO_MANY_ARGS;
        }

        arguments[arguments_count++] = argument_idx;

        if (ASTParser_Peek(parser).type == VISMUT_TOKEN_COMMA) {
            SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
        }
    }

    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    *out_arguments_count = arguments_count;
    *out_arguments = NULL;

    if (arguments_count > 0) {
        *out_arguments = Arena_Array(parser->arena, ASTNodeIdx, arguments_count, &err, &details);

        if (unlikely(err != VISMUT_OK)) {
            ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
            return err;
        }

        __builtin_memcpy(*out_arguments, arguments, sizeof(ASTNodeIdx) * arguments_count);
    }

    return VISMUT_OK;
}

attribute_nodiscard static VismutErrorType ASTParser_ParseIdentifier(ASTParser *restrict parser,
                                                                     ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;
    VismutErrorDetails details;

    StringView identifier = ASTParser_Peek(parser).data.str;

    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    if (ASTParser_Peek(parser).type == VISMUT_TOKEN_EQUAL) {
    }

    err = ASTParser_PushNode(parser,
                             ASTNode_CreateIdentifier(ASTParser_Peek(parser).position,
                                                      parser->type_ctx->type_auto, identifier),
                             out_idx, &details);
    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
        return err;
    }

    return VISMUT_OK;
}

attribute_nodiscard static VismutErrorType ASTParser_ParseTypeCast(ASTParser *restrict parser,
                                                                   ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;
    VismutErrorDetails details;

    const Position pos = ASTParser_Peek(parser).position;

    const VismutType *to_type =
        VismutTypeTokenToType(ASTParser_Peek(parser).type, parser->type_ctx);

    SAFE_RISKY_EXPRESSION(ASTParser_NextTokenExcept(parser, VISMUT_TOKEN_LPAREN), err);

    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    ASTNodeIdx binary;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseBinary(parser, &binary), err);

    SAFE_RISKY_EXPRESSION(ASTParser_AssertTokenType(parser, VISMUT_TOKEN_RPAREN), err);

    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    err = ASTParser_PushNode(
        parser, ASTNode_CreateTypeCast(pos, parser->type_ctx->type_auto, to_type, binary), out_idx,
        &details);
    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
        return err;
    }

    return VISMUT_OK;
}

attribute_nodiscard static VismutErrorType ASTParser_ParsePrimary(ASTParser *restrict parser,
                                                                  ASTNodeIdx *restrict out_idx) {
    switch (ASTParser_Peek(parser).type) {
    case VISMUT_TOKEN_I1_TYPE:
    case VISMUT_TOKEN_I8_TYPE:
    case VISMUT_TOKEN_I16_TYPE:
    case VISMUT_TOKEN_I32_TYPE:
    case VISMUT_TOKEN_I64_TYPE:
    case VISMUT_TOKEN_U8_TYPE:
    case VISMUT_TOKEN_U16_TYPE:
    case VISMUT_TOKEN_U32_TYPE:
    case VISMUT_TOKEN_U64_TYPE:
    case VISMUT_TOKEN_F32_TYPE:
    case VISMUT_TOKEN_F64_TYPE:
        return ASTParser_ParseTypeCast(parser, out_idx);
    case VISMUT_TOKEN_INT_LITERAL:
    case VISMUT_TOKEN_FLOAT_LITERAL:
    case VISMUT_TOKEN_STR_LITERAL:
        return ASTParser_ParseLiteral(parser, out_idx);
    case VISMUT_TOKEN_IDENTIFIER:
        return ASTParser_ParseIdentifier(parser, out_idx);
    case VISMUT_TOKEN_LPAREN:
        return ASTParser_ParseParenthesizedExpression(parser, out_idx);
    default:
        ASTParser_SetErrorInfo(parser, VISMUT_ERR_SYNTAX, ASTParser_Peek(parser).position,
                               (VismutErrorDetails){0});
        return VISMUT_ERR_SYNTAX;
    }
}

attribute_nodiscard static VismutErrorType ASTParser_ParseDot(ASTParser *restrict parser,
                                                              const ASTNodeIdx object_idx,
                                                              ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;
    VismutErrorDetails details;

    const Position pos = ASTParser_Peek(parser).position;
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    SAFE_RISKY_EXPRESSION(ASTParser_AssertTokenType(parser, VISMUT_TOKEN_IDENTIFIER), err);
    StringView member_name = ASTParser_Peek(parser).data.str;
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    err = ASTParser_PushNode(
        parser, ASTNode_CreateDot(pos, parser->type_ctx->type_auto, object_idx, member_name),
        out_idx, &details);

    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, pos, details);
        return err;
    }

    return VISMUT_OK;
}

attribute_nodiscard static VismutErrorType ASTParser_ParseCall(ASTParser *restrict parser,
                                                               const ASTNodeIdx object_idx,
                                                               ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;
    VismutErrorDetails details;

    const Position pos = ASTParser_Peek(parser).position;

    ASTNodeIdx *arguments;
    u32 arguments_count;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseCallArguments(parser, &arguments, &arguments_count), err);

    err = ASTParser_PushNode(parser,
                             ASTNode_CreateCall(pos, parser->type_ctx->type_auto, object_idx,
                                                arguments, arguments_count),
                             out_idx, &details);

    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, pos, details);
        return err;
    }

    return VISMUT_OK;
}

attribute_nodiscard static VismutErrorType ASTParser_ParsePostfix(ASTParser *restrict parser,
                                                                  ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;
    ASTNodeIdx left_idx;
    SAFE_RISKY_EXPRESSION(ASTParser_ParsePrimary(parser, &left_idx), err);

    i1 parsing = 1;
    while (parsing) {
        switch (ASTParser_Peek(parser).type) {
        case VISMUT_TOKEN_DOT:
            SAFE_RISKY_EXPRESSION(ASTParser_ParseDot(parser, left_idx, &left_idx), err);
            break;
        case VISMUT_TOKEN_LPAREN:
            SAFE_RISKY_EXPRESSION(ASTParser_ParseCall(parser, left_idx, &left_idx), err);
            break;
        default:
            parsing = 0;
            break;
        }
    }

    *out_idx = left_idx;
    return VISMUT_OK;
}

int is_l_value(const ASTParser *restrict parser, const ASTNodeIdx idx) {
    switch (ASTParser_NodeAt((ASTParser *)parser, idx)->type) {
    case VISMUT_AST_IDENTIFIER:
        return 1;
    default:
        return 0;
    }
}

attribute_nodiscard static VismutErrorType ASTParser_ParseUnary(ASTParser *restrict parser,
                                                                ASTNodeIdx *restrict out_idx);

attribute_nodiscard static VismutErrorType ASTParser_ParseUnary(ASTParser *restrict parser,
                                                                ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;
    VismutErrorDetails details;

    const VismutTokenType token_type = ASTParser_Peek(parser).type;
    const Position token_pos = ASTParser_Peek(parser).position;
    const ASTUnaryNodeType unary_op = GetUnaryType(token_type);

    if (unary_op == VISMUT_AST_UNARY_UNKNOWN) {
        return ASTParser_ParsePostfix(parser, out_idx);
    }

    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    ASTNodeIdx right;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseUnary(parser, &right), err);

    err = ASTParser_PushNode(
        parser, ASTNode_CreateUnary(token_pos, parser->type_ctx->type_auto, right, unary_op),
        out_idx, &details);
    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
        return err;
    }

    return VISMUT_OK;
}

static int IsRightAssocOperator(const ASTBinaryNodeType type) {
    return type == VISMUT_AST_BINARY_POW;
}

attribute_nodiscard static VismutErrorType
ASTParser_ParseBinaryWithPrecedence(ASTParser *restrict parser, ASTNodeIdx *restrict out_idx,
                                    const OperatorPrecedence min_precedence);

attribute_nodiscard static VismutErrorType
ASTParser_ParseBinaryWithPrecedence(ASTParser *restrict parser, ASTNodeIdx *restrict out_idx,
                                    const OperatorPrecedence min_precedence) {
    VismutErrorType err;
    VismutErrorDetails details;

    ASTNodeIdx left = ASTNodeIdx_None;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseUnary(parser, &left), err);

    while (1) {
        const VismutTokenType token_type = ASTParser_Peek(parser).type;
        const Position token_pos = ASTParser_Peek(parser).position;
        const ASTBinaryNodeType binary_op = GetBinaryType(token_type);
        if (binary_op == VISMUT_AST_BINARY_UNKNOWN) {
            break;
        }

        const OperatorPrecedence precedence = GetPrecedence(binary_op);
        if (precedence < min_precedence) {
            break;
        }

        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

        const int is_right_assoc = IsRightAssocOperator(binary_op);

        ASTNodeIdx right = ASTNodeIdx_None;
        SAFE_RISKY_EXPRESSION(
            ASTParser_ParseBinaryWithPrecedence(parser, &right, precedence + (1 - is_right_assoc)),
            err);

        err = ASTParser_PushNode(
            parser,
            ASTNode_CreateBinary(token_pos, parser->type_ctx->type_auto, left, right, binary_op),
            &left, &details);
        if (err != VISMUT_OK) {
            ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
            return err;
        }
    }

    *out_idx = left;
    return VISMUT_OK;
}

attribute_nodiscard static VismutErrorType ASTParser_ParseReturn(ASTParser *restrict parser,
                                                                 ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;
    VismutErrorDetails details;

    if (!parser->ctx.is_in_function_declaration) {
        ASTParser_SetErrorInfo(parser, VISMUT_ERR_RETURN_OUTSIDE_FN,
                               ASTParser_Peek(parser).position, (VismutErrorDetails){0});
        return VISMUT_ERR_RETURN_OUTSIDE_FN;
    }

    const Position pos = ASTParser_Peek(parser).position;

    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    ASTNodeIdx expression_idx;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseSubExpression(parser, &expression_idx), err);

    err = ASTParser_PushNode(parser, ASTNode_CreateReturn(pos, expression_idx), out_idx, &details);
    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
        return err;
    }

    return VISMUT_OK;
}

attribute_nodiscard static VismutErrorType ASTParser_ParseLoop(ASTParser *restrict parser,
                                                               ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;
    VismutErrorDetails details;

    const Position pos = ASTParser_Peek(parser).position;

    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    ASTNodeIdx condition;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseSubExpression(parser, &condition), err);

    ASTNodeIdx body;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseSubExpression(parser, &body), err);

    err = ASTParser_PushNode(parser, ASTNode_CreateLoop(pos, condition, body), out_idx, &details);
    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
        return err;
    }

    return VISMUT_OK;
}

attribute_nodiscard static VismutErrorType
ASTParser_ParseSubExpression(ASTParser *restrict parser, ASTNodeIdx *restrict out_idx) {
    switch (ASTParser_Peek(parser).type) {
    case VISMUT_TOKEN_CONDITION_STATEMENT:
        return ASTParser_ParseCondition(parser, out_idx);
    case VISMUT_TOKEN_LBRACE:
        return ASTParser_ParseBlock(parser, out_idx);
    case VISMUT_TOKEN_CIRCUMFLEX:
        return ASTParser_ParseReturn(parser, out_idx);
    default:
        return ASTParser_ParseBinary(parser, out_idx);
    }
}

static ASTAssignNodeType vismut_token_to_assign_node_type(const VismutTokenType token) {
    switch (token) {
    case VISMUT_TOKEN_EQUAL:
        return VISMUT_AST_ASSIGN_EQUAL;
    default:
        return VISMUT_AST_ASSIGN_UNKNOWN;
    }
}

attribute_nodiscard static VismutErrorType
ASTParser_ParseNoWrappedExpression(ASTParser *restrict parser, ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;
    VismutErrorDetails details;

    ASTNodeIdx left_idx;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseSubExpression(parser, &left_idx), err);

    const ASTAssignNodeType assign_type =
        vismut_token_to_assign_node_type(ASTParser_Peek(parser).type);
    if (assign_type == VISMUT_AST_ASSIGN_UNKNOWN) {
        *out_idx = left_idx;
        return VISMUT_OK;
    }
    if (!is_l_value(parser, left_idx)) {
        ASTParser_SetErrorInfo(parser, VISMUT_ERR_ASSIGN_RVALUE, ASTParser_Peek(parser).position,
                               (VismutErrorDetails){0});
        return VISMUT_ERR_ASSIGN_RVALUE;
    }

    const Position pos = ASTParser_Peek(parser).position;
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    ASTNodeIdx right_idx;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseSubExpression(parser, &right_idx), err);

    err = ASTParser_PushNode(
        parser, ASTNode_CreateAssignment(pos, left_idx, right_idx, assign_type), out_idx, &details);
    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
        return err;
    }

    return VISMUT_OK;
}

attribute_nodiscard static VismutErrorType ASTParser_ParseExpression(ASTParser *restrict parser,
                                                                     ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;
    VismutErrorDetails details;

    ASTNodeIdx node_idx;
    int is_expression_void_type;

    switch (ASTParser_Peek(parser).type) {
    case VISMUT_TOKEN_WHILE_STATEMENT:
        SAFE_RISKY_EXPRESSION(ASTParser_ParseLoop(parser, &node_idx), err);
        is_expression_void_type = 1;
        break;
    default:
        SAFE_RISKY_EXPRESSION(ASTParser_ParseNoWrappedExpression(parser, &node_idx), err);
        is_expression_void_type = 0;
        break;
    }

    if (ASTParser_Peek(parser).type == VISMUT_TOKEN_SEMICOLON) {
        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
        is_expression_void_type = 1;
    }

    err = ASTParser_PushNode(parser,
                             ASTNode_CreateExpression(ASTParser_NodeAt(parser, node_idx)->pos,
                                                      is_expression_void_type
                                                          ? parser->type_ctx->type_unit
                                                          : parser->type_ctx->type_auto,
                                                      node_idx),
                             out_idx, &details);
    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
        return err;
    }

    return VISMUT_OK;
}

attribute_nodiscard static VismutErrorType ASTParser_ParseBinary(ASTParser *restrict parser,
                                                                 ASTNodeIdx *restrict out_idx) {
    return ASTParser_ParseBinaryWithPrecedence(parser, out_idx, PRECEDENCE_MINIMAL);
}

attribute_nodiscard static VismutErrorType ASTParser_ParseDeclaration(ASTParser *restrict parser,
                                                                      ASTNodeIdx *restrict out_idx);

attribute_nodiscard static VismutErrorType ASTParser_ParseExport(ASTParser *restrict parser,
                                                                 ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    const i1 old_is_export_decl = parser->ctx.is_exported_decl;
    parser->ctx.is_exported_decl = 1;

    switch (ASTParser_Peek(parser).type) {
    case VISMUT_TOKEN_NAME_DECLARATION:
        SAFE_RISKY_EXPRESSION(ASTParser_ParseNameDeclaration(parser, out_idx), err);
        break;
    default:
        parser->ctx.is_exported_decl = old_is_export_decl;
        ASTParser_SetErrorInfo(parser, VISMUT_ERR_SYNTAX, ASTParser_Peek(parser).position,
                               (VismutErrorDetails){0});
        return VISMUT_ERR_SYNTAX;
    }

    parser->ctx.is_exported_decl = old_is_export_decl;
    return VISMUT_OK;
}

static int is_declaration_token(const VismutTokenType type) {
    return type == VISMUT_TOKEN_NAME_DECLARATION || type == VISMUT_TOKEN_EXPORT_STATEMENT ||
           type == VISMUT_TOKEN_NAMESPACE_DECLARATION ||
           type == VISMUT_TOKEN_STRUCTURE_DECLARATION || type == VISMUT_TOKEN_IMPORT_STATEMENT;
}

static StringView ASTParser_ResolveImportPath(ASTParser *restrict parser,
                                              StringView current_file_path, StringView import_path,
                                              VismutErrorType *restrict err,
                                              VismutErrorDetails *restrict details) {
    u32 last_slash_idx = 0;
    int has_slash = 0;
    for (u32 i = current_file_path.length; i > 0; i--) {
        if (current_file_path.data[i - 1] == '/' || current_file_path.data[i - 1] == '\\') {
            last_slash_idx = i;
            has_slash = 1;
            break;
        }
    }

    const u32 dir_len = has_slash ? last_slash_idx : 0;
    const StringView ext = StringView_FromCStr(".bi");
    const u32 total_len = dir_len + import_path.length + ext.length;

    u8 *resolved_path = Arena_Array(parser->arena, u8, total_len + 1, err, details);
    if (*err != VISMUT_OK) {
        return StringView_Empty();
    }

    u32 offset = 0;
    if (dir_len > 0) {
        __builtin_memcpy(resolved_path + offset, current_file_path.data, dir_len);
        offset += dir_len;
    }

    __builtin_memcpy(resolved_path + offset, import_path.data, import_path.length);
    offset += import_path.length;

    __builtin_memcpy(resolved_path + offset, ext.data, ext.length);
    resolved_path[total_len] = '\0';

    return (StringView){.data = resolved_path, .length = total_len};
}

attribute_nodiscard static VismutErrorType ASTParser_ParseImport(ASTParser *restrict parser,
                                                                 ASTNodeIdx *out_idx) {
    VismutErrorType err;
    VismutErrorDetails details;
    const Position pos = ASTParser_Peek(parser).position;

    SAFE_RISKY_EXPRESSION(ASTParser_NextTokenExcept(parser, VISMUT_TOKEN_STR_LITERAL), err);
    StringView raw_import_path = ASTParser_Peek(parser).data.str;

    SAFE_RISKY_EXPRESSION(ASTParser_NextTokenExcept(parser, VISMUT_TOKEN_FAT_ARROW_RIGHT), err);

    SAFE_RISKY_EXPRESSION(ASTParser_NextTokenExcept(parser, VISMUT_TOKEN_IDENTIFIER), err);
    StringView alias_name = ASTParser_Peek(parser).data.str;

    SAFE_RISKY_EXPRESSION(ASTParser_NextTokenExcept(parser, VISMUT_TOKEN_SEMICOLON), err);
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    VismutModuleEntry *current_mod =
        VismutModulesTable_ModuleAt(parser->modules_table, parser->current_module_idx);

    StringView resolved_module_path = ASTParser_ResolveImportPath(parser, current_mod->file_path,
                                                                  raw_import_path, &err, &details);
    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, pos, details);
        return err;
    }

    VismutModuleIdx imported_idx;
    err = VismutModulesTable_FindOrLoad(parser->modules_table, alias_name, resolved_module_path,
                                        parser->type_ctx, parser->error_info, &imported_idx,
                                        &details);
    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, pos, details);
        return err;
    }

    SAFE_RISKY_EXPRESSION(
        VismutModuleEntry_AddImport(parser->modules_table, current_mod, imported_idx, &details),
        err);

    const VismutType *type;
    err = VismutTypeContext_GetModuleRef(parser->type_ctx, imported_idx, &type, &details);
    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, pos, details);
        return err;
    }

    VismutSymbol *module_symbol;
    err = SymbolsTable_InsertSymbol(parser->symbols_table,
                                    SymbolEntry_CreateModuleRef(alias_name, type, imported_idx),
                                    &module_symbol, &details);
    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, pos, details);
        return err;
    }

    err = ASTParser_PushNode(
        parser, ASTNode_CreateImport(pos, raw_import_path, alias_name, imported_idx, module_symbol),
        out_idx, &details);
    if (unlikely(err != VISMUT_OK)) {
        ASTParser_SetErrorInfo(parser, err, pos, details);
        return err;
    }

    return VISMUT_OK;
}

attribute_nodiscard static VismutErrorType
ASTParser_ParseDeclaration(ASTParser *restrict parser, ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;
    VismutErrorDetails details;
    ASTNodeIdx idx;

    const Position pos = ASTParser_Peek(parser).position;
    int is_export = 0;

    switch (ASTParser_Peek(parser).type) {
    case VISMUT_TOKEN_NAME_DECLARATION:
        SAFE_RISKY_EXPRESSION(ASTParser_ParseNameDeclaration(parser, &idx), err);
        break;
    case VISMUT_TOKEN_EXPORT_STATEMENT:
        is_export = 1;
        SAFE_RISKY_EXPRESSION(ASTParser_ParseExport(parser, &idx), err);
        break;
    case VISMUT_TOKEN_IMPORT_STATEMENT:
        SAFE_RISKY_EXPRESSION(ASTParser_ParseImport(parser, &idx), err);
        break;
    default:
        assert(0 && "Unreachable!");
        return VISMUT_ERR_UNREACHABLE;
    }

    err = ASTParser_PushNode(parser, ASTNode_CreateDeclaration(pos, idx, is_export), out_idx,
                             &details);
    if (err != VISMUT_OK) {
        ASTParser_SetErrorInfo(parser, err, ASTParser_Peek(parser).position, details);
        return err;
    }

    return VISMUT_OK;
}

attribute_nodiscard static VismutErrorType ASTParser_ParseStatement(ASTParser *restrict parser,
                                                                    ASTNodeIdx *restrict out_idx) {
    if (is_declaration_token(ASTParser_Peek(parser).type)) {
        return ASTParser_ParseDeclaration(parser, out_idx);
    }

    return ASTParser_ParseExpression(parser, out_idx);
}
