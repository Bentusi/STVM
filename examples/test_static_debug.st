PROGRAM test_static_debug

VAR
    result_a1 : INT;
    result_a2 : INT;
    result_b1 : INT;
    result_b2 : INT;
END_VAR

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

(* 测试 Counter_A *)
result_a1 := Counter_A();
result_a2 := Counter_A();

PRINT('Counter_A call 1: %d (expected: 1)\n', result_a1);
PRINT('Counter_A call 2: %d (expected: 2)\n', result_a2);

(* 测试 Counter_B *)
result_b1 := Counter_B();
result_b2 := Counter_B();

PRINT('Counter_B call 1: %d (expected: 110)\n', result_b1);
PRINT('Counter_B call 2: %d (expected: 120)\n', result_b2);

END_PROGRAM
