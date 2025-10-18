(* =========================================== *)(* =========================================== *)

(* 简化 PID 控制器示例                          *)(* 简化 PID 控制器示例                          *)

(* 适合初学者理解PID原理                         *)(* 适合初学者理解PID原理                         *)

(* =========================================== *)(* =========================================== *)



PROGRAM SimplePIDPROGRAM Simple_PID



VAR_GLOBALVAR_GLOBAL

    Kp : REAL;    (* PID增益参数 *)

    Ki : REAL;    Kp : REAL;

    Kd : REAL;    Ki : REAL;

    integral_sum : REAL;    Kd : REAL;

    previous_error : REAL;    

END_VAR    (* 控制器内部状态 *)

    integral_sum : REAL;

VAR    previous_error : REAL;

    target : REAL;END_VAR

    current : REAL;

    output : REAL;VAR

    step : INT;    (* 系统参数 *)

    error : REAL;    target : REAL;

    p_part : REAL;    current : REAL;

    i_part : REAL;    output : REAL;

    d_part : REAL;    

    temp : REAL;    (* 测试变量 *)

END_VAR    step : INT;

    error : REAL;

    (* 初始化 *)    p_part : REAL;

    Kp := 0.5;    i_part : REAL;

    Ki := 0.1;    d_part : REAL;

    Kd := 0.05;END_VAR

    integral_sum := 0.0;

    previous_error := 0.0;    (* 初始化参数 *)

    target := 50.0;    Kp := 0.5;

    current := 20.0;    Ki := 0.1;

        Kd := 0.05;

    PRINT('=============================\n');    integral_sum := 0.0;

    PRINT('Simple PID Controller Demo\n');    previous_error := 0.0;

    PRINT('=============================\n');    

    PRINT('Target: %f, Start: %f\n', target, current);    target := 50.0;

    PRINT('Kp=%f Ki=%f Kd=%f\n', Kp, Ki, Kd);    current := 20.0;

    PRINT('\n');    output := 0.0;

        

    (* 主控制循环 - 20步 *)    PRINT('======================\n');

    FOR step := 1 TO 20 DO    PRINT('Simple PID Controller\n');

        (* 计算误差 *)    PRINT('======================\n');

        error := target - current;    PRINT('\n');

            

        (* P部分 *)    (* 显示初始状态 *)

        p_part := Kp * error;    PRINT('Initial State:\n');

            PRINT('Target Value: %f\n', target);

        (* I部分 *)    PRINT('Current Value: %f\n', current);

        integral_sum := integral_sum + error;    PRINT('\n');

        i_part := Ki * integral_sum;    

            PRINT('PID Gains:');

        (* D部分 *)    PRINT('Kp =');

        d_part := Kd * (error - previous_error);    PRINT(Kp);

            PRINT('Ki =');

        (* PID输出 *)    PRINT(Ki);

        output := p_part + i_part + d_part;    PRINT('Kd =');

            PRINT(Kd);

        (* 限幅 *)    PRINT('');

        IF output > 10.0 THEN    

            output := 10.0;    (* ===================================== *)

        END_IF;    (* 主控制循环 - 模拟20个控制周期          *)

        IF output < -10.0 THEN    (* ===================================== *)

            output := -10.0;    PRINT('Starting Control Loop...');

        END_IF;    PRINT('');

            

        (* 模拟被控对象 *)    FOR step := 1 TO 20 BY 1 DO

        temp := 0.3 * output;        (* 计算误差 *)

        current := current + temp;        error := target - current;

                

        (* 保存误差 *)        (* P部分：比例控制 *)

        previous_error := error;        p_part := Kp * error;

                

        (* 输出 *)        (* I部分：积分控制 *)

        PRINT('Step %d: Current=%f Error=%f Output=%f\n', step, current, error, output);        integral_sum := integral_sum + error;

                i_part := Ki * integral_sum;

        (* 显示PID各部分 *)        

        IF step = 1 OR step = 10 OR step = 20 THEN        (* D部分：微分控制 *)

            PRINT('  [P=%f I=%f D=%f]\n', p_part, i_part, d_part);        d_part := Kd * (error - previous_error);

        END_IF;        

    END_FOR;        (* PID总输出 *)

            output := p_part + i_part + d_part;

    PRINT('\n');        

    PRINT('Control Complete!\n');        (* 限制输出范围 -10 到 10 *)

    PRINT('Final Current: %f\n', current);        IF output > 10.0 THEN

    error := target - current;            output := 10.0;

    PRINT('Final Error: %f\n', error);        END_IF;

    PRINT('\n');        IF output < -10.0 THEN

                output := -10.0;

    (* 场景2：目标值改变 *)        END_IF;

    PRINT('=============================\n');        

    PRINT('Scenario 2: Target Change\n');        (* 模拟被控对象：简单的一阶系统 *)

    PRINT('=============================\n');        (* 新值 = 旧值 + 0.3 * 控制输出 *)

            current := current + 0.3 * output;

    target := 80.0;        

    PRINT('New Target: %f\n', target);        (* 保存误差供下次计算微分用 *)

    PRINT('Starting from: %f\n', current);        previous_error := error;

            

    (* 重置积分 *)        (* 输出控制信息 *)

    integral_sum := 0.0;        PRINT('Step:');

            PRINT(step);

    FOR step := 1 TO 15 DO        PRINT('  Current:');

        error := target - current;        PRINT(current);

        p_part := Kp * error;        PRINT('  Error:');

                PRINT(error);

        integral_sum := integral_sum + error;        PRINT('  Output:');

        i_part := Ki * integral_sum;        PRINT(output);

                

        d_part := Kd * (error - previous_error);        (* 显示PID各部分贡献 *)

        output := p_part + i_part + d_part;        IF step = 1 OR step = 10 OR step = 20 THEN

                    PRINT('  [P:');

        IF output > 10.0 THEN            PRINT(p_part);

            output := 10.0;            PRINT('   I:');

        END_IF;            PRINT(i_part);

        IF output < -10.0 THEN            PRINT('   D:');

            output := -10.0;            PRINT(d_part);

        END_IF;            PRINT(']');

                END_IF;

        temp := 0.3 * output;    END_FOR;

        current := current + temp;    

        previous_error := error;    PRINT('');

            PRINT('======================');

        PRINT('Step %d: Current=%f Error=%f\n', step, current, error);    PRINT('Control Complete');

    END_FOR;    PRINT('======================');

        PRINT('Final Current Value:');

    PRINT('\nFinal Value: %f\n', current);    PRINT(current);

    PRINT('\n=============================\n');    PRINT('Final Error:');

    PRINT('Demo Complete!\n');    error := target - current;

    PRINT('=============================\n');    PRINT(error);

    

