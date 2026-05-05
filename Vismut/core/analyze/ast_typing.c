#include "ast_typing.h"
#include "../utils/types.h"
#include <assert.h>

typedef struct {
    VismutTypeKind left_type;
    VismutTypeKind right_type;
    VismutTypeKind result_type;
} op_rule;

typedef struct {
    ASTBinaryNodeType op;
    const op_rule *rules;
    int rules_count;
} op_rules;

#define BEGIN_BINS(var_name) static const op_rules var_name[] = {

#define END_BINS                                                                                   \
    }                                                                                              \
    ;

#define BIN(op_name, ...)                                                                          \
    [VISMUT_AST_BINARY_##                                                                          \
        op_name] = {.op = VISMUT_AST_BINARY_##op_name,                                             \
                    .rules = (op_rule[]){__VA_ARGS__},                                             \
                    .rules_count = sizeof((op_rule[]){__VA_ARGS__}) / sizeof(op_rule)},

#define R(r, l, res) {VISMUT_TYPE_KIND_##r, VISMUT_TYPE_KIND_##l, VISMUT_TYPE_KIND_##res}

BEGIN_BINS(binary_op_rules)
// Арифметика
BIN(ADD, R(I8, I8, I8), R(I16, I16, I16), R(I32, I32, I32), R(I64, I64, I64), R(U8, U8, U8),
    R(U16, U16, U16), R(U32, U32, U32), R(U64, U64, U64), R(F32, F32, F32), R(F64, F64, F64))
BIN(SUB, R(I8, I8, I8), R(I16, I16, I16), R(I32, I32, I32), R(I64, I64, I64), R(U8, U8, U8),
    R(U16, U16, U16), R(U32, U32, U32), R(U64, U64, U64), R(F32, F32, F32), R(F64, F64, F64))
BIN(MUL, R(I8, I8, I8), R(I16, I16, I16), R(I32, I32, I32), R(I64, I64, I64), R(U8, U8, U8),
    R(U16, U16, U16), R(U32, U32, U32), R(U64, U64, U64), R(F32, F32, F32), R(F64, F64, F64))
BIN(DIV, R(I8, I8, I8), R(I16, I16, I16), R(I32, I32, I32), R(I64, I64, I64), R(U8, U8, U8),
    R(U16, U16, U16), R(U32, U32, U32), R(U64, U64, U64), R(F32, F32, F32), R(F64, F64, F64))
BIN(REM_DIV, R(I8, I8, I8), R(I16, I16, I16), R(I32, I32, I32), R(I64, I64, I64), R(U8, U8, U8),
    R(U16, U16, U16), R(U32, U32, U32), R(U64, U64, U64))
BIN(POW, R(I32, I32, I32), R(I64, I64, I64), R(F32, F32, F32), R(F64, F64, F64), R(F32, I32, F32),
    R(F64, I32, F64))

// Битовые операции и сдвиги
BIN(SHIFT_LEFT, R(I8, I8, I8), R(I16, I16, I16), R(I32, I32, I32), R(I64, I64, I64), R(U8, U8, U8),
    R(U16, U16, U16), R(U32, U32, U32), R(U64, U64, U64))
BIN(SHIFT_RIGHT, R(I8, I8, I8), R(I16, I16, I16), R(I32, I32, I32), R(I64, I64, I64), R(U8, U8, U8),
    R(U16, U16, U16), R(U32, U32, U32), R(U64, U64, U64))
BIN(BITWISE_OR, R(I8, I8, I8), R(I16, I16, I16), R(I32, I32, I32), R(I64, I64, I64), R(U8, U8, U8),
    R(U16, U16, U16), R(U32, U32, U32), R(U64, U64, U64))
BIN(BITWISE_AND, R(I8, I8, I8), R(I16, I16, I16), R(I32, I32, I32), R(I64, I64, I64), R(U8, U8, U8),
    R(U16, U16, U16), R(U32, U32, U32), R(U64, U64, U64))
BIN(BITWISE_XOR, R(I8, I8, I8), R(I16, I16, I16), R(I32, I32, I32), R(I64, I64, I64), R(U8, U8, U8),
    R(U16, U16, U16), R(U32, U32, U32), R(U64, U64, U64))

// Логика
BIN(LOGICAL_OR, R(I1, I1, I1))
BIN(LOGICAL_AND, R(I1, I1, I1))

