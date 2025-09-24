#include "symbol_table.h"
#include "libmgr.h"
#include "vm.h"
#include "mmgr.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* 全局库管理器实例 */
static library_manager_t g_libmgr = {0};
static char g_last_error[512] = {0};

/* 内部辅助函数声明 */
static library_info_t* create_library_info(const char *name, const char *version, library_type_t type);
static library_symbol_t* create_library_symbol(const char *name, symbol_type_t type);
static bool add_symbol_to_library(library_info_t *library, library_symbol_t *symbol);
static uint32_t hash_library_name(const char *name);
static void update_cache(library_symbol_t *symbol, library_info_t *library);

/* ========== 库管理器初始化 ========== */

/* 初始化库管理器 */
int libmgr_init(uint32_t init_flags) {
    if (g_libmgr.is_initialized) {
        return 0; // 已经初始化
    }
    
    /* 清零管理器结构 */
    memset(&g_libmgr, 0, sizeof(library_manager_t));
    
    /* 设置初始化标志 */
    g_libmgr.init_flags = init_flags;
    
    /* 添加默认搜索路径 */
    libmgr_add_search_path("./libs");
    libmgr_add_search_path("/usr/lib/stlibs");
    libmgr_add_search_path("/usr/local/lib/stlibs");
    
    /* 初始化内置库 */
    if (init_flags & LIBMGR_INIT_BUILTIN_MATH) {
        if (libmgr_init_builtin_math() != 0) {
            libmgr_set_error("Failed to initialize builtin math library");
            return -1;
        }
    }
    
    if (init_flags & LIBMGR_INIT_BUILTIN_STRING) {
        if (libmgr_init_builtin_string() != 0) {
            libmgr_set_error("Failed to initialize builtin string library");
            return -1;
        }
    }
    
    if (init_flags & LIBMGR_INIT_BUILTIN_IO) {
        if (libmgr_init_builtin_io() != 0) {
            libmgr_set_error("Failed to initialize builtin I/O library");
            return -1;
        }
    }
    
    if (init_flags & LIBMGR_INIT_BUILTIN_TIME) {
        if (libmgr_init_builtin_time() != 0) {
            libmgr_set_error("Failed to initialize builtin time library");
            return -1;
        }
    }
    
    if (init_flags & LIBMGR_INIT_BUILTIN_CONTROL) {
        if (libmgr_init_builtin_control() != 0) {
            libmgr_set_error("Failed to initialize builtin control library");
            return -1;
        }
    }
    
    g_libmgr.is_initialized = true;
    
    return 0;
}

/* 清理库管理器 */
void libmgr_cleanup(void) {
    if (!g_libmgr.is_initialized) {
        return;
    }
    
    /* 卸载所有库 */
    for (uint32_t i = 0; i < g_libmgr.library_count; i++) {
        library_info_t *lib = &g_libmgr.libraries[i];
        if (lib->library_scope) {
            // 作用域清理由符号表管理器处理
        }
    }
    
    /* 重置管理器状态 */
    memset(&g_libmgr, 0, sizeof(library_manager_t));
}

/* ========== 库操作实现 ========== */

/* 创建库信息结构 */
static library_info_t* create_library_info(const char *name, const char *version, library_type_t type) {
    if (g_libmgr.library_count >= MAX_LIBRARIES) {
        libmgr_set_error("Maximum number of libraries reached");
        return NULL;
    }
    
    library_info_t *lib = &g_libmgr.libraries[g_libmgr.library_count];
    memset(lib, 0, sizeof(library_info_t));
    
    strncpy(lib->name, name, MAX_LIBRARY_NAME - 1);
    strncpy(lib->version, version, MAX_LIBRARY_VERSION - 1);
    lib->type = type;
    lib->status = LIB_STATUS_UNLOADED;
    lib->load_time = time(NULL);
    
    g_libmgr.library_count++;
    
    return lib;
}

