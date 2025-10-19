(* ========================================================================= *)
(* ST 工程计算库完整测试（单文件版本）                                       *)
(* ========================================================================= *)

PROGRAM EngineeringLibTest
VAR
    test_total : INT := 28;
    test_passed : INT := 0;
END_VAR

(* ========================================================================= *)
(* RS 触发器                                                                *)
(* ========================================================================= *)
FUNCTION RS_Trigger : BOOL
VAR_INPUT
    Set : BOOL;
    Reset : BOOL;
END_VAR
VAR
    rs_state : BOOL := FALSE;
END_VAR
VAR_LOCAL
    result : BOOL;
END_VAR
    IF Reset THEN
        rs_state := FALSE;
    ELSIF Set THEN
        rs_state := TRUE;
    END_IF;
    result := rs_state;
    RS_Trigger := result;
END_FUNCTION

(* ========================================================================= *)
(* 平滑滤波器                                                               *)
(* ========================================================================= *)
FUNCTION SmoothFilter : REAL
VAR_INPUT
    input : REAL;
    alpha : REAL;
END_VAR
VAR
    filter_last : REAL := 0.0;
    filter_init : BOOL := FALSE;
END_VAR
VAR_LOCAL
    filtered : REAL;
    valid_alpha : REAL;
END_VAR
    IF alpha < 0.0 THEN
        valid_alpha := 0.0;
    ELSIF alpha > 1.0 THEN
        valid_alpha := 1.0;
    ELSE
        valid_alpha := alpha;
    END_IF;
    
    IF NOT filter_init THEN
        filtered := input;
        filter_init := TRUE;
    ELSE
        filtered := valid_alpha * input + (1.0 - valid_alpha) * filter_last;
    END_IF;
    
    filter_last := filtered;
    SmoothFilter := filtered;
END_FUNCTION

(* ========================================================================= *)
(* 1oo2 投票逻辑                                                            *)
(* ========================================================================= *)
FUNCTION Vote_1oo2 : BOOL
VAR_INPUT
    input1 : BOOL;
    input2 : BOOL;
END_VAR
VAR_LOCAL
    result : BOOL;
END_VAR
    result := input1 OR input2;
    Vote_1oo2 := result;
END_FUNCTION

(* ========================================================================= *)
(* 2oo3 投票逻辑                                                            *)
(* ========================================================================= *)
FUNCTION Vote_2oo3 : BOOL
VAR_INPUT
    input1 : BOOL;
    input2 : BOOL;
    input3 : BOOL;
END_VAR
VAR_LOCAL
    count : INT;
    result : BOOL;
END_VAR
    count := 0;
    IF input1 THEN count := count + 1; END_IF;
    IF input2 THEN count := count + 1; END_IF;
    IF input3 THEN count := count + 1; END_IF;
    result := count >= 2;
    Vote_2oo3 := result;
END_FUNCTION

(* ========================================================================= *)
(* 3oo4 投票逻辑                                                            *)
(* ========================================================================= *)
FUNCTION Vote_3oo4 : BOOL
VAR_INPUT
    input1 : BOOL;
    input2 : BOOL;
    input3 : BOOL;
    input4 : BOOL;
END_VAR
VAR_LOCAL
    count : INT;
    result : BOOL;
END_VAR
    count := 0;
    IF input1 THEN count := count + 1; END_IF;
    IF input2 THEN count := count + 1; END_IF;
    IF input3 THEN count := count + 1; END_IF;
    IF input4 THEN count := count + 1; END_IF;
    result := count >= 3;
    Vote_3oo4 := result;
END_FUNCTION

(* ========================================================================= *)
(* 限幅器                                                                   *)
(* ========================================================================= *)
FUNCTION Limiter : REAL
VAR_INPUT
    input : REAL;
    min_val : REAL;
    max_val : REAL;
END_VAR
VAR_LOCAL
    result : REAL;
    vmin : REAL;
    vmax : REAL;
END_VAR
    IF min_val > max_val THEN
        vmin := max_val;
        vmax := min_val;
    ELSE
        vmin := min_val;
        vmax := max_val;
    END_IF;
    
    result := input;
    IF input < vmin THEN
        result := vmin;
    ELSIF input > vmax THEN
        result := vmax;
    END_IF;
    
    Limiter := result;
END_FUNCTION

(* ========================================================================= *)
(* 死区处理器                                                               *)
(* ========================================================================= *)
FUNCTION DeadBand : REAL
VAR_INPUT
    input : REAL;
    threshold : REAL;
END_VAR
VAR_LOCAL
    result : REAL;
    abs_th : REAL;
END_VAR
    IF threshold < 0.0 THEN
        abs_th := -threshold;
    ELSE
        abs_th := threshold;
    END_IF;
    
    IF input > abs_th THEN
        result := input - abs_th;
    ELSIF input < -abs_th THEN
        result := input + abs_th;
    ELSE
        result := 0.0;
    END_IF;
    
    DeadBand := result;
