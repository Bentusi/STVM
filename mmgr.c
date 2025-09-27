#include "mmgr.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

/* 魔数定义 */
#define MMGR_MAGIC              0x4D4D4752  // "MMGR"
#define MMGR_ALLOCATION_MAGIC   0x414C4C43  // "ALLC"

/* 全局静态内存管理器 */
static mmgr_static_manager_t g_mmgr = {0};
static bool g_mmgr_debug_enabled = false;
static void (*g_mmgr_debug_callback)(const char *msg) = NULL;

/* 调试输出宏 */
#define MMGR_DEBUG_LOG(msg) do { \
    if (g_mmgr_debug_enabled && g_mmgr_debug_callback) { \
        g_mmgr_debug_callback(msg); \
    } \
} while(0)

/* 内部辅助函数声明 */
static uint32_t mmgr_calculate_checksum(void *data, size_t size);
static bool mmgr_init_string_pool(mmgr_string_pool_t *pool);
static bool mmgr_init_fixed_pool(mmgr_fixed_block_pool_t *pool, void *memory, 
                                  uint32_t block_size, uint32_t max_blocks,
                                  mmgr_block_state_t *states, uint32_t *free_list);
static bool mmgr_init_variable_pool(mmgr_variable_pool_t *pool, uint8_t *memory, uint32_t size);
static void mmgr_update_statistics(mmgr_pool_type_t pool_type, uint32_t size, bool is_allocation);

/* 初始化内存管理器 */
int mmgr_init(void) {
    if (g_mmgr.is_initialized) {
        return 0; // 已经初始化
    }
    
    MMGR_DEBUG_LOG("Initializing MMGR module...");
    
    /* 清零整个结构体 */
    memset(&g_mmgr, 0, sizeof(mmgr_static_manager_t));
    
    /* 初始化字符串池 */
    if (!mmgr_init_string_pool(&g_mmgr.string_pool)) {
        MMGR_DEBUG_LOG("Failed to initialize string pool");
        return -1;
    }
    
    /* 初始化AST节点池 */
    if (!mmgr_init_fixed_pool(&g_mmgr.ast_node_pool,
                              g_mmgr.ast_node_memory, 256, MMGR_MAX_AST_NODES,
                              g_mmgr.ast_node_states, g_mmgr.ast_free_list)) {
        MMGR_DEBUG_LOG("Failed to initialize AST node pool");
        return -1;
    }
    
    /* 初始化符号池 */
    if (!mmgr_init_fixed_pool(&g_mmgr.symbol_pool,
                              g_mmgr.symbol_memory, 128, MMGR_MAX_SYMBOLS,
                              g_mmgr.symbol_states, g_mmgr.symbol_free_list)) {
        MMGR_DEBUG_LOG("Failed to initialize symbol pool");
        return -1;
    }
    
    /* 初始化类型信息池 */
    if (!mmgr_init_fixed_pool(&g_mmgr.type_info_pool,
                              g_mmgr.type_info_memory, 64, MMGR_MAX_TYPE_INFO,
                              g_mmgr.type_info_states, g_mmgr.type_info_free_list)) {
        MMGR_DEBUG_LOG("Failed to initialize type info pool");
        return -1;
    }
    
    /* 初始化库信息池 */
    if (!mmgr_init_fixed_pool(&g_mmgr.library_pool,
                              g_mmgr.library_memory, 512, MMGR_MAX_LIBRARIES,
                              g_mmgr.library_states, g_mmgr.library_free_list)) {
        MMGR_DEBUG_LOG("Failed to initialize library pool");
        return -1;
    }
    
    /* 初始化字节码池 */
    if (!mmgr_init_variable_pool(&g_mmgr.bytecode_pool,
                                 g_mmgr.bytecode_memory, MMGR_MAX_BYTECODE_SIZE)) {
        MMGR_DEBUG_LOG("Failed to initialize bytecode pool");
        return -1;
    }
    
    /* 初始化常量池 */
    if (!mmgr_init_variable_pool(&g_mmgr.const_pool,
                                 g_mmgr.const_memory, MMGR_MAX_CONST_POOL_SIZE)) {
        MMGR_DEBUG_LOG("Failed to initialize constant pool");
        return -1;
    }
    
    /* 设置管理器状态 */
    g_mmgr.magic_number = MMGR_MAGIC;
    g_mmgr.is_initialized = true;
    
    /* 计算校验和 */
    g_mmgr.checksum = mmgr_calculate_checksum(&g_mmgr, 
                                              sizeof(mmgr_static_manager_t) - sizeof(uint32_t));
    
    MMGR_DEBUG_LOG("MMGR module initialized successfully");
    return 0;
}

