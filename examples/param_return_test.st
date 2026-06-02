(* 测试有参数的函数返回 *)

PROGRAM ParamReturn
VAR
    result : INT;
END_VAR

FUNCTION Double : INT
VAR_INPUT
    n : INT;
END_VAR
    Double := n * 2;
END_FUNCTION

    result := Double(21);
    PRINT('result = %d\n', result);
END_PROGRAM
