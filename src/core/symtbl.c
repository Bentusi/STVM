/**
 * @file symtbl.c
 * @brief 符号表的实现
 */

#include "symtbl.h"
#include "mmgr.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYMBOL_TABLE_SIZE 256  // 哈希表大小

/**
 * @brief 简单的字符串哈希函数
 */
static uint32_t hash_string(const char* str) {
    uint32_t hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    
    return hash % SYMBOL_TABLE_SIZE;
}

/**
 * @brief 创建作用域
 */
static Scope* scope_create(int level, Scope* parent) {
    Scope* scope = (Scope*)mmgr_alloc_from_pool(POOL_SYMBOL, sizeof(Scope));
    if (!scope) return NULL;
    
    scope->level = level;
    scope->parent = parent;
    scope->next = NULL;
    scope->symbol_count = 0;
    
    // 分配符号哈希表
    scope->symbols = (Symbol**)mmgr_calloc(sizeof(Symbol*) * SYMBOL_TABLE_SIZE);
    if (!scope->symbols) {
        mmgr_free(scope);
        return NULL;
    }
    
    return scope;
}

/**
 * @brief 释放作用域
 */
static void scope_free(Scope* scope) {
    if (!scope) return;
    
    // 释放所有符号
    for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        Symbol* sym = scope->symbols[i];
        while (sym) {
            Symbol* next = sym->next;
            
            if (sym->name) mmgr_free(sym->name);
            if (sym->qualified_name) mmgr_free(sym->qualified_name);
            if (sym->library_name) mmgr_free(sym->library_name);
            if (sym->type) type_info_free(sym->type);
            
            // 释放常量值
            if (sym->kind == SYM_CONSTANT) {
                value_free(&sym->const_value);
            }
            
            mmgr_free(sym);
            sym = next;
        }
    }
    
    if (scope->symbols) mmgr_free(scope->symbols);
    mmgr_free(scope);
}

/**
 * @brief 初始化符号表
 */
SymbolTable* symtbl_init(void) {
    SymbolTable* symtbl = (SymbolTable*)mmgr_calloc(sizeof(SymbolTable));
    if (!symtbl) return NULL;
    
    // 创建全局作用域
    symtbl->global_scope = scope_create(0, NULL);
    if (!symtbl->global_scope) {
        mmgr_free(symtbl);
        return NULL;
    }
    
    symtbl->current_scope = symtbl->global_scope;
    symtbl->current_level = 0;
    symtbl->global_var_count = 0;
    symtbl->local_var_offset = 0;
    symtbl->error_code = OK;
    
    return symtbl;
}

/**
 * @brief 释放符号表
 */
void symtbl_free(SymbolTable* symtbl) {
    if (!symtbl) return;
    
    // 释放所有作用域
    Scope* scope = symtbl->global_scope;
    while (scope) {
        Scope* next = scope->next;
        scope_free(scope);
        scope = next;
    }
    
    mmgr_free(symtbl);
}

/**
 * @brief 进入新作用域
 */
void symtbl_enter_scope(SymbolTable* symtbl) {
    if (!symtbl) return;
    
    symtbl->current_level++;
    Scope* new_scope = scope_create(symtbl->current_level, symtbl->current_scope);
    
    if (new_scope) {
        // 将新作用域添加到链表
        new_scope->next = symtbl->global_scope->next;
        symtbl->global_scope->next = new_scope;
        symtbl->current_scope = new_scope;
    }
}

/**
 * @brief 离开当前作用域
 */
void symtbl_leave_scope(SymbolTable* symtbl) {
    if (!symtbl || symtbl->current_scope == symtbl->global_scope) {
        return;
    }
    
    symtbl->current_scope = symtbl->current_scope->parent;
    symtbl->current_level--;
}

/**
 * @brief 创建符号
 */
static Symbol* symbol_create(const char* name, SymbolKind kind, TypeInfo* type) {
    Symbol* sym = (Symbol*)mmgr_alloc_from_pool(POOL_SYMBOL, sizeof(Symbol));
    if (!sym) return NULL;
    
    memset(sym, 0, sizeof(Symbol));
    sym->name = name ? mmgr_strdup(name) : NULL;
    sym->kind = kind;
    sym->type = type;
    sym->next = NULL;
    
    return sym;
}

