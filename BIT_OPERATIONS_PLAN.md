# ST 语言位操作支持实现方案

## 问题分析

### 当前状态

STVM 已经支持逻辑运算符（布尔运算）：
- `AND` → `OP_AND` → 逻辑与 (`a && b`)
- `OR` → `OP_OR` → 逻辑或 (`a || b`)
- `NOT` → `OP_NOT` → 逻辑非 (`!a`)
- `XOR` → (未实现)

但**不支持位运算**（按位操作）。

### IEC 61131-3 标准要求

根据 IEC 61131-3 标准，ST 语言应该支持：

1. **逻辑运算符**（用于 BOOL 类型）:
   - `AND`, `OR`, `XOR`, `NOT`
   - 操作布尔值，返回布尔值

2. **位运算符**（用于整数类型）:
   - `AND`, `OR`, `XOR`, `NOT` - 按位操作
   - 操作整数类型（BYTE, WORD, DWORD, LWORD）
   - 返回相同类型

3. **移位运算符**:
   - `SHL` (Shift Left) - 左移
   - `SHR` (Shift Right) - 右移
   - `ROL` (Rotate Left) - 循环左移
   - `ROR` (Rotate Right) - 循环右移

### 关键问题

**同一个关键字，根据操作数类型有不同语义**:

```st
// 逻辑运算（BOOL 类型）
VAR
    flag1 : BOOL := TRUE;
    flag2 : BOOL := FALSE;
END_VAR

result := flag1 AND flag2;  // 逻辑与 → FALSE

// 位运算（整数类型）
VAR
    mask1 : BYTE := 16#F0;  // 11110000
    mask2 : BYTE := 16#0F;  // 00001111
END_VAR

result := mask1 AND mask2;  // 按位与 → 16#00
```

## 实现方案

### 方案 A: 类型重载（推荐）

**核心思想**: 同一个运算符根据操作数类型生成不同的字节码指令。

#### 1. 扩展字节码指令集

**bytecode.h**:
```c
typedef enum {
    // ... 现有指令 ...
    
    // === 逻辑运算（布尔运算，4个）===
    OP_AND,             // 逻辑与（BOOL && BOOL）
    OP_OR,              // 逻辑或（BOOL || BOOL）
    OP_XOR,             // 逻辑异或（BOOL XOR BOOL）- 新增
    OP_NOT,             // 逻辑非（!BOOL）
    
    // === 位运算（整数运算，4个）===
    OP_BIT_AND,         // 按位与（INT & INT）- 新增
    OP_BIT_OR,          // 按位或（INT | INT）- 新增
    OP_BIT_XOR,         // 按位异或（INT ^ INT）- 新增
    OP_BIT_NOT,         // 按位非（~INT）- 新增
    
    // === 移位运算（4个）===
    OP_SHL,             // 左移（INT << n）- 新增
    OP_SHR,             // 右移（INT >> n）- 新增
    OP_ROL,             // 循环左移 - 新增
    OP_ROR,             // 循环右移 - 新增
    
    // ... 其他指令 ...
} Opcode;
```

**指令数量**: 从 30 个增加到 42 个

#### 2. 类型检查和代码生成

**typecheck.c / codegen.c**:

```c
// 类型检查阶段
ErrorCode typecheck_binary_op(TypeChecker* checker, ASTNode* node) {
    ASTNode* left = node->left;
    ASTNode* right = node->right;
    
    DataType left_type = infer_type(left);
    DataType right_type = infer_type(right);
    
    switch (node->op) {
        case OP_AND:
        case OP_OR:
        case OP_XOR:
            if (left_type == TYPE_BOOL && right_type == TYPE_BOOL) {
                // 逻辑运算
                node->result_type = TYPE_BOOL;
                node->bytecode_op = node->op;  // OP_AND, OP_OR, OP_XOR
            } else if (is_integer_type(left_type) && is_integer_type(right_type)) {
                // 位运算
                node->result_type = promote_type(left_type, right_type);
                node->bytecode_op = node->op + BIT_OP_OFFSET;  // OP_BIT_AND, OP_BIT_OR, OP_BIT_XOR
            } else {
                return ERR_TYPE;  // 类型不匹配
            }
            break;
            
        case OP_NOT:
            if (left_type == TYPE_BOOL) {
                node->result_type = TYPE_BOOL;
                node->bytecode_op = OP_NOT;
            } else if (is_integer_type(left_type)) {
                node->result_type = left_type;
                node->bytecode_op = OP_BIT_NOT;
            } else {
                return ERR_TYPE;
            }
            break;
    }
    
    return OK;
}
```

