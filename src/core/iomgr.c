/**
 * @file iomgr.c
 * @brief STVM 硬件 I/O 管理器 - 实现
 * 
 * 功能：
 * - IEC 61131-3 标准 I/O 地址解析
 * - I/O 点管理（添加、删除、查找）
 * - I/O 读写操作
 * - 自动刷新线程
 * - 统计与诊断
 */

#define _POSIX_C_SOURCE 200809L

#include "iomgr.h"
#include "mmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>

// ============================================================================
// 辅助函数
// ============================================================================

/**
 * @brief 获取当前时间（微秒）
 */
static uint64_t get_time_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

/**
 * @brief 日志输出
 */
static void log_message(IOManager* mgr, const char* level, const char* fmt, ...) {
    if (mgr && mgr->log_callback) {
        char buffer[512];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        mgr->log_callback(level, buffer);
    } else {
        // 默认输出到 stderr
        va_list args;
        va_start(args, fmt);
        fprintf(stderr, "[IO:%s] ", level);
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
        va_end(args);
    }
}

// ============================================================================
// I/O 地址解析
// ============================================================================

/**
 * @brief 解析 IEC 61131-3 I/O 地址字符串
 * 
 * 格式: %<location><size><offset>.<bit>
 * 
 * 示例:
 *   %IX0.0  -> location=I, size=X, byte_offset=0, bit_offset=0
 *   %QW5    -> location=Q, size=W, byte_offset=5, bit_offset=0
 *   %MD100  -> location=M, size=D, byte_offset=100, bit_offset=0
 */
ErrorCode io_address_parse(const char* addr_str, IOAddress* addr) {
    if (!addr_str || !addr || strlen(addr_str) < 4) {
        return ERR_INVALID_ARGUMENT;
    }
    
    // 检查 '%' 前缀
    if (addr_str[0] != '%') {
        return ERR_INVALID_ARGUMENT;
    }
    
    // 解析 location (I/Q/M)
    char loc = addr_str[1];
    if (loc != 'I' && loc != 'Q' && loc != 'M') {
        return ERR_INVALID_ARGUMENT;
    }
    addr->location = (IOLocation)loc;
    
    // 解析 size (X/B/W/D/L)
    char size = addr_str[2];
    if (size != 'X' && size != 'B' && size != 'W' && size != 'D' && size != 'L') {
        return ERR_INVALID_ARGUMENT;
    }
    addr->size = (IODataSize)size;
    
    // 解析 byte offset
    const char* offset_str = &addr_str[3];
    char* dot_pos = strchr(offset_str, '.');
    
    if (dot_pos) {
        // 有位偏移（仅对 X 有效）
        if (size != 'X') {
            return ERR_INVALID_ARGUMENT;
        }
        
        // 解析字节偏移
        char byte_str[32];
        size_t byte_len = dot_pos - offset_str;
        if (byte_len >= sizeof(byte_str)) {
            return ERR_INVALID_ARGUMENT;
        }
        strncpy(byte_str, offset_str, byte_len);
        byte_str[byte_len] = '\0';
        addr->byte_offset = atoi(byte_str);
        
        // 解析位偏移
        addr->bit_offset = atoi(dot_pos + 1);
        if (addr->bit_offset > 7) {
            return ERR_INVALID_ARGUMENT;
        }
    } else {
        // 无位偏移
        addr->byte_offset = atoi(offset_str);
        addr->bit_offset = 0;
    }
    
    return OK;
}

/**
 * @brief 格式化 I/O 地址为字符串
 */
void io_address_format(const IOAddress* addr, char* buffer, size_t size) {
    if (!addr || !buffer || size == 0) {
        return;
    }
    
    if (addr->size == IO_SIZE_BIT) {
        snprintf(buffer, size, "%%%c%c%u.%u",
                 (char)addr->location,
                 (char)addr->size,
                 addr->byte_offset,
                 addr->bit_offset);
    } else {
        snprintf(buffer, size, "%%%c%c%u",
                 (char)addr->location,
                 (char)addr->size,
                 addr->byte_offset);
    }
}

/**
 * @brief 比较两个 I/O 地址是否相等
 */
bool io_address_equal(const IOAddress* addr1, const IOAddress* addr2) {
    if (!addr1 || !addr2) {
        return false;
    }
    
    return addr1->location == addr2->location &&
           addr1->size == addr2->size &&
           addr1->byte_offset == addr2->byte_offset &&
           addr1->bit_offset == addr2->bit_offset;
}

