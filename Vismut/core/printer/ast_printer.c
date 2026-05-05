#include "ast_printer.h"
#include "../ast/ast.h"
#include "../ast/ast_builder.h"
#include "../memory/type_context.h"
#include "../utils/ast_utils.h"
#include "../utils/indent.h"
#include <inttypes.h>
#include <stdio.h>

ASTPrinter ASTPrinter_Create(const ASTBuilder *const nodes, FILE *out, const int enable_colors) {
    return (ASTPrinter){
        .nodes = nodes,
        .out = out,
        .enable_colors = enable_colors,
    };
}

#define file ctx->out
#define node_ptr(idx) ASTBuilder_ConstNodeAt(ctx->nodes, idx)
#define at(idx) node_ptr(idx)
#define type_at(idx) at(idx)->type
#define type_str_at(idx) (const char *)ASTNodeType_String(type_at(idx))
#define pos_at(idx) at(idx)->pos
#define write(str) fputs((const char *)(str), file)
#define writef(format, ...) fprintf(file, format, __VA_ARGS__)
#define writechar(char) fputc(char, file)
#define str_2fmt(view) (int)(view).length, (const char *)(view).data

#define str_fmt "%.*s"
#define new_line "\n"
#define space " "
#define fmt_i64 ("%" PRId64)
#define fmt_u64 ("%" PRIu64)

static void PrintType(const ASTPrinter *ctx, const VismutType *type) {
    const VismutTypeKind kind = type->kind;
    const u8 *type_kind_str = VismutTypeKind_String(kind);
    switch (kind) {
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
        write(type_kind_str);
        break;
    case VISMUT_TYPE_KIND_ARRAY:
        writechar('[');
        PrintType(ctx, type->array.element_type);
        writef("; %u]", type->array.size);
        break;
    case VISMUT_TYPE_KIND_VECTOR:
        writechar('<');
        PrintType(ctx, type->array.element_type);
        writechar('>');
        break;
    case VISMUT_TYPE_KIND_TUPLE:
        writechar('(');
        for (u32 i = 0; i < type->tuple.fields_count; ++i) {
            if (i > 0) {
                write(", ");
            }
            PrintType(ctx, type->tuple.fields[i]);
        }
        writechar(')');
        break;
    case VISMUT_TYPE_KIND_STRUCTURE:
        break;
    case VISMUT_TYPE_KIND_STRUCTURE_INSTANCE:
        break;
    case VISMUT_TYPE_KIND_FUNCTION:
        writechar('(');
        for (u32 i = 0; i < type->function.param_count; ++i) {
            if (i > 0) {
                write(", ");
            }
            PrintType(ctx, type->function.param_types[i]);
        }
        write(") -> ");
        PrintType(ctx, type->function.return_type);
        break;
    case VISMUT_TYPE_KIND_MODULE_REF:
        writef("mod(%u)", type->module_ref.idx);
        break;
    case VISMUT_TYPE_KIND_UNKNOWN:
    case VISMUT_TYPE_KIND_COUNT:
        break;
    }
}

static void PrintLiteral(const ASTPrinter *ctx, const VismutType *type,
                         const VismutSimpleValue value) {
    switch (type->kind) {
    case VISMUT_TYPE_KIND_I1:
    case VISMUT_TYPE_KIND_I8:
    case VISMUT_TYPE_KIND_I16:
    case VISMUT_TYPE_KIND_I32:
    case VISMUT_TYPE_KIND_I64:
        writef(fmt_i64, value.i);
        break;
    case VISMUT_TYPE_KIND_U8:
    case VISMUT_TYPE_KIND_U16:
    case VISMUT_TYPE_KIND_U32:
    case VISMUT_TYPE_KIND_U64:
    case VISMUT_TYPE_KIND_INT:
        writef(fmt_u64, value.u);
        break;
    case VISMUT_TYPE_KIND_F32:
    case VISMUT_TYPE_KIND_F64:
    case VISMUT_TYPE_KIND_FLOAT:
        writef("%.17g", value.f);
        break;
    case VISMUT_TYPE_KIND_STR: {
        writechar('"');
        const StringView str = value.str;
        for (const u8 *ptr = str.data; ptr < str.data + str.length; ++ptr) {
            const u8 c = *ptr;
            switch (c) {
            case '\n':
                write("\\n");
                break;
            case '\t':
                write("\\t");
                break;
            case '\r':
                write("\\r");
                break;
            case '\b':
                write("\\b");
                break;
            case '\v':
                write("\\v");
                break;
            case '\f':
                write("\\f");
                break;
            case '\\':
                write("\\\\");
                break;
            default:
                writechar(c);
                break;
            }
        }
        writechar('"');
        break;
    }
    default:
        break;
    }
}

