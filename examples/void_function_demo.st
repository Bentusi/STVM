(* 测试无返回值函数 (VOID/PROCEDURE 风格) *)

PROGRAM VoidFunctionDemo
VAR
    result : INT := 0;
    message : STRING;
END_VAR

(* 示例 1: 简单的打印函数 - 无参数 *)
FUNCTION PrintHeader
    PRINT('===================\n');
    PRINT('  STVM Demo\n');
    PRINT('===================\n');
END_FUNCTION

(* 示例 2: 带参数的打印函数 *)
FUNCTION PrintValue
VAR_INPUT
    label : STRING;
    value : INT;
END_VAR
    PRINT('%s: %d\n', label, value);
END_FUNCTION

(* 示例 3: 计算但不返回值的函数 *)
FUNCTION DoCalculation
VAR_INPUT
    a : INT;
    b : INT;
END_VAR
VAR_LOCAL
    sum : INT;
    product : INT;
END_VAR
    sum := a + b;
    product := a * b;
    PRINT('Sum: %d, Product: %d\n', sum, product);
END_FUNCTION

(* 示例 4: 对比 - 有返回值的函数 *)
FUNCTION Multiply : INT
VAR_INPUT
    x : INT;
    y : INT;
END_VAR
    Multiply := x * y;
END_FUNCTION

BEGIN
    (* 调用无返回值函数 *)
    PrintHeader();
    
    PRINT('\n--- 测试无返回值函数 ---\n');
    
    (* 不能赋值给变量，只能作为语句调用 *)
    PrintValue('Test Value', 42);
    PrintValue('Another Value', 100);
    
    DoCalculation(5, 10);
    DoCalculation(7, 8);
    
    (* 对比：有返回值的函数可以赋值 *)
    result := Multiply(6, 7);
    PRINT('\nMultiply result: %d\n', result);
    
    (* 有返回值的函数也可以作为语句调用（忽略返回值）*)
    Multiply(3, 4);
    
    PrintHeader();
END_PROGRAM
