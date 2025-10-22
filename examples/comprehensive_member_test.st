PROGRAM comprehensive_member_access_test

VAR
    // 不同类型的质量化变量
    temperature: QREAL;
    temp: QREAL;
    pressure: QINT; 
    temp2: QINT;
    alarm: QBOOL;
    temp3: QBOOL;
    status: QSTRING;
    temp4 : QSTRING;
    
    // 用于计算的变量
    temp_celsius: REAL;
    temp_fahrenheit: REAL;
    pressure_kpa: INT;
END_VAR

BEGIN
    PRINT('=== 质量化数据类型成员访问测试 ===\n');
    
    // 1. 设置各种质量化变量的值和质量位
    temperature.VAL := 23.5;
    temperature.QUALITY := 0;  // GOOD
    
    pressure.VAL := 1013;
    pressure.QUALITY := 1;     // UNCERTAIN
    
    status.VAL := 'RUNNING';
    status.QUALITY := 0;       // GOOD
    
    alarm.VAL := TRUE;
    alarm.QUALITY := 2;        // BAD
    
    PRINT('初始化完成\n');
    
    // 2. 读取并使用成员值进行计算
    temp_celsius := temperature.VAL;
    temp_fahrenheit := temp_celsius * 9.0 / 5.0 + 32.0;
    
    pressure_kpa := pressure.VAL;
    
    PRINT('温度转换完成\n');
    
    // 3. 基于质量位的条件判断
    IF temperature.QUALITY = 0 THEN
        PRINT('温度读数质量良好\n');
        IF temperature.VAL > 20.0 THEN
            PRINT('温度正常范围\n');
        END_IF;
    ELSE
        PRINT('温度读数质量有问题\n');
    END_IF;
    
    IF pressure.QUALITY <> 0 THEN
        PRINT('压力读数质量不确定\n');
        // 当质量不好时，可以重新设置质量位
        pressure.QUALITY := 0;  // 标记为GOOD
        PRINT('压力质量位已重置\n');
    END_IF;
    
    IF alarm.QUALITY = 2 THEN
        PRINT('报警状态数据质量差\n');
        alarm.QUALITY := 0;  // 修复质量位
        PRINT('报警质量位已修复\n');
    END_IF;
    
    // 4. 复杂表达式中的成员访问
    IF (temperature.VAL > 15.0) AND (temperature.QUALITY = 0) AND 
       (pressure.VAL > 1000) THEN
        PRINT('环境条件正常且数据可靠\n');
    END_IF;
    
    // 5. 动态修改值和质量
    temperature.VAL := temperature.VAL + 2.5;  // 温度上升
    IF temperature.VAL > 25.0 THEN
        temperature.QUALITY := 1;  // 标记为UNCERTAIN
        PRINT('温度过高，质量降级\n');
    END_IF;

    // 6.赋值测试
    temp := temperature; // 直接赋值
    PRINT('赋值测试完成，temp.VAL = %f, temp.QUALITY = %d\n', temp.VAL, temp.QUALITY);
    temp2 := pressure; // 直接赋值
    PRINT('赋值测试完成，temp2.VAL = %d, temp2.QUALITY = %d\n', temp2.VAL, temp2.QUALITY);
    temp3 := alarm; // 直接赋值
    PRINT('赋值测试完成，temp3.VAL = %b, temp3.QUALITY = %d\n', temp3.VAL, temp3.QUALITY);
    temp4 := status; // 直接赋值
    PRINT('赋值测试完成，temp4.VAL = %s, temp4.QUALITY = %d\n', temp4.VAL, temp4.QUALITY);

    PRINT('=== 所有成员访问测试完成 ===\n');
    
END_PROGRAM