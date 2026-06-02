PROGRAM test_member_access_simple

VAR
    temp: QREAL;
    value: REAL;
    qual: INT;
END_VAR

BEGIN
    // 设置值和质量
    temp.VAL := 25.5;
    temp.QUALITY := 0;
    
    // 读取值和质量
    value := temp.VAL;
    qual := temp.QUALITY;
    
    // 检查值是否正确
    IF value = 25.5 THEN
        PRINT('VAL access: PASS');
    ELSE
        PRINT('VAL access: FAIL');
    END_IF;
    
    // 检查质量是否正确
    IF qual = 0 THEN
        PRINT('QUALITY access: PASS');
    ELSE
        PRINT('QUALITY access: FAIL');
    END_IF;
    
    // 测试在表达式中使用
    IF temp.VAL > 20.0 THEN
        PRINT('Expression with VAL: PASS');
    END_IF;
    
    // 测试质量位修改
    temp.QUALITY := 2;
    IF temp.QUALITY = 2 THEN
        PRINT('QUALITY modification: PASS');
    END_IF;
    
    PRINT('All member access tests completed');
END_PROGRAM