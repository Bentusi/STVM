(*
 * 位操作示例程序
 * 展示 IEC 61131-3 标准位操作功能
 * 
 * 包含以下实际应用场景：
 * 1. GPIO 端口控制
 * 2. 位掩码操作
 * 3. 数据打包/解包
 * 4. 快速乘除法
 *)

PROGRAM BitOpsDemo

VAR
    (* GPIO 寄存器模拟 *)
    gpio_data : INT;
    gpio_direction : INT;
    
    (* 位掩码 *)
    mask_set : INT;
    mask_clear : INT;
    
    (* 数据打包 *)
    byte1 : INT;
    byte2 : INT;
    word_result : INT;
    
    (* 快速计算 *)
    value : INT;
    multiplied : INT;
    divided : INT;
    
    (* 状态标志 *)
    flag_enabled : INT;
    flag_active : INT;
END_VAR

BEGIN
    PRINT('=== 位操作演示程序 ===');
    PRINT('');
    
    (* ========================================
       场景1：GPIO端口控制
       ======================================== *)
    PRINT('场景1：GPIO端口控制');
    PRINT('------------------');
    
    (* 初始化GPIO寄存器 *)
    gpio_data := 0;
    PRINT('初始GPIO状态: ');
    PRINT(gpio_data);
    
    (* 设置引脚2和引脚5为高电平 *)
    gpio_data := (1 SHL 2) OR (1 SHL 5);
    PRINT('设置引脚2和5后: ');
    PRINT(gpio_data);
    
    (* 清除引脚2 *)
    gpio_data := gpio_data AND NOT (1 SHL 2);
    PRINT('清除引脚2后: ');
    PRINT(gpio_data);
    
    (* 翻转引脚5 *)
    gpio_data := gpio_data XOR (1 SHL 5);
    PRINT('翻转引脚5后: ');
    PRINT(gpio_data);
    
    PRINT('');
    
    (* ========================================
       场景2：位掩码操作
       ======================================== *)
    PRINT('场景2：位掩码操作');
    PRINT('------------------');
    
    (* 定义掩码 *)
    mask_set := 60;      (* 0b00111100 - 设置位2-5 *)
    mask_clear := 195;   (* 0b11000011 - 清除位2-5 *)
    
    value := 255;        (* 0b11111111 *)
    PRINT('原始值: ');
    PRINT(value);
    
    (* 应用清除掩码 *)
    value := value AND mask_clear;
    PRINT('应用清除掩码后: ');
    PRINT(value);
    
    (* 应用设置掩码 *)
    value := value OR mask_set;
    PRINT('应用设置掩码后: ');
    PRINT(value);
    
    PRINT('');
    
    (* ========================================
       场景3：数据打包（字节合并）
       ======================================== *)
    PRINT('场景3：数据打包');
    PRINT('------------------');
    
    byte1 := 18;   (* 0x12 *)
    byte2 := 52;   (* 0x34 *)
    
    PRINT('字节1: ');
    PRINT(byte1);
    PRINT('字节2: ');
    PRINT(byte2);
    
    (* 将两个字节打包成一个字 *)
    word_result := (byte1 SHL 8) OR byte2;
    PRINT('打包结果: ');
    PRINT(word_result);
    
    (* 解包：提取高字节 *)
    byte1 := (word_result SHR 8) AND 255;
    PRINT('解包高字节: ');
    PRINT(byte1);
    
    (* 解包：提取低字节 *)
    byte2 := word_result AND 255;
    PRINT('解包低字节: ');
    PRINT(byte2);
    
    PRINT('');
    
    (* ========================================
       场景4：快速乘除法（2的幂次）
       ======================================== *)
    PRINT('场景4：快速乘除法');
    PRINT('------------------');
    
    value := 10;
    PRINT('原始值: ');
    PRINT(value);
    
    (* 乘以8 (左移3位) *)
    multiplied := value SHL 3;
    PRINT('乘以8: ');
    PRINT(multiplied);
    
    (* 除以4 (右移2位) *)
    divided := multiplied SHR 2;
    PRINT('除以4: ');
    PRINT(divided);
    
    PRINT('');
    
    (* ========================================
       场景5：状态标志管理
       ======================================== *)
    PRINT('场景5：状态标志管理');
    PRINT('------------------');
    
    (* 定义标志位 *)
    flag_enabled := 1;  (* 位0 *)
    flag_active := 2;   (* 位1 *)
    
    value := 0;
    PRINT('初始状态: ');
    PRINT(value);
    
    (* 启用功能 *)
    value := value OR flag_enabled;
    PRINT('启用后: ');
    PRINT(value);
    
    (* 激活 *)
    value := value OR flag_active;
    PRINT('激活后: ');
    PRINT(value);
    
    (* 禁用功能 *)
    value := value AND NOT flag_enabled;
    PRINT('禁用后: ');
    PRINT(value);
    
    PRINT('');
    PRINT('=== 演示完成 ===');
END_PROGRAM