/* 加载库 */
library_info_t* libmgr_load_library(const char *name, const char *path) {
    if (!name || !g_libmgr.is_initialized) {
        return NULL;
    }
    
    /* 检查库是否已加载 */
    library_info_t *existing = libmgr_find_library(name);
    if (existing && existing->status == LIB_STATUS_LOADED) {
        existing->reference_count++;
        return existing;
    }
    
    /* 创建新的库信息 */
    library_info_t *lib = create_library_info(name, "1.0", LIB_TYPE_USER);
    if (!lib) {
        return NULL;
    }
    
    /* 设置路径 */
    if (path) {
        strncpy(lib->path, path, MAX_LIBRARY_PATH - 1);
    } else {
        const char *resolved_path = libmgr_resolve_library_path(name);
        if (resolved_path) {
            strncpy(lib->path, resolved_path, MAX_LIBRARY_PATH - 1);
        }
    }
    
    /* 解析库文件 */
    if (libmgr_parse_library_file(lib->path, lib) != 0) {
        lib->status = LIB_STATUS_ERROR;
        snprintf(lib->last_error, sizeof(lib->last_error), 
                "Failed to parse library file: %s", lib->path);
        return NULL;
    }
    
    /* 创建库作用域 */
    lib->library_scope = enter_scope(name, SCOPE_LIBRARY);
    if (!lib->library_scope) {
        lib->status = LIB_STATUS_ERROR;
        strncpy(lib->last_error, "Failed to create library scope", sizeof(lib->last_error) - 1);
        return NULL;
    }
    
    /* 解析依赖 */
    if (!libmgr_resolve_dependencies(name)) {
        lib->status = LIB_STATUS_ERROR;
        strncpy(lib->last_error, "Failed to resolve dependencies", sizeof(lib->last_error) - 1);
        exit_scope(); // 清理作用域
        return NULL;
    }
    
    lib->status = LIB_STATUS_LOADED;
    lib->reference_count = 1;
    lib->is_initialized = true;
    
    exit_scope(); // 退出库作用域
    
    return lib;
}

/* 查找库 */
library_info_t* libmgr_find_library(const char *name) {
    if (!name || !g_libmgr.is_initialized) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < g_libmgr.library_count; i++) {
        if (strcmp(g_libmgr.libraries[i].name, name) == 0) {
            return &g_libmgr.libraries[i];
        }
    }
    
    return NULL;
}

/* ========== 导入管理 ========== */

/* 导入库 */
import_info_t* libmgr_import_library(const char *name, const char *alias, const char *path) {
    if (!name || !g_libmgr.is_initialized) {
        return NULL;
    }
    
    if (g_libmgr.import_count >= MAX_LIBRARIES) {
        libmgr_set_error("Maximum number of imports reached");
        return NULL;
    }
    
    /* 加载库 */
    library_info_t *lib = libmgr_load_library(name, path);
    if (!lib) {
        return NULL;
    }
    
    /* 创建导入信息 */
    import_info_t *import = &g_libmgr.imports[g_libmgr.import_count];
    memset(import, 0, sizeof(import_info_t));
    
    strncpy(import->library_name, name, MAX_LIBRARY_NAME - 1);
    if (alias) {
        strncpy(import->alias, alias, MAX_LIBRARY_NAME - 1);
    }
    if (path) {
        strncpy(import->path, path, MAX_LIBRARY_PATH - 1);
    }
    
    import->mode = IMPORT_MODE_ALL;
    import->library = lib;
    import->is_successful = true;
    
    g_libmgr.import_count++;
    
    /* 将库符号导入到当前作用域 */
    scope_t *current_scope = get_current_scope();
    if (current_scope && lib->library_scope) {
        for (uint32_t i = 0; i < lib->symbol_count; i++) {
            library_symbol_t *lib_symbol = &lib->symbols[i];
            if (lib_symbol->is_exported) {
                /* 创建符号并添加到当前作用域 */
                const char *symbol_name = alias ? alias : lib_symbol->name;
                symbol_t *symbol = add_symbol(symbol_name, lib_symbol->symbol_type, lib_symbol->data_type);
                if (symbol) {
                    symbol->is_builtin = (lib->type == LIB_TYPE_BUILTIN);
                    if (lib_symbol->symbol_type == SYMBOL_FUNCTION) {
                        symbol->info.func_info.implementation = lib_symbol->implementation.builtin_func;
                        symbol->info.func_info.library_name = mmgr_alloc_string(lib->name);
                    }
                }
            }
        }
    }
    
    return import;
}