/* 初始化字符串池 */
static bool mmgr_init_string_pool(mmgr_string_pool_t *pool) {
    memset(pool, 0, sizeof(mmgr_string_pool_t));
    pool->used_size = 0;
    pool->entry_count = 0;
    pool->free_entry_count = 0;
    
    /* 初始化空闲条目列表 */
    for (uint32_t i = 0; i < MMGR_MAX_STRINGS; i++) {
        pool->free_entry_list[i] = i;
        pool->entries[i].is_allocated = false;
        pool->entries[i].ref_count = 0;
    }
    pool->free_entry_count = MMGR_MAX_STRINGS;
    
    return true;
}

/* 初始化固定大小池 */
static bool mmgr_init_fixed_pool(mmgr_fixed_block_pool_t *pool, void *memory, 
                                  uint32_t block_size, uint32_t max_blocks,
                                  mmgr_block_state_t *states, uint32_t *free_list) {
    pool->memory_base = memory;
    pool->block_size = block_size;
    pool->max_blocks = max_blocks;
    pool->allocated_count = 0;
    pool->block_states = states;
    pool->free_block_list = free_list;
    pool->free_block_count = max_blocks;
    pool->next_free_index = 0;
    
    /* 初始化块状态和空闲列表 */
    for (uint32_t i = 0; i < max_blocks; i++) {
        states[i] = MMGR_BLOCK_FREE;
        free_list[i] = i;
    }
    
    return true;
}

/* 初始化可变大小池 */
static bool mmgr_init_variable_pool(mmgr_variable_pool_t *pool, uint8_t *memory, uint32_t size) {
    pool->memory_base = memory;
    pool->pool_size = size;
    pool->used_size = 0;
    pool->allocation_count = 0;
    
    memset(pool->allocations, 0, sizeof(pool->allocations));
    
    return true;
}

/* 分配字符串 */
char* mmgr_alloc_string(const char *str) {
    if (!str || !g_mmgr.is_initialized) {
        return NULL;
    }
    
    return mmgr_alloc_string_with_length(str, strlen(str));
}

/* 按长度分配字符串 */
char* mmgr_alloc_string_with_length(const char *str, uint32_t length) {
    if (!str || length == 0 || !g_mmgr.is_initialized) {
        return NULL;
    }
    
    mmgr_string_pool_t *pool = &g_mmgr.string_pool;
    
    /* 检查是否已存在相同字符串 */
    for (uint32_t i = 0; i < pool->entry_count; i++) {
        if (pool->entries[i].is_allocated && 
            pool->entries[i].length == length) {
            char *existing = &pool->data[pool->entries[i].offset];
            if (strncmp(existing, str, length) == 0) {
                pool->entries[i].ref_count++;
                mmgr_update_statistics(MMGR_POOL_STRING, length, true);
                return existing; // 返回已存在的字符串
            }
        }
    }
    
    /* 检查是否有足够空间 */
    if (pool->used_size + length + 1 > MMGR_MAX_STRING_POOL_SIZE ||
        pool->free_entry_count == 0) {
        MMGR_DEBUG_LOG("String pool exhausted");
        return NULL;
    }
    
    /* 分配新字符串 */
    uint32_t entry_index = pool->free_entry_list[pool->free_entry_count - 1];
    pool->free_entry_count--;
    
    uint32_t offset = pool->used_size;
    strncpy(&pool->data[offset], str, length);
    pool->data[offset + length] = '\0';
    
    pool->entries[entry_index].offset = offset;
    pool->entries[entry_index].length = length;
    pool->entries[entry_index].ref_count = 1;
    pool->entries[entry_index].is_allocated = true;
    
    pool->used_size += length + 1;
    if (entry_index >= pool->entry_count) {
        pool->entry_count = entry_index + 1;
    }
    
    mmgr_update_statistics(MMGR_POOL_STRING, length + 1, true);
    
    return &pool->data[offset];
}

