(* 示例程序 5: CASE 语句 *)

PROGRAM CaseExample
VAR
    day : INT := 3;
    day_name : STRING;
    month : INT := 12;
    season : STRING;
END_VAR

    (* 根据数字显示星期几 *)
    CASE day OF
        1: day_name := 'Monday';
        2: day_name := 'Tuesday';
        3: day_name := 'Wednesday';
        4: day_name := 'Thursday';
        5: day_name := 'Friday';
        6: day_name := 'Saturday';
        7: day_name := 'Sunday';
    ELSE
        day_name := 'Invalid';
    END_CASE;
    
    (* 根据月份判断季节 *)
    CASE month OF
        12, 1, 2: season := 'Winter';
        3, 4, 5: season := 'Spring';
        6, 7, 8: season := 'Summer';
        9, 10, 11: season := 'Fall';
    ELSE
        season := 'Unknown';
    END_CASE;
END_PROGRAM
