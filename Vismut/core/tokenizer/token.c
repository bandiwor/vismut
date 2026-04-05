#include "token.h"

attribute_const const u8 *VismutTokenType_String(const VismutTokenType type) {
#define X(name, text) [name] = (const u8 *)text,
    static const u8 *const codes_table[] = {X_VISMUT_TOKENS(X)};
#undef X

    if (unlikely(type >= VISMUT_TOKEN_COUNT || type < 0)) {
        return codes_table[VISMUT_TOKEN_UNKNOWN];
    }

    return codes_table[type];
}

attribute_const const u8 *VismutIntSuffix_String(VismutIntSuffix type) {
#define X(name, text) [name] = (const u8 *)text,
    static const u8 *const codes_table[] = {X_INT_SUFFIX(X)};
#undef X

    if (unlikely(type >= VISMUT_INT_SUFFIX_COUNT || type < 0)) {
        return codes_table[VISMUT_INT_SUFFIX_UNKNOWN];
    }

    return codes_table[type];
}

attribute_const const u8 *VismutFloatSuffix_String(const VismutFloatSuffix type) {
#define X(name, text) [name] = (const u8 *)text,
    static const u8 *const codes_table[] = {X_FLOAT_SUFFIX(X)};
#undef X

    if (unlikely(type >= VISMUT_FLOAT_SUFFIX_COUNT || type < 0)) {
        return codes_table[VISMUT_FLOAT_SUFFIX_UNKNOWN];
    }

    return codes_table[type];
}
