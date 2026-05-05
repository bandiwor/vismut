#ifndef VISMUT_CORE_TYPES_H
#define VISMUT_CORE_TYPES_H
#include "defines.h"
#include <stdint.h>
#include <stdio.h>

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;
typedef _Bool i1;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef float f32;
typedef double f64;

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef struct {
    u8 *data;
    u64 length;
} StringView;

StringView StringView_Empty(void);

int StringView_Equals(StringView a, StringView b);

attribute_pure StringView StringView_FromCStr(const char *string);

StringView StringView_Clone(const StringView str);

attribute_pure u32 StringView_Hash(const StringView view);

int StringView_IsEmpty(const StringView view);

void StringView_Write(const StringView view, FILE *file);

typedef struct {
    u32 offset;
    u32 length;
} Position;

attribute_const Position Position_FromSubView(const StringView parent, const StringView child);

attribute_const Position Position_Create(u32 offset, u32 length);

typedef u32 VismutModuleIdx;
#define VismutModuleIdx_None UINT32_MAX
#define VismutModuleIdx_IsNone(idx) (VismutModuleIdx_None == (idx))

typedef u32 SymbolIdx;
#define SymbolIdx_None UINT32_MAX
#define SymbolIdx_IsNone(idx) ((idx) == SymbolIdx_None)

typedef u32 ASTNodeIdx;
#define ASTNodeIdx_None UINT32_MAX
#define ASTNodeIdx_IsNone(node_idx) (ASTNodeIdx_None == (node_idx))

typedef u16 ConstantPoolIdx;
#define ConstantPoolIdx_None UINT16_MAX
#define ConstantPoolIdx_IsNone(node_idx) (ConstantPoolIdx_None == (node_idx))

#endif
