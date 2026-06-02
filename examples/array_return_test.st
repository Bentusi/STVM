(* 测试局部数组和返回值 *)

PROGRAM ArrayReturnTest
VAR
    result : INT;
END_VAR

FUNCTION SumArray : INT
VAR_INPUT
    x : INT;
END_VAR
VAR_LOCAL
    arr : ARRAY[2] OF INT;
END_VAR
    PRINT('x = %d\n', x);
    arr[0] := 100;
    PRINT('arr[0] = %d\n', arr[0]);
    arr[1] := 200;
    PRINT('arr[1] = %d\n', arr[1]);
    
    PRINT('Computing: x + arr[0] + arr[1] = %d + %d + %d\n', x, arr[0], arr[1]);
    SumArray := x + arr[0] + arr[1];
    PRINT('SumArray = %d (expected 305)\n', SumArray);
END_FUNCTION

    result := SumArray(5);
    PRINT('result = %d (expected 305)\n', result);
END_PROGRAM
