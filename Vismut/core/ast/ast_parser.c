#include "ast_parser.h"
#include "ast.h"
#include "ast_builder.h"
#include "ast_utils.h"
#include "value.h"
#include <assert.h>
#include <stdatomic.h>
#include <stdio.h>

ASTParser ASTParser_Create(ASTBuilder *restrict builder, StringPool *restrict string_pool,
                           VismutTypeContext *restrict type_ctx) {
    return (ASTParser){
        .module_node = ASTNodeIdx_None,
        .builder = builder,
        .current_token = {0},
        .string_pool = string_pool,
        .type_ctx = type_ctx,
    };
}

attribute_const attribute_nodiscard static inline VismutErrorType
ASTParser_PushNode(const ASTParser *parser, const ASTNode node, ASTNodeIdx *out_idx) {
    *out_idx = parser->builder->nodes_length;
    return ASTBuilder_PushNode(parser->builder, &node);
}

attribute_pure static VismutToken ASTParser_Peek(const ASTParser *parser) {
    return parser->current_token;
}

attribute_nodiscard static VismutErrorType ASTParser_Next(ASTParser *parser) {
    return ASTBuilder_NextToken(parser->builder, &parser->current_token);
}

attribute_nodiscard static VismutErrorType ASTParser_NextTokenExcept(ASTParser *restrict parser,
                                                                     const VismutTokenType type) {
    VismutErrorType err;
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
    if (ASTParser_Peek(parser).type != type) {
        return VISMUT_ERROR_UNEXPECTED_TOKEN;
    }
    return VISMUT_ERROR_OK;
}

attribute_nodiscard static ASTNode *ASTParser_NodeAt(ASTParser *parser, const ASTNodeIdx idx) {
    return &parser->builder->nodes[idx];
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

    StringNode *module_name;
    StringPool_Intern(parser->string_pool,
                      (StringView){.data = parser->builder->tokenizer->source_filename,
                                   .length = __builtin_strlen(
                                       (const char *)parser->builder->tokenizer->source_filename)},
                      &module_name);

    SAFE_RISKY_EXPRESSION(
        ASTParser_PushNode(parser,
                           ASTNode_CreateModule(module_name, first_expression, ASTNodeIdx_None),
                           &parser->module_node),
        err);

    return VISMUT_ERROR_OK;
}

attribute_nodiscard static VismutErrorType ASTParser_ParseExpression(ASTParser *restrict parser,
                                                                     ASTNodeIdx *restrict out_idx);

