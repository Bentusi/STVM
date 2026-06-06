(*
 * ============================================================================
 * STVM 极限性能测试 - 核反应堆 2oo4 安全控制系统 [IEC 61508 SIL3]
 * ============================================================================
 * 
 * 架构:
 *   - 4 通道冗余传感器 (A/B/C/D)，每组 8 个传感器
 *   - 2oo4 表决逻辑 (4 取 2 触发)
 *   - 多重质量位传播与验证
 *   - PID 控制回路 x4 (温度/压力/流量/水位)
 *   - 安全联锁矩阵 (16x16)
 *   - 启动/运行/停机/紧急状态机
 *   - 报警管理 (16 类报警, 4 级优先级)
 *   - 所有循环带 @LOOP_MAX 标注
 * 
 * 规模: ~10000 行 ST 代码, ~500+ 变量, ~80+ 函数
 * ============================================================================
 *)

PROGRAM Reactor2oo4Control

(* ================================================================ *)
(* 全局配置常量 *)
(* ================================================================ *)
VAR
    (* 系统状态 *)
    SYS_IDLE        : INT := 0;
    SYS_STARTUP     : INT := 1;
    SYS_RUNNING     : INT := 2;
    SYS_SHUTDOWN    : INT := 3;
    SYS_EMERGENCY   : INT := 4;
    system_state    : INT := 0;
    
    (* 安全阈值 *)
    TEMP_TRIP       : REAL := 650.0;
    PRESS_TRIP      : REAL := 15.5;
    FLOW_MIN        : REAL := 100.0;
    LEVEL_MIN       : REAL := 2.0;
    LEVEL_MAX       : REAL := 8.0;
    
    (* 2oo4 表决配置 *)
    VOTE_THRESHOLD  : INT := 2;
    CHANNEL_FAULT   : INT := -1;
    CHANNEL_OK      : INT := 0;
    
    (* 全局计数器 *)
    scan_counter    : INT := 0;
    trip_counter    : INT := 0;
    alarm_counter   : INT := 0;
    fault_counter   : INT := 0;
    
    (* 通用临时变量 *)
    tmp_int         : INT;
    tmp_real        : REAL;
    tmp_bool        : BOOL;
    loop_idx        : INT;
    ch_idx          : INT;
    sn_idx          : INT;
END_VAR

(* ================================================================ *)
(* 通道传感器数据 (4 通道 x 8 传感器/通道 = 32 路) *)
(* ================================================================ *)

(* 通道传感器定义 (32路) *)
FUNCTION ReadChannelA_Sensor0 : QREAL
    ReadChannelA_Sensor0 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelA_Sensor0 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelA_Sensor0 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelA_Sensor1 : QREAL
    ReadChannelA_Sensor1 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelA_Sensor1 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelA_Sensor1 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelA_Sensor2 : QREAL
    ReadChannelA_Sensor2 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelA_Sensor2 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelA_Sensor2 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelA_Sensor3 : QREAL
    ReadChannelA_Sensor3 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelA_Sensor3 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelA_Sensor3 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelA_Sensor4 : QREAL
    ReadChannelA_Sensor4 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelA_Sensor4 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelA_Sensor4 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelA_Sensor5 : QREAL
    ReadChannelA_Sensor5 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelA_Sensor5 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelA_Sensor5 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelA_Sensor6 : QREAL
    ReadChannelA_Sensor6 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelA_Sensor6 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelA_Sensor6 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelA_Sensor7 : QREAL
    ReadChannelA_Sensor7 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelA_Sensor7 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelA_Sensor7 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelB_Sensor0 : QREAL
    ReadChannelB_Sensor0 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelB_Sensor0 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelB_Sensor0 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelB_Sensor1 : QREAL
    ReadChannelB_Sensor1 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelB_Sensor1 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelB_Sensor1 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelB_Sensor2 : QREAL
    ReadChannelB_Sensor2 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelB_Sensor2 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelB_Sensor2 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelB_Sensor3 : QREAL
    ReadChannelB_Sensor3 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelB_Sensor3 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelB_Sensor3 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelB_Sensor4 : QREAL
    ReadChannelB_Sensor4 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelB_Sensor4 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelB_Sensor4 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelB_Sensor5 : QREAL
    ReadChannelB_Sensor5 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelB_Sensor5 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelB_Sensor5 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelB_Sensor6 : QREAL
    ReadChannelB_Sensor6 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelB_Sensor6 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelB_Sensor6 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelB_Sensor7 : QREAL
    ReadChannelB_Sensor7 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelB_Sensor7 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelB_Sensor7 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelC_Sensor0 : QREAL
    ReadChannelC_Sensor0 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelC_Sensor0 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelC_Sensor0 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelC_Sensor1 : QREAL
    ReadChannelC_Sensor1 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelC_Sensor1 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelC_Sensor1 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelC_Sensor2 : QREAL
    ReadChannelC_Sensor2 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelC_Sensor2 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelC_Sensor2 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelC_Sensor3 : QREAL
    ReadChannelC_Sensor3 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelC_Sensor3 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelC_Sensor3 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelC_Sensor4 : QREAL
    ReadChannelC_Sensor4 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelC_Sensor4 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelC_Sensor4 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelC_Sensor5 : QREAL
    ReadChannelC_Sensor5 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelC_Sensor5 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelC_Sensor5 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelC_Sensor6 : QREAL
    ReadChannelC_Sensor6 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelC_Sensor6 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelC_Sensor6 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelC_Sensor7 : QREAL
    ReadChannelC_Sensor7 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelC_Sensor7 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelC_Sensor7 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelD_Sensor0 : QREAL
    ReadChannelD_Sensor0 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelD_Sensor0 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelD_Sensor0 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelD_Sensor1 : QREAL
    ReadChannelD_Sensor1 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelD_Sensor1 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelD_Sensor1 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelD_Sensor2 : QREAL
    ReadChannelD_Sensor2 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelD_Sensor2 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelD_Sensor2 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelD_Sensor3 : QREAL
    ReadChannelD_Sensor3 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelD_Sensor3 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelD_Sensor3 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelD_Sensor4 : QREAL
    ReadChannelD_Sensor4 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelD_Sensor4 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelD_Sensor4 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelD_Sensor5 : QREAL
    ReadChannelD_Sensor5 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelD_Sensor5 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelD_Sensor5 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelD_Sensor6 : QREAL
    ReadChannelD_Sensor6 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelD_Sensor6 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelD_Sensor6 := tmp_bool;
END_FUNCTION

FUNCTION ReadChannelD_Sensor7 : QREAL
    ReadChannelD_Sensor7 := MakeQReal(25.0, 0);
END_FUNCTION

FUNCTION ValidateChannelD_Sensor7 : BOOL
VAR_INPUT
    input_val : QREAL;
    min_limit : REAL;
    max_limit : REAL;
END_VAR
    tmp_real := ToReal(input_val);
    tmp_int := GetQuality(input_val);
    tmp_bool := FALSE;
    IF tmp_int = 0 THEN
        IF tmp_real >= min_limit THEN
            IF tmp_real <= max_limit THEN
                tmp_bool := TRUE;
            END_IF;
        END_IF;
    END_IF;
    ValidateChannelD_Sensor7 := tmp_bool;
END_FUNCTION

(* ================================================================ *)
(* 2oo4 表决逻辑 *)
(* ================================================================ *)

FUNCTION Vote2oo4_Bool : BOOL
VAR_INPUT
    ch_a : BOOL;
    ch_b : BOOL;
    ch_c : BOOL;
    ch_d : BOOL;
END_VAR
    tmp_int := 0;
    IF ch_a THEN tmp_int := tmp_int + 1; END_IF;
    IF ch_b THEN tmp_int := tmp_int + 1; END_IF;
    IF ch_c THEN tmp_int := tmp_int + 1; END_IF;
    IF ch_d THEN tmp_int := tmp_int + 1; END_IF;
    Vote2oo4_Bool := (tmp_int >= VOTE_THRESHOLD);
END_FUNCTION

FUNCTION Vote2oo4_Real : REAL
VAR_INPUT
    ch_a : REAL;
    ch_b : REAL;
    ch_c : REAL;
    ch_d : REAL;
END_VAR
    tmp_real := ch_a + ch_b + ch_c + ch_d;
    Vote2oo4_Real := tmp_real / 4.0;
END_FUNCTION

FUNCTION Vote2oo4_QReal : QREAL
VAR_INPUT
    ch_a : QREAL;
    ch_b : QREAL;
    ch_c : QREAL;
    ch_d : QREAL;
END_VAR
    tmp_real := ToReal(ch_a) + ToReal(ch_b) + ToReal(ch_c) + ToReal(ch_d);
    Vote2oo4_QReal := MakeQReal(tmp_real / 4.0, 0);
END_FUNCTION

(* ================================================================ *)
(* @LOOP_MAX(10000) - 传感器组 0 2oo4 表决 *)
(* ================================================================ *)
FUNCTION SensorGroup0_Vote : QREAL
VAR
    s_a : QREAL;
    s_b : QREAL;
    s_c : QREAL;
    s_d : QREAL;
    v_a : BOOL;
    v_b : BOOL;
    v_c : BOOL;
    v_d : BOOL;
    vt : BOOL;
    result : QREAL;
