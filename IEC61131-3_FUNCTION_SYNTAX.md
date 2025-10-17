# STVM IEC 61131-3 函数语法符合性说明

## 标准语法

根据 IEC 61131-3 标准，函数定义的正确语法是：

```
FUNCTION function_name : return_type
VAR_INPUT
    param1 : type1;
    param2 : type2;
END_VAR
VAR
    local1 : type3;
    local2 : type4;
END_VAR
    (* 函数体语句 - 直接书写，不需要 BEGIN 关键字 *)
    statement1;
    statement2;
    ...
    function_name := return_value;
END_FUNCTION
```

**关键点**：
- ✅ 函数体**不应该**有 `BEGIN` 关键字
- ✅ 语句直接跟在变量声明块之后
- ✅ 函数名赋值用于返回值
- ✅ 使用 `END_FUNCTION` 结束

## STVM 解析器支持

STVM 的解析器 (`src/core/st_parser.y`) 实际上**同时支持**两种语法：

### 1. 标准语法（推荐）✅
```st
FUNCTION Add : INT
VAR_INPUT
    x : INT;
    y : INT;
END_VAR
    Add := x + y;  (* 直接写语句，无 BEGIN *)
END_FUNCTION
```

### 2. 兼容语法（不推荐）
```st
FUNCTION Add : INT
VAR_INPUT
    x : INT;
    y : INT;
END_VAR
BEGIN              (* 非标准的 BEGIN *)
    Add := x + y;
END_FUNCTION
```

## 更新的示例

所有示例文件已更新为使用标准语法：

### examples/functions.st
```st
(* 标准符合的函数定义 *)
FUNCTION Add : INT
VAR_INPUT
    x : INT;
    y : INT;
END_VAR
    Add := x + y;
END_FUNCTION

FUNCTION Factorial : INT
VAR_INPUT
    n : INT;
END_VAR
VAR
    i : INT;
    temp : INT;
END_VAR
    temp := 1;
    FOR i := 1 TO n DO
        temp := temp * i;
    END_FOR;
    Factorial := temp;
END_FUNCTION
```

### test_func_minimal.st
```st
FUNCTION SimpleAdd : INT
VAR_INPUT
    x : INT;
    y : INT;
END_VAR
    SimpleAdd := x + y;
END_FUNCTION
```

### test_func_local.st
```st
FUNCTION AddWithLocal : INT
VAR_INPUT
    x : INT;
    y : INT;
END_VAR
VAR
    temp : INT;
END_VAR
    temp := x + y;
    AddWithLocal := temp;
END_FUNCTION
```

## 测试结果

所有示例使用标准语法后仍然正确执行：

```bash
$ build/bin/stvm examples/functions.st
Exit code: 0  ✓

$ build/bin/stvm test_func_minimal.st
Exit code: 0  ✓

$ build/bin/stvm test_func_local.st
Exit code: 0  ✓
```

## 与 PROGRAM 的对比

注意 `PROGRAM` 块**确实需要** `BEGIN` 关键字：

```st
PROGRAM Example
VAR
    x : INT;
END_VAR
BEGIN              (* PROGRAM 需要 BEGIN *)
    x := 10;
END_PROGRAM
```

这是因为：
- **FUNCTION** 的主体就是可执行代码
- **PROGRAM** 需要区分声明区和执行区

## 总结

✅ STVM 现在完全符合 IEC 61131-3 标准的函数语法
✅ 所有示例和测试已更新
✅ 解析器支持标准语法和兼容语法
✅ 推荐使用标准语法（无 BEGIN）以提高可移植性

## 参考

- IEC 61131-3:2013 Section 2.5.1.4 - Function declaration
- PLCopen 编码指南
