(*
 * 带质量位变量综合测试用例
 * 测试STVM中质量化数据类型(QINT, QREAL, QBOOL, QSTRING)的各种操作
 * 包括成员访问、质量位传播、运算规则等
 *)

PROGRAM QualityVariablesComprehensiveTest

VAR
    (* 质量化数据类型变量声明 *)
    sensor_temp : QREAL;        (* 传感器温度 - 质量化实数 *)
    sensor_pressure : QINT;     (* 传感器压力 - 质量化整数 *)
    system_status : QBOOL;      (* 系统状态 - 质量化布尔 *)
    device_name : QSTRING;      (* 设备名称 - 质量化字符串 *)
    
    (* 计算用的质量化变量 *)
    calculated_temp : QREAL;
    calculated_pressure : QINT;
    result_flag : QBOOL;
    
    (* 普通类型变量（用于混合运算测试） *)
    normal_temp : REAL;
    normal_pressure : INT;
    normal_flag : BOOL;
    temp_offset : REAL;
    pressure_adjust : INT;
    
    (* 用于质量位操作的变量 *)
    quality_code : INT;
    temp_value : REAL;
    pressure_value : INT;
    status_value : BOOL;
    
    (* 循环和控制变量 *)
    i : INT;
    test_count : INT;
END_VAR

