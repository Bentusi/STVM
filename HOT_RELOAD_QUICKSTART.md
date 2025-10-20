# STVM 热更新快速使用指南

## 快速开始

### 1. 编译 STVM（包含热更新支持）

```bash
make clean && make
```

### 2. 运行热更新测试

```bash
# 编译测试程序 V1 和 V2
./build/bin/stvm -c examples/hotreload_v1.st -o examples/hotreload_v1.stbc
./build/bin/stvm -c examples/hotreload_v2.st -o examples/hotreload_v2.stbc

# 运行热更新测试
./build/bin/test_hotreload examples/hotreload_v1.stbc examples/hotreload_v2.stbc
```

### 3. 查看测试结果

测试程序会：
1. 加载并运行 V1 代码（Calculate 函数: x * 2）
2. 创建热更新管理器
3. 暂存 V2 模块
4. 进行安全检查
5. 应用热更新
6. 运行更新后的代码（Calculate 函数: x * 3 + 10，新增 Square 函数）

## 在代码中使用热更新 API

### 基础示例

```c
#include "hotreload.h"

// 1. 创建热更新管理器
HotReloadManager* mgr = hotreload_create();

// 2. 暂存新模块
if (!hotreload_stage_module_from_file(mgr, "new_version.stbc")) {
    fprintf(stderr, "Failed to stage module\n");
    return;
}

// 3. 检查是否安全更新
if (hotreload_is_safe(mgr, vm)) {
    // 4. 应用热更新
    if (hotreload_apply_staged(mgr, vm)) {
        printf("Hot reload successful!\n");
        
        // 5. 获取统计信息
        HotReloadStats stats = hotreload_get_stats(mgr);
        printf("Total reloads: %d\n", stats.reload_count);
        printf("Functions updated: %d\n", stats.functions_updated);
        printf("Functions added: %d\n", stats.functions_added);
    }
} else {
    fprintf(stderr, "Not safe to reload at this point\n");
}

// 6. 清理
hotreload_free(mgr);
```

### 一步更新（简化接口）

```c
#include "hotreload.h"

// 直接更新，包含所有步骤
HotReloadResult result = hotreload_update(vm, "new_version.stbc");

switch (result) {
    case HOT_RELOAD_SUCCESS:
        printf("Update successful\n");
        break;
    case HOT_RELOAD_UNSAFE:
        printf("Cannot update at this point (call stack not empty)\n");
        break;
    case HOT_RELOAD_INCOMPATIBLE:
        printf("Module incompatible\n");
        break;
    case HOT_RELOAD_ERROR:
        printf("Update failed\n");
        break;
}
```

## 安全性说明

### 何时可以安全热更新？

✅ **安全时机：**
- 在主函数（main）层级
- 调用栈为空或只有 main 函数
- 没有函数正在执行

❌ **不安全时机：**
- 在深层函数调用中
- 有多个函数在调用栈上
- 正在执行循环或复杂逻辑

### 检查安全性

```c
// 方式 1：手动检查
if (hotreload_is_safe(mgr, vm)) {
    // 安全，可以更新
    hotreload_apply_staged(mgr, vm);
}

// 方式 2：强制更新（不推荐）
hotreload_apply_staged(mgr, vm);  // 会自动检查，不安全则失败
```

## 兼容性要求

### ✅ 支持的更新

- **修改函数体**：可以修改函数内部实现
- **新增函数**：可以添加新函数
- **删除未使用的函数**：可以删除没有被引用的函数
- **修改常量值**：可以修改字符串、数字等常量

### ❌ 不支持的更新

- **修改函数签名**：参数数量、类型改变
- **修改全局变量**：新增、删除、改变类型
- **修改类型定义**：结构体、枚举等
- **修改库依赖**：改变引用的库

## 差异分析

热更新前会分析新旧模块的差异：

```c
// 打印差异报告
hotreload_dump_diff(mgr);
```

输出示例：
```
========== Hot Reload Diff ==========
Functions Updated: 1
Functions Added:   2
Functions Deleted: 0

Function Mappings:
  Calculate: [UPDATED]
  Square: [NEW]
  Cube: [NEW]
=====================================
```

## 统计信息

