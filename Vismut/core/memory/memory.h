#ifndef VISMUT_CORE_MEMORY_MEMORY_H
#define VISMUT_CORE_MEMORY_MEMORY_H
#include "../types.h"

void *mmap_allocate(u64 size);

void mmap_deallocate(void *ptr, u64 allocated_size);

#endif
