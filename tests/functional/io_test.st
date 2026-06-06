(*
 * ============================================================================
 * STVM IO 功能综合测试 v3
 * 覆盖: 数字IO / 模拟IO / 安全输出 / 信号缩放 / PID
 * 运行: stvm tests/functional/io_test.st -I
 * ============================================================================
 *)

PROGRAM IOFunctionalTest
VAR_EXTERNAL
    button  AT %IX0.0 : BOOL;
    led     AT %QX0.0 : BOOL;
    adc_raw AT %IW0   : INT;
    pwm_out AT %QW0   : INT;
END_VAR
VAR
    raw_value   : INT;
    scaled_temp : REAL;
    error_flag  : BOOL;
    io_count    : INT;
    setpoint    : REAL;
    process_val : REAL;
    pid_output  : REAL;
    kp          : REAL;
    ki          : REAL;
    kd          : REAL;
    error_val   : REAL;
    integral    : REAL;
    derivative  : REAL;
    last_error  : REAL;
END_VAR

    PRINT('');
    PRINT('========================================');
    PRINT('  STVM IO 功能综合测试');
    PRINT('========================================');

    (* 数字 IO *)
    PRINT('--- 数字 IO ---');
    PRINT('DI button: %d', button);
    led := button;
    PRINT('DO led: %d', led);

    (* 模拟 IO *)
    PRINT('--- 模拟 IO ---');
    PRINT('AI raw: %d', adc_raw);
    pwm_out := 500;
    PRINT('AO pwm: %d', pwm_out);

    (* 信号缩放 *)
    PRINT('--- 缩放 ---');
    raw_value := adc_raw;
    scaled_temp := raw_value * 0.1;
    PRINT('Scaled: %f', scaled_temp);

    (* IO 安全检查 *)
    PRINT('--- 安全 ---');
    error_flag := FALSE;
    IF adc_raw < 0 THEN
        error_flag := TRUE;
    END_IF;
    IF error_flag THEN
        led := FALSE;
        pwm_out := 0;
        PRINT('SAFE: output disabled');
    END_IF;

    (* PID 演示 *)
    PRINT('--- PID ---');
    kp := 1.5;
    ki := 1.0;
    kd := 0.5;
    setpoint := 25.0;
    process_val := scaled_temp;
    error_val := setpoint - process_val;
    integral := integral + error_val * 0.1;
    derivative := (error_val - last_error) / 0.1;
    last_error := error_val;
    pid_output := kp * error_val + ki * integral + kd * derivative;
    PRINT('PID: SP=%f PV=%f OUT=%f', setpoint, process_val, pid_output);

    (* 周期计数器 *)
    io_count := io_count + 1;
    PRINT('IO cycle: %d', io_count);

    PRINT('========================================');
    PRINT('  IO 测试完成');
    PRINT('========================================');
END_PROGRAM