/* 分配AST节点 */
void* mmgr_alloc_ast_node(size_t size) {
    if (!g_mmgr.is_initialized || size > 256) {
        return NULL;
    }
    
    mmgr_fixed_block_pool_t *pool = &g_mmgr.ast_node_pool;
    
    if (pool->free_block_count == 0) {
        MMGR_DEBUG_LOG("AST node pool exhausted");
        return NULL;
    }
    
    /* 获取空闲块 */
    uint32_t block_index = pool->free_block_list[pool->free_block_count - 1];
    pool->free_block_count--;
    
    pool->block_states[block_index] = MMGR_BLOCK_ALLOCATED;
    pool->allocated_count++;
    
    void *ptr = (uint8_t*)pool->memory_base + (block_index * pool->block_size);
    memset(ptr, 0, pool->block_size);
    
    mmgr_update_statistics(MMGR_POOL_AST_NODE, pool->block_size, true);
    
    return ptr;
}

/* 分配符号 */
void* mmgr_alloc_symbol(size_t size) {
    if (!g_mmgr.is_initialized || size > 128) {
        return NULL;
    }
    
    mmgr_fixed_block_pool_t *pool = &g_mmgr.symbol_pool;
    
    if (pool->free_block_count == 0) {
        MMGR_DEBUG_LOG("Symbol pool exhausted");
        return NULL;
    }
    
    uint32_t block_index = pool->free_block_list[pool->free_block_count - 1];
    pool->free_block_count--;
    
    pool->block_states[block_index] = MMGR_BLOCK_ALLOCATED;
    pool->allocated_count++;
    
    void *ptr = (uint8_t*)pool->memory_base + (block_index * pool->block_size);
    memset(ptr, 0, pool->block_size);
    
    mmgr_update_statistics(MMGR_POOL_SYMBOL, pool->block_size, true);
    
    return ptr;
}

/* 分配类型信息 */
void* mmgr_alloc_type_info(size_t size) {
    if (!g_mmgr.is_initialized || size > 64) {
        return NULL;
    }
    
    mmgr_fixed_block_pool_t *pool = &g_mmgr.type_info_pool;
    
    if (pool->free_block_count == 0) {
        MMGR_DEBUG_LOG("Type info pool exhausted");
        return NULL;
    }
    
    uint32_t block_index = pool->free_block_list[pool->free_block_count - 1];
    pool->free_block_count--;
    
    pool->block_states[block_index] = MMGR_BLOCK_ALLOCATED;
    pool->allocated_count++;
    
    void *ptr = (uint8_t*)pool->memory_base + (block_index * pool->block_size);
    memset(ptr, 0, pool->block_size);
    
    mmgr_update_statistics(MMGR_POOL_TYPE_INFO, pool->block_size, true);
    
    return ptr;
}

/* 分配库信息 */
void* mmgr_alloc_library_info(size_t size) {
    if (!g_mmgr.is_initialized || size > 512) {
        return NULL;
    }
    
    mmgr_fixed_block_pool_t *pool = &g_mmgr.library_pool;
    
    if (pool->free_block_count == 0) {
        MMGR_DEBUG_LOG("Library pool exhausted");
        return NULL;
    }
    
    uint32_t block_index = pool->free_block_list[pool->free_block_count - 1];
    pool->free_block_count--;
    
    pool->block_states[block_index] = MMGR_BLOCK_ALLOCATED;
    pool->allocated_count++;
    
    void *ptr = (uint8_t*)pool->memory_base + (block_index * pool->block_size);
    memset(ptr, 0, pool->block_size);
    
    mmgr_update_statistics(MMGR_POOL_LIBRARY, pool->block_size, true);
    
    return ptr;
}

