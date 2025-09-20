(* ============================================================ *)
(* 控制逻辑函数实现                                             *)
(* ============================================================ *)
(* ============================================================ *)
(* IEC 61131-3 Structured Text 完整测试程序                    *)
(* 包含数学函数库、控制逻辑、RS触发器、滤波器等实现             *)
(* 版本: 2.0                                                   *)
(* 限制: 不支持数组、位运算、VAR_INPUT、OR/AND逻辑运算符        *)
(* ============================================================ *)

(* ============================================================ *)
(* 基础数学函数实现                                             *)
(* ============================================================ *)

(* 绝对值函数 - 整数版本 *)
FUNCTION ABS_INT(input_val : INT):INT
    IF input_val >= 0 THEN
        RETURN input_val;
    ELSE
        RETURN -input_val;
    END_IF
END_FUNCTION

(* 绝对值函数 - 实数版本 *)
FUNCTION ABS_REAL(input_val : REAL):REAL
    IF input_val >= 0.0 THEN
        RETURN input_val;
    ELSE
        RETURN -input_val;
    END_IF
END_FUNCTION

(* 最大值函数 - 整数版本 *)
FUNCTION MAX_INT
(
    val1 : INT,
    val2 : INT
) : INT
    IF val1 > val2 THEN
        RETURN val1;
    ELSE
        RETURN val2;
    END_IF
END_FUNCTION

(* 最小值函数 - 整数版本 *)
FUNCTION MIN_INT
(
    val1 : INT,
    val2 : INT
) : INT
    IF val1 < val2 THEN
        RETURN val1;
    ELSE
        RETURN val2;
    END_IF
END_FUNCTION

(* 最大值函数 - 实数版本 *)
FUNCTION MAX_REAL
(
    val1 : REAL,
    val2 : REAL
) : REAL
    IF val1 > val2 THEN
        RETURN val1;
    ELSE
        RETURN val2;
    END_IF
END_FUNCTION

(* 最小值函数 - 实数版本 *)
FUNCTION MIN_REAL
(
    val1 : REAL,
    val2 : REAL
) : REAL
    IF val1 < val2 THEN
        RETURN val1;
    ELSE
        RETURN val2;
    END_IF
END_FUNCTION

(* 符号函数 *)
FUNCTION SIGN 
(
    input_val : REAL
): INT
    IF input_val > 0.0 THEN
        RETURN 1;
    ELSE
        RETURN 0;
    END_IF
END_FUNCTION

(* 幂函数 - 整数指数版本 *)
FUNCTION POWER_INT
(
    base : REAL,
    exponent : INT,
    result : REAL,
    temp_exp : INT,
    i : INT
) : REAL
    result := 1.0;
    temp_exp := ABS_INT(exponent);
    
    FOR i := 1 TO temp_exp DO
        result := result * base;
    END_FOR
    
    IF exponent < 0 THEN
        result := 1.0 / result;
    END_IF

    RETURN result;
END_FUNCTION

(* 平方根函数 - 牛顿迭代法 *)
FUNCTION SQRT
(
    input_val : REAL,
    result : REAL,
    precision : REAL,
    old_result : REAL,
    iteration_count : INT,
    diff_val : REAL
) : REAL
    IF input_val < 0.0 THEN
        RETURN 0.0;
    END_IF
    
    IF input_val = 0.0 THEN
        RETURN 0.0;
    END_IF
    
    result := input_val / 2.0;
    precision := 0.000001;
    iteration_count := 0;
    
    WHILE iteration_count < 100 DO
        old_result := result;
        result := result + input_val / result;
        result := result / 2.0;
        
        diff_val := result - old_result;
        diff_val := ABS_REAL(diff_val);
        
        IF diff_val < precision THEN
            RETURN result;
        END_IF
        
        iteration_count := iteration_count + 1;
    END_WHILE

    RETURN result;
END_FUNCTION

