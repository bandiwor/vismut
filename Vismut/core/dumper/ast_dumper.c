#include "ast_dumper.h"
#include <assert.h>
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>

ASTDumper ASTDumper_Create(ASTBuilder *restrict nodes, const u32 mode) {
    return (ASTDumper){
        .builder = nodes,
        .str = NULL,
        .str_length = 0,
        .str_allocated = 0,
        .ctx =
            {
                .mode = mode,
            },
    };
}

void ASTDumper_Reset(ASTDumper *restrict dumper) {
    dumper->str_length = 0;
}

void ASTDumper_Destroy(ASTDumper *restrict dumper) {
    if (dumper->str_allocated > 0 && dumper->str != NULL) {
        free(dumper->str);
    }
}

static u32 get_new_capacity(const u32 old_capacity) {
    return old_capacity < 128 ? 128 : old_capacity * 2;
}

static void str_realloc(ASTDumper *restrict ctx, const u32 new_capacity) {
    u8 *new_str = realloc(ctx->str, new_capacity);
    assert(new_str != NULL);

    ctx->str = new_str;
    ctx->str_allocated = new_capacity;
}

static void str_ensure_capacity(ASTDumper *restrict ctx, const u32 add_capacity) {
    const u32 new_capacity = add_capacity + ctx->str_length;
    if (new_capacity < ctx->str_allocated) {
        return;
    }
    str_realloc(ctx, new_capacity > get_new_capacity(ctx->str_allocated)
                         ? new_capacity
                         : get_new_capacity(ctx->str_allocated));
}

static void write(ASTDumper *restrict ctx, const StringView sv) {
    str_ensure_capacity(ctx, sv.length);

    __builtin_memcpy(ctx->str + ctx->str_length, sv.data, sv.length);
    ctx->str_length += sv.length;
}

static void write_char(ASTDumper *restrict ctx, const char c) {
    str_ensure_capacity(ctx, 1);
    ctx->str[ctx->str_length++] = (u8)c;
}