// ============================================================================
// I/O 管理器生命周期
// ============================================================================

/**
 * @brief 创建 I/O 管理器
 */
IOManager* io_manager_create(IOHardwareAdapter* hal_adapter) {
    IOManager* mgr = (IOManager*)mmgr_alloc(sizeof(IOManager));
    if (!mgr) {
        return NULL;
    }
    
    memset(mgr, 0, sizeof(IOManager));
    
    // 设置 HAL 适配器
    mgr->hal_adapter = hal_adapter;
    
    // 初始化 I/O 点数组
    mgr->point_capacity = 64;  // 初始容量
    mgr->io_points = (IOPoint**)mmgr_alloc(sizeof(IOPoint*) * mgr->point_capacity);
    if (!mgr->io_points) {
        mmgr_free(mgr);
        return NULL;
    }
    
    // 初始化查找表
    mgr->lookup.input_size = 256;  // 假设最多 256 个输入字节
    mgr->lookup.output_size = 256;
    mgr->lookup.memory_size = 1024;
    
    mgr->lookup.inputs = (IOPoint**)mmgr_alloc(sizeof(IOPoint*) * mgr->lookup.input_size);
    mgr->lookup.outputs = (IOPoint**)mmgr_alloc(sizeof(IOPoint*) * mgr->lookup.output_size);
    mgr->lookup.memory = (IOPoint**)mmgr_alloc(sizeof(IOPoint*) * mgr->lookup.memory_size);
    
    if (!mgr->lookup.inputs || !mgr->lookup.outputs || !mgr->lookup.memory) {
        if (mgr->lookup.inputs) mmgr_free(mgr->lookup.inputs);
        if (mgr->lookup.outputs) mmgr_free(mgr->lookup.outputs);
        if (mgr->lookup.memory) mmgr_free(mgr->lookup.memory);
        mmgr_free(mgr->io_points);
        mmgr_free(mgr);
        return NULL;
    }
    
    memset(mgr->lookup.inputs, 0, sizeof(IOPoint*) * mgr->lookup.input_size);
    memset(mgr->lookup.outputs, 0, sizeof(IOPoint*) * mgr->lookup.output_size);
    memset(mgr->lookup.memory, 0, sizeof(IOPoint*) * mgr->lookup.memory_size);
    
    // 初始化互斥锁
    pthread_mutex_init(&mgr->mgr_mutex, NULL);
    
    // 初始化统计
    mgr->stats.start_time_us = get_time_us();
    
    // 初始化 HAL 适配器
    if (hal_adapter && hal_adapter->init) {
        hal_adapter->init(hal_adapter);
    }
    
    log_message(mgr, "INFO", "I/O Manager created");
    
    return mgr;
}

/**
 * @brief 释放 I/O 管理器
 */
void io_manager_free(IOManager* mgr) {
    if (!mgr) {
        return;
    }
    
    // 停止自动刷新
    if (mgr->auto_refresh) {
        io_manager_stop_refresh(mgr);
    }
    
    // 释放所有 I/O 点
    for (uint32_t i = 0; i < mgr->point_count; i++) {
        IOPoint* point = mgr->io_points[i];
        if (point) {
            // 关闭硬件设备
            if (mgr->hal_adapter && mgr->hal_adapter->close_device && point->hal_handle) {
                mgr->hal_adapter->close_device(point->hal_handle);
            }
            
            // 释放滤波器缓冲区
            if (point->filter_buffer) {
                mmgr_free(point->filter_buffer);
            }
            
            // 释放配置字符串
            if (point->config.hardware_path) {
                mmgr_free(point->config.hardware_path);
            }
            
            // 销毁互斥锁
            pthread_mutex_destroy(&point->mutex);
            
            mmgr_free(point);
        }
    }
    
    // 释放查找表
    if (mgr->lookup.inputs) mmgr_free(mgr->lookup.inputs);
    if (mgr->lookup.outputs) mmgr_free(mgr->lookup.outputs);
    if (mgr->lookup.memory) mmgr_free(mgr->lookup.memory);
    
    // 释放 I/O 点数组
    if (mgr->io_points) mmgr_free(mgr->io_points);
    
    // 清理 HAL 适配器
    if (mgr->hal_adapter && mgr->hal_adapter->cleanup) {
        mgr->hal_adapter->cleanup(mgr->hal_adapter);
    }
    
    // 销毁互斥锁
    pthread_mutex_destroy(&mgr->mgr_mutex);
    
    log_message(mgr, "INFO", "I/O Manager freed");
    
    mmgr_free(mgr);
}

