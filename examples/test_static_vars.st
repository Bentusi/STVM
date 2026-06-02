PROGRAM test_static_vars

VAR
    result_a1 : INT;
    result_a2 : INT;
    result_a3 : INT;
    result_b1 : INT;
    result_b2 : INT;
    result_b3 : INT;
    toggle_a1 : BOOL;
    toggle_a2 : BOOL;
    toggle_a3 : BOOL;
    toggle_b1 : BOOL;
    toggle_b2 : BOOL;
    toggle_b3 : BOOL;
END_VAR

(* 测试多个函数使用相同的静态变量名 *)
FUNCTION Counter_A : INT
VAR
    counter : INT := 0;
END_VAR
    counter := counter + 1;
    Counter_A := counter;
END_FUNCTION

FUNCTION Counter_B : INT
VAR
    counter : INT := 100;
END_VAR
    counter := counter + 10;
    Counter_B := counter;
END_FUNCTION

FUNCTION Toggle_A : BOOL
VAR
    state : BOOL := FALSE;
END_VAR
    state := NOT state;
    Toggle_A := state;
END_FUNCTION

FUNCTION Toggle_B : BOOL
VAR
    state : BOOL := FALSE;
END_VAR
    state := NOT state;
    Toggle_B := state;
END_FUNCTION

(* 测试 Counter_A - 应该从 0 开始递增 *)
result_a1 := Counter_A();  (* 应该返回 1 *)
result_a2 := Counter_A();  (* 应该返回 2 *)
result_a3 := Counter_A();  (* 应该返回 3 *)

(* 测试 Counter_B - 应该从 100 开始递增 *)
result_b1 := Counter_B();  (* 应该返回 110 *)
result_b2 := Counter_B();  (* 应该返回 120 *)
result_b3 := Counter_B();  (* 应该返回 130 *)

(* 测试 Toggle_A *)
toggle_a1 := Toggle_A();   (* 应该返回 TRUE *)
toggle_a2 := Toggle_A();   (* 应该返回 FALSE *)
toggle_a3 := Toggle_A();   (* 应该返回 TRUE *)

(* 测试 Toggle_B *)
toggle_b1 := Toggle_B();   (* 应该返回 TRUE *)
toggle_b2 := Toggle_B();   (* 应该返回 FALSE *)
toggle_b3 := Toggle_B();   (* 应该返回 TRUE *)

(* 输出测试结果 *)
PRINT('=== Static Variable Test ===');
PRINT('');

PRINT('[Test 1] Counter_A (init=0, step=1)\n');
PRINT('  Call 1: %d (expected: 1)\n', result_a1);
IF result_a1 = 1 THEN
    PRINT('    PASS');
ELSE
    PRINT('    FAIL');
END_IF;

PRINT('  Call 2: ', result_a2, ' (expected: 2)');
IF result_a2 = 2 THEN
    PRINT('    PASS');
ELSE
    PRINT('    FAIL');
END_IF;

PRINT('  Call 3: ', result_a3, ' (expected: 3)');
IF result_a3 = 3 THEN
    PRINT('    PASS');
ELSE
    PRINT('    FAIL');
END_IF;

PRINT('');
PRINT('[Test 2] Counter_B (init=100, step=10)');
PRINT('  Call 1: ', result_b1, ' (expected: 110)');
IF result_b1 = 110 THEN
    PRINT('    PASS');
ELSE
    PRINT('    FAIL');
END_IF;

PRINT('  Call 2: ', result_b2, ' (expected: 120)');
IF result_b2 = 120 THEN
    PRINT('    PASS');
ELSE
    PRINT('    FAIL');
END_IF;

PRINT('  Call 3: ', result_b3, ' (expected: 130)');
IF result_b3 = 130 THEN
    PRINT('    PASS');
ELSE
    PRINT('    FAIL');
END_IF;

PRINT('');
PRINT('[Test 3] Toggle_A (alternating BOOL)');
PRINT('  Call 1: ', toggle_a1, ' (expected: TRUE)');
IF toggle_a1 = TRUE THEN
    PRINT('    PASS');
ELSE
    PRINT('    FAIL');
END_IF;

PRINT('  Call 2: ', toggle_a2, ' (expected: FALSE)');
IF toggle_a2 = FALSE THEN
    PRINT('    PASS');
ELSE
    PRINT('    FAIL');
END_IF;

PRINT('  Call 3: ', toggle_a3, ' (expected: TRUE)');
IF toggle_a3 = TRUE THEN
    PRINT('    PASS');
ELSE
    PRINT('    FAIL');
END_IF;

PRINT('');
PRINT('[Test 4] Toggle_B (independent from A)');
PRINT('  Call 1: ', toggle_b1, ' (expected: TRUE)');
IF toggle_b1 = TRUE THEN
    PRINT('    PASS');
ELSE
    PRINT('    FAIL');
END_IF;

PRINT('  Call 2: ', toggle_b2, ' (expected: FALSE)');
IF toggle_b2 = FALSE THEN
    PRINT('    PASS');
ELSE
    PRINT('    FAIL');
END_IF;

PRINT('  Call 3: ', toggle_b3, ' (expected: TRUE)');
IF toggle_b3 = TRUE THEN
    PRINT('    PASS');
ELSE
    PRINT('    FAIL');
END_IF;

PRINT('');
PRINT('=== Test Summary ===');
IF result_a1 = 1 AND result_a2 = 2 AND result_a3 = 3 AND
   result_b1 = 110 AND result_b2 = 120 AND result_b3 = 130 AND
   toggle_a1 = TRUE AND toggle_a2 = FALSE AND toggle_a3 = TRUE AND
   toggle_b1 = TRUE AND toggle_b2 = FALSE AND toggle_b3 = TRUE THEN
    PRINT('Status: ALL 12 TESTS PASSED!');
ELSE
    PRINT('Status: SOME TESTS FAILED');
END_IF;

END_PROGRAM
