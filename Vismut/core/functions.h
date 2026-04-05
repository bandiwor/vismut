#ifndef VISMUT_CORE_FUNCTIONS_H
#define VISMUT_CORE_FUNCTIONS_H
#include "defines.h"
#include "types.h"

static attribute_always_inline inline void
Vismut_MemoryCopy(void *restrict dest, const void *const restrict src, const u64 bytes) {
    __builtin_memcpy(dest, src, bytes);
}

// Return 1 if equals, else 0
static attribute_pure inline int Vismut_CmpStringViewWithCString(const StringView buffer,
                                                                 const char *restrict string,
                                                                 const u64 string_length) {
    return string_length == buffer.length &&
           __builtin_strncmp((const char *)buffer.data, string, buffer.length) == 0;
}

#endif
