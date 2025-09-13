PROGRAM StateMachine
VAR
    current_state : INT;
    input_cmd : INT;
    output_status : INT;
    cycle_count : INT;
    error_flag : BOOL;
END_VAR

(* 状态定义：
   0 = STOP状态
   1 = RUN状态  
   2 = WAIT状态
*)

(* 初始化状态机 *)
current_state := 0;  (* 初始为STOP状态 *)
input_cmd := 1;      (* 模拟输入命令 *)
output_status := 0;
cycle_count := 0;
error_flag := FALSE;

(* 状态机主循环 - 执行5个周期进行测试 *)
FOR cycle_count := 1 TO 5 DO
    (* 根据周期模拟不同的输入命令 *)
    IF cycle_count = 1 THEN
        input_cmd := 1;  (* 第1周期：启动命令 *)
    END_IF
    
    IF cycle_count = 3 THEN
        input_cmd := 2;  (* 第3周期：等待命令 *)
    END_IF
    
    IF cycle_count = 5 THEN
        input_cmd := 3;  (* 第5周期：停止命令 *)
    END_IF
    
    (* 状态机核心：根据当前状态和输入处理状态转换 *)
    CASE current_state OF
        0: (* STOP状态处理 *)
           output_status := 100;  (* STOP状态输出码 *)
           CASE input_cmd OF
               1: current_state := 1;  (* 收到启动命令，转到RUN *)
               2: current_state := 0;  (* 停止状态下等待命令无效 *)
               3: current_state := 0;  (* 已经是停止状态 *)
           END_CASE
           
        1: (* RUN状态处理 *)
           output_status := 200;  (* RUN状态输出码 *)
           CASE input_cmd OF
               1: current_state := 1;  (* 已经在运行 *)
               2: current_state := 2;  (* 收到等待命令，转到WAIT *)
               3: current_state := 0;  (* 收到停止命令，转到STOP *)
           END_CASE
           
        2: (* WAIT状态处理 *)
           output_status := 300;  (* WAIT状态输出码 *)
           CASE input_cmd OF
               1: current_state := 1;  (* 收到启动命令，转到RUN *)
               2: current_state := 2;  (* 保持等待状态 *)
               3: current_state := 0;  (* 收到停止命令，转到STOP *)
           END_CASE
    END_CASE
    
    (* 清除输入命令，避免重复处理 *)
    input_cmd := 0;
END_FOR

(* 最终检查：确保状态机正确结束在STOP状态 *)
IF current_state = 0 THEN
    error_flag := FALSE;
ELSE
    error_flag := TRUE;
END_IF

END_PROGRAM