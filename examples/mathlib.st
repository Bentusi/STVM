(*****************************************************************************
 * MathLib - 综合数学函数库
 * 提供整型和实数的常用数学运算函数
 *****************************************************************************)

PROGRAM MathLib

(* 整型最大值 *)
FUNCTION Max : INT
VAR_INPUT
    a : INT;
    b : INT;
END_VAR
    IF a > b THEN
        Max := a;
    ELSE
        Max := b;
    END_IF;
END_FUNCTION

(* 整型最小值 *)
FUNCTION Min : INT
VAR_INPUT
    a : INT;
    b : INT;
END_VAR
    IF a < b THEN
        Min := a;
    ELSE
        Min := b;
    END_IF;
END_FUNCTION

(* 三个数的最大值 *)
FUNCTION Max3 : INT
VAR_INPUT
    a : INT;
    b : INT;
    c : INT;
END_VAR
VAR_LOCAL
    temp : INT;
END_VAR
    temp := a;
    IF b > temp THEN
        temp := b;
    END_IF;
    IF c > temp THEN
        temp := c;
    END_IF;
    Max3 := temp;
END_FUNCTION

(* 三个数的最小值 *)
FUNCTION Min3 : INT
VAR_INPUT
    a : INT;
    b : INT;
    c : INT;
END_VAR
VAR_LOCAL
    temp : INT;
END_VAR
    temp := a;
    IF b < temp THEN
        temp := b;
    END_IF;
    IF c < temp THEN
        temp := c;
    END_IF;
    Min3 := temp;
END_FUNCTION

(* 整型绝对值 *)
FUNCTION Abs : INT
VAR_INPUT
    x : INT;
END_VAR
    IF x < 0 THEN
        Abs := 0 - x;
    ELSE
        Abs := x;
    END_IF;
END_FUNCTION

(* 整型符号函数: 返回 -1, 0, 或 1 *)
FUNCTION Sign : INT
VAR_INPUT
    x : INT;
END_VAR
    Sign := 0;
    IF x < 0 THEN
        Sign := 0 - 1;
    END_IF;
    IF x > 0 THEN
        Sign := 1;
    END_IF;
END_FUNCTION

(* 整型取反 *)
FUNCTION Negate : INT
VAR_INPUT
    x : INT;
END_VAR
    Negate := 0 - x;
END_FUNCTION

(* 整型平方 *)
FUNCTION Square : INT
VAR_INPUT
    x : INT;
END_VAR
    Square := x * x;
END_FUNCTION

(* 整型立方 *)
FUNCTION Cube : INT
VAR_INPUT
    x : INT;
END_VAR
    Cube := x * x * x;
END_FUNCTION

(* 整型乘方 base^exp 非负整数指数 *)
FUNCTION Power : INT
VAR_INPUT
    base : INT;
    exp : INT;
END_VAR
VAR_LOCAL
    result : INT;
    i : INT;
END_VAR
    result := 1;
    IF exp > 0 THEN
        FOR i := 1 TO exp DO
            result := result * base;
        END_FOR;
    END_IF;
    Power := result;
END_FUNCTION

(* 限制值在范围内 *)
FUNCTION Clamp : INT
VAR_INPUT
    value : INT;
    min_val : INT;
    max_val : INT;
END_VAR
    Clamp := value;
    IF value < min_val THEN
        Clamp := min_val;
    END_IF;
    IF value > max_val THEN
        Clamp := max_val;
    END_IF;
END_FUNCTION

(* 判断是否在范围内 *)
FUNCTION InRange : BOOL
VAR_INPUT
    value : INT;
    min_val : INT;
    max_val : INT;
END_VAR
    InRange := FALSE;
    IF value >= min_val THEN
        IF value <= max_val THEN
            InRange := TRUE;
        END_IF;
    END_IF;
END_FUNCTION

(* 判断是否为偶数 *)
FUNCTION IsEven : BOOL
VAR_INPUT
    x : INT;
END_VAR
VAR_LOCAL
    half : INT;
END_VAR
    half := x / 2;
    IsEven := FALSE;
    IF half * 2 = x THEN
        IsEven := TRUE;
    END_IF;
