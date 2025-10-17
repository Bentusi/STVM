# STVM 默认行为更新

## 更新日期：2025年10月17日

## 变更概述

STVM 现在支持直接运行 `.st` 源文件和 `.stbc` 字节码文件，无需显式指定模式。

## 新特性

### 自动模式检测

- **`.st` 文件**：自动编译并运行
- **`.stbc` 文件**：直接运行字节码

### 使用示例

```bash
# 直接运行 ST 源文件（自动编译并执行）
./build/bin/stvm examples/hello.st

# 直接运行字节码文件
./build/bin/stvm examples/hello.stbc

# 带选项运行
./build/bin/stvm examples/hello.st -V        # 详细输出
./build/bin/stvm examples/hello.st -d        # 调试模式
./build/bin/stvm examples/hello.st -s        # 显示统计信息
./build/bin/stvm examples/hello.st --dump-ast   # 打印 AST
```

## 命令行模式

### 默认行为（新）

```bash
stvm <file.st>      # 编译并运行 ST 源文件
stvm <file.stbc>    # 运行字节码文件
```

### 显式模式

```bash
stvm -c <file.st>   # 仅编译（生成 .stbc）
stvm -r <file.stbc> # 仅运行字节码
stvm -i             # 交互式 REPL
```

## 完整命令行选项

```
用法: stvm [选项] [文件]

模式:
  <file.st>               编译并运行ST源文件 (默认)
  <file.stbc>             运行字节码文件 (默认)
  -c, --compile <file>    仅编译ST源文件为字节码
  -r, --run <file>        仅运行字节码文件
  -i, --repl              启动交互式REPL
  -h, --help              显示帮助信息
  -v, --version           显示版本信息

选项:
  -o, --output <file>     指定输出文件名
  -d, --debug             启用调试模式
  -V, --verbose           详细输出
  -O, --optimize          启用优化
  -s, --stats             显示统计信息
  -L <path>               添加库搜索路径
  --dump-ast              打印抽象语法树
  --dump-bytecode         打印字节码
```

## 实现细节

### 修改的文件

1. **`src/include/cli.h`**
   - 添加了 `MODE_COMPILE_AND_RUN` 枚举值
   - 添加了 `cli_compile_and_run()` 函数声明

2. **`src/core/main.c`**
   - 修改了 `cli_parse_args()` 以自动检测文件类型
   - 实现了 `cli_compile_and_run()` 函数
   - 更新了 `main()` 函数以处理新模式
   - 更新了 `cli_print_help()` 显示新的使用方式

### 工作流程

#### `.st` 文件执行流程

1. 解析源代码（Parsing）
2. 类型检查（Type Checking）
3. 代码生成（Code Generation）
4. 执行字节码（Execution）

#### `.stbc` 文件执行流程

1. 加载字节码
2. 执行字节码

## 向后兼容性

所有现有的命令行选项和模式仍然支持：

```bash
# 旧方式仍然有效
stvm -c examples/hello.st
stvm -r examples/hello.stbc

# 新方式更简洁
stvm examples/hello.st
stvm examples/hello.stbc
```

## 测试

使用提供的测试脚本验证功能：

```bash
./test_default_behavior.sh
```

## 优势

1. **更简单的用户体验**：用户无需记住 `-c` 和 `-r` 选项
2. **类似其他解释器**：行为类似 Python、Node.js 等
3. **保持灵活性**：仍支持显式模式用于特殊场景
4. **智能检测**：根据文件扩展名自动选择合适的行为

## 错误处理

- 不支持的文件类型会显示明确的错误消息
- 缺少文件扩展名会提示错误
- 所有错误情况都有清晰的提示信息
