#ifndef VISMUT_CORE_MEMORY_ARENA_H
#define VISMUT_CORE_MEMORY_ARENA_H
#include "../defines.h"
#include "../errors/error_details.h"
#include "../errors/errors.h"

typedef struct ArenaBlock ArenaBlock;

typedef struct {
    ArenaBlock *current;
} Arena;

Arena Arena_Create(void);

attribute_nonnull(1) void Arena_Destroy(Arena *arena);

attribute_alloc_align(3) attribute_malloc
    void *Arena_AllocateAligned(Arena *arena, const u64 size, const u64 alignment,
                                VismutErrorType *restrict out_error,
                                VismutErrorDetails *restrict out_details);

#define Arena_Type(arena, type, out_error, out_details)                                            \
    (type *)Arena_AllocateAligned(arena, sizeof(type), _Alignof(type), out_error, out_details)

#define Arena_Array(arena, type, count, out_error, out_details)                                    \
    (type *)Arena_AllocateAligned(arena, sizeof(type) * (count), _Alignof(type), out_error,        \
                                  out_details)

#endif
