/**
 * @file mmgr.c
 * @brief 内存管理器的实现
 */

#include "mmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 全局内存管理器实例
static MemoryManager g_mmgr = {0};

// 各内存池的配置
static const struct {
    size_t block_size;
    size_t block_count;
} pool_configs[POOL_COUNT] = {
    {64,   1024},   // POOL_SMALL: 64字节 × 1024块
    {512,  512},    // POOL_MEDIUM: 512字节 × 512块
    {4096, 128},    // POOL_LARGE: 4KB × 128块
    {128,  2048},   // POOL_AST: 128字节 × 2048块 (AST节点)
    {256,  1024},   // POOL_SYMBOL: 256字节 × 1024块 (符号表)
    {64,   2048},   // POOL_STRING: 64字节 × 2048块 (字符串)
};

/**
 * @brief 初始化单个内存池
 */
static bool init_pool(MemoryPool* pool, size_t block_size, size_t block_count) {
    pool->block_size = block_size;
    pool->block_count = block_count;
    pool->used_count = 0;
    pool->free_list = NULL;
    
    // 分配池数据区域
    size_t total_size = block_size * block_count;
    pool->pool_data = malloc(total_size);
    if (!pool->pool_data) {
        return false;
    }
    
    // 初始化空闲链表
    MemoryBlock* block = (MemoryBlock*)pool->pool_data;
    for (size_t i = 0; i < block_count; i++) {
        block->size = block_size;
        block->is_free = true;
        block->next = (i < block_count - 1) ? 
                      (MemoryBlock*)((char*)block + block_size) : NULL;
        block = block->next;
    }
    
    pool->free_list = (MemoryBlock*)pool->pool_data;
    return true;
}

/**
 * @brief 初始化内存管理器
 */
bool mmgr_init(void) {
    if (g_mmgr.initialized) {
        return true;
    }
    
    memset(&g_mmgr, 0, sizeof(MemoryManager));
    
    // 初始化各个内存池
    for (int i = 0; i < POOL_COUNT; i++) {
        if (!init_pool(&g_mmgr.pools[i], 
                      pool_configs[i].block_size, 
                      pool_configs[i].block_count)) {
            // 初始化失败，清理已创建的池
            mmgr_cleanup();
            return false;
        }
    }
    
    g_mmgr.initialized = true;
    return true;
}

/**
 * @brief 清理内存管理器
 */
void mmgr_cleanup(void) {
    if (!g_mmgr.initialized) {
        return;
    }
    
    // 释放所有内存池
    for (int i = 0; i < POOL_COUNT; i++) {
        if (g_mmgr.pools[i].pool_data) {
            free(g_mmgr.pools[i].pool_data);
            g_mmgr.pools[i].pool_data = NULL;
        }
    }
    
    g_mmgr.initialized = false;
}

/**
 * @brief 根据大小选择合适的内存池
 */
static PoolType select_pool(size_t size) {
    if (size <= 64) {
        return POOL_SMALL;
    } else if (size <= 512) {
        return POOL_MEDIUM;
    } else {
        return POOL_LARGE;
    }
}

/**
 * @brief 从指定池分配内存
 */
