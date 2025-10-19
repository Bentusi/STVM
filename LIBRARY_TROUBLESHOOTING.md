# STVM 库功能故障排除指南

## 概述

本指南帮助您解决 STVM 库功能相关的常见问题。

**重要提示**: STVM v1.0.0 已实现**自动库依赖跟踪**,大部分库加载问题已解决。如果您遇到问题,请首先确保使用最新版本。

## 常见问题

### 1. ❌ "External function not registered" 错误

**症状**:
```bash
$ stvm -r program.stbc
Runtime error: External function not registered: examples/mathlib.stbc.Power
```

**原因**:

字节码文件是用旧版本编译器生成的,不包含库依赖信息。

**解决方案**:

重新编译所有文件:

```bash
# 1. 重新编译库文件
stvm -c examples/mathlib.st --library

# 2. 重新编译程序文件
stvm -c examples/program.st

# 3. 运行 (现在会自动加载库)
stvm -r examples/program.stbc
```

**版本对比**:

| STVM 版本 | 动态链接 `-r` 模式 | 说明 |
|-----------|-------------------|------|
| v1.0.0+ | ✅ 支持 | 自动加载库依赖 |
| 早期版本 | ❌ 不支持 | 需要使用静态链接或编译并运行模式 |

### 2. ❌ "Failed to load library" 错误

生成包含所有库代码的独立可执行文件:

```bash
**症状**:
```bash
$ stvm -c program.st
Error: Failed to load library: examples/mathlib.stbc
Type error: Failed to load library 'examples/mathlib.stbc'
```

**原因**:

1. 库文件不存在
2. 库文件路径错误
3. 库文件已损坏或格式不兼容

**解决方案**:

```bash
# 检查库文件是否存在
ls -lh examples/mathlib.stbc

# 如果不存在,重新编译库
stvm -c examples/mathlib.st --library

# 如果文件格式不兼容,重新编译
stvm -c examples/mathlib.st --library  # 使用新编译器重新生成
```

### 3. ❌ 字节码版本不兼容

**症状**:
```bash
$ stvm -r old_file.stbc
Warning: Checksum mismatch in bytecode file
```

**原因**:

字节码文件格式在不同版本之间可能不兼容。

**解决方案**:

重新编译所有字节码文件:

```bash
# 找到所有 .stbc 文件
find . -name "*.stbc" -type f

# 删除旧的字节码文件
find . -name "*.stbc" -type f -delete

# 重新编译库
stvm -c examples/mathlib.st --library

# 重新编译程序
stvm -c examples/program.st
```

## 工作模式对比

STVM 支持三种主要工作模式:

### 模式 1: 编译并运行 (推荐用于开发)

```bash
stvm program.st
```

**特点**:
- ✅ 一步到位,自动编译和运行
- ✅ 自动加载库
- ✅ 适合快速迭代开发
- ❌ 每次都需要编译

### 模式 2: 动态链接 (推荐用于测试)

```bash
# 编译
stvm -c program.st

# 运行
stvm -r program.stbc
```

**特点**:
- ✅ 编译一次,多次运行
- ✅ 自动加载库依赖 (v1.0.0+)
- ✅ 库可以独立更新
- ✅ 文件小,节省空间
- ⚠️ 需要库文件存在

### 模式 3: 静态链接 (推荐用于部署)

```bash
# 编译
stvm -c program.st --static

# 运行
stvm -r program.stbc
```

**特点**:
- ✅ 单文件部署
- ✅ 无外部依赖
- ✅ 适合生产环境
- ❌ 文件较大
- ❌ 库更新需要重新编译

## 验证安装

测试 STVM 是否正常工作:

```bash
# 1. 编译库
stvm -c examples/mathlib.st --library

# 2. 测试编译并运行模式
stvm examples/test_mathlib_demo.st

# 3. 测试动态链接模式
stvm -c examples/test_mathlib_demo.st
stvm -r examples/test_mathlib_demo.stbc

