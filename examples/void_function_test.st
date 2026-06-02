(* 测试无返回值函数 - VOID 风格 *)

PROGRAM VoidFunctionTest
VAR
    counter : INT := 0;
    message : STRING;
END_VAR

(* 无返回值函数 - 类似 PROCEDURE *)
FUNCTION PrintMessage
VAR_INPUT
    msg : STRING;
END_VAR
    PRINT('Message: %s\n', msg);
END_FUNCTION

(* 无返回值函数 - 修改全局变量 *)
FUNCTION IncrementCounter
VAR
    counter : INT;  (* 引用全局变量 *)
END_VAR
    counter := counter + 1;
    PRINT('Counter incremented to: %d\n', counter);
END_FUNCTION

(* 无返回值函数 - 带多个参数 *)
FUNCTION PrintSum
VAR_INPUT
    a : INT;
    b : INT;
END_VAR
VAR_LOCAL
    sum : INT;
END_VAR
    sum := a + b;
    PRINT('%d + %d = %d\n', a, b, sum);
END_FUNCTION

(* 有返回值的函数用于对比 *)
FUNCTION Add : INT
VAR_INPUT
    x : INT;
    y : INT;
END_VAR
    Add := x + y;
END_FUNCTION

BEGIN
    (* 调用无返回值函数 *)
    PrintMessage('Hello STVM!');
    
    (* 多次调用修改全局状态 *)
    IncrementCounter();
    IncrementCounter();
    IncrementCounter();
    
    PRINT('Final counter value: %d\n', counter);
    
    (* 调用带参数的无返回值函数 *)
    PrintSum(10, 20);
    PrintSum(100, 200);
    
    (* 对比：调用有返回值的函数 *)
    counter := Add(5, 3);
    PRINT('Add result: %d\n', counter);
END_PROGRAM
