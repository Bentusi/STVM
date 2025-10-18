(* 简单PID控制器示例 *)

PROGRAM SimplePID

VAR
    Kp : REAL;
    Ki : REAL;
    Kd : REAL;
    integral : REAL;
    prev_error : REAL;
    target : REAL;
    current : REAL;
    output : REAL;
    error : REAL;
    i : INT;
    temp : REAL;
END_VAR

    Kp := 0.5;
    Ki := 0.1;
    Kd := 0.05;
    integral := 0.0;
    prev_error := 0.0;
    target := 50.0;
    current := 20.0;
    
    PRINT('PID Control Demo\n');
    PRINT('Target: %f, Start: %f\n', target, current);
    PRINT('\n');
    
    FOR i := 1 TO 20 DO
        error := target - current;
        
        output := Kp * error;
        integral := integral + error;
        output := output + Ki * integral;
        output := output + Kd * (error - prev_error);
        
        IF output > 10.0 THEN
            output := 10.0;
        END_IF;
        IF output < -10.0 THEN
            output := -10.0;
        END_IF;
        
        temp := 0.3 * output;
        current := current + temp;
        prev_error := error;
        
        IF i = 1 OR i = 10 OR i = 20 THEN
            PRINT('Step %d: Current=%f Error=%f\n', i, current, error);
        END_IF;
    END_FOR;
    
    PRINT('\nFinal: %f\n', current);

END_PROGRAM
