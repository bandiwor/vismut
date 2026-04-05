#include "memory.h"
#include "../defines.h"
#include <sys/mman.h>
#include <unistd.h>

void *mmap_allocate(u64 size) {
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return likely(ptr != MAP_FAILED) ? ptr : NULL;
}

void mmap_deallocate(void *ptr, u64 allocated_size) {
    munmap(ptr, allocated_size);
}
