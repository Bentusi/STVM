(* 数学库快速测试程序 *)

PROGRAM TestMathDemo

IMPORT Power FROM 'examples/mathlib.stbc';
IMPORT Factorial FROM 'examples/mathlib.stbc';
IMPORT GCD FROM 'examples/mathlib.stbc';
IMPORT Fibonacci FROM 'examples/mathlib.stbc';
IMPORT Lerp FROM 'examples/mathlib.stbc';

VAR
    int_result : INT;
    real_result : REAL;
END_VAR

int_result := Power(2, 10);
PRINT('Power(2, 10) = %d\n', int_result);
int_result := Factorial(5);
PRINT('Factorial(5) = %d\n', int_result);
int_result := GCD(48, 18);
PRINT('GCD(48, 18) = %d\n', int_result);
int_result := Fibonacci(10);
PRINT('Fibonacci(10) = %d\n', int_result);
real_result := Lerp(0.0, 100.0, 0.75);
PRINT('Lerp(0.0, 100.0, 0.75) = %f\n', real_result);

END_PROGRAM