END_FUNCTION

(* ========================================================================= *)
(* 滞环比较器                                                               *)
(* ========================================================================= *)
FUNCTION Hysteresis : BOOL
VAR_INPUT
    input : REAL;
    high_th : REAL;
    low_th : REAL;
END_VAR
VAR
    hyst_state : BOOL := FALSE;
END_VAR
VAR_LOCAL
    result : BOOL;
END_VAR
    IF input >= high_th THEN
        hyst_state := TRUE;
    ELSIF input <= low_th THEN
        hyst_state := FALSE;
    END_IF;
    result := hyst_state;
    Hysteresis := result;
END_FUNCTION

(* ========================================================================= *)
(* 主测试程序                                                               *)
(* ========================================================================= *)

    PRINT('\n================================================\n');
    PRINT('   Engineering Library Comprehensive Test\n');
    PRINT('================================================\n');
    
    (* ===== RS Trigger Tests ===== *)
    PRINT('\n[1] RS Trigger Tests\n');
    PRINT('--------------------\n');
    
    IF RS_Trigger(FALSE, FALSE) = FALSE THEN
        PRINT('Test 1.1: Initial state = FALSE [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 1.1: Initial state [FAIL]\n');
    END_IF;
    
    IF RS_Trigger(TRUE, FALSE) = TRUE THEN
        PRINT('Test 1.2: Set = TRUE [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 1.2: Set [FAIL]\n');
    END_IF;
    
    IF RS_Trigger(FALSE, FALSE) = TRUE THEN
        PRINT('Test 1.3: Hold = TRUE [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 1.3: Hold [FAIL]\n');
    END_IF;
    
    IF RS_Trigger(FALSE, TRUE) = FALSE THEN
        PRINT('Test 1.4: Reset = FALSE [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 1.4: Reset [FAIL]\n');
    END_IF;
    
    IF RS_Trigger(TRUE, TRUE) = FALSE THEN
        PRINT('Test 1.5: Reset priority [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 1.5: Reset priority [FAIL]\n');
    END_IF;
    
    (* ===== Smooth Filter Tests ===== *)
    PRINT('\n[2] Smooth Filter Tests\n');
    PRINT('-----------------------\n');
    
    IF SmoothFilter(100.0, 0.5) >= 99.9 AND SmoothFilter(100.0, 0.5) <= 100.1 THEN
        PRINT('Test 2.1: First call [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 2.1: First call [FAIL]\n');
    END_IF;
    
    IF SmoothFilter(50.0, 1.0) >= 49.9 AND SmoothFilter(50.0, 1.0) <= 50.1 THEN
        PRINT('Test 2.2: Alpha=1.0 [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 2.2: Alpha=1.0 [FAIL]\n');
    END_IF;
    
    IF SmoothFilter(100.0, 0.5) >= 74.9 AND SmoothFilter(100.0, 0.5) <= 75.1 THEN
        PRINT('Test 2.3: Alpha=0.5 smooth [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 2.3: Alpha=0.5 [FAIL]\n');
    END_IF;
    
    (* ===== 1oo2 Voting Tests ===== *)
    PRINT('\n[3] 1oo2 Voting Logic Tests\n');
    PRINT('---------------------------\n');
    
    IF Vote_1oo2(FALSE, FALSE) = FALSE THEN
        PRINT('Test 3.1: (F,F) = F [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 3.1: (F,F) [FAIL]\n');
    END_IF;
    
    IF Vote_1oo2(TRUE, FALSE) = TRUE THEN
        PRINT('Test 3.2: (T,F) = T [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 3.2: (T,F) [FAIL]\n');
    END_IF;
    
    IF Vote_1oo2(TRUE, TRUE) = TRUE THEN
        PRINT('Test 3.3: (T,T) = T [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 3.3: (T,T) [FAIL]\n');
    END_IF;
    
    (* ===== 2oo3 Voting Tests ===== *)
    PRINT('\n[4] 2oo3 Voting Logic Tests\n');
    PRINT('---------------------------\n');
    
    IF Vote_2oo3(TRUE, FALSE, FALSE) = FALSE THEN
        PRINT('Test 4.1: (T,F,F) = F [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 4.1: (T,F,F) [FAIL]\n');
    END_IF;
    
    IF Vote_2oo3(TRUE, TRUE, FALSE) = TRUE THEN
        PRINT('Test 4.2: (T,T,F) = T [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 4.2: (T,T,F) [FAIL]\n');
    END_IF;
    
    IF Vote_2oo3(TRUE, TRUE, TRUE) = TRUE THEN
        PRINT('Test 4.3: (T,T,T) = T [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 4.3: (T,T,T) [FAIL]\n');
    END_IF;
    
    (* ===== 3oo4 Voting Tests ===== *)
    PRINT('\n[5] 3oo4 Voting Logic Tests\n');
    PRINT('---------------------------\n');
    
    IF Vote_3oo4(TRUE, TRUE, FALSE, FALSE) = FALSE THEN
        PRINT('Test 5.1: (T,T,F,F) = F [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 5.1: (T,T,F,F) [FAIL]\n');
    END_IF;
    
    IF Vote_3oo4(TRUE, TRUE, TRUE, FALSE) = TRUE THEN
        PRINT('Test 5.2: (T,T,T,F) = T [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 5.2: (T,T,T,F) [FAIL]\n');
    END_IF;
    
    IF Vote_3oo4(TRUE, TRUE, TRUE, TRUE) = TRUE THEN
        PRINT('Test 5.3: (T,T,T,T) = T [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 5.3: (T,T,T,T) [FAIL]\n');
    END_IF;
    
    (* ===== Limiter Tests ===== *)
    PRINT('\n[6] Limiter Tests\n');
    PRINT('-----------------\n');
    
    IF Limiter(50.0, 0.0, 100.0) >= 49.9 AND Limiter(50.0, 0.0, 100.0) <= 50.1 THEN
        PRINT('Test 6.1: In range [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 6.1: In range [FAIL]\n');
    END_IF;
    
    IF Limiter(150.0, 0.0, 100.0) >= 99.9 AND Limiter(150.0, 0.0, 100.0) <= 100.1 THEN
        PRINT('Test 6.2: Above max [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 6.2: Above max [FAIL]\n');
    END_IF;
    
    IF Limiter(-50.0, 0.0, 100.0) >= -0.1 AND Limiter(-50.0, 0.0, 100.0) <= 0.1 THEN
        PRINT('Test 6.3: Below min [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 6.3: Below min [FAIL]\n');
    END_IF;
    
    (* ===== DeadBand Tests ===== *)
    PRINT('\n[7] Dead Band Tests\n');
    PRINT('-------------------\n');
    
    IF DeadBand(5.0, 10.0) >= -0.1 AND DeadBand(5.0, 10.0) <= 0.1 THEN
        PRINT('Test 7.1: Inside zone [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 7.1: Inside zone [FAIL]\n');
    END_IF;
    
    IF DeadBand(25.0, 10.0) >= 14.9 AND DeadBand(25.0, 10.0) <= 15.1 THEN
        PRINT('Test 7.2: Above zone [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 7.2: Above zone [FAIL]\n');
    END_IF;
    
    IF DeadBand(-25.0, 10.0) >= -15.1 AND DeadBand(-25.0, 10.0) <= -14.9 THEN
        PRINT('Test 7.3: Below zone [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 7.3: Below zone [FAIL]\n');
    END_IF;
    
    (* ===== Hysteresis Tests ===== *)
    PRINT('\n[8] Hysteresis Comparator Tests\n');
    PRINT('--------------------------------\n');
    
    IF Hysteresis(5.0, 80.0, 20.0) = FALSE THEN
        PRINT('Test 8.1: Init low [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 8.1: Init low [FAIL]\n');
    END_IF;
    
    IF Hysteresis(50.0, 80.0, 20.0) = FALSE THEN
        PRINT('Test 8.2: In zone stay low [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 8.2: In zone [FAIL]\n');
    END_IF;
    
    IF Hysteresis(85.0, 80.0, 20.0) = TRUE THEN
        PRINT('Test 8.3: Above high threshold [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 8.3: Above high [FAIL]\n');
    END_IF;
    
    IF Hysteresis(50.0, 80.0, 20.0) = TRUE THEN
        PRINT('Test 8.4: In zone stay high [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 8.4: Stay high [FAIL]\n');
    END_IF;
    
    IF Hysteresis(15.0, 80.0, 20.0) = FALSE THEN
        PRINT('Test 8.5: Below low threshold [PASS]\n');
        test_passed := test_passed + 1;
    ELSE
        PRINT('Test 8.5: Below low [FAIL]\n');
    END_IF;
    
    (* ===== Test Summary ===== *)
    PRINT('\n================================================\n');
    PRINT('   Test Summary\n');
    PRINT('================================================\n');
    PRINT('Total Tests:  %d\n', test_total);
    PRINT('Passed:       %d\n', test_passed);
    PRINT('Failed:       %d\n', test_total - test_passed);
    PRINT('Pass Rate:    %d%%\n', (test_passed * 100) / test_total);
    
    IF test_passed = test_total THEN
        PRINT('\n*** ALL TESTS PASSED! ***\n');
    ELSE
        PRINT('\n*** SOME TESTS FAILED ***\n');
    END_IF;
    PRINT('================================================\n\n');

END_PROGRAM
