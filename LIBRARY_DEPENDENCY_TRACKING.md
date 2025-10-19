# 库依赖自动跟踪功能说明

## 概述

STVM v1.0.0 现在支持**自动库依赖跟踪**,在编译时将库依赖信息保存到字节码文件中,运行时自动加载所需的库。这解决了之前 `-r` 模式无法使用动态链接的问题。

## 问题背景

### 之前的问题

在实现此功能之前,STVM 的库加载有以下限制:

```bash
# ✅ 工作 - 编译并运行模式
stvm program.st              # 有 LibraryManager,能加载库

# ✅ 工作 - 静态链接
stvm -c program.st --static  # 库代码嵌入到字节码中
stvm -r program.stbc         # 不需要库

# ❌ 失败 - 动态链接 + 运行模式
stvm -c program.st           # 编译,生成 CALL_EXT 指令
stvm -r program.stbc         # 失败! cli_run() 没有 LibraryManager
```

**根本原因**:
- `cli_run()` 函数没有创建 `LibraryManager`
- 字节码文件不包含库依赖信息
- VM 不知道需要加载哪些库

## 解决方案

### 1. 字节码格式扩展

在字节码模块和文件头中添加库依赖信息:

```c
// bytecode.h - BytecodeModule 结构
typedef struct {
    // ... 原有字段 ...
    
    // 库依赖信息(新增)
    char** library_deps;         // 依赖的库文件路径数组
    uint32_t library_dep_count;  // 依赖库的数量
} BytecodeModule;

// bytecode_io.h - STBC 文件头
typedef struct STBCHeader {
    // ... 原有字段 ...
    uint32_t library_dep_count;  // 库依赖数量(新增)
    // ...
} STBCHeader;
```

### 2. 编译时记录依赖

在 `cli_compile()` 中,非静态链接时记录所有库依赖:

```c
// main.c - cli_compile()
if (!options->static_link) {
    // 记录库依赖信息
    uint32_t lib_count = libmgr_get_library_count(libmgr);
    for (uint32_t i = 0; i < lib_count; i++) {
        const char* lib_path = libmgr_get_library_path(libmgr, lib_name);
        bytecode_add_library_dependency(module, lib_path);
    }
}
```

### 3. 保存和加载依赖

**保存** (`bytecode_io.c`):
```c
// 写入文件头(包含 library_dep_count)
fwrite(&header, sizeof(STBCHeader), 1, fp);

// 写入常量池、函数表、指令...

// 写入库依赖信息
for (uint32_t i = 0; i < module->library_dep_count; i++) {
    uint32_t len = strlen(module->library_deps[i]);
    fwrite(&len, sizeof(uint32_t), 1, fp);  // 长度
    fwrite(module->library_deps[i], 1, len, fp);  // 路径
}
```

**加载** (`bytecode_io.c`):
```c
// 读取文件头
fread(&header, sizeof(STBCHeader), 1, fp);

// 读取常量池、函数表、指令...

// 读取库依赖信息
module->library_dep_count = header.library_dep_count;
for (uint32_t i = 0; i < module->library_dep_count; i++) {
    uint32_t len;
    fread(&len, sizeof(uint32_t), 1, fp);
    module->library_deps[i] = (char*)mmgr_alloc(len + 1);
    fread(module->library_deps[i], 1, len, fp);
    module->library_deps[i][len] = '\0';
}
```

### 4. 运行时加载库

在 `cli_run()` 中,自动加载字节码中记录的库依赖:

```c
// main.c - cli_run()
LibraryManager* libmgr = NULL;
if (module->library_dep_count > 0) {
    // 创建库管理器
    libmgr = libmgr_create(NULL);
    
    // 加载所有依赖的库
    for (uint32_t i = 0; i < module->library_dep_count; i++) {
        const char* lib_path = module->library_deps[i];
        libmgr_load_library(libmgr, lib_path);
    }
}

// 创建 VM 并设置库管理器
VM* vm = vm_create(module);
if (libmgr) {
    vm_set_library_manager(vm, libmgr);
}
```

## 使用方式

### 模式 1: 编译并运行(默认)

```bash
# 直接运行源文件
stvm examples/test_mathlib_demo.st
```

**特点**:
- ✅ 自动编译、类型检查、执行
- ✅ 自动加载库
- ✅ 适合开发调试

### 模式 2: 动态链接

```bash
# 步骤 1: 编译(记录库依赖)
stvm -c examples/test_mathlib_demo.st

# 步骤 2: 运行(自动加载库)
stvm -r examples/test_mathlib_demo.stbc
```

