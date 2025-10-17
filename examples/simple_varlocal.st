(* 简化的 VAR_LOCAL 测试 *)
(* 只使用 VAR_LOCAL 定义函数局部变量 *)

PROGRAM SimpleVarLocalTest
VAR
    global_x : INT := 100;
    global_y : INT := 200;
    result : INT;
END_VAR

FUNCTION Calculate : INT
VAR_INPUT
    a : INT;
    b : INT;
END_VAR
VAR_LOCAL
    temp1 : INT;
    temp2 : INT;
    sum : INT;
END_VAR
    temp1 := a * 2;
    temp2 := b * 3;
    sum := temp1 + temp2 + global_x;
    PRINT('Calculate: temp1=%d, temp2=%d, sum=%d\n', temp1, temp2, sum);
    Calculate := sum;
END_FUNCTION

FUNCTION ProcessArray : INT
VAR_INPUT
    size : INT;
END_VAR
VAR_LOCAL
    i : INT;
    total : INT;
    numbers : ARRAY[5] OF INT;
END_VAR
    total := 0;
    FOR i := 0 TO size - 1 DO
        numbers[i] := i + 10;
        total := total + numbers[i];
        PRINT('  numbers[%d] = %d\n', i, numbers[i]);
    END_FOR;
    PRINT('ProcessArray: total=%d\n', total);
    ProcessArray := total;
END_FUNCTION

    PRINT('=== Simple VAR_LOCAL Test ===\n');
    PRINT('global_x=%d, global_y=%d\n', global_x, global_y);
    
    PRINT('\nTest 1: Calculate(10, 20)\n');
    result := Calculate(10, 20);
    PRINT('Result: %d\n', result);
    
    PRINT('\nTest 2: ProcessArray(5)\n');
    result := ProcessArray(5);
    PRINT('Result: %d\n', result);
    
    PRINT('\nTest 3: Calculate(1, 1)\n');
    result := Calculate(1, 1);
    PRINT('Result: %d\n', result);
    
    PRINT('\nAll tests completed!\n');
    
END_PROGRAM
