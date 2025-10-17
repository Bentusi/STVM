(* 简化的局部数组测试 *)

PROGRAM SimpleLocalArrayTest
VAR
    result : INT;
END_VAR

FUNCTION Sum3 : INT
VAR_LOCAL
    nums : ARRAY[3] OF INT;
END_VAR
    nums[0] := 10;
    nums[1] := 20;
    nums[2] := 30;
    
    Sum3 := nums[0] + nums[1] + nums[2];
END_FUNCTION

    result := Sum3();
    PRINT('Result: %d\n', result);
END_PROGRAM
