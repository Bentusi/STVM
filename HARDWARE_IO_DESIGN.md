# STVM 硬件 I/O 映射系统设计方案

## 需求分析

### 核心目标

1. **高效性**：最小化 I/O 访问开销，支持实时控制
2. **隔离性**：硬件抽象层隔离，便于跨平台移植
3. **安全性**：权限控制、边界检查、原子操作
4. **灵活性**：支持多种硬件类型（GPIO、ADC、DAC、PWM、SPI、I2C等）

### 工业控制场景

```st
PROGRAM MotorControl
VAR_EXTERNAL
    %IX0.0 : BOOL;      (* 启动按钮 - GPIO输入 *)
    %IX0.1 : BOOL;      (* 停止按钮 *)
    %QX0.0 : BOOL;      (* 电机使能 - GPIO输出 *)
    %IW0 : INT;         (* 速度传感器 - ADC输入 *)
    %QW0 : INT;         (* 速度设定 - DAC输出 *)
END_VAR

IF %IX0.0 AND NOT %IX0.1 THEN
    %QX0.0 := TRUE;
    %QW0 := 3000;       (* 设置速度 *)
END_IF
END_PROGRAM
```

## 架构设计

### 1. 分层架构

```
┌─────────────────────────────────────────┐
│   ST Program (User Code)                │
│   VAR_EXTERNAL %IX0.0, %QX0.0          │
└─────────────────┬───────────────────────┘
                  │
┌─────────────────▼───────────────────────┐
│   STVM I/O Manager                      │
│   - I/O 变量映射表                      │
│   - 访问权限控制                        │
│   - 刷新周期管理                        │
└─────────────────┬───────────────────────┘
                  │
┌─────────────────▼───────────────────────┐
│   Hardware Abstraction Layer (HAL)      │
│   - 平台抽象接口                        │
│   - 驱动适配器                          │
└─────────────────┬───────────────────────┘
                  │
┌─────────────────▼───────────────────────┐
│   Hardware Driver                       │
│   - Linux sysfs / /dev/*                │
│   - 裸机寄存器操作                      │
│   - 外部芯片驱动                        │
└─────────────────────────────────────────┘
```

### 2. IEC 61131-3 地址标准

#### 标准地址格式

```
%<location><size><address>.<bit>

location:
  I - Input (输入)
  Q - Output (输出)
  M - Memory (内部存储)

size:
  X - Bit (1位)
  B - Byte (8位)
  W - Word (16位)
  D - DWord (32位)
  L - LWord (64位)

示例：
  %IX0.0   - 输入字节0的第0位
  %QX1.7   - 输出字节1的第7位
  %IW0     - 输入字0 (16位)
  %QD0     - 输出双字0 (32位)
  %MW10    - 内部存储字10
```

## 核心数据结构

### 1. I/O 类型枚举

```c
/**
 * @brief I/O 设备类型
 */
typedef enum {
    IO_TYPE_DIGITAL_IN,     // 数字输入（GPIO）
    IO_TYPE_DIGITAL_OUT,    // 数字输出（GPIO）
    IO_TYPE_ANALOG_IN,      // 模拟输入（ADC）
    IO_TYPE_ANALOG_OUT,     // 模拟输出（DAC）
    IO_TYPE_PWM,            // PWM输出
    IO_TYPE_ENCODER,        // 编码器输入
    IO_TYPE_COUNTER,        // 计数器
    IO_TYPE_TIMER,          // 定时器
    IO_TYPE_MODBUS,         // Modbus寄存器
    IO_TYPE_CANOPEN,        // CANopen对象
    IO_TYPE_ETHERCAT,       // EtherCAT PDO
    IO_TYPE_CUSTOM          // 自定义类型
} IODeviceType;

/**
 * @brief I/O 访问模式
 */
typedef enum {
    IO_ACCESS_READ_ONLY,    // 只读（输入）
    IO_ACCESS_WRITE_ONLY,   // 只写（输出）
    IO_ACCESS_READ_WRITE    // 读写（内部存储）
} IOAccessMode;

/**
 * @brief I/O 地址位置
 */
typedef enum {
    IO_LOC_INPUT = 'I',     // 输入区域
    IO_LOC_OUTPUT = 'Q',    // 输出区域
    IO_LOC_MEMORY = 'M'     // 内存区域
} IOLocation;

/**
 * @brief I/O 数据大小
 */
typedef enum {
    IO_SIZE_BIT = 'X',      // 1位
    IO_SIZE_BYTE = 'B',     // 8位
    IO_SIZE_WORD = 'W',     // 16位
    IO_SIZE_DWORD = 'D',    // 32位
    IO_SIZE_LWORD = 'L'     // 64位
} IODataSize;
```

