# 函数静态变量特性说明

## 概述

STVM 支持在函数内部使用 `VAR` 块定义静态变量（类似 C 语言的 `static` 变量）。这些变量：
- **持久性**：在函数调用之间保持值
- **作用域**：仅在声明它们的函数内部可见
- **独立性**：不同函数可以使用相同的变量名而互不冲突

## 与 VAR_LOCAL 的区别

| 特性 | VAR（静态变量） | VAR_LOCAL（局部变量） |
|------|----------------|---------------------|
| 生命周期 | 程序整个运行期间 | 函数调用期间 |
| 初始化 | 只在程序启动时初始化一次 | 每次函数调用时初始化 |
| 值的保持 | 函数返回后保持值 | 函数返回后销毁 |
| 存储位置 | 全局/静态存储区 | 栈（局部变量区） |
| 作用域 | 函数内部 | 函数内部 |

## 语法示例

### 计数器函数
```st
FUNCTION Counter : INT
VAR
    count : INT := 0;  (* 静态变量，初始化一次 *)
END_VAR
    count := count + 1;
    Counter := count;
END_FUNCTION

(* 调用示例 *)
x := Counter();  (* 返回 1 *)
x := Counter();  (* 返回 2 *)
x := Counter();  (* 返回 3 *)
```

### RS 触发器
```st
FUNCTION RS_Trigger : BOOL
VAR_INPUT
    S : BOOL;  (* 置位输入 *)
    R : BOOL;  (* 复位输入 *)
END_VAR
VAR
    state : BOOL := FALSE;  (* 静态状态变量 *)
END_VAR
    IF R THEN
        state := FALSE;
    ELSIF S THEN
        state := TRUE;
    END_IF;
    RS_Trigger := state;
END_FUNCTION

(* 调用示例 *)
out := RS_Trigger(TRUE, FALSE);   (* state 置为 TRUE *)
out := RS_Trigger(FALSE, FALSE);  (* state 保持 TRUE *)
out := RS_Trigger(FALSE, TRUE);   (* state 复位为 FALSE *)
```

### 多函数使用相同变量名

不同函数可以使用相同的静态变量名，它们互不干扰：

```st
FUNCTION Counter_A : INT
VAR
    counter : INT := 0;  (* Counter_A 的计数器 *)
END_VAR
    counter := counter + 1;
    Counter_A := counter;
END_FUNCTION

FUNCTION Counter_B : INT
VAR
    counter : INT := 100;  (* Counter_B 的计数器，独立于 Counter_A *)
END_VAR
    counter := counter + 10;
    Counter_B := counter;
END_FUNCTION

(* Counter_A: 1, 2, 3, ... *)
(* Counter_B: 110, 120, 130, ... *)
```

## 实现原理

### 命名空间隔离
静态变量使用**完全限定名**存储，格式为 `FunctionName.VarName`：
- `Counter_A.counter` → 全局索引 4
- `Counter_B.counter` → 全局索引 5

这确保了不同函数的静态变量互不冲突。

### 符号表策略
采用双符号策略：
1. **全局存储**：使用完全限定名（如 `Counter_A.counter`）存储在全局作用域
2. **局部别名**：在函数内部使用短名（如 `counter`）访问

### 代码生成
在代码生成阶段，当访问变量时：
1. 如果当前在函数内，先尝试查找 `FunctionName.VarName`
2. 如果找到且是静态变量，使用其全局索引
3. 否则按常规方式查找局部/全局变量

## 典型应用场景

### 1. 状态机
```st
FUNCTION StateMachine : INT
VAR
    state : INT := 0;  (* 保持当前状态 *)
END_VAR
    (* 状态转换逻辑 *)
    IF state = 0 THEN
        state := 1;
    ELSIF state = 1 THEN
        state := 2;
    ELSE
        state := 0;
    END_IF;
    StateMachine := state;
END_FUNCTION
```

### 2. 滤波器
```st
FUNCTION FirstOrderFilter : REAL
VAR_INPUT
    input : REAL;
    alpha : REAL;  (* 滤波系数 0-1 *)
END_VAR
VAR
    last_output : REAL := 0.0;  (* 上次输出值 *)
END_VAR
    last_output := alpha * input + (1.0 - alpha) * last_output;
    FirstOrderFilter := last_output;
END_FUNCTION
```

### 3. 边沿检测
```st
FUNCTION RisingEdge : BOOL
VAR_INPUT
    signal : BOOL;
END_VAR
VAR
    last_signal : BOOL := FALSE;
    edge : BOOL;
END_VAR
    edge := signal AND NOT last_signal;
    last_signal := signal;
    RisingEdge := edge;
END_FUNCTION
```

### 4. 计数/累加
```st
FUNCTION Accumulator : REAL
VAR_INPUT
    value : REAL;
END_VAR
VAR
    sum : REAL := 0.0;
END_VAR
    sum := sum + value;
    Accumulator := sum;
END_FUNCTION
```

## 注意事项

1. **初始化时机**：静态变量只在程序启动时初始化一次，不是每次函数调用时初始化
2. **线程安全**：当前实现不保证多线程环境下的线程安全（STVM 是单线程的）
3. **递归调用**：静态变量在递归调用中是共享的，所有递归层级使用同一个变量
4. **初始值**：建议总是提供初始值（`:= value`），否则变量值未定义

## 测试验证

运行测试套件验证静态变量功能：

```bash
# 基础静态变量测试
./build/bin/stvm examples/test_static_complete.st

# 工程库测试（使用静态变量实现）
./build/bin/stvm examples/engineering_basic.st
```

预期结果：所有测试通过（12/12 和 10/10）。

## IEC 61131-3 标准符合性

此实现符合 IEC 61131-3 标准对函数 VAR 块的语义：
- VAR 块在函数中定义的变量具有"保持"语义（retained）
- 变量在函数调用之间保持值
- 每个函数实例有自己独立的变量存储

这与标准的 RETAIN 关键字语义一致，用于需要在电源循环或函数调用之间保持状态的变量。
