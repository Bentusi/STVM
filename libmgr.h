#ifndef LIBMGR_H
#define LIBMGR_H

#include <stdint.h>
#include <stdbool.h>
#include "symbol_table.h"

/* 常量定义 */
#define MAX_LIBRARIES 32
#define MAX_LIBRARY_NAME 64
#define MAX_LIBRARY_PATH 256
#define MAX_SYMBOLS_PER_LIB 128
#define MAX_SEARCH_PATHS 16

/* 库状态 */
typedef enum {
    LIB_STATUS_UNLOADED = 0,
    LIB_STATUS_LOADED,
    LIB_STATUS_ERROR
} library_status_t;

/* 库类型 */
typedef enum {
    LIB_TYPE_BUILTIN = 0,    // 内置库
    LIB_TYPE_USER,           // 用户库
    LIB_TYPE_DYNAMIC         // 动态库
} library_type_t;

/* 文件类型 */
typedef enum {
    FILE_TYPE_SOURCE = 0,    // .st源文件
    FILE_TYPE_BYTECODE,      // .stbc字节码文件
    FILE_TYPE_UNKNOWN
} library_file_type_t;

/* 错误码 */
typedef enum {
    LIBMGR_SUCCESS = 0,
    LIBMGR_ERROR_NOT_INITIALIZED,
    LIBMGR_ERROR_LIBRARY_NOT_FOUND,
    LIBMGR_ERROR_INVALID_PATH,
    LIBMGR_ERROR_PARSE_FAILED,
    LIBMGR_ERROR_MEMORY_ALLOCATION,
    LIBMGR_ERROR_SYMBOL_NOT_FOUND,
    LIBMGR_ERROR_MAX_LIBRARIES_REACHED,
    LIBMGR_ERROR_FILE_TYPE_UNKNOWN
} libmgr_error_t;

/* 库符号信息 - 简化版 */
typedef struct library_symbol {
    char name[MAX_LIBRARY_NAME];              // 原始符号名
    char qualified_name[MAX_LIBRARY_NAME*2];  // 限定名（alias.name）
    symbol_type_t type;                       // 符号类型（函数/变量/常量）
    type_info_t *data_type;                   // 数据类型
    void *implementation;                     // 实现指针（函数指针或变量地址）
    bool is_exported;                         // 是否导出
} library_symbol_t;

/* 库信息 - 简化版 */
typedef struct library_info {
    char name[MAX_LIBRARY_NAME];        // 库名
    char alias[MAX_LIBRARY_NAME];       // 别名（如果有）
    char path[MAX_LIBRARY_PATH];        // 文件路径
    
    library_type_t type;                // 库类型
    library_status_t status;            // 加载状态
    library_file_type_t file_type;      // 文件类型
    
    // 符号统计
    uint32_t function_count;            // 函数数量
    uint32_t variable_count;            // 全局变量数量
    uint32_t constant_count;            // 常量数量
    uint32_t total_symbols;             // 总符号数
    
    // 符号存储
    library_symbol_t symbols[MAX_SYMBOLS_PER_LIB];
    uint32_t symbol_count;              // 实际符号数量
    
    bool has_alias;                     // 是否有别名
    uint32_t reference_count;           // 引用计数
} library_info_t;

/* 库管理器 - 简化版 */
typedef struct library_manager {
    library_info_t libraries[MAX_LIBRARIES];        // 库数组
    uint32_t library_count;                         // 库数量
    
    char search_paths[MAX_SEARCH_PATHS][MAX_LIBRARY_PATH]; // 搜索路径
    uint32_t path_count;                            // 路径数量
    
    bool is_initialized;                            // 初始化状态
    char last_error[256];                          // 错误信息
} library_manager_t;

/* ========== 核心接口 ========== */

/* 生命周期管理 */
library_manager_t* libmgr_create(void);
int libmgr_init(library_manager_t *mgr);
void libmgr_destroy(library_manager_t *mgr);
bool libmgr_is_initialized(library_manager_t *mgr);

/* 库文件加载接口（核心功能） */
int libmgr_load_source_library(library_manager_t *mgr, const char *name, 
                               const char *path, const char *alias);
int libmgr_load_bytecode_library(library_manager_t *mgr, const char *name, 
                                 const char *path, const char *alias);
int libmgr_load_library(library_manager_t *mgr, const char *name, 
                        const char *path, const char *alias);
int libmgr_unload_library(library_manager_t *mgr, const char *name_or_alias);

/* 符号查找接口（支持别名） */
library_symbol_t* libmgr_find_symbol(library_manager_t *mgr, const char *symbol_name);
library_symbol_t* libmgr_find_function(library_manager_t *mgr, const char *func_name);
library_symbol_t* libmgr_find_variable(library_manager_t *mgr, const char *var_name);
bool libmgr_symbol_exists(library_manager_t *mgr, const char *symbol_name);

/* 库信息查询接口 */
library_info_t* libmgr_get_library_info(library_manager_t *mgr, const char *name_or_alias);
int libmgr_get_library_stats(library_manager_t *mgr, const char *name_or_alias,
                             uint32_t *func_count, uint32_t *var_count);
int libmgr_list_library_symbols(library_manager_t *mgr, const char *name_or_alias,
                                library_symbol_t **symbols, uint32_t *count);

/* 路径管理 */
int libmgr_add_search_path(library_manager_t *mgr, const char *path);
const char* libmgr_resolve_library_path(library_manager_t *mgr, const char *name);

/* 内置库注册接口 */
int libmgr_register_builtin_library(library_manager_t *mgr, const char *name, const char *alias);
int libmgr_load_builtin_libraries(library_manager_t *mgr);

/* 工具接口 */
void libmgr_print_loaded_libraries(library_manager_t *mgr);
uint32_t libmgr_get_loaded_count(library_manager_t *mgr);
const char* libmgr_get_last_error(library_manager_t *mgr);

#endif /* LIBMGR_H */
