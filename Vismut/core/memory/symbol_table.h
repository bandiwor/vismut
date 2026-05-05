#ifndef VISMUT_CORE_MEMORY_SYMBOL_TABLE_H
#define VISMUT_CORE_MEMORY_SYMBOL_TABLE_H
#include "../ast/symbol.h"
#include "../types.h"
#include "arena.h"
#include "raw_vector.h"
#include <stdint.h>

typedef struct SymbolNode SymbolNode;

struct SymbolNode {
    SymbolNode *next;
    VismutSymbol *symbol;
    u32 hash;
};

typedef struct {
    StringView module_name;
    Arena arena;
    RawVector buckets;
    u32 non_local_entries_count;
} SymbolsTable;

SymbolsTable SymbolsTable_Create(StringView module_name);

VismutErrorType SymbolsTable_Init(SymbolsTable *restrict table,
                                  VismutErrorDetails *restrict out_details);

void SymbolsTable_Free(SymbolsTable *table);

VismutErrorType SymbolsTable_AllocateLocalSymbol(SymbolsTable *restrict table,
                                                 const VismutSymbol entry,
                                                 VismutSymbol **restrict out_symbol,
                                                 VismutErrorDetails *restrict out_details);

VismutErrorType SymbolsTable_InsertSymbol(SymbolsTable *restrict table, const VismutSymbol entry,
                                          VismutSymbol **restrict out_symbol,
                                          VismutErrorDetails *restrict out_details);

VismutSymbol *SymbolsTable_FindSymbol(SymbolsTable *restrict table, const StringView name,
                                      u32 name_hash);

#define SymbolsTable_EntryAt(table, idx) &RawVector_At(&(table)->entries, SymbolEntry, idx)

VismutSymbol SymbolEntry_CreateGlobalVariable(const StringView name, const VismutType *type,
                                              i1 is_exported, u32 global_index, i1 is_mutable);

VismutSymbol SymbolEntry_CreateLocalVariable(const StringView name, const VismutType *type,
                                             u8 register_index, i1 is_mutable);

VismutSymbol SymbolEntry_CreateConstant(StringView name, const VismutType *type, i1 is_exported,
                                        ConstantPoolIdx cp_idx);

VismutSymbol SymbolEntry_CreateFunction(const StringView name, const VismutType *signature,
                                        i1 is_exported, ASTNodeIdx declaration_node,
                                        u32 bytecode_offset, u8 registers_needed);

VismutSymbol SymbolEntry_CreateModuleRef(const StringView name, const VismutType *signature,
                                         VismutModuleIdx module_idx);

#endif