/* 分配字节码 */
void* mmgr_alloc_bytecode(size_t size) {
    if (!g_mmgr.is_initialized || size == 0) {
        return NULL;
    }
    
    mmgr_variable_pool_t *pool = &g_mmgr.bytecode_pool;
    
    /* 对齐到4字节边界 */
    size_t aligned_size = (size + 3) & ~3;
    size_t total_size = aligned_size + sizeof(struct mmgr_allocation_header);
    
    if (pool->used_size + total_size > pool->pool_size) {
        MMGR_DEBUG_LOG("Bytecode pool exhausted");
        return NULL;
    }
    
    if (pool->allocation_count >= 1000) {
        MMGR_DEBUG_LOG("Too many bytecode allocations");
        return NULL;
    }
    
    /* 创建分配头 */
    struct mmgr_allocation_header *header = 
        (struct mmgr_allocation_header*)(pool->memory_base + pool->used_size);
    header->size = aligned_size;
    header->magic = MMGR_ALLOCATION_MAGIC;
    header->is_free = false;
    
    pool->allocations[pool->allocation_count] = header;
    pool->allocation_count++;
    pool->used_size += total_size;
    
    void *ptr = (uint8_t*)header + sizeof(struct mmgr_allocation_header);
    memset(ptr, 0, aligned_size);
    
    mmgr_update_statistics(MMGR_POOL_BYTECODE, total_size, true);
    
    return ptr;
}

/* 添加整数常量 */
uint32_t mmgr_add_const_int(int32_t value) {
    mmgr_variable_pool_t *pool = &g_mmgr.const_pool;
    
    /* 检查是否已存在 */
    uint8_t *current = pool->memory_base;
    uint32_t offset = 0;
    
    while (offset < pool->used_size) {
        struct mmgr_allocation_header *header = (struct mmgr_allocation_header*)(current + offset);
        if (header->magic == MMGR_ALLOCATION_MAGIC && !header->is_free && 
            header->size == sizeof(int32_t)) {
            int32_t *existing_value = (int32_t*)((uint8_t*)header + sizeof(struct mmgr_allocation_header));
            if (*existing_value == value) {
                return offset + sizeof(struct mmgr_allocation_header);
            }
        }
        offset += header->size + sizeof(struct mmgr_allocation_header);
    }
    
    /* 添加新常量 */
    void *ptr = mmgr_alloc_const_data(sizeof(int32_t));
    if (!ptr) return 0;
    
    *(int32_t*)ptr = value;
    return (uint8_t*)ptr - pool->memory_base;
}

/* 添加实数常量 */
uint32_t mmgr_add_const_real(double value) {
    mmgr_variable_pool_t *pool = &g_mmgr.const_pool;
    
    /* 检查是否已存在 */
    uint8_t *current = pool->memory_base;
    uint32_t offset = 0;
    
    while (offset < pool->used_size) {
        struct mmgr_allocation_header *header = (struct mmgr_allocation_header*)(current + offset);
        if (header->magic == MMGR_ALLOCATION_MAGIC && !header->is_free && 
            header->size == sizeof(double)) {
            double *existing_value = (double*)((uint8_t*)header + sizeof(struct mmgr_allocation_header));
            if (*existing_value == value) {
                return offset + sizeof(struct mmgr_allocation_header);
            }
        }
        offset += header->size + sizeof(struct mmgr_allocation_header);
    }
    
    /* 添加新常量 */
    void *ptr = mmgr_alloc_const_data(sizeof(double));
    if (!ptr) return 0;
    
    *(double*)ptr = value;
    return (uint8_t*)ptr - pool->memory_base;
}

