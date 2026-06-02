(* 全面测试多值标签 CASE *)

PROGRAM CaseFullTest
VAR
    month : INT;
    season : STRING;
END_VAR

    (* 测试 Winter: month = 1 *)
    month := 1;
    CASE month OF
        12, 1, 2: season := 'Winter';
        3, 4, 5: season := 'Spring';
        6, 7, 8: season := 'Summer';
        9, 10, 11: season := 'Fall';
    ELSE
        season := 'Unknown';
    END_CASE;
    PRINT(season);
    
    (* 测试 Spring: month = 4 *)
    month := 4;
    CASE month OF
        12, 1, 2: season := 'Winter';
        3, 4, 5: season := 'Spring';
        6, 7, 8: season := 'Summer';
        9, 10, 11: season := 'Fall';
    ELSE
        season := 'Unknown';
    END_CASE;
    PRINT(season);
    
    (* 测试 Summer: month = 7 *)
    month := 7;
    CASE month OF
        12, 1, 2: season := 'Winter';
        3, 4, 5: season := 'Spring';
        6, 7, 8: season := 'Summer';
        9, 10, 11: season := 'Fall';
    ELSE
        season := 'Unknown';
    END_CASE;
    PRINT(season);
    
    (* 测试 Fall: month = 10 *)
    month := 10;
    CASE month OF
        12, 1, 2: season := 'Winter';
        3, 4, 5: season := 'Spring';
        6, 7, 8: season := 'Summer';
        9, 10, 11: season := 'Fall';
    ELSE
        season := 'Unknown';
    END_CASE;
    PRINT(season);
    
    (* 测试 ELSE: month = 13 *)
    month := 13;
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