END_VAR
    s_a := ReadChannelA_Sensor0();
    s_b := ReadChannelB_Sensor0();
    s_c := ReadChannelC_Sensor0();
    s_d := ReadChannelD_Sensor0();
    
    v_a := ValidateChannelA_Sensor0(s_a, 0.0, 1000.0);
    v_b := ValidateChannelB_Sensor0(s_b, 0.0, 1000.0);
    v_c := ValidateChannelC_Sensor0(s_c, 0.0, 1000.0);
    v_d := ValidateChannelD_Sensor0(s_d, 0.0, 1000.0);
    
    vt := Vote2oo4_Bool(v_a, v_b, v_c, v_d);
    IF vt THEN
        result := Vote2oo4_QReal(s_a, s_b, s_c, s_d);
    ELSE
        result := MakeQReal(0.0, 3);
    END_IF;
    SensorGroup0_Vote := result;
END_FUNCTION

(* ================================================================ *)
(* @LOOP_MAX(10000) - 传感器组 1 2oo4 表决 *)
(* ================================================================ *)
FUNCTION SensorGroup1_Vote : QREAL
VAR
    s_a : QREAL;
    s_b : QREAL;
    s_c : QREAL;
    s_d : QREAL;
    v_a : BOOL;
    v_b : BOOL;
    v_c : BOOL;
    v_d : BOOL;
    vt : BOOL;
    result : QREAL;
END_VAR
    s_a := ReadChannelA_Sensor1();
    s_b := ReadChannelB_Sensor1();
    s_c := ReadChannelC_Sensor1();
    s_d := ReadChannelD_Sensor1();
    
    v_a := ValidateChannelA_Sensor1(s_a, 0.0, 1000.0);
    v_b := ValidateChannelB_Sensor1(s_b, 0.0, 1000.0);
    v_c := ValidateChannelC_Sensor1(s_c, 0.0, 1000.0);
    v_d := ValidateChannelD_Sensor1(s_d, 0.0, 1000.0);
    
    vt := Vote2oo4_Bool(v_a, v_b, v_c, v_d);
    IF vt THEN
        result := Vote2oo4_QReal(s_a, s_b, s_c, s_d);
    ELSE
        result := MakeQReal(0.0, 3);
    END_IF;
    SensorGroup1_Vote := result;
END_FUNCTION

(* ================================================================ *)
(* @LOOP_MAX(10000) - 传感器组 2 2oo4 表决 *)
(* ================================================================ *)
FUNCTION SensorGroup2_Vote : QREAL
VAR
    s_a : QREAL;
    s_b : QREAL;
    s_c : QREAL;
    s_d : QREAL;
    v_a : BOOL;
    v_b : BOOL;
    v_c : BOOL;
    v_d : BOOL;
    vt : BOOL;
    result : QREAL;
END_VAR
    s_a := ReadChannelA_Sensor2();
    s_b := ReadChannelB_Sensor2();
    s_c := ReadChannelC_Sensor2();
    s_d := ReadChannelD_Sensor2();
    
    v_a := ValidateChannelA_Sensor2(s_a, 0.0, 1000.0);
    v_b := ValidateChannelB_Sensor2(s_b, 0.0, 1000.0);
    v_c := ValidateChannelC_Sensor2(s_c, 0.0, 1000.0);
    v_d := ValidateChannelD_Sensor2(s_d, 0.0, 1000.0);
    
    vt := Vote2oo4_Bool(v_a, v_b, v_c, v_d);
    IF vt THEN
        result := Vote2oo4_QReal(s_a, s_b, s_c, s_d);
    ELSE
        result := MakeQReal(0.0, 3);
    END_IF;
    SensorGroup2_Vote := result;
END_FUNCTION

(* ================================================================ *)
(* @LOOP_MAX(10000) - 传感器组 3 2oo4 表决 *)
(* ================================================================ *)
FUNCTION SensorGroup3_Vote : QREAL
VAR
    s_a : QREAL;
    s_b : QREAL;
    s_c : QREAL;
    s_d : QREAL;
    v_a : BOOL;
    v_b : BOOL;
    v_c : BOOL;
    v_d : BOOL;
    vt : BOOL;
    result : QREAL;
END_VAR
    s_a := ReadChannelA_Sensor3();
    s_b := ReadChannelB_Sensor3();
    s_c := ReadChannelC_Sensor3();
    s_d := ReadChannelD_Sensor3();
    
    v_a := ValidateChannelA_Sensor3(s_a, 0.0, 1000.0);
    v_b := ValidateChannelB_Sensor3(s_b, 0.0, 1000.0);
    v_c := ValidateChannelC_Sensor3(s_c, 0.0, 1000.0);
    v_d := ValidateChannelD_Sensor3(s_d, 0.0, 1000.0);
    
    vt := Vote2oo4_Bool(v_a, v_b, v_c, v_d);
    IF vt THEN
        result := Vote2oo4_QReal(s_a, s_b, s_c, s_d);
    ELSE
        result := MakeQReal(0.0, 3);
    END_IF;
    SensorGroup3_Vote := result;
END_FUNCTION

(* ================================================================ *)
(* @LOOP_MAX(10000) - 传感器组 4 2oo4 表决 *)
(* ================================================================ *)
FUNCTION SensorGroup4_Vote : QREAL
VAR
    s_a : QREAL;
    s_b : QREAL;
    s_c : QREAL;
    s_d : QREAL;
    v_a : BOOL;
    v_b : BOOL;
    v_c : BOOL;
    v_d : BOOL;
    vt : BOOL;
    result : QREAL;
END_VAR
    s_a := ReadChannelA_Sensor4();
    s_b := ReadChannelB_Sensor4();
    s_c := ReadChannelC_Sensor4();
    s_d := ReadChannelD_Sensor4();
    
    v_a := ValidateChannelA_Sensor4(s_a, 0.0, 1000.0);
    v_b := ValidateChannelB_Sensor4(s_b, 0.0, 1000.0);
    v_c := ValidateChannelC_Sensor4(s_c, 0.0, 1000.0);
    v_d := ValidateChannelD_Sensor4(s_d, 0.0, 1000.0);
    
    vt := Vote2oo4_Bool(v_a, v_b, v_c, v_d);
    IF vt THEN
        result := Vote2oo4_QReal(s_a, s_b, s_c, s_d);
    ELSE
        result := MakeQReal(0.0, 3);
    END_IF;
    SensorGroup4_Vote := result;
END_FUNCTION

(* ================================================================ *)
(* @LOOP_MAX(10000) - 传感器组 5 2oo4 表决 *)
(* ================================================================ *)
FUNCTION SensorGroup5_Vote : QREAL
VAR
    s_a : QREAL;
    s_b : QREAL;
    s_c : QREAL;
    s_d : QREAL;
    v_a : BOOL;
    v_b : BOOL;
    v_c : BOOL;
    v_d : BOOL;
    vt : BOOL;
    result : QREAL;
END_VAR
    s_a := ReadChannelA_Sensor5();
    s_b := ReadChannelB_Sensor5();
    s_c := ReadChannelC_Sensor5();
    s_d := ReadChannelD_Sensor5();
    
    v_a := ValidateChannelA_Sensor5(s_a, 0.0, 1000.0);
    v_b := ValidateChannelB_Sensor5(s_b, 0.0, 1000.0);
    v_c := ValidateChannelC_Sensor5(s_c, 0.0, 1000.0);
    v_d := ValidateChannelD_Sensor5(s_d, 0.0, 1000.0);
    
    vt := Vote2oo4_Bool(v_a, v_b, v_c, v_d);
    IF vt THEN
        result := Vote2oo4_QReal(s_a, s_b, s_c, s_d);
    ELSE
        result := MakeQReal(0.0, 3);
    END_IF;
    SensorGroup5_Vote := result;
END_FUNCTION

(* ================================================================ *)
(* @LOOP_MAX(10000) - 传感器组 6 2oo4 表决 *)
(* ================================================================ *)
FUNCTION SensorGroup6_Vote : QREAL
VAR
    s_a : QREAL;
    s_b : QREAL;
    s_c : QREAL;
    s_d : QREAL;
    v_a : BOOL;
    v_b : BOOL;
    v_c : BOOL;
    v_d : BOOL;
    vt : BOOL;
    result : QREAL;
END_VAR
    s_a := ReadChannelA_Sensor6();
    s_b := ReadChannelB_Sensor6();
    s_c := ReadChannelC_Sensor6();
    s_d := ReadChannelD_Sensor6();
    
    v_a := ValidateChannelA_Sensor6(s_a, 0.0, 1000.0);
    v_b := ValidateChannelB_Sensor6(s_b, 0.0, 1000.0);
    v_c := ValidateChannelC_Sensor6(s_c, 0.0, 1000.0);
    v_d := ValidateChannelD_Sensor6(s_d, 0.0, 1000.0);
    
    vt := Vote2oo4_Bool(v_a, v_b, v_c, v_d);
    IF vt THEN
        result := Vote2oo4_QReal(s_a, s_b, s_c, s_d);
    ELSE
        result := MakeQReal(0.0, 3);
    END_IF;
    SensorGroup6_Vote := result;
END_FUNCTION

