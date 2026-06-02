/**
 * @file io_adapter_sim.c
 * @brief STVM 硬件 I/O 模拟器适配器
 * 
 * 功能：
 * - 在内存中模拟 GPIO、ADC、DAC、PWM 等设备
 * - 用于开发和测试（无需真实硬件）
 */

#include "iomgr.h"
#include "mmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// ============================================================================
// 模拟设备结构
// ============================================================================

typedef struct {
    IODeviceType type;
    uint32_t address;
    Value value;           // 当前值
    bool is_open;
} SimDevice;

typedef struct {
    SimDevice* devices;
    uint32_t device_count;
    uint32_t device_capacity;
    pthread_mutex_t mutex;
} SimAdapter;

// ============================================================================
// 模拟适配器实现
// ============================================================================

/**
 * @brief 初始化模拟适配器
 */
static ErrorCode sim_init(IOHardwareAdapter* adapter) {
    if (!adapter) {
        return ERR_INVALID_ARGUMENT;
    }
    
    SimAdapter* sim = (SimAdapter*)mmgr_alloc(sizeof(SimAdapter));
    if (!sim) {
        return ERR_OUT_OF_MEMORY;
    }
    
    sim->device_capacity = 32;
    sim->devices = (SimDevice*)mmgr_alloc(sizeof(SimDevice) * sim->device_capacity);
    if (!sim->devices) {
        mmgr_free(sim);
        return ERR_OUT_OF_MEMORY;
    }
    
    sim->device_count = 0;
    pthread_mutex_init(&sim->mutex, NULL);
    
    adapter->platform_data = sim;
    
    printf("[SimAdapter] Initialized\n");
    
    return OK;
}

/**
 * @brief 清理模拟适配器
 */
static void sim_cleanup(IOHardwareAdapter* adapter) {
    if (!adapter || !adapter->platform_data) {
        return;
    }
    
    SimAdapter* sim = (SimAdapter*)adapter->platform_data;
    
    pthread_mutex_destroy(&sim->mutex);
    mmgr_free(sim->devices);
    mmgr_free(sim);
    
    adapter->platform_data = NULL;
    
    printf("[SimAdapter] Cleaned up\n");
}

/**
 * @brief 打开模拟设备
 */
static void* sim_open_device(IODeviceType type, const char* path, uint32_t address) {
    // 在模拟器中，直接返回设备地址作为句柄
    // 真实硬件中这里会打开设备文件或初始化硬件寄存器
    
    SimDevice* device = (SimDevice*)mmgr_alloc(sizeof(SimDevice));
    if (!device) {
        return NULL;
    }
    
    device->type = type;
    device->address = address;
    device->value.int_val = 0;  // 初始化为 0
    device->is_open = true;
    
    printf("[SimAdapter] Opened device: type=%d, addr=%u, path=%s\n",
           type, address, path ? path : "NULL");
    
    return device;
}

/**
 * @brief 关闭模拟设备
 */
static void sim_close_device(void* device_handle) {
    if (!device_handle) {
        return;
    }
    
    SimDevice* device = (SimDevice*)device_handle;
    device->is_open = false;
    
    printf("[SimAdapter] Closed device: type=%d, addr=%u\n",
           device->type, device->address);
    
    mmgr_free(device);
}

/**
 * @brief 从模拟设备读取
 */
static ErrorCode sim_read(void* device_handle, uint32_t address, Value* value) {
    (void)address;  // 模拟设备暂不使用地址参数
    if (!device_handle || !value) {
        return ERR_INVALID_ARGUMENT;
    }
    
    SimDevice* device = (SimDevice*)device_handle;
    
    if (!device->is_open) {
        return ERR_DEVICE_NOT_OPEN;
    }
    
    // 根据设备类型模拟不同的读取行为
    switch (device->type) {
        case IO_DEV_GPIO_IN:
            // GPIO 输入：保持当前值（可以通过 write 模拟外部信号）
            *value = device->value;
            break;
            
        case IO_DEV_ADC:
            // ADC：模拟一个缓慢变化的正弦波（500 + 100*sin(t)）
            {
                static double phase = 0.0;
                phase += 0.1;
                device->value.real_val = 500.0 + 100.0 * sin(phase);
                *value = device->value;
            }
            break;
            
        case IO_DEV_ENCODER:
            // 编码器：模拟递增计数
            device->value.int_val++;
            *value = device->value;
            break;
            
        default:
            *value = device->value;
            break;
    }
    
    return OK;
}

/**
 * @brief 向模拟设备写入
 */