/* 分配常量数据 */
void* mmgr_alloc_const_data(size_t size) {
    if (!g_mmgr.is_initialized || size == 0) {
        return NULL;
    }
    
    mmgr_variable_pool_t *pool = &g_mmgr.const_pool;
    
    size_t aligned_size = (size + 3) & ~3;
    size_t total_size = aligned_size + sizeof(struct mmgr_allocation_header);
    
    if (pool->used_size + total_size > pool->pool_size) {
        MMGR_DEBUG_LOG("Constant pool exhausted");
        return NULL;
    }
    
    if (pool->allocation_count >= 1000) {
        MMGR_DEBUG_LOG("Too many constant allocations");
        return NULL;
    }
    
    struct mmgr_allocation_header *header = 
        (struct mmgr_allocation_header*)(pool->memory_base + pool->used_size);
    header->size = aligned_size;
    header->magic = MMGR_ALLOCATION_MAGIC;
    header->is_free = false;
    
    pool->allocations[pool->allocation_count] = header;
    pool->allocation_count++;
    pool->used_size += total_size;
    
    void *ptr = (uint8_t*)header + sizeof(struct mmgr_allocation_header);
    memset(ptr, 0, aligned_size);
    
    mmgr_update_statistics(MMGR_POOL_CONST, total_size, true);
    
    return ptr;
}

/* 获取内存统计信息 */
mmgr_statistics_t* mmgr_get_statistics(void) {
    if (!g_mmgr.is_initialized) {
        return NULL;
    }
    
    return &g_mmgr.stats;
}

/* 获取池使用情况 */
uint32_t mmgr_get_pool_usage(mmgr_pool_type_t pool_type) {
    if (!g_mmgr.is_initialized || pool_type >= MMGR_POOL_COUNT) {
        return 0;
    }
    
    return g_mmgr.stats.pool_usage[pool_type];
}

/* 验证内存完整性 */
bool mmgr_verify_integrity(void) {
    if (!g_mmgr.is_initialized) {
        return false;
    }
    
    /* 检查魔数 */
    if (g_mmgr.magic_number != MMGR_MAGIC) {
        MMGR_DEBUG_LOG("MMGR magic number corrupted");
        return false;
    }
    
    /* 检查校验和 */
    uint32_t calculated_checksum = mmgr_calculate_checksum(&g_mmgr,
                                                           sizeof(mmgr_static_manager_t) - sizeof(uint32_t));
    if (calculated_checksum != g_mmgr.checksum) {
        MMGR_DEBUG_LOG("MMGR checksum mismatch");
        return false;
    }
    
    return true;
}

/* 更新统计信息 */
static void mmgr_update_statistics(mmgr_pool_type_t pool_type, uint32_t size, bool is_allocation) {
    mmgr_statistics_t *stats = &g_mmgr.stats;
    
    if (is_allocation) {
        stats->total_allocated += size;
        stats->allocation_count++;
        stats->pool_usage[pool_type] += size;
        
        if (stats->total_allocated > stats->peak_allocated) {
            stats->peak_allocated = stats->total_allocated;
        }
    } else {
        stats->total_allocated -= size;
        stats->deallocation_count++;
        stats->pool_usage[pool_type] -= size;
    }
}

/* 计算校验和 */
static uint32_t mmgr_calculate_checksum(void *data, size_t size) {
    uint32_t checksum = 0;
    uint8_t *bytes = (uint8_t*)data;
    
    for (size_t i = 0; i < size; i++) {
        checksum += bytes[i];
    }
    
    return checksum;
}

/* 获取池名称 */
const char* mmgr_get_pool_name(mmgr_pool_type_t pool_type) {
    switch (pool_type) {
        case MMGR_POOL_STRING: return "String Pool";
        case MMGR_POOL_AST_NODE: return "AST Node Pool";
        case MMGR_POOL_SYMBOL: return "Symbol Pool";
        case MMGR_POOL_TYPE_INFO: return "Type Info Pool";
        case MMGR_POOL_LIBRARY: return "Library Pool";
        case MMGR_POOL_BYTECODE: return "Bytecode Pool";
        case MMGR_POOL_CONST: return "Constant Pool";
        default: return "Unknown Pool";
    }
}

