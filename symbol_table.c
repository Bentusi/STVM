#include "symbol_table.h"
#include "mmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 内部辅助函数声明 */
static uint32_t hash_string(const char *str);
static symbol_t* create_symbol(const char *name, symbol_type_t type, type_info_t *data_type);
static scope_t* create_scope(const char *name, scope_type_t type, uint32_t level);
static void add_symbol_to_scope(scope_t *scope, symbol_t *symbol);
static symbol_t* find_symbol_in_scope(scope_t *scope, const char *name);
static void init_builtin_types(symbol_table_t *table);

/* 全局内置类型实例 */
static type_info_t g_builtin_types[16];
static bool g_builtin_types_initialized = false;

/* ========== 生命周期管理 ========== */

symbol_table_t* symbol_table_create(void) {
    symbol_table_t *table = mmgr_alloc(sizeof(symbol_table_t));
    if (!table) {
        return NULL;
    }
    
    memset(table, 0, sizeof(symbol_table_t));
    return table;
}

int symbol_table_init(symbol_table_t *table) {
    if (!table) {
        return -1;
    }
    
    if (table->is_initialized) {
        return 0;
    }
    
    /* 初始化内置类型 */
    init_builtin_types(table);
    
    /* 创建全局作用域 */
    table->global_scope = create_scope("global", SCOPE_GLOBAL, 0);
    if (!table->global_scope) {
        return -1;
    }
    
    table->current_scope = table->global_scope;
    table->scope_stack[0] = table->global_scope;
    table->scope_depth = 0;
    
    table->is_initialized = true;
    return 0;
}

void symbol_table_destroy(symbol_table_t *table) {
    if (!table) {
        return;
    }
    
    /* 清理所有作用域和符号 */
    // 这里应该递归清理所有作用域中的符号
    
    mmgr_free(table);
}

bool symbol_table_is_initialized(symbol_table_t *table) {
    return table && table->is_initialized;
}

/* ========== 作用域管理 ========== */

int symbol_table_enter_scope(symbol_table_t *table, const char *scope_name, scope_type_t type) {
    if (!table || !table->is_initialized || !scope_name) {
        return -1;
    }
    
    if (table->scope_depth >= MAX_SCOPE_DEPTH - 1) {
        return -1; // 作用域层次过深
    }
    
    /* 创建新作用域 */
    scope_t *new_scope = create_scope(scope_name, type, table->scope_depth + 1);
    if (!new_scope) {
        return -1;
    }
    
    new_scope->parent = table->current_scope;
    
    /* 更新作用域栈 */
    table->scope_depth++;
    table->scope_stack[table->scope_depth] = new_scope;
    table->current_scope = new_scope;
    
    return 0;
}

int symbol_table_exit_scope(symbol_table_t *table) {
    if (!table || !table->is_initialized || table->scope_depth == 0) {
        return -1;
    }
    
    /* 恢复父作用域 */
    table->scope_depth--;
    table->current_scope = table->scope_stack[table->scope_depth];
    
    return 0;
}

scope_t* symbol_table_get_current_scope(symbol_table_t *table) {
    return table ? table->current_scope : NULL;
}

scope_t* symbol_table_get_global_scope(symbol_table_t *table) {
    return table ? table->global_scope : NULL;
}

/* ========== 符号定义 ========== */

symbol_t* symbol_table_define_variable(symbol_table_t *table, const char *name, 
                                       type_info_t *type, var_category_t category) {
    if (!table || !table->is_initialized || !name || !type) {
        return NULL;
    }
    
    /* 检查当前作用域中是否已存在同名符号 */
    if (find_symbol_in_scope(table->current_scope, name)) {
        return NULL; // 符号已存在
    }
    
    /* 创建变量符号 */
    symbol_t *symbol = create_symbol(name, SYMBOL_VARIABLE, type);
    if (!symbol) {
        return NULL;
    }
    
    symbol->info.var.category = category;
    symbol->scope_level = table->current_scope->level;
    symbol->address = table->current_scope->next_address;
    
    /* 更新地址 */
    table->current_scope->next_address += type->size;
    
    /* 添加到当前作用域 */
    add_symbol_to_scope(table->current_scope, symbol);
    table->total_symbols++;
    
    return symbol;
}

symbol_t* symbol_table_define_function(symbol_table_t *table, const char *name,
                                       type_info_t *return_type, void *implementation) {
    if (!table || !table->is_initialized || !name || !return_type) {
        return NULL;
    }
    
    /* 检查全局作用域中是否已存在同名函数 */
    if (find_symbol_in_scope(table->global_scope, name)) {
        return NULL; // 函数已存在
    }
    
    /* 创建函数符号 */
    symbol_t *symbol = create_symbol(name, SYMBOL_FUNCTION, return_type);
    if (!symbol) {
        return NULL;
    }
    
    symbol->info.func.implementation = implementation;
    symbol->info.func.param_count = return_type->compound.function_info.parameter_count;
    symbol->scope_level = 0; // 函数总是在全局作用域
    
    /* 添加到全局作用域 */
    add_symbol_to_scope(table->global_scope, symbol);
    table->total_symbols++;
    
    return symbol;
}