### 2. I/O 地址描述符

```c
/**
 * @brief IEC 61131-3 I/O 地址
 */
typedef struct IOAddress {
    IOLocation location;    // 位置 (I/Q/M)
    IODataSize size;        // 大小 (X/B/W/D/L)
    uint32_t byte_offset;   // 字节偏移
    uint8_t bit_offset;     // 位偏移 (仅对X有效)
} IOAddress;

/**
 * @brief I/O 点配置
 */
typedef struct IOPointConfig {
    IOAddress address;          // IEC地址 (%IX0.0)
    IODeviceType device_type;   // 设备类型
    IOAccessMode access_mode;   // 访问模式
    
    // 硬件映射
    char* hardware_path;        // 硬件路径 ("/sys/class/gpio/gpio17")
    uint32_t hardware_address;  // 硬件地址（寄存器地址、引脚号）
    
    // 数据转换
    double scale;               // 缩放系数（ADC/DAC）
    double offset;              // 偏移量
    bool invert;                // 反向逻辑
    
    // 过滤器（用于ADC）
    bool enable_filter;
    uint32_t filter_samples;    // 滤波采样数
    
    // 刷新策略
    uint32_t refresh_interval_us; // 刷新间隔（微秒）
    bool force_write;           // 强制写入（每次都写硬件）
    
    // 权限
    bool read_protected;        // 读保护
    bool write_protected;       // 写保护
} IOPointConfig;
```

### 3. I/O 点运行时状态

```c
/**
 * @brief I/O 点运行时数据
 */
typedef struct IOPoint {
    IOPointConfig config;       // 配置
    
    // 当前值
    Value current_value;        // STVM Value类型
    Value shadow_value;         // 影子值（用于检测变化）
    
    // 硬件接口
    void* hal_handle;           // HAL句柄（平台相关）
    
    // 统计信息
    uint64_t read_count;        // 读取次数
    uint64_t write_count;       // 写入次数
    uint64_t error_count;       // 错误次数
    uint64_t last_update_us;    // 最后更新时间（微秒）
    
    // 状态标志
    bool initialized;           // 已初始化
    bool fault;                 // 故障状态
    char error_msg[128];        // 错误消息
} IOPoint;
```

### 4. I/O 管理器

```c
/**
 * @brief I/O 管理器 - 核心
 */
typedef struct IOManager {
    // I/O 点数组
    IOPoint** io_points;        // I/O点指针数组
    uint32_t point_count;       // 点数量
    uint32_t point_capacity;    // 容量
    
    // 快速查找表
    struct {
        IOPoint** inputs;       // 输入区 %I
        IOPoint** outputs;      // 输出区 %Q
        IOPoint** memory;       // 内存区 %M
        uint32_t input_count;
        uint32_t output_count;
        uint32_t memory_count;
    } lookup;
    
    // HAL 适配器
    struct IOHardwareAdapter* hal_adapter;
    
    // 刷新任务
    bool auto_refresh;          // 自动刷新
    uint32_t refresh_cycle_us;  // 刷新周期（微秒）
    pthread_t refresh_thread;   // 刷新线程
    bool thread_running;
    
    // 统计
    struct {
        uint64_t total_reads;
        uint64_t total_writes;
        uint64_t total_errors;
        uint64_t refresh_cycles;
    } stats;
    
    // 日志回调
    void (*log_callback)(const char* msg);
} IOManager;
```

### 5. HAL 适配器接口

```c
/**
 * @brief 硬件抽象层适配器
 */
typedef struct IOHardwareAdapter {
    const char* platform_name;  // 平台名称 ("Linux", "STM32", "ESP32")
    
    // 初始化/清理
    ErrorCode (*init)(struct IOHardwareAdapter* adapter);
    void (*cleanup)(struct IOHardwareAdapter* adapter);
    
    // 数字I/O
    ErrorCode (*digital_read)(void* handle, bool* value);
    ErrorCode (*digital_write)(void* handle, bool value);
    
    // 模拟I/O
    ErrorCode (*analog_read)(void* handle, int32_t* value);
    ErrorCode (*analog_write)(void* handle, int32_t value);
    
    // PWM
    ErrorCode (*pwm_set)(void* handle, uint32_t frequency, uint32_t duty_cycle);
    
    // 通用读写（用于特殊设备）
    ErrorCode (*generic_read)(void* handle, void* buffer, size_t size);
    ErrorCode (*generic_write)(void* handle, const void* buffer, size_t size);
    
    // 设备打开/关闭
    void* (*open_device)(IODeviceType type, const char* path, uint32_t addr);
    void (*close_device)(void* handle);
    
    // 平台相关数据
    void* platform_data;
} IOHardwareAdapter;
```

