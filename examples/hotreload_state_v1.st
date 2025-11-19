PROGRAM HotReloadState_V1
VAR
    g_count : INT;
    g_temp : REAL;
END_VAR

    PRINT('V1: Initializing...\n');
    (* Only initialize if 0, though in V1 it starts at 0 *)
    IF g_count = 0 THEN
        g_count := 10;
        g_temp := 1.5;
    END_IF;
    
    PRINT('V1: g_count = %d, g_temp = %f\n', g_count, g_temp);
    g_count := g_count + 1;
    PRINT('V1: Incremented g_count to %d\n', g_count);

END_PROGRAM
