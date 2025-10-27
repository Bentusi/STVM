(*
 * 质量位变量核心功能测试用例
 * 专注于最重要的质量位功能测试，确保基本功能正常
 *)

PROGRAM QualityVariablesCoreTest

VAR
    (* 基本质量化变量 *)
    temp : QREAL;
    pressure : QINT;
    status : QBOOL;
    
    (* 辅助变量 *)
    temp_value : REAL;
    quality_value : INT;
    result : QREAL;
END_VAR

BEGIN
    PRINT("=== 质量位核心功能测试 ===\n");
    
    (* 测试1: 成员访问基础功能 *)
    PRINT("--- 测试1: 成员访问 ---\n");
    
    temp.VAL := 25.0;
    temp.QUALITY := 0;  (* GOOD *)
    
    temp_value := temp.VAL;
    quality_value := temp.QUALITY;
    
    PRINT("温度值: %f\n", temp_value);
    PRINT("质量位: %d\n", quality_value);
    
    (* 测试2: 不同质量位设置 *)
    PRINT("--- 测试2: 质量位类型 ---\n");
    
    pressure.VAL := 1013;
    pressure.QUALITY := 1;  (* UNCERTAIN *)
    PRINT("压力UNCERTAIN: 值=%d, 质量=%d\n", pressure.VAL, pressure.QUALITY);
    
    status.VAL := TRUE;
    status.QUALITY := 2;  (* BAD *)
    PRINT("状态BAD: 质量=%d\n", status.QUALITY);
    
    (* 测试3: 内置函数测试 *)
    PRINT("--- 测试3: 内置函数 ---\n");
    
    result := MakeQReal(30.5, 1);
    quality_value := GetQuality(result);
    temp_value := ToReal(result);
    
    PRINT("MakeQReal(30.5, 1): 值=%f, 质量=%d\n", temp_value, quality_value);
    
    (* 测试4: 质量位运算传播 *)
    PRINT("--- 测试4: 质量位传播 ---\n");
    
    temp.VAL := 20.0;
    temp.QUALITY := 0;  (* GOOD *)
    
    result.VAL := 5.0;
    result.QUALITY := 2;  (* BAD *)
    
    result := temp + result;  (* GOOD + BAD = BAD *)
    PRINT("GOOD + BAD 结果质量位: %d", GetQuality(result));
    
    (* 测试5: 质量位条件判断 *)
    PRINT("--- 测试5: 条件判断 ---\n");
    
    IF temp.QUALITY = 0 THEN
        PRINT("温度质量良好\n");
    ELSIF temp.QUALITY = 1 THEN
        PRINT("温度质量不确定\n");
    ELSIF temp.QUALITY = 2 THEN
        PRINT("温度质量差\n");
    ELSE
        PRINT("温度质量错误\n");
    END_IF;
    
    PRINT("=== 核心功能测试完成 ===\n");
    
END_PROGRAM