PROGRAM test_simple

VAR
    x : INT;
END_VAR

FUNCTION TestFunc : INT
VAR
    counter : INT := 5;
END_VAR
    TestFunc := counter;
END_FUNCTION

x := TestFunc();
PRINT('Result: ', x);

END_PROGRAM
