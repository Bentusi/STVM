PROGRAM HotReloadState_V2
VAR
    g_count : INT;      (* Should be preserved *)
    g_new_var : INT;    (* New variable *)
    g_temp : REAL;      (* Should be preserved *)
END_VAR

    PRINT('V2: Loaded.\n');
    PRINT('V2: g_count = %d (Expected > 10)\n', g_count);
    PRINT('V2: g_temp = %f (Expected 1.5)\n', g_temp);
    
    g_new_var := 999;
    PRINT('V2: g_new_var = %d\n', g_new_var);
    
    g_count := g_count + 1;
    PRINT('V2: Incremented g_count to %d\n', g_count);

END_PROGRAM
