
FUNCTION max (in1 : INT, in2 : INT): INT
    IF in1 > in2 THEN
        RETURN in1;
    ELSE
        RETURN in2;
    END_IF
END_FUNCTION

FUNCTION max (in1 : INT, in2 : INT): INT
    IF in1 < in2 THEN
        RETURN in1;
    ELSE
        RETURN in2;
    END_IF
END_FUNCTION


PROGRAM test
VAR
    max : INT;
    min : INT;
END_VAR

    max := max(1, 5);
    min := min(1, 5);

END_PROGRAM