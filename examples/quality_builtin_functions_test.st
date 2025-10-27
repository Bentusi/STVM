(*
 * 质量位内置函数测试用例
 * 测试MakeQReal, GetQuality, ToReal等内置函数
 *)

PROGRAM QualityBuiltinFunctionsTest

VAR
    (* 质量化变量 *)
    temp1 : QREAL;
    temp2 : QREAL;
    pressure1 : QINT;
    status1 : QBOOL;
    
    (* 普通类型变量 *)
    normal_temp : REAL;
    normal_pressure : INT;
    normal_status : BOOL;
    quality_code : INT;
    
    (* 测试用常量 *)
    test_value : REAL;
    test_quality : INT;
END_VAR

BEGIN
    PRINT('=== 质量位内置函数测试 ===\n');
    
    (* 测试1: MakeQReal函数 *)
    PRINT('--- 测试1: MakeQReal函数 ---\n');
    
    (* 创建不同质量位的QREAL值 *)
    temp1 := MakeQReal(25.5, 0);  (* GOOD *)
    temp2 := MakeQReal(30.0, 1);  (* UNCERTAIN *)
    
    PRINT('MakeQReal(25.5, 0): ', temp1);
    PRINT('MakeQReal(30.0, 1): ', temp2);
    
    (* 使用变量作为参数 *)
    test_value := 28.7;
    test_quality := 2;  (* BAD *)
    temp1 := MakeQReal(test_value, test_quality);
    PRINT('MakeQReal(变量): 值=', temp1.VAL, ', 质量=', temp1.QUALITY);
    
    (* 测试2: GetQuality函数 *)
    PRINT('--- 测试2: GetQuality函数 ---\n');
    
    temp1.VAL := 22.0;
    temp1.QUALITY := 0;  (* GOOD *)
    quality_code := GetQuality(temp1);
    PRINT('GetQuality(GOOD值): ', quality_code);
    
    temp1.QUALITY := 3;  (* ERROR *)
    quality_code := GetQuality(temp1);
    PRINT('GetQuality(ERROR值): ', quality_code);
    
    (* 测试3: ToReal函数 *)
    PRINT('--- 测试3: ToReal函数 ---\n');
    
    temp1.VAL := 35.5;
    temp1.QUALITY := 1;  (* UNCERTAIN *)
    normal_temp := ToReal(temp1);
    PRINT('ToReal从QREAL提取: ', normal_temp);
    
    (* 对不同质量位的值使用ToReal *)
    temp2 := MakeQReal(40.0, 2);  (* BAD *)
    normal_temp := ToReal(temp2);
    PRINT('ToReal从BAD质量值提取: ', normal_temp);
    
    (* 测试4: 综合使用内置函数 *)
    PRINT('--- 测试4: 函数组合使用 ---\n');
    
    (* 创建值 -> 检查质量 -> 提取值 *)
    temp1 := MakeQReal(18.5, 1);
    quality_code := GetQuality(temp1);
    normal_temp := ToReal(temp1);
    
    PRINT('组合操作: 原值=18.5, 质量=1\n');
    PRINT('GetQuality结果: ', quality_code);
    PRINT('ToReal结果: ', normal_temp);
    
    (* 测试5: 在表达式中使用内置函数 *)
    PRINT('--- 测试5: 表达式中的内置函数 ---\n');
    
    temp1 := MakeQReal(20.0, 0);
    temp2 := MakeQReal(10.0, 1);
    
    (* 在条件表达式中使用 *)
    IF GetQuality(temp1) = 0 THEN
        PRINT('temp1质量良好，值=', ToReal(temp1));
    END_IF;
    
    IF GetQuality(temp2) <> 0 THEN
        PRINT('temp2质量不良，值=', ToReal(temp2), ', 质量=', GetQuality(temp2));
    END_IF;
    
    (* 测试6: 边界情况 *)
    PRINT('--- 测试6: 边界情况测试 ---\n');
    
    (* 极值测试 *)
    temp1 := MakeQReal(0.0, 0);
    PRINT('零值GOOD: ', ToReal(temp1), ', 质量=', GetQuality(temp1));
    
    temp1 := MakeQReal(999999.99, 3);
    PRINT('大值ERROR: ', ToReal(temp1), ', 质量=', GetQuality(temp1));
    
    temp1 := MakeQReal(-273.15, 2);
    PRINT('负值BAD: ', ToReal(temp1), ', 质量=', GetQuality(temp1));
    
    (* 测试7: 函数链式调用 *)
    PRINT('--- 测试7: 链式调用 ---\n');
    
    (* 创建一个值，修改其质量位，再提取 *)
    normal_temp := ToReal(MakeQReal(15.5, GetQuality(MakeQReal(0.0, 1))));
    PRINT('链式调用结果: ', normal_temp);
    
    (* 测试8: 函数参数验证 *)
    PRINT('--- 测试8: 参数验证 ---\n');
    
    (* 使用变量作为质量位参数 *)
    quality_code := 0;
    temp1 := MakeQReal(25.0, quality_code);
    PRINT('使用变量质量位: 质量=', GetQuality(temp1));
    
    quality_code := 3;
    temp1 := MakeQReal(25.0, quality_code);
    PRINT('变量质量位ERROR: 质量=', GetQuality(temp1));
    
    (* 测试9: 运算结果与内置函数配合 *)
    PRINT('--- 测试9: 运算结果处理 ---\n');
    
    temp1 := MakeQReal(10.0, 0);  (* GOOD *)
    temp2 := MakeQReal(5.0, 2);   (* BAD *)
    
    (* 运算后检查质量 *)
    temp1 := temp1 + temp2;
    quality_code := GetQuality(temp1);
    normal_temp := ToReal(temp1);
    
    PRINT('运算后: 值=', normal_temp, ', 质量=', quality_code);
    
    (* 根据质量位决定是否使用结果 *)
    IF GetQuality(temp1) <= 1 THEN
        PRINT('运算结果可用: ', ToReal(temp1));
    ELSE
        PRINT('运算结果不可用，质量位=', GetQuality(temp1));
    END_IF;
    
    (* 测试10: 类型转换场景 *)
    PRINT('--- 测试10: 类型处理 ---\n');
    
    (* QREAL到REAL的转换 *)
    temp1 := MakeQReal(123.456, 0);
    normal_temp := ToReal(temp1);
    PRINT('精度保持: QREAL(123.456) -> REAL(', normal_temp, ')');
    
    (* 整数值在QREAL中的处理 *)
    temp1 := MakeQReal(42.0, 1);
    normal_temp := ToReal(temp1);
    PRINT('整数值: QREAL(42.0) -> REAL(', normal_temp, ')');
    
    PRINT('=== 内置函数测试完成 ===\n');
    
END_PROGRAM