/* 打印内存统计 */
void mmgr_print_statistics(void) {
    if (!g_mmgr.is_initialized) {
        printf("MMGR module not initialized\n");
        return;
    }
    
    mmgr_statistics_t *stats = &g_mmgr.stats;
    
    printf("\n=== MMGR Statistics ===\n");
    printf("Total Allocated: %u bytes\n", stats->total_allocated);
    printf("Peak Allocated: %u bytes\n", stats->peak_allocated);
    printf("Allocations: %u\n", stats->allocation_count);
    printf("Deallocations: %u\n", stats->deallocation_count);
    printf("Active Allocations: %u\n", stats->allocation_count - stats->deallocation_count);
    
    printf("\nPool Usage:\n");
    for (int i = 0; i < MMGR_POOL_COUNT; i++) {
        printf("  %s: %u bytes\n", mmgr_get_pool_name(i), stats->pool_usage[i]);
    }
    
    printf("\nPool Details:\n");
    printf("  String Pool: %u/%u entries, %u/%u bytes\n",
           g_mmgr.string_pool.entry_count, MMGR_MAX_STRINGS,
           g_mmgr.string_pool.used_size, MMGR_MAX_STRING_POOL_SIZE);
    printf("  AST Node Pool: %u/%u blocks\n",
           g_mmgr.ast_node_pool.allocated_count, MMGR_MAX_AST_NODES);
    printf("  Symbol Pool: %u/%u blocks\n",
           g_mmgr.symbol_pool.allocated_count, MMGR_MAX_SYMBOLS);
    printf("  Type Info Pool: %u/%u blocks\n",
           g_mmgr.type_info_pool.allocated_count, MMGR_MAX_TYPE_INFO);
    printf("  Library Pool: %u/%u blocks\n",
           g_mmgr.library_pool.allocated_count, MMGR_MAX_LIBRARIES);
    printf("  Bytecode Pool: %u/%u bytes\n",
           g_mmgr.bytecode_pool.used_size, MMGR_MAX_BYTECODE_SIZE);
    printf("  Constant Pool: %u/%u bytes\n",
           g_mmgr.const_pool.used_size, MMGR_MAX_CONST_POOL_SIZE);
    printf("======================\n\n");
}

/* 启用内存调试 */
void mmgr_enable_debug(bool enable) {
    g_mmgr_debug_enabled = enable;
}

/* 设置调试回调 */
void mmgr_set_debug_callback(void (*callback)(const char *msg)) {
    g_mmgr_debug_callback = callback;
}

/* 检查内存管理器是否已初始化 */
bool mmgr_is_initialized(void) {
    return g_mmgr.is_initialized;
}

/* 清理内存管理器 */
void mmgr_cleanup(void) {
    if (!g_mmgr.is_initialized) {
        return;
    }
    
    MMGR_DEBUG_LOG("Cleaning up MMGR module...");
    
    /* 重置所有池 */
    memset(&g_mmgr, 0, sizeof(mmgr_static_manager_t));
    
    MMGR_DEBUG_LOG("MMGR module cleanup completed");
}

/* ========== 通用内存分配 ========== */

