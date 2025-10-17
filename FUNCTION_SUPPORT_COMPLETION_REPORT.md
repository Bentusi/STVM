# STVM 函数支持完整修复报告

## 修复日期
2025年10月17日

## 问题概述
STVM 在实现函数支持时遇到多个问题，包括：
1. TypeInfo 双重释放导致的内存错误
2. 代码生成未正确注册局部变量
3. 函数调用使用错误的地址/索引
4. VM 未为局部变量分配栈空间
5. 程序结构导致全局初始化被跳过

## 修复详情

### 1. 符号表所有权管理 (symtbl.h/symtbl.c)

**问题**: TypeInfo 在 typecheck 和 codegen 两个阶段都被注册到符号表，导致双重释放。

**修复**:
- 在 `Symbol` 结构中添加 `bool owns_type` 字段
- 修改 `scope_free()` 仅当 `owns_type=true` 时才释放 TypeInfo
- 设置规则:
  - `symtbl_define_variable`: owns_type=false (TypeInfo 来自 AST)
  - `symtbl_define_parameter`: owns_type=false (TypeInfo 来自 AST)
  - `symtbl_define_function`: owns_type=true (新创建的函数类型)
  - `symtbl_define_constant`: owns_type=true (新创建的类型)

### 2. 代码生成局部变量注册 (codegen.c)

**问题**: `codegen_function` 未在生成代码前注册局部变量到符号表，导致 "Undefined variable" 错误。

**修复**:
- 在 `codegen_function` 中，参数注册后添加局部变量注册循环:
```c
// 注册局部变量到符号表（在生成初始化代码之前）
ASTNode* decl = func_decl->data.function_decl.declarations;
while (decl) {
    if (decl->type == AST_VAR_DECL) {
        const char* var_name = decl->data.var_decl.name;
        TypeInfo* var_type = decl->data.var_decl.type;
        bool is_const = decl->data.var_decl.is_const;
        
        if (!symtbl_define_variable(ctx->symtbl, var_name, var_type, is_const)) {
            // 错误处理
        }
        ctx->local_var_count++;
    }
    decl = decl->next;
}
```

- 修改 `generate_var_decls()` 移除计数逻辑，仅负责生成初始化代码

### 3. 全局变量计数设置 (codegen.c)

**问题**: `module->global_count` 未设置，导致 VM 无法正确分配全局变量数组。

**修复**:
- 在 `generate_program()` 开始处添加:
```c
ctx->module->global_count = ctx->symtbl->global_var_count;
```

### 4. 程序结构优化 (codegen.c)

**问题**: 全局初始化代码在函数之前，但 entry_point 指向主程序体，导致全局初始化被跳过。

**修复** - 重构 `generate_program()`:
```c
1. 设置 entry_point = 0
2. 生成全局变量初始化
3. 发射 JMP 跳过函数定义
4. 生成所有函数定义
5. 回填 JMP 目标到主程序体
6. 生成主程序体
7. 发射 HALT
```

### 5. 函数调用索引修复 (codegen.c, vm.c)

**问题**: `OP_CALL` 指令的 operand 使用函数地址，但应该使用函数索引。

**修复**:
- `generate_function_call()`: 改为发射函数索引
```c
uint32_t func_index = func - ctx->module->functions;
codegen_emit(ctx, OP_CALL, (uint16_t)func_index);
```

- `vm_run()` 和 `vm_step()` 中的 `OP_CALL` 处理: 将 operand 视为函数索引
```c
uint32_t func_idx = instr.operand;
FunctionEntry* func = &vm->module->functions[func_idx];
// ...
vm->pc = func->address;  // 使用函数表中的地址
```

### 6. 局部变量栈空间分配 (vm.c) ⭐ 关键修复

**问题**: 函数调用时只压入参数，未为额外的局部变量分配栈空间，导致访问越界。

**症状**: 简单函数（只有参数）可运行，带局部变量的函数失败。

