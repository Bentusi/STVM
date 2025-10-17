(* 最简单的返回测试 *)

PROGRAM SimpleTest
VAR
    result : INT;
END_VAR

FUNCTION AddTwo : INT
VAR_INPUT
    a : INT;
    b : INT;
END_VAR
    PRINT('In AddTwo: a=%d, b=%d\n', a, b);
    AddTwo := a + b;
    PRINT('In AddTwo: AddTwo=%d\n', AddTwo);
END_FUNCTION

    result := AddTwo(10, 20);
    PRINT('result = %d (expected 30)\n', result);
END_PROGRAM
