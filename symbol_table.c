#include "symbol_table.h"
#include "mmgr.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* 全局符号表管理器 */
static symbol_table_manager_t g_symbol_mgr = {0};

/* 内部辅助函数声明 */
static uint32_t hash_string(const char *str);
static symbol_t* create_symbol(const char *name, symbol_type_t type);
static void free_symbol(symbol_t *symbol);
static void free_type_info(type_info_t *type);
static scope_t* create_scope(const char *name, int scope_type, scope_t *parent);
static void free_scope(scope_t *scope);
static void add_symbol_to_scope(scope_t *scope, symbol_t *symbol);
static symbol_t* find_symbol_in_hash_table(scope_t *scope, const char *name);

/* 哈希函数实现 */
static uint32_t hash_string(const char *str) {
    uint32_t hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    
    return hash % SYMBOL_TABLE_SIZE;
}

/* 初始化符号表 */
int symbol_table_init(void) {
    if (g_symbol_mgr.is_initialized) {
        return 0; // 已经初始化
    }
    
    /* 清零管理器结构 */
    memset(&g_symbol_mgr, 0, sizeof(symbol_table_manager_t));
    
    /* 创建全局作用域 */
    g_symbol_mgr.global_scope = create_scope("global", SCOPE_GLOBAL, NULL);
    if (!g_symbol_mgr.global_scope) {
        return -1;
    }
    
    g_symbol_mgr.current_scope = g_symbol_mgr.global_scope;
    g_symbol_mgr.scopes[0] = g_symbol_mgr.global_scope;
    g_symbol_mgr.scope_depth = 0;
    
    /* 初始化内置类型 */
    g_symbol_mgr.builtin_types[TYPE_BOOL_ID] = create_basic_type(TYPE_BOOL_ID);
    g_symbol_mgr.builtin_types[TYPE_INT_ID] = create_basic_type(TYPE_INT_ID);
    g_symbol_mgr.builtin_types[TYPE_DINT_ID] = create_basic_type(TYPE_DINT_ID);
    g_symbol_mgr.builtin_types[TYPE_REAL_ID] = create_basic_type(TYPE_REAL_ID);
    g_symbol_mgr.builtin_types[TYPE_STRING_ID] = create_basic_type(TYPE_STRING_ID);
    g_symbol_mgr.builtin_types[TYPE_TIME_ID] = create_basic_type(TYPE_TIME_ID);
    
    /* 分配库管理内存 */
    g_symbol_mgr.library_info.imported_libraries = 
        (symbol_t**)mmgr_alloc_symbol(sizeof(symbol_t*) * 64);
    g_symbol_mgr.library_info.library_paths = 
        (char**)mmgr_alloc_symbol(sizeof(char*) * 32);
    
    if (!g_symbol_mgr.library_info.imported_libraries || 
        !g_symbol_mgr.library_info.library_paths) {
        return -1;
    }
    
    g_symbol_mgr.is_initialized = true;
    
    return 0;
}

/* 清理符号表 */
void symbol_table_cleanup(void) {
    if (!g_symbol_mgr.is_initialized) {
        return;
    }
    
    /* 清理所有作用域 */
    free_scope(g_symbol_mgr.global_scope);
    
    /* 清理内置类型 */
    for (int i = 0; i < 32; i++) {
        if (g_symbol_mgr.builtin_types[i]) {
            free_type_info(g_symbol_mgr.builtin_types[i]);
        }
    }
    
    /* 清理库信息 */
    // 库符号和路径由MMGR管理，这里不需要显式释放
    
    memset(&g_symbol_mgr, 0, sizeof(symbol_table_manager_t));
}

/* 创建作用域 */
static scope_t* create_scope(const char *name, int scope_type, scope_t *parent) {
    scope_t *scope = (scope_t*)mmgr_alloc_symbol(sizeof(scope_t));
    if (!scope) {
        return NULL;
    }
    
    memset(scope, 0, sizeof(scope_t));
    scope->name = mmgr_alloc_string(name);
    scope->scope_type = scope_type;
    scope->parent = parent;
    scope->level = parent ? parent->level + 1 : 0;
    scope->next_address = 0;
    
    /* 初始化哈希表 */
    memset(scope->hash_table, 0, sizeof(scope->hash_table));
    
    /* 添加到父作用域的子列表 */
    if (parent) {
        scope->next_sibling = parent->children;
        parent->children = scope;
    }
    
    return scope;
}

