#include "str_to_int.h"

attribute_const static inline u32 ParseHex(const u8 c) {
    return ((c) & 0xF) + (((c) >> 6) & 1) * 9;
}

attribute_pure u64 VismutConvert_BinStrToUInt(const StringView str) {
    const u8 *curr = str.data;
    const u8 *const end = str.data + str.length;

    u64 result = 0;

    while (end - curr >= 8) {
        const u32 b0 = curr[0] - '0';
        const u32 b1 = curr[1] - '0';
        const u32 b2 = curr[2] - '0';
        const u32 b3 = curr[3] - '0';
        const u32 b4 = curr[4] - '0';
        const u32 b5 = curr[5] - '0';
        const u32 b6 = curr[6] - '0';
        const u32 b7 = curr[7] - '0';

        const u32 chunk =
            (b0 << 7) | (b1 << 6) | (b2 << 5) | (b3 << 4) | (b4 << 3) | (b5 << 2) | (b6 << 1) | b7;

        result = (result << 8) | chunk;
        curr += 8;
    }

    while (curr < end) {
        result = (result << 1) | (*curr++ - '0');
    }

    return result;
}

attribute_pure u64 VismutConvert_OctStrToUInt(const StringView str) {
    const u8 *curr = str.data;
    const u8 *const end = str.data + str.length;

    u64 result = 0;

    while (end - curr >= 4) {
        const u32 o0 = curr[0] - '0';
        const u32 o1 = curr[1] - '0';
        const u32 o2 = curr[2] - '0';
        const u32 o3 = curr[3] - '0';

        const u32 chunk = (o0 << 9) | (o1 << 6) | (o2 << 3) | o3;
        result = (result << 12) | chunk;
        curr += 4;
    }

    while (curr < end) {
        result = (result << 3) | (*curr++ - '0');
    }

    return result;
}

attribute_pure u64 VismutConvert_DecStrToUInt(const StringView str) {
    const u8 *curr = str.data;
    const u8 *const end = str.data + str.length;

    u64 result = 0;

    while (end - curr >= 4) {
        const u32 d0 = curr[0] - '0';
        const u32 d1 = curr[1] - '0';
        const u32 d2 = curr[2] - '0';
        const u32 d3 = curr[3] - '0';

        const u32 chunk = (d0 * 1000) + (d1 * 100) + (d2 * 10) + d3;

        result = (result * 10000) + chunk;
        curr += 4;
    }

    while (curr < end) {
        result = result * 10 + (*curr++ - '0');
    }

    return result;
}

attribute_pure u64 VismutConvert_HexStrToUInt(const StringView str) {
    const u8 *curr = str.data;
    const u8 *const end = str.data + str.length;

    u64 result = 0;

    while (end - curr >= 4) {
        const u64 h0 = ParseHex(curr[0]);
        const u64 h1 = ParseHex(curr[1]);
        const u64 h2 = ParseHex(curr[2]);
        const u64 h3 = ParseHex(curr[3]);

        const u64 chunk = (h0 << 12) | (h1 << 8) | (h2 << 4) | h3;
        result = (result << 16) | chunk;
        curr += 4;
    }

    while (curr < end) {
        result = (result << 4) | ParseHex(*curr++);
    }

    return result;
}
