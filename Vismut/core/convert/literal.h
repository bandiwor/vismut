#ifndef VISMUT_CORE_CONVERT_LITERAL_H
#define VISMUT_CORE_CONVERT_LITERAL_H
#include "../errors/errors.h"
#include "../memory/type_context.h"

VismutErrorType narrow_literal(VismutSimpleValue from_value, VismutTypeKind from_type,
                               VismutTypeKind to_type, VismutSimpleValue *out_value);

#endif