/* 进入新作用域 */
scope_t* enter_scope(const char *name, int scope_type) {
    if (!g_symbol_mgr.is_initialized) {
        return NULL;
    }
    
    if (g_symbol_mgr.scope_depth >= MAX_SCOPE_DEPTH - 1) {
        return NULL; // 作用域深度超限
    }
    
    scope_t *new_scope = create_scope(name, scope_type, g_symbol_mgr.current_scope);
    if (!new_scope) {
        return NULL;
    }
    
    g_symbol_mgr.scope_depth++;
    g_symbol_mgr.scopes[g_symbol_mgr.scope_depth] = new_scope;
    g_symbol_mgr.current_scope = new_scope;
    
    return new_scope;
}

/* 退出当前作用域 */
scope_t* exit_scope(void) {
    if (!g_symbol_mgr.is_initialized || g_symbol_mgr.scope_depth == 0) {
        return g_symbol_mgr.current_scope;
    }
    
    g_symbol_mgr.scope_depth--;
    g_symbol_mgr.current_scope = g_symbol_mgr.scopes[g_symbol_mgr.scope_depth];
    
    return g_symbol_mgr.current_scope;
}

/* 获取当前作用域 */
scope_t* get_current_scope(void) {
    return g_symbol_mgr.current_scope;
}

/* 获取全局作用域 */
scope_t* get_global_scope(void) {
    return g_symbol_mgr.global_scope;
}

/* 创建符号 */
static symbol_t* create_symbol(const char *name, symbol_type_t type) {
    symbol_t *symbol = (symbol_t*)mmgr_alloc_symbol(sizeof(symbol_t));
    if (!symbol) {
        return NULL;
    }
    
    memset(symbol, 0, sizeof(symbol_t));
    symbol->name = mmgr_alloc_string(name);
    symbol->symbol_type = type;
    symbol->scope_level = g_symbol_mgr.current_scope->level;
    symbol->reference_count = 0;
    symbol->access_count = 0;
    
    return symbol;
}

/* 添加符号到作用域 */
static void add_symbol_to_scope(scope_t *scope, symbol_t *symbol) {
    /* 计算哈希值并添加到哈希表 */
    uint32_t hash = hash_string(symbol->name);
    symbol->hash_next = scope->hash_table[hash];
    scope->hash_table[hash] = symbol;
    
    /* 添加到符号链表 */
    symbol->next = scope->symbols;
    scope->symbols = symbol;
    
    /* 分配地址 */
    symbol->address = scope->next_address;
    if (symbol->data_type) {
        scope->next_address += symbol->data_type->size;
    } else {
        scope->next_address += 4; // 默认大小
    }
    
    scope->symbol_count++;
    g_symbol_mgr.total_symbols++;
}

/* 添加符号 */
symbol_t* add_symbol(const char *name, symbol_type_t type, type_info_t *data_type) {
    if (!name || !g_symbol_mgr.is_initialized) {
        return NULL;
    }
    
    /* 检查当前作用域中是否已存在同名符号 */
    symbol_t *existing = find_symbol_in_hash_table(g_symbol_mgr.current_scope, name);
    if (existing) {
        return NULL; // 符号已存在
    }
    
    symbol_t *symbol = create_symbol(name, type);
    if (!symbol) {
        return NULL;
    }
    
    symbol->data_type = data_type;
    symbol->is_global = (g_symbol_mgr.current_scope == g_symbol_mgr.global_scope);
    
    add_symbol_to_scope(g_symbol_mgr.current_scope, symbol);
    
    return symbol;
}

/* 添加变量符号 */
symbol_t* add_variable_symbol(const char *name, type_info_t *data_type, 
                             var_category_t category, void *default_value) {
    symbol_t *symbol = add_symbol(name, SYMBOL_VAR, data_type);
    if (!symbol) {
        return NULL;
    }
    
    symbol->info.var_info.category = category;
    symbol->info.var_info.default_value = default_value;
    symbol->info.var_info.current_value = NULL;
    
    /* 设置符号属性 */
    if (category == VAR_CONSTANT) {
        symbol->attributes |= SYMBOL_ATTR_CONSTANT | SYMBOL_ATTR_READ_ONLY;
    }
    
    return symbol;
}