(* ================================================================ *)
(* @LOOP_MAX(10000) - 传感器组 7 2oo4 表决 *)
(* ================================================================ *)
FUNCTION SensorGroup7_Vote : QREAL
VAR
    s_a : QREAL;
    s_b : QREAL;
    s_c : QREAL;
    s_d : QREAL;
    v_a : BOOL;
    v_b : BOOL;
    v_c : BOOL;
    v_d : BOOL;
    vt : BOOL;
    result : QREAL;
END_VAR
    s_a := ReadChannelA_Sensor7();
    s_b := ReadChannelB_Sensor7();
    s_c := ReadChannelC_Sensor7();
    s_d := ReadChannelD_Sensor7();
    
    v_a := ValidateChannelA_Sensor7(s_a, 0.0, 1000.0);
    v_b := ValidateChannelB_Sensor7(s_b, 0.0, 1000.0);
    v_c := ValidateChannelC_Sensor7(s_c, 0.0, 1000.0);
    v_d := ValidateChannelD_Sensor7(s_d, 0.0, 1000.0);
    
    vt := Vote2oo4_Bool(v_a, v_b, v_c, v_d);
    IF vt THEN
        result := Vote2oo4_QReal(s_a, s_b, s_c, s_d);
    ELSE
        result := MakeQReal(0.0, 3);
    END_IF;
    SensorGroup7_Vote := result;
END_FUNCTION

(* ================================================================ *)
(* PID 控制器 x4 *)
(* ================================================================ *)

FUNCTION PID_Temperature : REAL
VAR_INPUT
    sp : REAL;
    pv : REAL;
END_VAR
VAR
    kp_temp : REAL := 2.5;
    ki_temp : REAL := 0.8;
    kd_temp : REAL := 0.3;
    err     : REAL;
    integ   : REAL;
    deriv   : REAL;
    prev_err: REAL;
END_VAR
    err := sp - pv;
    integ := integ + err * 0.1;
    deriv := (err - prev_err) / 0.1;
    prev_err := err;
    PID_Temperature := kp_temp * err + ki_temp * integ + kd_temp * deriv;
END_FUNCTION

FUNCTION PID_Pressure : REAL
VAR_INPUT
    sp : REAL;
    pv : REAL;
END_VAR
VAR
    kp_val : REAL := 3.0;
    ki_val : REAL := 1.2;
    kd_val : REAL := 0.5;
    err     : REAL;
    integ   : REAL;
    deriv   : REAL;
    prev_err: REAL;
END_VAR
    err := sp - pv;
    integ := integ + err * 0.1;
    deriv := (err - prev_err) / 0.1;
    prev_err := err;
    PID_Pressure := kp_val * err + ki_val * integ + kd_val * deriv;
END_FUNCTION
FUNCTION PID_Flow : REAL
VAR_INPUT
    sp : REAL;
    pv : REAL;
END_VAR
VAR
    kp_val : REAL := 1.5;
    ki_val : REAL := 0.6;
    kd_val : REAL := 0.2;
    err     : REAL;
    integ   : REAL;
    deriv   : REAL;
    prev_err: REAL;
END_VAR
    err := sp - pv;
    integ := integ + err * 0.1;
    deriv := (err - prev_err) / 0.1;
    prev_err := err;
    PID_Flow := kp_val * err + ki_val * integ + kd_val * deriv;
END_FUNCTION
FUNCTION PID_Level : REAL
VAR_INPUT
    sp : REAL;
    pv : REAL;
END_VAR
VAR
    kp_val : REAL := 2.0;
    ki_val : REAL := 0.3;
    kd_val : REAL := 0.1;
    err     : REAL;
    integ   : REAL;
    deriv   : REAL;
    prev_err: REAL;
END_VAR
    err := sp - pv;
    integ := integ + err * 0.1;
    deriv := (err - prev_err) / 0.1;
    prev_err := err;
    PID_Level := kp_val * err + ki_val * integ + kd_val * deriv;
END_FUNCTION
(* ================================================================ *)
(* 安全联锁矩阵 16x16 *)
(* ================================================================ *)

FUNCTION InterlockMatrix : BOOL
VAR_INPUT
    signals : ARRAY[16] OF BOOL;
END_VAR
VAR
    row : INT;
    col : INT;
    result : BOOL;
END_VAR
    result := FALSE;
    FOR row := 0 TO 15 DO
        FOR col := 0 TO 15 DO
            IF signals[row] THEN
                IF signals[col] THEN
                    result := TRUE;
                END_IF;
            END_IF;
        END_FOR;
    END_FOR;
    InterlockMatrix := result;
END_FUNCTION

(* 联锁信号赋值函数 *)
FUNCTION Interlock_R0 : BOOL
VAR_INPUT
    signals : ARRAY[16] OF BOOL;
END_VAR
    Interlock_R0 := signals[0] AND signals[0];
END_FUNCTION
FUNCTION Interlock_R1 : BOOL
VAR_INPUT
    signals : ARRAY[16] OF BOOL;
END_VAR
    Interlock_R1 := signals[1] AND signals[0];
END_FUNCTION
FUNCTION Interlock_R2 : BOOL
VAR_INPUT
    signals : ARRAY[16] OF BOOL;
END_VAR
    Interlock_R2 := signals[2] AND signals[0];
END_FUNCTION
FUNCTION Interlock_R3 : BOOL
VAR_INPUT
    signals : ARRAY[16] OF BOOL;
END_VAR
    Interlock_R3 := signals[3] AND signals[0];
END_FUNCTION
FUNCTION Interlock_R4 : BOOL
VAR_INPUT
    signals : ARRAY[16] OF BOOL;
END_VAR
    Interlock_R4 := signals[4] AND signals[0];
END_FUNCTION
FUNCTION Interlock_R5 : BOOL
VAR_INPUT
    signals : ARRAY[16] OF BOOL;
END_VAR
    Interlock_R5 := signals[5] AND signals[0];
END_FUNCTION
FUNCTION Interlock_R6 : BOOL
VAR_INPUT
    signals : ARRAY[16] OF BOOL;
END_VAR
    Interlock_R6 := signals[6] AND signals[0];
END_FUNCTION
FUNCTION Interlock_R7 : BOOL
VAR_INPUT
    signals : ARRAY[16] OF BOOL;
END_VAR
    Interlock_R7 := signals[7] AND signals[0];
END_FUNCTION
FUNCTION Interlock_R8 : BOOL
VAR_INPUT
    signals : ARRAY[16] OF BOOL;
END_VAR
    Interlock_R8 := signals[8] AND signals[0];
END_FUNCTION
FUNCTION Interlock_R9 : BOOL
VAR_INPUT
    signals : ARRAY[16] OF BOOL;
END_VAR
    Interlock_R9 := signals[9] AND signals[0];
END_FUNCTION
FUNCTION Interlock_R10 : BOOL
VAR_INPUT
    signals : ARRAY[16] OF BOOL;
END_VAR
    Interlock_R10 := signals[10] AND signals[0];
END_FUNCTION
FUNCTION Interlock_R11 : BOOL
VAR_INPUT
    signals : ARRAY[16] OF BOOL;
END_VAR
    Interlock_R11 := signals[11] AND signals[0];
END_FUNCTION
FUNCTION Interlock_R12 : BOOL
VAR_INPUT
    signals : ARRAY[16] OF BOOL;
END_VAR
    Interlock_R12 := signals[12] AND signals[0];
END_FUNCTION
FUNCTION Interlock_R13 : BOOL
VAR_INPUT
    signals : ARRAY[16] OF BOOL;
END_VAR
    Interlock_R13 := signals[13] AND signals[0];
END_FUNCTION
FUNCTION Interlock_R14 : BOOL
VAR_INPUT
    signals : ARRAY[16] OF BOOL;
END_VAR
    Interlock_R14 := signals[14] AND signals[0];
END_FUNCTION
FUNCTION Interlock_R15 : BOOL
VAR_INPUT
    signals : ARRAY[16] OF BOOL;
END_VAR
    Interlock_R15 := signals[15] AND signals[0];
END_FUNCTION
(* ================================================================ *)
(* 报警管理系统 *)
(* ================================================================ *)

FUNCTION AlarmManager : INT
VAR_INPUT
    alarm_signals : ARRAY[16] OF BOOL;
END_VAR
VAR
    alm_idx : INT;
    active_count : INT;
END_VAR
    active_count := 0;
    FOR alm_idx := 0 TO 15 DO
        IF alarm_signals[alm_idx] THEN
            active_count := active_count + 1;
        END_IF;
    END_FOR;
    AlarmManager := active_count;
END_FUNCTION

FUNCTION Alarm_P1_A0 : BOOL
VAR_INPUT
    trigger : BOOL;
END_VAR
    alarm_counter := alarm_counter + 1;
    Alarm_P1_A0 := trigger;
END_FUNCTION
FUNCTION Alarm_P1_A1 : BOOL
VAR_INPUT
    trigger : BOOL;
END_VAR
    alarm_counter := alarm_counter + 1;
    Alarm_P1_A1 := trigger;
END_FUNCTION
FUNCTION Alarm_P1_A2 : BOOL
VAR_INPUT
    trigger : BOOL;
END_VAR
    alarm_counter := alarm_counter + 1;
    Alarm_P1_A2 := trigger;
