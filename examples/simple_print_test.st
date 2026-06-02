PROGRAM TestPrint

VAR
    test : INT := 42;
END_VAR

FUNCTION MAIN : INT
BEGIN
    PRINT('Hello World!\n');
    PRINT('Test value: %d\n', test);
    MAIN := 0;
END_FUNCTION

END_PROGRAM