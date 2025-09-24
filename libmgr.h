#ifndef LIBMGR_H
#define LIBMGR_H

#include "ast.h"
#include "symbol_table.h"
#include "mmgr.h"
#include <stdint.h>
#include <stdbool.h>

/* 库信息结构 - 静态内存版 */
#define MAX_LIBRARY_NAME 64
#define MAX_LIBRARY_VERSION 16
#define MAX_LIBRARY_PATH 256
#define MAX_LIBRARIES 50
#define MAX_EXPORTED_SYMBOLS 200
#define MAX_LIBRARY_DEPENDENCIES 10

/* 库类型枚举 */
typedef enum {
    LIB_TYPE_BUILTIN,       // 内置库
    LIB_TYPE_SYSTEM,        // 系统库
    LIB_TYPE_USER,          // 用户库
    LIB_TYPE_DYNAMIC        // 动态库
} library_type_t;

/* 库状态枚举 */
typedef enum {
    LIB_STATUS_UNLOADED,    // 未加载
    LIB_STATUS_LOADING,     // 加载中
    LIB_STATUS_LOADED,      // 已加载
    LIB_STATUS_ERROR        // 错误状态
} library_status_t;

/* 导入模式枚举 */
typedef enum {
    IMPORT_MODE_ALL,        // 导入所有符号
    IMPORT_MODE_SELECTIVE,  // 选择性导入
    IMPORT_MODE_ALIASED     // 别名导入
} import_mode_t;

/* 内置函数指针类型 */
struct vm_value;
typedef int (*builtin_function_t)(struct vm_value *args, uint32_t arg_count, struct vm_value *result);

/* 库符号信息 */
typedef struct library_symbol {
    char name[MAX_LIBRARY_NAME];        // 符号名
    char alias[MAX_LIBRARY_NAME];       // 别名
    symbol_type_t symbol_type;          // 符号类型
    type_info_t *data_type;             // 数据类型
    
    union {
        builtin_function_t builtin_func;    // 内置函数指针
        void *user_func_ptr;                // 用户函数指针
        void *variable_ptr;                 // 变量指针
    } implementation;
    
    bool is_exported;                   // 是否导出
    bool is_deprecated;                 // 是否已弃用
    uint32_t usage_count;               // 使用计数
    
    struct library_symbol *next;        // 链表下一项
} library_symbol_t;

/* 库依赖信息 */
typedef struct library_dependency {
    char name[MAX_LIBRARY_NAME];        // 依赖库名
    char version[MAX_LIBRARY_VERSION];  // 版本要求
    bool is_optional;                   // 是否可选
    bool is_loaded;                     // 是否已加载
} library_dependency_t;

/* 库信息结构 */
typedef struct library_info {
    char name[MAX_LIBRARY_NAME];        // 库名
    char version[MAX_LIBRARY_VERSION];  // 版本
    char path[MAX_LIBRARY_PATH];        // 路径
    char description[256];              // 描述
    
    library_type_t type;                // 库类型
    library_status_t status;            // 库状态
    
    /* 符号管理 */
    library_symbol_t symbols[MAX_EXPORTED_SYMBOLS];
    uint32_t symbol_count;              // 符号数量
    
    /* 依赖管理 */
    library_dependency_t dependencies[MAX_LIBRARY_DEPENDENCIES];
    uint32_t dependency_count;          // 依赖数量
    
    /* 作用域信息 */
    scope_t *library_scope;             // 库作用域
    
    /* 加载信息 */
    uint64_t load_time;                 // 加载时间
    uint32_t reference_count;           // 引用计数
    bool is_initialized;                // 是否已初始化
    
    /* 错误信息 */
    char last_error[256];               // 最后错误信息
    
    struct library_info *next;          // 链表下一项
} library_info_t;

/* 导入信息结构 */
typedef struct import_info {
    char library_name[MAX_LIBRARY_NAME]; // 库名
    char alias[MAX_LIBRARY_NAME];        // 别名
    char path[MAX_LIBRARY_PATH];         // 路径
    
    import_mode_t mode;                  // 导入模式
    library_info_t *library;             // 关联的库
    
    /* 选择性导入的符号列表 */
    char selected_symbols[32][MAX_LIBRARY_NAME];
    uint32_t selected_count;             // 选择的符号数量
    
    bool is_successful;                  // 导入是否成功
    
    struct import_info *next;            // 链表下一项
} import_info_t;

