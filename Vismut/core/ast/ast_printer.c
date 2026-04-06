#include "ast_printer.h"
#include "ast.h"
#include "value.h"
#include <stdio.h>

ASTPrinter ASTPrinter_Create(const ASTNode *const nodes, const u64 nodes_count, FILE *out,
                             const int enable_colors) {
    return (ASTPrinter){
        .nodes = nodes,
        .nodes_count = nodes_count,
        .out = out,
        .enable_colors = enable_colors,
    };
}

static const char *const indents[] = {"",
                                      "\t",
                                      "\t\t",
                                      "\t\t\t",
                                      "\t\t\t\t",
                                      "\t\t\t\t\t",
                                      "\t\t\t\t\t\t",
                                      "\t\t\t\t\t\t\t",
                                      "\t\t\t\t\t\t\t\t",
                                      "\t\t\t\t\t\t\t\t\t",
                                      "\t\t\t\t\t\t\t\t\t\t",
                                      "\t\t\t\t\t\t\t\t\t\t\t",
                                      "\t\t\t\t\t\t\t\t\t\t\t\t",
                                      "\t\t\t\t\t\t\t\t\t\t\t\t\t",
                                      "\t\t\t\t\t\t\t\t\t\t\t\t\t\t",
                                      "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t",
                                      "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t",
                                      "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t",
                                      "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t",
                                      "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t",
                                      "-<TOO DEEP>--"};

const int indents_count = COUNTOF(indents) - 1;

#define file() ctx->out
#define node_ptr(idx) (ctx->nodes[idx])
#define node_curr() node_ptr(idx)
#define node_t(idx) node_ptr(idx).type
#define node_t_str(idx) (const char *)ASTNodeType_String(node_t(idx))
#define node_pos(idx) node_ptr(idx).pos
#define get_indent(indent) ((indent < indents_count) ? indents[indent] : indents[indents_count])
#define write(str) fputs((const char *)(str), file())
#define writef(format, ...) fprintf(file(), format, __VA_ARGS__)
#define writechar(char) fputc(char, file())
#define str_2fmt(string_node) string_node->length, string_node->data

#define str_fmt "%.*s"
#define new_line "\n"
#define space " "