static ErrorCode sim_write(void* device_handle, uint32_t address, const Value* value) {
    if (!device_handle || !value) {
        return ERR_INVALID_ARGUMENT;
    }
    
    SimDevice* device = (SimDevice*)device_handle;
    
    if (!device->is_open) {
        return ERR_DEVICE_NOT_OPEN;
    }
    
    // 保存写入的值
    device->value = *value;
    
    // 根据设备类型打印不同信息
    switch (device->type) {
        case IO_DEV_GPIO_OUT:
            printf("[SimAdapter] GPIO OUT[%u] = %d\n", address, value->int_val);
            break;
            
        case IO_DEV_DAC:
            printf("[SimAdapter] DAC[%u] = %.2f\n", address, value->real_val);
            break;
            
        case IO_DEV_PWM:
            printf("[SimAdapter] PWM[%u] = %d (duty cycle)\n", address, value->int_val);
            break;
            
        default:
            printf("[SimAdapter] WRITE[%u] = %d\n", address, value->int_val);
            break;
    }
    
    return OK;
}

/**
 * @brief 批量读取（模拟 DMA 或批量传输）
 */
static ErrorCode sim_read_batch(void* device_handle, uint32_t start_addr,
                                 uint32_t count, Value* values) {
    if (!device_handle || !values || count == 0) {
        return ERR_INVALID_ARGUMENT;
    }
    
    // 模拟批量读取：逐个调用 read
    for (uint32_t i = 0; i < count; i++) {
        ErrorCode err = sim_read(device_handle, start_addr + i, &values[i]);
        if (err != OK) {
            return err;
        }
    }
    
    printf("[SimAdapter] Batch read: start=%u, count=%u\n", start_addr, count);
    
    return OK;
}

/**
 * @brief 批量写入
 */
static ErrorCode sim_write_batch(void* device_handle, uint32_t start_addr,
                                  uint32_t count, const Value* values) {
    if (!device_handle || !values || count == 0) {
        return ERR_INVALID_ARGUMENT;
    }
    
    // 模拟批量写入：逐个调用 write
    for (uint32_t i = 0; i < count; i++) {
        ErrorCode err = sim_write(device_handle, start_addr + i, &values[i]);
        if (err != OK) {
            return err;
        }
    }
    
    printf("[SimAdapter] Batch write: start=%u, count=%u\n", start_addr, count);
    
    return OK;
}

/**
 * @brief 设置设备参数（如 PWM 频率、ADC 采样率等）
 */
static ErrorCode sim_set_parameter(void* device_handle, const char* param_name,
                                    const Value* value) {
    if (!device_handle || !param_name || !value) {
        return ERR_INVALID_ARGUMENT;
    }
    
    SimDevice* device = (SimDevice*)device_handle;
    
    printf("[SimAdapter] Set parameter: device_type=%d, param=%s, value=%d\n",
           device->type, param_name, value->int_val);
    
    // 在真实硬件中，这里会配置硬件参数
    // 模拟器中仅打印信息
    
    return OK;
}

/**
 * @brief 获取设备参数
 */
static ErrorCode sim_get_parameter(void* device_handle, const char* param_name,
                                    Value* value) {
    if (!device_handle || !param_name || !value) {
        return ERR_INVALID_ARGUMENT;
    }
    
    // 模拟器中返回固定值
    value->int_val = 1000;  // 例如：1000 Hz 采样率
    value->type = TYPE_INT;
    
    printf("[SimAdapter] Get parameter: param=%s, value=%d\n",
           param_name, value->int_val);
    
    return OK;
}

// ============================================================================
// 创建模拟适配器
// ============================================================================

/**
 * @brief 创建并初始化模拟适配器
 */
IOHardwareAdapter* io_adapter_create_simulator(void) {
    IOHardwareAdapter* adapter = (IOHardwareAdapter*)mmgr_alloc(sizeof(IOHardwareAdapter));
    if (!adapter) {
        return NULL;
    }
    
    memset(adapter, 0, sizeof(IOHardwareAdapter));
    
    // 设置函数指针
    adapter->init = sim_init;
    adapter->cleanup = sim_cleanup;
    adapter->open_device = sim_open_device;
    adapter->close_device = sim_close_device;
    adapter->read = sim_read;
    adapter->write = sim_write;
    adapter->read_batch = sim_read_batch;
    adapter->write_batch = sim_write_batch;
    adapter->set_parameter = sim_set_parameter;
    adapter->get_parameter = sim_get_parameter;
    
    // 平台信息
    snprintf(adapter->platform_name, sizeof(adapter->platform_name), "Simulator");
    snprintf(adapter->version, sizeof(adapter->version), "1.0.0");
    adapter->supported_types = IO_DEV_GPIO_IN | IO_DEV_GPIO_OUT |
                               IO_DEV_ADC | IO_DEV_DAC |
                               IO_DEV_PWM | IO_DEV_ENCODER;
    
    printf("[SimAdapter] Created\n");
    
    return adapter;
}

/**
 * @brief 释放模拟适配器
 */
void io_adapter_free_simulator(IOHardwareAdapter* adapter) {
    if (!adapter) {
        return;
    }
    
    if (adapter->cleanup) {
        adapter->cleanup(adapter);
    }
    
    mmgr_free(adapter);
    
    printf("[SimAdapter] Freed\n");
}
