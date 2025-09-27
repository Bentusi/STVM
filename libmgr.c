#include "libmgr.h"
#include "symbol_table.h"
#include "mmgr.h"
#include "ast.h"
#include "bytecode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>

/* 外部解析器声明 */
extern FILE *yyin;
extern int yyparse(void);
extern ast_node_t* get_ast_root(void);

/* 内部辅助函数声明 */
static library_file_type_t detect_file_type(const char *path);
static bool file_exists(const char *path);
static int parse_source_file(const char *path, library_info_t *lib);
static int parse_bytecode_file(const char *path, library_info_t *lib);
static int extract_symbols_from_ast(ast_node_t *ast, library_info_t *lib);
static int extract_symbols_from_bytecode(bytecode_file_t *bytecode, library_info_t *lib);
static void generate_qualified_names(library_info_t *lib);
static library_info_t* find_library_by_name_or_alias(library_manager_t *mgr, const char *name);
static void set_error(library_manager_t *mgr, const char *format, ...);
static uint32_t hash_string(const char *str);

/* 内置库注册函数 */
static int register_builtin_math_library(library_manager_t *mgr, const char *alias);
static int register_builtin_string_library(library_manager_t *mgr, const char *alias);
static int register_builtin_io_library(library_manager_t *mgr, const char *alias);

/* ========== 生命周期管理 ========== */

library_manager_t* libmgr_create(void) {
    library_manager_t *mgr = mmgr_alloc(sizeof(library_manager_t));
    if (!mgr) {
        return NULL;
    }
    
    memset(mgr, 0, sizeof(library_manager_t));
    return mgr;
}

int libmgr_init(library_manager_t *mgr) {
    if (!mgr) {
        return LIBMGR_ERROR_NOT_INITIALIZED;
    }
    
    if (mgr->is_initialized) {
        return LIBMGR_SUCCESS;
    }
    
    /* 添加默认搜索路径 */
    libmgr_add_search_path(mgr, ".");
    libmgr_add_search_path(mgr, "./libs");
    libmgr_add_search_path(mgr, "/usr/local/lib/st");
    libmgr_add_search_path(mgr, "/usr/lib/st");
    
    mgr->is_initialized = true;
    return LIBMGR_SUCCESS;
}

void libmgr_destroy(library_manager_t *mgr) {
    if (!mgr) {
        return;
    }
    
    /* 清理所有库资源 */
    for (uint32_t i = 0; i < mgr->library_count; i++) {
        library_info_t *lib = &mgr->libraries[i];
        for (uint32_t j = 0; j < lib->symbol_count; j++) {
            library_symbol_t *symbol = &lib->symbols[j];
            if (symbol->data_type) {
                // 清理类型信息（如果需要）
            }
        }
    }
    
    mmgr_free(mgr);
}

bool libmgr_is_initialized(library_manager_t *mgr) {
    return mgr && mgr->is_initialized;
}

/* ========== 库文件加载实现 ========== */

int libmgr_load_library(library_manager_t *mgr, const char *name, 
                        const char *path, const char *alias) {
    if (!mgr || !mgr->is_initialized || !name) {
        return LIBMGR_ERROR_NOT_INITIALIZED;
    }
    
    /* 检查库是否已加载 */
    library_info_t *existing = find_library_by_name_or_alias(mgr, name);
    if (existing && existing->status == LIB_STATUS_LOADED) {
        existing->reference_count++;
        return LIBMGR_SUCCESS;
    }
    
    /* 检查库数量限制 */
    if (mgr->library_count >= MAX_LIBRARIES) {
        set_error(mgr, "Maximum libraries (%d) reached", MAX_LIBRARIES);
        return LIBMGR_ERROR_MAX_LIBRARIES_REACHED;
    }
    
    /* 解析路径 */
    const char *resolved_path = path;
    if (!resolved_path) {
        resolved_path = libmgr_resolve_library_path(mgr, name);
        if (!resolved_path) {
            set_error(mgr, "Cannot resolve path for library '%s'", name);
            return LIBMGR_ERROR_INVALID_PATH;
        }
    }
    
    if (!file_exists(resolved_path)) {
        set_error(mgr, "Library file not found: %s", resolved_path);
        return LIBMGR_ERROR_INVALID_PATH;
    }
    
    /* 创建新的库信息 */
    library_info_t *lib = &mgr->libraries[mgr->library_count];
    memset(lib, 0, sizeof(library_info_t));
    
    strncpy(lib->name, name, MAX_LIBRARY_NAME - 1);
    strncpy(lib->path, resolved_path, MAX_LIBRARY_PATH - 1);
    lib->type = LIB_TYPE_USER;
    lib->file_type = detect_file_type(resolved_path);
    lib->status = LIB_STATUS_UNLOADED;
    
    if (alias && strlen(alias) > 0) {
        strncpy(lib->alias, alias, MAX_LIBRARY_NAME - 1);
        lib->has_alias = true;
    }
    
    /* 根据文件类型解析 */
    int result;
    switch (lib->file_type) {
        case FILE_TYPE_SOURCE:
            result = parse_source_file(resolved_path, lib);
            break;
        case FILE_TYPE_BYTECODE:
            result = parse_bytecode_file(resolved_path, lib);
            break;
        default:
            set_error(mgr, "Unknown file type for '%s'", resolved_path);
            return LIBMGR_ERROR_FILE_TYPE_UNKNOWN;
    }
    
    if (result != 0) {
        set_error(mgr, "Failed to parse library file: %s", resolved_path);
        lib->status = LIB_STATUS_ERROR;
        return LIBMGR_ERROR_PARSE_FAILED;
    }
    
    /* 生成限定名 */
    generate_qualified_names(lib);
    
    lib->status = LIB_STATUS_LOADED;
    lib->reference_count = 1;
    mgr->library_count++;
    
    return LIBMGR_SUCCESS;
}