# 4. 测试静态链接模式
stvm -c examples/test_mathlib_demo.st --static -o /tmp/static_test.stbc
stvm -r /tmp/static_test.stbc
```

**预期输出** (所有三种模式):
```
Power(2, 10) = 1024
Factorial(5) = 120
GCD(48, 18) = 6
Fibonacci(10) = 55
Lerp(0.0, 100.0, 0.75) = 75.000000
```

## 调试技巧

### 1. 使用详细模式

```bash
stvm -r program.stbc -v
```

输出示例:
```
加载字节码文件: program.stbc
字节码加载完成
入口点: 0
全局变量数: 7
库依赖数: 1              # 关键信息!
加载库依赖...
  加载库: examples/mathlib.stbc
库依赖加载完成
开始执行...
```

### 2. 使用调试器

```bash
stvm -r program.stbc -d
```

调试器命令:
- `b 10` - 在指令 10 处设置断点
- `c` - 继续执行
- `s` - 单步执行
- `p` - 打印栈
- `i` - 查看当前指令

### 3. 打印字节码

```bash
stvm -r program.stbc --dump-bytecode
```

查看:
- 常量池
- 函数表
- 指令序列
- 库依赖信息

## 文件格式说明

### STBC 文件结构 (v1.0.0)

```
+-------------------+
| Header            |  包含版本、入口点、库依赖数量等
+-------------------+
| Constant Pool     |  字符串、整数、浮点数常量
+-------------------+
| Function Table    |  函数信息(名称、地址、参数)
+-------------------+
| Instructions      |  字节码指令序列
+-------------------+
| Library Deps      |  库依赖列表 (v1.0.0 新增)
|   - Lib Path 1    |
|   - Lib Path 2    |
|   - ...           |
+-------------------+
```

### 库依赖信息格式

```
[uint32_t] library_dep_count    # 库数量
[uint32_t] len1                 # 第一个库路径长度
[char[]] path1                  # 第一个库路径字符串
[uint32_t] len2                 # 第二个库路径长度
[char[]] path2                  # 第二个库路径字符串
...
```

## 获取帮助

### 命令行帮助

```bash
stvm --help
```

### 查看版本

```bash
stvm --version
```

### 相关文档

- [库依赖自动跟踪说明](LIBRARY_DEPENDENCY_TRACKING.md) - 详细技术说明
- [静态链接实现报告](STATIC_LINKING_REPORT.md) - 静态链接功能
- [如何编译库文件](HOW_TO_COMPILE_LIBRARY.md) - 库编译指南
- [库功能快速参考](LIBRARY_QUICK_REF.md) - 命令速查

---

**STVM v1.0.0** - 2025年10月


```bash
# 编译库
build/bin/stvm -c library.st

# 使用库
build/bin/stvm main.st
```

### 2. 检查库是否存在

```bash
ls -lh examples/mathlib.stbc
# 应该显示文件(约4.6KB)
```

### 3. 验证库搜索路径

默认搜索路径:
- `.` (当前目录)
- `examples/`

添加自定义路径:
```bash
build/bin/stvm main.st -L /path/to/libs
```

### 4. 使用详细模式调试

```bash
build/bin/stvm main.st -v
```

查看编译和运行过程的详细信息。

## 常见问题

### Q: 库文件存在但仍然报错

**A**: 可能原因:
1. 编译器版本过旧 → 重新编译
2. 库文件已损坏 → 重新编译库
3. 路径不匹配 → 检查IMPORT语句中的路径

### Q: 如何确认库被正确加载?

**A**: 使用 `-v` 选项,应该看到:
```
[libmgr] Loaded library: mathlib (52 functions)
```

### Q: 静态链接和动态链接有什么区别?

**A**: 
- **动态链接**: 运行时需要库文件,文件小
- **静态链接** (`--static`): 库代码内嵌,文件大,无运行时依赖

## 总结

问题已解决,关键步骤:
1. ✅ 重新编译项目 (`make clean && make`)
2. ✅ 确认库文件存在
3. ✅ 使用最新的编译器运行

现在应该可以正常使用库功能了!