symbol_t* symbol_table_define_constant(symbol_table_t *table, const char *name,
                                       type_info_t *type, void *value) {
    if (!table || !table->is_initialized || !name || !type || !value) {
        return NULL;
    }
    
    /* 检查当前作用域中是否已存在同名符号 */
    if (find_symbol_in_scope(table->current_scope, name)) {
        return NULL; // 符号已存在
    }
    
    /* 创建常量符号 */
    symbol_t *symbol = create_symbol(name, SYMBOL_CONSTANT, type);
    if (!symbol) {
        return NULL;
    }
    
    symbol->info.constant.const_value = value;
    symbol->scope_level = table->current_scope->level;
    
    /* 添加到当前作用域 */
    add_symbol_to_scope(table->current_scope, symbol);
    table->total_symbols++;
    
    return symbol;
}

/* ========== 库符号注册（关键接口） ========== */

symbol_t* symbol_table_register_library_function(symbol_table_t *table, const char *name,
                                                 const char *qualified_name, type_info_t *type,
                                                 void *implementation, const char *library_name) {
    if (!table || !table->is_initialized || !name || !qualified_name || !type || !library_name) {
        return NULL;
    }
    
    /* 创建函数符号 */
    symbol_t *symbol = create_symbol(qualified_name, SYMBOL_FUNCTION, type);
    if (!symbol) {
        return NULL;
    }
    
    symbol->info.func.implementation = implementation;
    symbol->info.func.param_count = type->compound.function_info.parameter_count;
    symbol->scope_level = 0; // 库函数在全局作用域
    symbol->is_library_symbol = true;
    symbol->source_library = mmgr_alloc_string(library_name);
    
    /* 添加到全局作用域 */
    add_symbol_to_scope(table->global_scope, symbol);
    table->total_symbols++;
    table->library_symbols++;
    
    /* 如果原名不冲突，也注册原名 */
    if (!find_symbol_in_scope(table->global_scope, name)) {
        symbol_t *alias_symbol = create_symbol(name, SYMBOL_FUNCTION, type);
        if (alias_symbol) {
            alias_symbol->info.func.implementation = implementation;
            alias_symbol->info.func.param_count = symbol->info.func.param_count;
            alias_symbol->scope_level = 0;
            alias_symbol->is_library_symbol = true;
            alias_symbol->source_library = mmgr_alloc_string(library_name);
            
            add_symbol_to_scope(table->global_scope, alias_symbol);
            table->total_symbols++;
            table->library_symbols++;
        }
    }
    
    return symbol;
}

symbol_t* symbol_table_register_library_variable(symbol_table_t *table, const char *name,
                                                 const char *qualified_name, type_info_t *type,
                                                 void *value_ptr, const char *library_name) {
    if (!table || !table->is_initialized || !name || !qualified_name || !type || !library_name) {
        return NULL;
    }
    
    /* 创建变量符号 */
    symbol_t *symbol = create_symbol(qualified_name, SYMBOL_VARIABLE, type);
    if (!symbol) {
        return NULL;
    }
    
    symbol->info.var.category = VAR_GLOBAL;
    symbol->info.var.value_ptr = value_ptr;
    symbol->scope_level = 0; // 库变量在全局作用域
    symbol->is_library_symbol = true;
    symbol->source_library = mmgr_alloc_string(library_name);
    
    /* 添加到全局作用域 */
    add_symbol_to_scope(table->global_scope, symbol);
    table->total_symbols++;
    table->library_symbols++;
    
    /* 如果原名不冲突，也注册原名 */
    if (!find_symbol_in_scope(table->global_scope, name)) {
        symbol_t *alias_symbol = create_symbol(name, SYMBOL_VARIABLE, type);
        if (alias_symbol) {
            alias_symbol->info.var.category = VAR_GLOBAL;
            alias_symbol->info.var.value_ptr = value_ptr;
            alias_symbol->scope_level = 0;
            alias_symbol->is_library_symbol = true;
            alias_symbol->source_library = mmgr_alloc_string(library_name);
            
            add_symbol_to_scope(table->global_scope, alias_symbol);
            table->total_symbols++;
            table->library_symbols++;
        }
    }
    
    return symbol;
}

/* ========== 符号查找 ========== */

symbol_t* symbol_table_lookup(symbol_table_t *table, const char *name) {
    if (!table || !table->is_initialized || !name) {
        return NULL;
    }
    
    /* 从当前作用域开始向上查找 */
    for (int i = table->scope_depth; i >= 0; i--) {
        scope_t *scope = table->scope_stack[i];
        symbol_t *symbol = find_symbol_in_scope(scope, name);
        if (symbol) {
            return symbol;
        }
    }
    
    return NULL;
}

symbol_t* symbol_table_lookup_function(symbol_table_t *table, const char *name) {
    symbol_t *symbol = symbol_table_lookup(table, name);
    return (symbol && symbol->type == SYMBOL_FUNCTION) ? symbol : NULL;
}