// ============================================================================
// I/O 点管理
// ============================================================================

/**
 * @brief 查找 I/O 点
 */
IOPoint* io_manager_find_point(IOManager* mgr, const IOAddress* addr) {
    if (!mgr || !addr) {
        return NULL;
    }
    
    // 根据地址类型选择查找表
    IOPoint** lookup_table = NULL;
    uint32_t lookup_size = 0;
    
    switch (addr->location) {
        case IO_LOC_INPUT:
            lookup_table = mgr->lookup.inputs;
            lookup_size = mgr->lookup.input_size;
            break;
        case IO_LOC_OUTPUT:
            lookup_table = mgr->lookup.outputs;
            lookup_size = mgr->lookup.output_size;
            break;
        case IO_LOC_MEMORY:
            lookup_table = mgr->lookup.memory;
            lookup_size = mgr->lookup.memory_size;
            break;
        default:
            return NULL;
    }
    
    // 快速查找
    if (addr->byte_offset < lookup_size) {
        IOPoint* point = lookup_table[addr->byte_offset];
        if (point && io_address_equal(&point->config.address, addr)) {
            return point;
        }
    }
    
    // 遍历查找（备用）
    for (uint32_t i = 0; i < mgr->point_count; i++) {
        IOPoint* point = mgr->io_points[i];
        if (point && io_address_equal(&point->config.address, addr)) {
            return point;
        }
    }
    
    return NULL;
}

/**
 * @brief 添加 I/O 点
 */
ErrorCode io_manager_add_point(IOManager* mgr, const IOPointConfig* config) {
    if (!mgr || !config) {
        return ERR_INVALID_ARGUMENT;
    }
    
    pthread_mutex_lock(&mgr->mgr_mutex);
    
    // 检查是否已存在
    if (io_manager_find_point(mgr, &config->address)) {
        pthread_mutex_unlock(&mgr->mgr_mutex);
        log_message(mgr, "ERROR", "I/O point already exists");
        return ERR_ALREADY_EXISTS;
    }
    
    // 检查容量
    if (mgr->point_count >= mgr->point_capacity) {
        // 扩容
        uint32_t new_capacity = mgr->point_capacity * 2;
        IOPoint** new_points = (IOPoint**)mmgr_alloc(sizeof(IOPoint*) * new_capacity);
        if (!new_points) {
            pthread_mutex_unlock(&mgr->mgr_mutex);
            return ERR_OUT_OF_MEMORY;
        }
        
        memcpy(new_points, mgr->io_points, sizeof(IOPoint*) * mgr->point_count);
        mmgr_free(mgr->io_points);
        mgr->io_points = new_points;
        mgr->point_capacity = new_capacity;
    }
    
    // 创建 I/O 点
    IOPoint* point = (IOPoint*)mmgr_alloc(sizeof(IOPoint));
    if (!point) {
        pthread_mutex_unlock(&mgr->mgr_mutex);
        return ERR_OUT_OF_MEMORY;
    }
    
    memset(point, 0, sizeof(IOPoint));
    memcpy(&point->config, config, sizeof(IOPointConfig));
    
    // 复制硬件路径字符串
    if (config->hardware_path) {
        point->config.hardware_path = strdup(config->hardware_path);
    }
    
    // 初始化互斥锁
    pthread_mutex_init(&point->mutex, NULL);
    
    // 初始化滤波器
    if (config->enable_filter && config->filter_samples > 0) {
        point->filter_buffer = (Value*)mmgr_alloc(sizeof(Value) * config->filter_samples);
        if (!point->filter_buffer) {
            pthread_mutex_destroy(&point->mutex);
            if (point->config.hardware_path) mmgr_free(point->config.hardware_path);
            mmgr_free(point);
            pthread_mutex_unlock(&mgr->mgr_mutex);
            return ERR_OUT_OF_MEMORY;
        }
        memset(point->filter_buffer, 0, sizeof(Value) * config->filter_samples);
    }
    
    // 打开硬件设备
    if (mgr->hal_adapter && mgr->hal_adapter->open_device) {
        point->hal_handle = mgr->hal_adapter->open_device(
            config->device_type,
            config->hardware_path,
            config->hardware_address
        );
        
        if (!point->hal_handle) {
            log_message(mgr, "WARNING", "Failed to open hardware device: %s",
                       config->hardware_path ? config->hardware_path : "NULL");
        }
    }
    
    point->initialized = true;
    
    // 添加到数组
    mgr->io_points[mgr->point_count++] = point;
    
    // 添加到查找表
    IOPoint** lookup_table = NULL;
    uint32_t lookup_size = 0;
    
    switch (config->address.location) {
        case IO_LOC_INPUT:
            lookup_table = mgr->lookup.inputs;
            lookup_size = mgr->lookup.input_size;
            break;
        case IO_LOC_OUTPUT:
            lookup_table = mgr->lookup.outputs;
            lookup_size = mgr->lookup.output_size;
            break;
        case IO_LOC_MEMORY:
            lookup_table = mgr->lookup.memory;
            lookup_size = mgr->lookup.memory_size;
            break;
    }
    
    if (lookup_table && config->address.byte_offset < lookup_size) {
        lookup_table[config->address.byte_offset] = point;
    }
    
    pthread_mutex_unlock(&mgr->mgr_mutex);
    
    char addr_str[32];
    io_address_format(&config->address, addr_str, sizeof(addr_str));
    log_message(mgr, "INFO", "I/O point added: %s", addr_str);
    
    return OK;
}