```c
HotReloadStats stats = hotreload_get_stats(mgr);

printf("热更新次数: %d\n", stats.reload_count);
printf("更新函数数: %d\n", stats.functions_updated);
printf("新增函数数: %d\n", stats.functions_added);
printf("删除函数数: %d\n", stats.functions_deleted);
printf("上次耗时: %lld ms\n", stats.last_reload_time_ms);
```

## 错误处理

```c
if (!hotreload_stage_module_from_file(mgr, "new.stbc")) {
    fprintf(stderr, "Failed to load module\n");
    return;
}

if (!hotreload_check_compatibility(mgr)) {
    fprintf(stderr, "Module incompatible:\n");
    // 打印详细差异
    hotreload_dump_diff(mgr);
    
    // 取消暂存
    hotreload_cancel_staged(mgr);
    return;
}

if (!hotreload_is_safe(mgr, vm)) {
    fprintf(stderr, "Not safe to reload, call stack depth: %d\n", 
            vm->call_stack_top);
    return;
}

if (!hotreload_apply_staged(mgr, vm)) {
    fprintf(stderr, "Failed to apply hot reload\n");
    return;
}
```

## 实用技巧

### 1. 监听文件变化自动热更新

```c
// 伪代码示例
while (vm_is_running) {
    if (file_modified("app.stbc")) {
        HotReloadResult result = hotreload_update(vm, "app.stbc");
        if (result == HOT_RELOAD_SUCCESS) {
            printf("Auto-reloaded: app.stbc\n");
        }
    }
    sleep(1);
}
```

### 2. 热更新日志

```c
void log_hot_reload(HotReloadManager* mgr) {
    HotReloadStats stats = hotreload_get_stats(mgr);
    
    FILE* log = fopen("hotreload.log", "a");
    fprintf(log, "[%s] Reload #%d: +%d -%d ~%d (%lld ms)\n",
            get_timestamp(),
            stats.reload_count,
            stats.functions_added,
            stats.functions_deleted,
            stats.functions_updated,
            stats.last_reload_time_ms);
    fclose(log);
}
```

### 3. 渐进式更新

```c
// 先检查兼容性
if (hotreload_check_compatibility(mgr)) {
    // 等待安全点
    while (!hotreload_is_safe(mgr, vm)) {
        usleep(100);  // 等待 100μs
    }
    
    // 应用更新
    hotreload_apply_staged(mgr, vm);
}
```

## 性能考虑

- **暂存模块**: ~100μs（小型模块）
- **安全检查**: ~50μs
- **应用更新**: <1μs（指针替换）
- **总耗时**: 通常 <1ms

热更新对运行时性能影响极小。

## 调试技巧

### 启用详细日志

```c
// 在 hotreload.c 中添加
#define HOT_RELOAD_VERBOSE
```

### 检查模块差异

```bash
# 对比两个字节码文件
./build/bin/stvm -d v1.stbc > v1.dump
./build/bin/stvm -d v2.stbc > v2.dump
diff v1.dump v2.dump
```

### 使用调试器

```bash
# GDB 调试
gdb --args ./build/bin/test_hotreload v1.stbc v2.stbc

# 设置断点
(gdb) break hotreload_apply_staged
(gdb) run
(gdb) print mgr->stats
```

## 常见问题

### Q: 为什么热更新失败？

A: 检查以下几点：
1. 调用栈是否为空（在 main 层级）
2. 新模块是否与旧模块兼容
3. 函数签名是否改变
4. 是否有全局变量变化

### Q: 如何处理全局变量？

A: 当前版本保留现有全局变量的值。如果需要修改全局变量，需要在应用更新后手动迁移。

### Q: 能否回滚更新？

A: 暂不支持自动回滚。建议保留旧版本模块，需要时重新加载：
```c
hotreload_update(vm, "old_version.stbc");
```

### Q: 多线程环境如何使用？

A: 当前版本不支持多线程。在多线程环境中需要：
1. 暂停所有工作线程
2. 应用热更新
3. 恢复线程执行

## 更多信息

- 详细实现报告：`HOT_RELOAD_IMPLEMENTATION.md`
- 可行性评估：`HOT_RELOAD_ASSESSMENT.md`
- 测试代码：`tests/test_hotreload.c`
- 示例程序：`examples/hotreload_v*.st`

---

**提示**：热更新是一个强大但复杂的功能，建议在充分测试后再用于生产环境。