/* 解析导入列表 */
bool libmgr_resolve_imports(ast_node_t *import_list) {
    if (!import_list || import_list->type != AST_IMPORT_LIST) {
        return true; // 空导入列表是合法的
    }
    
    /* 遍历导入节点 */
    for (uint32_t i = 0; i < ast_list_size(import_list); i++) {
        ast_node_t *import_node = ast_list_get(import_list, i);
        if (import_node && import_node->type == AST_IMPORT) {
            const char *lib_name = import_node->data.import.library_name;
            const char *alias = import_node->data.import.alias;
            const char *path = import_node->data.import.path;
            
            import_info_t *import = libmgr_import_library(lib_name, alias, path);
            if (!import) {
                return false;
            }
        }
    }
    
    return true;
}

/* ========== 符号查找 ========== */

/* 查找符号 */
library_symbol_t* libmgr_lookup_symbol(const char *name) {
    if (!name || !g_libmgr.is_initialized) {
        return NULL;
    }
    
    g_libmgr.statistics.symbol_lookups++;
    
    /* 检查缓存 */
    if (g_libmgr.cache.last_symbol && 
        strcmp(g_libmgr.cache.last_symbol->name, name) == 0) {
        g_libmgr.statistics.cache_hits++;
        return g_libmgr.cache.last_symbol;
    }
    
    /* 在所有已加载的库中查找 */
    for (uint32_t i = 0; i < g_libmgr.library_count; i++) {
        library_info_t *lib = &g_libmgr.libraries[i];
        if (lib->status == LIB_STATUS_LOADED) {
            for (uint32_t j = 0; j < lib->symbol_count; j++) {
                library_symbol_t *symbol = &lib->symbols[j];
                if (strcmp(symbol->name, name) == 0 && symbol->is_exported) {
                    update_cache(symbol, lib);
                    symbol->usage_count++;
                    return symbol;
                }
            }
        }
    }
    
    return NULL;
}

/* 查找限定符号 */
library_symbol_t* libmgr_lookup_qualified_symbol(const char *qualified_name) {
    if (!qualified_name || !g_libmgr.is_initialized) {
        return NULL;
    }
    
    /* 解析限定名称 library.symbol */
    char *dot = strchr(qualified_name, '.');
    if (!dot) {
        return libmgr_lookup_symbol(qualified_name); // 非限定名称
    }
    
    /* 分离库名和符号名 */
    size_t lib_name_len = dot - qualified_name;
    char lib_name[MAX_LIBRARY_NAME];
    strncpy(lib_name, qualified_name, lib_name_len);
    lib_name[lib_name_len] = '\0';
    
    char *symbol_name = dot + 1;
    
    return libmgr_lookup_in_library(lib_name, symbol_name);
}

/* 在指定库中查找符号 */
library_symbol_t* libmgr_lookup_in_library(const char *library_name, const char *symbol_name) {
    if (!library_name || !symbol_name || !g_libmgr.is_initialized) {
        return NULL;
    }
    
    library_info_t *lib = libmgr_find_library(library_name);
    if (!lib || lib->status != LIB_STATUS_LOADED) {
        return NULL;
    }
    
    for (uint32_t i = 0; i < lib->symbol_count; i++) {
        library_symbol_t *symbol = &lib->symbols[i];
        if (strcmp(symbol->name, symbol_name) == 0 && symbol->is_exported) {
            symbol->usage_count++;
            return symbol;
        }
    }
    
    return NULL;
}

/* ========== 内置库实现 ========== */

