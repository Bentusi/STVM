# MathLib - 综合数学函数库

这是一个为 STVM 创建的全面数学函数库，提供整型和实数的常用数学运算。

## 编译库文件

```bash
./build/bin/stvm -c examples/mathlib.st --library -V
```

生成文件：`examples/mathlib.stbc`

## 整型函数 (INT)

### 基本运算 (5个)
- **Add(a, b)** - 加法
- **Subtract(a, b)** - 减法
- **Multiply(a, b)** - 乘法
- **Divide(a, b)** - 除法（带零检查）
- **Modulo(a, b)** - 取模运算（带零检查）

### 比较和极值 (4个)
- **Max(a, b)** - 两个数的最大值
- **Min(a, b)** - 两个数的最小值
- **Max3(a, b, c)** - 三个数的最大值
- **Min3(a, b, c)** - 三个数的最小值

### 符号和绝对值 (3个)
- **Abs(x)** - 绝对值
- **Sign(x)** - 符号函数，返回 -1, 0, 或 1
- **Negate(x)** - 取反（负数）

### 幂运算 (3个)
- **Square(x)** - 平方 (x²)
- **Cube(x)** - 立方 (x³)
- **Power(base, exp)** - 乘方 (base^exp)，仅支持非负整数指数

### 范围和限制 (2个)
- **Clamp(value, min_val, max_val)** - 将值限制在范围内
- **InRange(value, min_val, max_val)** - 判断值是否在范围内（返回 BOOL）

### 奇偶性和整除性 (3个)
- **IsEven(x)** - 判断是否为偶数（返回 BOOL）
- **IsOdd(x)** - 判断是否为奇数（返回 BOOL）
- **IsDivisible(dividend, divisor)** - 判断是否能被整除（返回 BOOL）

### 组合数学 (4个)
- **Factorial(n)** - 阶乘 (n!)
- **GCD(a, b)** - 最大公约数（欧几里得算法）
- **LCM(a, b)** - 最小公倍数
- **Fibonacci(n)** - 斐波那契数列第n项

## 实数函数 (REAL)

### 基本运算 (4个)
- **AddReal(a, b)** - 实数加法
- **SubtractReal(a, b)** - 实数减法
- **MultiplyReal(a, b)** - 实数乘法
- **DivideReal(a, b)** - 实数除法（带零检查）

### 比较和极值 (4个)
- **MaxReal(a, b)** - 两个实数的最大值
- **MinReal(a, b)** - 两个实数的最小值
- **MaxReal3(a, b, c)** - 三个实数的最大值
- **MinReal3(a, b, c)** - 三个实数的最小值

### 符号和绝对值 (3个)
- **AbsReal(x)** - 实数绝对值
- **SignReal(x)** - 实数符号函数，返回 INT (-1, 0, 或 1)
- **NegateReal(x)** - 实数取反

### 范围和限制 (2个)
- **ClampReal(value, min_val, max_val)** - 将实数限制在范围内
- **InRangeReal(value, min_val, max_val)** - 判断实数是否在范围内（返回 BOOL）

### 幂运算 (3个)
- **SquareReal(x)** - 实数平方
- **CubeReal(x)** - 实数立方
- **PowerReal(base, exp)** - 实数整数次幂，支持负指数

### 插值和映射 (4个)
- **Lerp(start_val, end_val, t)** - 线性插值（t范围：0.0-1.0）
- **LerpUnclamped(start_val, end_val, t)** - 无限制线性插值
- **MapRange(value, in_min, in_max, out_min, out_max)** - 映射值从一个范围到另一个范围
- **InverseLerp(start_val, end_val, value)** - 反向插值，获取t值

### 近似比较 (2个)
- **ApproxEqual(a, b, epsilon)** - 近似相等（指定容差）
- **ApproxZero(x, epsilon)** - 近似为零

### 平均值 (3个)
- **Average(a, b)** - 两个数的算术平均值
- **Average3(a, b, c)** - 三个数的算术平均值
- **WeightedAverage(a, b, weight_a, weight_b)** - 加权平均值

### 百分比计算 (3个)
- **Percentage(part, whole)** - 计算百分比
- **FromPercentage(percent, whole)** - 从百分比计算值
- **PercentageChange(old_value, new_value)** - 百分比变化

## 函数总数

- **整型函数**: 24个
- **实数函数**: 24个
- **总计**: **48个函数**

## 使用示例

### 导入单个函数
```st
PROGRAM MyProgram

IMPORT Add FROM 'examples/mathlib.stbc';
IMPORT Square FROM 'examples/mathlib.stbc';
IMPORT Lerp FROM 'examples/mathlib.stbc';

VAR
    result : INT;
    real_result : REAL;
END_VAR

result := Add(10, 20);          (* 30 *)
result := Square(5);             (* 25 *)
real_result := Lerp(0.0, 100.0, 0.5);  (* 50.0 *)

END_PROGRAM
```

### 使用别名导入库
```st
PROGRAM MyProgram

IMPORT 'examples/mathlib.stbc' AS math;

VAR
    x : INT;
    y : REAL;
END_VAR

x := math.Power(2, 10);         (* 1024 *)
x := math.GCD(48, 18);          (* 6 *)
x := math.Fibonacci(10);        (* 55 *)
y := math.MapRange(50.0, 0.0, 100.0, 0.0, 1.0);  (* 0.5 *)

END_PROGRAM
```

## 实现特点

1. **安全的除法**：所有除法和取模运算都包含零检查
2. **避免负数字面值**：使用 `0 - x` 代替 `-x`（适配编译器特性）
3. **避免 ELSIF+ELSE 组合**：使用独立的 IF 语句（避免编译器 bug）
4. **支持负指数**：PowerReal 函数支持负整数指数
5. **欧几里得算法**：高效计算 GCD
6. **迭代实现**：Fibonacci 使用迭代而非递归

## 注意事项

- 字符串必须使用单引号：`'string'` 而非 `"string"`
- 函数局部变量必须使用 `VAR_LOCAL` 而非 `VAR`
- Power 和 PowerReal 的指数参数为整数类型
- 近似比较函数需要手动指定 epsilon 值
- GCD 和 LCM 对负数输入会自动取绝对值

## 文件信息

- **源文件**: `examples/mathlib.st`
- **库文件**: `examples/mathlib.stbc`
- **测试文件**: `examples/test_mathlib.st`
- **文档**: `examples/MATHLIB_README.md`

---
**版本**: 1.0  
**创建日期**: 2024  
**状态**: ✅ 编译成功，48个函数可用
