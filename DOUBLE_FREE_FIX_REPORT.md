# 双重释放问题修复报告

## 日期
2025年10月17日

## 问题描述

在运行 `build/bin/stvm examples/hello.st` 和 `build/bin/stvm examples/control_flow.st` 时，出现多个"Warning: Double free detected"警告。

### 错误输出示例
```
Warning: Double free detected at 0x763020687048
Warning: Double free detected at 0x763020687248
Warning: Double free detected at 0x763020687448
Warning: Double free detected at 0x763020687648
Warning: Double free detected at 0x763020687848
```

## 根本原因分析

通过代码审查和 valgrind 分析，发现了双重释放的根本原因：

### 问题所在

在 AST（抽象语法树）节点创建时，`TypeInfo` 类型信息使用**引用计数**管理内存。但是在 AST 创建函数中，**没有正确增加引用计数**。

#### 具体流程

1. **Parser 创建 TypeInfo**：
   ```c
   type_spec: TOKEN_INT { $$ = type_info_create(TYPE_INT); }  // ref_count = 1
   ```

2. **多个 AST 节点共享同一个 TypeInfo**：
   ```c
   // 第一个变量
   ast_create_var_decl("counter", type_info, NULL, false);  // 直接赋值，未增加 ref_count
   
   // 第二个变量（共享同一个 TYPE_INT）
   ast_create_var_decl("result", type_info, NULL, false);   // 直接赋值，未增加 ref_count
   ```

3. **释放时的问题**：
   ```c
   ast_free_node(first_var);   // 调用 type_info_free，ref_count: 1 -> 0，释放 TypeInfo
   ast_free_node(second_var);  // 再次调用 type_info_free，尝试释放已释放的 TypeInfo -> 双重释放！
   ```

### 代码缺陷位置

**`src/core/ast.c`**

1. **`ast_create_var_decl`** (第 82 行)：
   ```c
   node->data.var_decl.type = type;  // ❌ 应该调用 type_info_retain(type)
   ```

2. **`ast_create_function_decl`** (第 101 行)：
   ```c
   node->data.function_decl.return_type = return_type;  // ❌ 应该调用 type_info_retain
   ```

**`src/core/st_parser.y`**

3. **数组类型创建** (第 235 行)：
   ```c
   int32_t* sizes = (int32_t*)malloc(sizeof(int32_t));  // ❌ 应该使用 mmgr_alloc
   ```

## 修复方案

### 1. 修复 `ast_create_var_decl`

**修改前**：
```c
node->data.var_decl.type = type;
```

**修改后**：
```c
node->data.var_decl.type = type ? type_info_retain(type) : NULL;
```

### 2. 修复 `ast_create_function_decl`

**修改前**：
```c
node->data.function_decl.return_type = return_type;
```

**修改后**：
```c
node->data.function_decl.return_type = return_type ? type_info_retain(return_type) : NULL;
```

### 3. 修复 parser 中的内存分配

**修改前**：
```c
int32_t* sizes = (int32_t*)malloc(sizeof(int32_t));
```

**修改后**：
```c
int32_t* sizes = (int32_t*)mmgr_alloc(sizeof(int32_t));
```

## 修复后的验证

### 测试结果

1. **无警告运行**：
   ```bash
   $ ./build/bin/stvm examples/hello.st
   # 无输出，无警告
   ```

2. **Verbose 模式**：
   ```bash
   $ ./build/bin/stvm examples/hello.st -V
   编译并运行文件: examples/hello.st
   开始语法分析...
   语法分析完成
   开始类型检查...
   类型检查完成
   开始代码生成...
   代码生成完成
   开始执行...
   执行完成
   # 无双重释放警告
   ```

3. **Valgrind 验证**：
   ```bash
   $ valgrind --leak-check=summary --error-exitcode=1 ./build/bin/stvm examples/hello.st
   ...
   ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
   ```
   ✅ **0 errors** - 双重释放问题完全解决！

### 测试脚本验证

```bash
$ ./test_default_behavior.sh
=== 测试 STVM 默认行为 ===
...
=== 所有测试完成 ===
```

所有测试通过，无双重释放警告。

## 影响范围

### 修改的文件
- `src/core/ast.c` - 修复了 2 处 TypeInfo 引用计数问题
- `src/core/st_parser.y` - 修复了 1 处内存分配混用问题

### 影响的组件
- AST 节点创建
- TypeInfo 生命周期管理
- 内存管理一致性

## 剩余问题

### 内存泄漏（非关键）

Valgrind 报告显示仍有 67 字节的 definitely lost 内存泄漏，主要来源于：

**原因**：Lexer 使用 `mmgr_strdup` 或 `mmgr_alloc` 分配字符串并赋值给 `yylval.string_value`，但这些字符串在 AST 创建时被**再次复制**（使用 `mmgr_strdup`），原始字符串从未被释放。

**位置**：
- `src/core/st_lexer.l` 第 105 行和第 120 行
- Parser 将这些字符串传递给 AST 创建函数
- AST 创建函数再次复制字符串

**影响**：每个标识符和字符串字面量都会泄漏少量内存（通常几十字节）。

**建议修复**（可选）：
1. 在 Parser 规则中释放 lexer 分配的字符串（在 AST 节点创建后）
2. 或者修改 AST 创建函数直接接管字符串所有权（不复制）

**优先级**：低（不影响功能，内存泄漏量小）

## 总结

✅ **双重释放问题已完全解决**
- 修复了 TypeInfo 引用计数管理
- 修复了内存分配混用
- 所有测试通过，无警告
- Valgrind 确认无错误

⚠️ **仍存在少量内存泄漏**
- 来自 lexer 分配的字符串
- 不影响功能
- 可在后续优化中处理

## 技术要点

### TypeInfo 引用计数规则

1. **创建时**：`ref_count = 1`
2. **共享时**：调用 `type_info_retain()` 增加 `ref_count`
3. **释放时**：调用 `type_info_free()` 减少 `ref_count`，为 0 时真正释放

### 正确的使用模式

```c
// 创建 TypeInfo
TypeInfo* type = type_info_create(TYPE_INT);  // ref_count = 1

// 在多个地方使用
node1->type = type_info_retain(type);  // ref_count = 2
node2->type = type_info_retain(type);  // ref_count = 3

// 释放
type_info_free(node1->type);  // ref_count = 2
type_info_free(node2->type);  // ref_count = 1
type_info_free(type);         // ref_count = 0, 真正释放
```

## 参考

- Valgrind 内存检查日志：`valgrind.log`
- 测试脚本：`test_default_behavior.sh`
- 相关文档：`DEFAULT_BEHAVIOR_UPDATE.md`
