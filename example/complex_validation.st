(* 带有多个返回点的复杂函数 *)

FUNCTION ValidateAndProcess : INT
VAR_INPUT
    input_value : INT;
    mode : INT;
    enable : BOOL;
END_VAR
VAR_OUTPUT
    processed_value : INT;
    status : INT;
END_VAR
VAR
    temp_value : INT;
    multiplier : INT;
END_VAR
    (* 初始化 *)
    processed_value := 0;
    status := 0;
    
    (* 检查使能 *)
    IF NOT enable THEN
        status := 1;  (* 未使能 *)
        RETURN -1;
    END_IF
    
    (* 验证输入范围 *)
    IF input_value < 0 OR input_value > 1000 THEN
        status := 2;  (* 输入值超出范围 *)
        RETURN -2;
    END_IF
    
    (* 根据模式处理 *)
    CASE mode OF
        1: (* 线性模式 *)
            multiplier := 2;
            
        2: (* 平方模式 *)
            multiplier := input_value;
            
        3: (* 特殊模式 *)
            IF input_value MOD 2 = 0 THEN
                multiplier := 10;
            ELSE
                multiplier := 5;
            END_IF
            
        ELSE
            status := 3;  (* 无效模式 *)
            RETURN -3;
    END_CASE
    
    (* 执行处理 *)
    temp_value := input_value * multiplier;
    
    (* 检查溢出 *)
    IF temp_value > 32767 THEN
        status := 4;  (* 溢出 *)
        processed_value := 32767;
        RETURN -4;
    END_IF
    
    (* 成功处理 *)
    processed_value := temp_value;
    status := 0;
    RETURN processed_value;
END_FUNCTION