#ifndef VISMUT_CORE_AST_AST_PRINTER_H
#define VISMUT_CORE_AST_AST_PRINTER_H
#include "../ast/ast.h"
#include "../ast/ast_builder.h"
#include <stdio.h>

typedef struct {
    const ASTBuilder *nodes;
    const u64 nodes_count;
    FILE *out;
    int enable_colors;
} ASTPrinter;

ASTPrinter ASTPrinter_Create(const ASTBuilder *const nodes, FILE *out, int enable_colors);

void ASTPrinter_Print(ASTPrinter *printer, ASTNodeIdx idx);

#endif
