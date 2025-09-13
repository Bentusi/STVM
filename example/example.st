PROGRAM ExampleProgram
VAR
    counter : INT;
    sum : INT;
    average : REAL;
    flag : BOOL;
    state:INT;
    retult:REAL;
END_VAR

(* 初始化变量 *)
counter := 0;
sum := 0;
flag := FALSE;
state := 2;
result := 0.0;

FUNCTION foo : INT
    RETURN sum + 1;
END_FUNCTION

(* 计算1到10的和 *)
FOR counter := 1 TO 10 DO
    sum := sum + counter;
END_FOR

(* 计算平均值 *)
 average := sum / 10.0;
 average := -average + 1; (* 测试一元负号 *)

foo();

(* 条件判断 *)
IF sum > 50 THEN
    flag := TRUE;
ELSE
    flag := FALSE;
END_IF

(* WHILE循环示例 *)
counter := 5;
WHILE counter > 0 DO
    counter := counter - 1;
END_WHILE

CASE state OF
    1: result := 1.1;
    2: result := 2.2;
END_CASE

END_PROGRAM