PROGRAM MemberAccessTest
VAR
    temperature : QREAL;
END_VAR

(* 尝试使用成员访问语法 *)
temperature.VAL := 25.5;
temperature.QUALITY := 0;

PRINT("Temperature: ", temperature);

END_PROGRAM