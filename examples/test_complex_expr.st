PROGRAM TestComplexExpr
VAR
    temp: REAL;
    system_lag: REAL;
    system_gain: REAL;
    control_output: REAL;
    process_value: REAL;
    derivative: REAL;
END_VAR

    (* 初始化变量 *)
    temp := 10.0;
    system_lag := 0.3;
    system_gain := 0.5;
    control_output := 20.0;
    
    (* 测试1: 简单表达式 - 应该工作 *)
    PRINT('Test 1: Simple expression\n');
    derivative := 1.0 - system_lag;
    PRINT('derivative = %f\n', derivative);
    
    (* 测试2: 两个乘法 - 应该工作 *)
    PRINT('Test 2: Two multiplications\n');
    derivative := system_gain * control_output;
    PRINT('derivative = %f\n', derivative);
    
    (* 测试3: 复杂表达式 - 可能失败 *)
    PRINT('Test 3: Complex nested expression\n');
    process_value := temp + (1.0 - system_lag) * system_gain * control_output;
    PRINT('process_value = %f\n', process_value);
    
    PRINT('All tests completed\n');
END_PROGRAM