END_PROGRAM    (* ===================================== *)

    (* 测试场景2：目标值改变                  *)
    (* ===================================== *)
    PRINT('');
    PRINT('======================');
    PRINT('Scenario 2: Target Change');
    PRINT('======================');
    
    (* 改变目标值 *)
    target := 80.0;
    PRINT('New Target:');
    PRINT(target);
    PRINT('Starting from:');
    PRINT(current);
    PRINT('');
    
    (* 重置积分项避免累积 *)
    integral_sum := 0.0;
    
    (* 再运行15步 *)
    FOR step := 1 TO 15 BY 1 DO
        error := target - current;
        p_part := Kp * error;
        
        integral_sum := integral_sum + error;
        i_part := Ki * integral_sum;
        
        d_part := Kd * (error - previous_error);
        
        output := p_part + i_part + d_part;
        
        IF output > 10.0 THEN
            output := 10.0;
        END_IF;
        IF output < -10.0 THEN
            output := -10.0;
        END_IF;
        
        current := current + 0.3 * output;
        previous_error := error;
        
        IF (step MOD 3) = 0 THEN
            PRINT('Step:');
            PRINT(step);
            PRINT('  Current:');
            PRINT(current);
            PRINT('  Error:');
            PRINT(error);
        END_IF;
    END_FOR;
    
    PRINT('');
    PRINT('Final Value:');
    PRINT(current);
    PRINT('');
    
    (* ===================================== *)
    (* 测试场景3：纯P控制 vs PID控制对比      *)
    (* ===================================== *)
    PRINT('======================');
    PRINT('Scenario 3: P vs PID');
    PRINT('======================');
    PRINT('');
    
    (* 纯P控制测试 *)
    PRINT('--- Pure P Control (Ki=0, Kd=0) ---');
    TestControl(0.5, 0.0, 0.0, 60.0, 30.0);
    
    PRINT('');
    
    (* 完整PID控制测试 *)
    PRINT('--- Full PID Control ---');
    TestControl(0.5, 0.1, 0.05, 60.0, 30.0);
    
    PRINT('');
    PRINT('======================');
    PRINT('Demo Complete!');
    PRINT('======================');

END_PROGRAM

(* ===================================== *)
(* 辅助函数：测试不同PID参数              *)
(* ===================================== *)
FUNCTION TestControl : VOID
VAR_INPUT
    p_gain : REAL;
    i_gain : REAL;
    d_gain : REAL;
    set_target : REAL;
    start_value : REAL;
END_VAR
VAR
    test_current : REAL;
    test_output : REAL;
    test_error : REAL;
    test_integral : REAL;
    test_prev_error : REAL;
    test_p : REAL;
    test_i : REAL;
    test_d : REAL;
    n : INT;
END_VAR
    test_current := start_value;
    test_integral := 0.0;
    test_prev_error := 0.0;
    
    PRINT('Starting:');
    PRINT(test_current);
    PRINT('Target:');
    PRINT(set_target);
    
    FOR n := 1 TO 12 BY 1 DO
        test_error := set_target - test_current;
        
        test_p := p_gain * test_error;
        
        test_integral := test_integral + test_error;
        test_i := i_gain * test_integral;
        
        test_d := d_gain * (test_error - test_prev_error);
        
        test_output := test_p + test_i + test_d;
        
        IF test_output > 10.0 THEN
            test_output := 10.0;
        END_IF;
        IF test_output < -10.0 THEN
            test_output := -10.0;
        END_IF;
        
        test_current := test_current + 0.3 * test_output;
        test_prev_error := test_error;
        
        IF (n MOD 3) = 0 THEN
            PRINT('Step:');
            PRINT(n);
            PRINT(' Value:');
            PRINT(test_current);
        END_IF;
    END_FOR;
    
    PRINT('Final:');
    PRINT(test_current);
    PRINT('Final Error:');
    test_error := set_target - test_current;
    PRINT(test_error);
END_FUNCTION
