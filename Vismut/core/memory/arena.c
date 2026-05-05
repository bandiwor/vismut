#include "arena.h"
#include "memory.h"

struct ArenaBlock {
    u64 size;
    u64 used;
    ArenaBlock *next;
};

#define ARENA_BLOCK_PAGE_SIZE 4096
#define ARENA_BLOCK_DATA(block) ((u8 *)(block) + sizeof(ArenaBlock))

static VismutErrorType ArenaBlock_Create(ArenaBlock *restrict *restrict out_block,
                                         const u64 required_size,
                                         VismutErrorDetails *restrict out_details) {
    const u64 header_size = sizeof(ArenaBlock);
    const u64 total_needed = required_size + header_size;

    const u64 total_to_alloc = (total_needed > ARENA_BLOCK_PAGE_SIZE)
                                   ? ALIGN_UP(total_needed, ARENA_BLOCK_PAGE_SIZE)
                                   : ARENA_BLOCK_PAGE_SIZE;

    void *ptr = mmap_allocate(total_to_alloc);
    if (unlikely(ptr == NULL)) {
        *out_details =
            (VismutErrorDetails){.oom = {
                                     .location = StringView_FromCStr(
                                         INTERNAL_ERROR_LOCATION_TEMPLATE("ArenaBlock_Create")),
                                     .bytes_required = total_to_alloc,
                                 }};
        return VISMUT_ERR_OOM;
    }

    ArenaBlock *block = (ArenaBlock *)ptr;

    block->size = total_to_alloc - header_size;
    block->used = 0;
    block->next = NULL;

    *out_block = block;
    return VISMUT_OK;
}

void ArenaBlock_FreePage(const ArenaBlock *const block) {
    mmap_deallocate((void *)block, block->size + sizeof(ArenaBlock));
}

Arena Arena_Create(void) {
    return (Arena){
        .current = NULL,
    };
}

attribute_nonnull(1) void Arena_Destroy(Arena *arena) {
    if (arena->current == NULL) {
        return;
    }

    const ArenaBlock *current = arena->current;
    while (current != NULL) {
        const ArenaBlock *next = current->next;
        ArenaBlock_FreePage(current);
        current = next;
    }

    arena->current = NULL;
}

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

attribute_alloc_align(3)
    attribute_nonnull(1) void *Arena_AllocateAligned(Arena *restrict arena, const u64 size,
                                                     const u64 alignment,
                                                     VismutErrorType *restrict out_error,
                                                     VismutErrorDetails *restrict out_details) {
    if (arena->current != NULL) {
        const u64 curr_addr = (u64)(ARENA_BLOCK_DATA(arena->current) + arena->current->used);
        const u64 aligned_addr = ALIGN_UP(curr_addr, alignment);
        const u64 new_used = (aligned_addr - (u64)ARENA_BLOCK_DATA(arena->current)) + size;

        if (new_used <= arena->current->size) {
            arena->current->used = new_used;
            *out_error = VISMUT_OK;
            return (void *)aligned_addr;
        }
    }

    ArenaBlock *new_block = NULL;
    VismutErrorType err;
    if ((err = ArenaBlock_Create(&new_block, size + alignment, out_details)) != VISMUT_OK) {
        *out_error = err;
        return NULL;
    }

    new_block->next = arena->current;
    arena->current = new_block;

    const u64 curr_addr = (u64)ARENA_BLOCK_DATA(new_block);
    const u64 aligned_addr = ALIGN_UP(curr_addr, alignment);
    new_block->used = (aligned_addr - (u64)ARENA_BLOCK_DATA(new_block)) + size;

    *out_error = VISMUT_OK;
    return (void *)aligned_addr;
}