#### 3. VM 执行

**vm.c**:

```c
// 位运算实现
case OP_BIT_AND: {
    Value b = POP();
    Value a = POP();
    Value result;
    
    switch (a.type) {
        case TYPE_INT:
            result.type = TYPE_INT;
            result.int_val = a.int_val & b.int_val;
            break;
        case TYPE_DWORD:
            result.type = TYPE_DWORD;
            result.dword_val = a.dword_val & b.dword_val;
            break;
        // ... 其他整数类型
        default:
            return ERR_TYPE;
    }
    
    PUSH(result);
    break;
}

case OP_SHL: {
    Value shift_amount = POP();
    Value value = POP();
    Value result;
    
    result.type = value.type;
    switch (value.type) {
        case TYPE_INT:
            result.int_val = value.int_val << shift_amount.int_val;
            break;
        case TYPE_DWORD:
            result.dword_val = value.dword_val << shift_amount.int_val;
            break;
        // ... 其他类型
    }
    
    PUSH(result);
    break;
}
```

#### 4. 词法和语法

**st_lexer.l** (已存在):
```c
(?i:AND)            { return TOKEN_AND; }
(?i:OR)             { return TOKEN_OR; }
(?i:XOR)            { return TOKEN_XOR; }
(?i:NOT)            { return TOKEN_NOT; }
(?i:SHL)            { return TOKEN_SHL; }  // 新增
(?i:SHR)            { return TOKEN_SHR; }  // 新增
(?i:ROL)            { return TOKEN_ROL; }  // 新增
(?i:ROR)            { return TOKEN_ROR; }  // 新增
```

**st_parser.y**:
```c
// 运算符优先级
%left TOKEN_OR TOKEN_XOR
%left TOKEN_AND
%left TOKEN_EQ TOKEN_NE
%left TOKEN_LT TOKEN_LE TOKEN_GT TOKEN_GE
%left TOKEN_SHL TOKEN_SHR TOKEN_ROL TOKEN_ROR  // 新增
%left TOKEN_ADD TOKEN_SUB
%left TOKEN_MUL TOKEN_DIV TOKEN_MOD
%right TOKEN_NOT
%right UMINUS

// 语法规则（不需要修改，类型检查阶段处理）
expression:
    | expression TOKEN_AND expression { /* ... */ }
    | expression TOKEN_OR expression { /* ... */ }
    | expression TOKEN_XOR expression { /* ... */ }
    | TOKEN_NOT expression { /* ... */ }
    | expression TOKEN_SHL expression { /* 新增 */ }
    | expression TOKEN_SHR expression { /* 新增 */ }
```

### 方案 B: 显式位运算函数（补充）

作为库函数提供，用于复杂操作：

**bitlib.st**:
```st
// 位测试
FUNCTION BitTest : BOOL
    VAR_INPUT
        value : DWORD;
        bit_pos : INT;
    END_VAR
    BitTest := (value AND SHL(1, bit_pos)) <> 0;
END_FUNCTION

// 位设置
FUNCTION BitSet : DWORD
    VAR_INPUT
        value : DWORD;
        bit_pos : INT;
    END_VAR
    BitSet := value OR SHL(1, bit_pos);
END_FUNCTION

// 位清除
FUNCTION BitClear : DWORD
    VAR_INPUT
        value : DWORD;
        bit_pos : INT;
    END_VAR
    BitClear := value AND NOT SHL(1, bit_pos);
END_FUNCTION

// 位翻转
FUNCTION BitToggle : DWORD
    VAR_INPUT
        value : DWORD;
        bit_pos : INT;
    END_VAR
    BitToggle := value XOR SHL(1, bit_pos);
END_FUNCTION

// 计数置位位数（Population Count）
FUNCTION PopCount : INT
    VAR_INPUT
        value : DWORD;
    END_VAR
    VAR
        count : INT := 0;
        i : INT;
    END_VAR
    
    FOR i := 0 TO 31 DO
        IF BitTest(value, i) THEN
            count := count + 1;
        END_IF;
    END_FOR;
    
    PopCount := count;
END_FUNCTION
```

