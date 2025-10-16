(* 示例程序 4: 数组操作 *)

PROGRAM Arrays
VAR
    numbers : ARRAY[0..9] OF INT;
    sum : INT := 0;
    max : INT;
    i : INT;
END_VAR

BEGIN
    (* 初始化数组 *)
    FOR i := 0 TO 9 DO
        numbers[i] := i * 10;
    END_FOR;
    
    (* 计算数组元素和 *)
    FOR i := 0 TO 9 DO
        sum := sum + numbers[i];
    END_FOR;
    
    (* 查找最大值 *)
    max := numbers[0];
    FOR i := 1 TO 9 DO
        IF numbers[i] > max THEN
            max := numbers[i];
        END_IF;
    END_FOR;
END_PROGRAM
