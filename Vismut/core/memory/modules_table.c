#include "modules_table.h"
#include "../io/file_reader.h"
#include "../parser/ast_parser.h"
#include "arena.h"
#include "raw_vector.h"
#include "symbol_table.h"

VismutModulesTable VismutModulesTable_Create(void) {
    return (VismutModulesTable){
        .arena = Arena_Create(),
        .buckets = RawVector_Create(),
        .entries = RawVector_Create(),
    };
}

VismutErrorType VismutModulesTable_Init(VismutModulesTable *restrict table,
                                        VismutErrorDetails *restrict out_details) {
    VismutErrorType err;
    SAFE_RISKY_EXPRESSION(RawVector_Init(&table->entries, out_details), err);
    SAFE_RISKY_EXPRESSION(RawVector_InitZero(&table->buckets, out_details), err);
    table->buckets.size = table->buckets.capacity;

    return VISMUT_OK;
}

void VismutModulesTable_Destroy(VismutModulesTable *table) {
    RawVector_Free(&table->entries);
    RawVector_Free(&table->buckets);
    Arena_Destroy(&table->arena);
}

ImportsVector ImportsVector_Create(void) {
    return (ImportsVector){
        .imports = NULL,
        .size = 0,
        .capacity = 0,
    };
}

VismutErrorType ImportsVector_Realloc(ImportsVector *restrict vec, Arena *restrict arena,
                                      const u32 new_capacity,
                                      VismutErrorDetails *restrict out_details) {
    VismutErrorType err = VISMUT_OK;

    if (new_capacity <= vec->capacity) {
        return VISMUT_OK;
    }

    vec->capacity = new_capacity;

    vec->imports = Arena_Array(arena, VismutModuleIdx, new_capacity, &err, out_details);
    if (err != VISMUT_OK) {
        return err;
    }

    return VISMUT_OK;
}

VismutErrorType VismutModuleEntry_AddImport(VismutModulesTable *restrict modules_table,
                                            VismutModuleEntry *restrict module_entry,
                                            const VismutModuleIdx import_idx,
                                            VismutErrorDetails *restrict out_details) {
    VismutErrorType err;

    if (module_entry->imports.size >= module_entry->imports.capacity) {
        const u32 new_capacity =
            module_entry->imports.capacity == 0 ? 4 : module_entry->imports.capacity * 2;
        SAFE_RISKY_EXPRESSION(ImportsVector_Realloc(&module_entry->imports, &modules_table->arena,
                                                    new_capacity, out_details),
                              err);
    }

    module_entry->imports.imports[module_entry->imports.size++] = import_idx;
    return VISMUT_OK;
}

attribute_pure VismutModuleIdx VismutModulesTable_Find(const VismutModulesTable *table,
                                                       const StringView name) {
    const u32 hash = StringView_Hash(name);
    const u32 bucket_idx = hash % RawVector_Count(&table->buckets, VismutModuleNode *);
    const VismutModuleNode *current_node =
        RawVector_At(&table->buckets, VismutModuleNode *, bucket_idx);

    while (current_node != NULL) {
        if (current_node->hash == hash && StringView_Equals(current_node->name, name)) {
            return current_node->index;
        }
        current_node = current_node->next;
    }

    return VismutModuleIdx_None;
}

VismutErrorType VismutModulesTable_Add(VismutModulesTable *restrict table, StringView name,
                                       StringView file_path, VismutErrorInfo *restrict error_info,
                                       VismutModuleIdx *restrict out_idx,
                                       VismutErrorDetails *restrict out_details) {
    VismutErrorType err;

    if (RawVector_Count(&table->entries, VismutModuleEntry) >=
        RawVector_Capacity(&table->entries, VismutModuleEntry)) {
        SAFE_RISKY_EXPRESSION(RawVector_Grow(&table->entries, VismutModuleEntry, 1, out_details),
                              err);
    }

    const u32 new_idx = RawVector_Count(&table->entries, VismutModuleEntry);
    RawVector_AddCount(&table->entries, VismutModuleEntry, 1);

    VismutModuleEntry *entry = &RawVector_At(&table->entries, VismutModuleEntry, new_idx);
    entry->name = name;
    entry->file_path = file_path;
    entry->arena = Arena_Create();
    entry->state = VISMUT_MODULE_STATE_NEW;
    entry->root_idx = ASTNodeIdx_None;
    entry->error_info = error_info;
    entry->source = StringView_Empty();
    entry->symbols = SymbolsTable_Create(name);
    entry->tokenizer = VismutTokenizer_Create(entry->source, NULL, error_info);
    entry->ast_builder = ASTBuilder_Create(&entry->tokenizer);
    entry->imports = ImportsVector_Create();

    SAFE_RISKY_EXPRESSION(SymbolsTable_Init(&entry->symbols, out_details), err);
    SAFE_RISKY_EXPRESSION(ImportsVector_Realloc(&entry->imports, &entry->arena, 4, out_details),
                          err);

    const u32 hash = StringView_Hash(name);
    const u32 bucket_idx = hash % RawVector_Count(&table->buckets, VismutModuleNode *);

    VismutModuleNode *new_node = Arena_Type(&table->arena, VismutModuleNode, &err, out_details);
    if (err != VISMUT_OK) {
        return err;
    }

    new_node->name = name;
    new_node->hash = hash;
    new_node->index = new_idx;

    new_node->next = RawVector_At(&table->buckets, VismutModuleNode *, bucket_idx);
    RawVector_At(&table->buckets, VismutModuleNode *, bucket_idx) = new_node;

    *out_idx = new_idx;
    return VISMUT_OK;
}

