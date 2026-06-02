PROGRAM FunctionBasedQualityTest
VAR
    temperature : QREAL;
    quality_val : INT;
    normal_temp : REAL;
    temp_with_bad_quality : QREAL;
END_VAR

(* 创建带指定质量位的值 *)
temperature := MakeQReal(25.5, 0);  (* 值=25.5, 质量位=GOOD *)
temp_with_bad_quality := MakeQReal(100.0, 2);  (* 值=100.0, 质量位=BAD *)

(* 获取质量位 *)
quality_val := GetQuality(temperature);
PRINT("Temperature quality: %d", quality_val);

quality_val := GetQuality(temp_with_bad_quality);
PRINT("Bad temperature quality: %d", quality_val);

(* 提取普通值 *)
normal_temp := ToReal(temperature);
PRINT("Extracted temperature value: %f", normal_temp);

normal_temp := ToReal(temp_with_bad_quality);
PRINT("Extracted bad temperature value: %f", normal_temp);

(* 测试质量位传播 *)
temperature := temperature + temp_with_bad_quality;
quality_val := GetQuality(temperature);
PRINT("Result after adding bad quality: quality=%d", quality_val);

PRINT("Final temperature: %f", ToReal(temperature));

END_PROGRAM