## 实现步骤

### 第一阶段：基础位运算（必需）

1. **扩展字节码指令**:
   - 添加 `OP_BIT_AND`, `OP_BIT_OR`, `OP_BIT_XOR`, `OP_BIT_NOT`
   - 添加 `OP_XOR`（逻辑异或）
   - 添加 `OP_SHL`, `OP_SHR`

2. **修改类型检查器**:
   - 根据操作数类型选择正确的指令
   - 类型提升（BYTE + WORD = WORD）

3. **实现 VM 指令**:
   - 位运算执行逻辑
   - 移位运算执行逻辑

4. **添加词法支持**:
   - `TOKEN_SHL`, `TOKEN_SHR`

5. **更新语法分析器**:
   - 添加移位运算的语法规则

### 第二阶段：高级功能（可选）

1. **循环移位**:
   - `OP_ROL`, `OP_ROR`

2. **位操作库**:
   - 编写 `bitlib.st`
   - 提供常用位操作函数

3. **优化**:
   - 常量折叠（编译期计算常量位运算）
   - 强度削减（如 `x * 2` → `x SHL 1`）

## 示例代码

### 示例 1: 寄存器操作

```st
PROGRAM RegisterControl
VAR
    status_reg : BYTE := 16#00;
    MOTOR_ENABLE : BYTE := 16#01;    // bit 0
    PUMP_ENABLE : BYTE := 16#02;     // bit 1
    ALARM_ENABLE : BYTE := 16#04;    // bit 2
END_VAR

// 启用电机
status_reg := status_reg OR MOTOR_ENABLE;

// 禁用泵
status_reg := status_reg AND NOT PUMP_ENABLE;

// 切换报警
status_reg := status_reg XOR ALARM_ENABLE;

// 检查电机是否启用
IF (status_reg AND MOTOR_ENABLE) <> 0 THEN
    // 电机正在运行
END_IF;
END_PROGRAM
```

**编译后的字节码**:
```
LOAD status_reg
PUSH 16#01
BIT_OR                    // 位或
STORE status_reg

LOAD status_reg
PUSH 16#02
BIT_NOT                   // 位非
BIT_AND                   // 位与
STORE status_reg
```

### 示例 2: 数据打包

```st
FUNCTION PackBytes : DWORD
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
    
    PackBytes := result;
END_FUNCTION

FUNCTION UnpackByte : BYTE
    VAR_INPUT
        value : DWORD;
        index : INT;  // 0-3
    END_VAR
    
    UnpackByte := SHR(value, index * 8) AND 16#FF;
END_FUNCTION
```

### 示例 3: 快速乘除法

```st
PROGRAM FastMath
VAR
    x : INT := 100;
    result : INT;
END_VAR

// x * 4 = x << 2
result := SHL(x, 2);  // 400

// x / 8 = x >> 3
result := SHR(x, 3);  // 12

// x * 10 = (x << 3) + (x << 1)
result := SHL(x, 3) + SHL(x, 1);  // 1000
END_PROGRAM
```

## 类型系统考虑

### 整数类型层次

```
BYTE (8-bit)  ─┐
WORD (16-bit) ─┼─→ 类型提升规则
DWORD (32-bit)─┤
LWORD (64-bit)─┘
```

### 类型提升规则

```c
DataType promote_type(DataType t1, DataType t2) {
    if (t1 == TYPE_LWORD || t2 == TYPE_LWORD) return TYPE_LWORD;
    if (t1 == TYPE_DWORD || t2 == TYPE_DWORD) return TYPE_DWORD;
    if (t1 == TYPE_WORD || t2 == TYPE_WORD) return TYPE_WORD;
    return TYPE_BYTE;
}
```