END_FUNCTION

(* 判断是否为奇数 *)
FUNCTION IsOdd : BOOL
VAR_INPUT
    x : INT;
END_VAR
VAR_LOCAL
    half : INT;
END_VAR
    half := x / 2;
    IsOdd := TRUE;
    IF half * 2 = x THEN
        IsOdd := FALSE;
    END_IF;
END_FUNCTION

(* 判断是否能被整除 *)
FUNCTION IsDivisible : BOOL
VAR_INPUT
    dividend : INT;
    divisor : INT;
END_VAR
VAR_LOCAL
    quotient : INT;
END_VAR
    IsDivisible := FALSE;
    IF divisor <> 0 THEN
        quotient := dividend / divisor;
        IF quotient * divisor = dividend THEN
            IsDivisible := TRUE;
        END_IF;
    END_IF;
END_FUNCTION

(* 阶乘 n! *)
FUNCTION Factorial : INT
VAR_INPUT
    n : INT;
END_VAR
VAR_LOCAL
    result : INT;
    i : INT;
END_VAR
    result := 1;
    IF n > 0 THEN
        FOR i := 2 TO n DO
            result := result * i;
        END_FOR;
    END_IF;
    Factorial := result;
END_FUNCTION

(* 最大公约数 欧几里得算法 *)
FUNCTION GCD : INT
VAR_INPUT
    a : INT;
    b : INT;
END_VAR
VAR_LOCAL
    temp : INT;
    x : INT;
    y : INT;
    counter : INT;
END_VAR
    x := a;
    y := b;
    IF x < 0 THEN
        x := 0 - x;
    END_IF;
    IF y < 0 THEN
        y := 0 - y;
    END_IF;
    counter := 0;
    FOR counter := 1 TO 1000 DO
        IF y = 0 THEN
            GCD := x;
            RETURN;
        END_IF;
        temp := y;
        y := x MOD y;
        x := temp;
    END_FOR;
    GCD := x;
END_FUNCTION

(* 最小公倍数 *)
FUNCTION LCM : INT
VAR_INPUT
    a : INT;
    b : INT;
END_VAR
VAR_LOCAL
    gcd_val : INT;
    abs_a : INT;
    abs_b : INT;
END_VAR
    abs_a := a;
    abs_b := b;
    IF abs_a < 0 THEN
        abs_a := 0 - abs_a;
    END_IF;
    IF abs_b < 0 THEN
        abs_b := 0 - abs_b;
    END_IF;
    gcd_val := GCD(abs_a, abs_b);
    IF gcd_val <> 0 THEN
        LCM := (abs_a / gcd_val) * abs_b;
    ELSE
        LCM := 0;
    END_IF;
END_FUNCTION

(* 斐波那契数列第n项 *)
FUNCTION Fibonacci : INT
VAR_INPUT
    n : INT;
END_VAR
VAR_LOCAL
    a : INT;
    b : INT;
    temp : INT;
    i : INT;
END_VAR
    IF n <= 0 THEN
        Fibonacci := 0;
        RETURN;
    END_IF;
    IF n = 1 THEN
        Fibonacci := 1;
        RETURN;
    END_IF;
    a := 0;
    b := 1;
    FOR i := 2 TO n DO
        temp := a + b;
        a := b;
        b := temp;
    END_FOR;
    Fibonacci := b;
END_FUNCTION

(* 实数最大值 *)
FUNCTION MaxReal : REAL
VAR_INPUT
    a : REAL;
    b : REAL;
END_VAR
    IF a > b THEN
        MaxReal := a;
    ELSE
        MaxReal := b;
    END_IF;
END_FUNCTION

(* 实数最小值 *)
FUNCTION MinReal : REAL
VAR_INPUT
    a : REAL;
    b : REAL;
END_VAR
    IF a < b THEN
        MinReal := a;
    ELSE
        MinReal := b;
    END_IF;
END_FUNCTION

(* 三个实数的最大值 *)
FUNCTION MaxReal3 : REAL
VAR_INPUT
    a : REAL;
    b : REAL;
    c : REAL;
END_VAR
VAR_LOCAL
    temp : REAL;
