(* REPEAT...UNTIL 循环测试 *)

PROGRAM RepeatTest
VAR
    counter : INT := 0;
    sum : INT := 0;
END_VAR

    (* 测试1: 基本的 REPEAT 循环 *)
    counter := 1;
    REPEAT
        PRINT('Counter: %d\n', counter);
        counter := counter + 1;
    UNTIL counter > 5
    END_REPEAT;
    
    (* 测试2: 计算累加和 *)
    counter := 1;
    sum := 0;
    REPEAT
        sum := sum + counter;
        counter := counter + 1;
    UNTIL counter > 10
    END_REPEAT;
    
    PRINT('Sum 1-10: %d\n', sum);
    
    (* 测试3: 嵌套 REPEAT 循环 *)
    counter := 1;
    REPEAT
        PRINT('Outer: %d\n', counter);
        
        sum := 1;
        REPEAT
            PRINT('  Inner: %d\n', sum);
            sum := sum + 1;
        UNTIL sum > 3
        END_REPEAT;
        
        counter := counter + 1;
    UNTIL counter > 2
    END_REPEAT;
    
END_PROGRAM
