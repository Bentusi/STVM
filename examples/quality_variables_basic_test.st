(*
 * 质量位变量基础测试用例
 * 专注于测试质量化数据类型的核心功能
 *)

PROGRAM QualityVariablesBasicTest

VAR
    (* 基本质量化变量 *)
    temp_sensor : QREAL;
    pressure_sensor : QINT;
    alarm_status : QBOOL;
    device_id : QSTRING;
    
    (* 辅助变量 *)
    temp_val : REAL;
    press_val : INT;
    status_val : BOOL;
    quality_val : INT;
    result : QREAL;
END_VAR

BEGIN
    PRINT('=== 质量位变量基础测试 ===\n');
    
    (* 测试1: 基本赋值和读取 *)
    PRINT('--- 测试1: 基本操作 ---\n');
    
    temp_sensor := 25.0;
    pressure_sensor := 1013;
    alarm_status := TRUE;
    device_id := 'SENSOR_A1';
    
    PRINT('基本变量初始化完成\n');
    
    (* 测试2: 成员访问 *)
    PRINT('--- 测试2: 成员访问 ---\n');
    
    temp_sensor.VAL := 28.5;
    temp_sensor.QUALITY := 0;  (* GOOD *)
    
    temp_val := temp_sensor.VAL;
    quality_val := temp_sensor.QUALITY;
    
    PRINT('温度值: %f\n', temp_val);
    PRINT('温度质量位: %d\n', quality_val);
    
    (* 测试3: 不同质量位设置 *)
    PRINT('--- 测试3: 质量位设置 ---\n');
    
    pressure_sensor.VAL := 1020;
    pressure_sensor.QUALITY := 1;  (* UNCERTAIN *)
    PRINT('压力(UNCERTAIN): 值=%d, 质量=%d\n', pressure_sensor.VAL, pressure_sensor.QUALITY);
    
    alarm_status.VAL := FALSE;
    alarm_status.QUALITY := 2;  (* BAD *)
    PRINT('报警(BAD): 质量=%d\n', alarm_status.QUALITY);
    
    device_id.VAL := 'ERROR_DEV';
    device_id.QUALITY := 3;  (* ERROR *)
    PRINT('设备ID(ERROR): 质量=%d\n', device_id.QUALITY);
    
    (* 测试4: 质量化类型运算 *)
    PRINT('--- 测试4: 运算测试 ---\n');
    
    temp_sensor.VAL := 20.0;
    temp_sensor.QUALITY := 0;  (* GOOD *)
    
    result.VAL := 5.0;
    result.QUALITY := 0;  (* GOOD *)
    
    result := temp_sensor + result;
    PRINT('GOOD + GOOD = 值:%f, 质量:%d\n', result.VAL, result.QUALITY);
    
    (* 测试5: 质量位传播 *)
    PRINT('--- 测试5: 质量位传播 ---\n');
    
    temp_sensor.QUALITY := 2;  (* BAD *)
    result := temp_sensor + result;
    PRINT('BAD + GOOD = 质量位:%d\n', result.QUALITY);
    
    (* 测试6: 条件判断 *)
    PRINT('--- 测试6: 条件判断 ---\n');
    
    IF temp_sensor.QUALITY = 0 THEN
        PRINT('温度数据质量良好\n');
    ELSIF temp_sensor.QUALITY = 1 THEN
        PRINT('温度数据质量不确定\n');
    ELSIF temp_sensor.QUALITY = 2 THEN
        PRINT('温度数据质量差\n');
    ELSE
        PRINT('温度数据错误\n');
    END_IF;
    
    PRINT('=== 基础测试完成 ===\n');
    
END_PROGRAM