/* 内置数学函数实现 */
static int builtin_sin(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1 || args[0].type != VAL_REAL) {
        return -1; // 参数错误
    }
    
    result->type = VAL_REAL;
    result->data.real_val = sin(args[0].data.real_val);
    return 0;
}

static int builtin_cos(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1 || args[0].type != VAL_REAL) {
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = cos(args[0].data.real_val);
    return 0;
}

static int builtin_sqrt(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1 || args[0].type != VAL_REAL) {
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = sqrt(args[0].data.real_val);
    return 0;
}

static int builtin_abs_real(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1 || args[0].type != VAL_REAL) {
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = fabs(args[0].data.real_val);
    return 0;
}

/* 初始化内置数学库 */
int libmgr_init_builtin_math(void) {
    library_info_t *math_lib = create_library_info("MathLib", "1.0.0", LIB_TYPE_BUILTIN);
    if (!math_lib) {
        return -1;
    }
    
    strncpy(math_lib->description, "Built-in mathematical functions", sizeof(math_lib->description) - 1);
    math_lib->status = LIB_STATUS_LOADED;
    math_lib->is_initialized = true;
    
    /* 注册数学函数 */
    type_info_t *real_type = create_basic_type(TYPE_REAL_ID);
    
    if (!libmgr_register_builtin_function("MathLib", "Sin", builtin_sin, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "Cos", builtin_cos, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "Sqrt", builtin_sqrt, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "Abs", builtin_abs_real, real_type, &real_type, 1)) {
        return -1;
    }
    
    /* 注册数学常量 */
    double pi_value = 3.14159265359;
    double e_value = 2.71828182846;
    
    if (!libmgr_register_builtin_constant("MathLib", "PI", real_type, &pi_value) ||
        !libmgr_register_builtin_constant("MathLib", "E", real_type, &e_value)) {
        return -1;
    }
    
    g_libmgr.builtin_math_index = g_libmgr.library_count - 1;
    g_libmgr.statistics.builtin_symbols += math_lib->symbol_count;
    
    return 0;
}

/* 内置字符串函数实现 */
static int builtin_strlen(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1 || args[0].type != VAL_STRING) {
        return -1;
    }
    
    result->type = VAL_INT;
    result->data.int_val = strlen(args[0].data.string_val);
    return 0;
}

static int builtin_strcmp(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
        return -1;
    }
    
    result->type = VAL_INT;
    result->data.int_val = strcmp(args[0].data.string_val, args[1].data.string_val);
    return 0;
}

/* 初始化内置字符串库 */
int libmgr_init_builtin_string(void) {
    library_info_t *string_lib = create_library_info("StringLib", "1.0.0", LIB_TYPE_BUILTIN);
    if (!string_lib) {
        return -1;
    }
    
    strncpy(string_lib->description, "Built-in string manipulation functions", sizeof(string_lib->description) - 1);
    string_lib->status = LIB_STATUS_LOADED;
    string_lib->is_initialized = true;
    
    /* 注册字符串函数 */
    type_info_t *string_type = create_basic_type(TYPE_STRING_ID);
    type_info_t *int_type = create_basic_type(TYPE_INT_ID);
    type_info_t *string_params[] = {string_type, string_type};
    
    if (!libmgr_register_builtin_function("StringLib", "Len", builtin_strlen, int_type, &string_type, 1) ||
        !libmgr_register_builtin_function("StringLib", "Compare", builtin_strcmp, int_type, string_params, 2)) {
        return -1;
    }
    
    g_libmgr.builtin_string_index = g_libmgr.library_count - 1;
    g_libmgr.statistics.builtin_symbols += string_lib->symbol_count;
    
    return 0;
}

/* 初始化内置I/O库 */
int libmgr_init_builtin_io(void) {
    library_info_t *io_lib = create_library_info("IOLib", "1.0.0", LIB_TYPE_BUILTIN);
    if (!io_lib) {
        return -1;
    }
    
    strncpy(io_lib->description, "Built-in I/O functions", sizeof(io_lib->description) - 1);
    io_lib->status = LIB_STATUS_LOADED;
    io_lib->is_initialized = true;
    
    g_libmgr.builtin_io_index = g_libmgr.library_count - 1;
    
    return 0;
}

