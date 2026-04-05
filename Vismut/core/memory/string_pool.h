#ifndef VISMUT_CORE_MEMORY_STRING_POOL_H
#define VISMUT_CORE_MEMORY_STRING_POOL_H
#include "../errors/errors.h"
#include "arena.h"

typedef struct StringNode StringNode;

struct StringNode {
    u32 hash;
    u32 length;
    StringNode *next;
    u8 data[];
};

typedef struct {
    Arena arena;
    StringNode **buckets;
    u32 capacity;
    u32 count;
} StringPool;

StringPool StringPool_Create();

attribute_nonnull(1) VismutErrorType StringPool_Init(StringPool *pool, u32 capacity);

attribute_nonnull(1, 3) VismutErrorType StringPool_Intern(StringPool *pool, const StringView view,
                                                          StringNode *restrict *restrict out_node);

#endif
