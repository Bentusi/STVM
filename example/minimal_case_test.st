PROGRAM MinimalCaseTest
VAR
    test_input : INT;
    result : INT;
END_VAR

test_input := 2;
result := 0;

CASE test_input OF
    1: result := 11;
    2: result := 22;
    3: result := 33;
END_CASE

END_PROGRAM