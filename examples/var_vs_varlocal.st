(* VAR vs VAR_LOCAL 对比测试 *)
(* 验证在函数中 VAR 定义全局变量，VAR_LOCAL 定义局部变量 *)

PROGRAM VarVsVarLocal
VAR
    program_var : INT := 1;
END_VAR

(* ========================================================================== *)
FUNCTION TestVar : INT
VAR_INPUT
    input : INT;
END_VAR
VAR
    program_var : INT := 2;
    func_global : INT := 999;  (* 使用 VAR 定义，应该是全局变量 *)
END_VAR

VAR_LOCAL
    func_local : INT;          (* 使用 VAR_LOCAL 定义，是局部变量 *)
    temp : INT;
END_VAR

    temp := input + func_global;
    func_local := input * 10;
    PRINT('TestVar: func_global=%d, func_local=%d, program_var=%d\n', func_global, func_local, program_var);
    TestVar := func_local;
END_FUNCTION


(* ========================================================================== *)
FUNCTION AccessGlobal : INT
VAR_LOCAL
    temp : INT;
    program_var : INT := 3;
END_VAR

    (* 这个变量不能访问 func_global（因为它是全局的） *)
    temp := func_global + 1;
    PRINT('AccessGlobal: func_global=%d, temp=%d, program_var=%d\n', func_global, temp, program_var);
    AccessGlobal := temp;
END_FUNCTION


(* ========================================================================== *)
    PRINT('=== VAR vs VAR_LOCAL Test ===\n');
    PRINT('program_var = %d\n', program_var);
    
    PRINT('\nCalling TestVar(5):\n');
    program_var := TestVar(5);
    
    PRINT('\nCalling AccessGlobal():\n');
    program_var := AccessGlobal();
    
    PRINT('\nFinal program_var = %d\n', program_var);
    PRINT('func_global should be accessible = %d\n', func_global);
    
END_PROGRAM