BEGIN
    PRINT('=== 带质量位变量综合测试开始 ===\n');
    
    (* ========== 测试1: 基本质量化变量初始化 ========== *)
    PRINT('--- 测试1: 基本初始化 ---\n');
    
    (* 直接赋值（默认质量位为GOOD=0） *)
    sensor_temp := 25.5;
    sensor_pressure := 1013;
    system_status := TRUE;
    device_name := 'SENSOR_01';
    
    PRINT('传感器温度已初始化\n');
    PRINT('传感器压力已初始化\n');
    PRINT('系统状态已初始化\n');
    PRINT('设备名称已初始化\n');
    
    (* ========== 测试2: 成员访问(.VAL 和 .QUALITY) ========== *)
    PRINT('--- 测试2: 成员访问测试 ---\n');
    
    (* 设置具体的值和质量位 *)
    sensor_temp.VAL := 28.7;
    sensor_temp.QUALITY := 0;  (* GOOD *)
    
    sensor_pressure.VAL := 1015;
    sensor_pressure.QUALITY := 1;  (* UNCERTAIN *)
    
    system_status.VAL := FALSE;
    system_status.QUALITY := 2;  (* BAD *)
    
    device_name.VAL := 'TEMP_SENSOR';
    device_name.QUALITY := 0;  (* GOOD *)
    
    (* 读取成员值 *)
    temp_value := sensor_temp.VAL;
    quality_code := sensor_temp.QUALITY;
    PRINT('温度值: %f, 质量位: %d\n', temp_value, quality_code);
    
    pressure_value := sensor_pressure.VAL;
    quality_code := sensor_pressure.QUALITY;
    PRINT('压力值: %d, 质量位: %d\n', pressure_value, quality_code);
    
    status_value := system_status.VAL;
    quality_code := system_status.QUALITY;
    PRINT('状态值: %b, 质量位: %d\n', status_value, quality_code);
    
    (* ========== 测试3: 质量化类型之间的运算 ========== *)
    PRINT('--- 测试3: 质量化类型运算 ---\n');
    
    (* 重新设置测试数据 *)
    sensor_temp.VAL := 20.0;
    sensor_temp.QUALITY := 0;  (* GOOD *)
    
    calculated_temp.VAL := 5.0;
    calculated_temp.QUALITY := 0;  (* GOOD *)
    
    (* 质量化类型加法（GOOD + GOOD = GOOD） *)
    calculated_temp := sensor_temp + calculated_temp;
    PRINT('温度相加结果: 值=%f, 质量位=%d\n', calculated_temp.VAL, calculated_temp.QUALITY);
    
    (* 设置一个BAD质量位的值 *)
    sensor_temp.QUALITY := 2;  (* BAD *)
    
    (* GOOD + BAD = BAD（质量位传播） *)
    calculated_temp := sensor_temp + calculated_temp;
    PRINT('包含BAD质量位的运算结果质量位: %d\n', calculated_temp.QUALITY);
    
    (* ========== 测试4: 质量化类型与普通类型混合运算 ========== *)
    PRINT('--- 测试4: 混合类型运算 ---\n');
    
    sensor_temp.VAL := 25.0;
    sensor_temp.QUALITY := 0;  (* GOOD *)
    temp_offset := 3.5;
    
    (* QREAL + REAL = QREAL *)
    calculated_temp := sensor_temp + temp_offset;
    PRINT('质量化温度 + 普通偏移 = 值=%f, 质量位=%d\n', calculated_temp.VAL, calculated_temp.QUALITY);
    
    sensor_pressure.VAL := 1000;
    sensor_pressure.QUALITY := 1;  (* UNCERTAIN *)
    pressure_adjust := 50;
    
    (* QINT + INT = QINT *)
    calculated_pressure := sensor_pressure + pressure_adjust;
    PRINT('质量化压力 + 普通调整 = 值=%d, 质量位=%d\n', calculated_pressure.VAL, calculated_pressure.QUALITY);
    
    (* ========== 测试5: 质量位条件判断 ========== *)
    PRINT('--- 测试5: 基于质量位的条件判断 ---\n');
    
    (* 根据质量位进行不同处理 *)
    IF sensor_temp.QUALITY = 0 THEN
        PRINT('温度数据质量良好，可直接使用\n');
        normal_temp := sensor_temp.VAL;
    ELSIF sensor_temp.QUALITY = 1 THEN
        PRINT('温度数据质量不确定，需要验证\n');
        normal_temp := sensor_temp.VAL;  (* 可以使用但需标记 *)
    ELSIF sensor_temp.QUALITY = 2 THEN
        PRINT('温度数据质量差，使用默认值\n');
        normal_temp := 20.0;  (* 使用默认值 *)
    ELSE
        PRINT('温度数据错误，系统报警\n');
        normal_temp := 0.0;
        system_status.VAL := FALSE;
        system_status.QUALITY := 3;  (* ERROR *)
    END_IF;
    
    (* ========== 测试6: 复杂表达式中的质量位处理 ========== *)
    PRINT('--- 测试6: 复杂表达式测试 ---\n');
    
    sensor_temp.VAL := 22.5;
    sensor_temp.QUALITY := 0;  (* GOOD *)
    sensor_pressure.VAL := 1020;
    sensor_pressure.QUALITY := 0;  (* GOOD *)
    
    (* 复杂条件判断，同时检查值和质量位 *)
    IF (sensor_temp.VAL > 20.0) AND (sensor_temp.QUALITY = 0) AND 
       (sensor_pressure.VAL > 1000) AND (sensor_pressure.QUALITY = 0) THEN
        PRINT('环境条件正常且数据可靠\n');
        result_flag.VAL := TRUE;
        result_flag.QUALITY := 0;
    ELSE
        PRINT('环境条件异常或数据不可靠\n'); 
        result_flag.VAL := FALSE;
        result_flag.QUALITY := 1;  (* UNCERTAIN *)
    END_IF;
    
    (* ========== 测试7: 动态质量位管理 ========== *)
    PRINT('--- 测试7: 动态质量位管理 ---\n');
    
    test_count := 5;
    FOR i := 1 TO test_count DO
        (* 模拟传感器读数变化 *)
        sensor_temp.VAL := 20.0 + i * 2.0;
        
        (* 根据值的范围动态设置质量位 *)
        IF sensor_temp.VAL < 15.0 THEN
            sensor_temp.QUALITY := 2;  (* BAD - 温度过低 *)
        ELSIF sensor_temp.VAL > 35.0 THEN
            sensor_temp.QUALITY := 2;  (* BAD - 温度过高 *)
        ELSIF (sensor_temp.VAL < 18.0) OR (sensor_temp.VAL > 32.0) THEN
            sensor_temp.QUALITY := 1;  (* UNCERTAIN - 边界值 *)
        ELSE
            sensor_temp.QUALITY := 0;  (* GOOD - 正常范围 *)
        END_IF;
        
        PRINT('第%d次测试: 温度=%f, 质量位=%d\n', i, sensor_temp.VAL, sensor_temp.QUALITY);
        
        (* 基于质量位的处理逻辑 *)
        IF sensor_temp.QUALITY = 0 THEN
            calculated_temp := sensor_temp * 1.1;  (* 正常处理 *)
        ELSIF sensor_temp.QUALITY = 1 THEN
            calculated_temp := sensor_temp * 1.0;  (* 保守处理 *)
        ELSE
            calculated_temp.VAL := 25.0;  (* 使用默认值 *)
            calculated_temp.QUALITY := 2;  (* 标记为BAD *)
        END_IF;
    END_FOR;
    
    (* ========== 测试8: 质量位比较和逻辑运算 ========== *)
    PRINT('--- 测试8: 质量位比较运算 ---\n');
    
    sensor_temp.QUALITY := 0;    (* GOOD *)
    sensor_pressure.QUALITY := 1; (* UNCERTAIN *)
    system_status.QUALITY := 2;   (* BAD *)
    
    (* 检查哪些数据质量良好 *)
    IF sensor_temp.QUALITY = 0 THEN
        PRINT('温度数据质量: GOOD\n');
    END_IF;
    
    IF sensor_pressure.QUALITY <> 0 THEN
        PRINT('压力数据质量: 非GOOD\n');
    END_IF;
    
    (* 检查整体数据质量 *)
    IF (sensor_temp.QUALITY = 0) AND (sensor_pressure.QUALITY <= 1) THEN
        PRINT('核心传感器数据质量可接受\n');
    ELSE
        PRINT('核心传感器数据质量不可接受\n');
    END_IF;
    
    (* ========== 测试9: 质量位重置和恢复 ========== *)
    PRINT('--- 测试9: 质量位重置测试 ---\n');
    
    (* 保存原始质量位 *)
    quality_code := sensor_temp.QUALITY;
    
    (* 模拟质量位恶化 *)
    sensor_temp.QUALITY := 3;  (* ERROR *)
    PRINT('质量位恶化为: %d\n', sensor_temp.QUALITY);
    
    (* 恢复质量位 *)
    sensor_temp.QUALITY := quality_code;
    PRINT('质量位恢复为: %d\n', sensor_temp.QUALITY);
    
    (* ========== 测试10: 使用内置函数操作质量位 ========== *)
    PRINT('--- 测试10: 内置函数测试 ---\n');
    
    (* 使用MakeQReal创建指定质量位的值 *)
    calculated_temp := MakeQReal(30.5, 1);  (* 值=30.5, 质量位=UNCERTAIN *)
    PRINT('使用MakeQReal创建: 值=%f, 质量位=%d\n', ToReal(calculated_temp), GetQuality(calculated_temp));
    
    (* 测试质量位获取 *)
    quality_code := GetQuality(sensor_temp);
    PRINT('当前温度质量位: %d\n', quality_code);
    
    (* 提取普通值 *)
    normal_temp := ToReal(sensor_temp);
    PRINT('提取的温度值: %f\n', normal_temp);
    
    PRINT('=== 带质量位变量综合测试完成 ===\n');
    
END_PROGRAM