attribute_nodiscard static VismutErrorType
ASTParser_ParseType(ASTParser *restrict parser, const VismutType *restrict *restrict out_type) {
    const VismutTokenType token_type = ASTParser_Peek(parser).type;

    VismutErrorType err;
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    switch (token_type) {
    case VISMUT_TOKEN_I8_TYPE:
        *out_type = parser->type_ctx->type_i8;
        return VISMUT_ERROR_OK;
    case VISMUT_TOKEN_I16_TYPE:
        *out_type = parser->type_ctx->type_i16;
        return VISMUT_ERROR_OK;
    case VISMUT_TOKEN_I32_TYPE:
        *out_type = parser->type_ctx->type_i32;
        return VISMUT_ERROR_OK;
    case VISMUT_TOKEN_I64_TYPE:
        *out_type = parser->type_ctx->type_i64;
        return VISMUT_ERROR_OK;
    case VISMUT_TOKEN_U8_TYPE:
        *out_type = parser->type_ctx->type_u8;
        return VISMUT_ERROR_OK;
    case VISMUT_TOKEN_U16_TYPE:
        *out_type = parser->type_ctx->type_u16;
        return VISMUT_ERROR_OK;
    case VISMUT_TOKEN_U32_TYPE:
        *out_type = parser->type_ctx->type_u32;
        return VISMUT_ERROR_OK;
    case VISMUT_TOKEN_U64_TYPE:
        *out_type = parser->type_ctx->type_u64;
        return VISMUT_ERROR_OK;
    case VISMUT_TOKEN_F32_TYPE:
        *out_type = parser->type_ctx->type_f32;
        return VISMUT_ERROR_OK;
    case VISMUT_TOKEN_F64_TYPE:
        *out_type = parser->type_ctx->type_f64;
        return VISMUT_ERROR_OK;
    case VISMUT_TOKEN_STR_TYPE:
        *out_type = parser->type_ctx->type_str;
        return VISMUT_ERROR_OK;
    case VISMUT_TOKEN_LANGLE: {
        const VismutType *vector_type;
        SAFE_RISKY_EXPRESSION(ASTParser_ParseType(parser, &vector_type), err);
        if (ASTParser_Peek(parser).type != VISMUT_TOKEN_RANGLE) {
            return VISMUT_ERROR_INVALID_SYNTAX;
        }
        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

        return VismutTypeContext_GetVector(parser->type_ctx, vector_type, out_type);
    }
    case VISMUT_TOKEN_LBRACKET: {
        const VismutType *array_type;
        SAFE_RISKY_EXPRESSION(ASTParser_ParseType(parser, &array_type), err);
        if (ASTParser_Peek(parser).type != VISMUT_TOKEN_SEMICOLON) {
            return VISMUT_ERROR_INVALID_SYNTAX;
        }
        SAFE_RISKY_EXPRESSION(ASTParser_NextTokenExcept(parser, VISMUT_TOKEN_INT_LITERAL), err);
        const i64 array_length = ASTParser_Peek(parser).data.i.value;
        if (array_length <= 0) {
            return VISMUT_ERROR_INVALID_SYNTAX;
        }
        SAFE_RISKY_EXPRESSION(ASTParser_NextTokenExcept(parser, VISMUT_TOKEN_RBRACKET), err);
        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

        return VismutTypeContext_GetArray(parser->type_ctx, array_type, (u64)array_length,
                                          out_type);
    }
    case VISMUT_TOKEN_LPAREN: { // (t1, ...) -> r
        const VismutType *types[256];
        u32 types_count = 0;
        const u32 max_types_count = COUNTOF(types);
        while (ASTParser_Peek(parser).type != VISMUT_TOKEN_RPAREN) {
            if (types_count >= max_types_count) {
                return VISMUT_ERROR_TOO_MANY_FUNCTION_ARGUMENTS;
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
                return VISMUT_ERROR_OK;
            }
            return VismutTypeContext_GetTuple(parser->type_ctx, types, types_count, out_type);
        }
        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
        const VismutType *return_type;
        SAFE_RISKY_EXPRESSION(ASTParser_ParseType(parser, &return_type), err);

        return VismutTypeContext_GetFunction(parser->type_ctx, return_type, types, types_count,
                                             out_type);
    }

    default:
        return VISMUT_ERROR_INVALID_SYNTAX;
    }
}

attribute_nodiscard static VismutErrorType
ASTParser_ParseNameDeclaration(ASTParser *restrict parser, ASTNodeIdx *restrict out_idx) {
    const Position pos = ASTParser_Peek(parser).position;

    VismutErrorType err;
    SAFE_RISKY_EXPRESSION(ASTParser_NextTokenExcept(parser, VISMUT_TOKEN_IDENTIFIER), err);

    StringNode *name;
    const StringView name_view = ASTParser_Peek(parser).data.str;
    SAFE_RISKY_EXPRESSION(StringPool_Intern(parser->string_pool, name_view, &name), err);

    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    int is_mutable = 0;
    if (ASTParser_Peek(parser).type == VISMUT_TOKEN_EXCLAMATION_MARK) {
        is_mutable = 1;
        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
    }

    const VismutType *type = parser->type_ctx->type_auto;
    if (ASTParser_Peek(parser).type == VISMUT_TOKEN_SEMICOLON) {
        // skip ':'
        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
        SAFE_RISKY_EXPRESSION(ASTParser_ParseType(parser, &type), err);
    }

    if (unlikely(ASTParser_Peek(parser).type != VISMUT_TOKEN_EQUAL)) {
        return VISMUT_ERROR_INVALID_SYNTAX;
    }

    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    ASTNodeIdx init_node;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseExpression(parser, &init_node), err);

    if (ASTParser_Peek(parser).type != VISMUT_TOKEN_SEMICOLON) {
        return VISMUT_ERROR_INVALID_SYNTAX;
    }

    SAFE_RISKY_EXPRESSION(
        ASTParser_PushNode(
            parser, ASTNode_CreateVarDeclaration(pos, name, type, init_node, is_mutable), out_idx),
        err);

    return ASTParser_Next(parser);
}

