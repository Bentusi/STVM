(* 测试调试信息 *)

PROGRAM DebugInfo
VAR
    x : INT;
END_VAR

FUNCTION TestFunc : INT
VAR_INPUT
    a : INT;
END_VAR
VAR_LOCAL
    arr : ARRAY[3] OF INT;
END_VAR
    PRINT('a = %d\n', a);
    arr[0] := 10;
    arr[1] := 20;
    arr[2] := 30;
    TestFunc := arr[0] + arr[1] + arr[2] + a;
END_FUNCTION

    x := TestFunc(5);
    PRINT('x = %d (expected 65)\n', x);
END_PROGRAM
