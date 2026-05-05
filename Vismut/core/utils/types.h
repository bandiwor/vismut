#ifndef VISMUT_CORE_UTILS_TYPES_H
#define VISMUT_CORE_UTILS_TYPES_H
#include "../memory/type_context.h"
#include "../tokenizer/token.h"

attribute_const int VismutTypeKind_IsInt(const VismutTypeKind kind);

attribute_const int VismutTypeKind_IsFloat(const VismutTypeKind kind);

attribute_const int VismutTypeKind_IsUInt(const VismutTypeKind kind);

const VismutType *VismutType_FromTypeToken(const VismutTypeContext *ctx, VismutTokenType);

attribute_const VismutTypeKind VismutTypeKind_FromIntSuffix(VismutIntSuffix suffix);

#endif