/**
 * @brief 移除 I/O 点
 */
ErrorCode io_manager_remove_point(IOManager* mgr, const IOAddress* addr) {
    if (!mgr || !addr) {
        return ERR_INVALID_ARGUMENT;
    }
    
    pthread_mutex_lock(&mgr->mgr_mutex);
    
    // 查找点
    IOPoint* point = io_manager_find_point(mgr, addr);
    if (!point) {
        pthread_mutex_unlock(&mgr->mgr_mutex);
        return ERR_NOT_FOUND;
    }
    
    // 从查找表移除
    IOPoint** lookup_table = NULL;
    uint32_t lookup_size = 0;
    
    switch (addr->location) {
        case IO_LOC_INPUT:
            lookup_table = mgr->lookup.inputs;
            lookup_size = mgr->lookup.input_size;
            break;
        case IO_LOC_OUTPUT:
            lookup_table = mgr->lookup.outputs;
            lookup_size = mgr->lookup.output_size;
            break;
        case IO_LOC_MEMORY:
            lookup_table = mgr->lookup.memory;
            lookup_size = mgr->lookup.memory_size;
            break;
    }
    
    if (lookup_table && addr->byte_offset < lookup_size) {
        lookup_table[addr->byte_offset] = NULL;
    }
    
    // 从数组移除
    for (uint32_t i = 0; i < mgr->point_count; i++) {
        if (mgr->io_points[i] == point) {
            // 关闭硬件设备
            if (mgr->hal_adapter && mgr->hal_adapter->close_device && point->hal_handle) {
                mgr->hal_adapter->close_device(point->hal_handle);
            }
            
            // 释放资源
            if (point->filter_buffer) mmgr_free(point->filter_buffer);
            if (point->config.hardware_path) mmgr_free(point->config.hardware_path);
            pthread_mutex_destroy(&point->mutex);
            mmgr_free(point);
            
            // 移动后续元素
            for (uint32_t j = i; j < mgr->point_count - 1; j++) {
                mgr->io_points[j] = mgr->io_points[j + 1];
            }
            mgr->point_count--;
            
            break;
        }
    }
    
    pthread_mutex_unlock(&mgr->mgr_mutex);
    
    char addr_str[32];
    io_address_format(addr, addr_str, sizeof(addr_str));
    log_message(mgr, "INFO", "I/O point removed: %s", addr_str);
    
    return OK;
}

// ============================================================================
// I/O 读写操作
// ============================================================================

/**
 * @brief 读取 I/O 点
 */
