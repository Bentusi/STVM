/**
 * @file iomgr_new.h
 * @brief STVM 硬件 I/O 管理器 - 头文件 (简化版)
 */

#ifndef STVM_IOMGR_H
#define STVM_IOMGR_H

#include "types.h"
#include "error.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <pthread.h>

// ============================================================================
// I/O 设备类型
// ============================================================================

typedef enum {
    IO_DEV_GPIO_IN      = 0x01,
    IO_DEV_GPIO_OUT     = 0x02,
    IO_DEV_ADC          = 0x04,
    IO_DEV_DAC          = 0x08,
    IO_DEV_PWM          = 0x10,
    IO_DEV_ENCODER      = 0x20
} IODeviceType;

// ============================================================================
// I/O 访问模式
// ============================================================================

typedef enum {
    IO_ACCESS_READ_ONLY  = 0x01,
    IO_ACCESS_WRITE_ONLY = 0x02,
    IO_ACCESS_READ_WRITE = 0x03,
    IO_ACCESS_READ       = 0x01,
    IO_ACCESS_WRITE      = 0x02
} IOAccessMode;

// ============================================================================
// IEC 61131-3 地址定义
// ============================================================================

typedef enum {
    IO_LOC_INPUT  = 'I',
    IO_LOC_OUTPUT = 'Q',
    IO_LOC_MEMORY = 'M'
} IOLocation;

typedef enum {
    IO_SIZE_BIT   = 'X',
    IO_SIZE_BYTE  = 'B',
    IO_SIZE_WORD  = 'W',
    IO_SIZE_DWORD = 'D',
    IO_SIZE_LWORD = 'L'
} IODataSize;

typedef struct {
    IOLocation location;
    IODataSize size;
    uint32_t byte_offset;
    uint8_t bit_offset;
} IOAddress;

// ============================================================================
// I/O 点配置与状态
// ============================================================================

typedef struct {
    IOAddress address;
    IODeviceType device_type;
    IOAccessMode access_mode;
    char* hardware_path;
    uint32_t hardware_address;
    double scale;
    double offset;
    bool enable_filter;
    uint32_t filter_samples;
} IOPointConfig;

typedef struct {
    IOPointConfig config;
    Value current_value;
    void* hal_handle;
    Value* filter_buffer;
    uint64_t read_count;
    uint64_t write_count;
    uint64_t error_count;
    uint64_t last_update_us;
    bool initialized;
    pthread_mutex_t mutex;
} IOPoint;

// ============================================================================
// 硬件抽象层适配器
// ============================================================================

typedef struct IOHardwareAdapter {
    char platform_name[64];
    char version[32];
    uint32_t supported_types;
    
    ErrorCode (*init)(struct IOHardwareAdapter* adapter);
    void (*cleanup)(struct IOHardwareAdapter* adapter);
    
    void* (*open_device)(IODeviceType type, const char* path, uint32_t addr);
    void (*close_device)(void* handle);
    
    ErrorCode (*read)(void* handle, uint32_t address, Value* value);
    ErrorCode (*write)(void* handle, uint32_t address, const Value* value);
    
    ErrorCode (*read_batch)(void* handle, uint32_t start_addr, uint32_t count, Value* values);
    ErrorCode (*write_batch)(void* handle, uint32_t start_addr, uint32_t count, const Value* values);
    
    ErrorCode (*set_parameter)(void* handle, const char* param_name, const Value* value);
    ErrorCode (*get_parameter)(void* handle, const char* param_name, Value* value);
    
    void* platform_data;
} IOHardwareAdapter;

// ============================================================================
// I/O 管理器
// ============================================================================

typedef struct {
    IOPoint** io_points;
    uint32_t point_count;
    uint32_t point_capacity;
    
    struct {
        IOPoint** inputs;
        IOPoint** outputs;
        IOPoint** memory;
        uint32_t input_size;
        uint32_t output_size;
        uint32_t memory_size;
    } lookup;
    
    IOHardwareAdapter* hal_adapter;
    
    bool auto_refresh;
    uint32_t refresh_cycle_us;
    pthread_t refresh_thread;
    pthread_mutex_t mgr_mutex;
    
    struct {
        uint64_t total_reads;
        uint64_t total_writes;
        uint64_t total_errors;
        uint64_t refresh_cycles;
        uint64_t start_time_us;
    } stats;
    
    void (*log_callback)(const char* level, const char* message);
} IOManager;

// ============================================================================
// API 函数声明
// ============================================================================

// 管理器生命周期
IOManager* io_manager_create(IOHardwareAdapter* hal_adapter);
void io_manager_free(IOManager* mgr);

// I/O 点管理
ErrorCode io_manager_add_point(IOManager* mgr, const IOPointConfig* config);
ErrorCode io_manager_remove_point(IOManager* mgr, const IOAddress* addr);
IOPoint* io_manager_find_point(IOManager* mgr, const IOAddress* addr);

// I/O 读写
ErrorCode io_manager_read(IOManager* mgr, const IOAddress* addr, Value* value);
ErrorCode io_manager_write(IOManager* mgr, const IOAddress* addr, const Value* value);
ErrorCode io_manager_refresh_inputs(IOManager* mgr);
ErrorCode io_manager_refresh_outputs(IOManager* mgr);

// 自动刷新
ErrorCode io_manager_start_refresh(IOManager* mgr, uint32_t cycle_us);
ErrorCode io_manager_stop_refresh(IOManager* mgr);

// 地址解析
ErrorCode io_address_parse(const char* addr_str, IOAddress* addr);
void io_address_format(const IOAddress* addr, char* buffer, size_t size);
bool io_address_equal(const IOAddress* addr1, const IOAddress* addr2);

// 统计与诊断
void io_manager_reset_stats(IOManager* mgr);
void io_manager_set_log_callback(IOManager* mgr, void (*callback)(const char* level, const char* message));
uint32_t io_manager_get_point_count(IOManager* mgr);
IOPoint* io_manager_get_point(IOManager* mgr, uint32_t index);

// 适配器创建
IOHardwareAdapter* io_adapter_create_simulator(void);
void io_adapter_free_simulator(IOHardwareAdapter* adapter);

#endif // STVM_IOMGR_H
