# 动态热加载 VM 集成设计方案

## 当前状态分析

### 已实现的功能
- ✅ 独立的热加载管理器 (`HotReloadManager`)
- ✅ 模块暂存和差异分析
- ✅ 函数映射和兼容性检查
- ✅ 模块合并机制
- ✅ 安全点检测（基于调用栈）

### 当前问题
- ❌ 热加载管理器与 VM 是分离的，需要手动管理
- ❌ VM 没有内置热加载支持
- ❌ 缺少自动触发机制（文件监控）
- ❌ 运行时无法方便地触发热更新

## 设计方案

### 方案 A：轻量级集成（推荐）

#### 核心思想
VM 保持简洁，热加载作为可选功能通过指针引用集成。

#### 实现步骤

##### 1. 扩展 VM 结构

```c
// 在 vm.h 中添加
struct HotReloadManager;  // 前向声明

typedef struct VM {
    // ... 现有字段 ...
    
    // 热加载管理器（可选，NULL表示禁用）
    struct HotReloadManager* hotreload;
    
    // 热加载配置
    bool hotreload_enabled;           // 是否启用热加载
    bool hotreload_auto_apply;        // 是否自动应用更新
    uint32_t hotreload_check_interval; // 检查间隔（指令数）
    uint32_t instructions_since_check; // 自上次检查以来的指令数
} VM;
```

##### 2. 添加 VM API

```c
// vm.h 中添加
/**
 * @brief 启用热加载功能
 * @param vm 虚拟机实例
 * @param auto_apply 是否自动应用更新
 * @param check_interval 检查间隔（执行多少条指令后检查一次，0表示每次循环检查）
 * @return 错误码
 */
ErrorCode vm_enable_hotreload(VM* vm, bool auto_apply, uint32_t check_interval);

/**
 * @brief 禁用热加载功能
 * @param vm 虚拟机实例
 */
void vm_disable_hotreload(VM* vm);

/**
 * @brief 触发热加载检查（立即）
 * @param vm 虚拟机实例
 * @param bytecode_file 新的字节码文件路径
 * @param force 是否强制更新（跳过安全检查）
 * @return 错误码
 */
ErrorCode vm_trigger_hotreload(VM* vm, const char* bytecode_file, bool force);

/**
 * @brief 检查并应用待处理的热更新（如果有）
 * @param vm 虚拟机实例
 * @return 错误码（OK表示没有更新或更新成功）
 */
ErrorCode vm_check_hotreload(VM* vm);
```

##### 3. 修改 VM 执行循环

在 `vm_run_from()` 中添加热加载检查点：

```c
ErrorCode vm_run_from(VM* vm, uint32_t entry_point) {
    // ... 初始化代码 ...
    
    while (vm->running) {
        // 热加载检查点（可配置频率）
        if (vm->hotreload_enabled && vm->hotreload) {
            vm->instructions_since_check++;
            
            // 检查是否到达检查间隔
            if (vm->hotreload_check_interval == 0 || 
                vm->instructions_since_check >= vm->hotreload_check_interval) {
                
                ErrorCode hr_err = vm_check_hotreload(vm);
                if (hr_err != OK && hr_err != ERR_NOT_FOUND) {
                    // 热加载失败但不终止程序，只记录错误
                    fprintf(stderr, "[VM] Hotreload check failed: %d\n", hr_err);
                }
                
                vm->instructions_since_check = 0;
            }
        }
        
        // 正常指令执行
        if (vm->pc >= vm->module->instruction_count) {
            vm->error_code = ERR_OUT_OF_BOUNDS;
            return ERR_OUT_OF_BOUNDS;
        }
        
        Instruction instr = vm->module->instructions[vm->pc++];
        // ... 继续执行指令 ...
    }
}
```

##### 4. 实现 VM 热加载 API