(* 立方根函数 *)
FUNCTION CBRT
(
    input_val : REAL,
    result : REAL,
    precision : REAL,
    old_result : REAL,
    iteration_count : INT,
    sign_val : INT,
    abs_input : REAL,
    temp1 : REAL,
    temp2 : REAL,
    diff_val : REAL
) : REAL
    sign_val := SIGN(input_val);
    abs_input := ABS_REAL(input_val);
    
    IF abs_input = 0.0 THEN
        RETURN 0.0;
    END_IF
    
    result := abs_input / 3.0;
    precision := 0.000001;
    iteration_count := 0;
    
    WHILE iteration_count < 100 DO
        old_result := result;
        temp1 := result * result;
        temp2 := abs_input / temp1;
        result := 2.0 * result + temp2;
        result := result / 3.0;
        
        diff_val := result - old_result;
        diff_val := ABS_REAL(diff_val);
        
        IF diff_val < precision THEN
            RETURN result * sign_val;
        END_IF
        
        iteration_count := iteration_count + 1;
    END_WHILE
    
    RETURN result * sign_val;
END_FUNCTION

(* 自然指数函数 - 泰勒级数 *)
FUNCTION EXP 
(
    x : REAL,
    result : REAL,
    term : REAL,
    factorial_val : REAL,
    i : INT
): REAL
    result := 1.0;
    term := 1.0;
    factorial_val := 1.0;
    
    FOR i := 1 TO 20 DO
        factorial_val := factorial_val * i;
        term := term * x;
        result := result + term / factorial_val;
    END_FOR
    
    RETURN result;
END_FUNCTION

(* 自然对数函数 - 级数展开 *)
FUNCTION LN
(
    x : REAL,
    result : REAL,
    y : REAL,
    term : REAL,
    power : REAL,
    i : INT,
    temp1 : REAL,
    temp2 : REAL
): REAL
    IF x <= 0.0 THEN
        RETURN 0.0;
    END_IF
    
    IF x = 1.0 THEN
        RETURN 0.0;
    END_IF
    
    temp1 := x - 1.0;
    temp2 := x + 1.0;
    y := temp1 / temp2;
    result := 0.0;
    term := y;
    power := y;
    
    FOR i := 1 TO 20 DO
        temp1 := 2 * i - 1;
        result := result + term / temp1;
        power := power * y * y;
        term := power;
    END_FOR
    
    RETURN 2.0 * result;
END_FUNCTION

(* 常用对数函数 *)
FUNCTION LOG10
(
    x : REAL,
    ln_10 : REAL,
    ln_x : REAL
) : REAL
    ln_10 := 2.302585092994046;
    ln_x := LN(x);
    RETURN ln_x / ln_10;
END_FUNCTION

(* 以2为底的对数函数 *)
FUNCTION LOG2
(
    x : REAL,
    ln_2 : REAL,
    ln_x : REAL
) : REAL
    ln_2 := 0.6931471805599453;
    ln_x := LN(x);
    RETURN ln_x / ln_2;
END_FUNCTION

(* 实数幂函数 *)
FUNCTION POWER 
(
    base : REAL,
    exponent : REAL,
    ln_base : REAL,
    exp_val : REAL
): REAL
    IF base <= 0.0 THEN
        RETURN 0.0;
    ELSE
        ln_base := LN(base);
        exp_val := exponent * ln_base;
        RETURN EXP(exp_val);
    END_IF
END_FUNCTION

(* RS触发器实现 *)
FUNCTION RS_FLIP_FLOP
(
    set_input : BOOL,
    reset_input : BOOL,
    last_output : BOOL,
    output : BOOL
) : BOOL
    IF set_input = TRUE THEN
        IF reset_input = FALSE THEN
            output := TRUE;
        ELSE
            output := FALSE;
        END_IF
    ELSE
        output := last_output;
    END_IF
    
    RETURN output;
END_FUNCTION

