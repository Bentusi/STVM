(* 更新版本 - 用于热更新测试 *)

PROGRAM HotReloadTest_V2

VAR
    counter : INT;
    result : INT;
END_VAR

FUNCTION Calculate : INT
VAR_INPUT
    x : INT;
END_VAR
    Calculate := x * 3 + 10;  (* 版本2: 修改算法 *)
END_FUNCTION

(* 新增函数 *)
FUNCTION Square : INT
VAR_INPUT
    n : INT;
END_VAR
    Square := n * n;
END_FUNCTION

    counter := 0;
    PRINT('=== Hot Reload Test V2 ===\n');
    
    FOR counter := 1 TO 5 DO
        result := Calculate(counter);
        PRINT('Calculate(%d) = %d\n', counter, result);
    END_FOR;
    
    PRINT('\nNew function test:\n');
    FOR counter := 1 TO 3 DO
        result := Square(counter);
        PRINT('Square(%d) = %d\n', counter, result);
    END_FOR;
    
    PRINT('Version: 2.0\n');
    PRINT('=========================\n');

END_PROGRAM
