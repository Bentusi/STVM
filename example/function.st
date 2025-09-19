FUNCTION max (in1 : INT, in2 : INT): INT
    IF in1 > in2 THEN
        RETURN in1;
    ELSE
        RETURN in2;
    END_IF
END_FUNCTION

FUNCTION min (in1 : INT, in2 : INT): INT
    IF in1 < in2 THEN
        RETURN in1;
    ELSE
        RETURN in2;
    END_IF
END_FUNCTION


PROGRAM test
VAR
    ma : INT;
    mi : INT;
END_VAR

    ma := max(1, 5);
    mi := min(1, 5);

END_PROGRAM