/* 初始化内置时间库 */
int libmgr_init_builtin_time(void) {
    library_info_t *time_lib = create_library_info("TimeLib", "1.0.0", LIB_TYPE_BUILTIN);
    if (!time_lib) {
        return -1;
    }
    
    strncpy(time_lib->description, "Built-in time functions", sizeof(time_lib->description) - 1);
    time_lib->status = LIB_STATUS_LOADED;
    time_lib->is_initialized = true;
    
    g_libmgr.builtin_time_index = g_libmgr.library_count - 1;
    
    return 0;
}

/* 初始化内置控制库 */
int libmgr_init_builtin_control(void) {
    library_info_t *control_lib = create_library_info("ControlLib", "1.0.0", LIB_TYPE_BUILTIN);
    if (!control_lib) {
        return -1;
    }
    
    strncpy(control_lib->description, "Built-in control functions", sizeof(control_lib->description) - 1);
    control_lib->status = LIB_STATUS_LOADED;
    control_lib->is_initialized = true;
    
    g_libmgr.builtin_control_index = g_libmgr.library_count - 1;
    
    return 0;
}

/* ========== 符号注册 ========== */

/* 注册内置函数 */
bool libmgr_register_builtin_function(const char *library_name, const char *func_name,
                                      builtin_function_t func_ptr, type_info_t *return_type,
                                      type_info_t **param_types, uint32_t param_count) {
    if (!library_name || !func_name || !func_ptr) {
        return false;
    }
    
    library_info_t *lib = libmgr_find_library(library_name);
    if (!lib) {
        return false;
    }
    
    if (lib->symbol_count >= MAX_EXPORTED_SYMBOLS) {
        return false;
    }
    
    library_symbol_t *symbol = &lib->symbols[lib->symbol_count];
    memset(symbol, 0, sizeof(library_symbol_t));
    
    strncpy(symbol->name, func_name, MAX_LIBRARY_NAME - 1);
    symbol->symbol_type = SYMBOL_FUNCTION;
    symbol->data_type = libmgr_create_func_type(return_type, param_types, param_count);
    symbol->implementation.builtin_func = func_ptr;
    symbol->is_exported = true;
    
    lib->symbol_count++;
    g_libmgr.statistics.total_symbols++;
    
    return true;
}

/* 注册内置常量 */
bool libmgr_register_builtin_constant(const char *library_name, const char *const_name,
                                      type_info_t *type, void *value) {
    if (!library_name || !const_name || !type || !value) {
        return false;
    }
    
    library_info_t *lib = libmgr_find_library(library_name);
    if (!lib || lib->symbol_count >= MAX_EXPORTED_SYMBOLS) {
        return false;
    }
    
    library_symbol_t *symbol = &lib->symbols[lib->symbol_count];
    memset(symbol, 0, sizeof(library_symbol_t));
    
    strncpy(symbol->name, const_name, MAX_LIBRARY_NAME - 1);
    symbol->symbol_type = SYMBOL_CONSTANT;
    symbol->data_type = type;
    symbol->implementation.variable_ptr = value;
    symbol->is_exported = true;
    
    lib->symbol_count++;
    g_libmgr.statistics.total_symbols++;
    
    return true;
}

/* ========== 辅助函数 ========== */

/* 创建函数类型 */
type_info_t* libmgr_create_func_type(type_info_t *return_type, type_info_t **param_types, uint32_t param_count) {
    type_info_t *func_type = (type_info_t*)mmgr_alloc_type_info(sizeof(type_info_t));
    if (!func_type) {
        return NULL;
    }
    
    memset(func_type, 0, sizeof(type_info_t));
    func_type->base_type = TYPE_FUNCTION_ID;
    func_type->size = sizeof(void*);
    func_type->is_constant = true;
    
    func_type->compound.function_info.return_type = return_type;
    func_type->compound.function_info.parameter_count = param_count;
    
    if (param_count > 0 && param_types) {
        // 简化实现：这里可以存储参数类型信息
    }
    
    return func_type;
}

