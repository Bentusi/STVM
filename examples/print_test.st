(* 示例程序：PRINT 函数测试 *)
(* 测试格式化输出功能，类似 C 语言的 printf *)

PROGRAM PrintTest
VAR
    x : INT := 42;
    y : REAL := 3.14;
    flag : BOOL := TRUE;
    name : STRING := 'STVM';
    numbers : ARRAY[5] OF INT;
    i : INT;
    sum : INT := 0;
END_VAR

    (* 基本输出测试 *)
    PRINT('Hello, World!\n');
    PRINT('Welcome to STVM!\n\n');
    
    (* 整数输出 *)
    PRINT('Integer test:\n');
    PRINT('x = %d\n', x);
    PRINT('x * 2 = %d\n', x * 2);
    PRINT('\n');
    
    (* 浮点数输出 *)
    PRINT('Real number test:\n');
    PRINT('y = %f\n', y);
    PRINT('y * 2 = %f\n', y * 2.0);
    PRINT('\n');
    
    (* 布尔值输出 *)
    PRINT('Boolean test:\n');
    PRINT('flag = %b\n', flag);
    PRINT('NOT flag = %b\n', NOT flag);
    PRINT('\n');
    
    (* 字符串输出 *)
    PRINT('String test:\n');
    PRINT('name = %s\n', name);
    PRINT('Hello, %s!\n', name);
    PRINT('\n');
    
    (* 混合输出 *)
    PRINT('Mixed format test:\n');
    PRINT('x=%d, y=%f, flag=%b, name=%s\n', x, y, flag, name);
    PRINT('\n');
    
    (* 数组和循环输出 *)
    PRINT('Array test:\n');
    FOR i := 0 TO 4 DO
        numbers[i] := i * 10;
    END_FOR;
    
    FOR i := 0 TO 4 DO
        PRINT('numbers[%d] = %d\n', i, numbers[i]);
        sum := sum + numbers[i];
    END_FOR;
    
    PRINT('Sum = %d\n', sum);
    PRINT('\n');
    
    (* 转义序列测试 *)
    PRINT('Escape sequences:\n');
    PRINT('\tTab test\n');
    PRINT('Line 1\nLine 2\n');
    PRINT('50%% progress\n');
    PRINT('\n');
    
    (* 完成 *)
    PRINT('All tests completed!\n');
    
END_PROGRAM
