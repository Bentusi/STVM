(* 函数块示例 - 状态机控制器 *)

FUNCTION_BLOCK StateMachineController
VAR_INPUT
    start : BOOL;
    stop : BOOL;
    reset : BOOL;
    sensor_input : INT;
END_VAR
VAR_OUTPUT
    state : INT;
    output_value : INT;
    error : BOOL;
END_VAR
VAR
    internal_counter : INT;
    last_state : INT;
END_VAR
    (* 保存上一个状态 *)
    last_state := state;
    
    (* 状态机逻辑 *)
    CASE state OF
        0: (* 初始状态 *)
            output_value := 0;
            error := FALSE;
            internal_counter := 0;
            
            IF start THEN
                state := 1;
            END_IF
            
        1: (* 运行状态 *)
            output_value := sensor_input * 2;
            internal_counter := internal_counter + 1;
            
            IF stop THEN
                state := 2;
            ELSIF internal_counter > 100 THEN
                state := 3;  (* 超时状态 *)
                error := TRUE;
            END_IF
            
        2: (* 停止状态 *)
            output_value := 0;
            
            IF start THEN
                state := 1;
            ELSIF reset THEN
                state := 0;
            END_IF
            
        3: (* 错误状态 *)
            output_value := -1;
            error := TRUE;
            
            IF reset THEN
                state := 0;
            END_IF
    END_CASE
END_FUNCTION_BLOCK