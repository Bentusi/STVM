(* ========================================================================= *)
(* ST 工程计算库简化测试                                                     *)
(* ========================================================================= *)

PROGRAM TestEngineeringLib
VAR
    test_count : INT := 0;
    pass_count : INT := 0;
END_VAR

(* ========================================================================= *)
(* 测试 RS 触发器                                                           *)
(* ========================================================================= *)
FUNCTION TestRS : INT
VAR_LOCAL
    result : BOOL;
    tests : INT;
END_VAR
    PRINT('\n=== Test RS Trigger ===\n');
    tests := 0;
    
    (* 测试1: 初始状态 *)
    result := RS_Trigger(FALSE, FALSE);
    PRINT('Test 1 - Initial: %d (expect 0)\n', result);
    IF result = FALSE THEN tests := tests + 1; END_IF;
    
    (* 测试2: Set *)
    result := RS_Trigger(TRUE, FALSE);
    PRINT('Test 2 - Set: %d (expect 1)\n', result);
    IF result = TRUE THEN tests := tests + 1; END_IF;
    
    (* 测试3: Hold *)
    result := RS_Trigger(FALSE, FALSE);
    PRINT('Test 3 - Hold: %d (expect 1)\n', result);
    IF result = TRUE THEN tests := tests + 1; END_IF;
    
    (* 测试4: Reset *)
    result := RS_Trigger(FALSE, TRUE);
    PRINT('Test 4 - Reset: %d (expect 0)\n', result);
    IF result = FALSE THEN tests := tests + 1; END_IF;
    
    (* 测试5: Reset Priority *)
    result := RS_Trigger(TRUE, TRUE);
    PRINT('Test 5 - Reset Priority: %d (expect 0)\n', result);
    IF result = FALSE THEN tests := tests + 1; END_IF;
    
    PRINT('RS Trigger: %d/5 passed\n', tests);
    TestRS := tests;
END_FUNCTION

(* ========================================================================= *)
(* 测试平滑滤波                                                             *)
(* ========================================================================= *)
FUNCTION TestSmooth : INT
VAR_LOCAL
    result : REAL;
    tests : INT;
END_VAR
    PRINT('\n=== Test Smooth Filter ===\n');
    tests := 0;
    
    (* 测试1: 首次调用 *)
    result := SmoothFilter(100.0, 0.5);
    PRINT('Test 1 - First call: %.2f (expect 100.00)\n', result);
    IF result >= 99.9 AND result <= 100.1 THEN tests := tests + 1; END_IF;
    
    (* 测试2: alpha=1.0 *)
    result := SmoothFilter(50.0, 1.0);
    PRINT('Test 2 - Alpha=1.0: %.2f (expect 50.00)\n', result);
    IF result >= 49.9 AND result <= 50.1 THEN tests := tests + 1; END_IF;
    
    (* 测试3: alpha=0.5 平滑 *)
    result := SmoothFilter(100.0, 0.5);
    PRINT('Test 3 - Alpha=0.5: %.2f (expect 75.00)\n', result);
    IF result >= 74.9 AND result <= 75.1 THEN tests := tests + 1; END_IF;
    
    PRINT('Smooth Filter: %d/3 passed\n', tests);
    TestSmooth := tests;
END_FUNCTION

(* ========================================================================= *)
(* 测试投票逻辑                                                             *)
(* ========================================================================= *)
FUNCTION TestVoting : INT
VAR_LOCAL
    result : BOOL;
    tests : INT;
END_VAR
    PRINT('\n=== Test Voting Logic ===\n');
    tests := 0;
    
    (* 1oo2 测试 *)
    PRINT('-- 1oo2 Voting --\n');
    result := Vote_1oo2(FALSE, FALSE);
    PRINT('1oo2(F,F): %d (expect 0)\n', result);
    IF result = FALSE THEN tests := tests + 1; END_IF;
    
    result := Vote_1oo2(TRUE, FALSE);
    PRINT('1oo2(T,F): %d (expect 1)\n', result);
    IF result = TRUE THEN tests := tests + 1; END_IF;
    
    result := Vote_1oo2(TRUE, TRUE);
    PRINT('1oo2(T,T): %d (expect 1)\n', result);
    IF result = TRUE THEN tests := tests + 1; END_IF;
    
    (* 2oo3 测试 *)
    PRINT('-- 2oo3 Voting --\n');
    result := Vote_2oo3(TRUE, FALSE, FALSE);
    PRINT('2oo3(T,F,F): %d (expect 0)\n', result);
    IF result = FALSE THEN tests := tests + 1; END_IF;
    
    result := Vote_2oo3(TRUE, TRUE, FALSE);
    PRINT('2oo3(T,T,F): %d (expect 1)\n', result);
    IF result = TRUE THEN tests := tests + 1; END_IF;
    
    result := Vote_2oo3(TRUE, TRUE, TRUE);
    PRINT('2oo3(T,T,T): %d (expect 1)\n', result);
    IF result = TRUE THEN tests := tests + 1; END_IF;
    
    (* 3oo4 测试 *)
    PRINT('-- 3oo4 Voting --\n');
    result := Vote_3oo4(TRUE, TRUE, FALSE, FALSE);
    PRINT('3oo4(T,T,F,F): %d (expect 0)\n', result);
    IF result = FALSE THEN tests := tests + 1; END_IF;
    
    result := Vote_3oo4(TRUE, TRUE, TRUE, FALSE);
    PRINT('3oo4(T,T,T,F): %d (expect 1)\n', result);
    IF result = TRUE THEN tests := tests + 1; END_IF;
    
    result := Vote_3oo4(TRUE, TRUE, TRUE, TRUE);
    PRINT('3oo4(T,T,T,T): %d (expect 1)\n', result);
    IF result = TRUE THEN tests := tests + 1; END_IF;
    
    PRINT('Voting Logic: %d/9 passed\n', tests);
    TestVoting := tests;
