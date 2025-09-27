#ifndef MMGR_H
#define MMGR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* 静态MMGR_内存池配置 */
#define MMGR_MAX_STRING_POOL_SIZE    (64 * 1024)    // 64KB字符串池
#define MMGR_MAX_STRINGS             2000           // 最大字符串数量
#define MMGR_MAX_AST_NODES           5000           // 最大AST节点数量
#define MMGR_MAX_SYMBOLS             1000           // 最大符号数量
#define MMGR_MAX_TYPE_INFO           500            // 最大类型信息数量
#define MMGR_MAX_LIBRARIES           50             // 最大库数量
#define MMGR_MAX_BYTECODE_SIZE       (1024 * 1024) // 1MB字节码池
#define MMGR_MAX_CONST_POOL_SIZE     (64 * 1024)   // 64KB常量池

/* 内存块状态 */
typedef enum {
    MMGR_BLOCK_FREE = 0,
    MMGR_BLOCK_ALLOCATED = 1
} mmgr_block_state_t;

/* 内存池类型枚举 */
typedef enum {
    MMGR_POOL_STRING = 0,       // 字符串池
    MMGR_POOL_AST_NODE,         // AST节点池
    MMGR_POOL_SYMBOL,           // 符号池
    MMGR_POOL_TYPE_INFO,        // 类型信息池
    MMGR_POOL_LIBRARY,          // 库信息池
    MMGR_POOL_BYTECODE,         // 字节码池
    MMGR_POOL_CONST,            // 常量池
    MMGR_POOL_COUNT             // 池数量
} mmgr_pool_type_t;

/* 字符串池管理 */
typedef struct mmgr_string_pool {
    char data[MMGR_MAX_STRING_POOL_SIZE];           // 字符串数据池
    uint32_t used_size;                        // 已使用的大小
    
    struct string_entry {
        uint32_t offset;                       // 在池中的偏移
        uint32_t length;                       // 字符串长度
        uint32_t ref_count;                    // 引用计数
        bool is_allocated;                     // 是否已分配
    } entries[MMGR_MAX_STRINGS];
    
    uint32_t entry_count;                      // 条目数量
    uint32_t free_entry_list[MMGR_MAX_STRINGS];     // 空闲条目列表
    uint32_t free_entry_count;                 // 空闲条目数量
} mmgr_string_pool_t;

/* 固定大小块池管理 */
typedef struct mmgr_fixed_block_pool {
    void *memory_base;                         // 内存基地址
    uint32_t block_size;                       // 块大小
    uint32_t max_blocks;                       // 最大块数量
    uint32_t allocated_count;                  // 已分配块数量
    
    mmgr_block_state_t *block_states;          // 块状态数组
    uint32_t *free_block_list;                 // 空闲块列表
    uint32_t free_block_count;                 // 空闲块数量
    uint32_t next_free_index;                  // 下个空闲索引
} mmgr_fixed_block_pool_t;

/* 可变大小池管理 */
typedef struct mmgr_variable_pool {
    uint8_t *memory_base;                      // 内存基地址
    uint32_t pool_size;                        // 池总大小
    uint32_t used_size;                        // 已使用大小
    
    struct mmgr_allocation_header {
        uint32_t size;                         // 分配大小
        uint32_t magic;                        // 魔数（用于检测）
        bool is_free;                          // 是否空闲
    } *allocations[1000];                      // 分配记录
    
    uint32_t allocation_count;                 // 分配记录数量
} mmgr_variable_pool_t;

/* 通用内存池管理 */
typedef struct mmgr_general_pool {
    uint8_t *pool;                             // 内存池指针
    size_t total_size;                         // 池总大小
    size_t used;                               // 已使用大小
    uint32_t allocations;                      // 分配次数
} mmgr_general_pool_t;

/* 内存统计信息 */
typedef struct mmgr_statistics {
    uint32_t total_allocated;                 // 总分配字节数
    uint32_t peak_allocated;                  // 峰值分配字节数
    uint32_t allocation_count;                // 分配次数
    uint32_t deallocation_count;              // 释放次数
    uint32_t pool_usage[MMGR_POOL_COUNT];     // 各池使用情况
    uint32_t fragmentation_count;             // 碎片化计数
} mmgr_statistics_t;

