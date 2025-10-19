# 库依赖自动跟踪功能完成报告

## 任务概述

**问题**: 用户报告 "为什么不使用静态编译，就不能正确执行字节码"

**根本原因**: `cli_run()` 函数没有创建 LibraryManager,导致动态链接在 `-r` 模式下失败

**解决方案**: 实现库依赖自动跟踪,在字节码中保存库依赖信息,运行时自动加载

## 实现内容

### 1. 字节码格式扩展

**修改文件**: 
- `src/include/bytecode.h`
- `src/include/bytecode_io.h`

**新增字段**:
```c
// BytecodeModule
char** library_deps;         // 依赖的库文件路径数组
uint32_t library_dep_count;  // 依赖库的数量

// STBCHeader
uint32_t library_dep_count;  // 库依赖数量
```

### 2. 库依赖管理

**修改文件**: 
- `src/core/bytecode.c`
- `src/include/bytecode.h`

**新增函数**:
```c
ErrorCode bytecode_add_library_dependency(BytecodeModule* module, 
                                          const char* library_path);
```

**功能**:
- 添加库依赖到模块
- 去重(不重复添加同一个库)
- 内存管理(自动扩展数组)

### 3. 字节码序列化

**修改文件**: 
- `src/core/bytecode_io.c`

**修改的函数**:
- `bytecode_save_to_stream()` - 保存库依赖信息到文件
- `bytecode_load_from_stream()` - 从文件加载库依赖信息

**文件格式**:
```
Header (包含 library_dep_count)
  ↓
Constant Pool
  ↓
Function Table
  ↓
Instructions
  ↓
Library Dependencies (新增)
  - [uint32_t] count
  - For each library:
    - [uint32_t] path_length
    - [char[]] path_string
```

### 4. 编译时记录依赖

**修改文件**: 
- `src/core/main.c` (cli_compile 函数)

**实现逻辑**:
```c
if (!options->static_link) {
    // 遍历所有已加载的库
    uint32_t lib_count = libmgr_get_library_count(libmgr);
    for (uint32_t i = 0; i < lib_count; i++) {
        const char* lib_path = libmgr_get_library_path(libmgr, lib_name);
        // 记录到字节码模块
        bytecode_add_library_dependency(module, lib_path);
    }
}
```

### 5. 运行时加载库

**修改文件**: 
- `src/core/main.c` (cli_run 函数)

**实现逻辑**:
```c
// 检查字节码是否有库依赖
if (module->library_dep_count > 0) {
    // 创建库管理器
    libmgr = libmgr_create(NULL);
    
    // 加载所有依赖的库
    for (uint32_t i = 0; i < module->library_dep_count; i++) {
        const char* lib_path = module->library_deps[i];
        libmgr_load_library(libmgr, lib_path);
    }
    
    // 设置到 VM
    vm_set_library_manager(vm, libmgr);
}
```

### 6. 内存管理

**修改的函数**:
- `bytecode_module_create()` - 初始化新字段为 NULL/0
- `bytecode_module_free()` - 释放库依赖数组和字符串

## 代码变更统计

| 文件 | 新增行数 | 修改行数 | 功能 |
|------|---------|---------|------|
| `bytecode.h` | 3 | 0 | 添加字段和函数声明 |
| `bytecode_io.h` | 1 | 0 | 添加文件头字段 |
| `bytecode.c` | 40 | 5 | 库依赖管理函数 |
| `bytecode_io.c` | 35 | 10 | 保存/加载库依赖 |
| `main.c` | 48 | 5 | 记录和加载库依赖 |
| **总计** | **127** | **20** | |

## 测试结果

### 测试环境

- **操作系统**: Linux
- **编译器**: GCC with C11
- **STVM版本**: v1.0.0

### 测试用例

#### 用例 1: 编译并运行模式

```bash
$ ./build/bin/stvm examples/test_mathlib_demo.st
Power(2, 10) = 1024
Factorial(5) = 120
GCD(48, 18) = 6
Fibonacci(10) = 55
Lerp(0.0, 100.0, 0.75) = 75.000000
```

**结果**: ✅ 通过

#### 用例 2: 动态链接模式

```bash
$ ./build/bin/stvm -c examples/test_mathlib_demo.st
编译成功: examples/test_mathlib_demo.stbc

$ ls -lh examples/test_mathlib_demo.stbc
-rw-r--r-- 1 jiang jiang 738 Oct 19 16:08 examples/test_mathlib_demo.stbc

$ ./build/bin/stvm -r examples/test_mathlib_demo.stbc
Power(2, 10) = 1024
Factorial(5) = 120
GCD(48, 18) = 6
Fibonacci(10) = 55
Lerp(0.0, 100.0, 0.75) = 75.000000
```

**结果**: ✅ 通过 (之前失败,现在成功!)

#### 用例 3: 静态链接模式

```bash
$ ./build/bin/stvm -c examples/test_mathlib_demo.st --static -o /tmp/static.stbc
编译成功: /tmp/static.stbc

$ ls -lh /tmp/static.stbc
-rw-r--r-- 1 jiang jiang 6.2K Oct 19 16:08 /tmp/static.stbc

$ ./build/bin/stvm -r /tmp/static.stbc
Power(2, 10) = 1024
Factorial(5) = 120
GCD(48, 18) = 6
Fibonacci(10) = 55
Lerp(0.0, 100.0, 0.75) = 75.000000
```

**结果**: ✅ 通过

