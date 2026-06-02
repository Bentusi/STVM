PROGRAM QualityPropagationTest
VAR
    temp1 : QREAL;
    temp2 : QREAL;
    pressure : QINT;
    result1 : QREAL;
    result2 : QINT;
    normal_val : REAL;
END_VAR

(* 初始化变量，默认质量位为GOOD(0) *)
temp1 := 25.5;
temp2 := 30.0;
pressure := 1013;
normal_val := 20.0;

(* 质量化类型之间的运算 *)
result1 := temp1 + temp2;  (* 两个GOOD质量位 -> 结果GOOD *)

(* 质量化类型与普通类型运算 *)
result2 := pressure + 100;  (* QINT + INT -> QINT *)

(* 混合运算 *)
result1 := temp1 + normal_val;  (* QREAL + REAL -> QREAL *)

PRINT("Temperature 1: ", temp1);
PRINT("Temperature 2: ", temp2);
PRINT("Pressure: ", pressure);
PRINT("Result 1: ", result1);
PRINT("Result 2: ", result2);

END_PROGRAM