PROGRAM TestEngineeringLib

IMPORT RS_Trigger FROM 'examples/engineering_lib.stbc';
IMPORT SR_Trigger FROM 'examples/engineering_lib.stbc';
IMPORT SmoothFilter FROM 'examples/engineering_lib.stbc';
IMPORT Vote_1oo2 FROM 'examples/engineering_lib.stbc';
IMPORT Vote_2oo3 FROM 'examples/engineering_lib.stbc';
IMPORT Vote_3oo4 FROM 'examples/engineering_lib.stbc';
IMPORT Limiter FROM 'examples/engineering_lib.stbc';
IMPORT DeadBand FROM 'examples/engineering_lib.stbc';
IMPORT Hysteresis FROM 'examples/engineering_lib.stbc';

VAR
    test_passed : INT := 0;
    test_total : INT := 41;
END_VAR

FUNCTION AssertBool : BOOL
VAR_INPUT
    condition : BOOL;
    test_num : INT;
END_VAR
    IF condition THEN
        PRINT('  [PASS] Test %d\n', test_num);
        AssertBool := TRUE;
    ELSE
        PRINT('  [FAIL] Test %d\n', test_num);
        AssertBool := FALSE;
    END_IF;
END_FUNCTION

FUNCTION TestRS : INT
VAR_LOCAL
    result : BOOL;
    passed : INT;
END_VAR
    PRINT('\n--- Test RS Trigger ---\n');
    passed := 0;
    
    result := RS_Trigger(FALSE, FALSE);
    IF result = FALSE THEN
        passed := passed + 1;
    END_IF;
    
    result := RS_Trigger(TRUE, FALSE);
    IF result = TRUE THEN
        passed := passed + 1;
    END_IF;
    
    PRINT('RS: %d/2 passed\n', passed);
    TestRS := passed;
END_FUNCTION

    PRINT('\nEngineering Library Tests\n');
    
    test_passed := test_passed + TestRS();
    
    PRINT('\nTotal: %d/%d passed\n', test_passed, test_total);

END_PROGRAM