## API 设计

### 1. 管理器生命周期

```c
/**
 * @brief 创建 I/O 管理器
 */
IOManager* io_manager_create(IOHardwareAdapter* hal_adapter);

/**
 * @brief 释放 I/O 管理器
 */
void io_manager_free(IOManager* mgr);

/**
 * @brief 启动自动刷新
 */
ErrorCode io_manager_start_refresh(IOManager* mgr, uint32_t cycle_us);

/**
 * @brief 停止自动刷新
 */
void io_manager_stop_refresh(IOManager* mgr);
```

### 2. I/O 点管理

```c
/**
 * @brief 添加 I/O 点
 */
ErrorCode io_manager_add_point(IOManager* mgr, const IOPointConfig* config);

/**
 * @brief 移除 I/O 点
 */
ErrorCode io_manager_remove_point(IOManager* mgr, const IOAddress* addr);

/**
 * @brief 查找 I/O 点
 */
IOPoint* io_manager_find_point(IOManager* mgr, const IOAddress* addr);
```

### 3. I/O 访问

```c
/**
 * @brief 读取 I/O 点
 * @param mgr I/O管理器
 * @param addr I/O地址
 * @param value 输出值
 * @return 错误码
 */
ErrorCode io_manager_read(IOManager* mgr, const IOAddress* addr, Value* value);

/**
 * @brief 写入 I/O 点
 * @param mgr I/O管理器
 * @param addr I/O地址
 * @param value 输入值
 * @return 错误码
 */
ErrorCode io_manager_write(IOManager* mgr, const IOAddress* addr, const Value* value);

/**
 * @brief 刷新所有输入
 */
ErrorCode io_manager_refresh_inputs(IOManager* mgr);

/**
 * @brief 刷新所有输出
 */
ErrorCode io_manager_refresh_outputs(IOManager* mgr);
```

### 4. 地址解析

```c
/**
 * @brief 解析 IEC 地址字符串
 * @param addr_str 地址字符串 ("%IX0.0", "%QW5")
 * @param addr 输出地址结构
 * @return 成功返回OK
 * 
 * 示例：
 *   "%IX0.0"  -> {I, X, 0, 0}
 *   "%QW5"    -> {Q, W, 5, 0}
 *   "%MD100"  -> {M, D, 100, 0}
 */
ErrorCode io_address_parse(const char* addr_str, IOAddress* addr);

/**
 * @brief 格式化 I/O 地址为字符串
 */
void io_address_format(const IOAddress* addr, char* buffer, size_t size);
```

## 与 VM 集成

### 1. VM 扩展

```c
/**
 * @brief 扩展 VM 结构
 */
typedef struct VM {
    // ... 原有字段 ...
    
    // I/O 管理器
    IOManager* io_manager;
    
    // I/O 刷新模式
    enum {
        IO_REFRESH_MANUAL,      // 手动刷新
        IO_REFRESH_AUTO,        // 自动刷新
        IO_REFRESH_ON_ACCESS    // 按需刷新
    } io_refresh_mode;
} VM;
```

### 2. 新增指令

```c
/**
 * @brief I/O 相关字节码指令
 */
typedef enum {
    // ... 原有指令 ...
    
    OP_IO_READ,         // 读取I/O operand: I/O地址索引
    OP_IO_WRITE,        // 写入I/O operand: I/O地址索引
    OP_IO_REFRESH_IN,   // 刷新所有输入
    OP_IO_REFRESH_OUT,  // 刷新所有输出
} Opcode;
```

### 3. 编译器支持

```st
(* ST 代码 *)
VAR_EXTERNAL
    start_button AT %IX0.0 : BOOL;
    motor_enable AT %QX0.0 : BOOL;
    speed_sensor AT %IW0 : INT;
END_VAR

motor_enable := start_button;
```

编译为：

```
LOAD_IO %IX0.0      ; 读取 start_button
STORE_IO %QX0.0     ; 写入 motor_enable
```

## 硬件适配器实现示例

### 1. Linux sysfs GPIO

