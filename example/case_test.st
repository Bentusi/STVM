PROGRAM CaseTest
VAR
    state : INT;
    result : REAL;
END_VAR

state := 10;
result := 0.0;

IF state >= 10 THEN
    IF state >= 10 THEN
        state := 3;
    END_IF
END_IF

CASE state OF
    1: result := 1.1;
    2: result := 2.2;
    3: result := 3.3;
END_CASE

END_PROGRAM