END_FUNCTION
FUNCTION Alarm_P1_A3 : BOOL
VAR_INPUT
    trigger : BOOL;
END_VAR
    alarm_counter := alarm_counter + 1;
    Alarm_P1_A3 := trigger;
END_FUNCTION
FUNCTION Alarm_P2_A0 : BOOL
VAR_INPUT
    trigger : BOOL;
END_VAR
    alarm_counter := alarm_counter + 1;
    Alarm_P2_A0 := trigger;
END_FUNCTION
FUNCTION Alarm_P2_A1 : BOOL
VAR_INPUT
    trigger : BOOL;
END_VAR
    alarm_counter := alarm_counter + 1;
    Alarm_P2_A1 := trigger;
END_FUNCTION
FUNCTION Alarm_P2_A2 : BOOL
VAR_INPUT
    trigger : BOOL;
END_VAR
    alarm_counter := alarm_counter + 1;
    Alarm_P2_A2 := trigger;
END_FUNCTION
FUNCTION Alarm_P2_A3 : BOOL
VAR_INPUT
    trigger : BOOL;
END_VAR
    alarm_counter := alarm_counter + 1;
    Alarm_P2_A3 := trigger;
END_FUNCTION
FUNCTION Alarm_P3_A0 : BOOL
VAR_INPUT
    trigger : BOOL;
END_VAR
    alarm_counter := alarm_counter + 1;
    Alarm_P3_A0 := trigger;
END_FUNCTION
FUNCTION Alarm_P3_A1 : BOOL
VAR_INPUT
    trigger : BOOL;
END_VAR
    alarm_counter := alarm_counter + 1;
    Alarm_P3_A1 := trigger;
END_FUNCTION
FUNCTION Alarm_P3_A2 : BOOL
VAR_INPUT
    trigger : BOOL;
END_VAR
    alarm_counter := alarm_counter + 1;
    Alarm_P3_A2 := trigger;
END_FUNCTION
FUNCTION Alarm_P3_A3 : BOOL
VAR_INPUT
    trigger : BOOL;
END_VAR
    alarm_counter := alarm_counter + 1;
    Alarm_P3_A3 := trigger;
END_FUNCTION
FUNCTION Alarm_P4_A0 : BOOL
VAR_INPUT
    trigger : BOOL;
END_VAR
    alarm_counter := alarm_counter + 1;
    Alarm_P4_A0 := trigger;
END_FUNCTION
FUNCTION Alarm_P4_A1 : BOOL
VAR_INPUT
    trigger : BOOL;
END_VAR
    alarm_counter := alarm_counter + 1;
    Alarm_P4_A1 := trigger;
END_FUNCTION
FUNCTION Alarm_P4_A2 : BOOL
VAR_INPUT
    trigger : BOOL;
END_VAR
    alarm_counter := alarm_counter + 1;
    Alarm_P4_A2 := trigger;
END_FUNCTION
FUNCTION Alarm_P4_A3 : BOOL
VAR_INPUT
    trigger : BOOL;
END_VAR
    alarm_counter := alarm_counter + 1;
    Alarm_P4_A3 := trigger;
END_FUNCTION
(* ================================================================ *)
(* 反应堆状态机 *)
(* ================================================================ *)

FUNCTION StateMachine : INT
VAR_INPUT
    trip : BOOL;
END_VAR
VAR
    next_state : INT;
END_VAR
    next_state := system_state;
    IF trip THEN
        next_state := SYS_EMERGENCY;
    ELSIF system_state = SYS_IDLE THEN
        next_state := SYS_STARTUP;
    ELSIF system_state = SYS_STARTUP THEN
        next_state := SYS_RUNNING;
    ELSIF system_state = SYS_RUNNING THEN
        next_state := SYS_RUNNING;
    ELSIF system_state = SYS_EMERGENCY THEN
        next_state := SYS_SHUTDOWN;
    ELSIF system_state = SYS_SHUTDOWN THEN
        next_state := SYS_IDLE;
    END_IF;
    StateMachine := next_state;
END_FUNCTION

(* ================================================================ *)
(* 主控制周期 - 包含所有子系统 *)
(* ================================================================ *)

FUNCTION ControlCycle
VAR
    s0 : QREAL;
    s1 : QREAL;
    s2 : QREAL;
    s3 : QREAL;
    s4 : QREAL;
    s5 : QREAL;
    s6 : QREAL;
    s7 : QREAL;
    trip_cond : BOOL;
    avg_temp : REAL;
    avg_press : REAL;
    pid_temp : REAL;
    pid_press : REAL;
    pid_flow : REAL;
    pid_level : REAL;
    alm_signals : ARRAY[16] OF BOOL;
    alm_idx : INT;
    alm_cnt : INT;
END_VAR

    (* 读取所有传感器 *)
    s0 := SensorGroup0_Vote();
    s1 := SensorGroup1_Vote();
    s2 := SensorGroup2_Vote();
    s3 := SensorGroup3_Vote();
    s4 := SensorGroup4_Vote();
    s5 := SensorGroup5_Vote();
    s6 := SensorGroup6_Vote();
    s7 := SensorGroup7_Vote();
    
    (* 质量检查 *)
    tmp_int := GetQuality(s0);
    IF tmp_int > 1 THEN fault_counter := fault_counter + 1; END_IF;
    
    (* 2oo4 表决汇总 *)
    avg_temp := (ToReal(s0) + ToReal(s1) + ToReal(s2) + ToReal(s3)) / 4.0;
    avg_press := (ToReal(s4) + ToReal(s5) + ToReal(s6) + ToReal(s7)) / 4.0;
    
    (* PID 计算 *)
    pid_temp := PID_Temperature(500.0, avg_temp);
    pid_press := PID_Pressure(10.0, avg_press);
    pid_flow := PID_Flow(200.0, avg_temp * 0.5);
    pid_level := PID_Level(5.0, avg_press * 0.3);
    
    (* 跳闸判断 *)
    trip_cond := FALSE;
    IF avg_temp > TEMP_TRIP THEN trip_cond := TRUE; END_IF;
    IF avg_press > PRESS_TRIP THEN trip_cond := TRUE; END_IF;
    IF trip_cond THEN trip_counter := trip_counter + 1; END_IF;
    
    (* 报警更新 *)
    FOR alm_idx := 0 TO 15 DO
        alm_signals[alm_idx] := FALSE;
    END_FOR;
    IF avg_temp > TEMP_TRIP THEN alm_signals[0] := TRUE; END_IF;
    IF avg_press > PRESS_TRIP THEN alm_signals[1] := TRUE; END_IF;
    alm_cnt := AlarmManager(alm_signals);
    alarm_counter := alarm_counter + alm_cnt;
    
    (* 状态机 *)
    system_state := StateMachine(trip_cond);
    
    scan_counter := scan_counter + 1;
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 1 *)
FUNCTION Verify_Channel_1 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_1 := 0;
    ELSE
        Verify_Channel_1 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_1 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.1;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_1 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_1 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_1 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 2 *)
FUNCTION Verify_Channel_2 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_2 := 0;
    ELSE
        Verify_Channel_2 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_2 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.2;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_2 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_2 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_2 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 3 *)
FUNCTION Verify_Channel_3 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_3 := 0;
    ELSE
        Verify_Channel_3 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_3 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.3;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_3 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_3 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_3 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 4 *)
FUNCTION Verify_Channel_4 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_4 := 0;
    ELSE
        Verify_Channel_4 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_4 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.4;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_4 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_4 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_4 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 5 *)
FUNCTION Verify_Channel_5 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_5 := 0;
    ELSE
        Verify_Channel_5 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_5 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.5;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_5 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_5 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_5 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 6 *)
FUNCTION Verify_Channel_6 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_6 := 0;
    ELSE
        Verify_Channel_6 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_6 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.6;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_6 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_6 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_6 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 7 *)
FUNCTION Verify_Channel_7 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_7 := 0;
    ELSE
        Verify_Channel_7 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_7 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.7;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_7 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_7 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_7 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 8 *)
FUNCTION Verify_Channel_8 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_8 := 0;
    ELSE
        Verify_Channel_8 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_8 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.8;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_8 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_8 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_8 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 9 *)
FUNCTION Verify_Channel_9 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_9 := 0;
    ELSE
        Verify_Channel_9 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_9 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.9;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_9 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_9 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_9 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 10 *)
FUNCTION Verify_Channel_10 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_10 := 0;
    ELSE
        Verify_Channel_10 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_10 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.10;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_10 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_10 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_10 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 11 *)
FUNCTION Verify_Channel_11 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_11 := 0;
    ELSE
        Verify_Channel_11 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_11 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.11;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_11 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_11 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_11 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 12 *)
FUNCTION Verify_Channel_12 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_12 := 0;
    ELSE
        Verify_Channel_12 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_12 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.12;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_12 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_12 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_12 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 13 *)
FUNCTION Verify_Channel_13 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_13 := 0;
    ELSE
        Verify_Channel_13 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_13 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.13;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_13 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_13 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_13 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 14 *)
FUNCTION Verify_Channel_14 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_14 := 0;
    ELSE
        Verify_Channel_14 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_14 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.14;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_14 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_14 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_14 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 15 *)