/* 库管理器 - 静态内存版 */
typedef struct library_manager {
    /* 库列表 */
    library_info_t libraries[MAX_LIBRARIES];
    uint32_t library_count;
    
    /* 导入列表 */
    import_info_t imports[MAX_LIBRARIES];
    uint32_t import_count;
    
    /* 内置库索引 */
    uint32_t builtin_math_index;
    uint32_t builtin_string_index;
    uint32_t builtin_io_index;
    uint32_t builtin_time_index;
    uint32_t builtin_control_index;
    
    /* 搜索路径 */
    char search_paths[16][MAX_LIBRARY_PATH];
    uint32_t path_count;
    
    /* 符号表集成 */
    symbol_table_manager_t *symbol_mgr;
    
    /* 统计信息 */
    struct {
        uint32_t total_symbols;         // 总符号数
        uint32_t builtin_symbols;       // 内置符号数
        uint32_t imported_symbols;      // 导入符号数
        uint32_t symbol_lookups;        // 符号查找次数
        uint32_t cache_hits;            // 缓存命中次数
    } statistics;
    
    /* 缓存管理 */
    struct {
        library_symbol_t *last_symbol;  // 最后查找的符号
        library_info_t *last_library;   // 最后访问的库
        uint32_t cache_version;         // 缓存版本
    } cache;
    
    /* 管理器状态 */
    bool is_initialized;                // 是否已初始化
    uint32_t init_flags;                // 初始化标志
    
} library_manager_t;

/* 库查找结果 */
typedef struct library_lookup_result {
    library_symbol_t *symbol;           // 找到的符号
    library_info_t *library;            // 符号所在的库
    char resolved_name[MAX_LIBRARY_NAME]; // 解析后的名称
    bool is_exact_match;                // 是否精确匹配
    uint32_t match_score;               // 匹配分数
} library_lookup_result_t;

/* 库管理器初始化标志 */
#define LIBMGR_INIT_BUILTIN_MATH    0x0001
#define LIBMGR_INIT_BUILTIN_STRING  0x0002
#define LIBMGR_INIT_BUILTIN_IO      0x0004
#define LIBMGR_INIT_BUILTIN_TIME    0x0008
#define LIBMGR_INIT_BUILTIN_CONTROL 0x0010
#define LIBMGR_INIT_ALL_BUILTINS    0x001F

/* 库管理接口 */
int libmgr_init(uint32_t init_flags);
void libmgr_cleanup(void);
bool libmgr_is_initialized(void);

/* 库操作 */
library_info_t* libmgr_load_library(const char *name, const char *path);
bool libmgr_unload_library(const char *name);
library_info_t* libmgr_find_library(const char *name);
bool libmgr_reload_library(const char *name);

/* 导入管理 */
import_info_t* libmgr_import_library(const char *name, const char *alias, const char *path);
bool libmgr_import_selective(const char *library_name, const char **symbol_names, uint32_t count);
bool libmgr_resolve_imports(ast_node_t *import_list);
import_info_t* libmgr_find_import(const char *name);

/* 符号查找 */
library_symbol_t* libmgr_lookup_symbol(const char *name);
library_symbol_t* libmgr_lookup_qualified_symbol(const char *qualified_name);
library_lookup_result_t* libmgr_lookup_symbol_detailed(const char *name);
library_symbol_t* libmgr_lookup_in_library(const char *library_name, const char *symbol_name);

/* 符号注册 */
bool libmgr_register_builtin_function(const char *library_name, const char *func_name,
                                      builtin_function_t func_ptr, type_info_t *return_type,
                                      type_info_t **param_types, uint32_t param_count);
bool libmgr_register_builtin_constant(const char *library_name, const char *const_name,
                                      type_info_t *type, void *value);
bool libmgr_register_user_symbol(const char *library_name, const char *symbol_name,
                                 symbol_type_t type, void *implementation);

/* 内置库初始化 */
int libmgr_init_builtin_math(void);
int libmgr_init_builtin_string(void);
int libmgr_init_builtin_io(void);
int libmgr_init_builtin_time(void);
int libmgr_init_builtin_control(void);

/* 路径管理 */
bool libmgr_add_search_path(const char *path);
bool libmgr_remove_search_path(const char *path);
const char* libmgr_resolve_library_path(const char *name);

/* 依赖管理 */
bool libmgr_add_dependency(const char *library_name, const char *dependency_name, const char *version);
bool libmgr_resolve_dependencies(const char *library_name);
bool libmgr_check_circular_dependencies(void);

/* 版本管理 */
bool libmgr_check_version_compatibility(const char *required_version, const char *available_version);
int libmgr_compare_versions(const char *version1, const char *version2);

/* 库文件解析 */
int libmgr_parse_library_file(const char *filepath, library_info_t *lib_info);
int libmgr_save_library_info(const library_info_t *lib_info, const char *filepath);

/* 错误处理 */
const char* libmgr_get_last_error(void);
void libmgr_set_error(const char *error_message);
void libmgr_clear_error(void);

/* 统计和调试 */
void libmgr_print_statistics(void);
void libmgr_print_loaded_libraries(void);
void libmgr_print_library_symbols(const char *library_name);
void libmgr_dump_symbol_table(void);

/* 缓存管理 */
void libmgr_clear_cache(void);
void libmgr_update_cache_stats(void);

/* 验证和诊断 */
bool libmgr_validate_libraries(void);
bool libmgr_check_symbol_conflicts(void);
void libmgr_diagnose_import_issues(void);

/* 序列化支持 */
int libmgr_export_library_list(const char *filename);
int libmgr_import_library_list(const char *filename);

/* 内置函数类型创建辅助 */
type_info_t* libmgr_create_func_type(type_info_t *return_type, type_info_t **param_types, uint32_t param_count);

#endif /* LIBMGR_H */
