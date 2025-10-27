(*
 * 质量位变量错误处理和边界情况测试
 * 测试各种异常情况和边界条件下的质量位行为
 *)

PROGRAM QualityVariablesErrorTest

VAR
    (* 测试变量 *)
    sensor1 : QREAL;
    sensor2 : QREAL;
    counter : QINT;
    flag : QBOOL;
    message : QSTRING;
    
    (* 结果变量 *)
    result_real : QREAL;
    result_int : QINT;
    result_bool : QBOOL;
    
    (* 普通类型变量 *)
    normal_val : REAL;
    normal_count : INT;
    
    (* 控制变量 *)
    i : INT;
    test_quality : INT;
END_VAR

BEGIN
    PRINT('=== 质量位错误处理和边界测试 ===\n');
    
    (* 测试1: 极值质量位测试 *)
    PRINT('--- 测试1: 质量位边界值 ---\n');
    
    FOR i := 0 TO 3 DO
        sensor1.VAL := 25.0;
        sensor1.QUALITY := i;
        
        CASE i OF
            0: PRINT('质量位 0 (GOOD)\n');
            1: PRINT('质量位 1 (UNCERTAIN)\n');
            2: PRINT('质量位 2 (BAD)\n');
            3: PRINT('质量位 3 (ERROR)\n');
        END_CASE;
    END_FOR;
    
    (* 测试2: 不同质量位的运算传播规则 *)
    PRINT('--- 测试2: 质量位传播规则 ---\n');
    
    (* GOOD + UNCERTAIN = UNCERTAIN *)
    sensor1.VAL := 10.0;
    sensor1.QUALITY := 0;  (* GOOD *)
    sensor2.VAL := 5.0;
    sensor2.QUALITY := 1;  (* UNCERTAIN *)
    
    result_real := sensor1 + sensor2;
    PRINT('GOOD + UNCERTAIN = 质量位:%s\n', result_real.QUALITY);
    
    (* UNCERTAIN + BAD = BAD *)
    sensor2.QUALITY := 2;  (* BAD *)
    result_real := sensor1 + sensor2;
    PRINT('GOOD + BAD = 质量位:%s\n', result_real.QUALITY);
    
    (* BAD + ERROR = ERROR *)
    sensor1.QUALITY := 2;  (* BAD *)
    sensor2.QUALITY := 3;  (* ERROR *)
    result_real := sensor1 + sensor2;
    PRINT('BAD + ERROR = 质量位:%s\n', result_real.QUALITY);
    
    (* 测试3: 零值和特殊值处理 *)
    PRINT('--- 测试3: 特殊值处理 ---');
    
    (* 零值但不同质量位 *)
    sensor1.VAL := 0.0;
    sensor1.QUALITY := 0;  (* GOOD零值 *)
    sensor2.VAL := 0.0;
    sensor2.QUALITY := 2;  (* BAD零值 *)
    
    result_real := sensor1 + sensor2;
    PRINT('GOOD零值 + BAD零值 = 值:%s\n', result_real.VAL, ', 质量:', result_real.QUALITY);
    
    (* 测试4: 大数值和小数值处理 *)
    PRINT('--- 测试4: 极值处理 ---');
    
    sensor1.VAL := 999999.99;
    sensor1.QUALITY := 1;  (* UNCERTAIN *)
    sensor2.VAL := 0.00001;
    sensor2.QUALITY := 0;  (* GOOD *)
    
    result_real := sensor1 * sensor2;
    PRINT('大数*小数 = 值:%s\n', result_real.VAL, ', 质量:', result_real.QUALITY);
    
    (* 测试5: 布尔质量位的逻辑运算 *)
    PRINT('--- 测试5: 布尔质量位逻辑运算 ---');
    
    flag.VAL := TRUE;
    flag.QUALITY := 0;  (* GOOD *)
    
    result_bool.VAL := FALSE;
    result_bool.QUALITY := 2;  (* BAD *)
    
    (* 注意：布尔运算的质量位传播可能需要特殊处理 *)
    IF flag.VAL AND (flag.QUALITY = 0) THEN
        PRINT('GOOD的TRUE值可以信任');
    END_IF;
    
    IF result_bool.QUALITY <> 0 THEN
        PRINT('BAD质量位的布尔值不可信任，值=%s\n', result_bool.VAL);
    END_IF;
    
    (* 测试6: 字符串质量位处理 *)
    PRINT('--- 测试6: 字符串质量位 ---');
    
    message.VAL := 'NORMAL_STATUS';
    message.QUALITY := 0;  (* GOOD *)
    PRINT('良好状态消息: %s\n', message.VAL, ', 质量:', message.QUALITY);
    
    message.VAL := 'ERROR_MSG';
    message.QUALITY := 3;  (* ERROR *)
    PRINT('错误消息: %s\n', message.VAL, ', 质量:', message.QUALITY);
    
    (* 根据字符串质量位决定处理方式 *)
    IF message.QUALITY = 0 THEN
        PRINT('消息可信，正常处理');
    ELSE
        PRINT('消息不可信，需要验证');
    END_IF;
    
    (* 测试7: 质量位在循环中的变化 *)
    PRINT('--- 测试7: 循环中的质量位变化 ---');
    
    counter.VAL := 0;
    counter.QUALITY := 0;  (* 开始时GOOD *)
    
    FOR i := 1 TO 5 DO
        counter.VAL := counter.VAL + 1;
        
        (* 模拟质量随时间恶化 *)
        IF i = 3 THEN
            counter.QUALITY := 1;  (* 变为UNCERTAIN *)
            PRINT('第%s\n', i, '次：质量降级为UNCERTAIN');
        ELSIF i = 5 THEN
            counter.QUALITY := 2;  (* 变为BAD *)
            PRINT('第%s\n', i, '次：质量降级为BAD');
        END_IF;
        
        PRINT('计数器: 值=%s\n', counter.VAL, ', 质量=', counter.QUALITY);
    END_FOR;
    
    (* 测试8: 质量位比较的边界情况 *)
    PRINT('--- 测试8: 质量位比较 ---');
    
    sensor1.QUALITY := 0;  (* GOOD *)
    sensor2.QUALITY := 1;  (* UNCERTAIN *)
    
    IF sensor1.QUALITY < sensor2.QUALITY THEN
        PRINT('GOOD质量 < UNCERTAIN质量（数值比较）');
    END_IF;
    
    IF sensor1.QUALITY = 0 THEN
        PRINT('检测到GOOD质量位');
    END_IF;
    
    IF sensor2.QUALITY <> 0 THEN
        PRINT('检测到非GOOD质量位');
    END_IF;
    
    (* 测试9: 质量位重置和恢复场景 *)
    PRINT('--- 测试9: 质量位重置场景 ---');
    
    (* 模拟传感器故障和恢复 *)
    sensor1.VAL := 25.0;
    sensor1.QUALITY := 0;  (* 正常工作 *)
    PRINT('传感器正常: 值=%s\n', sensor1.VAL, ', 质量=', sensor1.QUALITY);
    
    (* 模拟故障 *)
    sensor1.QUALITY := 3;  (* ERROR *)
    PRINT('传感器故障: 质量=%s\n', sensor1.QUALITY);
    
    (* 故障期间数据处理 *)
    IF sensor1.QUALITY >= 2 THEN
        PRINT('传感器故障，使用备用值');
        normal_val := 20.0;  (* 使用默认值 *)
    ELSE
        normal_val := sensor1.VAL;
    END_IF;
    
    (* 模拟恢复 *)
    sensor1.VAL := 26.0;
    sensor1.QUALITY := 1;  (* 恢复但不确定 *)
    PRINT('传感器恢复: 值=%s\n', sensor1.VAL, ', 质量=', sensor1.QUALITY);
    
    (* 完全恢复 *)
    sensor1.QUALITY := 0;  (* GOOD *)
    PRINT('传感器完全恢复: 质量=%s\n', sensor1.QUALITY);
    
    (* 测试10: 复杂表达式中的质量位处理 *)
    PRINT('--- 测试10: 复杂表达式质量位 ---');
    
    sensor1.VAL := 20.0;
    sensor1.QUALITY := 0;  (* GOOD *)
    sensor2.VAL := 5.0;
    sensor2.QUALITY := 1;  (* UNCERTAIN *)
    
    result_real := (sensor1 + sensor2) * sensor1;
    PRINT('复杂运算结果: 值=%s\n', result_real.VAL, ', 质量=', result_real.QUALITY);
    
    (* 质量位的累积效应 *)
    result_real := sensor1;  (* GOOD *)
    result_real := result_real + sensor2;  (* GOOD + UNCERTAIN = UNCERTAIN *)
    result_real := result_real * sensor1;  (* UNCERTAIN * GOOD = UNCERTAIN *)
    PRINT('质量位累积结果: 质量=%s\n', result_real.QUALITY);
    
    PRINT('=== 错误处理和边界测试完成 ===');
    
END_PROGRAM