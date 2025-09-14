FUNCTION Add : INT
VAR_INPUT
    a : INT;
    b : INT;
END_VAR
    RETURN a + b;
END_FUNCTION

PROGRAM SimpleTest
VAR
    result : INT;
END_VAR
    result := Add(5, 3);
END_PROGRAM