**特点**:
- ✅ 字节码文件小 (738 字节)
- ✅ 自动加载依赖库
- ✅ 库可以共享和更新
- ✅ 适合有多个程序共享库的场景

**文件结构**:
```
examples/
  ├── mathlib.stbc           (4.6 KB - 库文件)
  └── test_mathlib_demo.stbc (738 B - 包含库依赖信息)
```

### 模式 3: 静态链接

```bash
# 编译为独立可执行文件
stvm -c examples/test_mathlib_demo.st --static

# 运行(不需要库文件)
stvm -r examples/test_mathlib_demo.stbc
```

**特点**:
- ✅ 字节码文件大 (6.2 KB)
- ✅ 独立部署,无外部依赖
- ✅ 适合生产环境

## 文件大小对比

```
动态链接版本:  738 字节 (包含库依赖元数据)
静态链接版本: 6.2 KB   (包含完整库代码)
库文件:       4.6 KB
```

## 技术细节

### 字节码格式变更

**STBC 文件结构** (v1.0):
```
+-------------------+
| Header            |  包含 library_dep_count
+-------------------+
| Constant Pool     |
+-------------------+
| Function Table    |
+-------------------+
| Instructions      |
+-------------------+
| Library Deps      |  新增部分
|   - Count         |
|   - Lib Path 1    |
|   - Lib Path 2    |
|   - ...           |
+-------------------+
```

### 向后兼容性

**注意**: 此更改修改了 STBC 文件格式,旧版本的字节码文件需要重新编译。

原因:
- 文件头 `STBCHeader` 添加了新字段 `library_dep_count`
- 文件末尾添加了库依赖列表

建议:
```bash
# 重新编译所有库文件
stvm -c examples/mathlib.st --library

# 重新编译所有程序文件
stvm -c examples/test_mathlib_demo.st
```

## 实现的函数

### bytecode.c

```c
// 添加库依赖到字节码模块
ErrorCode bytecode_add_library_dependency(BytecodeModule* module, 
                                          const char* library_path);
```

### main.c

**修改的函数**:
- `cli_compile()`: 在非静态链接模式下记录库依赖
- `cli_run()`: 自动加载库依赖并创建 LibraryManager

## 调试输出

使用 `-v` 选项查看详细信息:

```bash
$ stvm -r examples/test_mathlib_demo.stbc -v

加载字节码文件: examples/test_mathlib_demo.stbc
字节码加载完成
入口点: 0
全局变量数: 7
库依赖数: 1              # 显示依赖的库数量
加载库依赖...
  加载库: examples/mathlib.stbc  # 自动加载
库依赖加载完成
开始执行...
Power(2, 10) = 1024
Factorial(5) = 120
GCD(48, 18) = 6
Fibonacci(10) = 55
Lerp(0.0, 100.0, 0.75) = 75.000000
执行完成
```

## 测试结果

所有三种模式均通过测试:

```bash
=== 测试 1: 编译并运行模式 ===
✅ 通过 - Power(2, 10) = 1024

=== 测试 2: 动态链接模式 ===
✅ 通过 - 自动加载 examples/mathlib.stbc

=== 测试 3: 静态链接模式 ===
✅ 通过 - 无需外部库
```

## 优势总结

### 动态链接的优势

1. **自动化**: 不需要手动指定库列表
2. **透明性**: 用户无感,自动加载依赖
3. **灵活性**: 库可以独立更新
4. **效率**: 多个程序共享同一个库文件

### 静态链接的优势

1. **独立性**: 无外部依赖,单文件部署
2. **可靠性**: 不受库版本变化影响
3. **性能**: 无运行时库加载开销

## 使用建议

| 场景 | 推荐模式 | 原因 |
|------|---------|------|
| 开发调试 | 编译并运行 | 快速迭代,自动编译 |
| 测试部署 | 动态链接 | 灵活,可更新库 |
| 生产环境 | 静态链接 | 独立,稳定,可靠 |
| 共享库场景 | 动态链接 | 节省空间,统一管理 |
| 嵌入式系统 | 静态链接 | 减少依赖,简化部署 |

## 相关文档

- [静态链接实现报告](STATIC_LINKING_REPORT.md)
- [如何编译库文件](HOW_TO_COMPILE_LIBRARY.md)
- [库功能快速参考](LIBRARY_QUICK_REF.md)

---

**STVM v1.0.0** - 2025年10月
