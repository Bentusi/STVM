(* 循环类型对比示例 *)

PROGRAM LoopComparison
VAR
    i : INT;
    sum : INT;
END_VAR

    PRINT('=== WHILE 循环示例 ===\n');
    (* WHILE: 条件在开始前检查 *)
    i := 10;
    WHILE i < 5 DO
        PRINT('这行不会执行\n');
        i := i + 1;
    END_WHILE;
    PRINT('WHILE: i初始为10，大于5，循环体0次\n');
    
    PRINT('\n=== FOR 循环示例 ===\n');
    (* FOR: 指定范围的循环 *)
    sum := 0;
    FOR i := 1 TO 5 DO
        sum := sum + i;
    END_FOR;
    PRINT('FOR 1到5累加: %d\n', sum);
    
    PRINT('\n=== REPEAT 循环示例 ===\n');
    (* REPEAT: 至少执行一次 *)
    i := 10;
    REPEAT
        PRINT('REPEAT: 即使i=10大于5，这行也会执行一次\n');
        i := i + 1;
    UNTIL i > 5
    END_REPEAT;
    PRINT('REPEAT: 执行了1次，然后退出\n');
    
    PRINT('\n=== 三种循环计算1-10累加 ===\n');
    
    (* 方法1: WHILE *)
    i := 1;
    sum := 0;
    WHILE i <= 10 DO
        sum := sum + i;
        i := i + 1;
    END_WHILE;
    PRINT('WHILE 结果: %d\n', sum);
    
    (* 方法2: FOR *)
    sum := 0;
    FOR i := 1 TO 10 DO
        sum := sum + i;
    END_FOR;
    PRINT('FOR 结果: %d\n', sum);
    
    (* 方法3: REPEAT *)
    i := 1;
    sum := 0;
    REPEAT
        sum := sum + i;
        i := i + 1;
    UNTIL i > 10
    END_REPEAT;
    PRINT('REPEAT 结果: %d\n', sum);
    
END_PROGRAM
