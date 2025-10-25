(**
 * @file example_io_blink.st
 * @brief GPIO LED 闪烁示例 - 展示 I/O 映射功能
 * 
 * 硬件要求：
 * - LED 连接到 GPIO 17 (输出)
 * - 按钮连接到 GPIO 27 (输入，带上拉)
 * 
 * 功能：
 * - 按钮按下时，LED 闪烁
 * - 按钮释放时，LED 熄灭
 *)

PROGRAM LEDBlink

FUNCTION MAIN
    PRINT("开始 LED 闪烁示例程序(DUMMY)\n");
END_FUNCTION

FUNCTION IO_Blink
VAR_EXTERNAL
    (* 输入：按钮 *)
    button AT %IX0.0 : BOOL;        (* GPIO 27 - 按钮输入 *)
    
    (* 输出：LED *)
    led AT %QX0.0 : BOOL;           (* GPIO 17 - LED输出 *)
END_VAR

VAR
    blink_timer : INT := 0;
    led_state : BOOL := FALSE;
END_VAR

(* 主逻辑 *)
IF button THEN
    (* 按钮按下，LED闪烁 *)
    blink_timer := blink_timer + 1;
    
    IF blink_timer >= 5 THEN  (* 5循环周期 *)
        led_state := NOT led_state;
        led := led_state;
        PRINT("按钮按下，LED状态切换:%b\n", led_state);
        blink_timer := 0;
    END_IF;
ELSE
    (* 按钮释放，LED熄灭 *)
    led := FALSE;
    led_state := FALSE;
    blink_timer := 0;
    PRINT("按钮未按下，LED熄灭: %b\n", led_state);
END_IF;
END_FUNCTION

END_PROGRAM