FUNCTION Verify_Channel_15 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_15 := 0;
    ELSE
        Verify_Channel_15 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_15 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.15;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_15 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_15 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_15 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 16 *)
FUNCTION Verify_Channel_16 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_16 := 0;
    ELSE
        Verify_Channel_16 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_16 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.16;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_16 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_16 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_16 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 17 *)
FUNCTION Verify_Channel_17 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_17 := 0;
    ELSE
        Verify_Channel_17 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_17 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.17;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_17 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_17 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_17 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 18 *)
FUNCTION Verify_Channel_18 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_18 := 0;
    ELSE
        Verify_Channel_18 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_18 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.18;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_18 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_18 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_18 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 19 *)
FUNCTION Verify_Channel_19 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_19 := 0;
    ELSE
        Verify_Channel_19 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_19 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.19;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_19 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_19 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_19 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 20 *)
FUNCTION Verify_Channel_20 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_20 := 0;
    ELSE
        Verify_Channel_20 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_20 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.20;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_20 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_20 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_20 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 21 *)
FUNCTION Verify_Channel_21 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_21 := 0;
    ELSE
        Verify_Channel_21 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_21 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.21;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_21 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_21 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_21 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 22 *)
FUNCTION Verify_Channel_22 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_22 := 0;
    ELSE
        Verify_Channel_22 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_22 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.22;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_22 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_22 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_22 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 23 *)
FUNCTION Verify_Channel_23 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_23 := 0;
    ELSE
        Verify_Channel_23 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_23 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.23;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_23 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_23 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_23 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 24 *)
FUNCTION Verify_Channel_24 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_24 := 0;
    ELSE
        Verify_Channel_24 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_24 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.24;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_24 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_24 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_24 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 25 *)
FUNCTION Verify_Channel_25 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_25 := 0;
    ELSE
        Verify_Channel_25 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_25 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.25;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_25 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_25 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_25 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 26 *)
FUNCTION Verify_Channel_26 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_26 := 0;
    ELSE
        Verify_Channel_26 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_26 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.26;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_26 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_26 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_26 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 27 *)
FUNCTION Verify_Channel_27 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_27 := 0;
    ELSE
        Verify_Channel_27 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_27 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.27;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_27 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_27 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_27 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 28 *)
FUNCTION Verify_Channel_28 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_28 := 0;
    ELSE
        Verify_Channel_28 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_28 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.28;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_28 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_28 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_28 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 29 *)
FUNCTION Verify_Channel_29 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_29 := 0;
    ELSE
        Verify_Channel_29 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_29 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.29;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_29 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_29 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_29 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 30 *)
FUNCTION Verify_Channel_30 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_30 := 0;
    ELSE
        Verify_Channel_30 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_30 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.30;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_30 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_30 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_30 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 31 *)
FUNCTION Verify_Channel_31 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_31 := 0;
    ELSE
        Verify_Channel_31 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_31 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.31;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_31 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_31 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_31 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 32 *)
FUNCTION Verify_Channel_32 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_32 := 0;
    ELSE
        Verify_Channel_32 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_32 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.32;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_32 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_32 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_32 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 33 *)
FUNCTION Verify_Channel_33 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_33 := 0;
    ELSE
        Verify_Channel_33 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_33 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.33;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_33 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_33 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_33 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 34 *)
FUNCTION Verify_Channel_34 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_34 := 0;
    ELSE
        Verify_Channel_34 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_34 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.34;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_34 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_34 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_34 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 35 *)
FUNCTION Verify_Channel_35 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_35 := 0;
    ELSE
        Verify_Channel_35 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_35 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.35;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_35 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_35 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_35 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 36 *)
FUNCTION Verify_Channel_36 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_36 := 0;
    ELSE
        Verify_Channel_36 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_36 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.36;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_36 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_36 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_36 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 37 *)
FUNCTION Verify_Channel_37 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_37 := 0;
    ELSE
        Verify_Channel_37 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_37 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.37;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_37 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_37 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_37 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 38 *)
FUNCTION Verify_Channel_38 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_38 := 0;
    ELSE
        Verify_Channel_38 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_38 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.38;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_38 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_38 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_38 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 39 *)
FUNCTION Verify_Channel_39 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_39 := 0;
    ELSE
        Verify_Channel_39 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_39 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.39;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_39 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_39 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_39 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 40 *)
FUNCTION Verify_Channel_40 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_40 := 0;
    ELSE
        Verify_Channel_40 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_40 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.40;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_40 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_40 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_40 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 41 *)
FUNCTION Verify_Channel_41 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_41 := 0;
    ELSE
        Verify_Channel_41 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_41 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.41;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_41 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_41 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_41 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 42 *)
FUNCTION Verify_Channel_42 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_42 := 0;
    ELSE
        Verify_Channel_42 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_42 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.42;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_42 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_42 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_42 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 43 *)
FUNCTION Verify_Channel_43 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_43 := 0;
    ELSE
        Verify_Channel_43 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_43 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.43;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_43 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_43 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_43 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 44 *)
FUNCTION Verify_Channel_44 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_44 := 0;
    ELSE
        Verify_Channel_44 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_44 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.44;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_44 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_44 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_44 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 45 *)
FUNCTION Verify_Channel_45 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_45 := 0;
    ELSE
        Verify_Channel_45 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_45 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.45;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_45 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_45 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_45 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 46 *)
FUNCTION Verify_Channel_46 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_46 := 0;
    ELSE
        Verify_Channel_46 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_46 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.46;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_46 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_46 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_46 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 47 *)
FUNCTION Verify_Channel_47 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_47 := 0;
    ELSE
        Verify_Channel_47 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_47 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.47;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_47 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_47 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_47 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 48 *)
FUNCTION Verify_Channel_48 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_48 := 0;
    ELSE
        Verify_Channel_48 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_48 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.48;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_48 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_48 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_48 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 49 *)
FUNCTION Verify_Channel_49 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_49 := 0;
    ELSE
        Verify_Channel_49 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_49 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.49;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_49 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_49 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_49 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 50 *)
FUNCTION Verify_Channel_50 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_50 := 0;
    ELSE
        Verify_Channel_50 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_50 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.50;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_50 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_50 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_50 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 51 *)
FUNCTION Verify_Channel_51 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_51 := 0;
    ELSE
        Verify_Channel_51 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_51 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.51;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_51 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_51 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_51 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 52 *)
FUNCTION Verify_Channel_52 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_52 := 0;
    ELSE
        Verify_Channel_52 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_52 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.52;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_52 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_52 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_52 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 53 *)
FUNCTION Verify_Channel_53 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_53 := 0;
    ELSE
        Verify_Channel_53 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_53 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.53;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_53 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_53 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_53 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 54 *)
FUNCTION Verify_Channel_54 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_54 := 0;
    ELSE
        Verify_Channel_54 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_54 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.54;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_54 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_54 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_54 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 55 *)
FUNCTION Verify_Channel_55 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_55 := 0;
    ELSE
        Verify_Channel_55 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_55 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.55;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_55 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_55 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_55 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 56 *)
FUNCTION Verify_Channel_56 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_56 := 0;
    ELSE
        Verify_Channel_56 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_56 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.56;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_56 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_56 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_56 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 57 *)
FUNCTION Verify_Channel_57 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_57 := 0;
    ELSE
        Verify_Channel_57 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_57 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.57;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_57 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_57 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_57 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 58 *)
FUNCTION Verify_Channel_58 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_58 := 0;
    ELSE
        Verify_Channel_58 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_58 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.58;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_58 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_58 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_58 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 59 *)
FUNCTION Verify_Channel_59 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_59 := 0;
    ELSE
        Verify_Channel_59 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_59 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.59;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_59 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_59 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_59 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 60 *)
FUNCTION Verify_Channel_60 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_60 := 0;
    ELSE
        Verify_Channel_60 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_60 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.60;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_60 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_60 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_60 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 61 *)
FUNCTION Verify_Channel_61 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_61 := 0;
    ELSE
        Verify_Channel_61 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_61 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.61;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_61 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_61 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_61 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 62 *)
FUNCTION Verify_Channel_62 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_62 := 0;
    ELSE
        Verify_Channel_62 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_62 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.62;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_62 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_62 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_62 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 63 *)
FUNCTION Verify_Channel_63 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_63 := 0;
    ELSE
        Verify_Channel_63 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_63 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.63;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_63 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_63 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_63 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 64 *)
FUNCTION Verify_Channel_64 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_64 := 0;
    ELSE
        Verify_Channel_64 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_64 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.64;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_64 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_64 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_64 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 65 *)
FUNCTION Verify_Channel_65 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_65 := 0;
    ELSE
        Verify_Channel_65 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_65 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.65;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_65 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_65 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_65 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 66 *)
FUNCTION Verify_Channel_66 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_66 := 0;
    ELSE
        Verify_Channel_66 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_66 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.66;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_66 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_66 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_66 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 67 *)
FUNCTION Verify_Channel_67 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_67 := 0;
    ELSE
        Verify_Channel_67 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_67 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.67;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_67 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_67 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_67 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 68 *)
FUNCTION Verify_Channel_68 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_68 := 0;
    ELSE
        Verify_Channel_68 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_68 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.68;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_68 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_68 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_68 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 69 *)
