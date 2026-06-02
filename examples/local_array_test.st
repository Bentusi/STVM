(* 测试函数内的局部数组 *)

PROGRAM LocalArrayTest
VAR
    result : INT;
END_VAR

FUNCTION TestLocalArray : INT
VAR_LOCAL
    numbers : ARRAY[3] OF INT;
    i : INT;
END_VAR
    PRINT('Initializing local array...\n');
    
    numbers[0] := 10;
    PRINT('numbers[0] = %d\n', numbers[0]);
    
    numbers[1] := 20;
    PRINT('numbers[1] = %d\n', numbers[1]);
    
    numbers[2] := 30;
    PRINT('numbers[2] = %d\n', numbers[2]);
    
    TestLocalArray := numbers[0] + numbers[1] + numbers[2];
END_FUNCTION

    PRINT('Testing local arrays in functions...\n');
    result := TestLocalArray();
    PRINT('Result: %d\n', result);
    
END_PROGRAM
