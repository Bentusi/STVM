(* 超级简单的测试 *)

PROGRAM TinyTest
VAR
END_VAR

FUNCTION Five : INT
VAR_LOCAL
    x : INT;
END_VAR
    Five := 5;
END_FUNCTION

    PRINT('%d\n', Five());
END_PROGRAM
