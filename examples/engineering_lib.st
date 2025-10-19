(* ========================================================================= *)
(* ST 工程计算库 - Engineering Calculation Library                          *)
(* 包含: RS触发器、平滑滤波、投票逻辑(1oo2/2oo3/3oo4)、限幅                  *)
(* ========================================================================= *)

PROGRAM EngineeringLib
VAR
    (* 库版本信息 *)
    LIB_VERSION : INT := 100;  (* 版本 1.0.0 *)
END_VAR

(* ========================================================================= *)
(* RS 触发器 (Reset-Set Flip-Flop)                                          *)
(* 功能: 基本的触发器逻辑，Set置位，Reset复位                                *)
(* 优先级: Reset 优先                                                        *)
(* ========================================================================= *)
FUNCTION RS_Trigger : BOOL
VAR_INPUT
    Set : BOOL;      (* 置位输入 *)
    Reset : BOOL;    (* 复位输入 *)
END_VAR
VAR
    rs_state : BOOL := FALSE;  (* 保持状态 - 全局变量 *)
END_VAR
VAR_LOCAL
    result : BOOL;
END_VAR
    (* Reset 优先逻辑 *)
    IF Reset THEN
        rs_state := FALSE;
    ELSIF Set THEN
        rs_state := TRUE;
    END_IF;
    
    result := rs_state;
    RS_Trigger := result;
END_FUNCTION

(* ========================================================================= *)
(* SR 触发器 (Set-Reset Flip-Flop)                                          *)
(* 功能: Set优先的触发器                                                     *)
(* 优先级: Set 优先                                                          *)
(* ========================================================================= *)
FUNCTION SR_Trigger : BOOL
VAR_INPUT
    Set : BOOL;      (* 置位输入 *)
    Reset : BOOL;    (* 复位输入 *)
END_VAR
VAR
    sr_state : BOOL := FALSE;  (* 保持状态 - 全局变量 *)
END_VAR
VAR_LOCAL
    result : BOOL;
END_VAR
    (* Set 优先逻辑 *)
    IF Set THEN
        sr_state := TRUE;
    ELSIF Reset THEN
        sr_state := FALSE;
    END_IF;
    
    result := sr_state;
    SR_Trigger := result;
END_FUNCTION

(* ========================================================================= *)
(* 平滑滤波器 (Smooth Filter / Moving Average)                              *)
(* 功能: 对输入信号进行平滑处理，减少噪声                                    *)
(* 算法: 一阶低通滤波 Y(n) = α*X(n) + (1-α)*Y(n-1)                          *)
(* ========================================================================= *)
FUNCTION SmoothFilter : REAL
VAR_INPUT
    input : REAL;       (* 当前输入值 *)
    alpha : REAL;       (* 平滑系数 0.0-1.0, 越大响应越快 *)
END_VAR
VAR
    filter_last_output : REAL := 0.0;  (* 上次输出值 - 全局变量 *)
    filter_initialized : BOOL := FALSE; (* 初始化标志 *)
END_VAR
VAR_LOCAL
    filtered : REAL;
    valid_alpha : REAL;
END_VAR
    (* 限制 alpha 范围 [0.0, 1.0] *)
    IF alpha < 0.0 THEN
        valid_alpha := 0.0;
    ELSIF alpha > 1.0 THEN
        valid_alpha := 1.0;
    ELSE
        valid_alpha := alpha;
    END_IF;
    
    (* 首次调用时直接使用输入值 *)
    IF NOT filter_initialized THEN
        filtered := input;
        filter_initialized := TRUE;
    ELSE
        (* 一阶低通滤波公式 *)
        filtered := valid_alpha * input + (1.0 - valid_alpha) * filter_last_output;
    END_IF;
    
    filter_last_output := filtered;
    SmoothFilter := filtered;
END_FUNCTION

(* ========================================================================= *)
(* 1oo2 投票逻辑 (1 out of 2)                                               *)
(* 功能: 两个输入中至少有一个为TRUE时输出TRUE                                *)
(* 应用: 冗余传感器容错，允许一个传感器故障                                   *)
(* ========================================================================= *)
FUNCTION Vote_1oo2 : BOOL
VAR_INPUT
    input1 : BOOL;   (* 输入通道 1 *)
    input2 : BOOL;   (* 输入通道 2 *)
END_VAR
VAR_LOCAL
    result : BOOL;
END_VAR
    (* 至少一个为 TRUE *)
    result := input1 OR input2;
    Vote_1oo2 := result;
END_FUNCTION

(* ========================================================================= *)
(* 2oo3 投票逻辑 (2 out of 3)                                               *)
(* 功能: 三个输入中至少有两个为TRUE时输出TRUE                                *)
(* 应用: 三重冗余系统，容忍一个传感器故障                                    *)
(* ========================================================================= *)
FUNCTION Vote_2oo3 : BOOL
VAR_INPUT
    input1 : BOOL;   (* 输入通道 1 *)
    input2 : BOOL;   (* 输入通道 2 *)
    input3 : BOOL;   (* 输入通道 3 *)
END_VAR
VAR_LOCAL
    count : INT;
    result : BOOL;
