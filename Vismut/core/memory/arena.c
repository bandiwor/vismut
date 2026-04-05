#include "arena.h"
#include "../defines.h"
#include "../errors/errors.h"
#include "memory.h"
#include <stdlib.h>

#define ARENA_BLOCK_PAGE_SIZE 4096

struct ArenaBlock {
    void *memory;
    u64 size;
    u64 used;
    ArenaBlock *next;
};

static VismutErrorType ArenaBlock_Create(ArenaBlock **out_block, const u64 required_size) {
    const u64 header_size = sizeof(ArenaBlock);
    const u64 total_needed = required_size + header_size;
    const u64 total_to_alloc = (total_needed > ARENA_BLOCK_PAGE_SIZE)
                                   ? ALIGN_UP(total_needed, ARENA_BLOCK_PAGE_SIZE)
                                   : ARENA_BLOCK_PAGE_SIZE;

    const void *ptr = mmap_allocate(total_to_alloc);

    if (unlikely(ptr == NULL)) {
        return VISMUT_ERROR_OUT_OF_MEMORY;
    }

    ArenaBlock *block = (ArenaBlock *)ptr;

    block->size = total_to_alloc - header_size;
    block->used = 0;
    block->next = NULL;
    block->memory = (u8 *)ptr + header_size;

    *out_block = block;
    return VISMUT_ERROR_OK;
}

void ArenaBlock_FreePage(const ArenaBlock *const block) {
    mmap_deallocate(block->memory, block->size + sizeof(ArenaBlock));
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
    while (current->next != NULL) {
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
    attribute_nonnull(1) void *Arena_AllocateAligned(Arena *arena, const u64 size,
                                                     const u64 alignment) {
    if (arena->current != NULL) {
        const u64 curr_addr = (u64)((u8 *)arena->current->memory + arena->current->used);
        const u64 aligned_addr = ALIGN_UP(curr_addr, alignment);
        const u64 new_used = (aligned_addr - (u64)arena->current->memory) + size;

        if (new_used <= arena->current->size) {
            arena->current->used = new_used;
            return (void *)aligned_addr;
        }
    }

    ArenaBlock *new_block = NULL;
    if (ArenaBlock_Create(&new_block, size) != VISMUT_ERROR_OK) {
        exit(EXIT_FAILURE);
        return NULL;
    }

    new_block->next = arena->current;
    arena->current = new_block;

    const u64 curr_addr = (u64)new_block->memory;
    const u64 aligned_addr = ALIGN_UP(curr_addr, alignment);
    new_block->used = (aligned_addr - (u64)new_block->memory) + size;

    return (void *)aligned_addr;
}
