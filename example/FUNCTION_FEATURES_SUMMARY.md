# IEC61131 ST语言复杂函数定义和调用示例

## 已验证的功能特性

### 1. 基本函数定义 ✅
```st
FUNCTION Add : INT
VAR_INPUT
    a : INT;
    b : INT;
END_VAR
    RETURN a + b;
END_FUNCTION
```

### 2. 带有条件判断的函数 ✅
```st
FUNCTION Max : INT
VAR_INPUT
    a : INT;
    b : INT;
END_VAR
    IF a > b THEN
        RETURN a;
    ELSE
        RETURN b;
    END_IF
END_FUNCTION
```

### 3. 多种参数方向 ✅
```st
FUNCTION TestParams : BOOL
VAR_INPUT
    input_val : INT;      (* 输入参数 *)
END_VAR
VAR_OUTPUT
    output_val : INT;     (* 输出参数 *)
END_VAR
VAR_IN_OUT
    inout_val : INT;      (* 输入输出参数 *)
END_VAR
    output_val := input_val * 2;
    inout_val := inout_val + 1;
    RETURN TRUE;
END_FUNCTION
```

### 4. 递归函数 ✅
```st
FUNCTION Factorial : INT
VAR_INPUT
    n : INT;
END_VAR
    IF n <= 1 THEN
        RETURN 1;
    ELSE
        RETURN n * Factorial(n - 1);
    END_IF
END_FUNCTION
```

### 5. 带局部变量的函数 ⚠️ (编译成功但有显示问题)
```st
FUNCTION Power : INT
VAR_INPUT
    base : INT;
    exponent : INT;
END_VAR
VAR
    result : INT;         (* 局部变量 *)
    i : INT;             (* 局部变量 *)
END_VAR
    result := 1;
    FOR i := 1 TO exponent DO
        result := result * base;
    END_FOR
    RETURN result;
END_FUNCTION
```

## 已测试的复杂语法结构

### 控制流结构
- ✅ IF-THEN-ELSE 语句
- ✅ FOR 循环
- ✅ 多个 RETURN 语句
- ✅ 嵌套条件判断

### 表达式和运算
- ✅ 二元运算符 (+, -, *, /, MOD)
- ✅ 比较运算符 (<, <=, >, >=, =, <>)
- ✅ 布尔运算符 (AND, OR, NOT)
- ✅ 一元运算符 (-, NOT)

### 数据类型
- ✅ INT (整数)
- ✅ BOOL (布尔)
- ✅ REAL (实数)
- ✅ STRING (字符串)

## 字节码生成验证

编译器能正确生成以下字节码指令：

1. **变量操作**: `LOAD_VAR`, `STORE_VAR`
2. **常量加载**: `LOAD_INT`, `LOAD_BOOL`, `LOAD_REAL`
3. **算术运算**: `ADD`, `SUB`, `MUL`, `DIV`
4. **比较运算**: `GT`, `LT`, `LE`, `GE`, `EQ`, `NE`
5. **控制流**: `JZ`, `JMP`
6. **函数调用**: `CALL`, `RET`

## 当前限制

### 已知问题
1. **单文件多函数**: 目前不支持在一个文件中定义多个函数
2. **局部变量显示**: 带局部变量的函数可能有无限循环的显示问题
3. **函数重载**: 不支持同名函数的不同参数签名

### 不支持的IEC61131-3高级特性
1. **参数默认值**: `a : INT := 10`
2. **EN/ENO参数**: 标准的使能输入/输出
3. **ARRAY类型**: 数组参数
4. **STRUCT类型**: 结构体参数
5. **多个源文件**: 函数库和模块系统

## 总结

该IEC61131 ST语言编译器在函数定义和调用方面表现出色，能够：

1. ✅ **正确解析**复杂的函数语法
2. ✅ **生成正确的字节码**
3. ✅ **支持递归调用**
4. ✅ **处理多种参数方向**
5. ✅ **支持复杂的控制流**
6. ✅ **正确实现RETURN语句**

这是一个功能完整的基础实现，符合IEC61131-3标准的核心要求。