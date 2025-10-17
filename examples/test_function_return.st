(* 测试函数返回值 - 使用函数名作为返回值变量 *)

PROGRAM TestFunctionReturn
VAR
    result : INT;
END_VAR

FUNCTION Sum3Nums : INT
VAR_LOCAL
    nums : ARRAY[3] OF INT;
END_VAR
    PRINT('Setting up array...\n');
    nums[0] := 10;
    nums[1] := 20;
    nums[2] := 30;
    
    PRINT('Computing sum...\n');
    Sum3Nums := nums[0];
    PRINT('After nums[0]: Sum3Nums = %d\n', Sum3Nums);
    
    Sum3Nums := Sum3Nums + nums[1];
    PRINT('After nums[1]: Sum3Nums = %d\n', Sum3Nums);
    
    Sum3Nums := Sum3Nums + nums[2];
    PRINT('After nums[2]: Sum3Nums = %d\n', Sum3Nums);
END_FUNCTION

    PRINT('Calling Sum3Nums...\n');
    result := Sum3Nums();
    PRINT('Final result: %d\n', result);
END_PROGRAM
