# STVM 硬件 I/O 系统实现指南

## 已完成的工作

### 1. 设计文档
- ✅ `HARDWARE_IO_DESIGN.md` - 完整的架构设计方案
- ✅ API 设计
- ✅ 数据结构定义
- ✅ 安全性与性能考虑

### 2. 头文件
- ✅ `src/include/iomgr.h` - I/O 管理器 API 定义（~400行）

### 3. 示例程序
- ✅ `examples/io_blink.st` - LED 闪烁示例
- ✅ `examples/io_temperature.st` - 温度监控示例
- ✅ `examples/io_config.json` - I/O 配置文件示例

### 4. 测试程序
- ✅ `tests/test_io_manager.c` - 完整的测试套件

## 待实现的组件

### Phase 1: 核心 I/O 管理器（优先级：高）

需要实现的文件：`src/core/iomgr.c`

#### 1.1 基础功能

```c
// 管理器生命周期
IOManager* io_manager_create(IOHardwareAdapter* hal_adapter) {
    IOManager* mgr = mmgr_alloc(sizeof(IOManager));
    // 初始化字段
    // 创建查找表
    // 初始化互斥锁
    return mgr;
}

void io_manager_free(IOManager* mgr) {
    // 停止刷新线程
    // 释放所有I/O点
    // 释放查找表
    // 销毁互斥锁
}
```

#### 1.2 I/O 点管理

```c
ErrorCode io_manager_add_point(IOManager* mgr, const IOPointConfig* config) {
    // 1. 检查是否已存在
    // 2. 分配 IOPoint 结构
    // 3. 调用 HAL 打开设备
    // 4. 初始化滤波器（如果启用）
    // 5. 添加到管理器
    // 6. 更新查找表
    return OK;
}

IOPoint* io_manager_find_point(IOManager* mgr, const IOAddress* addr) {
    // 根据地址类型选择查找表
    // 快速查找
    return point;
}
```

#### 1.3 I/O 访问

```c
ErrorCode io_manager_read(IOManager* mgr, const IOAddress* addr, Value* value) {
    // 1. 查找点
    // 2. 权限检查
    // 3. 调用 HAL 读取
    // 4. 应用滤波器（如果启用）
    // 5. 数据转换（scale/offset）
    // 6. 更新统计
    return OK;
}

ErrorCode io_manager_write(IOManager* mgr, const IOAddress* addr, const Value* value) {
    // 1. 查找点
    // 2. 权限检查
    // 3. 类型检查
    // 4. 数据转换
    // 5. 检测值变化（如果不是force_write）
    // 6. 调用 HAL 写入
    // 7. 更新统计
    return OK;
}
```

#### 1.4 地址解析

```c
ErrorCode io_address_parse(const char* addr_str, IOAddress* addr) {
    // 解析格式：%<location><size><offset>.<bit>
    // 例如：%IX0.0, %QW5, %MD100
    
    // 1. 检查 '%' 前缀
    // 2. 解析 location (I/Q/M)
    // 3. 解析 size (X/B/W/D/L)
    // 4. 解析 byte offset
    // 5. 解析 bit offset (可选)
    
    return OK;
}
```

### Phase 2: 模拟适配器（优先级：高，用于测试）

需要实现的文件：`src/core/io_adapter_sim.c`

```c
IOHardwareAdapter* io_create_sim_adapter(void) {
    IOHardwareAdapter* adapter = mmgr_alloc(sizeof(IOHardwareAdapter));
    adapter->platform_name = "Simulator";
    
    // 设置回调函数
    adapter->init = sim_init;
    adapter->cleanup = sim_cleanup;
    adapter->digital_read = sim_digital_read;
    adapter->digital_write = sim_digital_write;
    adapter->analog_read = sim_analog_read;
    adapter->analog_write = sim_analog_write;
    adapter->open_device = sim_open_device;
    adapter->close_device = sim_close_device;
    
    // 创建模拟数据
    SimulatorData* sim_data = mmgr_alloc(sizeof(SimulatorData));
    adapter->platform_data = sim_data;
    
    return adapter;
}

static ErrorCode sim_digital_read(void* handle, bool* value) {
    // 返回模拟值（可以添加随机变化）
    *value = rand() % 2;
    return OK;
}

static ErrorCode sim_analog_read(void* handle, int32_t* value) {
    // 返回模拟 ADC 值（例如：模拟温度传感器）
    *value = 2000 + (rand() % 100);  // 2000-2100
    return OK;
}
```

### Phase 3: Linux 适配器（优先级：中）

需要实现的文件：`src/core/io_adapter_linux.c`