int libmgr_unload_library(library_manager_t *mgr, const char *name_or_alias) {
    if (!mgr || !mgr->is_initialized || !name_or_alias) {
        return LIBMGR_ERROR_NOT_INITIALIZED;
    }
    
    library_info_t *lib = find_library_by_name_or_alias(mgr, name_or_alias);
    if (!lib) {
        return LIBMGR_ERROR_LIBRARY_NOT_FOUND;
    }
    
    if (lib->reference_count > 1) {
        lib->reference_count--;
        return LIBMGR_SUCCESS;
    }
    
    /* 清理库资源并标记为卸载 */
    lib->status = LIB_STATUS_UNLOADED;
    lib->reference_count = 0;
    
    return LIBMGR_SUCCESS;
}

/* ========== 符号提取和注册实现（核心功能） ========== */

int libmgr_register_symbols_to_table(library_manager_t *mgr, const char *lib_name,
                                     struct symbol_table *symbol_table) {
    if (!mgr || !mgr->is_initialized || !lib_name || !symbol_table) {
        return LIBMGR_ERROR_NOT_INITIALIZED;
    }
    
    library_info_t *lib = find_library_by_name_or_alias(mgr, lib_name);
    if (!lib || lib->status != LIB_STATUS_LOADED) {
        set_error(mgr, "Library '%s' not found or not loaded", lib_name);
        return LIBMGR_ERROR_LIBRARY_NOT_FOUND;
    }
    
    /* 将库中的所有导出符号注册到符号表 */
    for (uint32_t i = 0; i < lib->symbol_count; i++) {
        library_symbol_t *lib_symbol = &lib->symbols[i];
        
        if (!lib_symbol->is_exported) {
            continue;
        }
        
        symbol_t *symbol = NULL;
        
        switch (lib_symbol->type) {
            case LIB_SYMBOL_FUNCTION:
                symbol = symbol_table_register_library_function(
                    symbol_table,
                    lib_symbol->name,
                    lib_symbol->qualified_name,
                    lib_symbol->data_type,
                    lib_symbol->implementation,
                    lib->name
                );
                break;
                
            case LIB_SYMBOL_VARIABLE:
                symbol = symbol_table_register_library_variable(
                    symbol_table,
                    lib_symbol->name,
                    lib_symbol->qualified_name,
                    lib_symbol->data_type,
                    lib_symbol->implementation,
                    lib->name
                );
                break;
                
            case LIB_SYMBOL_CONSTANT:
                // 常量可以作为特殊变量处理
                symbol = symbol_table_register_library_variable(
                    symbol_table,
                    lib_symbol->name,
                    lib_symbol->qualified_name,
                    lib_symbol->data_type,
                    lib_symbol->implementation,
                    lib->name
                );
                break;
        }
        
        if (!symbol) {
            set_error(mgr, "Failed to register symbol '%s' from library '%s'", 
                     lib_symbol->name, lib->name);
            return LIBMGR_ERROR_PARSE_FAILED;
        }
    }
    
    return LIBMGR_SUCCESS;
}

