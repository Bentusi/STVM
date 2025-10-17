# PRINT 函数实现完成报告

## 概述
成功为 STVM 添加了 PRINT 函数，提供类似 C 语言 printf 的格式化输出功能，符合 IEC 61131-3 ST 语言规范。

## 实现的功能

### 1. 内置函数模块
创建了独立的内置函数模块：
- `src/include/builtins.h` - 内置函数声明
- `src/core/builtins.c` - 内置函数实现

### 2. PRINT 函数特性

#### 支持的格式说明符
- `%d` - INT 类型整数
- `%f` - REAL 类型浮点数（自动支持 INT 到 REAL 的转换）
- `%s` - STRING 类型字符串
- `%b` - BOOL 类型布尔值（输出 TRUE/FALSE）
- `%%` - 字面百分号

#### 支持的转义序列
- `\n` - 换行符
- `\t` - 制表符
- `\r` - 回车符
- `\\` - 反斜杠
- `\'` - 单引号
- `\"` - 双引号

#### 可变参数支持
- PRINT 函数接受可变数量的参数
- 第一个参数必须是格式字符串
- 后续参数根据格式说明符匹配

### 3. 语言集成

#### 词法分析器扩展 (`src/core/st_lexer.l`)
```lex
(?i:PRINT)          { return TOKEN_PRINT; }
```

#### 语法分析器扩展 (`src/core/st_parser.y`)
- 添加 `TOKEN_PRINT` 声明
- 添加 `print_stmt` 语法规则
- 将 PRINT 语句编译为外部函数调用
```yacc
print_stmt:
    TOKEN_PRINT TOKEN_LPAREN argument_list TOKEN_RPAREN TOKEN_SEMICOLON
```

#### 类型检查器扩展 (`src/core/typecheck.c`)
- 在初始化时自动注册 PRINT 函数
- 支持可变参数函数类型检查（`param_count = -1`）

#### 代码生成器修正 (`src/core/codegen.c`)
- 修复外部函数调用生成逻辑
- 区分内部函数（address != 0）和外部函数（address == 0）
- 正确生成 `OP_CALL_EXT` 指令

#### 虚拟机集成 (`src/core/vm.c`)
- 在 VM 创建时自动注册所有内置函数
- 通过 `builtins_register_all()` 统一管理

## 使用示例

### 基本输出
```st
PRINT('Hello, World!\n');
```

### 格式化输出
```st
VAR
    x : INT := 42;
    y : REAL := 3.14;
    name : STRING := 'STVM';
    flag : BOOL := TRUE;
END_VAR

PRINT('x = %d, y = %f\n', x, y);
PRINT('Name: %s, Active: %b\n', name, flag);
```

### 数组和循环
```st
VAR
    numbers : ARRAY[5] OF INT;
    i : INT;
END_VAR

FOR i := 0 TO 4 DO
    numbers[i] := i * 10;
    PRINT('numbers[%d] = %d\n', i, numbers[i]);
END_FOR;
```

## 测试结果

### 测试文件
1. **simple_print.st** - 基本功能测试
   ```
   Hello, World!
   x = 42
   Done!
   ```
   ✅ 通过

2. **print_test.st** - 综合功能测试
   - 基本字符串输出
   - 整数格式化
   - 浮点数格式化
   - 布尔值输出
   - 字符串插值
   - 混合格式输出
   - 数组元素输出
   - 转义序列处理
   ✅ 全部通过

### 兼容性测试
- ✅ `examples/arrays.st` - 数组操作正常
- ✅ `examples/functions.st` - 函数调用正常
- ✅ `examples/control_flow.st` - 控制流正常
- ✅ `examples/hello.st` - 基本程序正常

## 技术架构

### 外部函数调用流程
```
ST源码 → 解析 (PRINT 语句) 
      → 类型检查 (验证可变参数)
      → 代码生成 (OP_CALL_EXT指令)
      → VM执行 (查找外部函数)
      → 调用C回调 (builtin_print)
      → 格式化输出到stdout
```

### 参数传递机制
- 参数从左到右压入栈
- VM 通过 `vm_get_arg(vm, index)` 获取参数
- 索引从栈底往上计数（index=argc-1 是第一个参数）
- 外部函数执行后，VM 自动弹出参数

### 可变参数处理
- 符号表中 `param_count = -1` 表示可变参数
- 类型检查器跳过参数数量验证
- 代码生成器将实际参数数量编码在 `flags` 字段
- VM 将参数数量传递给外部函数

## 设计优势

1. **模块化设计**
   - 内置函数独立于 VM 核心
   - 易于添加新的内置函数

2. **类型安全**
   - 运行时检查参数类型
   - 清晰的错误消息

3. **灵活性**
   - 支持可变参数
   - 自动类型转换（INT → REAL）

4. **符合标准**
   - 遵循 IEC 61131-3 语法
   - PRINT 作为关键字而非库函数

5. **可扩展性**
   - 易于添加更多格式说明符
   - 易于添加其他内置函数（如 PRINTLN, FORMAT 等）

## 未来改进方向

1. 更多格式说明符
   - `%x` - 十六进制输出
   - `%o` - 八进制输出
   - `%c` - 字符输出
   - 精度控制（如 `%.2f`）

2. 更多内置函数
   - `PRINTLN` - 自动换行
   - `FORMAT` - 返回格式化字符串
   - `READ` - 读取输入

3. 输出重定向
   - 支持输出到文件
   - 支持输出到字符串缓冲区

4. 国际化支持
   - 本地化数字格式
   - Unicode 字符支持

## 总结

成功实现了功能完整、易于使用的 PRINT 函数，为 STVM 提供了强大的调试和输出能力。实现遵循了模块化设计原则，代码清晰易维护，测试充分，与现有功能完美集成。

---
**完成日期**: 2025-10-17  
**测试状态**: ✅ 所有测试通过  
**代码状态**: ✅ 编译成功，无错误  
**新增文件**: 2 个 (builtins.h, builtins.c)  
**修改文件**: 5 个 (vm.c, st_lexer.l, st_parser.y, typecheck.c, codegen.c)
