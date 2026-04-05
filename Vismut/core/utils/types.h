#ifndef VISMUT_CORE_UTILS_TYPES_H
#define VISMUT_CORE_UTILS_TYPES_H
#include "../ast/value.h"
#include "../tokenizer/token.h"

const VismutType *VismutType_FromTypeToken(const VismutTypeContext *ctx, VismutTokenType);

#endif