static void PrintFunctionParams(const ASTPrinter *restrict ctx,
                                const VismutType **restrict types_begin,
                                StringView *restrict names_begin, const u32 length) {
    writechar('(');
    for (u32 i = 0; i < length; ++i) {
        if (i > 0) {
            write(", ");
        }
        writef(str_fmt ": ", str_2fmt(names_begin[i]));
        PrintType(ctx, types_begin[i]);
    }
    writechar(')');
}

static void PrintNode(const ASTPrinter *ctx, const ASTNodeIdx idx, const unsigned int indent);

static void PrintNodesList(const ASTPrinter *restrict ctx, const ASTNodeIdx *restrict begin,
                           const u32 length, const u32 indent) {
    writechar('(');
    if (length > 0) {
        writechar('\n');
    }

    const ASTNodeIdx *end = begin + length;
    for (const ASTNodeIdx *current = begin; current < end; ++current) {
        PrintNode(ctx, *current, indent + 1);
    }

    if (length > 0) {
        write(get_indent(indent));
    }
    write(")\n");
}

static void PrintNode(const ASTPrinter *ctx, const ASTNodeIdx idx, const unsigned int indent) {
    write(get_indent(indent));
    write(type_str_at(idx));
    writechar(' ');

    switch (type_at(idx)) {

    case VISMUT_AST_MODULE: {
        writechar('\n');
        ASTNodeIdx curr_expr = at(idx)->module.first_statement;
        while (!ASTNodeIdx_IsNone(curr_expr)) {
            PrintNode(ctx, curr_expr, indent + 1);
            curr_expr = ASTNode_GetNextStatement(at(curr_expr));
        }
    } break;
    case VISMUT_AST_EXPRESSION:
        PrintType(ctx, at(idx)->expression.type);
        write(":\n");
        PrintNode(ctx, at(idx)->expression.expr, indent + 1);
        break;
    case VISMUT_AST_DECLARATION:
        if (at(idx)->declaration.is_export) {
            write("<+");
        }
        write(":\n");
        PrintNode(ctx, at(idx)->declaration.decl, indent + 1);
        break;
    case VISMUT_AST_VAR_DECLARATION:
        writef(str_fmt "%s: ", str_2fmt(at(idx)->var_declaration.name),
               at(idx)->var_declaration.is_mutable ? "!" : "");
        PrintType(ctx, at(idx)->var_declaration.type);
        writechar('\n');
        PrintNode(ctx, at(idx)->var_declaration.init, indent + 1);
        break;
    case VISMUT_AST_LITERAL:
        PrintType(ctx, at(idx)->literal.type);
        writechar(' ');
        PrintLiteral(ctx, at(idx)->literal.type, at(idx)->literal.value);
        writechar('\n');
        break;
    case VISMUT_AST_IDENTIFIER:
        writef(str_fmt " ", str_2fmt(at(idx)->identifier.name));
        PrintType(ctx, at(idx)->identifier.type);
        writechar('\n');
        break;
    case VISMUT_AST_BINARY:
        write(ASTBinaryNodeType_String(at(idx)->binary.op));
        writechar(' ');
        PrintType(ctx, at(idx)->binary.type);
        writechar('\n');
        write(get_indent(indent));
        write("|>\n");
        PrintNode(ctx, at(idx)->binary.left, indent + 1);
        write(get_indent(indent));
        write("|>\n");
        PrintNode(ctx, at(idx)->binary.right, indent + 1);
        break;
    case VISMUT_AST_UNARY:
        write(ASTUnaryNodeType_String(at(idx)->unary.op));
        writechar(' ');
        PrintType(ctx, at(idx)->unary.type);
        writechar('\n');
        write(get_indent(indent));
        write("|>\n");
        PrintNode(ctx, at(idx)->unary.right, indent + 1);
        break;
    case VISMUT_AST_TYPE_CAST:
        PrintType(ctx, at(idx)->type_cast.from_type);
        write(" -> ");
        PrintType(ctx, at(idx)->type_cast.to_type);
        writechar('\n');
        PrintNode(ctx, at(idx)->type_cast.argument, indent + 1);
        break;
    case VISMUT_AST_LOOP:
        writechar('\n');
        write(get_indent(indent));
        write("?\n");
        PrintNode(ctx, at(idx)->loop.condition, indent + 1);
        write(get_indent(indent));
        write("|>\n");
        PrintNode(ctx, at(idx)->loop.body, indent + 1);
        break;
    case VISMUT_AST_CONDITION:
        PrintType(ctx, at(idx)->condition.type);
        writechar('\n');
        write(get_indent(indent + 1));
        write("?\n");
        PrintNode(ctx, at(idx)->condition.condition, indent + 2);
        write(get_indent(indent + 1));
        write("=>\n");
        PrintNode(ctx, at(idx)->condition.then, indent + 2);
        if (at(idx)->condition.else_ != ASTNodeIdx_None) {
            write(get_indent(indent + 1));
            write("!>\n");
            PrintNode(ctx, at(idx)->condition.else_, indent + 2);
        }
        break;
    case VISMUT_AST_BLOCK: {
        PrintType(ctx, at(idx)->block.type);
        writechar('\n');
        ASTNodeIdx curr_stmt = at(idx)->block.first_statement;
        while (!ASTNodeIdx_IsNone(curr_stmt)) {
            PrintNode(ctx, curr_stmt, indent + 1);
            curr_stmt = ASTNode_GetNextStatement(at(curr_stmt));
        }
    } break;
    case VISMUT_AST_FN_DECLARATION:
        writef(str_fmt, str_2fmt(at(idx)->fn_declaration.name));
        PrintFunctionParams(ctx, at(idx)->fn_declaration.signature->function.param_types,
                            at(idx)->fn_declaration.param_names,
                            at(idx)->fn_declaration.signature->function.param_count);
        write(" -> ");
        PrintType(ctx, at(idx)->fn_declaration.signature->function.return_type);
        writechar('\n');
        PrintNode(ctx, at(idx)->fn_declaration.body, indent + 1);
        break;
    case VISMUT_AST_TUPLE:
        PrintType(ctx, at(idx)->tuple.type);
        writechar(' ');
        PrintNodesList(ctx, at(idx)->tuple.fields, at(idx)->tuple.fields_count, indent);
        break;
    case VISMUT_AST_RETURN:
        writechar('\n');
        PrintNode(ctx, at(idx)->ret.expression, indent + 1);
        break;
    case VISMUT_AST_ASSIGNMENT:
        write(ASTAssignNodeType_String(at(idx)->assignment.operation));
        writechar('\n');
        PrintNode(ctx, at(idx)->assignment.target, indent + 1);
        PrintNode(ctx, at(idx)->assignment.value, indent + 1);
        break;
    case VISMUT_AST_IMPORT:
        writef("\"" str_fmt "\""
               " => " str_fmt " [%u]\n",
               str_2fmt(at(idx)->import.module), str_2fmt(at(idx)->import.alias),
               at(idx)->import.module_idx);
        break;
    case VISMUT_AST_DOT:
        writef("." str_fmt " ", str_2fmt(at(idx)->dot.member));
        PrintType(ctx, at(idx)->dot.type);
        writechar('\n');

        PrintNode(ctx, at(idx)->dot.object, indent + 1);
        break;
    case VISMUT_AST_CALL:
        PrintType(ctx, at(idx)->call.type);
        writechar(' ');
        PrintNodesList(ctx, at(idx)->call.arguments, at(idx)->call.arguments_count, indent);
        PrintNode(ctx, at(idx)->call.object, indent + 1);
        break;
    case VISMUT_AST_UNIT:
    case VISMUT_AST_UNKNOWN:
        writechar('\n');
        break;
    default:
        write(": Unhandled node type!\n");
        break;
    }
}

void ASTPrinter_Print(ASTPrinter *printer, const ASTNodeIdx idx) {
    PrintNode(printer, idx, 0);
}