/* 更新缓存 */
static void update_cache(library_symbol_t *symbol, library_info_t *library) {
    g_libmgr.cache.last_symbol = symbol;
    g_libmgr.cache.last_library = library;
    g_libmgr.cache.cache_version++;
}

/* 添加搜索路径 */
bool libmgr_add_search_path(const char *path) {
    if (!path || g_libmgr.path_count >= 16) {
        return false;
    }
    
    strncpy(g_libmgr.search_paths[g_libmgr.path_count], path, MAX_LIBRARY_PATH - 1);
    g_libmgr.path_count++;
    
    return true;
}

/* 解析库路径 */
const char* libmgr_resolve_library_path(const char *name) {
    if (!name) {
        return NULL;
    }
    
    static char full_path[MAX_LIBRARY_PATH];
    
    for (uint32_t i = 0; i < g_libmgr.path_count; i++) {
        snprintf(full_path, sizeof(full_path), "%s/%s.st", g_libmgr.search_paths[i], name);
        // 这里可以检查文件是否存在
        return full_path; // 简化实现，返回第一个候选路径
    }
    
    return NULL;
}

/* 解析库文件 */
int libmgr_parse_library_file(const char *filepath, library_info_t *lib_info) {
    // 简化实现：这里应该解析ST库文件格式
    // 现在只是标记为成功
    return 0;
}

/* 解析依赖 */
bool libmgr_resolve_dependencies(const char *library_name) {
    // 简化实现：假设没有依赖或依赖已解析
    return true;
}

/* ========== 错误处理 ========== */

/* 设置错误信息 */
void libmgr_set_error(const char *error_message) {
    if (error_message) {
        strncpy(g_last_error, error_message, sizeof(g_last_error) - 1);
        g_last_error[sizeof(g_last_error) - 1] = '\0';
    }
}

/* 获取最后的错误信息 */
const char* libmgr_get_last_error(void) {
    return g_last_error;
}

/* ========== 调试和统计 ========== */

/* 打印统计信息 */
void libmgr_print_statistics(void) {
    if (!g_libmgr.is_initialized) {
        printf("Library manager not initialized\n");
        return;
    }
    
    printf("\n=== Library Manager Statistics ===\n");
    printf("Loaded libraries: %u\n", g_libmgr.library_count);
    printf("Active imports: %u\n", g_libmgr.import_count);
    printf("Total symbols: %u\n", g_libmgr.statistics.total_symbols);
    printf("Builtin symbols: %u\n", g_libmgr.statistics.builtin_symbols);
    printf("Imported symbols: %u\n", g_libmgr.statistics.imported_symbols);
    printf("Symbol lookups: %u\n", g_libmgr.statistics.symbol_lookups);
    printf("Cache hits: %u (%.1f%%)\n", g_libmgr.statistics.cache_hits,
           g_libmgr.statistics.symbol_lookups > 0 ? 
           (100.0 * g_libmgr.statistics.cache_hits / g_libmgr.statistics.symbol_lookups) : 0.0);
    printf("==================================\n\n");
}

/* 打印已加载的库 */
void libmgr_print_loaded_libraries(void) {
    if (!g_libmgr.is_initialized) {
        printf("Library manager not initialized\n");
        return;
    }
    
    printf("\n=== Loaded Libraries ===\n");
    for (uint32_t i = 0; i < g_libmgr.library_count; i++) {
        library_info_t *lib = &g_libmgr.libraries[i];
        printf("- %s v%s (%s, %u symbols, refs: %u)\n",
               lib->name, lib->version,
               lib->type == LIB_TYPE_BUILTIN ? "builtin" : "user",
               lib->symbol_count, lib->reference_count);
    }
    printf("========================\n\n");
}

/* 检查是否已初始化 */
bool libmgr_is_initialized(void) {
    return g_libmgr.is_initialized;
}
