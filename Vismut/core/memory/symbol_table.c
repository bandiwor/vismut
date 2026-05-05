#include "symbol_table.h"
#include "arena.h"
#include "raw_vector.h"
#include <stddef.h>
#include <string.h>
#include <strings.h>

SymbolsTable SymbolsTable_Create(StringView module_name) {
    return (SymbolsTable){
        .module_name = module_name,
        .arena = Arena_Create(),
        .buckets = RawVector_Create(),
        .non_local_entries_count = 0,
    };
}

VismutErrorType SymbolsTable_Init(SymbolsTable *restrict table,
                                  VismutErrorDetails *restrict out_details) {
    VismutErrorType err;
    SAFE_RISKY_EXPRESSION(RawVector_InitZero(&table->buckets, out_details), err);
    table->buckets.size = table->buckets.capacity;

    return VISMUT_OK;
}

void SymbolsTable_Free(SymbolsTable *table) {
    RawVector_Free(&table->buckets);
    Arena_Destroy(&table->arena);
}

static VismutErrorType rehash_buckets(SymbolsTable *restrict table,
                                      const u32 new_buckets_capacity_bytes,
                                      VismutErrorDetails *restrict out_details) {
    const u32 old_count = RawVector_Count(&table->buckets, SymbolNode *);
    SymbolNode **old_buckets = RawVector_Elements(&table->buckets, SymbolNode *);

    RawVector new_buckets_vec = RawVector_Create();
    VismutErrorType err;

    SAFE_RISKY_EXPRESSION(
        RawVector_InitZeroWithCapacity(&new_buckets_vec, new_buckets_capacity_bytes, out_details),
        err);
    new_buckets_vec.size = new_buckets_vec.capacity;

    SymbolNode **new_buckets = RawVector_Elements(&new_buckets_vec, SymbolNode *);
    const u32 new_count = RawVector_Count(&new_buckets_vec, SymbolNode *);

    for (u32 i = 0; i < old_count; ++i) {
        SymbolNode *current = old_buckets[i];

        while (current != NULL) {
            SymbolNode *next = current->next;

            const u32 new_bucket_idx = current->hash % new_count;
            current->next = new_buckets[new_bucket_idx];
            new_buckets[new_bucket_idx] = current;

            current = next;
        }
    }

    RawVector_Free(&table->buckets);
    table->buckets = new_buckets_vec;

    return VISMUT_OK;
}

VismutSymbol *SymbolsTable_FindSymbol(SymbolsTable *restrict table, const StringView name,
                                      const u32 name_hash) {
    const size_t num_buckets = RawVector_Count(&table->buckets, SymbolNode *);
    if (num_buckets == 0) {
        return NULL;
    }

    const u32 bucket_idx = name_hash % num_buckets;
    SymbolNode **buckets = RawVector_Elements(&table->buckets, SymbolNode *);

    SymbolNode *current = buckets[bucket_idx];
    while (current != NULL) {
        if (current->hash == name_hash && StringView_Equals(current->symbol->name, name)) {
            return current->symbol;
        }
        current = current->next;
    }

    return NULL;
}

