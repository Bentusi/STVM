# 位操作快速实现指南

## 回答您的问题

**Q: 如何支持位操作，是使用 ST 库实现还是需要原生词法和语法支持？**

**A: 推荐使用原生语法支持（方案 A）+ 库函数补充（方案 B）**

### 为什么需要原生支持？

1. **符合标准**: IEC 61131-3 标准定义了位运算符
2. **性能最优**: 直接编译为字节码指令
3. **类型安全**: 编译期类型检查
4. **代码可读**: 符合 PLC 编程习惯

### 实现方式对比

| 方式 | 优点 | 缺点 | 推荐度 |
|------|------|------|--------|
| **纯库函数** | 简单，无需修改编译器 | 性能差，语法繁琐 | ⭐⭐ |
| **原生支持** | 性能好，语法简洁 | 需要修改编译器 | ⭐⭐⭐⭐⭐ |
| **混合方案** | 灵活，兼顾性能和扩展性 | 需要维护两套 | ⭐⭐⭐⭐ |

## 最小实现（30 分钟）

只需修改 **3 个文件**即可实现基础位运算：

### 步骤 1: 扩展字节码指令（5 分钟）

**文件**: `src/include/bytecode.h`

```c
typedef enum {
    // ... 现有指令 ...
    OP_AND,             // 逻辑与
    OP_OR,              // 逻辑或
    OP_NOT,             // 逻辑非
    
    // 新增：位运算指令
    OP_BIT_AND,         // 按位与
    OP_BIT_OR,          // 按位或
    OP_BIT_XOR,         // 按位异或
    OP_BIT_NOT,         // 按位非
    OP_SHL,             // 左移
    OP_SHR,             // 右移
    
    // ... 其他指令 ...
} Opcode;
```

### 步骤 2: 实现 VM 指令（15 分钟）

**文件**: `src/core/vm.c`

在 `vm_run_from()` 函数的 switch 语句中添加：

```c
case OP_BIT_AND: {
    Value b = POP();
    Value a = POP();
    Value result;
    result.type = a.type;
    result.int_val = a.int_val & b.int_val;
    PUSH(result);
    break;
}

case OP_BIT_OR: {
    Value b = POP();
    Value a = POP();
    Value result;
    result.type = a.type;
    result.int_val = a.int_val | b.int_val;
    PUSH(result);
    break;
}

case OP_BIT_XOR: {
    Value b = POP();
    Value a = POP();
    Value result;
    result.type = a.type;
    result.int_val = a.int_val ^ b.int_val;
    PUSH(result);
    break;
}

case OP_BIT_NOT: {
    Value a = POP();
    Value result;
    result.type = a.type;
    result.int_val = ~a.int_val;
    PUSH(result);
    break;
}

case OP_SHL: {
    Value shift = POP();
    Value value = POP();
    Value result;
    result.type = value.type;
    result.int_val = value.int_val << shift.int_val;
    PUSH(result);
    break;
}

case OP_SHR: {
    Value shift = POP();
    Value value = POP();
    Value result;
    result.type = value.type;
    result.int_val = value.int_val >> shift.int_val;
    PUSH(result);
    break;
}
```

### 步骤 3: 类型检查和代码生成（10 分钟）

**文件**: `src/core/codegen.c`

修改二元运算符代码生成函数：

```c
static ErrorCode codegen_binary_op(CodeGenContext* ctx, ASTNode* node) {
    // ... 生成左右操作数代码 ...
    
    // 根据操作数类型选择指令
    DataType left_type = /* 获取左操作数类型 */;
    DataType right_type = /* 获取右操作数类型 */;
    
    Opcode op;
    switch (node->data.binary_op.op) {
        case TOKEN_AND:
            if (left_type == TYPE_BOOL && right_type == TYPE_BOOL) {
                op = OP_AND;  // 逻辑与
            } else {
                op = OP_BIT_AND;  // 位与
            }
            break;
            
        case TOKEN_OR:
            if (left_type == TYPE_BOOL && right_type == TYPE_BOOL) {
                op = OP_OR;
            } else {
                op = OP_BIT_OR;
            }
            break;
            
        case TOKEN_XOR:
            op = OP_BIT_XOR;  // XOR 总是位运算
            break;
            
        case TOKEN_NOT:
            if (left_type == TYPE_BOOL) {
                op = OP_NOT;
            } else {
                op = OP_BIT_NOT;
            }
            break;
            
        // 其他运算符...
    }
    
    bytecode_add_instruction(ctx->module, op, 0, 0);
    return OK;
}
```

## 完整实现（2-4 小时）

### 额外步骤

4. **添加词法支持** (`src/core/st_lexer.l`):
   ```c
   (?i:SHL)  { return TOKEN_SHL; }
   (?i:SHR)  { return TOKEN_SHR; }
   ```