END_FUNCTION

(* ========================================================================= *)
(* 测试限幅器                                                               *)
(* ========================================================================= *)
FUNCTION TestLimiter : INT
VAR_LOCAL
    result : REAL;
    tests : INT;
END_VAR
    PRINT('\n=== Test Limiter ===\n');
    tests := 0;
    
    (* 测试1: 正常范围 *)
    result := Limiter(50.0, 0.0, 100.0);
    PRINT('Test 1 - In range: %.2f (expect 50.00)\n', result);
    IF result >= 49.9 AND result <= 50.1 THEN tests := tests + 1; END_IF;
    
    (* 测试2: 超上限 *)
    result := Limiter(150.0, 0.0, 100.0);
    PRINT('Test 2 - Above max: %.2f (expect 100.00)\n', result);
    IF result >= 99.9 AND result <= 100.1 THEN tests := tests + 1; END_IF;
    
    (* 测试3: 低于下限 *)
    result := Limiter(-50.0, 0.0, 100.0);
    PRINT('Test 3 - Below min: %.2f (expect 0.00)\n', result);
    IF result >= -0.1 AND result <= 0.1 THEN tests := tests + 1; END_IF;
    
    PRINT('Limiter: %d/3 passed\n', tests);
    TestLimiter := tests;
END_FUNCTION

(* ========================================================================= *)
(* 测试死区                                                                 *)
(* ========================================================================= *)
FUNCTION TestDeadBand : INT
VAR_LOCAL
    result : REAL;
    tests : INT;
END_VAR
    PRINT('\n=== Test Dead Band ===\n');
    tests := 0;
    
    (* 测试1: 死区内 *)
    result := DeadBand(5.0, 10.0);
    PRINT('Test 1 - Inside: %.2f (expect 0.00)\n', result);
    IF result >= -0.1 AND result <= 0.1 THEN tests := tests + 1; END_IF;
    
    (* 测试2: 超过死区 *)
    result := DeadBand(25.0, 10.0);
    PRINT('Test 2 - Above: %.2f (expect 15.00)\n', result);
    IF result >= 14.9 AND result <= 15.1 THEN tests := tests + 1; END_IF;
    
    (* 测试3: 负向超过 *)
    result := DeadBand(-25.0, 10.0);
    PRINT('Test 3 - Below: %.2f (expect -15.00)\n', result);
    IF result >= -15.1 AND result <= -14.9 THEN tests := tests + 1; END_IF;
    
    PRINT('Dead Band: %d/3 passed\n', tests);
    TestDeadBand := tests;
END_FUNCTION

(* ========================================================================= *)
(* 测试滞环比较器                                                           *)
(* ========================================================================= *)
FUNCTION TestHysteresis : INT
VAR_LOCAL
    result : BOOL;
    tests : INT;
END_VAR
    PRINT('\n=== Test Hysteresis ===\n');
    tests := 0;
    
    (* 测试1: 初始低 *)
    result := Hysteresis(5.0, 80.0, 20.0);
    PRINT('Test 1 - Init low: %d (expect 0)\n', result);
    IF result = FALSE THEN tests := tests + 1; END_IF;
    
    (* 测试2: 滞环内保持 *)
    result := Hysteresis(50.0, 80.0, 20.0);
    PRINT('Test 2 - In zone: %d (expect 0)\n', result);
    IF result = FALSE THEN tests := tests + 1; END_IF;
    
    (* 测试3: 超过高阈值 *)
    result := Hysteresis(85.0, 80.0, 20.0);
    PRINT('Test 3 - Above high: %d (expect 1)\n', result);
    IF result = TRUE THEN tests := tests + 1; END_IF;
    
    (* 测试4: 回到滞环内 *)
    result := Hysteresis(50.0, 80.0, 20.0);
    PRINT('Test 4 - In zone keep: %d (expect 1)\n', result);
    IF result = TRUE THEN tests := tests + 1; END_IF;
    
    (* 测试5: 低于低阈值 *)
    result := Hysteresis(15.0, 80.0, 20.0);
    PRINT('Test 5 - Below low: %d (expect 0)\n', result);
    IF result = FALSE THEN tests := tests + 1; END_IF;
    
    PRINT('Hysteresis: %d/5 passed\n', tests);
    TestHysteresis := tests;
END_FUNCTION

    (* 主程序 *)
    PRINT('\n');
    PRINT('=============================================\n');
    PRINT('  Engineering Library Test Suite\n');
    PRINT('=============================================\n');
    
    pass_count := 0;
    pass_count := pass_count + TestRS();
    pass_count := pass_count + TestSmooth();
    pass_count := pass_count + TestVoting();
    pass_count := pass_count + TestLimiter();
    pass_count := pass_count + TestDeadBand();
    pass_count := pass_count + TestHysteresis();
    
    test_count := 28;
    
    PRINT('\n');
    PRINT('=============================================\n');
    PRINT('  Test Summary\n');
    PRINT('=============================================\n');
    PRINT('Total:  %d tests\n', test_count);
    PRINT('Passed: %d tests\n', pass_count);
    PRINT('Failed: %d tests\n', test_count - pass_count);
    
    IF pass_count = test_count THEN
        PRINT('\n*** ALL TESTS PASSED! ***\n');
    ELSE
        PRINT('\n*** Some tests failed ***\n');
    END_IF;
    PRINT('=============================================\n');

END_PROGRAM
