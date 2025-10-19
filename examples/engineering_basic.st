(* 基础工程库测试 - 逐步测试版 *)

PROGRAM BasicTest
VAR
    pass : INT := 0;
END_VAR

(* RS触发器 *)
FUNCTION RS_Trigger : BOOL
VAR_INPUT
    S : BOOL;
    R : BOOL;
END_VAR
VAR
    state : BOOL := FALSE;
END_VAR
    IF R THEN
        state := FALSE;
    ELSIF S THEN
        state := TRUE;
    END_IF;
    RS_Trigger := state;
END_FUNCTION

(* 限幅器 *)
FUNCTION Limiter : REAL
VAR_INPUT
    val : REAL;
    min : REAL;
    max : REAL;
END_VAR
VAR_LOCAL
    result : REAL;
END_VAR
    result := val;
    IF val < min THEN
        result := min;
    ELSIF val > max THEN
        result := max;
    END_IF;
    Limiter := result;
END_FUNCTION

(* 1oo2投票 *)
FUNCTION Vote_1oo2 : BOOL
VAR_INPUT
    a : BOOL;
    b : BOOL;
END_VAR
    Vote_1oo2 := a OR b;
END_FUNCTION

(* 2oo3投票 *)
FUNCTION Vote_2oo3 : BOOL
VAR_INPUT
    a : BOOL;
    b : BOOL;
    c : BOOL;
END_VAR
VAR_LOCAL
    cnt : INT;
END_VAR
    cnt := 0;
    IF a THEN cnt := cnt + 1; END_IF;
    IF b THEN cnt := cnt + 1; END_IF;
    IF c THEN cnt := cnt + 1; END_IF;
    Vote_2oo3 := cnt >= 2;
END_FUNCTION

    PRINT('=== Engineering Library Basic Test ===\n\n');
    
    (* Test 1: RS Trigger *)
    PRINT('[Test 1] RS Trigger\n');
    IF RS_Trigger(FALSE, FALSE) = FALSE THEN
        PRINT('  1.1 Init: PASS\n');
        pass := pass + 1;
    ELSE
        PRINT('  1.1 Init: FAIL\n');
    END_IF;
    
    IF RS_Trigger(TRUE, FALSE) = TRUE THEN
        PRINT('  1.2 Set: PASS\n');
        pass := pass + 1;
    ELSE
        PRINT('  1.2 Set: FAIL\n');
    END_IF;
    
    IF RS_Trigger(FALSE, TRUE) = FALSE THEN
        PRINT('  1.3 Reset: PASS\n');
        pass := pass + 1;
    ELSE
        PRINT('  1.3 Reset: FAIL\n');
    END_IF;
    
    (* Test 2: Limiter *)
    PRINT('\n[Test 2] Limiter\n');
    IF Limiter(50.0, 0.0, 100.0) = 50.0 THEN
        PRINT('  2.1 In range: PASS\n');
        pass := pass + 1;
    ELSE
        PRINT('  2.1 In range: FAIL\n');
    END_IF;
    
    IF Limiter(150.0, 0.0, 100.0) = 100.0 THEN
        PRINT('  2.2 Above max: PASS\n');
        pass := pass + 1;
    ELSE
        PRINT('  2.2 Above max: FAIL\n');
    END_IF;
    
    IF Limiter(-50.0, 0.0, 100.0) = 0.0 THEN
        PRINT('  2.3 Below min: PASS\n');
        pass := pass + 1;
    ELSE
        PRINT('  2.3 Below min: FAIL\n');
    END_IF;
    
    (* Test 3: 1oo2 Voting *)
    PRINT('\n[Test 3] 1oo2 Voting\n');
    IF Vote_1oo2(FALSE, FALSE) = FALSE THEN
        PRINT('  3.1 (F,F): PASS\n');
        pass := pass + 1;
    ELSE
        PRINT('  3.1 (F,F): FAIL\n');
    END_IF;
    
    IF Vote_1oo2(TRUE, FALSE) = TRUE THEN
        PRINT('  3.2 (T,F): PASS\n');
        pass := pass + 1;
    ELSE
        PRINT('  3.2 (T,F): FAIL\n');
    END_IF;
    
    (* Test 4: 2oo3 Voting *)
    PRINT('\n[Test 4] 2oo3 Voting\n');
    IF Vote_2oo3(TRUE, FALSE, FALSE) = FALSE THEN
        PRINT('  4.1 (T,F,F): PASS\n');
        pass := pass + 1;
    ELSE
        PRINT('  4.1 (T,F,F): FAIL\n');
    END_IF;
    
    IF Vote_2oo3(TRUE, TRUE, FALSE) = TRUE THEN
        PRINT('  4.2 (T,T,F): PASS\n');
        pass := pass + 1;
    ELSE
        PRINT('  4.2 (T,T,F): FAIL\n');
    END_IF;
    
    PRINT('\n=== Summary ===\n');
    PRINT('Passed: %d/10 tests\n', pass);
    IF pass = 10 THEN
        PRINT('Status: ALL TESTS PASSED!\n');
    ELSE
        PRINT('Status: Some tests failed\n');
    END_IF;
    PRINT('================\n');

END_PROGRAM