attribute_nodiscard static VismutErrorType ASTParser_ParseCondition(ASTParser *restrict parser,
                                                                    ASTNodeIdx *restrict out_idx);

attribute_nodiscard static VismutErrorType ASTParser_ParseCondition(ASTParser *restrict parser,
                                                                    ASTNodeIdx *restrict out_idx) {
    const Position pos = ASTParser_Peek(parser).position;

    VismutErrorType err;
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    ASTNodeIdx condition;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseExpression(parser, &condition), err);

    ASTNodeIdx then;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseExpression(parser, &then), err);

    ASTNodeIdx else_ = ASTNodeIdx_None;
    if (ASTParser_Peek(parser).type == VISMUT_TOKEN_EXCLAMATION_MARK) {
        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
        SAFE_RISKY_EXPRESSION(ASTParser_ParseExpression(parser, &else_), err);
    } else if (ASTParser_Peek(parser).type == VISMUT_TOKEN_CONDITION_ELSE_IF) {
        SAFE_RISKY_EXPRESSION(ASTParser_ParseCondition(parser, &else_), err);
    }

    const VismutType *node_type =
        ASTParser_NodeAt(parser, then)->expression.type == parser->type_ctx->type_unit ||
                ASTNodeIdx_IsNone(else_) ||
                ASTParser_NodeAt(parser, else_)->expression.type == parser->type_ctx->type_unit
            ? parser->type_ctx->type_unit
            : parser->type_ctx->type_auto;

    return ASTParser_PushNode(
        parser, ASTNode_CreateCondition(pos, node_type, condition, then, else_), out_idx);
}

attribute_nodiscard static VismutErrorType ASTParser_ParseLiteral(ASTParser *restrict parser,
                                                                  ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;
    VismutToken token = ASTParser_Peek(parser);
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    VismutSimpleValue value;
    const VismutType *type = NULL;
    switch (token.type) {
    case VISMUT_TOKEN_INT_LITERAL:
        value.i = token.data.i.value;
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
            return VISMUT_ERROR_UNREACHABLE;
        }
        break;
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
            return VISMUT_ERROR_UNREACHABLE;
        }
        break;
    case VISMUT_TOKEN_STR_LITERAL:
        SAFE_RISKY_EXPRESSION(StringPool_Intern(parser->string_pool, token.data.str, &value.str),
                              err);
        type = parser->type_ctx->type_str;
        break;
    default:
        assert(0 && "Unreachable!");
        return VISMUT_ERROR_OK;
    }

    SAFE_RISKY_EXPRESSION(
        ASTParser_PushNode(parser, ASTNode_CreateLiteral(token.position, type, value), out_idx),
        err);

    return VISMUT_ERROR_OK;
}

attribute_nodiscard static VismutErrorType ASTParser_ParseBinary(ASTParser *restrict parser,
                                                                 ASTNodeIdx *restrict out_idx);

attribute_nodiscard static VismutErrorType ASTParser_ParseTuple(ASTParser *restrict parser,
                                                                ASTNodeIdx *restrict out_idx,
                                                                const ASTNodeIdx first_field,
                                                                const Position pos) {
    ASTNodeIdx fields[256] = {[0] = first_field};
    const u32 max_fields_count = COUNTOF(fields);
    u32 fields_count = 1;

    VismutErrorType err;
    while (ASTParser_Peek(parser).type != VISMUT_TOKEN_RPAREN) {
        if (fields_count >= max_fields_count) {
            return VISMUT_ERROR_TOO_MANY_FUNCTION_ARGUMENTS;
        }
        ASTNodeIdx field;
        SAFE_RISKY_EXPRESSION(ASTParser_ParseBinary(parser, &field), err);
        fields[fields_count++] = field;
        if (ASTParser_Peek(parser).type == VISMUT_TOKEN_COMMA) {
            SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
        }
    }
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    ASTNodeIdx *heap_fields = Arena_Array(parser->builder->arena, ASTNodeIdx, fields_count);
    __builtin_memcpy(heap_fields, fields, fields_count * sizeof(ASTNodeIdx));

    return ASTParser_PushNode(
        parser, ASTNode_CreateTuple(pos, heap_fields, fields_count, parser->type_ctx->type_auto),
        out_idx);
}