FUNCTION Verify_Channel_69 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_69 := 0;
    ELSE
        Verify_Channel_69 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_69 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.69;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_69 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_69 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_69 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 70 *)
FUNCTION Verify_Channel_70 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_70 := 0;
    ELSE
        Verify_Channel_70 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_70 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.70;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_70 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_70 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_70 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 71 *)
FUNCTION Verify_Channel_71 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_71 := 0;
    ELSE
        Verify_Channel_71 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_71 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.71;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_71 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_71 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_71 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 72 *)
FUNCTION Verify_Channel_72 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_72 := 0;
    ELSE
        Verify_Channel_72 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_72 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.72;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_72 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_72 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_72 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 73 *)
FUNCTION Verify_Channel_73 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_73 := 0;
    ELSE
        Verify_Channel_73 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_73 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.73;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_73 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_73 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_73 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 74 *)
FUNCTION Verify_Channel_74 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_74 := 0;
    ELSE
        Verify_Channel_74 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_74 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.74;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_74 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_74 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_74 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 75 *)
FUNCTION Verify_Channel_75 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_75 := 0;
    ELSE
        Verify_Channel_75 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_75 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.75;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_75 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_75 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_75 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 76 *)
FUNCTION Verify_Channel_76 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_76 := 0;
    ELSE
        Verify_Channel_76 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_76 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.76;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_76 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_76 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_76 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 77 *)
FUNCTION Verify_Channel_77 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_77 := 0;
    ELSE
        Verify_Channel_77 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_77 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.77;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_77 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_77 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_77 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 78 *)
FUNCTION Verify_Channel_78 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_78 := 0;
    ELSE
        Verify_Channel_78 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_78 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.78;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_78 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_78 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_78 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 79 *)
FUNCTION Verify_Channel_79 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_79 := 0;
    ELSE
        Verify_Channel_79 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_79 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.79;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_79 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_79 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_79 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 80 *)
FUNCTION Verify_Channel_80 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_80 := 0;
    ELSE
        Verify_Channel_80 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_80 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.80;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_80 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_80 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_80 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 81 *)
FUNCTION Verify_Channel_81 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_81 := 0;
    ELSE
        Verify_Channel_81 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_81 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.81;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_81 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_81 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_81 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 82 *)
FUNCTION Verify_Channel_82 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_82 := 0;
    ELSE
        Verify_Channel_82 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_82 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.82;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_82 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_82 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_82 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 83 *)
FUNCTION Verify_Channel_83 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_83 := 0;
    ELSE
        Verify_Channel_83 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_83 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.83;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_83 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_83 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_83 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 84 *)
FUNCTION Verify_Channel_84 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_84 := 0;
    ELSE
        Verify_Channel_84 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_84 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.84;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_84 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_84 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_84 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 85 *)
FUNCTION Verify_Channel_85 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_85 := 0;
    ELSE
        Verify_Channel_85 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_85 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.85;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_85 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_85 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_85 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 86 *)
FUNCTION Verify_Channel_86 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_86 := 0;
    ELSE
        Verify_Channel_86 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_86 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.86;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_86 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_86 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_86 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 87 *)
FUNCTION Verify_Channel_87 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_87 := 0;
    ELSE
        Verify_Channel_87 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_87 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.87;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_87 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_87 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_87 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 88 *)
FUNCTION Verify_Channel_88 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_88 := 0;
    ELSE
        Verify_Channel_88 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_88 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.88;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_88 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_88 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_88 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 89 *)
FUNCTION Verify_Channel_89 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_89 := 0;
    ELSE
        Verify_Channel_89 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_89 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.89;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_89 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_89 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_89 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 90 *)
FUNCTION Verify_Channel_90 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_90 := 0;
    ELSE
        Verify_Channel_90 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_90 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.90;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_90 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_90 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_90 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 91 *)
FUNCTION Verify_Channel_91 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_91 := 0;
    ELSE
        Verify_Channel_91 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_91 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.91;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_91 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_91 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_91 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 92 *)
FUNCTION Verify_Channel_92 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_92 := 0;
    ELSE
        Verify_Channel_92 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_92 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.92;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_92 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_92 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_92 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 93 *)
FUNCTION Verify_Channel_93 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_93 := 0;
    ELSE
        Verify_Channel_93 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_93 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.93;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_93 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_93 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_93 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 94 *)
FUNCTION Verify_Channel_94 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_94 := 0;
    ELSE
        Verify_Channel_94 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_94 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.94;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_94 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_94 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_94 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 95 *)
FUNCTION Verify_Channel_95 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_95 := 0;
    ELSE
        Verify_Channel_95 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_95 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.95;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_95 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_95 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_95 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 96 *)
FUNCTION Verify_Channel_96 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_96 := 0;
    ELSE
        Verify_Channel_96 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_96 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.96;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_96 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_96 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_96 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 97 *)
FUNCTION Verify_Channel_97 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_97 := 0;
    ELSE
        Verify_Channel_97 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_97 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.97;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_97 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_97 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_97 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 98 *)
FUNCTION Verify_Channel_98 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_98 := 0;
    ELSE
        Verify_Channel_98 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_98 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.98;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_98 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_98 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_98 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 99 *)
FUNCTION Verify_Channel_99 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_99 := 0;
    ELSE
        Verify_Channel_99 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_99 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.99;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_99 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_99 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_99 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 100 *)
FUNCTION Verify_Channel_100 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_100 := 0;
    ELSE
        Verify_Channel_100 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_100 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.100;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_100 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_100 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_100 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 101 *)
FUNCTION Verify_Channel_101 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_101 := 0;
    ELSE
        Verify_Channel_101 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_101 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.101;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_101 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_101 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_101 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 102 *)
FUNCTION Verify_Channel_102 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_102 := 0;
    ELSE
        Verify_Channel_102 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_102 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.102;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_102 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_102 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_102 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 103 *)
FUNCTION Verify_Channel_103 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_103 := 0;
    ELSE
        Verify_Channel_103 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_103 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.103;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_103 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_103 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_103 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 104 *)
FUNCTION Verify_Channel_104 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_104 := 0;
    ELSE
        Verify_Channel_104 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_104 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.104;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_104 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_104 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_104 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 105 *)
FUNCTION Verify_Channel_105 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_105 := 0;
    ELSE
        Verify_Channel_105 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_105 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.105;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_105 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_105 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_105 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 106 *)
FUNCTION Verify_Channel_106 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_106 := 0;
    ELSE
        Verify_Channel_106 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_106 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.106;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_106 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_106 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_106 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 107 *)
FUNCTION Verify_Channel_107 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_107 := 0;
    ELSE
        Verify_Channel_107 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_107 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.107;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_107 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_107 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_107 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 108 *)
FUNCTION Verify_Channel_108 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_108 := 0;
    ELSE
        Verify_Channel_108 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_108 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.108;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_108 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_108 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_108 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 109 *)
FUNCTION Verify_Channel_109 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_109 := 0;
    ELSE
        Verify_Channel_109 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_109 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.109;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_109 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_109 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_109 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 110 *)
FUNCTION Verify_Channel_110 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_110 := 0;
    ELSE
        Verify_Channel_110 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_110 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.110;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_110 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_110 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_110 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 111 *)
FUNCTION Verify_Channel_111 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_111 := 0;
    ELSE
        Verify_Channel_111 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_111 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.111;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_111 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_111 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_111 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 112 *)
FUNCTION Verify_Channel_112 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_112 := 0;
    ELSE
        Verify_Channel_112 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_112 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.112;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_112 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_112 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_112 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 113 *)
FUNCTION Verify_Channel_113 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_113 := 0;
    ELSE
        Verify_Channel_113 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_113 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.113;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_113 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_113 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_113 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 114 *)
FUNCTION Verify_Channel_114 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_114 := 0;
    ELSE
        Verify_Channel_114 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_114 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.114;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_114 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_114 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_114 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 115 *)
FUNCTION Verify_Channel_115 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_115 := 0;
    ELSE
        Verify_Channel_115 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_115 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.115;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_115 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_115 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_115 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 116 *)
FUNCTION Verify_Channel_116 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_116 := 0;
    ELSE
        Verify_Channel_116 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_116 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.116;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_116 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_116 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_116 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 117 *)
FUNCTION Verify_Channel_117 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_117 := 0;
    ELSE
        Verify_Channel_117 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_117 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.117;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_117 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_117 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_117 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 118 *)
FUNCTION Verify_Channel_118 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_118 := 0;
    ELSE
        Verify_Channel_118 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_118 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.118;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_118 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_118 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_118 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 119 *)
FUNCTION Verify_Channel_119 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_119 := 0;
    ELSE
        Verify_Channel_119 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_119 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.119;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_119 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_119 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_119 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 120 *)
FUNCTION Verify_Channel_120 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_120 := 0;
    ELSE
        Verify_Channel_120 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_120 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.120;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_120 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_120 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_120 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 121 *)
FUNCTION Verify_Channel_121 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_121 := 0;
    ELSE
        Verify_Channel_121 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_121 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.121;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_121 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_121 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_121 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 122 *)
FUNCTION Verify_Channel_122 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_122 := 0;
    ELSE
        Verify_Channel_122 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_122 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.122;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_122 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_122 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_122 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 123 *)
FUNCTION Verify_Channel_123 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_123 := 0;
    ELSE
        Verify_Channel_123 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_123 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.123;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_123 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_123 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_123 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 124 *)