```c
IOHardwareAdapter* io_create_linux_adapter(void) {
    // 类似模拟适配器，但使用真实硬件接口
}

static void* linux_gpio_open(IODeviceType type, const char* path, uint32_t pin) {
    // 1. Export GPIO
    // 2. 设置方向
    // 3. 打开 value 文件
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

static ErrorCode linux_adc_read(void* handle, int32_t* value) {
    // 使用 SPI 读取 MCP3008 ADC
    // 或使用 IIO 框架 /sys/bus/iio/devices/iio:device0/
    return OK;
}
```

### Phase 4: 自动刷新线程（优先级：中）

```c
ErrorCode io_manager_start_refresh(IOManager* mgr, uint32_t cycle_us) {
    mgr->refresh_cycle_us = cycle_us;
    mgr->thread_running = true;
    
    pthread_create(&mgr->refresh_thread, NULL, refresh_thread_func, mgr);
    
    return OK;
}

static void* refresh_thread_func(void* arg) {
    IOManager* mgr = (IOManager*)arg;
    
    while (mgr->thread_running) {
        // 刷新所有输入
        io_manager_refresh_inputs(mgr);
        
        // 刷新所有输出
        io_manager_refresh_outputs(mgr);
        
        mgr->stats.refresh_cycles++;
        
        // 休眠
        usleep(mgr->refresh_cycle_us);
    }
    
    return NULL;
}
```

### Phase 5: VM 集成（优先级：中）

#### 5.1 扩展 VM 结构

修改 `src/include/vm.h`:

```c
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

#### 5.2 添加 I/O 指令

修改 `src/include/bytecode.h`:

```c
typedef enum {
    // ... 原有指令 ...
    
    OP_IO_READ,         // 读取I/O operand: I/O地址索引
    OP_IO_WRITE,        // 写入I/O operand: I/O地址索引
    OP_IO_REFRESH_IN,   // 刷新所有输入
    OP_IO_REFRESH_OUT,  // 刷新所有输出
} Opcode;
```

#### 5.3 VM 指令处理

修改 `src/core/vm.c`:

```c
// 在 vm_run() 中添加
case OP_IO_READ: {
    uint32_t io_index = instr.operand;
    IOAddress* addr = &vm->module->io_addresses[io_index];
    Value value;
    
    ErrorCode err = io_manager_read(vm->io_manager, addr, &value);
    if (err != OK) {
        return err;
    }
    
    PUSH(value);
    break;
}

case OP_IO_WRITE: {
    uint32_t io_index = instr.operand;
    IOAddress* addr = &vm->module->io_addresses[io_index];
    Value value = POP();
    
    ErrorCode err = io_manager_write(vm->io_manager, addr, &value);
    if (err != OK) {
        return err;
    }
    break;
}
```

### Phase 6: 编译器支持（优先级：低）

#### 6.1 词法分析

修改 `src/core/st_lexer.l`:

```lex
/* IEC 地址 */
%[IQM][XBWDL][0-9]+(\.[0-7])? { 
    yylval.string = strdup(yytext); 
    return IO_ADDRESS; 
}

"VAR_EXTERNAL"  { return VAR_EXTERNAL; }
"AT"            { return AT; }
```

#### 6.2 语法分析

修改 `src/core/st_parser.y`:

```yacc
var_external_section: VAR_EXTERNAL var_external_list END_VAR
                    {
                        $$ = create_var_external_section($2);
                    }
                    ;

var_external_decl: IDENTIFIER AT IO_ADDRESS COLON data_type
                 {
                     $$ = create_var_external_decl($1, $3, $5);
                 }
                 ;
```

#### 6.3 代码生成

修改 `src/core/codegen.c`:

```c
static void codegen_io_variable(CodeGenerator* cg, ASTNode* node) {
    // 查找 I/O 地址
    IOAddress addr;
    io_address_parse(node->io_address, &addr);
    
    // 添加到模块的 I/O 地址表
    uint32_t index = add_io_address(cg->module, &addr);
    
    // 生成 IO_READ/IO_WRITE 指令
    if (node->is_read) {
        emit_instruction(cg, OP_IO_READ, index);
    } else {
        emit_instruction(cg, OP_IO_WRITE, index);
    }
}
```

### Phase 7: 配置文件加载（优先级：低）

```c
ErrorCode io_manager_load_config(IOManager* mgr, const char* config_file) {
    // 1. 解析 JSON 文件
    // 2. 遍历 points 数组
    // 3. 为每个点创建 IOPointConfig
    // 4. 调用 io_manager_add_point
    
    return OK;
}
```

可以使用 cJSON 库进行 JSON 解析。

## 实现优先级

### 立即实现（1-2天）

1. ✅ 设计文档（已完成）
2. ✅ 头文件定义（已完成）
3. ✅ 测试程序框架（已完成）
4. ⏸️ 核心 I/O 管理器实现
5. ⏸️ 模拟适配器实现

### 短期实现（3-5天）

6. ⏸️ 地址解析功能
7. ⏸️ 自动刷新线程
8. ⏸️ 统计与诊断功能

### 中期实现（1-2周）

9. ⏸️ Linux GPIO 适配器
10. ⏸️ Linux ADC/DAC 适配器
11. ⏸️ VM 集成
12. ⏸️ 完整测试验证

### 长期实现（2-4周）

13. ⏸️ 编译器 VAR_EXTERNAL 支持
14. ⏸️ 配置文件加载
15. ⏸️ Modbus/CANopen 支持
16. ⏸️ 文档和示例完善

## 构建系统集成

需要修改 `Makefile`:

```makefile
# I/O 管理器源文件
IO_SRCS = src/core/iomgr.c \
          src/core/io_adapter_sim.c \
          src/core/io_adapter_linux.c