attribute_nodiscard static VismutErrorType
ASTParser_ParseParenthesizedExpression(ASTParser *restrict parser, ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;
    const Position pos = ASTParser_Peek(parser).position;
    // skip '('
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
    if (ASTParser_Peek(parser).type == VISMUT_TOKEN_COMMA) {
        SAFE_RISKY_EXPRESSION(ASTParser_NextTokenExcept(parser, VISMUT_TOKEN_RPAREN), err);
        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
        return ASTParser_PushNode(parser, ASTNode_CreateUnit(pos), out_idx);
    }

    // parse (...)
    ASTNodeIdx binary_idx;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseBinary(parser, &binary_idx), err);
    if (ASTParser_Peek(parser).type == VISMUT_TOKEN_COMMA) {
        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
        return ASTParser_ParseTuple(parser, out_idx, binary_idx, pos);
    }

    *out_idx = binary_idx;
    if (ASTParser_Peek(parser).type != VISMUT_TOKEN_RPAREN) {
        return VISMUT_ERROR_INVALID_SYNTAX;
    }

    return ASTParser_Next(parser);
}

attribute_nodiscard static VismutErrorType
ASTParser_ParseFunctionArguments(ASTParser *restrict parser,
                                 ASTNodeIdx *restrict *restrict out_arguments,
                                 u64 *restrict out_arguments_count) {
    VismutErrorType err;
    ASTNodeIdx arguments[256];
    const u64 max_arguments_count = COUNTOF(arguments);
    u64 arguments_count = 0;

    // skip '('
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
    while (ASTParser_Peek(parser).type != VISMUT_TOKEN_RPAREN) {
        ASTNodeIdx argument_idx;
        SAFE_RISKY_EXPRESSION(ASTParser_ParseBinary(parser, &argument_idx), err);
        if (arguments_count >= max_arguments_count) {
            return VISMUT_ERROR_TOO_MANY_FUNCTION_ARGUMENTS;
        }
        arguments[arguments_count++] = argument_idx;

        if (ASTParser_Peek(parser).type != VISMUT_TOKEN_COMMA &&
            ASTParser_Peek(parser).type != VISMUT_TOKEN_RPAREN) {
            return VISMUT_ERROR_INVALID_SYNTAX;
        }
        if (ASTParser_Peek(parser).type == VISMUT_TOKEN_COMMA) {
            SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
        }
    }
    // skip ')'
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    *out_arguments_count = arguments_count;
    *out_arguments = arguments_count > 0
                         ? Arena_Array(parser->builder->arena, ASTNodeIdx, arguments_count)
                         : NULL;
    if (out_arguments != NULL) {
        __builtin_memcpy(*out_arguments, arguments, sizeof(ASTNodeIdx) * arguments_count);
    }

    return VISMUT_ERROR_OK;
}

attribute_nodiscard static VismutErrorType ASTParser_ParseFunctionCall(ASTParser *restrict parser,
                                                                       ASTNodeIdx *restrict out_idx,
                                                                       const Position start_pos,
                                                                       StringNode *name) {
    VismutErrorType err;

    // Parse arguments
    ASTNodeIdx *arguments;
    u64 arguments_count;

    SAFE_RISKY_EXPRESSION(ASTParser_ParseFunctionArguments(parser, &arguments, &arguments_count),
                          err);

    return ASTParser_PushNode(
        parser, ASTNode_CreateFnCall(start_pos, name, NULL, arguments, arguments_count), out_idx);
}

