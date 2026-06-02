(*
 * PRINT Function printf-style Examples
 * Demonstrates formatted output
 *)

PROGRAM PrintExamples

VAR
    count : INT;
    temperature : REAL;
    status : BOOL;
    name : STRING;
    sensor_temp : QREAL;
END_VAR

BEGIN
    PRINT("=== PRINT Function Examples ===\n");
    
    (* Example 1: Basic string output *)
    PRINT("--- Example 1: Basic Output ---\n");
    PRINT("Hello, World!\n");
    PRINT("Line 1\n");
    PRINT("Line 2\n\n");
    
    (* Example 2: Integer formatting *)
    PRINT("--- Example 2: Integer Output ---\n");
    count := 42;
    PRINT("Count: %d\n", count);
    PRINT("Decimal: %d, Same: %d\n", count, count);
    PRINT("\n");
    
    (* Example 3: Real number formatting *)
    PRINT("--- Example 3: Real Output ---\n");
    temperature := 25.567;
    PRINT("Temperature: %f C\n", temperature);
    PRINT("Precise value: %f\n", temperature);
    PRINT("\n");
    
    (* Example 4: Boolean output *)
    PRINT("--- Example 4: Boolean ---\n");
    status := TRUE;
    PRINT("Status1: %b\n", status);
    status := FALSE;
    PRINT("Status2: %b\n", status);
    PRINT("\n");
    
    (* Example 5: String output *)
    PRINT("--- Example 5: String ---\n");
    name := 'SENSOR_01';
    PRINT("Device name: %s\n", name);
    PRINT("Full info: device=%s, count=%d\n", name, count);
    PRINT("\n");
    
    (* Example 6: Mixed types *)
    PRINT("--- Example 6: Mixed Types ---\n");
    count := 10;
    temperature := 28.5;
    status := TRUE;
    PRINT("count=%d, temp=%f, status=%b\n", count, temperature, status);
    PRINT("\n");
    
    (* Example 7: Quality variables *)
    PRINT("--- Example 7: Quality Variables ---\n");
    sensor_temp.VAL := 30.5;
    sensor_temp.QUALITY := 0;
    PRINT("Sensor temp: %f C\n", sensor_temp.VAL);
    PRINT("Data quality: %d (GOOD)\n", sensor_temp.QUALITY);
    PRINT("Full: value=%f, quality=%d\n", sensor_temp.VAL, sensor_temp.QUALITY);
    PRINT("\n");
    
    (* Example 8: Escape sequences *)
    PRINT("--- Example 8: Escape Sequences ---\n");
    PRINT("Newline: Line1\nLine2\n");
    PRINT("Tab:\tData1\tData2\n");
    PRINT("Percent: Progress 50%%\n");
    PRINT("Backslash: C:\\Path\\File\n");
    PRINT("\n");
    
    (* Example 9: Conditional output *)
    PRINT("--- Example 9: Conditional Output ---\n");
    temperature := 26.0;
    IF temperature > 25.0 THEN
        PRINT("Warning: High temp (%f C)\n", temperature);
    ELSE
        PRINT("Normal: Good temp (%f C)\n", temperature);
    END_IF;
    PRINT("\n");
    
    (* Example 10: Loop output *)
    PRINT("--- Example 10: Loop Output ---\n");
    PRINT("Countdown:\n");
    FOR count := 5 TO 1 BY -1 DO
        PRINT("  %d...\n", count);
    END_FOR;
    PRINT("  Done!\n\n");
    
    PRINT("=== All Examples Complete ===\n");
    
END_PROGRAM
