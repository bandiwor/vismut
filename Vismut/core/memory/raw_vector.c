#include "raw_vector.h"
#include "memory.h"

RawVector RawVector_Create(void) {
    return (RawVector){
        .memory = NULL,
        .size = 0,
        .capacity = 0,
    };
}

VismutErrorType RawVector_InitWithCapacity(RawVector *restrict vector, const u32 capacity,
                                           VismutErrorDetails *restrict out_details) {
    void *block = mmap_allocate(capacity);
    if (unlikely(block == NULL)) {
        *out_details = (VismutErrorDetails){
            .oom =
                {
                    .location =
                        StringView_FromCStr(INTERNAL_ERROR_LOCATION_TEMPLATE("RawVector_Alloc")),
                    .bytes_required = capacity,
                },
        };
        return VISMUT_ERR_OOM;
    }

    vector->memory = block;
    vector->capacity = capacity;
    return VISMUT_OK;
}

VismutErrorType RawVector_Init(RawVector *restrict vector,
                               VismutErrorDetails *restrict out_details) {
    return RawVector_InitWithCapacity(vector, 4096, out_details);
}

VismutErrorType RawVector_InitZeroWithCapacity(RawVector *vector, u32 capacity,
                                               VismutErrorDetails *restrict out_details) {
    VismutErrorType err;
    SAFE_RISKY_EXPRESSION(RawVector_InitWithCapacity(vector, capacity, out_details), err);
    __builtin_memset(vector->memory, 0, capacity);
    return VISMUT_OK;
}

VismutErrorType RawVector_InitZero(RawVector *restrict vector,
                                   VismutErrorDetails *restrict out_details) {
    return RawVector_InitZeroWithCapacity(vector, 4096, out_details);
}

void RawVector_Free(RawVector *restrict vector) {
    if (vector->memory) {
        mmap_deallocate(vector->memory, vector->capacity);
        vector->memory = NULL;
    }
}

VismutErrorType RawVector_Realloc(RawVector *restrict vector, const u32 new_capacity,
                                  VismutErrorDetails *restrict out_details) {
    void *new_block = mmap_allocate(new_capacity);
    if (unlikely(new_block == NULL)) {
        *out_details = (VismutErrorDetails){
            .oom =
                {
                    .location =
                        StringView_FromCStr(INTERNAL_ERROR_LOCATION_TEMPLATE("RawVector_Realloc")),
                    .bytes_required = new_capacity,
                },
        };
        return VISMUT_ERR_OOM;
    }

    const u32 size = vector->size;
    if (likely(size > 0)) {
        __builtin_memcpy(new_block, vector->memory, size);
    }
    RawVector_Free(vector);

    vector->capacity = new_capacity;
    vector->memory = new_block;

    return VISMUT_OK;
}

VismutErrorType RawVector_GrowBytes(RawVector *restrict vector, const u32 additional_bytes,
                                    VismutErrorDetails *restrict out_details) {
    u32 new_capacity = vector->capacity == 0 ? 4096 : vector->capacity * 2;
    if (new_capacity < vector->size + additional_bytes) {
        new_capacity = ALIGN_UP(vector->size + additional_bytes, 4096);
    }

    return RawVector_Realloc(vector, new_capacity, out_details);
}