attribute_nodiscard static VismutErrorType ASTParser_ParseIdentifier(ASTParser *restrict parser,
                                                                     ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;
    VismutToken name_token = ASTParser_Peek(parser);

    StringNode *identifier;
    SAFE_RISKY_EXPRESSION(StringPool_Intern(parser->string_pool, name_token.data.str, &identifier),
                          err);

    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    if (ASTParser_Peek(parser).type == VISMUT_TOKEN_LPAREN) {
        return ASTParser_ParseFunctionCall(parser, out_idx, name_token.position, identifier);
    }

    SAFE_RISKY_EXPRESSION(
        ASTParser_PushNode(
            parser,
            ASTNode_CreateIdentifier(name_token.position, parser->type_ctx->type_auto, identifier),
            out_idx),
        err);

    return VISMUT_ERROR_OK;
}

attribute_nodiscard static VismutErrorType ASTParser_ParseTypeCast(ASTParser *restrict parser,
                                                                   ASTNodeIdx *restrict out_idx) {
    const Position pos = ASTParser_Peek(parser).position;
    const VismutType *to_type =
        VismutTypeTokenToType(ASTParser_Peek(parser).type, parser->type_ctx);

    VismutErrorType err;
    SAFE_RISKY_EXPRESSION(ASTParser_NextTokenExcept(parser, VISMUT_TOKEN_LPAREN), err);
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    ASTNodeIdx binary;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseBinary(parser, &binary), err);

    if (ASTParser_Peek(parser).type != VISMUT_TOKEN_RPAREN) {
        return VISMUT_ERROR_INVALID_SYNTAX;
    }

    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    return ASTParser_PushNode(
        parser, ASTNode_CreateTypeCast(pos, parser->type_ctx->type_auto, to_type, binary), out_idx);
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
        return VISMUT_ERROR_UNEXPECTED_TOKEN;
    }
}

attribute_nodiscard static VismutErrorType ASTParser_ParseUnary(ASTParser *restrict parser,
                                                                ASTNodeIdx *restrict out_idx);

attribute_nodiscard static VismutErrorType ASTParser_ParseUnary(ASTParser *restrict parser,
                                                                ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;

    const VismutTokenType token_type = ASTParser_Peek(parser).type;
    const Position token_pos = ASTParser_Peek(parser).position;
    const ASTUnaryNodeType unary_op = GetUnaryType(token_type);

    if (unary_op == VISMUT_AST_UNARY_UNKNOWN) {
        return ASTParser_ParsePrimary(parser, out_idx);
    }

    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    ASTNodeIdx right;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseUnary(parser, &right), err);

    ASTNodeIdx node;
    SAFE_RISKY_EXPRESSION(
        ASTParser_PushNode(
            parser, ASTNode_CreateUnary(token_pos, parser->type_ctx->type_auto, right, unary_op),
            &node),
        err);

    *out_idx = node;
    return VISMUT_ERROR_OK;
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

    ASTNodeIdx left = ASTNodeIdx_None;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseUnary(parser, &left), err);

    while (1) {
        const VismutTokenType token_type = ASTParser_Peek(parser).type;
        const Position token_pos = ASTParser_Peek(parser).position;
        const OperatorPrecedence precedence = GetPrecedence(token_type);

        if (precedence < min_precedence) {
            break;
        }

        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

        const ASTBinaryNodeType binary_op = GetBinaryType(token_type);
        const int is_right_assoc = IsRightAssocOperator(binary_op);

        ASTNodeIdx right = ASTNodeIdx_None;
        SAFE_RISKY_EXPRESSION(
            ASTParser_ParseBinaryWithPrecedence(parser, &right, precedence + (1 - is_right_assoc)),
            err);

        SAFE_RISKY_EXPRESSION(
            ASTParser_PushNode(parser,
                               ASTNode_CreateBinary(token_pos, parser->type_ctx->type_auto, left,
                                                    right, binary_op),
                               &left),
            err);
    }

    *out_idx = left;
    return VISMUT_ERROR_OK;
}