VismutErrorType SymbolsTable_InsertSymbol(SymbolsTable *restrict table, const VismutSymbol entry,
                                          VismutSymbol **restrict out_symbol,
                                          VismutErrorDetails *restrict out_details) {
    const u32 name_hash = StringView_Hash(entry.name);
    VismutErrorType err = VISMUT_OK;

    VismutSymbol *existing_symbol = SymbolsTable_FindSymbol(table, entry.name, name_hash);
    if (unlikely(existing_symbol != NULL)) {
        *out_details = (VismutErrorDetails){
            .symbol_name = entry.name,
        };
        return VISMUT_ERR_NAME_CONFLICT;
    }

    const u32 current_entries_count = table->non_local_entries_count;
    const u32 current_buckets_count = RawVector_Count(&table->buckets, SymbolNode *);

    if (current_buckets_count == 0 || (current_entries_count * 4 >= current_buckets_count * 3)) {
        const u32 current_capacity_bytes = table->buckets.size;
        const u32 new_capacity_bytes =
            current_capacity_bytes == 0 ? 4096 : current_capacity_bytes * 2;

        SAFE_RISKY_EXPRESSION(rehash_buckets(table, new_capacity_bytes, out_details), err);
    }

    VismutSymbol *new_symbol = Arena_Type(&table->arena, VismutSymbol, &err, out_details);
    if (unlikely(new_symbol == NULL)) {
        return err;
    }

    *new_symbol = entry;

    SymbolNode *new_node = Arena_Type(&table->arena, SymbolNode, &err, out_details);
    if (unlikely(new_node == NULL)) {
        return err;
    }

    new_node->symbol = new_symbol;
    new_node->hash = name_hash;

    const u32 buckets_count = RawVector_Count(&table->buckets, SymbolNode *);
    const u32 bucket_idx = name_hash % buckets_count;

    SymbolNode **buckets = RawVector_Elements(&table->buckets, SymbolNode *);
    new_node->next = buckets[bucket_idx];
    buckets[bucket_idx] = new_node;

    table->non_local_entries_count++;

    *out_symbol = new_symbol;
    return VISMUT_OK;
}

VismutErrorType SymbolsTable_AllocateLocalSymbol(SymbolsTable *restrict table,
                                                 const VismutSymbol entry,
                                                 VismutSymbol **restrict out_symbol,
                                                 VismutErrorDetails *restrict out_details) {
    VismutErrorType err = VISMUT_OK;

    VismutSymbol *new_symbol = Arena_Type(&table->arena, VismutSymbol, &err, out_details);
    if (unlikely(new_symbol == NULL)) {
        return err;
    }

    *new_symbol = entry;
    *out_symbol = new_symbol;

    return VISMUT_OK;
}

VismutSymbol SymbolEntry_CreateFunction(const StringView name, const VismutType *signature,
                                        const i1 is_exported, const ASTNodeIdx declaration_node,
                                        const u32 bytecode_offset, const u8 registers_needed) {
    return (VismutSymbol){
        .name = name,
        .kind = VISMUT_SYMBOL_FUNCTION,
        .type = signature,
        .is_exported = is_exported,
        .as.func =
            {
                .declaration_node = declaration_node,
                .bytecode_offset = bytecode_offset,
                .registers_needed = registers_needed,
            },
    };
}

VismutSymbol SymbolEntry_CreateGlobalVariable(const StringView name, const VismutType *type,
                                              const i1 is_exported, const u32 global_index,
                                              const i1 is_mutable) {
    return (VismutSymbol){
        .name = name,
        .type = type,
        .kind = VISMUT_SYMBOL_GLOBAL_VAR,
        .is_exported = is_exported,
        .as.global_var =
            {
                .is_mutable = is_mutable,
                .global_index = global_index,
            },
    };
}

VismutSymbol SymbolEntry_CreateLocalVariable(const StringView name, const VismutType *type,
                                             const u8 register_index, const i1 is_mutable) {
    return (VismutSymbol){
        .name = name,
        .type = type,
        .kind = VISMUT_SYMBOL_LOCAL_VAR,
        .is_exported = 0,
        .as.local_var =
            {
                .is_mutable = is_mutable,
                .register_index = register_index,
            },
    };
}

VismutSymbol SymbolEntry_CreateConstant(const StringView name, const VismutType *type,
                                        const i1 is_exported, const ConstantPoolIdx cp_idx) {
    return (VismutSymbol){
        .name = name,
        .type = type,
        .kind = VISMUT_SYMBOL_CONSTANT,
        .is_exported = is_exported,
        .as.constant =
            {
                .cp_idx = cp_idx,
            },
    };
}

VismutSymbol SymbolEntry_CreateModuleRef(const StringView name, const VismutType *type,
                                         const VismutModuleIdx module_idx) {
    return (VismutSymbol){.name = name,
                          .type = type,
                          .kind = VISMUT_SYMBOL_MODULE_REF,
                          .is_exported = 0,
                          .as.module_ref = {
                              .idx = module_idx,
                          }};
}
