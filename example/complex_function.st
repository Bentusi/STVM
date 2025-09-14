(* 复杂的函数定义示例 - 带有输入、输出和输入输出参数 *)

FUNCTION CalculateStatistics : BOOL
VAR_INPUT
    values : INT;        (* 数值数组的起始值 *)
    count : INT;         (* 数值的个数 *)
    threshold : INT;     (* 阈值 *)
END_VAR
VAR_OUTPUT
    sum : INT;           (* 总和 *)
    average : REAL;      (* 平均值 *)
    max_value : INT;     (* 最大值 *)
    min_value : INT;     (* 最小值 *)
END_VAR
VAR_IN_OUT
    error_code : INT;    (* 错误代码 *)
END_VAR
VAR
    i : INT;             (* 局部变量：循环计数器 *)
    temp : REAL;         (* 局部变量：临时值 *)
END_VAR
    (* 初始化输出参数 *)
    sum := 0;
    max_value := values;
    min_value := values;
    
    (* 验证输入参数 *)
    IF count <= 0 THEN
        error_code := 1;  (* 无效的计数 *)
        RETURN FALSE;
    END_IF
    
    (* 计算统计数据 *)
    FOR i := 0 TO count - 1 DO
        sum := sum + values + i;  (* 模拟数组访问 *)
        
        IF values + i > max_value THEN
            max_value := values + i;
        END_IF
        
        IF values + i < min_value THEN
            min_value := values + i;
        END_IF
    END_FOR
    
    (* 计算平均值 *)
    temp := sum;
    average := temp / count;
    
    (* 检查阈值 *)
    IF sum > threshold THEN
        error_code := 2;  (* 超过阈值 *)
        RETURN FALSE;
    END_IF
    
    error_code := 0;  (* 成功 *)
    RETURN TRUE;
END_FUNCTION