PROGRAM ForceIntegrationTest
VAR
    counter : INT := 0;
    temperature : REAL := 20.0;
    running : BOOL := TRUE;
END_VAR

    // 正常递增计数器
    counter := counter + 1;
    
    // 更新温度（模拟读取）
    temperature := temperature + 0.5;
    
    // 打印当前值
    PRINT("Cycle: counter=%d, temp=%.1f\n", counter, temperature);
    
    // 简单的条件检查
    IF counter > 10 THEN
        PRINT("Counter > 10\n");
    END_IF;
    
    IF temperature > 30.0 THEN
        PRINT("Temperature too high!\n");
    END_IF;

END_PROGRAM
