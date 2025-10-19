PROGRAM test_static_complete

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
    pass_count : INT := 0;
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
result_a1 := Counter_A();
result_a2 := Counter_A();
result_a3 := Counter_A();

(* 测试 Counter_B - 应该从 100 开始递增 *)
result_b1 := Counter_B();
result_b2 := Counter_B();
result_b3 := Counter_B();

(* 测试 Toggle_A *)
toggle_a1 := Toggle_A();
toggle_a2 := Toggle_A();
toggle_a3 := Toggle_A();

(* 测试 Toggle_B *)
toggle_b1 := Toggle_B();
toggle_b2 := Toggle_B();
toggle_b3 := Toggle_B();

(* 输出测试结果 *)
PRINT('=== Static Variable Test ===\n\n');

PRINT('[Test 1] Counter_A (init=0, step=1)\n');
PRINT('  Call 1: %d (expected: 1)\n', result_a1);
IF result_a1 = 1 THEN
    PRINT('    PASS\n');
    pass_count := pass_count + 1;
ELSE
    PRINT('    FAIL\n');
END_IF;

PRINT('  Call 2: %d (expected: 2)\n', result_a2);
IF result_a2 = 2 THEN
    PRINT('    PASS\n');
    pass_count := pass_count + 1;
ELSE
    PRINT('    FAIL\n');
END_IF;

PRINT('  Call 3: %d (expected: 3)\n', result_a3);
IF result_a3 = 3 THEN
    PRINT('    PASS\n');
    pass_count := pass_count + 1;
ELSE
    PRINT('    FAIL\n');
END_IF;

PRINT('\n[Test 2] Counter_B (init=100, step=10)\n');
PRINT('  Call 1: %d (expected: 110)\n', result_b1);
IF result_b1 = 110 THEN
    PRINT('    PASS\n');
    pass_count := pass_count + 1;
ELSE
    PRINT('    FAIL\n');
END_IF;

PRINT('  Call 2: %d (expected: 120)\n', result_b2);
IF result_b2 = 120 THEN
    PRINT('    PASS\n');
    pass_count := pass_count + 1;
ELSE
    PRINT('    FAIL\n');
END_IF;

PRINT('  Call 3: %d (expected: 130)\n', result_b3);
IF result_b3 = 130 THEN
    PRINT('    PASS\n');
    pass_count := pass_count + 1;
ELSE
    PRINT('    FAIL\n');
END_IF;

PRINT('\n[Test 3] Toggle_A (alternating BOOL)\n');
PRINT('  Call 1: %b (expected: TRUE)\n', toggle_a1);
IF toggle_a1 = TRUE THEN
    PRINT('    PASS\n');
    pass_count := pass_count + 1;
ELSE
    PRINT('    FAIL\n');
END_IF;

PRINT('  Call 2: %b (expected: FALSE)\n', toggle_a2);
IF toggle_a2 = FALSE THEN
    PRINT('    PASS\n');
    pass_count := pass_count + 1;
ELSE
    PRINT('    FAIL\n');
END_IF;

PRINT('  Call 3: %b (expected: TRUE)\n', toggle_a3);
IF toggle_a3 = TRUE THEN
    PRINT('    PASS\n');
    pass_count := pass_count + 1;
ELSE
    PRINT('    FAIL\n');
END_IF;

PRINT('\n[Test 4] Toggle_B (independent from A)\n');
PRINT('  Call 1: %b (expected: TRUE)\n', toggle_b1);
IF toggle_b1 = TRUE THEN
    PRINT('    PASS\n');
    pass_count := pass_count + 1;
ELSE
    PRINT('    FAIL\n');
END_IF;

PRINT('  Call 2: %b (expected: FALSE)\n', toggle_b2);
IF toggle_b2 = FALSE THEN
    PRINT('    PASS\n');
    pass_count := pass_count + 1;
ELSE
    PRINT('    FAIL\n');
END_IF;

PRINT('  Call 3: %b (expected: TRUE)\n', toggle_b3);
IF toggle_b3 = TRUE THEN
    PRINT('    PASS\n');
    pass_count := pass_count + 1;
ELSE
    PRINT('    FAIL\n');
END_IF;

PRINT('\n=== Test Summary ===\n');
PRINT('Passed: %d/12 tests\n', pass_count);
IF pass_count = 12 THEN
    PRINT('Status: ALL TESTS PASSED!\n');
ELSE
    PRINT('Status: SOME TESTS FAILED\n');
END_IF;

END_PROGRAM