symbol_t* symbol_table_lookup_variable(symbol_table_t *table, const char *name) {
    symbol_t *symbol = symbol_table_lookup(table, name);
    return (symbol && symbol->type == SYMBOL_VARIABLE) ? symbol : NULL;
}

bool symbol_table_symbol_exists(symbol_table_t *table, const char *name) {
    return symbol_table_lookup(table, name) != NULL;
}

/* ========== 类型管理 ========== */

type_info_t* symbol_table_get_builtin_type(base_type_t type) {
    if (!g_builtin_types_initialized) {
        return NULL;
    }
    
    if (type >= 0 && type < 16) {
        return &g_builtin_types[type];
    }
    
    return NULL;
}

/* ========== 内部辅助函数实现 ========== */

static uint32_t hash_string(const char *str) {
    uint32_t hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    
    return hash % SYMBOL_HASH_SIZE;
}

static symbol_t* create_symbol(const char *name, symbol_type_t type, type_info_t *data_type) {
    symbol_t *symbol = mmgr_alloc(sizeof(symbol_t));
    if (!symbol) {
        return NULL;
    }
    
    memset(symbol, 0, sizeof(symbol_t));
    strncpy(symbol->name, name, MAX_SYMBOL_NAME - 1);
    symbol->type = type;
    symbol->data_type = data_type;
    
    return symbol;
}

static scope_t* create_scope(const char *name, scope_type_t type, uint32_t level) {
    scope_t *scope = mmgr_alloc(sizeof(scope_t));
    if (!scope) {
        return NULL;
    }
    
    memset(scope, 0, sizeof(scope_t));
    strncpy(scope->name, name, MAX_SYMBOL_NAME - 1);
    scope->type = type;
    scope->level = level;
    
    return scope;
}

static void add_symbol_to_scope(scope_t *scope, symbol_t *symbol) {
    if (!scope || !symbol) {
        return;
    }
    
    /* 添加到链表 */
    symbol->next = scope->symbols;
    scope->symbols = symbol;
    
    /* 添加到哈希表 */
    uint32_t hash = hash_string(symbol->name);
    symbol->hash_next = scope->hash_table[hash];
    scope->hash_table[hash] = symbol;
    
    scope->symbol_count++;
}

static symbol_t* find_symbol_in_scope(scope_t *scope, const char *name) {
    if (!scope || !name) {
        return NULL;
    }
    
    uint32_t hash = hash_string(name);
    symbol_t *symbol = scope->hash_table[hash];
    
    while (symbol) {
        if (strcmp(symbol->name, name) == 0) {
            return symbol;
        }
        symbol = symbol->hash_next;
    }
    
    return NULL;
}

static void init_builtin_types(symbol_table_t *table) {
    if (g_builtin_types_initialized) {
        return;
    }
    
    /* 初始化基本类型 */
    g_builtin_types[TYPE_BOOL_ID] = (type_info_t){TYPE_BOOL_ID, 1, false, false, {0}, "BOOL"};
    g_builtin_types[TYPE_BYTE_ID] = (type_info_t){TYPE_BYTE_ID, 1, false, false, {0}, "BYTE"};
    g_builtin_types[TYPE_INT_ID] = (type_info_t){TYPE_INT_ID, 4, false, false, {0}, "INT"};
    g_builtin_types[TYPE_REAL_ID] = (type_info_t){TYPE_REAL_ID, 4, false, false, {0}, "REAL"};
    g_builtin_types[TYPE_STRING_ID] = (type_info_t){TYPE_STRING_ID, 8, false, false, {0}, "STRING"};
    
    /* 设置类型指针 */
    for (int i = 0; i < 16; i++) {
        if (i < 5) { // 只有前5个类型被初始化
            table->builtin_types[i] = &g_builtin_types[i];
        }
    }
    
    g_builtin_types_initialized = true;
}

/* ========== 工具接口实现 ========== */

void symbol_table_print_symbols(symbol_table_t *table) {
    if (!table || !table->is_initialized) {
        printf("Symbol table not initialized\n");
        return;
    }
    
    printf("\n=== Symbol Table ===\n");
    printf("Total symbols: %u (Library symbols: %u)\n", 
           table->total_symbols, table->library_symbols);
    
    symbol_table_print_scope(table, table->global_scope);
    printf("===================\n\n");
}

void symbol_table_print_scope(symbol_table_t *table, scope_t *scope) {
    if (!scope) return;
    
    printf("Scope: %s (level %u, %u symbols)\n", 
           scope->name, scope->level, scope->symbol_count);
    
    symbol_t *symbol = scope->symbols;
    while (symbol) {
        printf("  - %s (%s", symbol->name, 
               symbol->type == SYMBOL_FUNCTION ? "function" :
               symbol->type == SYMBOL_VARIABLE ? "variable" : "constant");
        
        if (symbol->is_library_symbol) {
            printf(", from library: %s", symbol->source_library);
        }
        printf(")\n");
        
        symbol = symbol->next;
    }
}

uint32_t symbol_table_get_symbol_count(symbol_table_t *table) {
    return table ? table->total_symbols : 0;
}

uint32_t symbol_table_get_library_symbol_count(symbol_table_t *table) {
    return table ? table->library_symbols : 0;
}