attribute_nodiscard static VismutErrorType ASTParser_ParseBlock(ASTParser *restrict parser,
                                                                ASTNodeIdx *restrict out_idx) {
    const Position pos = ASTParser_Peek(parser).position;

    VismutErrorType err;
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    ASTNodeIdx first_expression = ASTNodeIdx_None;
    ASTNodeIdx last_expression = ASTNodeIdx_None;

    int is_void_type_block = 1;
    while (ASTParser_Peek(parser).type != VISMUT_TOKEN_RBRACE) {
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

        if (unlikely(ASTParser_NodeAt(parser, current_expression)->expression.type !=
                     parser->type_ctx->type_unit)) {
            is_void_type_block = 0;
            break;
        }
    }
    if (unlikely(ASTParser_Peek(parser).type != VISMUT_TOKEN_RBRACE)) {
        return VISMUT_ERROR_INVALID_SYNTAX;
    }
    SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);

    SAFE_RISKY_EXPRESSION(
        ASTParser_PushNode(parser,
                           ASTNode_CreateBlock(pos, first_expression,
                                               is_void_type_block ? parser->type_ctx->type_unit
                                                                  : parser->type_ctx->type_auto),
                           out_idx),
        err);

    return VISMUT_ERROR_OK;
}

attribute_nodiscard static VismutErrorType ASTParser_ParseExpression(ASTParser *restrict parser,
                                                                     ASTNodeIdx *restrict out_idx) {
    VismutErrorType err = VISMUT_ERROR_OK;
    ASTNodeIdx node_idx = ASTNodeIdx_None;

    int is_expression_void_type = 0;

    switch (ASTParser_Peek(parser).type) {
    case VISMUT_TOKEN_NAME_DECLARATION:
        err = ASTParser_ParseNameDeclaration(parser, &node_idx);
        is_expression_void_type = 1;
        break;
    case VISMUT_TOKEN_CONDITION_STATEMENT:
        err = ASTParser_ParseCondition(parser, &node_idx);
        if (ASTParser_NodeAt(parser, node_idx)->condition.type == parser->type_ctx->type_unit) {
            is_expression_void_type = 1;
        }
        break;
    case VISMUT_TOKEN_LBRACE:
        err = ASTParser_ParseBlock(parser, &node_idx);
        if (ASTParser_NodeAt(parser, node_idx)->block.type == parser->type_ctx->type_unit) {
            is_expression_void_type = 1;
        }
        break;
    default:
        err = ASTParser_ParseBinary(parser, &node_idx);
        break;
    }

    if (unlikely(err != VISMUT_ERROR_OK)) {
        return err;
    }

    SAFE_RISKY_EXPRESSION(
        ASTParser_PushNode(parser,
                           ASTNode_CreateExpression(ASTParser_NodeAt(parser, node_idx)->pos,
                                                    is_expression_void_type
                                                        ? parser->type_ctx->type_unit
                                                        : parser->type_ctx->type_auto,
                                                    node_idx),
                           out_idx),
        err);

    return VISMUT_ERROR_OK;
}

attribute_nodiscard static VismutErrorType ASTParser_ParseBinary(ASTParser *restrict parser,
                                                                 ASTNodeIdx *restrict out_idx) {
    return ASTParser_ParseBinaryWithPrecedence(parser, out_idx, PRECEDENCE_MINIMAL);
}

attribute_nodiscard static VismutErrorType ASTParser_ParseStatement(ASTParser *restrict parser,
                                                                    ASTNodeIdx *restrict out_idx) {
    VismutErrorType err;
    ASTNodeIdx idx;
    SAFE_RISKY_EXPRESSION(ASTParser_ParseExpression(parser, &idx), err);

    int is_void_type_expression = 0;
    if (ASTParser_Peek(parser).type == VISMUT_TOKEN_SEMICOLON) {
        SAFE_RISKY_EXPRESSION(ASTParser_Next(parser), err);
        is_void_type_expression = 1;
    }
    if (ASTParser_NodeAt(parser, idx)->expression.type == parser->type_ctx->type_unit) {
        is_void_type_expression = 1;
    }

    ASTParser_NodeAt(parser, idx)->expression.type =
        is_void_type_expression ? parser->type_ctx->type_unit : parser->type_ctx->type_auto;

    *out_idx = idx;

    return VISMUT_ERROR_OK;
}