FUNCTION Verify_Channel_124 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_124 := 0;
    ELSE
        Verify_Channel_124 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_124 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.124;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_124 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_124 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_124 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 125 *)
FUNCTION Verify_Channel_125 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_125 := 0;
    ELSE
        Verify_Channel_125 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_125 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.125;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_125 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_125 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_125 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 126 *)
FUNCTION Verify_Channel_126 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_126 := 0;
    ELSE
        Verify_Channel_126 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_126 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.126;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_126 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_126 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_126 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 127 *)
FUNCTION Verify_Channel_127 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_127 := 0;
    ELSE
        Verify_Channel_127 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_127 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.127;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_127 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_127 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_127 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 128 *)
FUNCTION Verify_Channel_128 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_128 := 0;
    ELSE
        Verify_Channel_128 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_128 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.128;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_128 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_128 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_128 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 129 *)
FUNCTION Verify_Channel_129 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_129 := 0;
    ELSE
        Verify_Channel_129 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_129 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.129;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_129 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_129 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_129 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 130 *)
FUNCTION Verify_Channel_130 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_130 := 0;
    ELSE
        Verify_Channel_130 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_130 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.130;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_130 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_130 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_130 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 131 *)
FUNCTION Verify_Channel_131 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_131 := 0;
    ELSE
        Verify_Channel_131 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_131 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.131;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_131 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_131 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_131 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 132 *)
FUNCTION Verify_Channel_132 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_132 := 0;
    ELSE
        Verify_Channel_132 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_132 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.132;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_132 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_132 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_132 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 133 *)
FUNCTION Verify_Channel_133 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_133 := 0;
    ELSE
        Verify_Channel_133 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_133 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.133;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_133 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_133 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_133 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 134 *)
FUNCTION Verify_Channel_134 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_134 := 0;
    ELSE
        Verify_Channel_134 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_134 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.134;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_134 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_134 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_134 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 135 *)
FUNCTION Verify_Channel_135 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_135 := 0;
    ELSE
        Verify_Channel_135 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_135 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.135;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_135 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_135 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_135 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 136 *)
FUNCTION Verify_Channel_136 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_136 := 0;
    ELSE
        Verify_Channel_136 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_136 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.136;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_136 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_136 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_136 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 137 *)
FUNCTION Verify_Channel_137 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_137 := 0;
    ELSE
        Verify_Channel_137 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_137 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.137;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_137 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_137 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_137 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 138 *)
FUNCTION Verify_Channel_138 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_138 := 0;
    ELSE
        Verify_Channel_138 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_138 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.138;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_138 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_138 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_138 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 139 *)
FUNCTION Verify_Channel_139 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_139 := 0;
    ELSE
        Verify_Channel_139 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_139 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.139;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_139 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_139 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_139 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 140 *)
FUNCTION Verify_Channel_140 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_140 := 0;
    ELSE
        Verify_Channel_140 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_140 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.140;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_140 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_140 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_140 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 141 *)
FUNCTION Verify_Channel_141 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_141 := 0;
    ELSE
        Verify_Channel_141 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_141 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.141;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_141 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_141 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_141 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 142 *)
FUNCTION Verify_Channel_142 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_142 := 0;
    ELSE
        Verify_Channel_142 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_142 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.142;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_142 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_142 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_142 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 143 *)
FUNCTION Verify_Channel_143 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_143 := 0;
    ELSE
        Verify_Channel_143 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_143 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.143;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_143 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_143 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_143 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 144 *)
FUNCTION Verify_Channel_144 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_144 := 0;
    ELSE
        Verify_Channel_144 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_144 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.144;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_144 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_144 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_144 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 145 *)
FUNCTION Verify_Channel_145 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_145 := 0;
    ELSE
        Verify_Channel_145 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_145 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.145;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_145 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_145 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_145 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 146 *)
FUNCTION Verify_Channel_146 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_146 := 0;
    ELSE
        Verify_Channel_146 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_146 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.146;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_146 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_146 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_146 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 147 *)
FUNCTION Verify_Channel_147 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_147 := 0;
    ELSE
        Verify_Channel_147 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_147 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.147;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_147 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_147 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_147 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 148 *)
FUNCTION Verify_Channel_148 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_148 := 0;
    ELSE
        Verify_Channel_148 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_148 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.148;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_148 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_148 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_148 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 149 *)
FUNCTION Verify_Channel_149 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_149 := 0;
    ELSE
        Verify_Channel_149 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_149 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.149;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_149 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_149 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_149 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 150 *)
FUNCTION Verify_Channel_150 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_150 := 0;
    ELSE
        Verify_Channel_150 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_150 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.150;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_150 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_150 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_150 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 151 *)
FUNCTION Verify_Channel_151 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_151 := 0;
    ELSE
        Verify_Channel_151 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_151 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.151;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_151 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_151 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_151 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 152 *)
FUNCTION Verify_Channel_152 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_152 := 0;
    ELSE
        Verify_Channel_152 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_152 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.152;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_152 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_152 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_152 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 153 *)
FUNCTION Verify_Channel_153 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_153 := 0;
    ELSE
        Verify_Channel_153 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_153 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.153;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_153 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_153 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_153 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 154 *)
FUNCTION Verify_Channel_154 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_154 := 0;
    ELSE
        Verify_Channel_154 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_154 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.154;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_154 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_154 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_154 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 155 *)
FUNCTION Verify_Channel_155 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_155 := 0;
    ELSE
        Verify_Channel_155 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_155 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.155;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_155 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_155 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_155 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 156 *)
FUNCTION Verify_Channel_156 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_156 := 0;
    ELSE
        Verify_Channel_156 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_156 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.156;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_156 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_156 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_156 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 157 *)
FUNCTION Verify_Channel_157 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_157 := 0;
    ELSE
        Verify_Channel_157 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_157 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.157;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_157 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_157 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_157 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 158 *)
FUNCTION Verify_Channel_158 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_158 := 0;
    ELSE
        Verify_Channel_158 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_158 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.158;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_158 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_158 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_158 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 159 *)
FUNCTION Verify_Channel_159 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_159 := 0;
    ELSE
        Verify_Channel_159 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_159 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.159;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_159 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_159 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_159 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 160 *)
FUNCTION Verify_Channel_160 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_160 := 0;
    ELSE
        Verify_Channel_160 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_160 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.160;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_160 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_160 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_160 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 161 *)
FUNCTION Verify_Channel_161 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_161 := 0;
    ELSE
        Verify_Channel_161 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_161 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.161;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_161 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_161 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_161 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 162 *)
FUNCTION Verify_Channel_162 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_162 := 0;
    ELSE
        Verify_Channel_162 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_162 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.162;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_162 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_162 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_162 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 163 *)
FUNCTION Verify_Channel_163 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_163 := 0;
    ELSE
        Verify_Channel_163 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_163 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.163;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_163 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_163 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_163 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 164 *)
FUNCTION Verify_Channel_164 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_164 := 0;
    ELSE
        Verify_Channel_164 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_164 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.164;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_164 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_164 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_164 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 165 *)
FUNCTION Verify_Channel_165 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_165 := 0;
    ELSE
        Verify_Channel_165 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_165 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.165;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_165 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_165 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_165 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 166 *)
FUNCTION Verify_Channel_166 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_166 := 0;
    ELSE
        Verify_Channel_166 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_166 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.166;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_166 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_166 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_166 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 167 *)
FUNCTION Verify_Channel_167 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_167 := 0;
    ELSE
        Verify_Channel_167 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_167 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.167;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_167 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_167 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_167 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 168 *)
FUNCTION Verify_Channel_168 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_168 := 0;
    ELSE
        Verify_Channel_168 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_168 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.168;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_168 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_168 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_168 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 169 *)
FUNCTION Verify_Channel_169 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_169 := 0;
    ELSE
        Verify_Channel_169 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_169 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.169;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_169 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_169 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_169 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 170 *)
FUNCTION Verify_Channel_170 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_170 := 0;
    ELSE
        Verify_Channel_170 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_170 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.170;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_170 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_170 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_170 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 171 *)
FUNCTION Verify_Channel_171 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_171 := 0;
    ELSE
        Verify_Channel_171 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_171 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.171;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_171 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_171 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_171 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 172 *)
FUNCTION Verify_Channel_172 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_172 := 0;
    ELSE
        Verify_Channel_172 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_172 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.172;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_172 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_172 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_172 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 173 *)
FUNCTION Verify_Channel_173 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_173 := 0;
    ELSE
        Verify_Channel_173 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_173 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.173;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_173 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_173 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_173 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 174 *)
FUNCTION Verify_Channel_174 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_174 := 0;
    ELSE
        Verify_Channel_174 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_174 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.174;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_174 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_174 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_174 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 175 *)
FUNCTION Verify_Channel_175 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_175 := 0;
    ELSE
        Verify_Channel_175 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_175 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.175;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_175 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_175 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_175 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 176 *)
FUNCTION Verify_Channel_176 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_176 := 0;
    ELSE
        Verify_Channel_176 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_176 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.176;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_176 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_176 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_176 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 177 *)
FUNCTION Verify_Channel_177 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_177 := 0;
    ELSE
        Verify_Channel_177 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_177 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.177;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_177 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_177 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_177 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 178 *)