static void PrintType(const ASTPrinter *ctx, const VismutType *type) {
    const VismutTypeKind kind = type->kind;
    const u8 *type_kind_str = VismutTypeKind_String(kind);
    switch (kind) {
    case VISMUT_TYPE_KIND_VOID:
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
        write(type_kind_str);
        break;
    case VISMUT_TYPE_KIND_POINTER:
        PrintType(ctx, type->pointer.element_type);
        write("*");
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
    case VISMUT_TYPE_KIND_STRUCTURE:
        break;
    case VISMUT_TYPE_KIND_STRUCTURE_INSTANCE:
        break;
    case VISMUT_TYPE_KIND_FUNCTION:
        writechar('(');
        if (type->function.param_count > 0) {
            for (const VismutType **current_type = type->function.param_types;
                 current_type < type->function.param_types + type->function.param_count;
                 ++current_type) {
                PrintType(ctx, *current_type);
                if (current_type < type->function.param_types + type->function.param_count - 1) {
                    write(", ");
                }
            }
        }
        write(") -> ");
        PrintType(ctx, type->function.return_type);
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
    case VISMUT_TYPE_KIND_U8:
    case VISMUT_TYPE_KIND_U16:
    case VISMUT_TYPE_KIND_U32:
    case VISMUT_TYPE_KIND_U64:
    case VISMUT_TYPE_KIND_INT:
        writef("%ld", value.u);
        break;
    case VISMUT_TYPE_KIND_F32:
    case VISMUT_TYPE_KIND_F64:
    case VISMUT_TYPE_KIND_FLOAT:
        writef("%.17g", value.f);
        break;
    case VISMUT_TYPE_KIND_STR: {
        writechar('"');
        const StringNode *str = value.str;
        for (const u8 *ptr = str->data; ptr < str->data + str->length; ++ptr) {
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

static void PrintNode(const ASTPrinter *ctx, const ASTNodeIdx idx, const unsigned int indent) {
    write(get_indent(indent));
    write(node_t_str(idx));
    writechar(' ');

    switch (node_t(idx)) {

    case VISMUT_AST_MODULE: {
        writef(str_fmt new_line, str_2fmt(node_curr().module.name));

        ASTNodeIdx curr_expr = node_ptr(idx).module.first_expression;
        while (!ASTNodeIdx_IsNone(curr_expr)) {
            PrintNode(ctx, curr_expr, indent + 1);
            curr_expr = node_ptr(curr_expr).expression.next_expr;
        }
    } break;
    case VISMUT_AST_EXPRESSION:
        PrintType(ctx, node_curr().expression.type);
        write(":\n");
        PrintNode(ctx, node_ptr(idx).expression.expr, indent + 1);
        break;
    case VISMUT_AST_VAR_DECLARATION:
        writef(str_fmt "%s: ", str_2fmt(node_curr().var_declaration.name),
               node_curr().var_declaration.is_mutable ? "!" : "");
        PrintType(ctx, node_curr().var_declaration.type);
        writechar('\n');
        PrintNode(ctx, node_curr().var_declaration.init, indent + 1);
        break;
    case VISMUT_AST_LITERAL:
        PrintType(ctx, node_curr().literal.type);
        writechar(' ');
        PrintLiteral(ctx, node_curr().literal.type, node_curr().literal.value);
        writechar('\n');
        break;
    case VISMUT_AST_IDENTIFIER:
        writef(str_fmt " ", str_2fmt(node_curr().identifier.name));
        PrintType(ctx, node_curr().identifier.type);
        writechar('\n');
        break;
    case VISMUT_AST_BINARY:
        write(ASTBinaryNodeType_String(node_curr().binary.op));
        writechar(' ');
        PrintType(ctx, node_curr().binary.type);
        writechar('\n');
        write(get_indent(indent));
        write("|> left\n");
        PrintNode(ctx, node_curr().binary.left, indent + 1);
        write(get_indent(indent));
        write("|> right\n");
        PrintNode(ctx, node_curr().binary.right, indent + 1);
        break;
    case VISMUT_AST_UNARY:
        write(ASTUnaryNodeType_String(node_curr().unary.op));
        writechar(' ');
        PrintType(ctx, node_curr().unary.type);
        writechar('\n');
        write(get_indent(indent));
        write("|> right\n");
        PrintNode(ctx, node_curr().unary.right, indent + 1);
        break;
    case VISMUT_AST_TYPE_CAST:
        PrintType(ctx, node_curr().type_cast.from_type);
        write(" -> ");
        PrintType(ctx, node_curr().type_cast.to_type);
        writechar('\n');
        PrintNode(ctx, node_curr().type_cast.argument, indent + 1);
        break;
    case VISMUT_AST_CONDITION:
        PrintType(ctx, node_curr().condition.type);
        writechar('\n');
        PrintNode(ctx, node_curr().condition.condition, indent);
        write(get_indent(indent));
        write("|> then\n");
        PrintNode(ctx, node_curr().condition.then, indent + 1);
        if (node_curr().condition.else_ != ASTNodeIdx_None) {
            write(get_indent(indent));
            write("|> else\n");
            PrintNode(ctx, node_curr().condition.else_, indent + 1);
        }
        break;
    case VISMUT_AST_BLOCK: {
        PrintType(ctx, node_curr().block.type);
        writechar('\n');
        ASTNodeIdx curr_expr = node_ptr(idx).block.first_expression;
        while (!ASTNodeIdx_IsNone(curr_expr)) {
            PrintNode(ctx, curr_expr, indent + 1);
            curr_expr = node_ptr(curr_expr).expression.next_expr;
        }
    } break;
    case VISMUT_AST_FN_CALL:
        if (node_curr().fn_call.fn_signature != NULL) {
            PrintType(ctx, node_curr().fn_call.fn_signature->function.return_type);
        } else {
            write("<no signature>");
        }
        writef(space str_fmt "(", str_2fmt(node_curr().fn_call.name));
        if (node_curr().fn_call.arguments_count > 0) {
            writechar('\n');
        }
        for (ASTNodeIdx *arg = node_curr().fn_call.arguments;
             arg < node_curr().fn_call.arguments + node_curr().fn_call.arguments_count; ++arg) {
            PrintNode(ctx, *arg, indent + 1);
        }
        if (node_curr().fn_call.arguments_count > 0) {
            write(get_indent(indent));
        }
        write(")\n");
        break;
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
