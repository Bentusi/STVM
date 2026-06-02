PROGRAM test_member_access

VAR
    // 质量化类型变量
    temperature: QREAL;
    pressure: QINT;
    status: QSTRING;
    
    // 基础类型变量（用于比较）
    temp_val: REAL;
    temp_quality: INT;
    result: REAL;
END_VAR

// 测试 1: 基本的成员访问语法
BEGIN
    // 初始化质量化变量的值和质量位
    temperature.VAL := 25.5;
    temperature.QUALITY := 0;  // GOOD quality
    
    pressure.VAL := 1000;
    pressure.QUALITY := 1;     // UNCERTAIN quality
    
    status.VAL := 'RUNNING';
    status.QUALITY := 0;       // GOOD quality
    
    // 测试 2: 读取成员值
    temp_val := temperature.VAL;
    temp_quality := temperature.QUALITY;
    
    PRINT('Temperature VAL: ');
    PRINT(temp_val);
    PRINT('Temperature QUALITY: ');
    PRINT(temp_quality);
    
    // 测试 3: 在表达式中使用成员访问
    result := temperature.VAL + 5.0;
    PRINT('Temperature + 5.0 = ');
    PRINT(result);
    
    // 测试 4: 质量位条件检查
    IF temperature.QUALITY = 0 THEN
        PRINT('Temperature reading is GOOD');
    ELSE
        PRINT('Temperature reading has issues');
    END_IF;
    
    IF pressure.QUALITY <> 0 THEN
        PRINT('Pressure reading is UNCERTAIN/BAD');
    END_IF;
    
    // 测试 5: 修改质量位
    pressure.QUALITY := 2;  // BAD quality
    PRINT('Updated pressure quality to BAD: ');
    PRINT(pressure.QUALITY);
    
    // 测试 6: 复杂表达式中的成员访问
    IF (temperature.VAL > 20.0) AND (temperature.QUALITY = 0) THEN
        PRINT('Temperature is good and within range');
    END_IF;
    
END_PROGRAM