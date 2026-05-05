#include "overflow.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

attribute_const i1 IsNumberOverflowed(const VismutSimpleValue value, const VismutTypeKind kind) {
    switch (kind) {
    case VISMUT_TYPE_KIND_I1:
        return (u64)value.i > 1;
    case VISMUT_TYPE_KIND_I8:
        return value.i > INT8_MAX || value.i < INT8_MIN;
    case VISMUT_TYPE_KIND_I16:
        return value.i > INT16_MAX || value.i < INT16_MIN;
    case VISMUT_TYPE_KIND_I32:
        return value.i > INT32_MAX || value.i < INT32_MIN;
    case VISMUT_TYPE_KIND_I64:
        return 0;
    case VISMUT_TYPE_KIND_U8:
        return value.u > UINT8_MAX;
    case VISMUT_TYPE_KIND_U16:
        return value.u > UINT16_MAX;
    case VISMUT_TYPE_KIND_U32:
        return value.u > UINT32_MAX;
    case VISMUT_TYPE_KIND_U64:
        return 0;
    default:
        assert(0 && "Unreachable!");
        return 0;
    }
}

attribute_const i1 IsTokenizerNumberOverflowed(const u64 value, const VismutIntSuffix suffix) {
    switch (suffix) {
    case VISMUT_INT_SUFFIX_I1:
        return value > 1;
    case VISMUT_INT_SUFFIX_I8:
        return value > INT8_MAX + 1;
    case VISMUT_INT_SUFFIX_I16:
        return value > INT16_MAX + 1;
    case VISMUT_INT_SUFFIX_I32:
        return value > (u64)((u64)INT32_MAX + 1);
    case VISMUT_INT_SUFFIX_I64:
        return value > (u64)((u64)INT64_MAX + 1);
    case VISMUT_INT_SUFFIX_U8:
        return value > UINT8_MAX;
    case VISMUT_INT_SUFFIX_U16:
        return value > UINT16_MAX;
    case VISMUT_INT_SUFFIX_U32:
        return value > UINT32_MAX;
    case VISMUT_INT_SUFFIX_U64:
    default:
        return 0;
    }
}

attribute_pure i1 IsCleanStrU64Overflow(StringView clean_str, NumberBase base) {
    const u64 len = clean_str.length;
    if (len == 0)
        return 0;

    switch (base) {
    case NB_BIN:
        return len > 64;

    case NB_HEX:
        return len > 16;

    case NB_OCT:
        if (len > 22)
            return 1;
        if (len == 22)
            return clean_str.data[0] > '1';
        return 0;

    case NB_DEC:
        if (len > 20)
            return 1;
        if (len == 20) {
            const char *max_u64 = "18446744073709551615";
            for (int i = 0; i < 20; i++) {
                if (clean_str.data[i] > max_u64[i])
                    return 1;
                if (clean_str.data[i] < max_u64[i])
                    return 0;
            }
            return 0;
        }
        return 0;

    default:
        return 1;
    }
}
