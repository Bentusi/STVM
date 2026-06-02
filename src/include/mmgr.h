/**
 * @file mmgr.h
 * @brief 内存管理器 - 提供内存池和统一的内存分配接口
 */

#ifndef STVM_MMGR_H
#define STVM_MMGR_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief 内存池类型
 */
typedef enum {
    POOL_SMALL,     // 小对象池 (8-64字节)
    POOL_MEDIUM,    // 中对象池 (64-512字节)
    POOL_LARGE,     // 大对象池 (512字节+)
    POOL_AST,       // AST节点专用池
    POOL_SYMBOL,    // 符号表专用池
    POOL_STRING,    // 字符串常量池
    POOL_COUNT      // 池总数
} PoolType;

/**
 * @brief 内存块头（用于追踪和调试）
 */
typedef struct MemoryBlock {
    uint32_t magic;             // 魔数，用于验证内存块有效性 (0xDEADBEEF)
    size_t size;                // 块大小
    PoolType pool_type;         // 所属池类型
    struct MemoryBlock* next;   // 空闲链表指针
    bool is_free;               // 是否空闲
#ifdef DEBUG
    const char* file;           // 分配文件（调试用）
    int line;                   // 分配行号（调试用）
#endif
} MemoryBlock;

// 内存块魔数
#define MMGR_MAGIC 0xDEADBEEF
#define MMGR_FREED_MAGIC 0xFEEDF00D

/**
 * @brief 内存池结构
 */
typedef struct {
    size_t block_size;          // 固定块大小
    size_t block_count;         // 块总数
    size_t used_count;          // 已使用块数
    MemoryBlock* free_list;     // 空闲链表头
    void* pool_data;            // 池数据区域
} MemoryPool;

/**
 * @brief 内存管理器统计信息
 */
typedef struct {
    size_t total_allocated;     // 总分配字节数
    size_t total_freed;         // 总释放字节数
    size_t current_usage;       // 当前使用字节数
    size_t peak_usage;          // 峰值使用字节数
    size_t alloc_count;         // 分配次数
    size_t free_count;          // 释放次数
    size_t pool_stats[POOL_COUNT]; // 各池使用统计
} MemoryStats;

/**
 * @brief 内存管理器主结构
 */
typedef struct {
    MemoryPool pools[POOL_COUNT];   // 内存池数组
    MemoryStats stats;               // 统计信息
    bool initialized;                // 是否已初始化
} MemoryManager;

/**
 * @brief 初始化内存管理器
 * @return 成功返回true，失败返回false
 */
bool mmgr_init(void);

/**
 * @brief 清理内存管理器，释放所有资源
 */
void mmgr_cleanup(void);

/**
 * @brief 分配内存
 * @param size 请求大小（字节）
 * @return 分配的内存指针，失败返回NULL
 */
void* mmgr_alloc(size_t size);

/**
 * @brief 分配并清零内存
 * @param size 请求大小（字节）
 * @return 分配的内存指针，失败返回NULL
 */
void* mmgr_calloc(size_t size);

/**
 * @brief 重新分配内存
 * @param ptr 原内存指针
 * @param new_size 新大小（字节）
 * @return 新内存指针，失败返回NULL
 */
void* mmgr_realloc(void* ptr, size_t new_size);

/**
 * @brief 释放内存
 * @param ptr 内存指针
 */
void mmgr_free(void* ptr);

/**
 * @brief 从指定池分配内存
 * @param pool_type 池类型
 * @param size 请求大小
 * @return 分配的内存指针，失败返回NULL
 */
void* mmgr_alloc_from_pool(PoolType pool_type, size_t size);

/**
 * @brief 复制字符串（自动管理内存）
 * @param str 源字符串
 * @return 新字符串指针，失败返回NULL
 */
char* mmgr_strdup(const char* str);

/**
 * @brief 重置内存池（快速清理，用于编译阶段结束后）
 * @param pool_type 池类型
 */
void mmgr_reset_pool(PoolType pool_type);

/**
 * @brief 获取内存使用统计信息
 * @return 统计信息指针
 */
const MemoryStats* mmgr_get_stats(void);

/**
 * @brief 打印内存使用统计（调试用）
 */
void mmgr_print_stats(void);

/**
 * @brief 检查内存泄漏（调试用）
 * @return 有泄漏返回true，否则返回false
 */
bool mmgr_check_leaks(void);

/**
 * @brief 调试宏：带文件名和行号的内存分配
 */
#ifdef DEBUG
#define MMGR_ALLOC(size) mmgr_alloc_debug(size, __FILE__, __LINE__)
#define MMGR_FREE(ptr) mmgr_free_debug(ptr, __FILE__, __LINE__)
void* mmgr_alloc_debug(size_t size, const char* file, int line);
void mmgr_free_debug(void* ptr, const char* file, int line);
#else
#define MMGR_ALLOC(size) mmgr_alloc(size)
#define MMGR_FREE(ptr) mmgr_free(ptr)
#endif

#endif // STVM_MMGR_H
