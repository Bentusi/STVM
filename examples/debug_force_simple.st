PROGRAM DebugForceSimple
VAR
    counter : INT := 0;
    temperature : REAL := 20.0;
    alarm : BOOL := FALSE;
END_VAR

    counter := counter + 1;
    temperature := temperature + 0.5;
    
    IF counter > 5 THEN
        alarm := TRUE;
    END_IF;

END_PROGRAM
