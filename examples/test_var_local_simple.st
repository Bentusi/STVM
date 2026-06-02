PROGRAM test_var_local

VAR
    gx : INT := 10;
END_VAR

FUNCTION TestLocal : INT
VAR_LOCAL
    lx : INT := 20;
END_VAR
    lx := lx + gx;
    TestLocal := lx;
END_FUNCTION

gx := TestLocal();
PRINT('Result: ', gx);

END_PROGRAM