```c
/**
 * @brief Linux GPIO 适配器
 */
typedef struct LinuxGPIOHandle {
    int gpio_num;
    int value_fd;
    int direction_fd;
} LinuxGPIOHandle;

static void* linux_gpio_open(IODeviceType type, const char* path, uint32_t pin) {
    LinuxGPIOHandle* handle = malloc(sizeof(LinuxGPIOHandle));
    handle->gpio_num = pin;
    
    // Export GPIO
    FILE* export_file = fopen("/sys/class/gpio/export", "w");
    fprintf(export_file, "%d", pin);
    fclose(export_file);
    
    // 设置方向
    char dir_path[256];
    snprintf(dir_path, sizeof(dir_path), "/sys/class/gpio/gpio%d/direction", pin);
    FILE* dir_file = fopen(dir_path, "w");
    fprintf(dir_file, type == IO_TYPE_DIGITAL_IN ? "in" : "out");
    fclose(dir_file);
    
    // 打开value文件
    char value_path[256];
    snprintf(value_path, sizeof(value_path), "/sys/class/gpio/gpio%d/value", pin);
    handle->value_fd = open(value_path, O_RDWR);
    
    return handle;
}

static ErrorCode linux_gpio_read(void* handle, bool* value) {
    LinuxGPIOHandle* gpio = (LinuxGPIOHandle*)handle;
    char buf[2];
    
    lseek(gpio->value_fd, 0, SEEK_SET);
    read(gpio->value_fd, buf, 1);
    *value = (buf[0] == '1');
    
    return OK;
}

static ErrorCode linux_gpio_write(void* handle, bool value) {
    LinuxGPIOHandle* gpio = (LinuxGPIOHandle*)handle;
    
    lseek(gpio->value_fd, 0, SEEK_SET);
    write(gpio->value_fd, value ? "1" : "0", 1);
    
    return OK;
}
```

### 2. STM32 HAL

```c
/**
 * @brief STM32 GPIO 适配器
 */
typedef struct STM32GPIOHandle {
    GPIO_TypeDef* port;
    uint16_t pin;
} STM32GPIOHandle;

static ErrorCode stm32_gpio_read(void* handle, bool* value) {
    STM32GPIOHandle* gpio = (STM32GPIOHandle*)handle;
    *value = HAL_GPIO_ReadPin(gpio->port, gpio->pin) == GPIO_PIN_SET;
    return OK;
}

static ErrorCode stm32_gpio_write(void* handle, bool value) {
    STM32GPIOHandle* gpio = (STM32GPIOHandle*)handle;
    HAL_GPIO_WritePin(gpio->port, gpio->pin, 
                      value ? GPIO_PIN_SET : GPIO_PIN_RESET);
    return OK;
}
```

## 配置文件格式

### JSON 配置示例

```json
{
  "io_config": {
    "platform": "linux",
    "auto_refresh": true,
    "refresh_cycle_us": 10000,
    "points": [
      {
        "address": "%IX0.0",
        "type": "digital_in",
        "hardware": {
          "path": "/sys/class/gpio/gpio17",
          "pin": 17
        },
        "properties": {
          "invert": false,
          "read_protected": false
        }
      },
      {
        "address": "%QX0.0",
        "type": "digital_out",
        "hardware": {
          "path": "/sys/class/gpio/gpio27",
          "pin": 27
        },
        "properties": {
          "force_write": false
        }
      },
      {
        "address": "%IW0",
        "type": "analog_in",
        "hardware": {
          "path": "/dev/spidev0.0",
          "channel": 0
        },
        "conversion": {
          "scale": 0.001,
          "offset": 0,
          "filter_samples": 10
        }
      },
      {
        "address": "%QW0",
        "type": "analog_out",
        "hardware": {
          "path": "/dev/i2c-1",
          "address": "0x60"
        },
        "conversion": {
          "scale": 1000.0,
          "offset": 0
        }
      }
    ]
  }
}
```

## 安全性设计

### 1. 权限控制

```c
/**
 * @brief I/O 访问权限检查
 */
typedef struct IOPermissions {
    bool allow_read;
    bool allow_write;
    uint32_t required_privilege_level;
} IOPermissions;

ErrorCode io_check_permission(IOPoint* point, bool is_write) {
    if (is_write) {
        if (point->config.write_protected) {
            return ERR_PERMISSION_DENIED;
        }
    } else {
        if (point->config.read_protected) {
            return ERR_PERMISSION_DENIED;
        }
    }
    return OK;
}
```

### 2. 边界检查

