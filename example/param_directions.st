FUNCTION TestParams : BOOL
VAR_INPUT
    input_val : INT;
END_VAR
VAR_OUTPUT
    output_val : INT;
END_VAR
VAR_IN_OUT
    inout_val : INT;
END_VAR
    output_val := input_val * 2;
    inout_val := inout_val + 1;
    RETURN TRUE;
END_FUNCTION