static void write_int(ASTDumper *restrict dumper, u64 value, int is_negative) {
    u8 buf[24];
    u8 *p = buf + sizeof(buf);

    do {
        *--p = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    if (is_negative) {
        *--p = '-';
    }

    const u32 len = (buf + sizeof(buf)) - p;
    write(dumper, (StringView){.data = p, .length = len});
}

#define at(idx) ASTBuilder_ConstNodeAt(ctx->builder, idx)

static void ASTDumper_DumpNode(ASTDumper *restrict ctx, const ASTNodeIdx idx);

static ASTNodeIdx list_next(ASTDumper *restrict ctx, const ASTNodeIdx idx) {
    switch (at(idx)->type) {
    case VISMUT_AST_EXPRESSION:
        return at(idx)->expression.next;
    case VISMUT_AST_DECLARATION:
        return at(idx)->declaration.next;
    default:
        return ASTNodeIdx_None;
    }
}

static ASTNodeIdx list_node_payload(ASTDumper *restrict ctx, const ASTNodeIdx idx) {
    switch (at(idx)->type) {
    case VISMUT_AST_EXPRESSION:
        return at(idx)->expression.expr;
    case VISMUT_AST_DECLARATION:
        return at(idx)->declaration.decl;
    default:
        return ASTNodeIdx_None;
    }
}

static void dump_list(ASTDumper *restrict ctx, const ASTNodeIdx list_head) {
    ASTNodeIdx list_current = list_head;

    while (!ASTNodeIdx_IsNone(list_current)) {
        ASTDumper_DumpNode(ctx, list_node_payload(ctx, list_current));

        list_current = list_next(ctx, list_current);

        if (!ASTNodeIdx_IsNone(list_current)) {
            write(ctx, StringView_FromCStr(", "));
        }
    }
}

static void write_literal(ASTDumper *restrict ctx, const VismutTypeKind kind,
                          const VismutSimpleValue value) {
    switch (kind) {
    case VISMUT_TYPE_KIND_I1:
    case VISMUT_TYPE_KIND_I8:
    case VISMUT_TYPE_KIND_I16:
    case VISMUT_TYPE_KIND_I32:
    case VISMUT_TYPE_KIND_I64:
        write_int(ctx, value.i, value.i < 0);
        return;
    case VISMUT_TYPE_KIND_INT:
    case VISMUT_TYPE_KIND_U8:
    case VISMUT_TYPE_KIND_U16:
    case VISMUT_TYPE_KIND_U32:
    case VISMUT_TYPE_KIND_U64:
        write_int(ctx, value.u, 0);
        return;
    case VISMUT_TYPE_KIND_F32:
    case VISMUT_TYPE_KIND_F64:
    case VISMUT_TYPE_KIND_FLOAT:
        str_ensure_capacity(ctx, 64);
        int written = snprintf((char *)ctx->str + ctx->str_length,
                               ctx->str_allocated - ctx->str_length, "%.14g", value.f);
        if (written > 0) {
            ctx->str_length += written;
        }
        return;
    case VISMUT_TYPE_KIND_STR: {
        const StringView sv = value.str;
        str_ensure_capacity(ctx, sv.length + 2 + (sv.length / 8));

        write_char(ctx, '"');
        for (u32 i = 0; i < sv.length; ++i) {
            const u8 c = sv.data[i];
            switch (c) {
            case '\n':
                write(ctx, StringView_FromCStr("\\n"));
                break;
            case '\r':
                write(ctx, StringView_FromCStr("\\r"));
                break;
            case '\t':
                write(ctx, StringView_FromCStr("\\t"));
                break;
            case '\\':
                write(ctx, StringView_FromCStr("\\\\"));
                break;
            case '"':
                write(ctx, StringView_FromCStr("\\\""));
                break;
            case '\0':
                write(ctx, StringView_FromCStr("\\0"));
                break;
            default:
                if (c >= 0x20 && c <= 0x7E) {
                    write_char(ctx, (char)c);
                } else {
                    u8 hex_buf[4];
                    hex_buf[0] = '\\';
                    hex_buf[1] = 'x';
                    hex_buf[2] = "0123456789ABCDEF"[c >> 4];
                    hex_buf[3] = "0123456789ABCDEF"[c & 0x0F];
                    write(ctx, (StringView){.data = hex_buf, .length = 4});
                }
                break;
            }
        }
        write_char(ctx, '"');
        return;
    }
    default:
        return;
    }
}

static void ASTDumper_DumpType(ASTDumper *restrict ctx, const VismutType *type) {
    switch (type->kind) {
    case VISMUT_TYPE_KIND_UNIT:
    case VISMUT_TYPE_KIND_AUTO:
    case VISMUT_TYPE_KIND_I1:
    case VISMUT_TYPE_KIND_I8:
    case VISMUT_TYPE_KIND_I16:
    case VISMUT_TYPE_KIND_I32:
    case VISMUT_TYPE_KIND_I64:
    case VISMUT_TYPE_KIND_U8:
    case VISMUT_TYPE_KIND_U16:
    case VISMUT_TYPE_KIND_U32:
    case VISMUT_TYPE_KIND_U64:
    case VISMUT_TYPE_KIND_F32:
    case VISMUT_TYPE_KIND_F64:
    case VISMUT_TYPE_KIND_STR:
    case VISMUT_TYPE_KIND_INT:
    case VISMUT_TYPE_KIND_FLOAT:
    case VISMUT_TYPE_KIND_NEVER:
        write(ctx, StringView_FromCStr((const char *)VismutTypeKind_String(type->kind)));
        return;
    case VISMUT_TYPE_KIND_ARRAY: {
        write_char(ctx, '[');
        ASTDumper_DumpType(ctx, type->array.element_type);
        write(ctx, StringView_FromCStr("; "));

        u8 buf[16];
        u8 *p = buf + sizeof(buf);
        u32 val = type->array.size;
        do {
            *--p = '0' + (val % 10);
            val /= 10;
        } while (val > 0);

        write(ctx, (StringView){.data = p, .length = buf + sizeof(buf) - p});
        write_char(ctx, ']');
    }
        return;
    case VISMUT_TYPE_KIND_VECTOR:
        write_char(ctx, '<');
        ASTDumper_DumpType(ctx, type->array.element_type);
        write_char(ctx, '>');
        return;
    case VISMUT_TYPE_KIND_TUPLE:
        write_char(ctx, '(');
        for (u32 i = 0; i < type->tuple.fields_count; ++i) {
            if (i > 0) {
                write(ctx, StringView_FromCStr(", "));
            }
            ASTDumper_DumpType(ctx, type->tuple.fields[i]);
        }
        write_char(ctx, ')');
        return;
    case VISMUT_TYPE_KIND_FUNCTION:
        write_char(ctx, '(');
        for (u32 i = 0; i < type->function.param_count; ++i) {
            if (i > 0) {
                write(ctx, StringView_FromCStr(", "));
            }
            ASTDumper_DumpType(ctx, type->function.param_types[i]);
        }
        write(ctx, StringView_FromCStr(") -> "));
        ASTDumper_DumpType(ctx, type->function.return_type);
        return;
    case VISMUT_TYPE_KIND_STRUCTURE:
    case VISMUT_TYPE_KIND_STRUCTURE_INSTANCE:
        write(ctx, StringView_FromCStr("<struct>"));
        return;
    default:
        write(ctx, StringView_FromCStr("<unknown>"));
        return;
    }
}

static void ASTDumper_DumpNode(ASTDumper *restrict ctx, const ASTNodeIdx idx) {
    if (unlikely(ASTNodeIdx_IsNone(idx))) {
        write(ctx, StringView_FromCStr("None"));
        return;
    }

    switch (at(idx)->type) {
    case VISMUT_AST_MODULE:
        write(ctx, StringView_FromCStr("module("));
        dump_list(ctx, at(idx)->module.first_statement);
        write(ctx, StringView_FromCStr(")"));
        return;

    case VISMUT_AST_BLOCK:
        write(ctx, StringView_FromCStr("block("));
        dump_list(ctx, at(idx)->block.first_statement);
        write(ctx, StringView_FromCStr(")"));
        return;

    case VISMUT_AST_EXPRESSION:
        ASTDumper_DumpNode(ctx, at(idx)->expression.expr);
        return;

    case VISMUT_AST_DECLARATION:
        ASTDumper_DumpNode(ctx, at(idx)->declaration.decl);
        return;

    case VISMUT_AST_BINARY:
        write(ctx, StringView_FromCStr("bin("));
        write(ctx, StringView_FromCStr((const char *)ASTBinaryNodeType_String(at(idx)->binary.op)));
        write(ctx, StringView_FromCStr(", "));
        ASTDumper_DumpNode(ctx, at(idx)->binary.left);
        write(ctx, StringView_FromCStr(", "));
        ASTDumper_DumpNode(ctx, at(idx)->binary.right);
        write(ctx, StringView_FromCStr(")"));
        return;

    case VISMUT_AST_UNARY:
        write(ctx, StringView_FromCStr("un("));
        write(ctx, StringView_FromCStr((const char *)ASTUnaryNodeType_String(at(idx)->unary.op)));
        write(ctx, StringView_FromCStr(", "));
        ASTDumper_DumpNode(ctx, at(idx)->unary.right);
        write(ctx, StringView_FromCStr(")"));
        return;

    case VISMUT_AST_LITERAL:
        write(ctx, StringView_FromCStr("lit("));
        write_literal(ctx, at(idx)->literal.type->kind, at(idx)->literal.value);
        write(ctx, StringView_FromCStr(")"));
        return;

    case VISMUT_AST_IDENTIFIER:
        write(ctx, StringView_FromCStr("`"));
        write(ctx, at(idx)->identifier.name);
        write(ctx, StringView_FromCStr("`"));
        return;

    case VISMUT_AST_CONDITION:
        write(ctx, StringView_FromCStr("cond("));
        ASTDumper_Dump(ctx, at(idx)->condition.condition);
        write(ctx, StringView_FromCStr(", "));
        ASTDumper_Dump(ctx, at(idx)->condition.then);
        write(ctx, StringView_FromCStr(", "));
        ASTDumper_Dump(ctx, at(idx)->condition.else_);
        write(ctx, StringView_FromCStr(")"));
        return;

    case VISMUT_AST_LOOP:
        write(ctx, StringView_FromCStr("loop("));
        ASTDumper_Dump(ctx, at(idx)->loop.condition);
        write(ctx, StringView_FromCStr(", "));
        ASTDumper_Dump(ctx, at(idx)->loop.body);
        write(ctx, StringView_FromCStr(")"));
        return;

    case VISMUT_AST_VAR_DECLARATION:
        write(ctx, StringView_FromCStr("var_decl"));
        if (at(idx)->var_declaration.is_mutable) {
            write_char(ctx, '!');
        }
        write(ctx, StringView_FromCStr("("));
        write(ctx, at(idx)->var_declaration.name);
        write(ctx, StringView_FromCStr(", "));
        ASTDumper_DumpNode(ctx, at(idx)->var_declaration.init);
        write(ctx, StringView_FromCStr(")"));
        return;

    case VISMUT_AST_FN_DECLARATION:
        write(ctx, StringView_FromCStr("fn_decl("));
        write(ctx, at(idx)->fn_declaration.name);
        write(ctx, StringView_FromCStr(", ("));

        const u32 params_count = at(idx)->fn_declaration.signature->function.param_count;
        u32 param = 0;
        while (param < params_count) {
            write(ctx, at(idx)->fn_declaration.param_names[0]);
            write(ctx, StringView_FromCStr(": "));
            ASTDumper_DumpType(ctx, at(idx)->fn_declaration.signature->function.param_types[param]);

            ++param;

            if (param < params_count) {
                write(ctx, StringView_FromCStr(", "));
            }
        }

        write(ctx, StringView_FromCStr("), "));
        ASTDumper_DumpNode(ctx, at(idx)->fn_declaration.body);
        write(ctx, StringView_FromCStr(")"));
        return;

    case VISMUT_AST_ASSIGNMENT:
        write(ctx, StringView_FromCStr("assign("));
        ASTDumper_DumpNode(ctx, at(idx)->assignment.target);
        write(ctx, StringView_FromCStr(", "));
        write(ctx, StringView_FromCStr(
                       (const char *)ASTAssignNodeType_String(at(idx)->assignment.operation)));
        write(ctx, StringView_FromCStr(", "));
        ASTDumper_DumpNode(ctx, at(idx)->assignment.value);
        write(ctx, StringView_FromCStr(")"));
        return;

    case VISMUT_AST_RETURN:
        write(ctx, StringView_FromCStr("ret("));
        ASTDumper_DumpNode(ctx, at(idx)->ret.expression);
        write(ctx, StringView_FromCStr(")"));
        return;

    case VISMUT_AST_TYPE_CAST:
        write(ctx, StringView_FromCStr("cast("));
        ASTDumper_DumpType(ctx, at(idx)->type_cast.to_type);
        write(ctx, StringView_FromCStr(", "));
        ASTDumper_DumpNode(ctx, at(idx)->type_cast.argument);
        write(ctx, StringView_FromCStr(")"));
        return;

    case VISMUT_AST_TUPLE: {
        write(ctx, StringView_FromCStr("tuple("));
        const u32 fields_count = at(idx)->tuple.fields_count;
        u32 field = 0;

        while (field < fields_count) {
            ASTDumper_DumpNode(ctx, at(idx)->tuple.fields[field]);

            ++field;
            if (field < fields_count) {
                write(ctx, StringView_FromCStr(", "));
            }
        }

        write(ctx, StringView_FromCStr(")"));
    }
        return;

    case VISMUT_AST_UNIT: {
        write(ctx, StringView_FromCStr("unit"));
    }
        return;

    case VISMUT_AST_DOT: {
        ASTDumper_DumpNode(ctx, at(idx)->dot.object);
        write(ctx, StringView_FromCStr("."));
        write(ctx, at(idx)->dot.member);
    }
        return;

    case VISMUT_AST_CALL:
        ASTDumper_DumpNode(ctx, at(idx)->call.object);
        write(ctx, StringView_FromCStr("("));

        const u32 args_count = at(idx)->call.arguments_count;
        u32 arg = 0;
        while (arg < args_count) {
            ASTDumper_DumpNode(ctx, at(idx)->call.arguments[arg]);
            ++arg;
            if (arg < args_count) {
                write(ctx, StringView_FromCStr(", "));
            }
        }

        write(ctx, StringView_FromCStr(")"));
        return;

    default:
        write(ctx, StringView_FromCStr("(unknown)"));
        return;
    }
}

StringView ASTDumper_Dump(ASTDumper *restrict dumper, const ASTNodeIdx idx) {
    ASTDumper_DumpNode(dumper, idx);
    return (StringView){.data = dumper->str, .length = dumper->str_length};
}