```c
/**
 * @brief 数值范围检查
 */
ErrorCode io_validate_value(IOPoint* point, const Value* value) {
    switch (point->config.device_type) {
        case IO_TYPE_ANALOG_OUT:
            if (value->type != TYPE_INT && value->type != TYPE_REAL) {
                return ERR_TYPE_MISMATCH;
            }
            // 检查DAC范围 (例如 0-4095 for 12-bit DAC)
            if (value->int_val < 0 || value->int_val > 4095) {
                return ERR_OUT_OF_RANGE;
            }
            break;
            
        case IO_TYPE_DIGITAL_OUT:
            if (value->type != TYPE_BOOL) {
                return ERR_TYPE_MISMATCH;
            }
            break;
            
        default:
            break;
    }
    return OK;
}
```

### 3. 原子操作

```c
/**
 * @brief 原子 I/O 操作（用于多线程环境）
 */
ErrorCode io_atomic_read_modify_write(IOManager* mgr, 
                                      const IOAddress* addr,
                                      Value (*modify_fn)(Value old_val)) {
    pthread_mutex_lock(&mgr->io_mutex);
    
    Value old_val;
    ErrorCode err = io_manager_read(mgr, addr, &old_val);
    if (err != OK) {
        pthread_mutex_unlock(&mgr->io_mutex);
        return err;
    }
    
    Value new_val = modify_fn(old_val);
    err = io_manager_write(mgr, addr, &new_val);
    
    pthread_mutex_unlock(&mgr->io_mutex);
    return err;
}
```

## 性能优化

### 1. 批量刷新

```c
/**
 * @brief 批量刷新（减少系统调用）
 */
ErrorCode io_manager_refresh_batch(IOManager* mgr, 
                                   IOAddress* addrs, 
                                   uint32_t count) {
    // 按硬件设备分组
    // 一次性读取所有GPIO状态寄存器
    // 一次性写入所有输出
    
    for (uint32_t i = 0; i < count; i++) {
        IOPoint* point = io_manager_find_point(mgr, &addrs[i]);
        if (point && point->config.access_mode == IO_ACCESS_READ_ONLY) {
            // 批量读取
        }
    }
    
    return OK;
}
```

### 2. DMA 支持

```c
/**
 * @brief DMA 传输配置（用于高速ADC/DAC）
 */
typedef struct IODMAConfig {
    void* buffer;
    uint32_t buffer_size;
    uint32_t transfer_rate;
    void (*completion_callback)(void* user_data);
} IODMAConfig;
```

### 3. 缓存策略

```c
/**
 * @brief I/O 缓存（减少硬件访问）
 */
typedef struct IOCache {
    Value cached_value;
    uint64_t cache_time_us;
    uint32_t cache_ttl_us;      // 缓存有效期
    bool valid;
} IOCache;

ErrorCode io_read_cached(IOPoint* point, Value* value) {
    uint64_t now = get_time_us();
    
    if (point->cache.valid && 
        (now - point->cache.cache_time_us) < point->cache.cache_ttl_us) {
        // 使用缓存
        *value = point->cache.cached_value;
        return OK;
    }
    
    // 缓存失效，读取硬件
    ErrorCode err = io_hardware_read(point, value);
    if (err == OK) {
        point->cache.cached_value = *value;
        point->cache.cache_time_us = now;
        point->cache.valid = true;
    }
    
    return err;
}
```

## 使用示例

### 1. 初始化 I/O 系统

```c
int main() {
    // 创建 Linux GPIO 适配器
    IOHardwareAdapter* hal = create_linux_gpio_adapter();
    
    // 创建 I/O 管理器
    IOManager* io_mgr = io_manager_create(hal);
    
    // 添加 I/O 点
    IOPointConfig config = {
        .address = {IO_LOC_INPUT, IO_SIZE_BIT, 0, 0},  // %IX0.0
        .device_type = IO_TYPE_DIGITAL_IN,
        .access_mode = IO_ACCESS_READ_ONLY,
        .hardware_path = "/sys/class/gpio/gpio17",
        .hardware_address = 17,
        .refresh_interval_us = 10000
    };
    io_manager_add_point(io_mgr, &config);
    
    // 启动自动刷新
    io_manager_start_refresh(io_mgr, 10000);  // 10ms 周期
    
    // 创建 VM 并绑定 I/O 管理器
    VM* vm = vm_create(module);
    vm->io_manager = io_mgr;
    
    // 运行 VM
    vm_run(vm);
    
    // 清理
    io_manager_stop_refresh(io_mgr);
    io_manager_free(io_mgr);
    vm_free(vm);
    
    return 0;
}
```