// Сравнения (результат всегда I1)
BIN(EQUALS, R(I1, I1, I1), R(I8, I8, I1), R(I16, I16, I1), R(I32, I32, I1), R(I64, I64, I1),
    R(U8, U8, I1), R(U16, U16, I1), R(U32, U32, I1), R(U64, U64, I1), R(F32, F32, I1),
    R(F64, F64, I1))
BIN(NOT_EQUALS, R(I1, I1, I1), R(I8, I8, I1), R(I16, I16, I1), R(I32, I32, I1), R(I64, I64, I1),
    R(U8, U8, I1), R(U16, U16, I1), R(U32, U32, I1), R(U64, U64, I1), R(F32, F32, I1),
    R(F64, F64, I1))
BIN(LESS_THAN, R(I8, I8, I1), R(I16, I16, I1), R(I32, I32, I1), R(I64, I64, I1), R(U8, U8, I1),
    R(U16, U16, I1), R(U32, U32, I1), R(U64, U64, I1), R(F32, F32, I1), R(F64, F64, I1))
BIN(LESS_THAN_OR_EQUAL, R(I8, I8, I1), R(I16, I16, I1), R(I32, I32, I1), R(I64, I64, I1),
    R(U8, U8, I1), R(U16, U16, I1), R(U32, U32, I1), R(U64, U64, I1), R(F32, F32, I1),
    R(F64, F64, I1))
BIN(GREATER_THAN, R(I8, I8, I1), R(I16, I16, I1), R(I32, I32, I1), R(I64, I64, I1), R(U8, U8, I1),
    R(U16, U16, I1), R(U32, U32, I1), R(U64, U64, I1), R(F32, F32, I1), R(F64, F64, I1))
BIN(GREATER_THAN_OR_EQUAL, R(I8, I8, I1), R(I16, I16, I1), R(I32, I32, I1), R(I64, I64, I1),
    R(U8, U8, I1), R(U16, U16, I1), R(U32, U32, I1), R(U64, U64, I1), R(F32, F32, I1),
    R(F64, F64, I1))
END_BINS

attribute_noinline attribute_const static VismutTypeKind
GetBinaryOpResult_ForNumeric(const VismutTypeKind left, const VismutTypeKind right,
                             const ASTBinaryNodeType operation) {
    if (operation < 0 || operation >= COUNTOF(binary_op_rules)) {
        return VISMUT_TYPE_KIND_UNKNOWN;
    }
    const op_rules *rules = &binary_op_rules[operation];
    if (rules->rules_count == 0 || rules->op != operation) {
        return VISMUT_TYPE_KIND_UNKNOWN;
    }

    for (int rule_id = 0; rule_id < rules->rules_count; ++rule_id) {
        const op_rule *rule = &rules->rules[rule_id];
        if (rule->left_type == left && rule->right_type == right) {
            return rule->result_type;
        }
    }

    return VISMUT_TYPE_KIND_UNKNOWN;
}

attribute_pure static const VismutType *
VismutTypeNumericKind_ToVismutType(const VismutTypeContext *ctx, const VismutTypeKind kind) {
    switch (kind) {
    case VISMUT_TYPE_KIND_I1:
        return ctx->type_i1;
    case VISMUT_TYPE_KIND_I8:
        return ctx->type_i8;
    case VISMUT_TYPE_KIND_I16:
        return ctx->type_i16;
    case VISMUT_TYPE_KIND_I32:
        return ctx->type_i32;
    case VISMUT_TYPE_KIND_I64:
        return ctx->type_i64;
    case VISMUT_TYPE_KIND_U8:
        return ctx->type_u8;
    case VISMUT_TYPE_KIND_U16:
        return ctx->type_u16;
    case VISMUT_TYPE_KIND_U32:
        return ctx->type_u32;
    case VISMUT_TYPE_KIND_U64:
        return ctx->type_u64;
    case VISMUT_TYPE_KIND_F32:
        return ctx->type_f32;
    case VISMUT_TYPE_KIND_F64:
        return ctx->type_f64;
    default:
        assert("Unreachable!");
        return NULL;
    }
}