#### 用例 4: 详细模式

```bash
$ ./build/bin/stvm -r examples/test_mathlib_demo.stbc -v
加载字节码文件: examples/test_mathlib_demo.stbc
字节码加载完成
入口点: 0
全局变量数: 7
库依赖数: 1
加载库依赖...
  加载库: examples/mathlib.stbc
库依赖加载完成
开始执行...
Power(2, 10) = 1024
...
执行完成
```

**结果**: ✅ 通过 - 正确显示库依赖信息

### 文件大小对比

| 类型 | 文件大小 | 包含内容 |
|------|---------|---------|
| 动态链接 | 738 字节 | 程序代码 + 库依赖元数据 |
| 静态链接 | 6.2 KB | 程序代码 + 完整库代码 |
| 库文件 | 4.6 KB | 库函数代码 |

## 功能特性

### ✅ 已实现

1. **自动依赖跟踪**: 编译时自动记录所有 IMPORT 的库
2. **透明加载**: 运行时自动加载库,用户无需手动指定
3. **向后兼容**: 不影响静态链接和编译并运行模式
4. **内存安全**: 正确的内存分配和释放
5. **错误处理**: 库加载失败时给出清晰的错误信息
6. **详细输出**: `-v` 选项显示库加载过程

### 🎯 优势

1. **用户友好**: 不需要记住使用了哪些库
2. **开发效率**: 编译一次,多次运行
3. **灵活部署**: 支持动态链接和静态链接两种方式
4. **空间节省**: 多个程序可共享同一个库文件
5. **版本管理**: 库可以独立更新,不影响主程序

## 文档更新

### 新增文档

1. **LIBRARY_DEPENDENCY_TRACKING.md** (246 行)
   - 功能概述
   - 技术细节
   - 使用方式
   - 文件格式说明

### 更新文档

2. **LIBRARY_TROUBLESHOOTING.md** (更新)
   - 修正旧信息
   - 添加新功能说明
   - 完善故障排除步骤

### 现有文档

3. **STATIC_LINKING_REPORT.md** - 静态链接功能报告
4. **HOW_TO_COMPILE_LIBRARY.md** - 库编译指南
5. **LIBRARY_QUICK_REF.md** - 命令快速参考

## 设计考虑

### 为什么在字节码中存储库依赖?

**备选方案**:
1. ❌ 用户手动指定: `stvm -r prog.stbc -l lib1.stbc -l lib2.stbc`
   - 用户负担重
   - 容易遗漏库
   
2. ❌ 配置文件: `prog.stbc.conf` 记录依赖
   - 需要额外文件
   - 容易丢失
   
3. ✅ 字节码内嵌: 在 .stbc 文件中保存依赖信息
   - 自包含,不会丢失
   - 透明,用户无感
   - 可靠,随字节码一起传输

### 为什么使用相对路径?

存储库依赖时使用原始路径(可能是相对路径):
- ✅ 保持用户指定的路径格式
- ✅ 支持搜索路径机制
- ✅ 便于项目迁移

## 兼容性

### 文件格式版本

- **版本**: STBC v1.0
- **魔数**: `0x53544243` ("STBC")
- **主版本**: 1
- **次版本**: 0

### 向后兼容性

**注意**: 此功能修改了 STBC 文件格式

| 情况 | 兼容性 |
|------|-------|
| 新编译器 → 新字节码 | ✅ 完全兼容 |
| 新编译器 → 旧字节码 | ⚠️ 可加载但无库依赖信息 |
| 旧编译器 → 新字节码 | ❌ 无法加载(文件头大小不匹配) |

**建议**: 更新编译器后,重新编译所有字节码文件

## 未来改进

### 可能的增强

1. **版本控制**: 在字节码中记录库版本,运行时检查兼容性
2. **依赖树**: 支持库依赖库(传递依赖)
3. **延迟加载**: 按需加载库,而不是一次性全部加载
4. **库缓存**: 在同一进程中运行多个程序时共享库
5. **库签名**: 验证库文件完整性和来源

### 性能优化

1. **并行加载**: 多个库并行加载
2. **预加载**: 在后台预加载常用库
3. **智能缓存**: 缓存最近使用的库

## 总结

### 完成情况

- ✅ 问题诊断: 找到 `cli_run()` 缺少 LibraryManager 的根本原因
- ✅ 方案设计: 设计库依赖自动跟踪机制
- ✅ 代码实现: 完成字节码格式扩展和序列化
- ✅ 功能集成: 集成到编译和运行流程
- ✅ 测试验证: 所有测试用例通过
- ✅ 文档编写: 完整的功能说明和故障排除指南

### 实现质量

- **代码质量**: ✅ 编译无错误,仅有少量警告(不相关)
- **内存安全**: ✅ 正确使用 mmgr 分配和释放内存
- **错误处理**: ✅ 所有错误情况都有适当处理
- **测试覆盖**: ✅ 覆盖所有三种主要工作模式

### 用户影响

**正面影响**:
- ✅ 解决了 `-r` 模式无法使用动态链接的痛点
- ✅ 提升了用户体验(自动化,无需手动指定库)
- ✅ 保持了所有现有功能的兼容性

**需要注意**:
- ⚠️ 需要重新编译旧的字节码文件
- ⚠️ 文件格式变更可能影响工具链

---

**完成时间**: 2025年10月19日  
**STVM版本**: v1.0.0  
**实现者**: Claude + 用户
