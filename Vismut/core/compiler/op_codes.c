#include "op_codes.h"

const u8 *VismutOpcode_String(const VismutOpcode type) {
#define X(name, text, fmt) [name] = (const u8 *)text,
    static const u8 *const codes_table[] = {X_VISMUT_OPCODES(X)};
#undef X

    if (unlikely(type >= OP_COUNT || type < 0)) {
        return codes_table[OP_UNKNOWN];
    }

    return codes_table[type];
}

OpcodeFormat VismutOpcode_GetFormat(const VismutOpcode type) {
#define X(name, text, fmt) [name] = fmt,
    static OpcodeFormat codes_table[] = {X_VISMUT_OPCODES(X)};
#undef X

    if (unlikely(type >= OP_COUNT || type < 0)) {
        return codes_table[OP_UNKNOWN];
    }

    return codes_table[type];
}