/* 通用内存分配器 - 用于分配任意大小的内存块 */
void* mmgr_alloc_general(size_t size) {
    if (size == 0 || !g_mmgr.is_initialized) {
        mmgr_set_error("Invalid size or MMGR not initialized");
        return NULL;
    }
    
    /* 检查是否有足够的剩余内存 */
    if (g_mmgr.general_pool.used + size > g_mmgr.general_pool.total_size) {
        mmgr_set_error("General memory pool exhausted");
        return NULL;
    }
    
    /* 内存对齐处理 - 8字节对齐 */
    size_t aligned_size = ALIGN_SIZE(size, 8);
    
    /* 检查对齐后的大小 */
    if (g_mmgr.general_pool.used + aligned_size > g_mmgr.general_pool.total_size) {
        mmgr_set_error("General memory pool exhausted after alignment");
        return NULL;
    }
    
    /* 计算分配地址 */
    void *ptr = (char*)g_mmgr.general_pool.pool + g_mmgr.general_pool.used;
    
    /* 更新使用统计 */
    g_mmgr.general_pool.used += aligned_size;
    g_mmgr.general_pool.allocations++;
    g_mmgr.stats.allocation_count++;
    g_mmgr.stats.total_allocated += aligned_size;
    
    /* 记录分配信息（用于调试） */
    if (g_mmgr.debug_enabled) {
        allocation_record_t *record = &g_mmgr.allocation_records[g_mmgr.record_count % MAX_ALLOCATION_RECORDS];
        record->ptr = ptr;
        record->size = aligned_size;
        record->type = ALLOC_GENERAL;
        record->line = 0;  // 调用者可以通过mmgr_alloc_general_debug设置
        record->file = "unknown";
        record->is_active = true;
        g_mmgr.record_count++;
    }
    
    /* 内存初始化为零 */
    memset(ptr, 0, aligned_size);
    
    return ptr;
}

#ifdef MMGR_DEBUG
/* 通用内存分配器（带调试信息） */
void* mmgr_alloc_general_debug(size_t size, const char *file, int line) {
    void *ptr = mmgr_alloc_general(size);
    
    if (ptr && g_mmgr.debug_enabled && g_mmgr.record_count > 0) {
        /* 更新最后一条分配记录的调试信息 */
        allocation_record_t *record = &g_mmgr.allocation_records[(g_mmgr.record_count - 1) % MAX_ALLOCATION_RECORDS];
        if (record->ptr == ptr) {
            record->file = file ? file : "unknown";
            record->line = line;
        }
    }
    
    return ptr;
}
#endif

/* 通用内存重新分配 */
void* mmgr_realloc_general(void *ptr, size_t new_size) {
    if (!g_mmgr.is_initialized) {
        mmgr_set_error("MMGR not initialized");
        return NULL;
    }
    
    /* 如果ptr为NULL，相当于malloc */
    if (!ptr) {
        return mmgr_alloc_general(new_size);
    }
    
    /* 如果new_size为0，相当于free（但在静态内存管理中不释放） */
    if (new_size == 0) {
        return NULL;
    }
    
    /* 查找原始分配记录 */
    allocation_record_t *record = NULL;
    for (uint32_t i = 0; i < MAX_ALLOCATION_RECORDS && i < g_mmgr.record_count; i++) {
        allocation_record_t *r = &g_mmgr.allocation_records[i];
        if (r->ptr == ptr && r->is_active) {
            record = r;
            break;
        }
    }
    
    if (!record) {
        mmgr_set_error("Invalid pointer for reallocation");
        return NULL;
    }
    
    size_t old_size = record->size;
    
    /* 如果新大小小于等于旧大小，直接返回原指针 */
    if (new_size <= old_size) {
        return ptr;
    }
    
    /* 分配新内存 */
    void *new_ptr = mmgr_alloc_general(new_size);
    if (!new_ptr) {
        return NULL;
    }
    
    /* 复制旧数据 */
    memcpy(new_ptr, ptr, old_size);
    
    /* 标记旧分配记录为非活跃（在静态内存管理中不实际释放） */
    record->is_active = false;
    
    return new_ptr;
}

/* 获取通用内存池统计信息 */
void mmgr_get_general_stats(mmgr_general_stats_t *stats) {
    if (!stats || !g_mmgr.is_initialized) {
        return;
    }
    
    stats->total_size = g_mmgr.general_pool.total_size;
    stats->used_size = g_mmgr.general_pool.used;
    stats->free_size = g_mmgr.general_pool.total_size - g_mmgr.general_pool.used;
    stats->allocation_count = g_mmgr.general_pool.allocations;
    stats->fragmentation_ratio = 0.0f; // 静态分配无碎片
}

