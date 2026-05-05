#ifndef VISMUT_CORE_UTILS_SUFFIX_H
#define VISMUT_CORE_UTILS_SUFFIX_H
#include "../defines.h"
#include "../tokenizer/token.h"

attribute_pure VismutIntSuffix ParseIntSuffix(const StringView buffer);

attribute_pure VismutFloatSuffix ParseFloatSuffix(const StringView buffer);

#endif