void* mmgr_alloc_from_pool(PoolType pool_type, size_t size) {
    if (!g_mmgr.initialized || pool_type >= POOL_COUNT) {
        return NULL;
    }
    
    MemoryPool* pool = &g_mmgr.pools[pool_type];
    
    // 检查是否有空闲块
    if (!pool->free_list) {
        // 池已满，回退到标准malloc
        void* ptr = malloc(size + sizeof(MemoryBlock));
        if (!ptr) return NULL;
        
        MemoryBlock* block = (MemoryBlock*)ptr;
        block->size = size;
        block->pool_type = pool_type;
        block->is_free = false;
        block->next = NULL;
        
        g_mmgr.stats.total_allocated += size;
        g_mmgr.stats.current_usage += size;
        g_mmgr.stats.alloc_count++;
        
        if (g_mmgr.stats.current_usage > g_mmgr.stats.peak_usage) {
            g_mmgr.stats.peak_usage = g_mmgr.stats.current_usage;
        }
        
        return (char*)ptr + sizeof(MemoryBlock);
    }
    
    // 从空闲链表分配
    MemoryBlock* block = pool->free_list;
    pool->free_list = block->next;
    pool->used_count++;
    
    block->is_free = false;
    block->pool_type = pool_type;
    
    // 更新统计信息
    g_mmgr.stats.total_allocated += block->size;
    g_mmgr.stats.current_usage += block->size;
    g_mmgr.stats.alloc_count++;
    g_mmgr.stats.pool_stats[pool_type]++;
    
    if (g_mmgr.stats.current_usage > g_mmgr.stats.peak_usage) {
        g_mmgr.stats.peak_usage = g_mmgr.stats.current_usage;
    }
    
    return (char*)block + sizeof(MemoryBlock);
}

/**
 * @brief 分配内存
 */
void* mmgr_alloc(size_t size) {
    if (!g_mmgr.initialized) {
        if (!mmgr_init()) {
            return NULL;
        }
    }
    
    if (size == 0) {
        return NULL;
    }
    
    PoolType pool_type = select_pool(size);
    return mmgr_alloc_from_pool(pool_type, size);
}

/**
 * @brief 分配并清零内存
 */
