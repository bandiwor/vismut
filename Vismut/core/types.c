#include "types.h"
#include "defines.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

StringView StringView_Empty(void) {
    return (StringView){
        .data = NULL,
        .length = 0,
    };
}

int StringView_Equals(const StringView a, const StringView b) {
    if (a.length != b.length) {
        return 0;
    }
    if (a.data == b.data) {
        return 1;
    }

    return __builtin_memcmp(a.data, b.data, a.length) == 0;
}

StringView StringView_FromCStr(const char *string) {
    return (StringView){
        .data = (u8 *)string,
        .length = __builtin_strlen(string),
    };
}

StringView StringView_Clone(const StringView str) {
    u8 *memory = malloc(str.length);
    if (unlikely(memory == NULL)) {
        return StringView_Empty();
    }

    memcpy(memory, str.data, str.length);

    return (StringView){
        .data = memory,
        .length = str.length,
    };
}

attribute_pure u32 StringView_Hash(const StringView view) {
    u32 hash = 2166136261u;
    for (u64 i = 0; i < view.length; i++) {
        hash ^= view.data[i];
        hash *= 16777619;
    }
    return hash;
}

int StringView_IsEmpty(const StringView view) {
    return view.data == NULL || view.length == 0;
}

void StringView_Write(const StringView view, FILE *file) {
    if (likely(!StringView_IsEmpty(view))) {
        fwrite(view.data, sizeof(u8), view.length / sizeof(u8), file);
    }
}

attribute_const Position Position_Create(const u32 offset, const u32 length) {
    return (Position){
        .offset = offset,
        .length = length,
    };
}

attribute_const Position Position_FromSubView(const StringView parent, const StringView child) {
    assert(child.data >= parent.data);
    assert(child.data + child.length <= parent.data + parent.length);

    return (Position){.offset = (u32)(child.data - parent.data), .length = (u32)child.length};
}
