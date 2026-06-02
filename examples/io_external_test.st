(**
 * @file io_external_test.st
 * @brief 测试 VAR_EXTERNAL 和 IO 模拟器集成
 *)

PROGRAM IOExternalTest

VAR_EXTERNAL
    (* 数字输入 - 按钮 *)
    button AT %IX0.0 : BOOL;
    
    (* 数字输出 - LED *)
    led AT %QX0.0 : BOOL;
    
    (* 模拟输入 - 温度传感器 *)
    temperature AT %IW0 : INT;
    
    (* 模拟输出 - PWM *)
    pwm_duty AT %QW0 : INT;
END_VAR

VAR
    counter : INT := 0;
    led_state : BOOL := FALSE;
END_VAR

(* 主逻辑 *)
counter := counter + 1;

(* LED闪烁测试 *)
IF counter MOD 100 = 0 THEN
    led_state := NOT led_state;
    led := led_state;
END_IF;

(* 读取温度值 *)
IF temperature > 500 THEN
    pwm_duty := 80;  (* 温度高，PWM 80% *)
ELSE
    pwm_duty := 30;  (* 温度低，PWM 30% *)
END_IF;

(* 按钮控制 *)
IF button THEN
    led := TRUE;
ELSE
    led := led_state;
END_IF;

END_PROGRAM