attribute_nodiscard VismutErrorType GetBinaryOpResult(VismutTypeContext *restrict ctx,
                                                      const VismutType *restrict left_t,
                                                      const VismutType *restrict right_t,
                                                      const VismutType **restrict out_type,
                                                      const ASTBinaryNodeType operation,
                                                      VismutErrorDetails *restrict out_details) {
    const int left_is_numeric =
        VismutTypeKind_IsInt(left_t->kind) || VismutTypeKind_IsFloat(left_t->kind);
    const int right_is_numeric =
        VismutTypeKind_IsInt(right_t->kind) || VismutTypeKind_IsFloat(right_t->kind);

    if (left_is_numeric && right_is_numeric) {
        const VismutTypeKind result_type =
            GetBinaryOpResult_ForNumeric(left_t->kind, right_t->kind, operation);
        if (result_type == VISMUT_TYPE_KIND_UNKNOWN) {
            *out_details = (VismutErrorDetails){
                .binary =
                    {
                        .left = left_t->kind,
                        .right = right_t->kind,
                        .op = operation,
                    },
            };
            return VISMUT_ERR_UNSUPPORTED_BINARY;
        }
        *out_type = VismutTypeNumericKind_ToVismutType(ctx, result_type);
        return VISMUT_OK;
    }

    *out_details = (VismutErrorDetails){
        .binary =
            {
                .left = left_t->kind,
                .right = right_t->kind,
                .op = operation,
            },
    };
    return VISMUT_ERR_UNSUPPORTED_BINARY;
}

VismutErrorType GetUnaryOpResult(VismutTypeContext *restrict ctx,
                                 const VismutType *restrict right_t,
                                 const ASTUnaryNodeType operation,
                                 const VismutType **restrict out_type,
                                 VismutErrorDetails *restrict out_details) {
    switch (operation) {
    case VISMUT_AST_UNARY_PLUS:
        if (right_t == ctx->type_i1) {
            break;
        }
        if (!VismutTypeKind_IsInt(right_t->kind) && !VismutTypeKind_IsFloat(right_t->kind)) {
            break;
        }
        *out_type = right_t;
        return VISMUT_OK;
    case VISMUT_AST_UNARY_MINUS:
        if (right_t == ctx->type_i1 || VismutTypeKind_IsUInt(right_t->kind)) {
            break;
        }
        if (!VismutTypeKind_IsInt(right_t->kind) && !VismutTypeKind_IsFloat(right_t->kind)) {
            break;
        }
        *out_type = right_t;
        return VISMUT_OK;
    case VISMUT_AST_UNARY_LOGICAL_NOT:
        if (right_t != ctx->type_i1) {
            break;
        }
        *out_type = right_t;
        return VISMUT_OK;
    case VISMUT_AST_UNARY_BITWISE_NOT:
        if (right_t == ctx->type_i1) {
            break;
        }
        if (!VismutTypeKind_IsInt(right_t->kind)) {
            break;
        }
        *out_type = right_t;
        return VISMUT_OK;
    default:
        break;
    }

    *out_details = (VismutErrorDetails){
        .unary =
            {
                .right = right_t->kind,
                .op = operation,
            },
    };
    return VISMUT_ERR_UNSUPPORTED_UNARY;
}

static ASTBinaryNodeType assign_type_to_binary_op_type(const ASTAssignNodeType type) {
    switch (type) {
    default:
        return VISMUT_AST_BINARY_UNKNOWN;
    }
}

VismutErrorType CheckAssignment(VismutTypeContext *restrict ctx,
                                const VismutType *restrict target_t,
                                const VismutType *restrict value_t,
                                const ASTAssignNodeType operation,
                                VismutErrorDetails *restrict out_details) {
    if (operation == VISMUT_AST_ASSIGN_EQUAL) {
        if (target_t != value_t) {
            *out_details = (VismutErrorDetails){
                .type_kind = target_t->kind,
            };
            return VISMUT_ERR_UNEXPECTED_TYPE;
        } else {
            return VISMUT_OK;
        }
    }

    const ASTBinaryNodeType binary_operation = assign_type_to_binary_op_type(operation);

    const VismutType *op_result_type;
    VismutErrorType err;
    SAFE_RISKY_EXPRESSION(
        GetBinaryOpResult(ctx, target_t, value_t, &op_result_type, binary_operation, out_details),
        err);

    if (op_result_type != target_t) {
        *out_details = (VismutErrorDetails){
            .type_kind = target_t->kind,
        };
        return VISMUT_ERR_UNEXPECTED_TYPE;
    }

    return VISMUT_OK;
}
