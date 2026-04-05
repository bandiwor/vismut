#include "str_to_int.h"

attribute_pure u64 VismutConvert_BinStrToUInt(const StringView str) {
    const u8 *curr = str.data;
    const u8 *const end = str.data + str.length;

    u64 result = 0;
    while (curr < end) {
        result = (result << 1) | (*curr++ - '0');
    }

    return result;
}

attribute_pure u64 VismutConvert_OctStrToUInt(const StringView str) {
    const u8 *curr = str.data;
    const u8 *const end = str.data + str.length;

    u64 result = 0;
    while (curr < end) {
        result = (result << 3) | (*curr++ - '0');
    }

    return result;
}

attribute_pure u64 VismutConvert_DecStrToUInt(const StringView str) {
    const u8 *curr = str.data;
    const u8 *const end = str.data + str.length;

    u64 result = 0;
    while (curr < end) {
        result = result * 10 + (*curr++ - '0');
    }

    return result;
}

attribute_pure u64 VismutConvert_HexStrToUInt(const StringView str) {
    const u8 *curr = str.data;
    const u8 *const end = str.data + str.length;

    u64 result = 0;
    while (curr < end) {
        const u8 c = *curr++;
        const u32 is_letter = (c >> 6) & 1;
        const u32 val = (c & 0xF) + (is_letter * 9);

        result = (result << 4) | val;
    }

    return result;
}
