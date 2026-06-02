(* 逐步调试局部数组 *)

PROGRAM DebugLocalArray
VAR
    result : INT;
END_VAR

FUNCTION TestArray : INT
VAR_LOCAL
    nums : ARRAY[3] OF INT;
END_VAR
    PRINT('Setting nums[0] = 10\n');
    nums[0] := 10;
    PRINT('nums[0] is now %d\n', nums[0]);
    
    PRINT('Setting nums[1] = 20\n');
    nums[1] := 20;
    PRINT('nums[1] is now %d\n', nums[1]);
    
    PRINT('Setting nums[2] = 30\n');
    nums[2] := 30;
    PRINT('nums[2] is now %d\n', nums[2]);
    
    PRINT('Computing sum...\n');
    result := nums[0];
    PRINT('After nums[0]: result = %d\n', result);
    
    result := result + nums[1];
    PRINT('After nums[1]: result = %d\n', result);
    
    result := result + nums[2];
    PRINT('After nums[2]: result = %d\n', result);
    
    TestArray := result;
END_FUNCTION

    result := TestArray();
    PRINT('Final result: %d\n', result);
END_PROGRAM