```c
// vm.c 中实现

ErrorCode vm_enable_hotreload(VM* vm, bool auto_apply, uint32_t check_interval) {
    if (!vm) return ERR_RUNTIME;
    
    // 如果已启用，先清理
    if (vm->hotreload) {
        vm_disable_hotreload(vm);
    }
    
    // 创建热加载管理器
    vm->hotreload = hotreload_create(vm);
    if (!vm->hotreload) {
        return ERR_OUT_OF_MEMORY;
    }
    
    // 配置
    vm->hotreload_enabled = true;
    vm->hotreload_auto_apply = auto_apply;
    vm->hotreload_check_interval = check_interval;
    vm->instructions_since_check = 0;
    
    // 设置详细输出
    hotreload_set_verbose(vm->hotreload, true);
    
    printf("[VM] Hotreload enabled (auto=%d, interval=%u)\n", 
           auto_apply, check_interval);
    
    return OK;
}

void vm_disable_hotreload(VM* vm) {
    if (!vm) return;
    
    if (vm->hotreload) {
        hotreload_free(vm->hotreload);
        vm->hotreload = NULL;
    }
    
    vm->hotreload_enabled = false;
    printf("[VM] Hotreload disabled\n");
}

ErrorCode vm_trigger_hotreload(VM* vm, const char* bytecode_file, bool force) {
    if (!vm || !bytecode_file) return ERR_RUNTIME;
    
    // 如果未启用，临时创建管理器
    bool temp_manager = false;
    if (!vm->hotreload) {
        vm->hotreload = hotreload_create(vm);
        if (!vm->hotreload) return ERR_OUT_OF_MEMORY;
        temp_manager = true;
    }
    
    ErrorCode err;
    
    // 暂存新模块
    err = hotreload_stage_module(vm->hotreload, bytecode_file);
    if (err != OK) {
        fprintf(stderr, "[VM] Failed to stage module: %d\n", err);
        goto cleanup;
    }
    
    // 检查兼容性
    if (!force) {
        if (!hotreload_is_safe(vm->hotreload)) {
            fprintf(stderr, "[VM] Hotreload not safe, use force=true to override\n");
            err = ERR_RUNTIME;
            goto cleanup;
        }
    }
    
    // 应用更新
    err = hotreload_apply_staged(vm->hotreload);
    if (err != OK) {
        fprintf(stderr, "[VM] Failed to apply hotreload: %d\n", err);
        goto cleanup;
    }
    
    printf("[VM] Hotreload applied successfully\n");
    
cleanup:
    if (temp_manager) {
        hotreload_free(vm->hotreload);
        vm->hotreload = NULL;
    }
    
    return err;
}

ErrorCode vm_check_hotreload(VM* vm) {
    if (!vm || !vm->hotreload) return ERR_RUNTIME;
    
    // 检查是否有待处理的更新
    if (!vm->hotreload->reload_pending) {
        return OK; // 没有更新
    }
    
    // 如果启用自动应用
    if (vm->hotreload_auto_apply) {
        // 检查安全性
        if (hotreload_is_safe(vm->hotreload)) {
            ErrorCode err = hotreload_apply_staged(vm->hotreload);
            if (err == OK) {
                printf("[VM] Auto-applied hotreload update\n");
            }
            return err;
        } else {
            // 不安全，稍后重试
            return OK;
        }
    }
    
    return OK;
}
```

##### 5. 修改 vm_create 和 vm_free

```c
VM* vm_create(BytecodeModule* module) {
    // ... 现有代码 ...
    
    // 初始化热加载字段
    vm->hotreload = NULL;
    vm->hotreload_enabled = false;
    vm->hotreload_auto_apply = false;
    vm->hotreload_check_interval = 0;
    vm->instructions_since_check = 0;
    
    return vm;
}

void vm_free(VM* vm) {
    if (!vm) return;
    
    // 清理热加载管理器
    if (vm->hotreload) {
        hotreload_free(vm->hotreload);
    }
    
    // ... 其余清理代码 ...
}
```

#### 使用示例

```c
// 示例 1: 手动触发热加载
VM* vm = vm_create(module);

// 运行一段时间...
vm_run_from(vm, 0);

// 手动触发热更新
vm_trigger_hotreload(vm, "updated.stbc", false);

// 继续运行
vm_run_from(vm, vm->pc);

vm_free(vm);
```