VismutErrorType VismutModule_LoadAndParse(VismutModulesTable *restrict table,
                                          const VismutModuleIdx idx,
                                          VismutTypeContext *restrict type_context,
                                          VismutErrorDetails *restrict out_details) {
    VismutErrorType err;
    VismutModuleEntry *mod = VismutModulesTable_ModuleAt(table, idx);

    if (mod->state != VISMUT_MODULE_STATE_NEW) {
        *out_details = (VismutErrorDetails){.module_name = mod->name};
        return VISMUT_ERR_MODULE_ALREADY_LOADED;
    }

    mod->state = VISMUT_MODULE_STATE_PARSING;

    StringView file_content;
    SAFE_RISKY_EXPRESSION(
        FileReader_ReadText(mod->file_path, &mod->arena, &file_content, out_details), err);

    mod->source = file_content;
    mod->tokenizer = VismutTokenizer_Create(file_content, &mod->arena, mod->error_info);

    ASTParser parser = ASTParser_Create(&mod->ast_builder, type_context, &mod->symbols, table, idx);
    SAFE_RISKY_EXPRESSION(ASTParser_Parse(&parser), err);

    mod->root_idx = parser.module_node;

    mod->state = VISMUT_MODULE_STATE_PARSED;
    return VISMUT_OK;
}

VismutErrorType VismutModulesTable_FindOrLoad(VismutModulesTable *restrict table, StringView name,
                                              StringView file_path,
                                              VismutTypeContext *restrict type_context,
                                              VismutErrorInfo *restrict error_info,
                                              VismutModuleIdx *restrict out_idx,
                                              VismutErrorDetails *restrict out_details) {
    VismutErrorType err;
    const VismutModuleIdx existing_idx = VismutModulesTable_Find(table, name);

    if (!VismutModuleIdx_IsNone(existing_idx)) {
        VismutModuleEntry *mod = VismutModulesTable_ModuleAt(table, existing_idx);

        if (mod->state == VISMUT_MODULE_STATE_PARSING) {
            *out_details = (VismutErrorDetails){.module_name = name};
            return VISMUT_ERR_CIRCULAR_DEPENDENCY;
        }

        *out_idx = existing_idx;
        return VISMUT_OK;
    }

    VismutModuleIdx new_idx;
    SAFE_RISKY_EXPRESSION(
        VismutModulesTable_Add(table, name, file_path, error_info, &new_idx, out_details), err);

    SAFE_RISKY_EXPRESSION(VismutModule_LoadAndParse(table, new_idx, type_context, out_details),
                          err);

    *out_idx = new_idx;
    return VISMUT_OK;
}

static VismutErrorType DFS_Visit(VismutModulesTable *restrict table, const VismutModuleIdx idx,
                                 u8 *restrict visited, VismutModuleIdx *restrict init_order,
                                 u32 *restrict current_length,
                                 VismutErrorDetails *restrict out_details) {
    VismutErrorType err;

    if (visited[idx]) {
        return VISMUT_OK;
    }

    visited[idx] = 1;

    VismutModuleEntry *mod = VismutModulesTable_ModuleAt(table, idx);

    for (u32 i = 0; i < mod->imports.size; i++) {
        VismutModuleIdx import_idx = mod->imports.imports[i];
        SAFE_RISKY_EXPRESSION(
            DFS_Visit(table, import_idx, visited, init_order, current_length, out_details), err);
    }

    init_order[*current_length] = idx;
    (*current_length)++;

    return VISMUT_OK;
}

VismutErrorType VismutModulesTable_GetInitOrder(VismutModulesTable *restrict table,
                                                const VismutModuleIdx main_idx,
                                                VismutModuleIdx **restrict out_init_order,
                                                u32 *restrict out_init_order_length,
                                                VismutErrorDetails *restrict out_details) {
    VismutErrorType err = VISMUT_OK;
    const u32 modules_count = VismutModulesTable_GetModulesCount(table);

    Arena temp_arena = Arena_Create();

    u8 *visited = Arena_Array(&temp_arena, u8, modules_count, &err, out_details);
    if (err != VISMUT_OK) {
        Arena_Destroy(&temp_arena);
        return err;
    }
    __builtin_memset(visited, 0, modules_count);

    VismutModuleIdx *init_order =
        Arena_Array(&table->arena, VismutModuleIdx, modules_count, &err, out_details);
    if (err != VISMUT_OK) {
        Arena_Destroy(&temp_arena);
        return err;
    }

    u32 current_length = 0;

    err = DFS_Visit(table, main_idx, visited, init_order, &current_length, out_details);
    Arena_Destroy(&temp_arena);

    if (err != VISMUT_OK) {
        return err;
    }

    *out_init_order = init_order;
    *out_init_order_length = current_length;

    return VISMUT_OK;
}
