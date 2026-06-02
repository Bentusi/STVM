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
        RETURN 0;
    END_IF
    
    FOR i := 1 TO exponent DO
        result := result * base;
    END_FOR
    
    RETURN result;
END_FUNCTION