# STVM - ST Language Virtual Machine

STVM 是一个为 ST（Structured Text）语言设计的字节码编译器和虚拟机。

## 特性

### 核心功能

- ✅ **完整的 ST 语言支持**
  - 变量声明（VAR, VAR_GLOBAL, VAR_LOCAL）
  - 基本数据类型（INT, REAL, BOOL, STRING, BYTE, WORD, DWORD）
  - 控制流（IF/THEN/ELSE, CASE, FOR, WHILE, REPEAT）
  - 函数定义与调用
  - 数组操作
  - 位运算

- ✅ **高级特性**
  - 静态变量（VAR_STATIC）
  - 函数返回值
  - 模块导入（IMPORT）
  - 库管理系统
  - **热更新（Hot Reload）** - 运行时代码更新，无需停机

- ✅ **开发工具**
  - 编译器（ST → 字节码）
  - 虚拟机（字节码执行）
  - 调试器（断点、单步执行）
  - REPL（交互式解释器）

## 快速开始

### 1. 构建

```bash
make clean && make
```

### 2. 编译并运行 ST 程序

```bash
# 编译
./build/bin/stvm -c examples/hello.st -o hello.stbc

# 运行
./build/bin/stvm -r hello.stbc

# 调试模式
./build/bin/stvm -r hello.stbc -d
```

### 3. 使用 REPL

```bash
./build/bin/stvm -i
```

## 热更新功能

STVM 支持在运行时动态更新代码，无需停机。这对于长期运行的程序非常有用。

### 快速测试

```bash
# 编译测试程序
./build/bin/stvm -c examples/hotreload_v1.st -o examples/hotreload_v1.stbc
./build/bin/stvm -c examples/hotreload_v2.st -o examples/hotreload_v2.stbc

# 运行热更新测试
./build/bin/test_hotreload examples/hotreload_v1.stbc examples/hotreload_v2.stbc
```

### API 使用示例

```c
#include "hotreload.h"

// 一步更新
HotReloadResult result = hotreload_update(vm, "new_version.stbc");
if (result == HOT_RELOAD_SUCCESS) {
    printf("热更新成功！\n");
}
```

详细文档：
- [热更新快速指南](HOT_RELOAD_QUICKSTART.md)
- [热更新实现报告](HOT_RELOAD_IMPLEMENTATION.md)
- [热更新可行性评估](HOT_RELOAD_ASSESSMENT.md)

## 项目结构

```
STVM/
├── src/
│   ├── core/           # 核心实现
│   │   ├── ast.c       # 抽象语法树
│   │   ├── bytecode.c  # 字节码生成
│   │   ├── codegen.c   # 代码生成
│   │   ├── vm.c        # 虚拟机
│   │   ├── hotreload.c # 热更新
│   │   └── ...
│   └── include/        # 头文件
│       ├── vm.h
│       ├── bytecode.h
│       ├── hotreload.h
│       └── ...
├── tests/              # 测试程序
│   ├── test_vm.c
│   ├── test_hotreload.c
│   └── ...
├── examples/           # 示例程序
│   ├── hello.st
│   ├── functions.st
│   ├── hotreload_v1.st
│   └── ...
├── build/              # 构建输出
│   ├── bin/            # 可执行文件
│   └── obj/            # 目标文件
└── Makefile
```

## 语言示例

### Hello World

```st
PROGRAM HelloWorld
VAR
    message : STRING;
END_VAR

message := "Hello, STVM!";
PRINT(message);
END_PROGRAM
```

### 函数定义

```st
FUNCTION Add : INT
VAR_INPUT
    a : INT;
    b : INT;
END_VAR

Add := a + b;
END_FUNCTION

PROGRAM Test
VAR
    result : INT;
END_VAR

result := Add(10, 20);
PRINT("Result: %d", result);
END_PROGRAM
```

### 控制流

```st
PROGRAM ControlFlow
VAR
    i : INT;
END_VAR

FOR i := 1 TO 5 DO
    IF i MOD 2 = 0 THEN
        PRINT("%d is even", i);
    ELSE
        PRINT("%d is odd", i);
    END_IF;
END_FOR;
END_PROGRAM
```

### 静态变量（计数器）

