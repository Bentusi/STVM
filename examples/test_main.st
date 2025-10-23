(* 测试程序：包含main函数的示例 *)

PROGRAM TestProgram
VAR
    counter : INT := 0;
    result : REAL := 1.0;
    main_result : INT;
    test_result : INT;
END_VAR

(* 主函数 *)
FUNCTION main : INT
VAR
    local_var : INT := 1;
END_VAR
    local_var := local_var + 1;
    main := local_var;
END_FUNCTION

(* 另一个测试函数 *)
FUNCTION test_func : INT
VAR
    x : INT := 100;
END_VAR
    x := x * 2;
    test_func := x;
END_FUNCTION

    counter := 10;
    result := 3.14;
    counter := counter + 5;

END_PROGRAM