ErrorCode io_manager_read(IOManager* mgr, const IOAddress* addr, Value* value) {
    if (!mgr || !addr || !value) {
        return ERR_INVALID_ARGUMENT;
    }
    
    IOPoint* point = io_manager_find_point(mgr, addr);
    if (!point) {
        return ERR_NOT_FOUND;
    }
    
    pthread_mutex_lock(&point->mutex);
    
    // 检查权限
    if (!(point->config.access_mode & IO_ACCESS_READ)) {
        pthread_mutex_unlock(&point->mutex);
        log_message(mgr, "ERROR", "I/O point is not readable");
        return ERR_PERMISSION_DENIED;
    }
    
    // 从缓存读取
    *value = point->current_value;
    
    // 更新统计
    point->read_count++;
    point->last_update_us = get_time_us();
    
    mgr->stats.total_reads++;
    
    pthread_mutex_unlock(&point->mutex);
    
    return OK;
}

/**
 * @brief 写入 I/O 点
 */
ErrorCode io_manager_write(IOManager* mgr, const IOAddress* addr, const Value* value) {
    if (!mgr || !addr || !value) {
        return ERR_INVALID_ARGUMENT;
    }
    
    IOPoint* point = io_manager_find_point(mgr, addr);
    if (!point) {
        return ERR_NOT_FOUND;
    }
    
    pthread_mutex_lock(&point->mutex);
    
    // 检查权限
    if (!(point->config.access_mode & IO_ACCESS_WRITE)) {
        pthread_mutex_unlock(&point->mutex);
        log_message(mgr, "ERROR", "I/O point is not writable");
        return ERR_PERMISSION_DENIED;
    }
    
    // 写入缓存
    point->current_value = *value;
    
    // 立即写入硬件（如果有 HAL 适配器）
    if (mgr->hal_adapter && mgr->hal_adapter->write && point->hal_handle) {
        ErrorCode err = mgr->hal_adapter->write(
            point->hal_handle,
            point->config.hardware_address,
            value
        );
        
        if (err != OK) {
            pthread_mutex_unlock(&point->mutex);
            mgr->stats.total_errors++;
            log_message(mgr, "ERROR", "Hardware write failed");
            return err;
        }
    }
    
    // 更新统计
    point->write_count++;
    point->last_update_us = get_time_us();
    
    mgr->stats.total_writes++;
    
    pthread_mutex_unlock(&point->mutex);
    
    return OK;
}

/**
 * @brief 刷新输入（从硬件读取到缓存）
 */
ErrorCode io_manager_refresh_inputs(IOManager* mgr) {
    if (!mgr) {
        return ERR_INVALID_ARGUMENT;
    }
    
    ErrorCode result = OK;
    
    for (uint32_t i = 0; i < mgr->point_count; i++) {
        IOPoint* point = mgr->io_points[i];
        
        if (!point || point->config.address.location != IO_LOC_INPUT) {
            continue;
        }
        
        pthread_mutex_lock(&point->mutex);
        
        // 从硬件读取
        if (mgr->hal_adapter && mgr->hal_adapter->read && point->hal_handle) {
            Value raw_value;
            ErrorCode err = mgr->hal_adapter->read(
                point->hal_handle,
                point->config.hardware_address,
                &raw_value
            );
            
            if (err == OK) {
                // 应用滤波器
                if (point->config.enable_filter && point->config.filter_samples > 0) {
                    // 移动滤波器窗口
                    for (uint32_t j = point->config.filter_samples - 1; j > 0; j--) {
                        point->filter_buffer[j] = point->filter_buffer[j - 1];
                    }
                    point->filter_buffer[0] = raw_value;
                    
                    // 计算平均值
                    int64_t sum = 0;
                    for (uint32_t j = 0; j < point->config.filter_samples; j++) {
                        sum += point->filter_buffer[j].int_val;
                    }
                    point->current_value.int_val = (int32_t)(sum / point->config.filter_samples);
                    point->current_value.type = TYPE_INT;
                } else {
                    point->current_value = raw_value;
                }
                
                point->error_count = 0;  // 重置错误计数
            } else {
                point->error_count++;
                mgr->stats.total_errors++;
                result = err;
            }
        }
        
        pthread_mutex_unlock(&point->mutex);
    }
    
    mgr->stats.refresh_cycles++;
    
    return result;
}

/**
 * @brief 刷新输出（从缓存写入到硬件）
 */