/* 分配记录（调试用） */
#define MAX_ALLOCATION_RECORDS 1000
typedef enum {
    ALLOC_STRING = 0,
    ALLOC_SYMBOL,
    ALLOC_TYPE_INFO,
    ALLOC_AST_NODE,
    ALLOC_GENERAL,
    ALLOC_LIBRARY
} allocation_type_t;

typedef struct allocation_record {
    void *ptr;                                 // 分配的指针
    size_t size;                               // 分配大小
    allocation_type_t type;                    // 分配类型
    const char *file;                          // 文件名（调试用）
    int line;                                  // 行号（调试用）
    bool is_active;                            // 是否活跃
} allocation_record_t;

/* 静态内存管理器 */
typedef struct mmgr_static_manager {
    /* 字符串池 */
    mmgr_string_pool_t string_pool;
    
    /* 固定大小池 */
    mmgr_fixed_block_pool_t ast_node_pool;         // AST节点池
    mmgr_fixed_block_pool_t symbol_pool;           // 符号池
    mmgr_fixed_block_pool_t type_info_pool;        // 类型信息池
    mmgr_fixed_block_pool_t library_pool;          // 库信息池
    
    /* 可变大小池 */
    mmgr_variable_pool_t bytecode_pool;            // 字节码池
    mmgr_variable_pool_t const_pool;               // 常量池
    
    /* 通用内存池 */
    mmgr_general_pool_t general_pool;              // 通用内存池
    
    /* 内存数据区 */
    uint8_t ast_node_memory[MMGR_MAX_AST_NODES * 256];      // AST节点内存区
    uint8_t symbol_memory[MMGR_MAX_SYMBOLS * 128];          // 符号内存区
    uint8_t type_info_memory[MMGR_MAX_TYPE_INFO * 64];      // 类型信息内存区
    uint8_t library_memory[MMGR_MAX_LIBRARIES * 512];       // 库信息内存区
    uint8_t bytecode_memory[MMGR_MAX_BYTECODE_SIZE];        // 字节码内存区
    uint8_t const_memory[MMGR_MAX_CONST_POOL_SIZE];         // 常量内存区
    uint8_t general_memory[256 * 1024];                     // 通用内存区（256KB）

    /* 块状态数组 */
    mmgr_block_state_t ast_node_states[MMGR_MAX_AST_NODES];
    mmgr_block_state_t symbol_states[MMGR_MAX_SYMBOLS];
    mmgr_block_state_t type_info_states[MMGR_MAX_TYPE_INFO];
    mmgr_block_state_t library_states[MMGR_MAX_LIBRARIES];
    
    /* 空闲块列表 */
    uint32_t ast_free_list[MMGR_MAX_AST_NODES];
    uint32_t symbol_free_list[MMGR_MAX_SYMBOLS];
    uint32_t type_info_free_list[MMGR_MAX_TYPE_INFO];
    uint32_t library_free_list[MMGR_MAX_LIBRARIES];
    
    /* 分配记录（调试用） */
    allocation_record_t allocation_records[MAX_ALLOCATION_RECORDS];
    uint32_t record_count;                          // 记录数量
    
    /* 缓存信息（用于快速查找） */
    struct {
        allocation_record_t *last_symbol;          // 最后访问的符号
        allocation_record_t *last_library;         // 最后访问的库
        uint32_t cache_version;                    // 缓存版本
    } cache;
    
    /* 统计信息 */
    mmgr_statistics_t stats;
    
    /* 管理器状态 */
    bool is_initialized;                       // 是否已初始化
    bool debug_enabled;                        // 是否启用调试
    uint32_t magic_number;                     // 魔数用于验证
    uint32_t checksum;                         // 校验和
    char last_error[256];                      // 最后错误信息
    
} mmgr_static_manager_t;

/* 内存分配结果 */
typedef struct mmgr_allocation_result {
    void *ptr;                                 // 分配的指针
    uint32_t size;                             // 分配的大小
    mmgr_pool_type_t pool_type;                // 池类型
    uint32_t index;                            // 在池中的索引
    bool success;                              // 是否成功
} mmgr_allocation_result_t;