/* 添加函数符号 */
symbol_t* add_function_symbol(const char *name, type_info_t *return_type,
                             symbol_t **parameters, uint32_t param_count,
                             function_category_t category) {
    symbol_t *symbol = add_symbol(name, SYMBOL_FUNCTION, return_type);
    if (!symbol) {
        return NULL;
    }
    
    symbol->info.func_info.category = category;
    symbol->info.func_info.parameters = parameters;
    symbol->info.func_info.parameter_count = param_count;
    symbol->info.func_info.implementation = NULL;
    symbol->info.func_info.library_name = NULL;
    
    /* 内置函数标记 */
    if (category == FUNC_BUILTIN) {
        symbol->is_builtin = true;
        symbol->attributes |= SYMBOL_ATTR_READ_ONLY;
    }
    
    return symbol;
}

/* 在哈希表中查找符号 */
static symbol_t* find_symbol_in_hash_table(scope_t *scope, const char *name) {
    uint32_t hash = hash_string(name);
    symbol_t *symbol = scope->hash_table[hash];
    
    while (symbol) {
        if (strcmp(symbol->name, name) == 0) {
            symbol->access_count++;
            return symbol;
        }
        symbol = symbol->hash_next;
    }
    
    return NULL;
}

/* 查找符号 */
symbol_t* lookup_symbol(const char *name) {
    if (!name || !g_symbol_mgr.is_initialized) {
        return NULL;
    }
    
    /* 从当前作用域开始向上查找 */
    for (int i = g_symbol_mgr.scope_depth; i >= 0; i--) {
        scope_t *scope = g_symbol_mgr.scopes[i];
        symbol_t *symbol = find_symbol_in_hash_table(scope, name);
        if (symbol) {
            return symbol;
        }
    }
    
    return NULL; // 符号未找到
}

/* 在指定作用域查找符号 */
symbol_t* lookup_symbol_in_scope(const char *name, scope_t *scope) {
    if (!name || !scope) {
        return NULL;
    }
    
    return find_symbol_in_hash_table(scope, name);
}

/* 查找限定符号 */
symbol_t* lookup_qualified_symbol(const char *qualified_name) {
    if (!qualified_name || !g_symbol_mgr.is_initialized) {
        return NULL;
    }
    
    /* 解析限定名称 library.symbol */
    char *dot = strchr(qualified_name, '.');
    if (!dot) {
        return lookup_symbol(qualified_name); // 非限定名称
    }
    
    /* 分离库名和符号名 */
    size_t lib_name_len = dot - qualified_name;
    char *lib_name = mmgr_alloc_string_with_length(qualified_name, lib_name_len);
    char *symbol_name = dot + 1;
    
    if (!lib_name) {
        return NULL;
    }
    
    /* 查找库符号 */
    for (uint32_t i = 0; i < g_symbol_mgr.library_info.library_count; i++) {
        symbol_t *lib_symbol = g_symbol_mgr.library_info.imported_libraries[i];
        if (lib_symbol && strcmp(lib_symbol->name, lib_name) == 0) {
            /* 在库作用域中查找符号 */
            return resolve_library_symbol(lib_name, symbol_name);
        }
    }
    
    return NULL;
}

