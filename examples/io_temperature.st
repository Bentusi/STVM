/**
 * @file example_io_adc.st
 * @brief ADC 模拟输入示例 - 温度监控
 * 
 * 硬件要求：
 * - 温度传感器连接到 ADC 通道 0
 * - 风扇 PWM 连接到 GPIO 18
 * - 报警LED 连接到 GPIO 23
 * 
 * 功能：
 * - 读取温度传感器值
 * - 温度过高时启动风扇
 * - 温度危险时触发报警
 */

PROGRAM TemperatureMonitor

VAR_EXTERNAL
    (* 输入：传感器 *)
    temp_raw AT %IW0 : INT;         (* ADC通道0 - 温度传感器 (0-4095) *)
    
    (* 输出：控制 *)
    fan_pwm AT %QW0 : INT;          (* PWM输出 - 风扇速度 (0-100%) *)
    alarm_led AT %QX1.0 : BOOL;     (* GPIO 23 - 报警LED *)
END_VAR

VAR
    temperature : REAL;             (* 实际温度 (摄氏度) *)
    fan_speed : INT;                (* 风扇速度百分比 *)
    
    (* 温度阈值 *)
    TEMP_FAN_ON : REAL := 30.0;     (* 30°C 启动风扇 *)
    TEMP_ALARM : REAL := 45.0;      (* 45°C 报警 *)
    TEMP_MAX : REAL := 50.0;        (* 50°C 最大风扇转速 *)
END_VAR

(* ADC 转温度：假设 0-4095 对应 0-100°C *)
temperature := INT_TO_REAL(temp_raw) * 100.0 / 4095.0;

(* 风扇控制逻辑 *)
IF temperature < TEMP_FAN_ON THEN
    (* 温度正常，风扇关闭 *)
    fan_speed := 0;
    
ELSIF temperature >= TEMP_FAN_ON AND temperature < TEMP_MAX THEN
    (* 温度升高，风扇线性加速 *)
    fan_speed := REAL_TO_INT((temperature - TEMP_FAN_ON) / (TEMP_MAX - TEMP_FAN_ON) * 100.0);
    
ELSE
    (* 温度过高，风扇全速 *)
    fan_speed := 100;
END_IF;

(* 输出风扇速度 *)
fan_pwm := fan_speed;

(* 报警逻辑 *)
IF temperature >= TEMP_ALARM THEN
    alarm_led := TRUE;
ELSE
    alarm_led := FALSE;
END_IF;

(* 调试输出 *)
PRINT("Temperature: %.1f C, Fan: %d%%", temperature, fan_speed);

END_PROGRAM
