FUNCTION Add : INT
VAR_INPUT
    a : INT;
    b : INT;
END_VAR
    RETURN a + b;
END_FUNCTION

FUNCTION Max : INT
VAR_INPUT
    x : INT;
    y : INT;
END_VAR
    IF x > y THEN
        RETURN x;
    ELSE
        RETURN y;
    END_IF
END_FUNCTION

FUNCTION Min : INT
VAR_INPUT
    x : INT;
    y : INT;
END_VAR
    IF x < y THEN
        RETURN x;
    ELSE
        RETURN y;
    END_IF
END_FUNCTION

PROGRAM TestMultipleFunctions
VAR
    num1 : INT;
    num2 : INT;
    result : INT;
END_VAR
    num1 := 15;
    num2 := 25;
    
    result := Add(num1, num2);
    result := Max(num1, num2);
    result := Min(num1, num2);
END_PROGRAM