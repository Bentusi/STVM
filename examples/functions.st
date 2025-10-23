(* 示例程序 3: 函数定义和调用 *)

PROGRAM Functions
VAR
    a : INT := 5;
    b : INT := 3;
    result : INT;
    fact : INT;
    sum : INT :=0;
END_VAR

(* 计算两个数的和 *)
FUNCTION Add : INT
VAR_INPUT
    x : INT;
    y : INT;
END_VAR
    Add := x + y;
END_FUNCTION

(* 计算阶乘 *)
FUNCTION Factorial : INT
VAR_INPUT
    n : INT;
END_VAR
VAR
    i : INT;
    temp : INT;
END_VAR
    temp := 1;
    FOR i := 1 TO n DO
        temp := temp * i;
    END_FOR;
    Factorial := temp;
END_FUNCTION

// 实现带质量位的三取二逻辑
// 任意质量为异常则退出表决，退化为二取一，如果两个质量异常则直接用质量为好的输出
FUNCTION vote_2oo3 : QBOOL
VAR_INPUT
    a : QBOOL;
    b : QBOOL;
    c : QBOOL;
END_VAR
    IF a.quality = 0 AND b.quality = 0 THEN
        vote_2oo3 := a;
    ELSIF a.quality = 0 AND c.quality = 0 THEN
        vote_2oo3 := a;
    ELSIF b.quality = 0 AND c.quality = 0 THEN
        vote_2oo3 := b;
    ELSIF a.quality = 1 THEN
        vote_2oo3 := a;
    ELSIF b.quality = 1 THEN
        vote_2oo3 := b;
    ELSIF c.quality = 1 THEN
        vote_2oo3 := c;
    ELSE
        // 全部异常，返回0并标记异常
        vote_2oo3 := FALSE;
        vote_2oo3.quality := 1;
    END_IF;
END_FUNCTION

FUNCTION MAIN
    (* 调用函数 *)
    result := Add(a, b);
    fact := Factorial(5);
    PRINT("result: %d\n", result);
    PRINT("fact: %d\n", fact);
    sum := sum + 1;
    PRINT("SUM := %d\n", sum);
END_FUNCTION

END_PROGRAM
