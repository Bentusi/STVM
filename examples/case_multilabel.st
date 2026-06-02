(* 测试多值标签 *)

PROGRAM MultiLabelTest
VAR
    month : INT := 12;
    season : STRING;
END_VAR

    CASE month OF
        12, 1, 2: season := 'Winter';
        3, 4, 5: season := 'Spring';
        6, 7, 8: season := 'Summer';
        9, 10, 11: season := 'Fall';
    ELSE
        season := 'Unknown';
    END_CASE;
    
    PRINT(season);
END_PROGRAM