/**
 * @brief 在作用域中添加符号
 */
static bool scope_add_symbol(Scope* scope, Symbol* symbol) {
    if (!scope || !symbol || !symbol->name) return false;
    
    uint32_t hash = hash_string(symbol->name);
    
    // 检查是否已存在同名符号
    Symbol* existing = scope->symbols[hash];
    while (existing) {
        if (strcmp(existing->name, symbol->name) == 0) {
            return false; // 重复定义
        }
        existing = existing->next;
    }
    
    // 添加到哈希表
    symbol->next = scope->symbols[hash];
    scope->symbols[hash] = symbol;
    scope->symbol_count++;
    
    return true;
}

/**
 * @brief 定义变量符号
 */
Symbol* symtbl_define_variable(SymbolTable* symtbl, const char* name, TypeInfo* type, bool is_const) {
    if (!symtbl || !name || !type) return NULL;
    
    Symbol* sym = symbol_create(name, is_const ? SYM_CONSTANT : SYM_VARIABLE, type);
    if (!sym) return NULL;
    
    sym->scope_level = symtbl->current_level;
    
    if (symtbl->current_level == 0) {
        // 全局变量
        sym->is_global = true;
        sym->index = symtbl->global_var_count++;
    } else {
        // 局部变量
        sym->is_global = false;
        sym->offset = symtbl->local_var_offset++;
    }
    
    if (!scope_add_symbol(symtbl->current_scope, sym)) {
        REPORT_ERROR(symtbl, ERR_NAME, "Variable '%s' already defined in current scope", name);
        mmgr_free(sym->name);
        mmgr_free(sym);
        return NULL;
    }
    
    return sym;
}

/**
 * @brief 定义函数符号
 */
Symbol* symtbl_define_function(SymbolTable* symtbl, const char* name, TypeInfo* return_type,
                                TypeInfo** param_types, int param_count) {
    if (!symtbl || !name) return NULL;
    
    // 创建函数类型
    TypeInfo* func_type = type_info_create_function(return_type, param_types, param_count);
    if (!func_type) return NULL;
    
    Symbol* sym = symbol_create(name, SYM_FUNCTION, func_type);
    if (!sym) {
        type_info_free(func_type);
        return NULL;
    }
    
    sym->scope_level = 0; // 函数总是在全局作用域
    sym->param_count = param_count;
    sym->local_count = 0; // 将在代码生成时更新
    
    if (!scope_add_symbol(symtbl->global_scope, sym)) {
        REPORT_ERROR(symtbl, ERR_NAME, "Function '%s' already defined", name);
        mmgr_free(sym->name);
        type_info_free(func_type);
        mmgr_free(sym);
        return NULL;
    }
    
    return sym;
}

/**
 * @brief 定义参数符号
 */
Symbol* symtbl_define_parameter(SymbolTable* symtbl, const char* name, TypeInfo* type) {
    if (!symtbl || !name || !type) return NULL;
    
    Symbol* sym = symbol_create(name, SYM_PARAMETER, type);
    if (!sym) return NULL;
    
    sym->scope_level = symtbl->current_level;
    sym->is_global = false;
    sym->offset = symtbl->local_var_offset++;
    
    if (!scope_add_symbol(symtbl->current_scope, sym)) {
        REPORT_ERROR(symtbl, ERR_NAME, "Parameter '%s' already defined", name);
        mmgr_free(sym->name);
        mmgr_free(sym);
        return NULL;
    }
    
    return sym;
}

/**
 * @brief 定义常量符号
 */
