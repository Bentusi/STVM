PROGRAM TestImport

(* 测试IMPORT语法 - 导入整个库 *)
IMPORT Abs FROM 'examples/mathlib.stbc';

(* 测试IMPORT语法 - 从库中导入函数并使用别名 *)
IMPORT Abs AS aaa FROM 'examples/mathlib.stbc';

VAR
    x : INT := -10;
    y : INT := -20;
    result : INT;
END_VAR

    (* 使用导入的函数 *)
    result := Abs(x);
    PRINT('Result: %d\n', result);
    (* 使用导入的函数 *)
    result := aaa(y);
    PRINT('Result: %d\n', result);
    
END_PROGRAM