(* SR触发器实现（与RS相反逻辑） *)
FUNCTION SR_FLIP_FLOP
(
    set_input : BOOL,
    reset_input : BOOL,
    last_output : BOOL,
    output : BOOL
) : BOOL
    IF reset_input = TRUE THEN
        IF set_input = FALSE THEN
            output := FALSE;
        ELSE
            output := TRUE;
        END_IF
    ELSE
        output := last_output;
    END_IF
    
    RETURN output;
END_FUNCTION

(* 2oo3逻辑（3选2表决逻辑） *)
FUNCTION VOTE_2oo3
(
    input1 : BOOL,
    input2 : BOOL,
    input3 : BOOL,
    count : INT,
    result : BOOL
): BOOL
    count := 0;
    
    IF input1 = TRUE THEN
        count := count + 1;
    END_IF
    
    IF input2 = TRUE THEN
        count := count + 1;
    END_IF
    
    IF input3 = TRUE THEN
        count := count + 1;
    END_IF
    
    IF count >= 2 THEN
        result := TRUE;
    ELSE
        result := FALSE;
    END_IF
    
    RETURN result;
END_FUNCTION

(* 2oo4逻辑（4选2表决逻辑） *)
FUNCTION VOTE_2oo4
(
    input1 : BOOL,
    input2 : BOOL,
    input3 : BOOL,
    input4 : BOOL,
    count : INT,
    result : BOOL
): BOOL
    count := 0;
    
    IF input1 = TRUE THEN
        count := count + 1;
    END_IF
    
    IF input2 = TRUE THEN
        count := count + 1;
    END_IF
    
    IF input3 = TRUE THEN
        count := count + 1;
    END_IF
    
    IF input4 = TRUE THEN
        count := count + 1;
    END_IF
    
    IF count >= 2 THEN
        result := TRUE;
    ELSE
        result := FALSE;
    END_IF
    
    RETURN result;
END_FUNCTION

(* 一阶低通滤波器 *)
FUNCTION LOW_PASS_FILTER
(
    input_val : REAL,
    filter_const : REAL,
    last_output : REAL,
    output : REAL,
    temp1 : REAL,
    temp2 : REAL
) : REAL
    temp1 := 1.0 - filter_const;
    temp2 := temp1 * last_output;
    output := filter_const * input_val + temp2;
    
    RETURN output;
END_FUNCTION

(* 滑动平均滤波器（5点） *)
FUNCTION MOVING_AVG_5(
    val1 : REAL,
    val2 : REAL,
    val3 : REAL,
    val4 : REAL,
    val5 : REAL,
    sum : REAL,
    result : REAL
) : REAL
    sum := val1 + val2;
    sum := sum + val3;
    sum := sum + val4;
    sum := sum + val5;
    result := sum / 5.0;
    
    RETURN result;
END_FUNCTION

(* 限幅器函数 *)
FUNCTION LIMITER
(
    input_val : REAL,
    min_limit : REAL,
    max_limit : REAL,
    output : REAL
) : REAL
    IF input_val < min_limit THEN
        output := min_limit;
    ELSE
        IF input_val > max_limit THEN
            output := max_limit;
        ELSE
            output := input_val;
        END_IF
    END_IF

    RETURN output;
END_FUNCTION

(* 死区函数 *)
FUNCTION DEAD_ZONE 
(
    input_val : REAL,
    dead_band : REAL,
    output : REAL,
    abs_input : REAL
): REAL
    abs_input := ABS_REAL(input_val);
    
    IF abs_input <= dead_band THEN
        output := 0.0;
    ELSE
        IF input_val > 0.0 THEN
            output := input_val - dead_band;
        ELSE
            output := input_val + dead_band;
        END_IF
    END_IF
    
    RETURN output;
END_FUNCTION