FUNCTION Verify_Channel_178 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_178 := 0;
    ELSE
        Verify_Channel_178 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_178 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.178;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_178 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_178 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_178 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 179 *)
FUNCTION Verify_Channel_179 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_179 := 0;
    ELSE
        Verify_Channel_179 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_179 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.179;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_179 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_179 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_179 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 180 *)
FUNCTION Verify_Channel_180 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_180 := 0;
    ELSE
        Verify_Channel_180 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_180 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.180;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_180 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_180 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_180 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 181 *)
FUNCTION Verify_Channel_181 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_181 := 0;
    ELSE
        Verify_Channel_181 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_181 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.181;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_181 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_181 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_181 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 182 *)
FUNCTION Verify_Channel_182 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_182 := 0;
    ELSE
        Verify_Channel_182 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_182 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.182;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_182 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_182 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_182 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 183 *)
FUNCTION Verify_Channel_183 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_183 := 0;
    ELSE
        Verify_Channel_183 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_183 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.183;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_183 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_183 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_183 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 184 *)
FUNCTION Verify_Channel_184 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_184 := 0;
    ELSE
        Verify_Channel_184 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_184 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.184;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_184 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_184 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_184 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 185 *)
FUNCTION Verify_Channel_185 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_185 := 0;
    ELSE
        Verify_Channel_185 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_185 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.185;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_185 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_185 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_185 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 186 *)
FUNCTION Verify_Channel_186 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_186 := 0;
    ELSE
        Verify_Channel_186 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_186 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.186;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_186 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_186 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_186 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 187 *)
FUNCTION Verify_Channel_187 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_187 := 0;
    ELSE
        Verify_Channel_187 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_187 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.187;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_187 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_187 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_187 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 188 *)
FUNCTION Verify_Channel_188 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_188 := 0;
    ELSE
        Verify_Channel_188 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_188 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.188;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_188 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_188 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_188 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 189 *)
FUNCTION Verify_Channel_189 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_189 := 0;
    ELSE
        Verify_Channel_189 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_189 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.189;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_189 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_189 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_189 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 190 *)
FUNCTION Verify_Channel_190 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_190 := 0;
    ELSE
        Verify_Channel_190 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_190 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.190;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_190 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_190 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_190 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 191 *)
FUNCTION Verify_Channel_191 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_191 := 0;
    ELSE
        Verify_Channel_191 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_191 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.191;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_191 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_191 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_191 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 192 *)
FUNCTION Verify_Channel_192 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_192 := 0;
    ELSE
        Verify_Channel_192 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_192 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.192;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_192 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_192 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_192 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 193 *)
FUNCTION Verify_Channel_193 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_193 := 0;
    ELSE
        Verify_Channel_193 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_193 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.193;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_193 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_193 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_193 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 194 *)
FUNCTION Verify_Channel_194 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_194 := 0;
    ELSE
        Verify_Channel_194 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_194 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.194;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_194 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_194 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_194 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 195 *)
FUNCTION Verify_Channel_195 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_195 := 0;
    ELSE
        Verify_Channel_195 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_195 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.195;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_195 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_195 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_195 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 196 *)
FUNCTION Verify_Channel_196 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_196 := 0;
    ELSE
        Verify_Channel_196 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_196 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.196;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_196 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_196 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_196 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 197 *)
FUNCTION Verify_Channel_197 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_197 := 0;
    ELSE
        Verify_Channel_197 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_197 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.197;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_197 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_197 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_197 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 198 *)
FUNCTION Verify_Channel_198 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_198 := 0;
    ELSE
        Verify_Channel_198 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_198 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.198;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_198 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_198 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_198 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 199 *)
FUNCTION Verify_Channel_199 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_199 := 0;
    ELSE
        Verify_Channel_199 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_199 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.199;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_199 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_199 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_199 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 200 *)
FUNCTION Verify_Channel_200 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_200 := 0;
    ELSE
        Verify_Channel_200 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_200 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.200;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_200 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_200 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_200 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 201 *)
FUNCTION Verify_Channel_201 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_201 := 0;
    ELSE
        Verify_Channel_201 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_201 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.201;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_201 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_201 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_201 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 202 *)
FUNCTION Verify_Channel_202 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_202 := 0;
    ELSE
        Verify_Channel_202 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_202 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.202;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_202 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_202 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_202 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 203 *)
FUNCTION Verify_Channel_203 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_203 := 0;
    ELSE
        Verify_Channel_203 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_203 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.203;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_203 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_203 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_203 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 204 *)
FUNCTION Verify_Channel_204 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_204 := 0;
    ELSE
        Verify_Channel_204 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_204 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.204;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_204 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_204 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_204 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 205 *)
FUNCTION Verify_Channel_205 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_205 := 0;
    ELSE
        Verify_Channel_205 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_205 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.205;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_205 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_205 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_205 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 206 *)
FUNCTION Verify_Channel_206 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_206 := 0;
    ELSE
        Verify_Channel_206 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_206 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.206;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_206 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_206 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_206 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 207 *)
FUNCTION Verify_Channel_207 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_207 := 0;
    ELSE
        Verify_Channel_207 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_207 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.207;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_207 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_207 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_207 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 208 *)
FUNCTION Verify_Channel_208 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_208 := 0;
    ELSE
        Verify_Channel_208 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_208 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.208;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_208 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_208 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_208 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 209 *)
FUNCTION Verify_Channel_209 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_209 := 0;
    ELSE
        Verify_Channel_209 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_209 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.209;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_209 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_209 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_209 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 210 *)
FUNCTION Verify_Channel_210 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_210 := 0;
    ELSE
        Verify_Channel_210 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_210 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.210;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_210 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_210 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_210 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 211 *)
FUNCTION Verify_Channel_211 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_211 := 0;
    ELSE
        Verify_Channel_211 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_211 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.211;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_211 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_211 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_211 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 212 *)
FUNCTION Verify_Channel_212 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_212 := 0;
    ELSE
        Verify_Channel_212 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_212 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.212;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_212 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_212 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_212 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 213 *)
FUNCTION Verify_Channel_213 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_213 := 0;
    ELSE
        Verify_Channel_213 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_213 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.213;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_213 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_213 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_213 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 214 *)
FUNCTION Verify_Channel_214 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_214 := 0;
    ELSE
        Verify_Channel_214 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_214 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.214;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_214 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_214 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_214 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 215 *)
FUNCTION Verify_Channel_215 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_215 := 0;
    ELSE
        Verify_Channel_215 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_215 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.215;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_215 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_215 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_215 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 216 *)
FUNCTION Verify_Channel_216 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_216 := 0;
    ELSE
        Verify_Channel_216 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_216 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.216;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_216 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_216 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_216 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 217 *)
FUNCTION Verify_Channel_217 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_217 := 0;
    ELSE
        Verify_Channel_217 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_217 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.217;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_217 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_217 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_217 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 218 *)
FUNCTION Verify_Channel_218 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_218 := 0;
    ELSE
        Verify_Channel_218 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_218 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.218;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_218 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_218 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_218 := (value >= lo) AND (value <= hi);
END_FUNCTION

(* @LOOP_MAX(100) - 校验函数 219 *)
FUNCTION Verify_Channel_219 : INT
VAR_INPUT
    value : REAL;
    limit : REAL;
END_VAR
    tmp_bool := (value <= limit);
    IF tmp_bool THEN
        Verify_Channel_219 := 0;
    ELSE
        Verify_Channel_219 := -1;
    END_IF;
END_FUNCTION

FUNCTION Scale_219 : REAL
VAR_INPUT
    raw : REAL;
END_VAR
    tmp_real := raw * 1.219;
    IF tmp_real > 10000.0 THEN
        tmp_real := 10000.0;
    END_IF;
    Scale_219 := tmp_real;
END_FUNCTION

FUNCTION LimitCheck_219 : BOOL
VAR_INPUT
    value : REAL;
    lo : REAL;
    hi : REAL;
END_VAR
    LimitCheck_219 := (value >= lo) AND (value <= hi);
END_FUNCTION


(* ================================================================ *)
(* 主程序 *)
(* ================================================================ *)
    PRINT('');
    PRINT('================================================');
    PRINT('  反应堆 2oo4 安全控制系统');
    PRINT('  IEC 61508 SIL3 极限测试');
    PRINT('================================================');
    
    (* 初始化 *)
    system_state := SYS_STARTUP;
    scan_counter := 0;
    trip_counter := 0;
    alarm_counter := 0;
    fault_counter := 0;
    
    (* 执行控制周期 *)
    ControlCycle();
    
    (* 输出状态 *)
    PRINT('System State: %d', system_state);
    PRINT('Scan Count:   %d', scan_counter);
    PRINT('Trip Count:   %d', trip_counter);
    PRINT('Alarm Count:  %d', alarm_counter);
    PRINT('Fault Count:  %d', fault_counter);
    
    (* 执行 WCET 分析标注的所有循环 *)
    FOR loop_idx := 1 TO 10 DO
        tmp_real := Scale_1(loop_idx);
    END_FOR;
    
    PRINT('================================================');
    PRINT('  反应堆控制测试完成');
    PRINT('================================================');
END_PROGRAM
