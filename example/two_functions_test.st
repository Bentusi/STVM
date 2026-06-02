FUNCTION Add : INT
VAR_INPUT
    a : INT;
    b : INT;
END_VAR
    RETURN a + b;
END_FUNCTION

FUNCTION Multiply : INT
VAR_INPUT
    x : INT;
    y : INT;
END_VAR
    RETURN x * y;
END_FUNCTION

PROGRAM TwoFunctionsTest
VAR
    result1 : INT;
    result2 : INT;
END_VAR
    result1 := Add(3, 4);
    result2 := Multiply(5, 6);
END_PROGRAM