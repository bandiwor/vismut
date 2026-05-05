#ifndef VISMUT_CORE_TOKENIZER_TOKENIZER_H
#define VISMUT_CORE_TOKENIZER_TOKENIZER_H
#include "../errors/errors.h"
#include "../memory/arena.h"
#include "token.h"

typedef struct {
    Arena *arena;
    const u8 *cursor;
    const u8 *limit;
    const u8 *token_start;
    const u8 *start;
    VismutErrorInfo *error_info;
} VismutTokenizer;

attribute_nodiscard VismutTokenizer VismutTokenizer_Create(const StringView source, Arena *restrict,
                                                           VismutErrorInfo *restrict);

attribute_nonnull(1) void VismutTokenizer_Reset(VismutTokenizer *);

attribute_nonnull(1, 2) attribute_nodiscard VismutErrorType
    VismutTokenizer_Next(VismutTokenizer *restrict, VismutToken *restrict);

#endif
