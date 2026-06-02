PROGRAM StringTest
VAR
    msg1 : STRING;
    msg2 : STRING;
END_VAR

BEGIN
    msg1 := 'Hello with single quotes';
    msg2 := "Hello with double quotes";
    PRINT(msg1);
    PRINT(msg2);
END_PROGRAM
