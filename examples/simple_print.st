(* 简单的 PRINT 测试 *)

PROGRAM SimplePrintTest
VAR
    x : INT := 42;
END_VAR

    PRINT('Hello, World!\n');
    PRINT('x = %d\n', x);
    PRINT('Done!\n');
    
END_PROGRAM
