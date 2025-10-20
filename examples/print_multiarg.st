(* 测试 PRINT 多参数 *)

PROGRAM PrintTest
VAR
    x : INT := 42;
    s : STRING := 'Hello';
END_VAR

    PRINT('Value: ', x, ', String: ', s);
END_PROGRAM