Symbol* symtbl_define_constant(SymbolTable* symtbl, const char* name, Value value) {
    if (!symtbl || !name) return NULL;
    
    TypeInfo* type = type_info_create(value.type);
    if (!type) return NULL;
    
    Symbol* sym = symbol_create(name, SYM_CONSTANT, type);
    if (!sym) {
        type_info_free(type);
        return NULL;
    }
    
    sym->scope_level = symtbl->current_level;
    sym->const_value = value_copy(value);
    
    if (symtbl->current_level == 0) {
        sym->is_global = true;
    }
    
    if (!scope_add_symbol(symtbl->current_scope, sym)) {
        REPORT_ERROR(symtbl, ERR_NAME, "Constant '%s' already defined", name);
        mmgr_free(sym->name);
        type_info_free(type);
        value_free(&sym->const_value);
        mmgr_free(sym);
        return NULL;
    }
    
    return sym;
}

/**
 * @brief 在作用域中查找符号
 */
static Symbol* scope_lookup(Scope* scope, const char* name) {
    if (!scope || !name) return NULL;
    
    uint32_t hash = hash_string(name);
    Symbol* sym = scope->symbols[hash];
    
    while (sym) {
        if (strcmp(sym->name, name) == 0) {
            return sym;
        }
        sym = sym->next;
    }
    
    return NULL;
}

/**
 * @brief 查找符号（在当前作用域及父作用域中查找）
 */
Symbol* symtbl_lookup(SymbolTable* symtbl, const char* name) {
    if (!symtbl || !name) return NULL;
    
    // 检查是否是限定名（包含.）
    if (strchr(name, '.')) {
        // 限定名，只在全局作用域查找
        return scope_lookup(symtbl->global_scope, name);
    }
    
    // 从当前作用域向上查找
    Scope* scope = symtbl->current_scope;
    while (scope) {
        Symbol* sym = scope_lookup(scope, name);
        if (sym) return sym;
        scope = scope->parent;
    }
    
    return NULL;
}

/**
 * @brief 仅在当前作用域中查找符号
 */
Symbol* symtbl_lookup_current_scope(SymbolTable* symtbl, const char* name) {
    if (!symtbl || !name) return NULL;
    return scope_lookup(symtbl->current_scope, name);
}

/**
 * @brief 在全局作用域中查找符号
 */
Symbol* symtbl_lookup_global(SymbolTable* symtbl, const char* name) {
    if (!symtbl || !name) return NULL;
    return scope_lookup(symtbl->global_scope, name);
}

/**
 * @brief 注册库符号
 */
bool symtbl_register_library_symbol(SymbolTable* symtbl, const char* library_name, Symbol* symbol) {
    if (!symtbl || !library_name || !symbol) return false;
    
    // 设置库信息
    symbol->is_library = true;
    symbol->library_name = mmgr_strdup(library_name);
    
    // 创建限定名
    symbol->qualified_name = symtbl_make_qualified_name(library_name, symbol->name);
    
    // 添加到全局作用域
    if (!scope_add_symbol(symtbl->global_scope, symbol)) {
        REPORT_ERROR(symtbl, ERR_NAME, "Library symbol '%s' conflicts with existing symbol", 
                    symbol->qualified_name);
        return false;
    }
    
    return true;
}

/**
 * @brief 创建限定名
 */
char* symtbl_make_qualified_name(const char* module_name, const char* symbol_name) {
    if (!module_name || !symbol_name) return NULL;
    
    size_t len = strlen(module_name) + strlen(symbol_name) + 2; // +2 for '.' and '\0'
    char* qualified = (char*)mmgr_alloc(len);
    if (!qualified) return NULL;
    
    snprintf(qualified, len, "%s.%s", module_name, symbol_name);
    return qualified;
}

/**
 * @brief 解析限定名
 */
bool symtbl_parse_qualified_name(const char* qualified_name, char* module_name, 
                                  char* symbol_name, size_t buffer_size) {
    if (!qualified_name || !module_name || !symbol_name || buffer_size == 0) {
        return false;
    }
    
    const char* dot = strchr(qualified_name, '.');
    if (!dot) {
        // 不是限定名
        return false;
    }
    
    size_t module_len = dot - qualified_name;
    size_t symbol_len = strlen(dot + 1);
    
    if (module_len >= buffer_size || symbol_len >= buffer_size) {
        return false; // 缓冲区太小
    }
    
    strncpy(module_name, qualified_name, module_len);
    module_name[module_len] = '\0';
    
    strcpy(symbol_name, dot + 1);
    
    return true;
}

