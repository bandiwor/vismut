#include "string_pool.h"
#include "../functions.h"
#include "arena.h"
#include <assert.h>

attribute_pure static u32 HashString(const StringView str) {
    u32 hash = 2166136261u;
    for (u64 i = 0; i < str.length; i++) {
        hash ^= str.data[i];
        hash *= 16777619u;
    }
    return hash;
}

StringPool StringPool_Create(void) {
    return (StringPool){0};
}

attribute_nonnull(1) VismutErrorType StringPool_Init(StringPool *pool, u32 capacity) {
    assert(capacity > 0 && "capacity must be > 0!");

    pool->arena = Arena_Create();
    pool->capacity = capacity;
    pool->count = 0;

    pool->buckets = Arena_Array(&pool->arena, StringNode *, capacity);
    if (!pool->buckets) {
        return VISMUT_ERROR_OUT_OF_MEMORY;
    }

    __builtin_memset(pool->buckets, 0, sizeof(StringNode *) * capacity);

    return VISMUT_ERROR_OK;
}

attribute_nonnull(1) StringNode *StringPool_Find(const StringPool *restrict pool,
                                                 const StringView view) {
    if (view.length == 0) {
        return NULL;
    }

    const u32 hash = HashString(view);
    const u32 index = hash % pool->capacity;

    StringNode *node = pool->buckets[index];
    while (node) {
        if (node->hash == hash && node->length == view.length) {
            if (Vismut_CmpStringViewWithCString(view, (const char *)node->data, node->length)) {
                return node;
            }
        }
        node = node->next;
    }

    return NULL;
}

attribute_nonnull(1, 3) VismutErrorType StringPool_Intern(StringPool *pool, const StringView view,
                                                          StringNode *restrict *restrict out_node) {
    if (view.length > UINT32_MAX) {
        return VISMUT_ERROR_STRING_TOO_LONG;
    }

    StringNode *existing_node = StringPool_Find(pool, view);
    if (existing_node != NULL) {
        *out_node = existing_node;
        return VISMUT_ERROR_OK;
    }

    const u32 hash = HashString(view);
    const u32 index = hash % pool->capacity;

    // For temporaly c-string compability
    const u64 alloc_size = sizeof(StringNode) + view.length + 1;
    StringNode *new_node = Arena_AllocateAligned(&pool->arena, alloc_size, sizeof(u8));
    if (new_node == NULL) {
        return VISMUT_ERROR_OUT_OF_MEMORY;
    }

    new_node->hash = hash;
    new_node->length = (u32)view.length;
    __builtin_memcpy(new_node->data, view.data, view.length);
    new_node->data[view.length] = '\0';

    new_node->next = pool->buckets[index];
    pool->buckets[index] = new_node;

    pool->count++;
    *out_node = new_node;

    return VISMUT_ERROR_OK;
}
