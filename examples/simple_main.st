FUNCTION main : INT
VAR
    x : INT := 42;
END_VAR
    x := x + 1;
    main := x;
END_FUNCTION

PROGRAM TestProgram
VAR
    result : INT;
END_VAR
    result := main();
END_PROGRAM