/* 内存管理接口 */
int mmgr_init(void);
void mmgr_cleanup(void);
bool mmgr_is_initialized(void);

/* 字符串分配接口 */
char* mmgr_alloc_string(const char *str);
void mmgr_free_string(const char *str);
uint32_t mmgr_get_string_ref_count(const char *str);

/* AST节点分配接口 */
void* mmgr_alloc_ast_node(size_t size);
void mmgr_free_ast_node(void *node);
uint32_t mmgr_get_ast_node_index(void *node);
void* mmgr_get_ast_node_by_index(uint32_t index);

/* 符号分配接口 */
void* mmgr_alloc_symbol(size_t size);
void mmgr_free_symbol(void *symbol);
uint32_t mmgr_get_symbol_index(void *symbol);
void* mmgr_get_symbol_by_index(uint32_t index);

/* 类型信息分配接口 */
void* mmgr_alloc_type_info(size_t size);
void mmgr_free_type_info(void *type_info);
uint32_t mmgr_get_type_info_index(void *type_info);

/* 库信息分配接口 */
void* mmgr_alloc_library_info(size_t size);
void mmgr_free_library_info(void *lib_info);

/* 字节码分配接口 */
void* mmgr_alloc_bytecode(size_t size);
void mmgr_free_bytecode(void *bytecode);
void* mmgr_realloc_bytecode(void *old_ptr, size_t new_size);

/* 常量池分配接口 */
void* mmgr_alloc_const_data(size_t size);
void mmgr_free_const_data(void *data);
uint32_t mmgr_add_const_int(int32_t value);
uint32_t mmgr_add_const_real(double value);
uint32_t mmgr_add_const_string(const char *str);

/* 内存统计接口 */
mmgr_statistics_t* mmgr_get_statistics(void);
uint32_t mmgr_get_pool_usage(mmgr_pool_type_t pool_type);
uint32_t mmgr_get_total_usage(void);
double mmgr_get_fragmentation_ratio(void);

/* 内存检查和验证接口 */
bool mmgr_verify_integrity(void);
bool mmgr_is_valid_pointer(void *ptr, mmgr_pool_type_t expected_pool);
void mmgr_dump_info(void);
void mmgr_print_statistics(void);

/* 内存压缩和清理接口 */
int mmgr_compact_string_pool(void);
int mmgr_defragment_pools(void);
void mmgr_reset_statistics(void);

/* 调试和诊断接口 */
void mmgr_enable_debug(bool enable);
void mmgr_set_debug_callback(void (*callback)(const char *msg));
const char* mmgr_get_pool_name(mmgr_pool_type_t pool_type);

/* ========== 内存分配接口 ========== */

/* 字符串分配 */
char* mmgr_alloc_string(const char *str);
char* mmgr_alloc_string_with_length(const char *str, uint32_t length);

/* 符号分配 */
void* mmgr_alloc_symbol(size_t size);

/* 类型信息分配 */ 
void* mmgr_alloc_type_info(size_t size);

/* 通用内存分配 */
void* mmgr_alloc_general(size_t size);
void* mmgr_alloc_general_debug(size_t size, const char *file, int line);
void* mmgr_realloc_general(void *ptr, size_t new_size);

/* 通用内存池管理 */
bool mmgr_is_general_pointer(const void *ptr);
size_t mmgr_compact_general_pool(void);
void mmgr_clear_general_pool(void);

/* ========== 统计和调试接口 ========== */

/* 内存使用统计 */
typedef struct {
    size_t total_size;          // 总大小
    size_t used_size;           // 已使用大小
    size_t free_size;           // 剩余大小
    uint32_t allocation_count;  // 分配次数
    float fragmentation_ratio;  // 碎片率
} mmgr_general_stats_t;

void mmgr_get_general_stats(mmgr_general_stats_t *stats);
void mmgr_dump_general_allocations(void);

/* ========== 调试宏定义 ========== */

#ifdef MMGR_DEBUG
#define mmgr_alloc_general_debug(size) mmgr_alloc_general_debug(size, __FILE__, __LINE__)
#else
#define mmgr_alloc_general_debug(size) mmgr_alloc_general(size)
#endif

#endif /* MMGR_H */
