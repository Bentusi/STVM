(* 递归函数示例 - 计算阶乘 *)

FUNCTION Factorial : INT
VAR_INPUT
    n : INT;
END_VAR
    IF n <= 1 THEN
        RETURN 1;
    ELSE
        RETURN n * Factorial(n - 1);
    END_IF
END_FUNCTION