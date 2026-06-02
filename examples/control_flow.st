(* 示例程序 2: 控制流 *)
(* 演示 IF-THEN-ELSE 和 WHILE 循环 *)

PROGRAM ControlFlow
VAR
    x : INT := 10;
    y : INT := 5;
    max : INT;
    sum : INT := 0;
    i : INT;
END_VAR

    (* IF-THEN-ELSE 语句 *)
    IF x > y THEN
        max := x;
    ELSE
        max := y;
    END_IF;
    
    (* WHILE 循环 - 计算 1 到 10 的和 *)
    i := 1;
    WHILE i <= 10 DO
        sum := sum + i;
        i := i + 1;
    END_WHILE;
    
    (* FOR 循环 *)
    FOR i := 1 TO 5 DO
        x := x + i;
    END_FOR;
END_PROGRAM