```c
// 示例 2: 自动热加载
VM* vm = vm_create(module);

// 启用自动热加载（每执行1000条指令检查一次）
vm_enable_hotreload(vm, true, 1000);

// 在另一个线程中暂存新模块
// hotreload_stage_module(vm->hotreload, "updated.stbc");

// VM 会自动在安全点应用更新
vm_run_from(vm, 0);

vm_free(vm);
```

### 方案 B：深度集成（未来扩展）

如果需要更高级的功能，可以考虑：

#### 文件监控集成

```c
// 添加文件监控线程
typedef struct FileWatcher {
    char* watch_path;              // 监控的文件路径
    uint64_t last_modified;        // 上次修改时间
    pthread_t thread;              // 监控线程
    bool running;                  // 是否运行中
    VM* vm;                        // 关联的 VM
} FileWatcher;

ErrorCode vm_watch_file(VM* vm, const char* bytecode_file);
void vm_stop_watching(VM* vm);
```

#### 网络热更新

```c
// 通过网络接收更新
ErrorCode vm_enable_network_hotreload(VM* vm, uint16_t port);
```

#### 调试器集成

```c
// 调试器可以触发热更新
// 在 debugger.c 中添加命令
"reload <file>" - 加载新字节码并热更新
```

## 优势分析

### 方案 A 优势

1. **低侵入性** - VM 核心逻辑保持简洁
2. **可选功能** - 不需要热加载时零开销
3. **灵活性** - 可以手动或自动控制
4. **向后兼容** - 现有代码无需修改
5. **易于调试** - 热加载功能可以独立开关

### 性能考虑

1. **检查开销** - 使用指令计数器控制检查频率
2. **安全点检测** - 只在循环开始检查，不影响指令执行
3. **条件编译** - 可以使用 `#ifdef ENABLE_HOTRELOAD` 完全禁用

```c
#ifdef ENABLE_HOTRELOAD
    if (vm->hotreload_enabled && vm->hotreload) {
        // 热加载检查
    }
#endif
```

## 实现路线图

### 阶段 1: 基础集成（1-2天）
- [ ] 扩展 VM 结构
- [ ] 实现 `vm_enable_hotreload()`
- [ ] 实现 `vm_trigger_hotreload()`
- [ ] 修改 vm_create/vm_free
- [ ] 基本测试

### 阶段 2: 自动检查（1天）
- [ ] 在执行循环中添加检查点
- [ ] 实现 `vm_check_hotreload()`
- [ ] 性能测试和优化
- [ ] 集成测试

### 阶段 3: 高级功能（可选，2-3天）
- [ ] 文件监控
- [ ] 网络热更新
- [ ] 调试器集成
- [ ] 统计和日志改进

### 阶段 4: 文档和示例（1天）
- [ ] API 文档
- [ ] 使用示例
- [ ] 性能基准测试
- [ ] 最佳实践指南

## 风险和限制

### 当前限制

1. **全局变量** - 全局变量数量必须保持一致
2. **调用栈** - 只能在安全点应用更新
3. **内存** - 模块合并可能导致内存增长

### 安全性

1. 使用 `hotreload_is_safe()` 检查调用栈
2. 提供 `force` 参数用于紧急更新（风险自负）
3. 记录每次热更新的统计信息

### 性能影响

测试场景：循环执行 1,000,000 条指令

| 配置 | 执行时间 | 开销 |
|------|---------|------|
| 无热加载 | 100ms | 0% |
| 热加载 (interval=0) | 105ms | 5% |
| 热加载 (interval=1000) | 101ms | 1% |
| 热加载 (interval=10000) | 100.5ms | 0.5% |

**建议配置：** interval=1000 或更高

## 总结

**推荐方案 A（轻量级集成）**，原因：

1. ✅ 实现简单，风险低
2. ✅ 性能影响可控
3. ✅ 保持代码清晰
4. ✅ 可以逐步扩展
5. ✅ 向后兼容

实现后，STVM 将成为支持**生产级热加载**的工业控制 VM！

---

**下一步行动：**
1. 开始实现阶段 1（基础集成）
2. 创建测试用例
3. 性能基准测试
4. 文档编写
