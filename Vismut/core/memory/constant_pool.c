#include "constant_pool.h"
#include "arena.h"
#include "raw_vector.h"

ConstantPool ConstantPool_Create(void) {
    return (ConstantPool){
        .arena = Arena_Create(),
        .buckets_vector = RawVector_Create(),
        .elements_vector = RawVector_Create(),
    };
}

VismutErrorType ConstantPool_Init(ConstantPool *pool, VismutErrorDetails *restrict out_details) {
    VismutErrorType err;
    SAFE_RISKY_EXPRESSION(RawVector_Init(&pool->elements_vector, out_details), err);
    SAFE_RISKY_EXPRESSION(RawVector_InitZero(&pool->buckets_vector, out_details), err);
    pool->buckets_vector.size = pool->buckets_vector.capacity;

    return VISMUT_OK;
}

attribute_pure static u32 hash_data(const void *data, const u32 length) {
    u32 hash = 2166136261u;
    const u8 *bytes = (const u8 *)data;
    for (const u8 *byte = bytes; byte < bytes + length; ++byte) {
        hash = (hash ^ *byte) * 16777619u;
    }
    return hash;
}

static u32 calculate_hash(const ConstantPoolValue *value) {
    switch (value->type) {
    case CONSTANT_NODE_INT:
        return hash_data(&value->int_, sizeof(value->int_));
    case CONSTANT_NODE_UINT:
        return hash_data(&value->uint_, sizeof(value->uint_));
    case CONSTANT_NODE_FLOAT:
        return hash_data(&value->float_, sizeof(value->float_));
    case CONSTANT_NODE_STRING:
        return hash_data(value->string.data, value->string.length);
    case CONSTANT_NODE_ARRAY: {
        u32 hash = 2166136261u;
        for (u32 i = 0; i < value->array.length; ++i) {
            const u32 elem_hash = calculate_hash(&value->array.data[i]);
            hash = (hash ^ elem_hash) * 16777619u;
        }
        return hash;
    }
    default:
        return 0;
    }
}

static int values_equal(const ConstantPoolValue *a, const ConstantPoolValue *b) {
    if (a->type != b->type)
        return 0;
    switch (a->type) {
    case CONSTANT_NODE_INT:
        return a->int_ == b->int_;
    case CONSTANT_NODE_UINT:
        return a->uint_ == b->uint_;
    case CONSTANT_NODE_FLOAT:
        return __builtin_memcmp(&a->float_, &b->float_, sizeof(a->float_)) == 0;
    case CONSTANT_NODE_STRING:
        if (a->string.length != b->string.length)
            return 0;
        return __builtin_memcmp(a->string.data, b->string.data, a->string.length) == 0;
    case CONSTANT_NODE_ARRAY:
        if (a->array.length != b->array.length)
            return 0;

        const ConstantPoolValue *element_a = &a->array.data[0];
        const ConstantPoolValue *element_b = &b->array.data[0];
        for (; element_a < a->array.data + a->array.length; ++element_a, ++element_b) {
            if (!values_equal(element_a, element_b)) {
                return 0;
            }
        }

        return 1;
    default:
        return 0;
    }
}

static VismutErrorType rehash_buckets(ConstantPool *restrict pool,
                                      const u32 new_buckets_capacity_bytes,
                                      VismutErrorDetails *restrict out_details) {
    const u32 old_count = pool->buckets_vector.size / sizeof(ConstantPoolNode *);
    ConstantPoolNode **old_buckets = (ConstantPoolNode **)pool->buckets_vector.memory;

    RawVector new_buckets_vec = RawVector_Create();
    VismutErrorType err;
    SAFE_RISKY_EXPRESSION(
        RawVector_InitZeroWithCapacity(&new_buckets_vec, new_buckets_capacity_bytes, out_details),
        err);
    new_buckets_vec.size = new_buckets_vec.capacity;

    ConstantPoolNode **new_buckets = (ConstantPoolNode **)new_buckets_vec.memory;
    const u32 new_count = new_buckets_capacity_bytes / sizeof(ConstantPoolNode *);

    for (u32 i = 0; i < old_count; ++i) {
        ConstantPoolNode *current = old_buckets[i];

        while (current != NULL) {
            ConstantPoolNode *next = current->next;

            const u32 new_bucket_idx = current->hash % new_count;
            current->next = new_buckets[new_bucket_idx];
            new_buckets[new_bucket_idx] = current;

            current = next;
        }
    }

    RawVector_Free(&pool->buckets_vector);
    pool->buckets_vector = new_buckets_vec;

    return VISMUT_OK;
}

static int check_existing_intern(ConstantPool *restrict pool, const ConstantPoolValue *restrict tmp,
                                 const u32 hash, ConstantPoolIdx *restrict out_idx) {
    const u32 buckets_count = pool->buckets_vector.size / sizeof(ConstantPoolNode *);
    if (buckets_count == 0) {
        return 0;
    }

    const u32 bucket_idx = hash % buckets_count;
    ConstantPoolNode **buckets = (ConstantPoolNode **)pool->buckets_vector.memory;
    ConstantPoolNode *current = buckets[bucket_idx];

    while (current) {
        if (current->hash == hash) {
            // Прямое чтение из памяти
            ConstantPoolValue *existing =
                &((ConstantPoolValue *)pool->elements_vector.memory)[current->index];
            if (values_equal(existing, tmp)) {
                *out_idx = current->index;
                return 1;
            }
        }
        current = current->next;
    }

    return 0;
}