void* mmgr_calloc(size_t size) {
    void* ptr = mmgr_alloc(size);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

/**
 * @brief 释放内存
 */
void mmgr_free(void* ptr) {
    if (!ptr || !g_mmgr.initialized) {
        return;
    }
    
    // 获取内存块头
    MemoryBlock* block = (MemoryBlock*)((char*)ptr - sizeof(MemoryBlock));
    
    if (block->is_free) {
        // 双重释放检测
        fprintf(stderr, "Warning: Double free detected at %p\n", ptr);
        return;
    }
    
    // 更新统计信息（在释放前）
    size_t block_size = block->size;
    g_mmgr.stats.total_freed += block_size;
    if (g_mmgr.stats.current_usage >= block_size) {
        g_mmgr.stats.current_usage -= block_size;
    }
    g_mmgr.stats.free_count++;
    
    PoolType pool_type = block->pool_type;
    
    // 检查是否属于池管理
    if (pool_type < POOL_COUNT) {
        MemoryPool* pool = &g_mmgr.pools[pool_type];
        
        // 检查块是否在池范围内
        char* pool_start = (char*)pool->pool_data;
        char* pool_end = pool_start + (pool->block_size * pool->block_count);
        char* block_addr = (char*)block;
        
        if (block_addr >= pool_start && block_addr < pool_end) {
            // 归还到池的空闲链表
            block->is_free = true;
            block->next = pool->free_list;
            pool->free_list = block;
            pool->used_count--;
        } else {
            // 不在池范围内，使用标准free
            free(block);
        }
    } else {
        // 不属于池管理，使用标准free
        free(block);
    }
}

/**
 * @brief 重新分配内存
 */
void* mmgr_realloc(void* ptr, size_t new_size) {
    if (!ptr) {
        return mmgr_alloc(new_size);
    }
    
    if (new_size == 0) {
        mmgr_free(ptr);
        return NULL;
    }
    
    // 获取原块信息
    MemoryBlock* old_block = (MemoryBlock*)((char*)ptr - sizeof(MemoryBlock));
    size_t old_size = old_block->size - sizeof(MemoryBlock);
    
    // 如果新大小小于等于原大小，直接返回
    if (new_size <= old_size) {
        return ptr;
    }
    
    // 分配新内存并复制数据
    void* new_ptr = mmgr_alloc(new_size);
    if (!new_ptr) {
        return NULL;
    }
    
    memcpy(new_ptr, ptr, old_size);
    mmgr_free(ptr);
    
    return new_ptr;
}

/**
 * @brief 复制字符串
 */
char* mmgr_strdup(const char* str) {
    if (!str) {
        return NULL;
    }
    
    size_t len = strlen(str) + 1;
    char* new_str = (char*)mmgr_alloc_from_pool(POOL_STRING, len);
    if (new_str) {
        memcpy(new_str, str, len);
    }
    
    return new_str;
}

/**
 * @brief 重置内存池
 */
void mmgr_reset_pool(PoolType pool_type) {
    if (!g_mmgr.initialized || pool_type >= POOL_COUNT) {
        return;
    }
    
    MemoryPool* pool = &g_mmgr.pools[pool_type];
    
    // 重新初始化空闲链表
    pool->used_count = 0;
    MemoryBlock* block = (MemoryBlock*)pool->pool_data;
    for (size_t i = 0; i < pool->block_count; i++) {
        block->is_free = true;
        block->next = (i < pool->block_count - 1) ? 
                      (MemoryBlock*)((char*)block + pool->block_size) : NULL;
        block = block->next;
    }
    
    pool->free_list = (MemoryBlock*)pool->pool_data;
}

/**
 * @brief 获取内存使用统计信息
 */
const MemoryStats* mmgr_get_stats(void) {
    return &g_mmgr.stats;
}

/**
 * @brief 打印内存使用统计
 */
void mmgr_print_stats(void) {
    const MemoryStats* stats = &g_mmgr.stats;
    
    printf("\n=== Memory Statistics ===\n");
    printf("Total allocated:  %zu bytes\n", stats->total_allocated);
    printf("Total freed:      %zu bytes\n", stats->total_freed);
    printf("Current usage:    %zu bytes\n", stats->current_usage);
    printf("Peak usage:       %zu bytes\n", stats->peak_usage);
    printf("Alloc count:      %zu\n", stats->alloc_count);
    printf("Free count:       %zu\n", stats->free_count);
    
    printf("\nPool Statistics:\n");
    const char* pool_names[] = {
        "Small", "Medium", "Large", "AST", "Symbol", "String"
    };
    for (int i = 0; i < POOL_COUNT; i++) {
        printf("  %-8s: %zu allocations, %zu/%zu blocks used\n",
               pool_names[i],
               stats->pool_stats[i],
               g_mmgr.pools[i].used_count,
               g_mmgr.pools[i].block_count);
    }
    printf("=========================\n\n");
}

/**
 * @brief 检查内存泄漏
 */
bool mmgr_check_leaks(void) {
    if (!g_mmgr.initialized) {
        return false;
    }
    
    bool has_leaks = (g_mmgr.stats.current_usage > 0);
    
    if (has_leaks) {
        printf("WARNING: Memory leak detected!\n");
        printf("Current usage: %zu bytes\n", g_mmgr.stats.current_usage);
        printf("Alloc count: %zu, Free count: %zu\n",
               g_mmgr.stats.alloc_count,
               g_mmgr.stats.free_count);
    }
    
    return has_leaks;
}

#ifdef DEBUG
/**
 * @brief 调试版本的内存分配
 */
void* mmgr_alloc_debug(size_t size, const char* file, int line) {
    void* ptr = mmgr_alloc(size);
    if (ptr) {
        MemoryBlock* block = (MemoryBlock*)((char*)ptr - sizeof(MemoryBlock));
        block->file = file;
        block->line = line;
    }
    return ptr;
}

/**
 * @brief 调试版本的内存释放
 */
void mmgr_free_debug(void* ptr, const char* file, int line) {
    if (ptr) {
        MemoryBlock* block = (MemoryBlock*)((char*)ptr - sizeof(MemoryBlock));
        printf("Free: %p (allocated at %s:%d, freed at %s:%d)\n",
               ptr, block->file, block->line, file, line);
    }
    mmgr_free(ptr);
}
#endif
