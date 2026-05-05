#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef _Bool i1;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef void unit;

typedef uintptr_t usize;

typedef float f32;
typedef double f64;

#ifndef NULL
#define NULL ((void *)0)
#endif
#define CONCAT_HELPER(a, b) a##b
#define CONCAT(a, b) CONCAT_HELPER(a, b)
#define METHOD_HELPER(obj, method) obj##_##method
#define METHOD(obj, method) METHOD_HELPER(obj, method)
#define temp_var(idx) vsmt_tmp_##idx
#define Vec(T) CONCAT(Vec_, T)

#define VECTOR_LARGE_TYPE_THRESHOLD 64

#define VecDef(T)                                                                                  \
    typedef struct {                                                                               \
        T *data;                                                                                   \
        u32 length;                                                                                \
        u32 capacity;                                                                              \
    } Vec(T);                                                                                      \
                                                                                                   \
    Vec(T) METHOD(Vec(T), New)() {                                                                 \
        return (Vec(T)){                                                                           \
            .data = NULL,                                                                          \
            .length = 0,                                                                           \
            .capacity = 0,                                                                         \
        };                                                                                         \
    }                                                                                              \
                                                                                                   \
    Vec(T) METHOD(Vec(T), WithCapacity)(const u32 capacity) {                                      \
        T *data = malloc(sizeof(T) * capacity);                                                    \
        if (data == NULL) {                                                                        \
            exit(-1);                                                                              \
        }                                                                                          \
        return (Vec(T)){                                                                           \
            .data = data,                                                                          \
            .length = 0,                                                                           \
            .capacity = capacity,                                                                  \
        };                                                                                         \
    }                                                                                              \
                                                                                                   \
    unit METHOD(Vec(T), Destroy)(Vec(T) * self) {                                                  \
        if (self->data != NULL) {                                                                  \
            free(self->data);                                                                      \
            self->data = NULL;                                                                     \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    void METHOD(Vec(T), Extend)(Vec(T) * self, const u32 new_capacity) {                           \
        if (new_capacity < self->capacity) {                                                       \
            return;                                                                                \
        }                                                                                          \
        T *new_data = realloc(self->data, sizeof(T) * new_capacity);                               \
        if (new_data == NULL) {                                                                    \
            exit(-1);                                                                              \
        }                                                                                          \
        self->data = new_data;                                                                     \
        self->capacity = new_capacity;                                                             \
    }                                                                                              \
                                                                                                   \
    u32 METHOD(Vec(T), GrowStrategy)(Vec(T) * self, const u32 old_capacity) {                      \
        if (sizeof(T) <= VECTOR_LARGE_TYPE_THRESHOLD) {                                            \
            return old_capacity == 0 ? 8 : old_capacity * 2;                                       \
        } else {                                                                                   \
            return old_capacity == 0 ? 2 : old_capacity * 2;                                       \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    unit METHOD(Vec(T), Push)(Vec(T) * self, const T element) {                                    \
        if (self->length >= self->capacity) {                                                      \
            METHOD(Vec(T), Extend)(self, METHOD(Vec(T), GrowStrategy)(self, self->capacity));      \
        }                                                                                          \
        self->data[self->length++] = element;                                                      \
    }                                                                                              \
                                                                                                   \
    T *METHOD(Vec(T), At)(Vec(T) * self, const u32 idx) {                                          \
        if (idx >= self->length) {                                                                 \
            exit(-1);                                                                              \
        }                                                                                          \
        return &self->data[idx];                                                                   \
    }

int main(void) {
    return 0;
}