IO_OBJS = $(patsubst src/core/%.c,$(OBJ_DIR)/%.o,$(IO_SRCS))

# 添加到核心对象
CORE_OBJS += $(IO_OBJS)

# I/O 管理器测试
test_io_manager: $(BIN_DIR)/test_io_manager

$(BIN_DIR)/test_io_manager: $(TESTS_DIR)/test_io_manager.c $(CORE_OBJS) $(PARSER_OBJ) $(LEXER_OBJ) | dirs
	@echo "Building test_io_manager..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) -lpthread

# 添加到测试目标
test: ... test_io_manager
	...
	@echo "=== Running I/O Manager Tests ==="
	@./$(BIN_DIR)/test_io_manager
```

## 使用流程

### 1. 编写 ST 程序

```st
PROGRAM MyControl
VAR_EXTERNAL
    start_button AT %IX0.0 : BOOL;
    motor_enable AT %QX0.0 : BOOL;
END_VAR

motor_enable := start_button;
END_PROGRAM
```

### 2. 创建 I/O 配置

```json
{
  "io_config": {
    "platform": "linux",
    "points": [
      {
        "address": "%IX0.0",
        "type": "digital_in",
        "hardware": {"path": "/sys/class/gpio/gpio17", "pin": 17}
      },
      {
        "address": "%QX0.0",
        "type": "digital_out",
        "hardware": {"path": "/sys/class/gpio/gpio27", "pin": 27}
      }
    ]
  }
}
```

### 3. 运行程序

```bash
# 编译 ST 程序
./build/bin/stvm -c mycontrol.st -o mycontrol.stbc

# 运行（加载 I/O 配置）
./build/bin/stvm -r mycontrol.stbc --io-config io_config.json
```

## 测试策略

### 单元测试
- ✅ 地址解析测试
- ✅ 管理器生命周期测试
- ✅ I/O 点管理测试
- ✅ 读写操作测试
- ✅ 自动刷新测试
- ✅ 统计功能测试

### 集成测试
- ⏸️ 模拟适配器完整流程
- ⏸️ Linux 真实硬件测试
- ⏸️ VM 集成测试

### 性能测试
- ⏸️ I/O 访问延迟测试
- ⏸️ 刷新周期精度测试
- ⏸️ 多线程并发测试

## 文档

需要创建的文档：

1. ✅ `HARDWARE_IO_DESIGN.md` - 架构设计（已完成）
2. ⏸️ `HARDWARE_IO_API.md` - API 参考手册
3. ⏸️ `HARDWARE_IO_TUTORIAL.md` - 使用教程
4. ⏸️ `HARDWARE_IO_EXAMPLES.md` - 示例集合
5. ⏸️ `HARDWARE_IO_PLATFORM_GUIDE.md` - 平台移植指南

## 下一步行动

### 立即开始

1. **实现核心 I/O 管理器**
   ```bash
   touch src/core/iomgr.c
   # 实现基础功能
   ```

2. **实现模拟适配器**
   ```bash
   touch src/core/io_adapter_sim.c
   # 用于测试
   ```

3. **编译测试**
   ```bash
   make test_io_manager
   ./build/bin/test_io_manager
   ```

### 验证里程碑

- [ ] 核心 API 实现并通过所有测试
- [ ] 模拟适配器运行正常
- [ ] Linux GPIO 适配器工作
- [ ] VM 集成完成
- [ ] 真实硬件测试通过

---

**当前状态**：设计阶段完成 ✅，准备进入实现阶段

**预计完成时间**：
- MVP (模拟适配器 + 核心功能): 2-3天
- 生产版本 (Linux 适配器 + VM 集成): 1-2周
- 完整版本 (编译器支持 + 文档): 3-4周
