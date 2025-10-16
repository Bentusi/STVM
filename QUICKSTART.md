# STVM 快速开始指南

欢迎使用 STVM - ST 语言编译器和虚拟机！

## 安装和构建

### 前置要求

- GCC 11+ 或兼容的 C 编译器
- GNU Make
- Flex（词法分析器生成器）
- Bison（语法分析器生成器）

### 构建 STVM

```bash
cd /path/to/lexer
make stvm
```

构建完成后，可执行文件位于 `build/bin/stvm`。

## 第一个程序

### 1. 编写 ST 程序

创建文件 `hello.st`：

```st
PROGRAM HelloWorld
VAR
    message : STRING := 'Hello, STVM!';
    counter : INT := 42;
END_VAR

BEGIN
    counter := counter + 1;
END_PROGRAM
```

### 2. 编译程序

```bash
./build/bin/stvm -c hello.st
```

这将生成字节码文件 `hello.stbc`。

### 3. 运行程序

```bash
./build/bin/stvm -r hello.stbc
```

### 4. 调试程序（可选）

```bash
./build/bin/stvm -r hello.stbc -d
```

在调试器中：
```
(stdb) b 5          # 在地址 5 设置断点
(stdb) r            # 运行程序
(stdb) info locals  # 查看局部变量
(stdb) s            # 单步执行
(stdb) c            # 继续执行
(stdb) q            # 退出
```

## 常用命令

### 编译选项

```bash
# 基本编译
./build/bin/stvm -c program.st

# 指定输出文件
./build/bin/stvm -c program.st -o output.stbc

# 显示 AST
./build/bin/stvm -c program.st --dump-ast

# 显示字节码
./build/bin/stvm -c program.st --dump-bytecode

# 详细输出
./build/bin/stvm -c program.st -V

# 显示统计信息
./build/bin/stvm -c program.st -s
```

### 运行选项

```bash
# 基本运行
./build/bin/stvm -r program.stbc

# 调试模式
./build/bin/stvm -r program.stbc -d

# 显示字节码
./build/bin/stvm -r program.stbc --dump-bytecode

# 显示统计信息
./build/bin/stvm -r program.stbc -s
```

### REPL 模式

```bash
./build/bin/stvm -i
```

REPL 命令：
- `.help` - 显示帮助
- `.vars` - 显示所有变量
- `.dump` - 显示字节码
- `.clear` - 清除所有定义
- `.quit` - 退出

### 帮助信息

```bash
./build/bin/stvm --help
./build/bin/stvm --version
```

## 示例程序

STVM 提供了多个示例程序：

```bash
# Hello World
./build/bin/stvm -c examples/hello.st
./build/bin/stvm -r examples/hello.stbc

# 控制流（IF, WHILE, FOR）
./build/bin/stvm -c examples/control_flow.st
./build/bin/stvm -r examples/control_flow.stbc

# 函数定义和调用
./build/bin/stvm -c examples/functions.st
./build/bin/stvm -r examples/functions.stbc

# 数组操作
./build/bin/stvm -c examples/arrays.st
./build/bin/stvm -r examples/arrays.stbc

# CASE 语句
./build/bin/stvm -c examples/case.st
./build/bin/stvm -r examples/case.stbc
```

查看 `examples/README.md` 了解更多详情。

## 调试器快速参考

启动调试器：
```bash
./build/bin/stvm -r program.stbc -d
```

### 常用命令

| 命令 | 简写 | 说明 |
|------|------|------|
| help | h | 显示帮助 |
| run | r | 开始执行 |
| continue | c | 继续执行 |
| step | s | 单步执行（进入函数） |
| next | n | 单步执行（跳过函数） |
| finish | f | 执行到函数返回 |
| break <addr> | b | 设置断点 |
| delete <addr> | d | 删除断点 |
| info breakpoints | | 列出断点 |
| info frame | | 显示当前栈帧 |
| info locals | | 显示局部变量 |
| info globals | | 显示全局变量 |
| backtrace | bt | 显示调用栈 |
| stack | | 显示操作数栈 |
| disasm [addr] [n] | | 反汇编 |
| quit | q | 退出 |

## ST 语言快速参考

### 程序结构

```st
PROGRAM ProgramName
VAR
    (* 变量声明 *)
END_VAR

BEGIN
    (* 程序代码 *)
END_PROGRAM
```

### 数据类型

- `BOOL` - 布尔型：TRUE, FALSE
- `INT` - 整数型：-2147483648 到 2147483647
- `REAL` - 实数型：浮点数
- `STRING` - 字符串型：'text'
- `ARRAY[lower..upper] OF TYPE` - 数组

### 控制流

```st
(* IF 语句 *)
IF condition THEN
    (* ... *)
ELSIF another_condition THEN
    (* ... *)
ELSE
    (* ... *)
END_IF;

(* WHILE 循环 *)
WHILE condition DO
    (* ... *)
END_WHILE;

(* FOR 循环 *)
FOR var := start TO end DO
    (* ... *)
END_FOR;

(* CASE 语句 *)
CASE expression OF
    value1: (* ... *)
    value2: (* ... *)
ELSE
    (* ... *)
END_CASE;
```

### 函数

```st
FUNCTION FunctionName : ReturnType
VAR_INPUT
    param1 : Type1;
    param2 : Type2;
END_VAR
VAR
    (* 局部变量 *)
END_VAR
BEGIN
    (* 函数体 *)
    FunctionName := result;
END_FUNCTION
```

## 故障排除

### 编译错误

如果编译失败：
1. 检查语法错误
2. 使用 `--dump-ast` 查看解析结果
3. 使用 `-V` 获取详细输出

### 运行时错误

如果运行失败：
1. 使用 `-d` 启动调试器
2. 使用断点和单步执行
3. 使用 `info locals` 和 `info globals` 检查变量

### 构建问题

如果构建失败：
```bash
make clean    # 清理构建产物
make stvm     # 重新构建
```

## 获取帮助

- 查看 `README.md` - 项目概述和完整文档
- 查看 `examples/README.md` - 示例程序说明
- 查看 `PHASE6_COMPLETION_REPORT.md` - 第六阶段完成报告
- 使用 `./build/bin/stvm --help` - 命令行帮助
- 使用调试器的 `help` 命令 - 调试命令帮助

## 下一步

- 尝试修改示例程序
- 编写自己的 ST 程序
- 探索调试器功能
- 阅读源代码了解实现细节

祝你使用愉快！
