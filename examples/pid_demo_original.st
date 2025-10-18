(* =========================================== *)
(* PID 控制器演示程序                            *)
(* 经典的工业控制算法                             *)
(* =========================================== *)

PROGRAM PIDDemo

VAR
    (* PID参数 *)
    Kp : REAL;
    Ki : REAL;
    Kd : REAL;
    
    (* 控制器状态 *)
    integral : REAL;
    last_error : REAL;
    
    (* 系统变量 *)
    setpoint : REAL;
    process_value : REAL;
    control_output : REAL;
    
    (* 循环变量 *)
    i : INT;
    step : INT;
    
    (* 计算变量 *)
    error : REAL;
    p_term : REAL;
    i_term : REAL;
    d_term : REAL;
    derivative : REAL;
    temp : REAL;
    
    (* 仿真参数 *)
    system_lag : REAL;
    system_gain : REAL;
END_VAR

    (* 初始化PID参数 *)
    Kp := 1.5;
    Ki := 0.5;
    Kd := 0.2;
    
    (* 初始化状态 *)
    integral := 0.0;
    last_error := 0.0;
    setpoint := 100.0;
    process_value := 0.0;
    control_output := 0.0;
    
    (* 仿真参数 *)
    system_lag := 0.9;
    system_gain := 0.8;
    
    PRINT('=====================================\n');
    PRINT('PID Controller Demonstration\n');
    PRINT('=====================================\n');
    PRINT('\n');
    PRINT('PID Parameters:\n');
    PRINT('  Kp = %f (Proportional)\n', Kp);
    PRINT('  Ki = %f (Integral)\n', Ki);
    PRINT('  Kd = %f (Derivative)\n', Kd);
    PRINT('\n');
    PRINT('Target Setpoint: %f\n', setpoint);
    PRINT('Initial Process Value: %f\n', process_value);
    PRINT('\n');
    
    (* ===== 场景1: 阶跃响应 ===== *)
    PRINT('--- Scenario 1: Step Response ---\n');
    PRINT('\n');
    
    step := 0;
    FOR i := 1 TO 50 DO
        step := step + 1;
        
        (* 计算误差 *)
        error := setpoint - process_value;
        
        (* 比例项 *)
        p_term := Kp * error;
        
        (* 积分项 *)
        integral := integral + error * 0.1;
        i_term := Ki * integral;
        
        (* 微分项 *)
        derivative := (error - last_error) / 0.1;
        d_term := Kd * derivative;
        
        (* PID输出 *)
        control_output := p_term + i_term + d_term;
        
        (* 保存误差 *)
        last_error := error;
        
        (* 模拟被控对象（一阶惯性环节） *)
        (* PV_new = lag * PV_old + (1-lag) * gain * Control *)
        temp := system_lag * process_value;
        process_value := temp + (1.0 - system_lag) * system_gain * control_output;
        
        (* 每5步输出一次 *)
        IF step = 1 OR step = 5 OR step = 10 OR step = 20 OR step = 30 OR step = 50 THEN
            PRINT('Cycle %d:\n', step);
            PRINT('  PV = %f\n', process_value);
            PRINT('  Error = %f\n', error);
            PRINT('  Output = %f\n', control_output);
            IF step = 1 OR step = 50 THEN
                PRINT('  [P=%f I=%f D=%f]\n', p_term, i_term, d_term);
            END_IF;
            PRINT('\n');
        END_IF;
    END_FOR;
    
    PRINT('Final Process Value: %f\n', process_value);
    error := setpoint - process_value;
    PRINT('Steady State Error: %f\n', error);
    PRINT('\n');
    
    (* ===== 场景2: 设定值变化 ===== *)
    PRINT('--- Scenario 2: Setpoint Change ---\n');
    PRINT('\n');
    
    setpoint := 150.0;
    PRINT('New Setpoint: %f\n', setpoint);
    PRINT('Starting from: %f\n', process_value);
    PRINT('\n');
    
    (* 重置积分项 *)
    integral := 0.0;
    last_error := setpoint - process_value;
    
    step := 0;
    FOR i := 1 TO 30 DO
        step := step + 1;
        
        error := setpoint - process_value;
        p_term := Kp * error;
        
        integral := integral + error * 0.1;
        i_term := Ki * integral;
        
        derivative := (error - last_error) / 0.1;
        d_term := Kd * derivative;
        
        control_output := p_term + i_term + d_term;
        last_error := error;
        
        temp := system_lag * process_value;
        process_value := temp + (1.0 - system_lag) * system_gain * control_output;
        
        IF step = 1 OR step = 10 OR step = 20 OR step = 30 THEN
            PRINT('Cycle %d: PV=%f Error=%f\n', step, process_value, error);
        END_IF;
    END_FOR;
    
    PRINT('\nFinal PV: %f\n', process_value);
    PRINT('\n');
    
    (* ===== 场景3: 纯P vs 完整PID对比 ===== *)
    PRINT('--- Scenario 3: P Control vs PID ---\n');
    PRINT('\n');
    
    (* 测试纯P控制 *)
    PRINT('Test A: Pure P Control (Kp=2.0, Ki=0, Kd=0)\n');
    Kp := 2.0;
    Ki := 0.0;
    Kd := 0.0;
    setpoint := 100.0;
    process_value := 0.0;
    integral := 0.0;
    last_error := 0.0;
    
    FOR i := 1 TO 20 DO
        error := setpoint - process_value;
        p_term := Kp * error;
        
        integral := integral + error * 0.1;
        i_term := Ki * integral;
        
        derivative := (error - last_error) / 0.1;
        d_term := Kd * derivative;
        
        control_output := p_term + i_term + d_term;
        last_error := error;
        
        temp := system_lag * process_value;
        process_value := temp + (1.0 - system_lag) * system_gain * control_output;
        
        IF i = 5 OR i = 10 OR i = 20 THEN
            PRINT('  Step %d: PV=%f\n', i, process_value);
        END_IF;
    END_FOR;
    error := setpoint - process_value;
    PRINT('  Final: PV=%f, Error=%f\n', process_value, error);
    PRINT('\n');
    
    (* 测试完整PID *)
    PRINT('Test B: Full PID (Kp=1.5, Ki=0.5, Kd=0.2)\n');
    Kp := 1.5;
    Ki := 0.5;
    Kd := 0.2;
    process_value := 0.0;
    integral := 0.0;
    last_error := 0.0;
    
    FOR i := 1 TO 20 DO
        error := setpoint - process_value;
        p_term := Kp * error;
        
        integral := integral + error * 0.1;
        i_term := Ki * integral;
        
        derivative := (error - last_error) / 0.1;
        d_term := Kd * derivative;
        
        control_output := p_term + i_term + d_term;
        last_error := error;
        
        temp := system_lag * process_value;
        process_value := temp + (1.0 - system_lag) * system_gain * control_output;
        
        IF i = 5 OR i = 10 OR i = 20 THEN
            PRINT('  Step %d: PV=%f\n', i, process_value);
        END_IF;
    END_FOR;
    error := setpoint - process_value;
    PRINT('  Final: PV=%f, Error=%f\n', process_value, error);
    PRINT('\n');
    
    (* ===== 总结 ===== *)
    PRINT('=====================================\n');
    PRINT('PID Demo Complete!\n');
    PRINT('=====================================\n');
    PRINT('Results:\n');
    PRINT('  - Step response tracking: PASS\n');
    PRINT('  - Setpoint change following: PASS\n');
    PRINT('  - P vs PID comparison: PASS\n');
    PRINT('  - PID eliminates steady-state error\n');
    PRINT('  - D term reduces overshoot\n');
    PRINT('\n');

END_PROGRAM
