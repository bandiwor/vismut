#ifndef VISMUT_CORE_MEMORY_CONSTANT_POOL_H
#define VISMUT_CORE_MEMORY_CONSTANT_POOL_H
#include "../types.h"
#include "arena.h"
#include "raw_vector.h"

typedef enum {
    CONSTANT_NODE_INT,
    CONSTANT_NODE_UINT,
    CONSTANT_NODE_FLOAT,
    CONSTANT_NODE_STRING,
    CONSTANT_NODE_ARRAY,
} ConstantPoolValueType;

typedef struct {
    u8 *data;
    u32 length;
} ConstantPoolString;

typedef struct ConstantPoolValue ConstantPoolValue;

typedef struct {
    ConstantPoolValue *data;
    u32 length;
} ConstantPoolArray;

struct ConstantPoolValue {
    ConstantPoolValueType type;
    union {
        i64 int_;
        u64 uint_;
        f64 float_;
        ConstantPoolString string;
        ConstantPoolArray array;
    };
};

typedef struct ConstantPoolNode ConstantPoolNode;

struct ConstantPoolNode {
    ConstantPoolNode *next;
    ConstantPoolIdx index;
    u32 hash;
};

typedef struct {
    Arena arena;
    RawVector buckets_vector;
    RawVector elements_vector;
} ConstantPool;

ConstantPool ConstantPool_Create(void);

VismutErrorType ConstantPool_Init(ConstantPool *restrict pool,
                                  VismutErrorDetails *restrict out_details);

VismutErrorType ConstantPool_InternInt(ConstantPool *restrict pool, i64 value,
                                       ConstantPoolIdx *restrict out_idx,
                                       VismutErrorDetails *restrict out_details);

VismutErrorType ConstantPool_InternUInt(ConstantPool *restrict pool, u64 value,
                                        ConstantPoolIdx *restrict out_idx,
                                        VismutErrorDetails *restrict out_details);

VismutErrorType ConstantPool_InternFloat(ConstantPool *restrict pool, f64 value,
                                         ConstantPoolIdx *out_idx,
                                         VismutErrorDetails *restrict out_details);

VismutErrorType ConstantPool_InternString(ConstantPool *restrict pool, StringView str,
                                          ConstantPoolIdx *out_idx,
                                          VismutErrorDetails *restrict out_details);

VismutErrorType ConstantPool_InternArray(ConstantPool *restrict pool, ConstantPoolArray array,
                                         ConstantPoolIdx *restrict out_idx,
                                         VismutErrorDetails *restrict out_details);

#endif
