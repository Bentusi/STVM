(* 数学工具函数集合 *)

FUNCTION Max : INT
VAR_INPUT
    a : INT;
    b : INT;
END_VAR
    IF a > b THEN
        RETURN a;
    ELSE
        RETURN b;
    END_IF
END_FUNCTION

FUNCTION Min : INT
VAR_INPUT
    a : INT;
    b : INT;
END_VAR
    IF a < b THEN
        RETURN a;
    ELSE
        RETURN b;
    END_IF
END_FUNCTION

FUNCTION Abs : INT
VAR_INPUT
    value : INT;
END_VAR
    IF value < 0 THEN
        RETURN -value;
    ELSE
        RETURN value;
    END_IF
END_FUNCTION

FUNCTION Power : INT
VAR_INPUT
    base : INT;
    exponent : INT;
END_VAR
VAR
    result : INT;
    i : INT;
END_VAR
    result := 1;
    
    IF exponent < 0 THEN
        RETURN 0;  (* 不支持负指数 *)
    END_IF
    
    FOR i := 1 TO exponent DO
        result := result * base;
    END_FOR
    
    RETURN result;
END_FUNCTION