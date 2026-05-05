#ifndef VISMUT_CORE_MEMORY_MODULES_TABLE_H
#define VISMUT_CORE_MEMORY_MODULES_TABLE_H
#include "../ast/ast_builder.h"
#include "../memory/symbol_table.h"
#include "raw_vector.h"
#include <stdint.h>

typedef struct {
    VismutModuleIdx *imports;
    u32 size;
    u32 capacity;
} ImportsVector;

typedef enum {
    VISMUT_MODULE_STATE_NEW,
    VISMUT_MODULE_STATE_PARSING,
    VISMUT_MODULE_STATE_PARSED,
    VISMUT_MODULE_STATE_ANALYZED,
    VISMUT_MODULE_STATE_COMPILED,
} VismutModuleState;

typedef struct {
    StringView name;
    StringView file_path;
    StringView source;
    Arena arena;
    SymbolsTable symbols;
    VismutTokenizer tokenizer;
    ASTBuilder ast_builder;
    ASTNodeIdx root_idx;
    VismutModuleState state;
    ImportsVector imports;
    VismutErrorInfo *error_info;
} VismutModuleEntry;

typedef struct VismutModuleNode VismutModuleNode;

struct VismutModuleNode {
    VismutModuleNode *next;
    StringView name;
    u32 hash;
    VismutModuleIdx index;
};

typedef struct {
    Arena arena;
    RawVector buckets;
    RawVector entries;
} VismutModulesTable;

VismutModulesTable VismutModulesTable_Create(void);

VismutErrorType VismutModulesTable_Init(VismutModulesTable *restrict table,
                                        VismutErrorDetails *restrict out_details);

void VismutModulesTable_Destroy(VismutModulesTable *table);

#define VismutModulesTable_GetModulesCount(table)                                                  \
    RawVector_Count(&(table)->entries, VismutModuleEntry)

attribute_pure VismutModuleIdx VismutModulesTable_Find(const VismutModulesTable *table,
                                                       const StringView name);

#define VismutModulesTable_ModuleAt(table, idx)                                                    \
    (&RawVector_At(&(table)->entries, VismutModuleEntry, idx))

VismutErrorType VismutModulesTable_FindOrLoad(VismutModulesTable *restrict table, StringView name,
                                              StringView file_path,
                                              VismutTypeContext *restrict type_context,
                                              VismutErrorInfo *restrict error_info,
                                              VismutModuleIdx *restrict out_idx,
                                              VismutErrorDetails *restrict out_details);

VismutErrorType VismutModuleEntry_AddImport(VismutModulesTable *restrict modules_table,
                                            VismutModuleEntry *restrict module_entry,
                                            const VismutModuleIdx import_idx,
                                            VismutErrorDetails *restrict out_details);

VismutErrorType VismutModulesTable_GetInitOrder(VismutModulesTable *restrict table,
                                                const VismutModuleIdx main_idx,
                                                VismutModuleIdx **restrict out_init_order,
                                                u32 *restrict out_init_order_length,
                                                VismutErrorDetails *restrict out_details);

#endif
