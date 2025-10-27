(*
 * 变量强制功能演示程序
 * 演示如何在运行时强制变量值
 *)

PROGRAM ForceDemo

VAR
    counter : INT;
    temperature : REAL;
    alarm : BOOL;
    status : STRING;
    forced_temp : QREAL;
    i : INT;
END_VAR

BEGIN
    PRINT("=== 变量强制功能演示 ===\n\n");
    
    (* 示例1: 正常执行 - 无强制 *)
    PRINT("--- 示例1: 正常执行（无强制）---\n");
    counter := 0;
    FOR i := 1 TO 5 DO
        counter := counter + 1;
        PRINT("循环 %d: counter = %d\n", i, counter);
    END_FOR;
    PRINT("\n");
    
    (* 示例2: 演示强制效果 *)
    PRINT("--- 示例2: 强制counter为固定值 ---\n");
    PRINT("提示: 使用调试器命令 'force counter 100' 强制counter\n");
    PRINT("然后观察counter值不会改变\n");
    counter := 0;
    FOR i := 1 TO 5 DO
        counter := counter + 1;
        PRINT("循环 %d: counter = %d (应该是1,2,3,4,5)\n", i, counter);
        (* 如果counter被强制为100，这里将始终显示100 *)
    END_FOR;
    PRINT("\n");
    
    (* 示例3: 温度传感器模拟 *)
    PRINT("--- 示例3: 模拟传感器读数 ---\n");
    temperature := 25.0;
    PRINT("初始温度: %f C\n", temperature);
    
    FOR i := 1 TO 5 DO
        (* 正常情况下温度会逐渐上升 *)
        temperature := temperature + 0.5;
        
        (* 检查温度是否过高 *)
        IF temperature > 27.0 THEN
            alarm := TRUE;
            PRINT("周期 %d: 温度 %f C - 警报!\n", i, temperature);
        ELSE
            alarm := FALSE;
            PRINT("周期 %d: 温度 %f C - 正常\n", i, temperature);
        END_IF;
    END_FOR;
    
    PRINT("\n提示: 使用 'force temperature 30.0' 模拟高温\n");
    PRINT("提示: 使用 'force alarm FALSE' 抑制警报\n\n");
    
    (* 示例4: 质量位变量强制 *)
    PRINT("--- 示例4: 质量位变量强制 ---\n");
    forced_temp.VAL := 22.5;
    forced_temp.QUALITY := 0;  (* GOOD *)
    
    PRINT("传感器温度: %f, 质量: %d\n", forced_temp.VAL, forced_temp.QUALITY);
    
    (* 可以分别强制VAL和QUALITY *)
    PRINT("提示: 'force forced_temp.VAL 100.0' - 强制温度值\n");
    PRINT("提示: 'force forced_temp.QUALITY 2' - 强制质量位为BAD\n\n");
    
    (* 示例5: 条件测试 *)
    PRINT("--- 示例5: 使用强制测试不同条件 ---\n");
    status := 'NORMAL';
    
    CASE status OF
        'NORMAL':  PRINT("状态: 正常运行\n");
        'WARNING': PRINT("状态: 警告\n");
        'ERROR':   PRINT("状态: 错误\n");
        'FAULT':   PRINT("状态: 故障\n");
    END_CASE;
    
    PRINT("\n提示: 使用 'force status WARNING' 测试警告情况\n");
    PRINT("提示: 使用 'force status ERROR' 测试错误情况\n");
    PRINT("提示: 使用 'force status FAULT' 测试故障情况\n\n");
    
    (* 示例6: 调试命令示例 *)
    PRINT("=== 强制功能调试命令 ===\n");
    PRINT("force <var> <value>      - 强制变量为指定值\n");
    PRINT("force <var> <value> -p   - 持久化强制（跨周期）\n");
    PRINT("unforce <var>            - 取消变量强制\n");
    PRINT("unforce_all              - 取消所有强制\n");
    PRINT("force_enable <0|1>       - 禁用/启用全局强制\n");
    PRINT("force_status             - 显示所有强制状态\n");
    PRINT("force_save <file>        - 保存强制列表到文件\n");
    PRINT("force_load <file>        - 从文件加载强制列表\n\n");
    
    PRINT("=== 演示完成 ===\n");
    
END_PROGRAM
