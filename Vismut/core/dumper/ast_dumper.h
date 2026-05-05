#ifndef VISMUT_CORE_DUMPER_AST_DUMPER_H
#define VISMUT_CORE_DUMPER_AST_DUMPER_H
#include "../ast/ast_builder.h"

typedef enum {
    AST_DUMPER_NONE = 0,
    AST_DUMPER_PRETTY = 1 << 0,
    AST_DUMPER_DEBUG = 1 << 1,
} ASTDumperMode;

typedef struct {
    u32 mode;
} ASTDumperCtx;

typedef struct {
    ASTBuilder *builder;
    u8 *str;
    u32 str_allocated;
    u32 str_length;
    ASTDumperCtx ctx;
} ASTDumper;

ASTDumper ASTDumper_Create(ASTBuilder *restrict nodes, u32 mode);

void ASTDumper_Reset(ASTDumper *restrict dumper);

StringView ASTDumper_Dump(ASTDumper *restrict dumper, ASTNodeIdx idx);

void ASTDumper_Destroy(ASTDumper *restrict dumper);

#endif