### 2. ST 代码示例

```st
PROGRAM TrafficLight
VAR_EXTERNAL
    (* 输入：传感器 *)
    car_sensor AT %IX0.0 : BOOL;      (* 车辆检测传感器 *)
    ped_button AT %IX0.1 : BOOL;      (* 行人按钮 *)
    
    (* 输出：交通灯 *)
    red_light AT %QX0.0 : BOOL;
    yellow_light AT %QX0.1 : BOOL;
    green_light AT %QX0.2 : BOOL;
    
    (* 模拟量 *)
    light_intensity AT %IW0 : INT;    (* 光照强度传感器 *)
END_VAR

VAR
    state : INT := 0;
    timer : INT := 0;
END_VAR

(* 简单的交通灯状态机 *)
CASE state OF
    0:  (* 红灯 *)
        red_light := TRUE;
        yellow_light := FALSE;
        green_light := FALSE;
        IF timer > 5000 THEN
            state := 1;
            timer := 0;
        END_IF;
        
    1:  (* 绿灯 *)
        red_light := FALSE;
        green_light := TRUE;
        IF car_sensor OR timer > 10000 THEN
            state := 2;
            timer := 0;
        END_IF;
        
    2:  (* 黄灯 *)
        green_light := FALSE;
        yellow_light := TRUE;
        IF timer > 2000 THEN
            state := 0;
            timer := 0;
        END_IF;
END_CASE

timer := timer + 1;
END_PROGRAM
```

## 扩展功能

### 1. Modbus 支持

```c
/**
 * @brief Modbus 寄存器映射
 */
typedef struct ModbusIOPoint {
    IOPoint base;
    uint8_t slave_id;
    uint16_t register_addr;
    enum {
        MODBUS_COIL,
        MODBUS_DISCRETE_INPUT,
        MODBUS_HOLDING_REGISTER,
        MODBUS_INPUT_REGISTER
    } register_type;
} ModbusIOPoint;
```

### 2. 诊断与监控

```c
/**
 * @brief I/O 诊断信息
 */
typedef struct IODiagnostics {
    uint64_t last_read_time_us;
    uint64_t last_write_time_us;
    uint32_t consecutive_errors;
    double average_read_latency_us;
    double average_write_latency_us;
} IODiagnostics;

void io_print_diagnostics(IOManager* mgr) {
    printf("=== I/O Diagnostics ===\n");
    printf("Total reads: %lu\n", mgr->stats.total_reads);
    printf("Total writes: %lu\n", mgr->stats.total_writes);
    printf("Total errors: %lu\n", mgr->stats.total_errors);
    printf("Refresh cycles: %lu\n", mgr->stats.refresh_cycles);
    
    for (uint32_t i = 0; i < mgr->point_count; i++) {
        IOPoint* point = mgr->io_points[i];
        char addr_str[32];
        io_address_format(&point->config.address, addr_str, sizeof(addr_str));
        printf("  %s: R=%lu W=%lu E=%lu\n", 
               addr_str, 
               point->read_count,
               point->write_count,
               point->error_count);
    }
}
```

### 3. 热插拔支持

```c
/**
 * @brief I/O 设备热插拔检测
 */
typedef struct IOHotplugDetector {
    void (*on_device_added)(IODeviceType type, const char* path);
    void (*on_device_removed)(IODeviceType type, const char* path);
} IOHotplugDetector;
```

## 总结

本设计方案提供了：

1. ✅ **高效性**：
   - 批量刷新减少系统调用
   - 缓存机制减少硬件访问
   - DMA 支持高速传输

2. ✅ **隔离性**：
   - HAL 抽象层隔离平台差异
   - 适配器模式支持多平台
   - 配置文件驱动，无需修改代码

3. ✅ **安全性**：
   - 权限控制
   - 边界检查
   - 原子操作支持

4. ✅ **灵活性**：
   - 支持多种I/O类型
   - 符合IEC 61131-3标准
   - 扩展性强（Modbus、CANopen等）

下一步可以：
1. 实现核心 I/O 管理器
2. 实现 Linux 平台适配器
3. 添加编译器支持（VAR_EXTERNAL、AT语法）
4. 创建测试程序验证功能

---

**关键优势**：
- 工业标准兼容（IEC 61131-3）
- 生产级可靠性
- 跨平台可移植性
- 实时控制性能
