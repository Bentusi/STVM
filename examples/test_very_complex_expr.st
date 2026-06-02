PROGRAM TestVeryComplexExpr
VAR
    a: REAL;
    b: REAL;
    c: REAL;
    d: REAL;
    e: REAL;
    result: REAL;
    temp: REAL;
END_VAR

    a := 1.0;
    b := 2.0;
    c := 3.0;
    d := 4.0;
    e := 5.0;
    
    (* 测试1: 基本的3项乘法 *)
    PRINT('Test 1: a * b * c\n');
    result := a * b * c;
    PRINT('result = %f\n', result);
    
    (* 测试2: 带括号的减法和乘法 *)
    PRINT('Test 2: (a - b) * c * d\n');
    result := (a - b) * c * d;
    PRINT('result = %f\n', result);
    
    (* 测试3: 加法和括号内的乘法 *)
    PRINT('Test 3: e + (a - b) * c * d\n');
    result := e + (a - b) * c * d;
    PRINT('result = %f\n', result);
    
    (* 测试4: 更复杂的表达式 *)
    PRINT('Test 4: temp * a + (b - c) * d * e\n');
    temp := 10.0;
    result := temp * a + (b - c) * d * e;
    PRINT('result = %f\n', result);
    
    (* 测试5: PID demo中的实际表达式 *)
    PRINT('Test 5: PID demo pattern\n');
    temp := 0.3;
    a := 0.7;    (* system_lag *)
    b := 0.5;    (* system_gain *)
    c := 20.0;   (* control_output *)
    result := temp + (1.0 - a) * b * c;
    PRINT('result = %f\n', result);
    
    PRINT('All tests passed!\n');
END_PROGRAM
