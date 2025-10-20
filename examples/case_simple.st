(* 简单 CASE 测试 *)

PROGRAM SimpleCaseTest
VAR
    x : INT := 2;
    result : STRING;
END_VAR

    CASE x OF
        1: result := 'One';
        2: result := 'Two';
        3: result := 'Three';
    ELSE
        result := 'Other';
    END_CASE;
    
    PRINT(result);
END_PROGRAM