int libmgr_get_library_symbols(library_manager_t *mgr, const char *lib_name,
                               library_symbol_t **symbols, uint32_t *count) {
    if (!mgr || !mgr->is_initialized || !lib_name) {
        return LIBMGR_ERROR_NOT_INITIALIZED;
    }
    
    library_info_t *lib = find_library_by_name_or_alias(mgr, lib_name);
    if (!lib || lib->status != LIB_STATUS_LOADED) {
        return LIBMGR_ERROR_LIBRARY_NOT_FOUND;
    }
    
    if (symbols) *symbols = lib->symbols;
    if (count) *count = lib->symbol_count;
    
    return LIBMGR_SUCCESS;
}

/* ========== 库信息查询实现 ========== */

library_info_t* libmgr_get_library_info(library_manager_t *mgr, const char *name_or_alias) {
    if (!mgr || !mgr->is_initialized || !name_or_alias) {
        return NULL;
    }
    
    return find_library_by_name_or_alias(mgr, name_or_alias);
}

bool libmgr_is_library_loaded(library_manager_t *mgr, const char *name_or_alias) {
    library_info_t *lib = libmgr_get_library_info(mgr, name_or_alias);
    return lib && lib->status == LIB_STATUS_LOADED;
}

int libmgr_get_library_stats(library_manager_t *mgr, const char *name_or_alias,
                             uint32_t *func_count, uint32_t *var_count) {
    library_info_t *lib = libmgr_get_library_info(mgr, name_or_alias);
    if (!lib) {
        return LIBMGR_ERROR_LIBRARY_NOT_FOUND;
    }
    
    if (func_count) *func_count = lib->function_count;
    if (var_count) *var_count = lib->variable_count;
    
    return LIBMGR_SUCCESS;
}

/* ========== 内置库注册实现 ========== */

int libmgr_load_builtin_libraries(library_manager_t *mgr) {
    if (!mgr || !mgr->is_initialized) {
        return LIBMGR_ERROR_NOT_INITIALIZED;
    }
    
    int result = LIBMGR_SUCCESS;
    
    /* 注册数学库 */
    if (register_builtin_math_library(mgr, "math") != LIBMGR_SUCCESS) {
        result = LIBMGR_ERROR_PARSE_FAILED;
    }
    
    /* 注册字符串库 */
    if (register_builtin_string_library(mgr, "string") != LIBMGR_SUCCESS) {
        result = LIBMGR_ERROR_PARSE_FAILED;
    }
    
    /* 注册IO库 */
    if (register_builtin_io_library(mgr, "io") != LIBMGR_SUCCESS) {
        result = LIBMGR_ERROR_PARSE_FAILED;
    }
    
    return result;
}

/* ========== 内部辅助函数实现 ========== */

static library_file_type_t detect_file_type(const char *path) {
    if (!path) return FILE_TYPE_UNKNOWN;
    
    const char *ext = strrchr(path, '.');
    if (!ext) return FILE_TYPE_UNKNOWN;
    
    if (strcmp(ext, ".st") == 0) return FILE_TYPE_SOURCE;
    if (strcmp(ext, ".stbc") == 0) return FILE_TYPE_BYTECODE;
    
    return FILE_TYPE_UNKNOWN;
}

static bool file_exists(const char *path) {
    if (!path) return false;
    struct stat st;
    return stat(path, &st) == 0;
}

static int parse_source_file(const char *path, library_info_t *lib) {
    FILE *file = fopen(path, "r");
    if (!file) {
        return -1;
    }
    
    yyin = file;
    
    if (yyparse() != 0) {
        fclose(file);
        return -1;
    }
    
    ast_node_t *ast = get_ast_root();
    if (!ast) {
        fclose(file);
        return -1;
    }
    
    int result = extract_symbols_from_ast(ast, lib);
    
    fclose(file);
    return result;
}

static int parse_bytecode_file(const char *path, library_info_t *lib) {
    bytecode_file_t bytecode_file = {0};
    
    if (bytecode_load_file(&bytecode_file, path) != 0) {
        return -1;
    }
    
    if (!bytecode_verify_file(&bytecode_file)) {
        bytecode_free_file(&bytecode_file);
        return -1;
    }
    
    int result = extract_symbols_from_bytecode(&bytecode_file, lib);
    
    bytecode_free_file(&bytecode_file);
    return result;
}

static int extract_symbols_from_ast(ast_node_t *ast, library_info_t *lib) {
    if (!ast || !lib) return -1;
    
    // 这里需要遍历AST，提取函数和变量定义
    // 简化实现，实际需要根据AST结构来提取
    
    return 0;
}