END_VAR
    temp := a;
    IF b > temp THEN
        temp := b;
    END_IF;
    IF c > temp THEN
        temp := c;
    END_IF;
    MaxReal3 := temp;
END_FUNCTION

(* 三个实数的最小值 *)
FUNCTION MinReal3 : REAL
VAR_INPUT
    a : REAL;
    b : REAL;
    c : REAL;
END_VAR
VAR_LOCAL
    temp : REAL;
END_VAR
    temp := a;
    IF b < temp THEN
        temp := b;
    END_IF;
    IF c < temp THEN
        temp := c;
    END_IF;
    MinReal3 := temp;
END_FUNCTION

(* 实数绝对值 *)
FUNCTION AbsReal : REAL
VAR_INPUT
    x : REAL;
END_VAR
    IF x < 0.0 THEN
        AbsReal := 0.0 - x;
    ELSE
        AbsReal := x;
    END_IF;
END_FUNCTION

(* 实数符号函数 *)
FUNCTION SignReal : INT
VAR_INPUT
    x : REAL;
END_VAR
    SignReal := 0;
    IF x < 0.0 THEN
        SignReal := 0 - 1;
    END_IF;
    IF x > 0.0 THEN
        SignReal := 1;
    END_IF;
END_FUNCTION

(* 实数取反 *)
FUNCTION NegateReal : REAL
VAR_INPUT
    x : REAL;
END_VAR
    NegateReal := 0.0 - x;
END_FUNCTION

(* 实数限制在范围内 *)
FUNCTION ClampReal : REAL
VAR_INPUT
    value : REAL;
    min_val : REAL;
    max_val : REAL;
END_VAR
    ClampReal := value;
    IF value < min_val THEN
        ClampReal := min_val;
    END_IF;
    IF value > max_val THEN
        ClampReal := max_val;
    END_IF;
END_FUNCTION

(* 实数范围判断 *)
FUNCTION InRangeReal : BOOL
VAR_INPUT
    value : REAL;
    min_val : REAL;
    max_val : REAL;
END_VAR
    InRangeReal := FALSE;
    IF value >= min_val THEN
        IF value <= max_val THEN
            InRangeReal := TRUE;
        END_IF;
    END_IF;
END_FUNCTION

(* 实数平方 *)
FUNCTION SquareReal : REAL
VAR_INPUT
    x : REAL;
END_VAR
    SquareReal := x * x;
END_FUNCTION

(* 实数立方 *)
FUNCTION CubeReal : REAL
VAR_INPUT
    x : REAL;
END_VAR
    CubeReal := x * x * x;
END_FUNCTION

(* 实数整数次幂 *)
FUNCTION PowerReal : REAL
VAR_INPUT
    base : REAL;
    exp : INT;
END_VAR
VAR_LOCAL
    result : REAL;
    i : INT;
    abs_exp : INT;
END_VAR
    result := 1.0;
    abs_exp := exp;
    IF exp < 0 THEN
        abs_exp := 0 - exp;
    END_IF;
    FOR i := 1 TO abs_exp DO
        result := result * base;
    END_FOR;
    IF exp < 0 THEN
        IF result <> 0.0 THEN
            result := 1.0 / result;
        ELSE
            result := 0.0;
        END_IF;
    END_IF;
    PowerReal := result;
END_FUNCTION

(* 线性插值 *)
FUNCTION Lerp : REAL
VAR_INPUT
    start_val : REAL;
    end_val : REAL;
    t : REAL;
END_VAR
VAR_LOCAL
    clamped_t : REAL;
END_VAR
    clamped_t := t;
    IF t < 0.0 THEN
        clamped_t := 0.0;
    END_IF;
    IF t > 1.0 THEN
        clamped_t := 1.0;
    END_IF;
    Lerp := start_val + (end_val - start_val) * clamped_t;
END_FUNCTION

(* 无限制线性插值 *)
FUNCTION LerpUnclamped : REAL
VAR_INPUT
    start_val : REAL;
    end_val : REAL;
    t : REAL;
END_VAR
    LerpUnclamped := start_val + (end_val - start_val) * t;
