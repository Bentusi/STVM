(*
 * 简单位操作测试
 * 测试基本位运算功能
 *)

PROGRAM SimpleBitOpsTest

VAR
    a : INT;
    b : INT;
    result : INT;
END_VAR

BEGIN
    PRINT('=== 简单位操作测试 ===');
    
    (* 测试位与 *)
    a := 15;   (* 0x0F *)
    b := 51;   (* 0x33 *)
    result := a AND b;  (* 应该是 3 *)
    PRINT('位与测试: 15 AND 51 = 3');
    
    (* 测试位或 *)
    a := 15;   (* 0x0F *)
    b := 48;   (* 0x30 *)
    result := a OR b;   (* 应该是 63 *)
    PRINT('位或测试: 15 OR 48 = 63');
    
    (* 测试位异或 *)
    a := 255;  (* 0xFF *)
    b := 170;  (* 0xAA *)
    result := a XOR b;  (* 应该是 85 *)
    PRINT('位异或测试: 255 XOR 170 = 85');
    
    (* 测试左移 *)
    a := 1;
    result := a SHL 3;  (* 应该是 8 *)
    PRINT('左移测试: 1 SHL 3 = 8');
    
    (* 测试右移 *)
    a := 16;
    result := a SHR 2;  (* 应该是 4 *)
    PRINT('右移测试: 16 SHR 2 = 4');
    
    PRINT('=== 测试完成 ===');
END_PROGRAM
