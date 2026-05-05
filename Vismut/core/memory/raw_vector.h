#ifndef VISMUT_CORE_MEMORY_RAW_VECTOR_H
#define VISMUT_CORE_MEMORY_RAW_VECTOR_H
#include "../defines.h"
#include "../errors/error_details.h"
#include "../errors/errors.h"
#include "../types.h"

typedef struct {
    void *memory;
    u32 size;
    u32 capacity;
} RawVector;

RawVector RawVector_Create(void);

VismutErrorType RawVector_Init(RawVector *restrict vector,
                               VismutErrorDetails *restrict out_details);

VismutErrorType RawVector_InitWithCapacity(RawVector *restrict vector, u32 capacity,
                                           VismutErrorDetails *restrict out_details);

VismutErrorType RawVector_InitZero(RawVector *restrict vector,
                                   VismutErrorDetails *restrict out_details);

VismutErrorType RawVector_InitZeroWithCapacity(RawVector *vector, u32 capacity,
                                               VismutErrorDetails *restrict out_details);

void RawVector_Free(RawVector *restrict vector);

VismutErrorType RawVector_Realloc(RawVector *restrict vector, u32 new_capacity,
                                  VismutErrorDetails *restrict out_details);

VismutErrorType RawVector_GrowBytes(RawVector *restrict vector, const u32 additional_bytes,
                                    VismutErrorDetails *restrict out_details);

VismutErrorType RawVector_PushBytes(RawVector *restrict vector, const void *restrict data, u32 size,
                                    VismutErrorDetails *restrict out_details);

#define RawVector_MemoryAligned(vec_ptr) assume_aligned((vec_ptr)->memory, 4096)

#define RawVector_Elements(vec_ptr, type) ((type *)RawVector_MemoryAligned(vec_ptr))

#define RawVector_At(vec_ptr, type, index) (((type *)(RawVector_MemoryAligned(vec_ptr)))[index])

#define RawVector_Count(vec_ptr, type) ((vec_ptr)->size / sizeof(type))

#define RawVector_AddCount(vec_ptr, type, count) ((vec_ptr)->size += count * sizeof(type))

#define RawVector_Capacity(vec_ptr, type) ((vec_ptr)->capacity / sizeof(type))

#define RawVector_Grow(vec_ptr, type, count, out_details)                                          \
    RawVector_GrowBytes(vec_ptr, sizeof(type) * count, out_details)

#define RawVector_PushBytes(vec_ptr, data, data_size, err, out_details)                            \
    do {                                                                                           \
        if (unlikely((vec_ptr)->size + data_size > (vec_ptr)->capacity)) {                         \
            SAFE_RISKY_EXPRESSION(RawVector_GrowBytes(vec_ptr, data_size, out_details), err);      \
        }                                                                                          \
        u8 *memory = assume_aligned((vec_ptr)->memory, 4096);                                      \
        __builtin_memcpy(memory + (vec_ptr)->size, data, data_size);                               \
        (vec_ptr)->size += data_size;                                                              \
    } while (0)

#define RawVector_Push(vec_ptr, type, value, err, out_details)                                     \
    do {                                                                                           \
        err = VISMUT_OK;                                                                           \
        type __tmp_val = (value);                                                                  \
        RawVector_PushBytes((vec_ptr), &__tmp_val, sizeof(type), err, (out_details));              \
    } while (0)

#endif
