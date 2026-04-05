#include "suffix.h"
#include "../functions.h"

attribute_pure attribute_nodiscard VismutIntSuffix ParseIntSuffix(const StringView buffer) {
    if (buffer.length == 2) {
        if (Vismut_CmpStringViewWithCString(buffer, "i8", 2)) {
            return VISMUT_INT_SUFFIX_I8;
        }
        if (Vismut_CmpStringViewWithCString(buffer, "u8", 2)) {
            return VISMUT_INT_SUFFIX_U8;
        }
        return VISMUT_INT_SUFFIX_UNKNOWN;
    } else if (buffer.length == 3) {
        if (Vismut_CmpStringViewWithCString(buffer, "i16", 3))
            return VISMUT_INT_SUFFIX_I16;
        if (Vismut_CmpStringViewWithCString(buffer, "i32", 3))
            return VISMUT_INT_SUFFIX_I32;
        if (Vismut_CmpStringViewWithCString(buffer, "i64", 3))
            return VISMUT_INT_SUFFIX_I64;
        if (Vismut_CmpStringViewWithCString(buffer, "u16", 3))
            return VISMUT_INT_SUFFIX_U16;
        if (Vismut_CmpStringViewWithCString(buffer, "u32", 3))
            return VISMUT_INT_SUFFIX_U32;
        if (Vismut_CmpStringViewWithCString(buffer, "u64", 3))
            return VISMUT_INT_SUFFIX_U64;
        return VISMUT_INT_SUFFIX_UNKNOWN;
    } else {
        return VISMUT_INT_SUFFIX_UNKNOWN;
    }
}

attribute_pure attribute_nodiscard VismutFloatSuffix ParseFloatSuffix(const StringView buffer) {
    if (buffer.length == 1 && buffer.data[0] == 'f') {
        return VISMUT_FLOAT_SUFFIX_NONE;
    } else if (buffer.length == 3) {
        if (Vismut_CmpStringViewWithCString(buffer, "f32", 3))
            return VISMUT_FLOAT_SUFFIX_F32;
        if (Vismut_CmpStringViewWithCString(buffer, "f64", 3))
            return VISMUT_FLOAT_SUFFIX_F64;
        return VISMUT_FLOAT_SUFFIX_UNKNOWN;
    } else {
        return VISMUT_FLOAT_SUFFIX_UNKNOWN;
    }
}
