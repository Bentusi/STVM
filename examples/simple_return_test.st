(* 最简单的函数返回测试 *)

PROGRAM SimpleReturn
VAR
    x : INT;
END_VAR

FUNCTION GetFive : INT
VAR_LOCAL
    dummy : INT;
END_VAR
    GetFive := 5;
END_FUNCTION

    x := GetFive();
    PRINT('x = %d\n', x);
END_PROGRAM