END_FUNCTION

(* 映射值从一个范围到另一个范围 *)
FUNCTION MapRange : REAL
VAR_INPUT
    value : REAL;
    in_min : REAL;
    in_max : REAL;
    out_min : REAL;
    out_max : REAL;
END_VAR
VAR_LOCAL
    in_range : REAL;
    out_range : REAL;
    ratio : REAL;
END_VAR
    in_range := in_max - in_min;
    out_range := out_max - out_min;
    IF in_range <> 0.0 THEN
        ratio := (value - in_min) / in_range;
        MapRange := out_min + ratio * out_range;
    ELSE
        MapRange := out_min;
    END_IF;
END_FUNCTION

(* 反向插值 获取t值 *)
FUNCTION InverseLerp : REAL
VAR_INPUT
    start_val : REAL;
    end_val : REAL;
    value : REAL;
END_VAR
VAR_LOCAL
    range : REAL;
END_VAR
    range := end_val - start_val;
    IF range <> 0.0 THEN
        InverseLerp := (value - start_val) / range;
    ELSE
        InverseLerp := 0.0;
    END_IF;
END_FUNCTION

(* 近似相等 指定容差 *)
FUNCTION ApproxEqual : BOOL
VAR_INPUT
    a : REAL;
    b : REAL;
    epsilon : REAL;
END_VAR
VAR_LOCAL
    diff : REAL;
END_VAR
    diff := a - b;
    IF diff < 0.0 THEN
        diff := 0.0 - diff;
    END_IF;
    ApproxEqual := FALSE;
    IF diff <= epsilon THEN
        ApproxEqual := TRUE;
    END_IF;
END_FUNCTION

(* 近似为零 *)
FUNCTION ApproxZero : BOOL
VAR_INPUT
    x : REAL;
    epsilon : REAL;
END_VAR
VAR_LOCAL
    abs_x : REAL;
END_VAR
    abs_x := x;
    IF abs_x < 0.0 THEN
        abs_x := 0.0 - abs_x;
    END_IF;
    ApproxZero := FALSE;
    IF abs_x <= epsilon THEN
        ApproxZero := TRUE;
    END_IF;
END_FUNCTION

(* 算术平均值 *)
FUNCTION Average : REAL
VAR_INPUT
    a : REAL;
    b : REAL;
END_VAR
    Average := (a + b) / 2.0;
END_FUNCTION

(* 三个数的算术平均值 *)
FUNCTION Average3 : REAL
VAR_INPUT
    a : REAL;
    b : REAL;
    c : REAL;
END_VAR
    Average3 := (a + b + c) / 3.0;
END_FUNCTION

(* 加权平均值 *)
FUNCTION WeightedAverage : REAL
VAR_INPUT
    a : REAL;
    b : REAL;
    weight_a : REAL;
    weight_b : REAL;
END_VAR
VAR_LOCAL
    total_weight : REAL;
END_VAR
    total_weight := weight_a + weight_b;
    IF total_weight <> 0.0 THEN
        WeightedAverage := (a * weight_a + b * weight_b) / total_weight;
    ELSE
        WeightedAverage := 0.0;
    END_IF;
END_FUNCTION

(* 计算百分比 *)
FUNCTION Percentage : REAL
VAR_INPUT
    part : REAL;
    whole : REAL;
END_VAR
    IF whole <> 0.0 THEN
        Percentage := (part / whole) * 100.0;
    ELSE
        Percentage := 0.0;
    END_IF;
END_FUNCTION

(* 从百分比计算值 *)
FUNCTION FromPercentage : REAL
VAR_INPUT
    percent : REAL;
    whole : REAL;
END_VAR
    FromPercentage := (percent * whole) / 100.0;
END_FUNCTION

(* 百分比变化 *)
FUNCTION PercentageChange : REAL
VAR_INPUT
    old_value : REAL;
    new_value : REAL;
END_VAR
    IF old_value <> 0.0 THEN
        PercentageChange := ((new_value - old_value) / old_value) * 100.0;
    ELSE
        PercentageChange := 0.0;
    END_IF;
END_FUNCTION

END_PROGRAM
