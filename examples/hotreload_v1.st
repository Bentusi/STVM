(* 原始版本 - 用于热更新测试 *)

PROGRAM HotReloadTest_V1

VAR
    counter : INT;
    result : INT;
END_VAR

FUNCTION Calculate : INT
VAR_INPUT
    x : INT;
END_VAR
    Calculate := x * 2;  (* 版本1: 乘以2 *)
END_FUNCTION

    counter := 0;
    PRINT('=== Hot Reload Test V1 ===\n');
    
    FOR counter := 1 TO 5 DO
        result := Calculate(counter);
        PRINT('Calculate(%d) = %d\n', counter, result);
    END_FOR;
    
    PRINT('Version: 1.0\n');
    PRINT('=========================\n');

END_PROGRAM
