#include "type.h"

const u8 *VismutTypeKind_String(const VismutTypeKind type) {
#define X(name, text) [name] = (const u8 *)text,
    static const u8 *const codes_table[] = {X_VISMUT_TYPE_KIND(X)};
#undef X

    if (unlikely(type >= VISMUT_TYPE_KIND_COUNT || type < 0)) {
        return codes_table[VISMUT_TYPE_KIND_UNKNOWN];
    }

    return codes_table[type];
}
