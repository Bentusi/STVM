(* 示例程序 3: 函数定义和调用 *)

PROGRAM Functions
VAR
    a : INT := 5;
    b : INT := 3;
    result : INT;
    fact : INT;
END_VAR

(* 计算两个数的和 *)
FUNCTION Add : INT
VAR_INPUT
    x : INT;
    y : INT;
END_VAR
    Add := x + y;
END_FUNCTION

(* 计算阶乘 *)
FUNCTION Factorial : INT
VAR_INPUT
    n : INT;
END_VAR
VAR
    i : INT;
    temp : INT;
END_VAR
    temp := 1;
    FOR i := 1 TO n DO
        temp := temp * i;
    END_FOR;
    Factorial := temp;
END_FUNCTION

    (* 调用函数 *)
    result := Add(a, b);
    fact := Factorial(5);
END_PROGRAM
