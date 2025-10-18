PROGRAM TestImport

(* 测试IMPORT语法 - 导入整个库 *)
IMPORT 'mathlib.stbc';

(* 测试IMPORT语法 - 导入库并使用别名 *)
IMPORT 'stringlib.stbc' AS str;

(* 测试IMPORT语法 - 从库中导入特定函数 *)
IMPORT Add FROM 'mathlib.stbc';

(* 测试IMPORT语法 - 从库中导入函数并使用别名 *)
IMPORT Multiply AS Mul FROM 'mathlib.stbc';

VAR
    x : INT := 10;
    y : INT := 20;
    result : INT;
END_VAR

    (* 使用导入的函数 *)
    result := Add(x, y);
    
END_PROGRAM
