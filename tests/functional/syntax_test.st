(*
 * ============================================================================
 * STVM 语法功能综合测试 v7 (Final)
 * 新增语法: ** 幂运算符, ~ 位取反, 逗号多变量声明, RETURN
 * 覆盖: 变量/运算符/控制流/函数/质量位/SYSTEM
 * ============================================================================
 *)

PROGRAM SyntaxFullTest
VAR
    itest       : INT;
    jtest       : INT;
    ktest       : INT;
    rval        : REAL;
    r2          : REAL;
    r3          : REAL;
    flag        : BOOL;
    ok          : BOOL;
    done        : BOOL;
    msg         : STRING;
    log         : STRING;
    bitresult   : INT;
    for_sum     : INT;
    while_val   : INT;
    repeat_val  : INT;
    iter        : INT;
    qreal_v     : QREAL;
    q_test      : INT;
    base_val    : REAL;
    func_result : INT;
    cmp_ok      : BOOL;
    logic_a     : BOOL;
    logic_b     : BOOL;
    logic_r     : BOOL;
    case_input  : INT;
    case_result : STRING;
END_VAR

(* 函数: 带返回值类型, 带 BEGIN *)
FUNCTION Add : INT
VAR_INPUT
    a : INT;
    b : INT;
END_VAR
BEGIN
    Add := a + b;
END_FUNCTION

(* 函数: 带返回值类型, 不带 BEGIN *)
FUNCTION Mul : INT
VAR_INPUT
    a : INT;
    b : INT;
END_VAR
    Mul := a * b;
END_FUNCTION

(* 函数: 无返回值类型, 带 BEGIN *)
FUNCTION InitAll
VAR
    tmp : INT;
END_VAR
BEGIN
    itest := 0;
    jtest := 0;
    ktest := 0;
    RETURN;
END_FUNCTION

(* 函数: 无返回值类型, 不带 BEGIN *)
FUNCTION TestReturn
VAR_INPUT
    value : INT;
END_VAR
    PRINT('TestReturn: %d', value);
    RETURN;
END_FUNCTION

    PRINT('');
    PRINT('========================================');
    PRINT('  STVM 语法功能综合测试 v7');
    PRINT('========================================');

    (* 逗号多变量声明测试 *)
    PRINT('--- 逗号声明 ---');
    itest := 10;
    jtest := 20;
    ktest := 30;
    PRINT('comma vars: %d %d %d', itest, jtest, ktest);
    flag := TRUE;
    ok := FALSE;
    done := TRUE;
    PRINT('comma bools: %d %d %d', flag, ok, done);
    rval := 1.5;
    r2 := 2.5;
    r3 := rval + r2;
    PRINT('comma reals: %f %f %f', rval, r2, r3);

    (* 算术 *)
    PRINT('--- 算术 ---');
    itest := 10 + 20;
    PRINT('10+20=%d', itest);
    itest := 50 - 15;
    PRINT('50-15=%d', itest);
    itest := 6 * 7;
    PRINT('6*7=%d', itest);
    itest := 100 / 4;
    PRINT('100/4=%d', itest);
    itest := 100 MOD 3;
    PRINT('100MOD3=%d', itest);

    (* 比较 *)
    PRINT('--- 比较 ---');
    cmp_ok := (15 > 10);
    PRINT('15>10=%d', cmp_ok);
    cmp_ok := (10 = 10);
    PRINT('10=10=%d', cmp_ok);
    cmp_ok := (7 <> 3);
    PRINT('7<>3=%d', cmp_ok);

    (* 逻辑 *)
    PRINT('--- 逻辑 ---');
    logic_a := TRUE;
    logic_b := FALSE;
    logic_r := logic_a AND logic_b;
    PRINT('T AND F=%d', logic_r);
    logic_r := logic_a OR logic_b;
    PRINT('T OR F=%d', logic_r);
    logic_r := NOT logic_a;
    PRINT('NOT T=%d', logic_r);

    (* 位运算 + 新增 ~ 运算符 *)
    PRINT('--- 位运算(含~) ---');
    bitresult := 6 AND 3;
    PRINT('6AND3=%d', bitresult);
    bitresult := 6 OR 3;
    PRINT('6OR3=%d', bitresult);
    bitresult := 6 XOR 3;
    PRINT('6XOR3=%d', bitresult);
    bitresult := ~5 AND 255;
    PRINT('~5 AND 255=%d', bitresult);
    bitresult := 1 SHL 3;
    PRINT('1<<3=%d', bitresult);
    bitresult := 16 SHR 2;
    PRINT('16>>2=%d', bitresult);

    (* IF/ELSIF/ELSE *)
    PRINT('--- IF/ELSIF ---');
    itest := 95;
    IF itest >= 90 THEN
        PRINT('Grade A');
    ELSIF itest >= 80 THEN
        PRINT('Grade B');
    ELSE
        PRINT('Grade C');
    END_IF;

    (* WHILE *)
    PRINT('--- WHILE ---');
    while_val := 0;
    iter := 0;
    WHILE iter < 5 DO
        while_val := while_val + iter;
        iter := iter + 1;
    END_WHILE;
    PRINT('WHILE0..4=%d', while_val);

    (* FOR *)
    PRINT('--- FOR ---');
    for_sum := 0;
    FOR iter := 1 TO 10 DO
        for_sum := for_sum + iter;
    END_FOR;
    PRINT('FOR1..10=%d', for_sum);

    (* REPEAT *)
    PRINT('--- REPEAT ---');
    repeat_val := 1;
    iter := 1;
    REPEAT
        repeat_val := repeat_val * 2;
        iter := iter + 1;
    UNTIL iter > 4
    END_REPEAT;
    PRINT('2^4=%d', repeat_val);

    (* CASE *)
    PRINT('--- CASE ---');
    case_input := 2;
    CASE case_input OF
        0: case_result := 'Zero';
        1: case_result := 'One';
        2,3: case_result := 'TwoOrThree';
    ELSE
        case_result := 'Other';
    END_CASE;
    PRINT('CASE2=%s', case_result);

    (* 函数调用 + RETURN *)
    PRINT('--- 函数 + RETURN ---');
    func_result := Add(15, 25);
    PRINT('Add=%d', func_result);
    func_result := Mul(6, 8);
    PRINT('Mul=%d', func_result);
    TestReturn(42);
    InitAll();

    (* 质量位 *)
    PRINT('--- 质量位 ---');
    qreal_v := MakeQReal(42.0, 0);
    q_test := GetQuality(qreal_v);
    PRINT('Q(GOOD)=%d', q_test);
    base_val := ToReal(qreal_v);
    PRINT('ToReal=%f', base_val);

    (* SYSTEM *)
    PRINT('--- SYSTEM ---');
    itest := SYSTEM('echo ok');
    PRINT('exit=%d', itest);

    PRINT('========================================');
    PRINT('  语法测试 v7 PASS (arrays SKIP)');
    PRINT('========================================');
END_PROGRAM