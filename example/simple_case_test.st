PROGRAM SimpleCaseTest
VAR
    input_value : INT;
    state : INT;
    output : INT;
END_VAR

(* 测试1: 基本CASE功能 *)
input_value := 2;
output := 0;

CASE input_value OF
    1: output := 100;
    2: output := 200;
    3: output := 300;
END_CASE

(* 测试2: 状态转换 *)
state := 0;  (* 初始状态：STOP *)

(* 模拟启动命令 *)
input_value := 1;
CASE input_value OF
    1: state := 1;  (* 转到RUN状态 *)
    2: state := 2;  (* 转到WAIT状态 *)
    3: state := 0;  (* 转到STOP状态 *)
END_CASE

(* 测试3: 验证状态转换结果 *)
CASE state OF
    0: output := 999;  (* STOP *)
    1: output := 888;  (* RUN *)
    2: output := 777;  (* WAIT *)
END_CASE

END_PROGRAM