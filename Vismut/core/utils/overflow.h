#ifndef VISMUT_CORE_UTILS_OVERFLOW_H
#define VISMUT_CORE_UTILS_OVERFLOW_H
#include "../memory/type_context.h"

attribute_const i1 IsNumberOverflowed(VismutSimpleValue value, VismutTypeKind kind);

attribute_const i1 IsTokenizerNumberOverflowed(u64 value, VismutIntSuffix suffix);

attribute_pure i1 IsCleanStrU64Overflow(StringView clean_str, NumberBase base);

#endif