ErrorCode io_manager_refresh_outputs(IOManager* mgr) {
    if (!mgr) {
        return ERR_INVALID_ARGUMENT;
    }
    
    ErrorCode result = OK;
    
    for (uint32_t i = 0; i < mgr->point_count; i++) {
        IOPoint* point = mgr->io_points[i];
        
        if (!point || point->config.address.location != IO_LOC_OUTPUT) {
            continue;
        }
        
        pthread_mutex_lock(&point->mutex);
        
        // 写入硬件
        if (mgr->hal_adapter && mgr->hal_adapter->write && point->hal_handle) {
            ErrorCode err = mgr->hal_adapter->write(
                point->hal_handle,
                point->config.hardware_address,
                &point->current_value
            );
            
            if (err == OK) {
                point->error_count = 0;
            } else {
                point->error_count++;
                mgr->stats.total_errors++;
                result = err;
            }
        }
        
        pthread_mutex_unlock(&point->mutex);
    }
    
    return result;
}

// ============================================================================
// 自动刷新线程
// ============================================================================

/**
 * @brief 自动刷新线程函数
 */
static void* refresh_thread_func(void* arg) {
    IOManager* mgr = (IOManager*)arg;
    
    log_message(mgr, "INFO", "Refresh thread started (cycle: %u us)", mgr->refresh_cycle_us);
    
    while (mgr->auto_refresh) {
        uint64_t start_time = get_time_us();
        
        // 刷新输入
        io_manager_refresh_inputs(mgr);
        
        // 刷新输出
        io_manager_refresh_outputs(mgr);
        
        // 计算耗时
        uint64_t elapsed = get_time_us() - start_time;
        
        // 睡眠剩余时间
        if (elapsed < mgr->refresh_cycle_us) {
            usleep(mgr->refresh_cycle_us - elapsed);
        } else {
            log_message(mgr, "WARNING", "Refresh cycle overrun: %lu us (limit: %u us)",
                       elapsed, mgr->refresh_cycle_us);
        }
    }
    
    log_message(mgr, "INFO", "Refresh thread stopped");
    
    return NULL;
}

/**
 * @brief 启动自动刷新
 */
ErrorCode io_manager_start_refresh(IOManager* mgr, uint32_t cycle_us) {
    if (!mgr) {
        return ERR_INVALID_ARGUMENT;
    }
    
    if (mgr->auto_refresh) {
        log_message(mgr, "WARNING", "Auto refresh already started");
        return OK;
    }
    
    mgr->refresh_cycle_us = cycle_us;
    mgr->auto_refresh = true;
    
    // 创建线程
    if (pthread_create(&mgr->refresh_thread, NULL, refresh_thread_func, mgr) != 0) {
        mgr->auto_refresh = false;
        log_message(mgr, "ERROR", "Failed to create refresh thread");
        return ERR_SYSTEM_ERROR;
    }
    
    return OK;
}

/**
 * @brief 停止自动刷新
 */
ErrorCode io_manager_stop_refresh(IOManager* mgr) {
    if (!mgr) {
        return ERR_INVALID_ARGUMENT;
    }
    
    if (!mgr->auto_refresh) {
        return OK;
    }
    
    mgr->auto_refresh = false;
    
    // 等待线程结束
    pthread_join(mgr->refresh_thread, NULL);
    
    log_message(mgr, "INFO", "Auto refresh stopped");
    
    return OK;
}

// ============================================================================
// 统计与诊断
// ============================================================================

/**
 * @brief 重置统计
 */
void io_manager_reset_stats(IOManager* mgr) {
    if (!mgr) {
        return;
    }
    
    pthread_mutex_lock(&mgr->mgr_mutex);
    
    mgr->stats.total_reads = 0;
    mgr->stats.total_writes = 0;
    mgr->stats.total_errors = 0;
    mgr->stats.refresh_cycles = 0;
    mgr->stats.start_time_us = get_time_us();
    
    pthread_mutex_unlock(&mgr->mgr_mutex);
    
    log_message(mgr, "INFO", "Statistics reset");
}

/**
 * @brief 设置日志回调
 */
void io_manager_set_log_callback(IOManager* mgr, void (*callback)(const char* level, const char* message)) {
    if (!mgr) {
        return;
    }
    
    mgr->log_callback = callback;
}

/**
 * @brief 获取 I/O 点数量
 */
uint32_t io_manager_get_point_count(IOManager* mgr) {
    if (!mgr) {
        return 0;
    }
    
    return mgr->point_count;
}

/**
 * @brief 获取指定索引的 I/O 点
 */
IOPoint* io_manager_get_point(IOManager* mgr, uint32_t index) {
    if (!mgr || index >= mgr->point_count) {
        return NULL;
    }
    
    return mgr->io_points[index];
}
