#include "../defines.h"
#include "../tokenizer/token.h"

attribute_pure attribute_nodiscard VismutIntSuffix ParseIntSuffix(const StringView buffer);

attribute_pure attribute_nodiscard VismutFloatSuffix ParseFloatSuffix(const StringView buffer);