/* 创建基础类型 */
type_info_t* create_basic_type(int base_type) {
    type_info_t *type = (type_info_t*)mmgr_alloc_type_info(sizeof(type_info_t));
    if (!type) {
        return NULL;
    }
    
    memset(type, 0, sizeof(type_info_t));
    type->base_type = base_type;
    type->is_constant = false;
    type->is_array = false;
    type->is_pointer = false;
    type->alignment = 1;
    
    /* 设置类型大小 */
    switch (base_type) {
        case TYPE_BOOL_ID:
        case TYPE_BYTE_ID:
        case TYPE_SINT_ID:
        case TYPE_USINT_ID:
            type->size = 1;
            break;
        case TYPE_WORD_ID:
        case TYPE_INT_ID:
        case TYPE_UINT_ID:
            type->size = 2;
            type->alignment = 2;
            break;
        case TYPE_DWORD_ID:
        case TYPE_DINT_ID:
        case TYPE_UDINT_ID:
        case TYPE_REAL_ID:
            type->size = 4;
            type->alignment = 4;
            break;
        case TYPE_LWORD_ID:
        case TYPE_LINT_ID:
        case TYPE_ULINT_ID:
        case TYPE_LREAL_ID:
            type->size = 8;
            type->alignment = 8;
            break;
        case TYPE_STRING_ID:
        case TYPE_WSTRING_ID:
            type->size = sizeof(char*);
            type->alignment = sizeof(void*);
            break;
        case TYPE_TIME_ID:
        case TYPE_DATE_ID:
        case TYPE_TIME_OF_DAY_ID:
        case TYPE_DATE_AND_TIME_ID:
            type->size = 8;
            type->alignment = 8;
            break;
        default:
            type->size = 4; // 默认大小
            break;
    }
    
    g_symbol_mgr.total_types++;
    
    return type;
}

/* 创建数组类型 */
type_info_t* create_array_type(type_info_t *element_type, uint32_t dimensions, uint32_t *bounds) {
    if (!element_type || dimensions == 0 || !bounds) {
        return NULL;
    }
    
    type_info_t *type = (type_info_t*)mmgr_alloc_type_info(sizeof(type_info_t));
    if (!type) {
        return NULL;
    }
    
    memset(type, 0, sizeof(type_info_t));
    type->base_type = TYPE_ARRAY_ID;
    type->is_array = true;
    type->alignment = element_type->alignment;
    
    /* 设置数组信息 */
    type->compound.array_info.element_type = element_type;
    type->compound.array_info.dimensions = dimensions;
    type->compound.array_info.bounds = (uint32_t*)mmgr_alloc_type_info(sizeof(uint32_t) * dimensions * 2);
    
    if (!type->compound.array_info.bounds) {
        return NULL;
    }
    
    /* 计算数组大小 */
    uint32_t total_elements = 1;
    for (uint32_t i = 0; i < dimensions * 2; i += 2) {
        type->compound.array_info.bounds[i] = bounds[i];     // 下界
        type->compound.array_info.bounds[i + 1] = bounds[i + 1]; // 上界
        total_elements *= (bounds[i + 1] - bounds[i] + 1);
    }
    
    type->size = total_elements * element_type->size;
    g_symbol_mgr.total_types++;
    
    return type;
}

/* 导入库符号 */
bool import_library_symbols(const char *library_name, const char *alias) {
    if (!library_name || !g_symbol_mgr.is_initialized) {
        return false;
    }
    
    /* 检查库是否已导入 */
    for (uint32_t i = 0; i < g_symbol_mgr.library_info.library_count; i++) {
        symbol_t *lib = g_symbol_mgr.library_info.imported_libraries[i];
        if (lib && strcmp(lib->name, library_name) == 0) {
            return true; // 已导入
        }
    }
    
    /* 创建库符号 */
    symbol_t *lib_symbol = add_library_symbol(library_name, "1.0");
    if (!lib_symbol) {
        return false;
    }
    
    /* 添加到导入库列表 */
    if (g_symbol_mgr.library_info.library_count < 64) {
        g_symbol_mgr.library_info.imported_libraries[g_symbol_mgr.library_info.library_count] = lib_symbol;
        g_symbol_mgr.library_info.library_count++;
    }
    
    /* 设置别名 */
    if (alias) {
        symbol_t *alias_symbol = create_symbol(alias, SYMBOL_LIBRARY);
        if (alias_symbol) {
            alias_symbol->info.lib_info.library_name = mmgr_alloc_string(library_name);
            add_symbol_to_scope(g_symbol_mgr.current_scope, alias_symbol);
        }
    }
    
    return true;
}

