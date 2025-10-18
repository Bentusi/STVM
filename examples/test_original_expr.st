PROGRAM TestOriginalExpression
VAR
    temp: REAL;
    system_lag: REAL;
    system_gain: REAL;
    control_output: REAL;
    process_value: REAL;
END_VAR

    (* 初始化 *)
    process_value := 0.0;
    system_lag := 0.3;
    system_gain := 0.5;
    control_output := 20.0;
    
    (* 原始的单行复杂表达式 *)
    temp := system_lag * process_value;
    process_value := temp + (1.0 - system_lag) * system_gain * control_output;
    
    PRINT('Result: %f\n', process_value);
    PRINT('Expected: 7.0\n');
    
END_PROGRAM
