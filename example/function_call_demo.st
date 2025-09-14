(* 主程序 - 演示函数调用 *)

PROGRAM ComplexFunctionDemo
VAR
    (* 基本变量 *)
    a, b, c : INT;
    result : INT;
    flag : BOOL;
    
    (* 统计函数相关变量 *)
    data_start : INT;
    data_count : INT;
    threshold_val : INT;
    sum_result : INT;
    avg_result : REAL;
    max_result : INT;
    min_result : INT;
    error_status : INT;
    
    (* 状态机控制器实例 *)
    (* controller : StateMachineController; *)
    
    (* 验证函数相关变量 *)
    input_val : INT;
    proc_mode : INT;
    enable_flag : BOOL;
    processed_val : INT;
    process_status : INT;
    validation_result : INT;
    
    (* 循环变量 *)
    i : INT;
END_VAR

(* 演示基本数学函数 *)
a := 10;
b := 25;
c := -15;

result := Max(a, b);       (* 最大值函数 *)
result := Min(a, b);       (* 最小值函数 *)
result := Abs(c);          (* 绝对值函数 *)
result := Power(2, 5);     (* 幂函数: 2^5 = 32 *)

(* 演示递归函数 *)
result := Factorial(5);    (* 计算 5! = 120 *)

(* 演示复杂统计函数 *)
data_start := 10;
data_count := 5;
threshold_val := 100;
error_status := 0;

flag := CalculateStatistics(
    values := data_start,
    count := data_count,
    threshold := threshold_val,
    sum => sum_result,
    average => avg_result,
    max_value => max_result,
    min_value => min_result,
    error_code := error_status
);

(* 演示验证和处理函数 *)
input_val := 50;
proc_mode := 2;
enable_flag := TRUE;

validation_result := ValidateAndProcess(
    input_value := input_val,
    mode := proc_mode,
    enable := enable_flag,
    processed_value => processed_val,
    status => process_status
);

(* 条件控制演示 *)
IF flag THEN
    result := sum_result + processed_val;
ELSE
    result := error_status * 100;
END_IF

(* 循环中使用函数 *)
result := 0;
FOR i := 1 TO 10 DO
    result := result + Power(i, 2);  (* 计算平方和 *)
END_FOR

END_PROGRAM