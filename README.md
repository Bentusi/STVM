# STVM - ST Language Compiler and Virtual Machine

IEC 61131-3 结构化文本（ST）语言编译器与虚拟机实现

## 项目概述

STVM 是一个用 C 语言实现的 ST 语言编译器和虚拟机，设计目标是提供高效、易于理解的代码生成和执行系统。

## 已完成的工作

### 第一阶段：基础设施 ✅

1. **内存管理器 (mmgr)**
   - 文件：`src/include/mmgr.h`, `src/core/mmgr.c`
   - 功能：
     - 内存池管理（小/中/大对象池，AST/符号/字符串专用池）
     - 统一的内存分配接口
     - 内存使用统计
     - 内存泄漏检测
   - 测试：`tests/test_mmgr.c` ✅

2. **基础类型系统 (types)**
   - 文件：`src/include/types.h`, `src/core/types.c`
   - 功能：
     - 数据类型定义（VOID, BOOL, INT, REAL, STRING, ARRAY, FUNCTION）
     - 运行时值表示（Value结构）
     - 类型信息管理（TypeInfo）
     - 类型转换和比较
   - 测试：`tests/test_types.c` ⚠️（需要修复内存管理）

3. **错误处理系统 (error)**
   - 文件：`src/include/error.h`（仅头文件）
   - 功能：
     - 简化的错误码定义
     - 错误码转字符串
     - 错误报告宏（融入各模块上下文）
   - 无独立测试（集成在其他模块中）

### 第二阶段：核心数据结构 ✅

4. **字节码系统 (bytecode)**
   - 文件：`src/include/bytecode.h`, `src/core/bytecode.c`
   - 功能：
     - 精简的28个核心指令集
     - 固定4字节指令格式
     - 常量池管理
     - 函数表管理
     - 字节码模块结构
     - 反汇编支持
   - 测试：`tests/test_bytecode.c` ✅

5. **抽象语法树 (ast)**
   - 文件：`src/include/ast.h`, `src/core/ast.c`
   - 功能：
     - 完整的AST节点类型（程序、声明、语句、表达式）
     - 二元/一元运算符支持
     - 控制流节点（if, while, for, case）
     - 函数声明和调用
     - 数组访问
     - 源码位置追踪
     - AST打印和访问者模式
   - 测试：`tests/test_ast.c` ✅

6. **符号表 (symtbl)**
   - 文件：`src/include/symtbl.h`, `src/core/symtbl.c`
   - 功能：
     - 哈希表实现
     - 作用域管理（嵌套作用域）
     - 符号类型（变量、函数、参数、常量）
     - 全局/局部变量管理
     - 限定名支持（模块.符号）
     - 库符号注册
     - 符号查找和遍历
   - 测试：`tests/test_symtbl.c` ✅

## 项目结构

```
lexer/
├── src/
│   ├── include/          # 头文件
│   │   ├── error.h       # 错误码定义
│   │   ├── types.h       # 基础类型
│   │   ├── mmgr.h        # 内存管理器
│   │   ├── bytecode.h    # 字节码定义
│   │   ├── ast.h         # 抽象语法树
│   │   └── symtbl.h      # 符号表
│   └── core/             # 实现文件
│       ├── mmgr.c
│       ├── types.c
│       ├── bytecode.c
│       ├── ast.c
│       └── symtbl.c
├── tests/                # 测试文件
│   ├── test_mmgr.c
│   ├── test_types.c
│   ├── test_bytecode.c
│   ├── test_ast.c
│   └── test_symtbl.c
├── build/                # 构建输出
│   ├── obj/              # 目标文件
│   └── bin/              # 可执行文件
├── Makefile              # 构建系统
├── CLAUDE.md             # 设计文档
└── README.md             # 本文件
```

## 构建和测试

### 编译所有模块

```bash
make all
```

### 运行所有测试

```bash
make test
```

### 运行单个测试

```bash
make test_mmgr      # 内存管理器测试
make test_types     # 类型系统测试
make test_bytecode  # 字节码测试
make test_ast       # AST测试
make test_symtbl    # 符号表测试
```

### 清理构建产物

```bash
make clean
```

### 查看帮助

```bash
make help
```

## 设计亮点

### 1. 精简的指令集（28个核心指令）

- **栈操作**：PUSH, POP, DUP, LOAD, STORE（5个）
- **算术运算**：ADD, SUB, MUL, DIV, MOD, NEG（6个）
- **比较运算**：EQ, NE, LT, LE, GT, GE（6个）
- **逻辑运算**：AND, OR, NOT（3个）
- **控制流**：JMP, JZ, JNZ, CALL, RET（5个）
- **其他**：HALT, CALL_EXT, NOP（3个）

指令正交性好，易于解释执行，为JIT优化留空间。

### 2. 简化的错误处理

- 错误码直接作为函数返回值
- 错误信息存储在产生错误的上下文中
- 无需全局错误状态管理
- 代码更简洁，调用链清晰

### 3. 高效的内存管理

- 多级内存池（小/中/大对象池）
- 专用池（AST节点、符号表、字符串）
- 内存使用统计和泄漏检测
- 池重置功能（快速清理）

### 4. 灵活的符号表

- 支持嵌套作用域
- 限定名支持库模块系统
- 哈希表实现，查找高效
- 区分全局/局部变量

## 下一步计划

### 第三阶段：编译器前端（5-7天）

7. **词法分析器 (st_lexer)**
   - 使用 Flex 或手写
   - 识别ST语言关键字、标识符、字面量
   - 处理注释和空白

8. **语法分析器 (st_parser)**
   - 使用 Bison 或递归下降
   - 构建AST
   - 简单的语法错误恢复

9. **类型检查器 (typecheck)**
   - 语义分析
   - 类型推导和验证
   - 符号引用解析

### 第四阶段：编译器后端（4-5天）

10. **代码生成器 (codegen)**
    - AST到字节码转换
    - 栈式表达式求值
    - 跳转地址回填

11. **字节码序列化 (bytecode_io)**
    - .stbc文件格式
    - 加载和保存字节码

### 第五阶段：运行时系统（4-5天）

12. **虚拟机 (vm)**
    - 字节码解释执行
    - 直接跳转表优化
    - 调用栈管理

13. **库管理器 (libmgr)**
    - 库文件加载
    - 符号导入和解析

### 第六阶段：应用集成（2-3天）

14. **命令行工具 (main)**
    - 编译模式
    - 运行模式
    - 交互模式（REPL）

15. **调试器（可选）**
    - 断点、单步执行
    - 变量查看
    - 调用栈追踪

## 已知问题

1. ⚠️ **types测试中的内存管理问题**
   - 类型信息的递归释放导致double free
   - 需要更仔细地管理类型信息的所有权
   - 建议：使用引用计数或明确所有权规则

2. ⚠️ **内存管理器的小泄漏**
   - test_mmgr显示有320字节泄漏
   - 可能是测试代码未完全清理
   - 不影响核心功能

## 开发环境

- **编译器**：GCC 11+
- **标准**：C11
- **调试**：GDB
- **构建系统**：Make
- **平台**：Linux (也应该可在 macOS/Windows 移植)

## 贡献指南

当前项目处于早期开发阶段，欢迎：

- Bug 报告
- 代码审查
- 性能优化建议
- 测试用例完善

## 许可证

[待定]

## 作者

开发团队

---

**更新日期**：2025-10-10
**状态**：第二阶段完成