/* 检查指针是否在通用内存池范围内 */
bool mmgr_is_general_pointer(const void *ptr) {
    if (!ptr || !g_mmgr.is_initialized) {
        return false;
    }
    
    const char *pool_start = (const char*)g_mmgr.general_pool.pool;
    const char *pool_end = pool_start + g_mmgr.general_pool.total_size;
    const char *check_ptr = (const char*)ptr;
    
    return (check_ptr >= pool_start && check_ptr < pool_end);
}

/* 通用内存池压缩（静态内存管理中的模拟实现） */
size_t mmgr_compact_general_pool(void) {
    if (!g_mmgr.is_initialized) {
        return 0;
    }
    
    /* 在静态内存管理中，压缩操作主要是统计和重置非活跃分配 */
    size_t reclaimed = 0;
    uint32_t active_allocations = 0;
    
    for (uint32_t i = 0; i < MAX_ALLOCATION_RECORDS && i < g_mmgr.record_count; i++) {
        allocation_record_t *record = &g_mmgr.allocation_records[i];
        if (record->type == ALLOC_GENERAL) {
            if (record->is_active) {
                active_allocations++;
            } else {
                reclaimed += record->size;
            }
        }
    }
    
    /* 更新统计信息 */
    g_mmgr.general_pool.allocations = active_allocations;
    
    return reclaimed;
}

/* 通用内存池清理 */
void mmgr_clear_general_pool(void) {
    if (!g_mmgr.is_initialized) {
        return;
    }
    
    /* 重置池状态 */
    g_mmgr.general_pool.used = 0;
    g_mmgr.general_pool.allocations = 0;
    
    /* 标记所有通用分配记录为非活跃 */
    for (uint32_t i = 0; i < MAX_ALLOCATION_RECORDS && i < g_mmgr.record_count; i++) {
        allocation_record_t *record = &g_mmgr.allocation_records[i];
        if (record->type == ALLOC_GENERAL) {
            record->is_active = false;
        }
    }
    
    /* 清零内存池 */
    if (g_mmgr.general_pool.pool) {
        memset(g_mmgr.general_pool.pool, 0, g_mmgr.general_pool.total_size);
    }
}

/* 设置错误 */
void mmgr_set_error(const char *msg) {
    if (msg && g_mmgr.is_initialized) {
        strncpy(g_mmgr.last_error, msg, sizeof(g_mmgr.last_error) - 1);
        g_mmgr.last_error[sizeof(g_mmgr.last_error) - 1] = '\0';
        MMGR_DEBUG_LOG(g_mmgr.last_error);
    }
}

/* 通用内存分配器调试信息 */
void mmgr_dump_general_allocations(void) {
    if (!g_mmgr.is_initialized || !g_mmgr.debug_enabled) {
        printf("General memory debugging not available\n");
        return;
    }
    
    printf("\n=== General Memory Allocations ===\n");
    printf("Pool size: %zu bytes\n", g_mmgr.general_pool.total_size);
    printf("Used: %zu bytes (%.1f%%)\n", 
           g_mmgr.general_pool.used,
           100.0 * g_mmgr.general_pool.used / g_mmgr.general_pool.total_size);
    printf("Free: %zu bytes\n", 
           g_mmgr.general_pool.total_size - g_mmgr.general_pool.used);
    printf("Active allocations: %u\n", g_mmgr.general_pool.allocations);
    
    printf("\nActive Allocation Details:\n");
    uint32_t count = 0;
    size_t total_active = 0;
    
    for (uint32_t i = 0; i < MAX_ALLOCATION_RECORDS && i < g_mmgr.record_count; i++) {
        allocation_record_t *record = &g_mmgr.allocation_records[i];
        if (record->type == ALLOC_GENERAL && record->is_active) {
            printf("  [%u] %p: %zu bytes (%s:%d)\n", 
                   count++, record->ptr, record->size, 
                   record->file, record->line);
            total_active += record->size;
        }
    }
    
    printf("Total active memory: %zu bytes\n", total_active);
    printf("================================\n\n");
}