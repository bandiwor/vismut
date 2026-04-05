#ifndef VISMUT_CORE_MEMORY_ARENA_H
#define VISMUT_CORE_MEMORY_ARENA_H
#include "../defines.h"
#include "../types.h"
#include <sys/cdefs.h>

typedef struct ArenaBlock ArenaBlock;

typedef struct {
    ArenaBlock *current;
} Arena;

Arena Arena_Create(void);

attribute_nonnull(1) void Arena_Destroy(Arena *arena);

attribute_alloc_align(3) void *Arena_AllocateAligned(Arena *arena, const u64 size,
                                                     const u64 alignment);

#define Arena_Type(arena, type) (type *)Arena_AllocateAligned(arena, sizeof(type), _Alignof(type))

#define Arena_Array(arena, type, count)                                                            \
    (type *)Arena_AllocateAligned(arena, sizeof(type) * (count), _Alignof(type))

#endif