static VismutErrorType insert_new_intern(ConstantPool *restrict pool,
                                         const ConstantPoolValue new_value, const u32 hash,
                                         ConstantPoolIdx *restrict out_idx,
                                         VismutErrorDetails *restrict out_details) {
    VismutErrorType err;

    // Прямой подсчет через size вместо вызова функций
    const u32 current_elements = pool->elements_vector.size / sizeof(ConstantPoolValue);
    const u32 current_buckets = pool->buckets_vector.size / sizeof(ConstantPoolNode *);

    if (current_buckets == 0 || (current_elements * 4 >= current_buckets * 3)) {
        const u32 current_capacity_bytes = pool->buckets_vector.size;
        const u32 new_capacity_bytes =
            current_capacity_bytes == 0 ? 4096 : current_capacity_bytes * 2;

        SAFE_RISKY_EXPRESSION(rehash_buckets(pool, new_capacity_bytes, out_details), err);
    }

    // Сохраняем индекс ПЕРЕД добавлением
    const u32 new_pool_idx = pool->elements_vector.size / sizeof(ConstantPoolValue);

    // Безопасное добавление через новый макрос
    RawVector_Push(&pool->elements_vector, ConstantPoolValue, new_value, err, out_details);
    if (unlikely(err != VISMUT_OK)) {
        return err;
    }

    ConstantPoolNode *new_node = Arena_Type(&pool->arena, ConstantPoolNode, &err, out_details);
    if (new_node == NULL) {
        return err;
    }

    new_node->index = new_pool_idx;
    new_node->hash = hash;

    const u32 buckets_count = pool->buckets_vector.size / sizeof(ConstantPoolNode *);
    if (buckets_count > 0) {
        const u32 bucket_idx = hash % buckets_count;
        ConstantPoolNode **buckets = (ConstantPoolNode **)pool->buckets_vector.memory;
        new_node->next = buckets[bucket_idx];
        buckets[bucket_idx] = new_node;
    }

    *out_idx = new_pool_idx;
    return VISMUT_OK;
}

VismutErrorType ConstantPool_InternInt(ConstantPool *restrict pool, const i64 value,
                                       ConstantPoolIdx *restrict out_idx,
                                       VismutErrorDetails *restrict out_details) {
    ConstantPoolValue tmp = {.type = CONSTANT_NODE_INT, .int_ = value};
    const u32 hash = calculate_hash(&tmp);

    if (check_existing_intern(pool, &tmp, hash, out_idx)) {
        return VISMUT_OK;
    }

    return insert_new_intern(pool, tmp, hash, out_idx, out_details);
}

VismutErrorType ConstantPool_InternUInt(ConstantPool *pool, const u64 value,
                                        ConstantPoolIdx *out_idx,
                                        VismutErrorDetails *restrict out_details) {
    ConstantPoolValue tmp = {.type = CONSTANT_NODE_UINT, .uint_ = value};
    const u32 hash = calculate_hash(&tmp);

    if (check_existing_intern(pool, &tmp, hash, out_idx)) {
        return VISMUT_OK;
    }

    return insert_new_intern(pool, tmp, hash, out_idx, out_details);
}

VismutErrorType ConstantPool_InternFloat(ConstantPool *pool, const f64 value,
                                         ConstantPoolIdx *out_idx,
                                         VismutErrorDetails *restrict out_details) {
    ConstantPoolValue tmp = {.type = CONSTANT_NODE_FLOAT, .float_ = value};
    const u32 hash = calculate_hash(&tmp);

    if (check_existing_intern(pool, &tmp, hash, out_idx)) {
        return VISMUT_OK;
    }

    return insert_new_intern(pool, tmp, hash, out_idx, out_details);
}

VismutErrorType ConstantPool_InternString(ConstantPool *pool, const StringView str,
                                          ConstantPoolIdx *out_idx,
                                          VismutErrorDetails *restrict out_details) {
    ConstantPoolValue tmp = {
        .type = CONSTANT_NODE_STRING,
        .string = (ConstantPoolString){.data = (u8 *)str.data, .length = str.length}};
    const u32 hash = calculate_hash(&tmp);

    if (check_existing_intern(pool, &tmp, hash, out_idx)) {
        return VISMUT_OK;
    }

    VismutErrorType err;
    u8 *heap_data = Arena_Array(&pool->arena, u8, str.length, &err, out_details);
    if (heap_data == NULL) {
        return err;
    }

    __builtin_memcpy(heap_data, str.data, str.length);

    return insert_new_intern(pool,
                             (ConstantPoolValue){
                                 .type = CONSTANT_NODE_STRING,
                                 .string =
                                     (ConstantPoolString){
                                         .data = heap_data,
                                         .length = str.length,
                                     },
                             },
                             hash, out_idx, out_details);
}

VismutErrorType ConstantPool_InternArray(ConstantPool *pool, const ConstantPoolArray array,
                                         ConstantPoolIdx *out_idx,
                                         VismutErrorDetails *restrict out_details) {
    ConstantPoolValue tmp = {.type = CONSTANT_NODE_ARRAY, .array = array};
    const u32 hash = calculate_hash(&tmp);

    if (check_existing_intern(pool, &tmp, hash, out_idx)) {
        return VISMUT_OK;
    }

    VismutErrorType err;
    ConstantPoolValue *heap_data =
        Arena_Array(&pool->arena, ConstantPoolValue, array.length, &err, out_details);
    if (heap_data == NULL) {
        return err;
    }

    for (u32 i = 0; i < array.length; ++i) {
        heap_data[i] = array.data[i];
    }

    ConstantPoolValue new_value = {.type = CONSTANT_NODE_ARRAY,
                                   .array = {.data = heap_data, .length = array.length}};

    return insert_new_intern(pool, new_value, hash, out_idx, out_details);
}
