#ifndef VISMUT_CORE_UTILS_ISSOMECHAR_H
#define VISMUT_CORE_UTILS_ISSOMECHAR_H
#include "../defines.h"
#include "../types.h"

attribute_const attribute_nodiscard static inline int IsHexDigit(const u8 c) {
    return (c >= '0' && c <= '9') | ((c | 0x20) >= 'a' && (c | 0x20) <= 'f');
}

attribute_const attribute_nodiscard static inline int IsOctDigit(const u8 c) {
    return c >= '0' && c <= '7';
}

attribute_const attribute_nodiscard static inline int IsDecDigit(const u8 c) {
    return c >= '0' && c <= '9';
}

attribute_const attribute_nodiscard static inline int IsBinDigit(const u8 c) {
    return c == '0' || c == '1';
}

attribute_always_inline attribute_const attribute_nodiscard static inline int IsSpace(const u8 c) {
    return (c == ' ') || (c >= '\t' && c <= '\r');
}

attribute_always_inline attribute_const attribute_nodiscard static inline int
IsValidIdentifier(const u8 c) {
    return ((u8)((c | 0x20) - 'a') <= 25) || ((u8)(c - '0') <= 9) || (c == '_');
}

attribute_always_inline attribute_const attribute_nodiscard static inline int
IsValidIdentifierStart(const u8 c) {
    return ((u8)((c | 0x20) - 'a') <= 25) || (c == '_');
}

#endif
