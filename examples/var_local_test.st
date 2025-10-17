(* VAR_LOCAL 测试 *)
(* 测试函数中使用 VAR_LOCAL 定义局部变量 *)

PROGRAM VarLocalTest
VAR
    global_counter : INT := 100;
    result : INT;
END_VAR

FUNCTION Add : INT
VAR_INPUT
    a : INT;
    b : INT;
END_VAR
VAR_LOCAL
    temp : INT;
    sum : INT;
END_VAR
    temp := a + b;
    sum := temp + global_counter;
    PRINT('In Add: temp=%d, sum=%d, global_counter=%d\n', temp, sum, global_counter);
    Add := sum;
END_FUNCTION

FUNCTION Multiply : INT
VAR_INPUT
    x : INT;
    y : INT;
END_VAR
VAR_LOCAL
    temp : INT;      (* 这个 temp 是 Multiply 的局部变量 *)
    product : INT;
END_VAR
    temp := x * y;
    product := temp + 1;
    PRINT('In Multiply: temp=%d, product=%d\n', temp, product);
    Multiply := product;
END_FUNCTION

    PRINT('Testing VAR_LOCAL...\n');
    PRINT('global_counter = %d\n', global_counter);
    
    result := Add(10, 20);
    PRINT('Add(10, 20) = %d\n', result);
    
    result := Multiply(5, 6);
    PRINT('Multiply(5, 6) = %d\n', result);
    
    (* 验证 temp 在不同函数中是独立的 *)
    result := Add(1, 2);
    PRINT('Add(1, 2) = %d\n', result);
    
    PRINT('Test completed!\n');
    
END_PROGRAM
