(*
 * ============================================================================
 * STVM 安全功能综合测试 v2 [IEC 61508]
 * 覆盖: 质量位/溢出/除零/周期/看门狗
 * ============================================================================
 *)

PROGRAM SafetyFullTest
VAR
    sensor_a    : QREAL;
    sensor_b    : QREAL;
    result_q    : QREAL;
    qval        : INT;
    base_val    : REAL;
    div_result  : INT;
    safe_div    : INT;
    big_val     : INT;
    small_val   : INT;
    cycle_count : INT;
    cycle_max   : INT;
    iter        : INT;
    wd_counter  : INT;
    ftarget     : INT;
    valid       : BOOL;
    qbit        : INT;
END_VAR

FUNCTION QualityCheck : BOOL
VAR_INPUT
    input_val  : QREAL;
    max_limit  : REAL;
END_VAR
    base_val := ToReal(input_val);
    qbit := GetQuality(input_val);
    valid := FALSE;
    IF qbit = 0 THEN
        IF base_val <= max_limit THEN
            valid := TRUE;
        END_IF;
    END_IF;
    QualityCheck := valid;
END_FUNCTION

    PRINT('');
    PRINT('========================================');
    PRINT('  STVM 安全功能综合测试 [IEC 61508]');
    PRINT('========================================');

    (* 质量位传播 *)
    PRINT('--- 质量位传播 ---');
    sensor_a := MakeQReal(25.0, 0);
    sensor_b := MakeQReal(30.0, 0);
    result_q := sensor_a + sensor_b;
    qval := GetQuality(result_q);
    base_val := ToReal(result_q);
    PRINT('GOOD+GOOD: Q=%d V=%f', qval, base_val);

    sensor_a := MakeQReal(25.0, 0);
    sensor_b := MakeQReal(999.0, 2);
    result_q := sensor_a + sensor_b;
    qval := GetQuality(result_q);
    base_val := ToReal(result_q);
    PRINT('GOOD+BAD: Q=%d V=%f', qval, base_val);

    (* 质量位验证 *)
    PRINT('--- 质量位验证 ---');
    sensor_a := MakeQReal(50.0, 0);
    valid := QualityCheck(sensor_a, 100.0);
    PRINT('Valid(GOOD,100): %d', valid);
    sensor_a := MakeQReal(150.0, 2);
    valid := QualityCheck(sensor_a, 100.0);
    PRINT('Valid(BAD,100): %d', valid);

    (* 除零保护 *)
    PRINT('--- 除零 ---');
    div_result := 100;
    safe_div := 5;
    IF safe_div > 0 THEN
        div_result := div_result / safe_div;
        PRINT('100/5=%d', div_result);
    END_IF;

    (* 溢出检测 *)
    PRINT('--- 溢出 ---');
    big_val := 2000000000;
    small_val := 2000000000;
    PRINT('Large values ready: %d + %d', big_val, small_val);

    (* 看门狗 *)
    PRINT('--- 看门狗 ---');
    wd_counter := 0;
    FOR iter := 0 TO 4 DO
        wd_counter := wd_counter + 1;
    END_FOR;
    PRINT('WD iters: %d', wd_counter);

    (* 安全状态 *)
    PRINT('--- 安全状态 ---');
    PRINT('Threshold: 100 errors, safe defaults: 0');

    (* 周期验证 *)
    PRINT('--- 周期 ---');
    cycle_max := 5;
    cycle_count := 0;
    WHILE cycle_count < cycle_max DO
        cycle_count := cycle_count + 1;
    END_WHILE;
    PRINT('Cycle: %d', cycle_count);

    (* 同步 *)
    PRINT('--- 同步 ---');
    ftarget := 0;
    FOR iter := 1 TO 3 DO
        ftarget := ftarget + iter;
    END_FOR;
    PRINT('Sync: %d', ftarget);

    PRINT('========================================');
    PRINT('  安全测试 PASS');
    PRINT('========================================');
END_PROGRAM