static int extract_symbols_from_bytecode(bytecode_file_t *bytecode, library_info_t *lib) {
    if (!bytecode || !lib) return -1;
    
    // 这里需要从字节码文件的符号表中提取符号信息
    // 简化实现，实际需要解析字节码的符号表
    
    return 0;
}

static void generate_qualified_names(library_info_t *lib) {
    if (!lib) return;
    
    const char *prefix = lib->has_alias ? lib->alias : lib->name;
    
    for (uint32_t i = 0; i < lib->symbol_count; i++) {
        library_symbol_t *symbol = &lib->symbols[i];
        snprintf(symbol->qualified_name, sizeof(symbol->qualified_name),
                "%s.%s", prefix, symbol->name);
    }
}

static library_info_t* find_library_by_name_or_alias(library_manager_t *mgr, const char *name) {
    if (!mgr || !name) return NULL;
    
    for (uint32_t i = 0; i < mgr->library_count; i++) {
        library_info_t *lib = &mgr->libraries[i];
        
        if (strcmp(lib->name, name) == 0) {
            return lib;
        }
        
        if (lib->has_alias && strcmp(lib->alias, name) == 0) {
            return lib;
        }
    }
    
    return NULL;
}

/* ========== 工具接口实现 ========== */

int libmgr_add_search_path(library_manager_t *mgr, const char *path) {
    if (!mgr || !path || mgr->path_count >= MAX_SEARCH_PATHS) {
        return LIBMGR_ERROR_INVALID_PATH;
    }
    
    strncpy(mgr->search_paths[mgr->path_count], path, MAX_LIBRARY_PATH - 1);
    mgr->search_paths[mgr->path_count][MAX_LIBRARY_PATH - 1] = '\0';
    mgr->path_count++;
    
    return LIBMGR_SUCCESS;
}

const char* libmgr_resolve_library_path(library_manager_t *mgr, const char *name) {
    if (!mgr || !name) return NULL;
    
    static char full_path[MAX_LIBRARY_PATH];
    
    for (uint32_t i = 0; i < mgr->path_count; i++) {
        /* 尝试 .st 文件 */
        snprintf(full_path, sizeof(full_path), "%s/%s.st", mgr->search_paths[i], name);
        if (file_exists(full_path)) {
            return full_path;
        }
        
        /* 尝试 .stbc 文件 */
        snprintf(full_path, sizeof(full_path), "%s/%s.stbc", mgr->search_paths[i], name);
        if (file_exists(full_path)) {
            return full_path;
        }
    }
    
    return NULL;
}

void libmgr_print_loaded_libraries(library_manager_t *mgr) {
    if (!mgr || !mgr->is_initialized) {
        printf("Library manager not initialized\n");
        return;
    }
    
    printf("\n=== Loaded Libraries ===\n");
    for (uint32_t i = 0; i < mgr->library_count; i++) {
        library_info_t *lib = &mgr->libraries[i];
        printf("- %s", lib->name);
        if (lib->has_alias) {
            printf(" (alias: %s)", lib->alias);
        }
        printf(" [%s, %u funcs, %u vars, refs: %u]\n",
               lib->file_type == FILE_TYPE_SOURCE ? "source" : "bytecode",
               lib->function_count, lib->variable_count, lib->reference_count);
    }
    printf("========================\n\n");
}

uint32_t libmgr_get_loaded_count(library_manager_t *mgr) {
    return mgr ? mgr->library_count : 0;
}

const char* libmgr_get_last_error(library_manager_t *mgr) {
    return mgr ? mgr->last_error : "Manager not initialized";
}

static void set_error(library_manager_t *mgr, const char *format, ...) {
    if (!mgr || !format) return;
    
    va_list args;
    va_start(args, format);
    vsnprintf(mgr->last_error, sizeof(mgr->last_error) - 1, format, args);
    va_end(args);
}

/* 内置库注册函数的简化实现 */
static int register_builtin_math_library(library_manager_t *mgr, const char *alias) {
    // 简化实现，实际需要创建内置数学库并注册函数
    return LIBMGR_SUCCESS;
}

static int register_builtin_string_library(library_manager_t *mgr, const char *alias) {
    // 简化实现，实际需要创建内置字符串库并注册函数
    return LIBMGR_SUCCESS;
}

static int register_builtin_io_library(library_manager_t *mgr, const char *alias) {
    // 简化实现，实际需要创建内置IO库并注册函数
    return LIBMGR_SUCCESS;
}