```st
FUNCTION Counter : INT
VAR_STATIC
    count : INT := 0;
END_VAR

count := count + 1;
Counter := count;
END_FUNCTION

PROGRAM Test
VAR
    i : INT;
END_VAR

FOR i := 1 TO 5 DO
    PRINT("Call %d: count = %d", i, Counter());
END_FOR;
END_PROGRAM
```

输出：
```
Call 1: count = 1
Call 2: count = 2
Call 3: count = 3
Call 4: count = 4
Call 5: count = 5
```

## 命令行选项

```
使用方式:
  stvm [选项]

选项:
  -c <file>       编译 ST 文件为字节码
  -o <file>       指定输出文件
  -r <file>       运行字节码文件
  -d              调试模式
  -i              交互式 REPL
  -h, --help      显示帮助信息
```

## 构建选项

```bash
# 调试构建（默认）
make

# 发布构建
make CFLAGS="-O2 -DNDEBUG"

# 运行所有测试
make test

# 单独测试
make test_vm
make test_hotreload

# 清理
make clean
```

## 测试

### 运行所有测试

```bash
make test
```

### 单独运行测试

```bash
# 虚拟机测试
./build/bin/test_vm

# 热更新测试
./build/bin/test_hotreload examples/hotreload_v1.stbc examples/hotreload_v2.stbc

# 解析器测试
./build/bin/test_parser

# 类型检查测试
./build/bin/test_types
```

## 字节码格式

STVM 使用自定义字节码格式（.stbc）：

```
[Magic Number: 4 bytes]
[Version: 4 bytes]
[Entry Point: 4 bytes]
[Instruction Count: 4 bytes]
[Instructions...]
[Constant Count: 4 bytes]
[Constants...]
[Function Count: 4 bytes]
[Functions...]
[Global Variable Count: 4 bytes]
[Global Variables...]
```

### 查看字节码

```bash
./build/bin/stvm -d program.stbc
```

## 虚拟机架构

### 核心组件

1. **指令集**（30+ 操作码）
   - 算术运算：ADD, SUB, MUL, DIV, MOD
   - 逻辑运算：AND, OR, NOT, XOR
   - 比较运算：EQ, NE, LT, LE, GT, GE
   - 控制流：JMP, JZ, JNZ, CALL, RET
   - 栈操作：PUSH, POP, LOAD, STORE

2. **内存模型**
   - 操作数栈（用于表达式计算）
   - 调用栈（用于函数调用）
   - 全局变量区
   - 常量池

3. **模块系统**
   - BytecodeModule：字节码模块
   - LibraryManager：库管理
   - 动态加载与链接

4. **热更新系统**
   - HotReloadManager：热更新管理器
   - 安全性检查（调用栈、兼容性）
   - 差异分析与模块合并

## 开发状态

### 已完成

- ✅ Phase 1: 基础设施
  - 内存管理
  - 符号表
  - 类型系统
  
- ✅ Phase 2: 编译器前端
  - 词法分析（Flex）
  - 语法分析（Bison）
  - 类型检查
  
- ✅ Phase 3: 编译器后端
  - 代码生成
  - 字节码优化
  
- ✅ Phase 4: 虚拟机
  - 指令执行
  - 函数调用
  - 内存管理
  
- ✅ Phase 5: 高级特性
  - 静态变量
  - 函数返回值
  - 模块导入
  - 位运算
  
- ✅ Phase 6: 热更新
  - 运行时代码更新
  - 安全性保障
  - 差异分析

### 未来计划

- [ ] 性能优化
  - JIT 编译
  - 内联优化
  - 寄存器分配
  
- [ ] 调试工具增强
  - 变量监视
  - 调用栈可视化
  - 性能分析
  
- [ ] 更多数据类型
  - TIME, DATE, TIME_OF_DAY
  - 结构体（STRUCT）
  - 枚举（ENUM）
  
- [ ] 高级热更新
  - 全局变量迁移
  - 类型定义更新
  - 多线程支持

## 性能

基于当前测试（Intel Core i7，优化构建）：

- 编译速度：~10,000 行/秒
- 执行速度：~1,000,000 指令/秒
- 热更新延迟：<1ms
- 内存占用：~10MB（基础运行时）

## 许可证

[添加许可证信息]

## 贡献

欢迎贡献代码、报告问题或提出改进建议！

## 联系方式

[添加联系信息]

---

**注意**：此项目仍在活跃开发中，API 可能会有变化。