(* 滞回比较器 *)
FUNCTION HYSTERESIS
(
    input_val : REAL,
    upper_limit : REAL,
    lower_limit : REAL,
    last_output : BOOL,
    output : BOOL
) : BOOL
    IF last_output = FALSE THEN
        IF input_val >= upper_limit THEN
            output := TRUE;
        ELSE
            output := FALSE;
        END_IF
    ELSE
        IF input_val <= lower_limit THEN
            output := FALSE;
        ELSE
            output := TRUE;
        END_IF
    END_IF
    
    RETURN output;
END_FUNCTION

(* PID控制器简化版本 *)
FUNCTION PID_SIMPLE(
    setpoint : REAL,
    process_var : REAL,
    kp : REAL,
    ki : REAL,
    kd : REAL,
    last_error : REAL,
    integral : REAL,
    dt : REAL,
    error : REAL,
    derivative : REAL,
    output : REAL,
    prop_term : REAL,
    int_term : REAL,
    der_term : REAL
    ): REAL
    error := setpoint - process_var;
    
    prop_term := kp * error;
    
    integral := integral + error * dt;
    int_term := ki * integral;
    
    derivative := error - last_error;
    derivative := derivative / dt;
    der_term := kd * derivative;
    
    output := prop_term + int_term;
    output := output + der_term;
    
    RETURN output;
END_FUNCTION

(* 对上述控制函数进行测试
 * 主要测试函数调用和变量声明
 *)
PROGRAM test
VAR
    a : INT;
    b : INT;
    c : INT;
    d : REAL;
    e : REAL;
    f : REAL;
    g : REAL;
    h : REAL;
    i : REAL;
    j : REAL;
    k : REAL;
    m : REAL;
    n : REAL;
    o : REAL;
    p : REAL;
    q : REAL;
    r : REAL;
    s : REAL;
    t : REAL;
    u : REAL;
    v : REAL;
    w : REAL;
    x : REAL;
    y : REAL;
    z : REAL;
END_VAR

    a := ABS_INT(-10);
    b := MAX_INT(5, 8);
    c := MIN_INT(5, 8);
    
    d := ABS_REAL(-3.14);
    e := MAX_REAL(2.71, 3.14);
    f := MIN_REAL(2.71, 3.14);
    
    g := POWER_INT(2.0, 10);
    h := SQRT(16.0);
    i := CBRT(27.0);
    
    j := EXP(1.0);
    k := LN(2.718281828459045);
    m := LOG10(1000.0);
    n := LOG2(8.0);
    
    o := POWER(2.0, 3.0);
    
    (* RS触发器测试 *)
    p := RS_FLIP_FLOP(TRUE, FALSE, FALSE);
    p := RS_FLIP_FLOP(FALSE, TRUE, p);
    
    (* SR触发器测试 *)
    q := SR_FLIP_FLOP(TRUE, FALSE, FALSE);
    q := SR_FLIP_FLOP(FALSE, TRUE, q);
    
    (* 2oo3逻辑测试 *)
    r := VOTE_2oo3(TRUE, FALSE, TRUE);
    
    (* 2oo4逻辑测试 *)
    s := VOTE_2oo4(TRUE, FALSE, TRUE, FALSE);
    
    (* 一阶低通滤波器测试 *)
    t := LOW_PASS_FILTER(10.0, 0.1, 0.0);
    
    (* 滑动平均滤波器测试 *)
    u := MOVING_AVG_5(1.0, 2.0, 3.0, 4.0, 5.0);
    
    (* 限幅器测试 *)
    v := LIMITER(15.0, 0.0, 10.0);
    
    (* 死区函数测试 *)
    w := DEAD_ZONE(0.5, 1.0);
    w := DEAD_ZONE(2.0, 1.0);
    
    (* 滞回比较器测试 *)
    x := HYSTERESIS(5.0, 7.0, 3.0, FALSE);
    x := HYSTERESIS(8.0, 7.0, 3.0, x);
    x := HYSTERESIS(4.0, 7.0, 3.0, x);


END_PROGRAM