5. **更新语法分析器** (`src/core/st_parser.y`):
   ```c
   %left TOKEN_SHL TOKEN_SHR
   
   expression:
       | expression TOKEN_SHL expression {
           $$ = ast_create_binary_op(OP_SHL, $1, $3);
       }
       | expression TOKEN_SHR expression {
           $$ = ast_create_binary_op(OP_SHR, $1, $3);
       }
   ```

6. **添加测试用例**:
   ```st
   // test_bitops.st
   PROGRAM test_bitops
   VAR
       a : INT := 16#AA;
       b : INT := 16#55;
       result : INT;
   END_VAR
   
   result := a AND b;    // 位与
   result := a OR b;     // 位或
   result := a XOR b;    // 位异或
   result := NOT a;      // 位非
   result := SHL(a, 2);  // 左移
   result := SHR(a, 2);  // 右移
   END_PROGRAM
   ```

## 使用示例

### 示例 1: 位掩码操作

```st
VAR
    status : BYTE;
    MOTOR_BIT : BYTE := 16#01;  // bit 0
    PUMP_BIT : BYTE := 16#02;   // bit 1
END_VAR

// 设置位
status := status OR MOTOR_BIT;

// 清除位
status := status AND NOT PUMP_BIT;

// 切换位
status := status XOR MOTOR_BIT;

// 测试位
IF (status AND MOTOR_BIT) <> 0 THEN
    // 位已设置
END_IF;
```

### 示例 2: 快速乘除法

```st
VAR
    x : INT := 100;
END_VAR

// x * 4 = x << 2
result := SHL(x, 2);  // 400

// x / 8 = x >> 3
result := SHR(x, 3);  // 12
```

### 示例 3: 数据提取

```st
// 从 32 位值中提取字节
FUNCTION GetByte : BYTE
VAR_INPUT
    value : DWORD;
    index : INT;  // 0-3
END_VAR
    GetByte := SHR(value, index * 8) AND 16#FF;
END_FUNCTION

// 组合字节为 32 位值
FUNCTION MakeDword : DWORD
VAR_INPUT
    b0, b1, b2, b3 : BYTE;
END_VAR
VAR
    result : DWORD;
END_VAR
    result := b0;
    result := result OR SHL(b1, 8);
    result := result OR SHL(b2, 16);
    result := result OR SHL(b3, 24);
    MakeDword := result;
END_FUNCTION
```

## 库函数方案（备选）

如果不想修改编译器，可以纯用库函数实现：

**bitlib.st**:
```st
// 注意：这种方式性能较差，仅作为演示

FUNCTION BitAnd : INT
VAR_INPUT
    a, b : INT;
END_VAR
VAR
    result : INT := 0;
    i : INT;
    mask : INT;
END_VAR
    FOR i := 0 TO 15 DO
        mask := SHL(1, i);
        IF (a MOD (mask * 2)) >= mask AND (b MOD (mask * 2)) >= mask THEN
            result := result + mask;
        END_IF;
    END_FOR;
    BitAnd := result;
END_FUNCTION

// 使用
result := BitAnd(16#AA, 16#55);
```

**缺点**:
- ❌ 性能差（循环实现）
- ❌ 语法繁琐（函数调用）
- ❌ 不符合标准

## 建议

### 推荐方案：原生支持

**理由**:
1. ✅ 符合 IEC 61131-3 标准
2. ✅ 性能最优（单周期指令）
3. ✅ 语法简洁（`a AND b`）
4. ✅ 类型安全（编译期检查）
5. ✅ 易于维护

### 实施建议

**第一阶段（立即实施）**:
- 实现基础位运算（AND, OR, XOR, NOT）
- 实现移位运算（SHL, SHR）
- 添加测试用例

**第二阶段（可选）**:
- 添加库函数（复杂位操作）
- 实现循环移位（ROL, ROR）
- 编译器优化（强度削减）

### 工作量

| 任务 | 时间 |
|------|------|
| 最小实现（核心功能） | 30 分钟 |
| 完整实现（含测试） | 2-4 小时 |
| 库函数开发 | 1-2 小时 |
| 文档编写 | 1 小时 |

## 参考资料

### IEC 61131-3 标准

**位运算符** (Table 51):
- `AND` - Bitwise AND
- `OR` - Bitwise OR
- `XOR` - Bitwise exclusive OR
- `NOT` - Bitwise complement

**移位运算符** (Table 52):
- `SHL` - Shift left
- `SHR` - Shift right
- `ROL` - Rotate left
- `ROR` - Rotate right

### 相关文档

- `BIT_OPERATIONS_PLAN.md` - 详细实现方案
- `README.md` - 项目说明
- IEC 61131-3 标准文档

---

**总结**: 强烈推荐使用**原生语法支持**来实现位操作，这是最符合标准、性能最优、最易维护的方案。
