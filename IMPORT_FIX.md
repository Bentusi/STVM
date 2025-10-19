# IMPORT 外部库功能修复说明

## 问题描述

用户报告运行 `examples/test_mathlib_demo.st` 时出现错误：
```
运行时错误: External function 'Power' not registered at PC=3
```

该文件使用 `IMPORT` 语法从预编译的 `.stbc` 字节码库中导入函数：
```st
IMPORT Power FROM 'examples/mathlib.stbc';
IMPORT Factorial FROM 'examples/mathlib.stbc';
```

## 根本原因

虽然 STVM 的虚拟机（VM）已经实现了库函数调用机制，但代码生成器在生成外部函数调用时使用了**简单函数名**（如 `Power`），而不是**完全限定名**（如 `examples/mathlib.stbc.Power`）。

### VM 的库函数查找机制

VM 的 `OP_CALL_EXT` 指令处理程序（在 `src/core/vm.c` 第 690-778 行）实现了以下逻辑：

1. 检查函数名是否包含 `.stbc.` 模式
2. 提取库路径（如 `examples/mathlib.stbc`）
3. 在 `vm->libmgr` 中查找对应的库
4. 在库模块中查找函数
5. 如果找到，切换模块并执行库函数

这个机制**要求函数名是完全限定名**，格式为：`<library_path>.stbc.<function_name>`

### 代码生成器的问题

在 `src/core/codegen.c` 的 `generate_function_call` 函数中，代码生成器直接使用 AST 中的简单函数名：
```c
FunctionEntry* func = bytecode_find_function(ctx->module, node->data.function_call.name);
uint32_t func_idx = bytecode_add_function(ctx->module,
                                          node->data.function_call.name,  // 问题：使用简单名
                                          ...);
```

这导致生成的字节码中函数名是 `Power`，而不是 `examples/mathlib.stbc.Power`，VM 无法识别为库函数。

### 符号表中的信息

库管理器（`libmgr`）在导入符号时，已经正确保存了完全限定名：
```c
// src/core/libmgr.c, libmgr_import_symbol 函数
char qualified_name[512];
snprintf(qualified_name, sizeof(qualified_name), "%s.%s", library_name, symbol_name);
global_sym->qualified_name = mmgr_strdup(qualified_name);  // 保存完全限定名
global_sym->is_library = true;  // 标记为库符号
```

但代码生成器没有使用这个信息。

## 修复方案

修改 `src/core/codegen.c` 的 `generate_function_call` 函数，使其对库函数使用完全限定名：

```c
// 首先检查符号表，判断是内部函数还是外部函数
Symbol* sym = symtbl_lookup(ctx->symtbl, node->data.function_call.name);

// 获取函数名（对于库函数使用完全限定名）
const char* func_name = node->data.function_call.name;
if (sym && sym->is_library && sym->qualified_name) {
    // 这是导入的库函数，使用完全限定名
    func_name = sym->qualified_name;
}

// 后续使用 func_name 而不是 node->data.function_call.name
FunctionEntry* func = bytecode_find_function(ctx->module, func_name);
// ...
uint32_t func_idx = bytecode_add_function(ctx->module,
                                          func_name,  // 使用完全限定名
                                          ...);
```

### 关键改动

1. **检查 `is_library` 标志**：确定符号是否来自外部库
2. **使用 `qualified_name`**：如果是库符号，使用完全限定名
3. **传递正确的函数名**：确保字节码中存储的是完全限定名

## 测试验证

### 测试 1：数学库导入
```bash
./build/bin/stvm examples/test_mathlib_demo.st
```

**预期输出：**
```
Power(2, 10) = 1024
Factorial(5) = 120
GCD(48, 18) = 6
Fibonacci(10) = 55
Lerp(0.0, 100.0, 0.75) = 75.000000
```

✅ **结果：PASS**

### 测试 2：简单数学库
```bash
./build/bin/stvm examples/test_mathlib.st
```

**预期输出：**
```
Square(5) = 25
Power(2, 10) = 1024
Factorial(5) = 120
```

✅ **结果：PASS**

### 测试 3：静态变量功能（回归测试）
```bash
./build/bin/stvm examples/test_static_complete.st
```

**预期输出：**
```
Passed: 12/12 tests
Status: ALL TESTS PASSED!
```

✅ **结果：PASS**

### 测试 4：工程库（回归测试）
```bash
./build/bin/stvm examples/engineering_basic.st
```

**预期输出：**
```
Passed: 10/10 tests
Status: ALL TESTS PASSED!
```

✅ **结果：PASS**

## 技术细节

### 完全限定名的格式

对于导入的库函数，完全限定名遵循以下格式：
```
<library_path>.<function_name>
```

示例：
- `examples/mathlib.stbc.Power`
- `examples/mathlib.stbc.Factorial`
- `lib/utils.stbc.Printf`

### VM 的函数查找流程

1. 接收 `OP_CALL_EXT` 指令
2. 从函数表中获取函数名
3. 检查名称是否包含 `.stbc.` 模式
4. 如果包含：
   - 提取库路径（`.stbc` 之前的部分）
   - 提取函数名（`.stbc.` 之后的部分）
   - 在 `vm->libmgr->libraries` 中查找库
   - 在库模块的函数表中查找函数
   - 执行库函数
5. 如果不包含：
   - 在 `vm->external_functions` 中查找 C 回调函数
   - 执行外部函数

### 库管理器的角色

`LibraryManager` (`libmgr`) 负责：
1. **加载库文件**：读取 `.stbc` 字节码文件
2. **构建符号表**：从库模块创建符号表
3. **符号导入**：将库符号注册到全局符号表
4. **保存完全限定名**：为每个库符号保存 `qualified_name`
5. **运行时支持**：VM 通过 `vm->libmgr` 访问已加载的库

## 影响范围

### 修改的文件
- `src/core/codegen.c`：修改 `generate_function_call` 函数（8 行修改）

### 不影响的功能
- 内部函数调用（无完全限定名）
- C 外部函数调用（通过 `vm_register_external_function` 注册）
- 所有现有测试（静态变量、工程库等）

### 向后兼容性
✅ 完全向后兼容 - 只影响库函数调用，不改变任何现有行为

## 相关代码位置

- **VM 库函数查找**：`src/core/vm.c` 第 690-778 行
- **代码生成器修复**：`src/core/codegen.c` 第 680-720 行
- **库符号导入**：`src/core/libmgr.c` 第 299-396 行
- **符号表结构**：`src/include/symtbl.h` 第 35-60 行

## 总结

通过让代码生成器对库函数使用完全限定名，IMPORT 外部库功能现在完全正常工作。这个修复：

1. ✅ 解决了 "External function not registered" 错误
2. ✅ 保持了向后兼容性
3. ✅ 不影响任何现有功能
4. ✅ 代码改动最小（8 行）
5. ✅ 所有测试通过

用户现在可以成功使用 IMPORT 语法导入和调用外部库函数。