**修复** - 在 `OP_CALL` 处理中添加栈空间分配:
```c
vm->call_stack[++vm->call_sp] = frame;

// 为局部变量（参数之外的）在栈上分配空间
int32_t locals_only = func->local_count - func->param_count;
for (int32_t i = 0; i < locals_only; i++) {
    Value v = {.type = TYPE_VOID};
    PUSH(v);
}

vm->pc = func->address;
```

**原理**:
- 调用前: 栈 = [..., arg0, arg1, arg2], sp 指向 arg2
- base_pointer = sp - param_count + 1 (指向 arg0)
- 需要为额外的局部变量(local_count - param_count)压入空值
- 调用后: 栈 = [..., arg0, arg1, arg2, local3, local4, ...], sp 增加
- 局部变量访问: stack[base_pointer + offset] 现在在有效范围内

## 测试结果

### 通过的测试
✅ `examples/hello.st` - 基本程序执行
✅ `examples/control_flow.st` - IF/FOR/WHILE 控制流
✅ `examples/functions.st` - 函数定义、调用、带局部变量、递归
✅ `test_func_minimal.st` - 最小函数测试
✅ `test_func_local.st` - 局部变量测试

### 未实现的功能
❌ `examples/arrays.st` - 数组赋值未实现
❌ `examples/case.st` - CASE 语句解析未实现

## 关键代码变更统计

### 修改的文件
1. `src/include/symtbl.h` - 添加 owns_type 字段
2. `src/core/symtbl.c` - 条件释放 TypeInfo，设置 owns_type
3. `src/core/codegen.c` - 局部变量注册、程序结构、global_count
4. `src/core/vm.c` - OP_CALL 局部变量栈分配、函数索引处理

### 新增测试文件
1. `test_func_minimal.st` - 最小函数测试
2. `test_func_local.st` - 局部变量测试

## 技术要点

### 函数调用约定
```
调用前栈布局:
  [...other data...]
  arg0          <- base_pointer (after CALL)
  arg1
  arg2          <- sp (before CALL)

CALL 执行后:
  [...other data...]
  arg0          <- base_pointer
  arg1
  arg2
  local3        <- 新分配
  local4        <- 新分配, sp (after CALL)

RET 执行:
  清理局部变量，保留返回值
  sp = base_pointer - 1
  如果有返回值，压栈
```

### 符号表作用域管理
- typecheck 阶段: 进入作用域、注册符号、检查类型、离开作用域
- codegen 阶段: 进入作用域、**重新注册**符号（使用 AST 的 TypeInfo）、生成代码、离开作用域
- 离开作用域不释放 Scope，只改变 current_scope
- TypeInfo 所有权由 owns_type 控制，避免双重释放

## 性能影响

局部变量栈空间预分配的影响:
- ✅ 优点: 简化实现，访问快速，无需动态扩展
- ⚠️ 缺点: 每次调用都分配，即使变量未使用
- 💡 优化方向: 编译时分析哪些局部变量实际使用，按需分配

## 已知限制

1. 函数不支持可变参数
2. 不支持函数指针/高阶函数
3. 局部变量必须在函数开始声明，不支持块级作用域
4. 递归深度受调用栈大小限制（当前固定）
5. 返回值通过覆盖第一个参数位置传递（IEC 61131-3 风格）

## 下一步建议

1. ✅ 已完成: 基本函数支持
2. 🔄 进行中: 标准库函数绑定
3. 📋 待实现:
   - 数组作为函数参数/返回值
   - 结构体/用户定义类型
   - CASE 语句支持
   - 优化: 尾调用消除
   - 优化: 内联小函数

## 总结

本次修复完成了 STVM 函数支持的核心功能，包括:
- ✅ 函数定义与声明
- ✅ 函数调用与参数传递
- ✅ 局部变量与作用域
- ✅ 函数返回值
- ✅ 递归函数（Factorial 示例）

所有核心示例（hello, control_flow, functions）现在都能正确执行，标志着 STVM 达到了一个重要的里程碑。