/* 添加库符号 */
symbol_t* add_library_symbol(const char *name, const char *version) {
    symbol_t *symbol = add_symbol(name, SYMBOL_LIBRARY, NULL);
    if (!symbol) {
        return NULL;
    }
    
    symbol->info.lib_info.library_name = mmgr_alloc_string(name);
    symbol->info.lib_info.version = mmgr_alloc_string(version);
    symbol->info.lib_info.is_loaded = false;
    
    return symbol;
}

/* 解析库符号 */
symbol_t* resolve_library_symbol(const char *library_name, const char *symbol_name) {
    // 这里简化实现，实际应该加载库并查找符号
    return lookup_symbol(symbol_name);
}

/* 遍历符号 */
void traverse_symbols(symbol_visitor_fn visitor, void *context) {
    if (!visitor || !g_symbol_mgr.is_initialized) {
        return;
    }
    
    traverse_scope_symbols(g_symbol_mgr.global_scope, visitor, context);
}

/* 遍历作用域符号 */
void traverse_scope_symbols(scope_t *scope, symbol_visitor_fn visitor, void *context) {
    if (!scope || !visitor) {
        return;
    }
    
    symbol_t *symbol = scope->symbols;
    while (symbol) {
        if (!visitor(symbol, context)) {
            break; // 访问者返回false，停止遍历
        }
        symbol = symbol->next;
    }
    
    /* 递归遍历子作用域 */
    scope_t *child = scope->children;
    while (child) {
        traverse_scope_symbols(child, visitor, context);
        child = child->next_sibling;
    }
}

/* 打印符号表 */
void print_symbol_table(void) {
    if (!g_symbol_mgr.is_initialized) {
        printf("Symbol table not initialized\n");
        return;
    }
    
    printf("\n=== Symbol Table ===\n");
    printf("Total symbols: %u\n", g_symbol_mgr.total_symbols);
    printf("Total types: %u\n", g_symbol_mgr.total_types);
    printf("Current scope depth: %u\n", g_symbol_mgr.scope_depth);
    printf("Imported libraries: %u\n", g_symbol_mgr.library_info.library_count);
    
    print_scope_symbols(g_symbol_mgr.global_scope, 0);
    printf("==================\n\n");
}

/* 打印作用域符号 */
void print_scope_symbols(scope_t *scope, int indent) {
    if (!scope) return;
    
    /* 打印作用域信息 */
    for (int i = 0; i < indent; i++) printf("  ");
    printf("Scope: %s (level %u, %u symbols)\n", 
           scope->name, scope->level, scope->symbol_count);
    
    /* 打印符号 */
    symbol_t *symbol = scope->symbols;
    while (symbol) {
        for (int i = 0; i < indent + 1; i++) printf("  ");
        printf("- %s: %s", symbol->name, 
               symbol->symbol_type == SYMBOL_VAR ? "VAR" :
               symbol->symbol_type == SYMBOL_FUNCTION ? "FUNC" :
               symbol->symbol_type == SYMBOL_LIBRARY ? "LIB" : "OTHER");
        if (symbol->data_type) {
            printf(" (size: %u)", symbol->data_type->size);
        }
        printf("\n");
        symbol = symbol->next;
    }
    
    /* 递归打印子作用域 */
    scope_t *child = scope->children;
    while (child) {
        print_scope_symbols(child, indent + 1);
        child = child->next_sibling;
    }
}

/* 检查符号表是否已初始化 */
bool symbol_table_is_initialized(void) {
    return g_symbol_mgr.is_initialized;
}

/* 释放符号 */
static void free_symbol(symbol_t *symbol) {
    // 由MMGR管理，这里不需要显式释放
}

/* 释放类型信息 */
static void free_type_info(type_info_t *type) {
    // 由MMGR管理，这里不需要显式释放
}

/* 释放作用域 */
static void free_scope(scope_t *scope) {
    if (!scope) return;
    
    /* 递归释放子作用域 */
    scope_t *child = scope->children;
    while (child) {
        scope_t *next = child->next_sibling;
        free_scope(child);
        child = next;
    }
    
    /* 释放符号链表 */
    symbol_t *symbol = scope->symbols;
    while (symbol) {
        symbol_t *next = symbol->next;
        free_symbol(symbol);
        symbol = next;
    }
    
    // 作用域本身由MMGR管理
}