### 隐式转换

```st
VAR
    b : BYTE := 16#FF;
    w : WORD;
END_VAR

w := b;  // OK - 向上转换（零扩展）
b := w;  // 警告或错误 - 向下转换（可能丢失数据）
```

## 性能考虑

### 位运算 vs 布尔运算

| 操作 | 指令 | 类型 | 性能 |
|------|------|------|------|
| `a && b` | `OP_AND` | BOOL | 快速（短路求值） |
| `a & b` | `OP_BIT_AND` | INT | 快速（单周期） |
| `a AND b` | 根据类型 | 自动选择 | 快速 |

### 移位 vs 乘除法

```st
// 慢速（需要完整的乘法器）
result := x * 16;

// 快速（位移运算，1个周期）
result := SHL(x, 4);
```

编译器优化可以自动将 2 的幂次乘除转换为移位。

## 测试用例

### test_bitops.st

```st
PROGRAM test_bitops
VAR
    a : BYTE := 16#AA;  // 10101010
    b : BYTE := 16#55;  // 01010101
    result : BYTE;
END_VAR

// 测试位与
result := a AND b;  // 应该 = 16#00
ASSERT(result = 16#00, 'BIT AND failed');

// 测试位或
result := a OR b;   // 应该 = 16#FF
ASSERT(result = 16#FF, 'BIT OR failed');

// 测试位异或
result := a XOR b;  // 应该 = 16#FF
ASSERT(result = 16#FF, 'BIT XOR failed');

// 测试位非
result := NOT a;    // 应该 = 16#55
ASSERT(result = 16#55, 'BIT NOT failed');

// 测试左移
result := SHL(16#01, 4);  // 应该 = 16#10
ASSERT(result = 16#10, 'SHL failed');

// 测试右移
result := SHR(16#80, 4);  // 应该 = 16#08
ASSERT(result = 16#08, 'SHR failed');
END_PROGRAM
```

## 兼容性考虑

### 与现有代码的兼容性

✅ **完全兼容** - 现有使用 `AND`, `OR`, `NOT` 的布尔逻辑代码不受影响

```st
// 现有代码 - 仍然正常工作
IF flag1 AND flag2 THEN
    // ...
END_IF;
```

### 与 IEC 61131-3 标准的兼容性

✅ **符合标准** - 实现符合 IEC 61131-3 定义的位运算语义

## 文档更新

需要更新以下文档：

1. **README.md** - 添加位运算支持说明
2. **语言参考手册** - 详细说明位运算语法
3. **操作码参考** - 新增指令说明
4. **库函数参考** - bitlib 文档

## 总结

### 推荐实现方案

✅ **方案 A（类型重载）+ 方案 B（库函数）**

**理由**:
1. 符合 IEC 61131-3 标准
2. 性能最优（原生指令）
3. 类型安全（编译期检查）
4. 灵活性高（库函数补充）

### 实现优先级

**高优先级**（核心功能）:
1. ✅ 位运算指令（AND, OR, XOR, NOT）
2. ✅ 移位指令（SHL, SHR）
3. ✅ 类型检查和代码生成

**中优先级**（增强功能）:
1. ⏳ 循环移位（ROL, ROR）
2. ⏳ 位操作库函数
3. ⏳ 编译器优化

**低优先级**（高级功能）:
1. ⏹️ 向量位运算
2. ⏹️ 并行位操作

### 工作量估算

| 任务 | 工作量 | 说明 |
|------|--------|------|
| 扩展指令集 | 2 小时 | 修改头文件 |
| 类型检查 | 4 小时 | 类型推导逻辑 |
| VM 实现 | 3 小时 | 指令执行 |
| 词法/语法 | 1 小时 | 添加 token |
| 测试 | 4 小时 | 单元测试 + 集成测试 |
| 文档 | 2 小时 | 使用说明 |
| **总计** | **16 小时** | 约 2 个工作日 |

---

**建议**: 先实现基础位运算（方案 A），然后逐步添加库函数（方案 B）和优化功能。
