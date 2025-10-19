(* 内联数学函数测试 - 不使用外部库 *)

PROGRAM TestMathInline

VAR
    result : INT;
END_VAR

(* 本地定义数学函数 *)
FUNCTION Power : INT
VAR_INPUT
    base : INT;
    exp : INT;
END_VAR
VAR_LOCAL
    i : INT;
    res : INT;
END_VAR
    res := 1;
    FOR i := 1 TO exp DO
        res := res * base;
    END_FOR;
    Power := res;
END_FUNCTION

FUNCTION Factorial : INT
VAR_INPUT
    n : INT;
END_VAR
VAR_LOCAL
    i : INT;
    res : INT;
END_VAR
    res := 1;
    FOR i := 1 TO n DO
        res := res * i;
    END_FOR;
    Factorial := res;
END_FUNCTION

(* 测试 *)
PRINT('=== Math Functions Test ===\n\n');

result := Power(2, 10);
PRINT('Power(2, 10) = %d (expected: 1024)\n', result);

result := Factorial(5);
PRINT('Factorial(5) = %d (expected: 120)\n', result);

result := Power(3, 4);
PRINT('Power(3, 4) = %d (expected: 81)\n', result);

result := Factorial(6);
PRINT('Factorial(6) = %d (expected: 720)\n', result);

PRINT('\n=== Test Complete ===\n');

END_PROGRAM
