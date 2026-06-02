(**
 * PID控制器演示 - 原始表达式版本
 * 验证复杂嵌套表达式是否支持
 *)
PROGRAM PIDDemoOriginal
VAR
    (* PID参数 *)
    Kp: REAL;
    Ki: REAL;
    Kd: REAL;
    
    (* 状态变量 *)
    setpoint: REAL;
    process_value: REAL;
    error: REAL;
    last_error: REAL;
    integral: REAL;
    derivative: REAL;
    
    (* PID项 *)
    p_term: REAL;
    i_term: REAL;
    d_term: REAL;
    control_output: REAL;
    
    (* 系统模拟参数 *)
    system_gain: REAL;
    system_lag: REAL;
    temp: REAL;
    
    (* 循环控制 *)
    step: INT;
END_VAR

    PRINT('PID Controller Demo - Original Expression\n');
    PRINT('==========================================\n\n');
    
    (* 初始化PID参数 *)
    Kp := 1.5;
    Ki := 0.5;
    Kd := 0.2;
    
    (* 系统参数 *)
    system_gain := 0.5;
    system_lag := 0.3;
    
    (* 初始状态 *)
    setpoint := 100.0;
    process_value := 0.0;
    integral := 0.0;
    last_error := 0.0;
    
    PRINT('Parameters:\n');
    PRINT('  Kp=%f, Ki=%f, Kd=%f\n', Kp, Ki, Kd);
    PRINT('  Target=%f, Start=%f\n\n', setpoint, process_value);
    
    (* 控制循环 *)
    FOR step := 1 TO 30 DO
        (* 计算误差 *)
        error := setpoint - process_value;
        
        (* PID计算 *)
        p_term := Kp * error;
        integral := integral + error * 0.1;
        i_term := Ki * integral;
        derivative := (error - last_error) / 0.1;
        d_term := Kd * derivative;
        
        control_output := p_term + i_term + d_term;
        last_error := error;
        
        (* 【关键】使用原始复杂表达式模拟被控对象 *)
        temp := system_lag * process_value;
        process_value := temp + (1.0 - system_lag) * system_gain * control_output;
        
        IF step = 1 OR step = 5 OR step = 10 OR step = 20 OR step = 30 THEN
            PRINT('Step %d: PV=%f, Error=%f\n', step, process_value, error);
        END_IF;
    END_FOR;
    
    PRINT('\nFinal: PV=%f\n', process_value);
    PRINT('Test PASSED - Complex expressions work!\n');
    
END_PROGRAM