/**
 * @brief 获取当前作用域层级
 */
int symtbl_current_level(SymbolTable* symtbl) {
    return symtbl ? symtbl->current_level : -1;
}

/**
 * @brief 重置局部变量偏移计数器
 */
void symtbl_reset_local_offset(SymbolTable* symtbl) {
    if (symtbl) {
        symtbl->local_var_offset = 0;
    }
}

/**
 * @brief 获取下一个全局变量索引
 */
int32_t symtbl_next_global_index(SymbolTable* symtbl) {
    return symtbl ? symtbl->global_var_count++ : -1;
}

/**
 * @brief 获取下一个局部变量偏移
 */
int32_t symtbl_next_local_offset(SymbolTable* symtbl) {
    return symtbl ? symtbl->local_var_offset++ : -1;
}

/**
 * @brief 遍历当前作用域的所有符号
 */
void symtbl_iterate_current_scope(SymbolTable* symtbl, SymbolIterator callback, void* user_data) {
    if (!symtbl || !callback) return;
    
    Scope* scope = symtbl->current_scope;
    if (!scope) return;
    
    for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        Symbol* sym = scope->symbols[i];
        while (sym) {
            callback(sym, user_data);
            sym = sym->next;
        }
    }
}

/**
 * @brief 遍历所有作用域的所有符号
 */
void symtbl_iterate_all(SymbolTable* symtbl, SymbolIterator callback, void* user_data) {
    if (!symtbl || !callback) return;
    
    Scope* scope = symtbl->global_scope;
    while (scope) {
        for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
            Symbol* sym = scope->symbols[i];
            while (sym) {
                callback(sym, user_data);
                sym = sym->next;
            }
        }
        scope = scope->next;
    }
}

/**
 * @brief 打印符号信息
 */
void symbol_print(Symbol* symbol) {
    if (!symbol) return;
    
    const char* kind_str[] = {"VAR", "FUNC", "PARAM", "CONST"};
    
    printf("  %s '%s'", kind_str[symbol->kind], symbol->name);
    
    if (symbol->qualified_name) {
        printf(" (qualified: %s)", symbol->qualified_name);
    }
    
    if (symbol->type) {
        printf(" : %s", type_to_string(symbol->type->base_type));
    }
    
    if (symbol->is_library) {
        printf(" [from library: %s]", symbol->library_name);
    }
    
    if (symbol->kind == SYM_VARIABLE || symbol->kind == SYM_PARAMETER) {
        if (symbol->is_global) {
            printf(" [global #%d]", symbol->index);
        } else {
            printf(" [local @%d]", symbol->offset);
        }
    }
    
    if (symbol->kind == SYM_FUNCTION) {
        printf(" [params:%d, address:0x%x]", symbol->param_count, symbol->address);
    }
    
    printf("\n");
}

/**
 * @brief 打印符号表
 */
void symtbl_print(SymbolTable* symtbl) {
    if (!symtbl) return;
    
    printf("\n=== Symbol Table ===\n");
    printf("Current level: %d\n", symtbl->current_level);
    printf("Global variables: %d\n", symtbl->global_var_count);
    
    // 打印全局作用域
    printf("\n--- Global Scope (level 0) ---\n");
    Scope* global = symtbl->global_scope;
    for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        Symbol* sym = global->symbols[i];
        while (sym) {
            symbol_print(sym);
            sym = sym->next;
        }
    }
    
    // 打印其他作用域
    Scope* scope = symtbl->global_scope->next;
    while (scope) {
        printf("\n--- Scope (level %d) ---\n", scope->level);
        for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
            Symbol* sym = scope->symbols[i];
            while (sym) {
                symbol_print(sym);
                sym = sym->next;
            }
        }
        scope = scope->next;
    }
    
    printf("===================\n\n");
}
