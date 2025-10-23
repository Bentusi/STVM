(* 测试包含数字的函数名 *)

PROGRAM TestFunctionNames
VAR
    result1 : INT;
    result2 : INT;
    result3 : INT;
END_VAR

(* ✅ 合法：字母开头，包含数字 *)
FUNCTION add2numbers : INT
VAR_INPUT
    x : INT;
    y : INT;
END_VAR
    add2numbers := x + y;
END_FUNCTION

(* ✅ 合法：字母开头，数字在中间 *)
FUNCTION calc3times : INT
VAR_INPUT
    value : INT;
END_VAR
    calc3times := value * 3;
END_FUNCTION

(* ✅ 合法：下划线开头，包含数字 *)
FUNCTION _test123 : INT
    _test123 := 456;
END_FUNCTION

(* ✅ 合法：实际的三取二函数名 *)
FUNCTION vote_2oo3 : INT
VAR_INPUT
    a : INT;
    b : INT;
    c : INT;
END_VAR
VAR_LOCAL
    count : INT;
END_VAR
    count := 0;
    IF a = 1 THEN count := count + 1; END_IF;
    IF b = 1 THEN count := count + 1; END_IF;
    IF c = 1 THEN count := count + 1; END_IF;
    
    IF count >= 2 THEN
        vote_2oo3 := 1;
    ELSE
        vote_2oo3 := 0;
    END_IF;
END_FUNCTION

BEGIN
    (* 测试所有函数 *)
    result1 := add2numbers(10, 20);
    PRINT('add2numbers(10, 20) = %d\n', result1);
    
    result2 := calc3times(7);
    PRINT('calc3times(7) = %d\n', result2);
    
    result3 := _test123();
    PRINT('_test123() = %d\n', result3);
    
    (* 测试三取二表决 *)
    result1 := vote_2oo3(1, 1, 0);
    PRINT('vote_2oo3(1,1,0) = %d\n', result1);
    
    result2 := vote_2oo3(1, 0, 0);
    PRINT('vote_2oo3(1,0,0) = %d\n', result2);
    
    result3 := vote_2oo3(0, 1, 1);
    PRINT('vote_2oo3(0,1,1) = %d\n', result3);
END_PROGRAM
