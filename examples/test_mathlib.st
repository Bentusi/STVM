PROGRAM TestMath

IMPORT Square FROM 'examples/mathlib.stbc';
IMPORT Power FROM 'examples/mathlib.stbc';
IMPORT Factorial FROM 'examples/mathlib.stbc';

VAR
    result : INT;
END_VAR

result := Square(5);
PRINT('Square(5) = %d\n', result);
result := Power(2, 10);
PRINT('Power(2, 10) = %d\n', result);
result := Factorial(5);
PRINT('Factorial(5) = %d\n', result);

END_PROGRAM
