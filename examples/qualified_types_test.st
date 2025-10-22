PROGRAM QualifiedTypeTest
VAR
    temperature : QREAL;
    pressure : QINT;
    status : QBOOL;
    normal_temp : REAL;
    result : QREAL;
END_VAR

(* 初始化质量化变量，系统默认质量位为0（GOOD） *)
temperature := 25.5;
pressure := 1013;
status := TRUE;
normal_temp := 20.0;

(* 质量化类型之间的运算，质量位会自动传播 *)
result := temperature + normal_temp;  (* 结果类型为QREAL，质量位为GOOD *)

(* 使用PRINT输出结果 *)
PRINT("Temperature: ", temperature);
PRINT("Pressure: ", pressure);
PRINT("Status: ", status);
PRINT("Result: ", result);

END_PROGRAM