END_VAR
    (* 统计TRUE的数量 *)
    count := 0;
    IF input1 THEN
        count := count + 1;
    END_IF;
    IF input2 THEN
        count := count + 1;
    END_IF;
    IF input3 THEN
        count := count + 1;
    END_IF;
    
    (* 至少2个为TRUE *)
    result := count >= 2;
    Vote_2oo3 := result;
END_FUNCTION

(* ========================================================================= *)
(* 3oo4 投票逻辑 (3 out of 4)                                               *)
(* 功能: 四个输入中至少有三个为TRUE时输出TRUE                                *)
(* 应用: 四重冗余系统，容忍一个传感器故障                                    *)
(* ========================================================================= *)
FUNCTION Vote_3oo4 : BOOL
VAR_INPUT
    input1 : BOOL;   (* 输入通道 1 *)
    input2 : BOOL;   (* 输入通道 2 *)
    input3 : BOOL;   (* 输入通道 3 *)
    input4 : BOOL;   (* 输入通道 4 *)
END_VAR
VAR_LOCAL
    count : INT;
    result : BOOL;
END_VAR
    (* 统计TRUE的数量 *)
    count := 0;
    IF input1 THEN
        count := count + 1;
    END_IF;
    IF input2 THEN
        count := count + 1;
    END_IF;
    IF input3 THEN
        count := count + 1;
    END_IF;
    IF input4 THEN
        count := count + 1;
    END_IF;
    
    (* 至少3个为TRUE *)
    result := count >= 3;
    Vote_3oo4 := result;
END_FUNCTION

(* ========================================================================= *)
(* 限幅器 (Limiter / Clamp)                                                 *)
(* 功能: 将输入值限制在指定范围内                                            *)
(* ========================================================================= *)
FUNCTION Limiter : REAL
VAR_INPUT
    input : REAL;     (* 输入值 *)
    min_val : REAL;   (* 最小值 *)
    max_val : REAL;   (* 最大值 *)
END_VAR
VAR_LOCAL
    result : REAL;
    valid_min : REAL;
    valid_max : REAL;
END_VAR
    (* 确保 min <= max *)
    IF min_val > max_val THEN
        valid_min := max_val;
        valid_max := min_val;
    ELSE
        valid_min := min_val;
        valid_max := max_val;
    END_IF;
    
    (* 限幅逻辑 *)
    result := input;
    IF input < valid_min THEN
        result := valid_min;
    ELSIF input > valid_max THEN
        result := valid_max;
    END_IF;
    
    Limiter := result;
END_FUNCTION

(* ========================================================================= *)
(* 死区处理器 (Dead Band / Dead Zone)                                       *)
(* 功能: 在阈值范围内的输入信号输出为0，避免小信号抖动                       *)
(* ========================================================================= *)
FUNCTION DeadBand : REAL
VAR_INPUT
    input : REAL;      (* 输入值 *)
    threshold : REAL;  (* 死区阈值(正值) *)
END_VAR
VAR_LOCAL
    result : REAL;
    abs_threshold : REAL;
END_VAR
    (* 确保阈值为正 *)
    IF threshold < 0.0 THEN
        abs_threshold := -threshold;
    ELSE
        abs_threshold := threshold;
    END_IF;
    
    (* 死区处理 *)
    IF input > abs_threshold THEN
        result := input - abs_threshold;
    ELSIF input < -abs_threshold THEN
        result := input + abs_threshold;
    ELSE
        result := 0.0;
    END_IF;
    
    DeadBand := result;
END_FUNCTION

(* ========================================================================= *)
(* 滞环比较器 (Hysteresis Comparator / Schmitt Trigger)                    *)
(* 功能: 带滞环的比较器，防止临界点抖动                                      *)
(* ========================================================================= *)
FUNCTION Hysteresis : BOOL
VAR_INPUT
    input : REAL;      (* 输入值 *)
    high_th : REAL;    (* 高阈值(上升沿) *)
    low_th : REAL;     (* 低阈值(下降沿) *)
END_VAR
VAR
    hysteresis_state : BOOL := FALSE;  (* 输出状态 - 全局变量 *)
END_VAR
VAR_LOCAL
    result : BOOL;
END_VAR
    (* 滞环逻辑 *)
    IF input >= high_th THEN
        hysteresis_state := TRUE;
    ELSIF input <= low_th THEN
        hysteresis_state := FALSE;
    END_IF;
    (* 否则保持原状态 *)
    
    result := hysteresis_state;
    Hysteresis := result;
END_FUNCTION

    (* 主程序：库加载成功提示 *)
    PRINT('===========================================\n');
    PRINT('Engineering Library v1.0.0 Loaded\n');
    PRINT('===========================================\n');
    PRINT('Available Functions:\n');
    PRINT('  - RS_Trigger: RS Flip-Flop\n');
    PRINT('  - SR_Trigger: SR Flip-Flop\n');
    PRINT('  - SmoothFilter: Smooth Filter\n');
    PRINT('  - Vote_1oo2: 1 out of 2 Voting\n');
    PRINT('  - Vote_2oo3: 2 out of 3 Voting\n');
    PRINT('  - Vote_3oo4: 3 out of 4 Voting\n');
    PRINT('  - Limiter: Signal Limiter\n');
    PRINT('  - DeadBand: Dead Band Filter\n');
    PRINT('  - Hysteresis: Hysteresis Comparator\n');
    PRINT('===========================================\n');

END_PROGRAM
