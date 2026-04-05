#ifndef VISMUT_CORE_AST_AST_PRINTER_H
#define VISMUT_CORE_AST_AST_PRINTER_H

#include "ast.h"
#include <stdio.h>

typedef struct {
    const ASTNode *const nodes;
    const u64 nodes_count;
    FILE *out;
    int enable_colors;
} ASTPrinter;

ASTPrinter ASTPrinter_Create(const ASTNode *const nodes, u64 nodes_count, FILE *out,
                             int enable_colors);

void ASTPrinter_Print(ASTPrinter *printer, ASTNodeIdx idx);

#endif
