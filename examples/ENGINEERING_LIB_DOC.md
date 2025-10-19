# ST工程计算库文档

## 库简介

本ST工程计算库提供了工业控制中常用的算法函数，包括：
- **触发器**: RS/SR触发器
- **滤波器**: 平滑滤波、死区处理
- **投票逻辑**: 1oo2、2oo3、3oo4冗余容错
- **限幅**: 信号限幅、滞环比较

## 函数列表

### 1. RS触发器 (RS_Trigger)

**功能**: Reset优先的RS触发器  
**参数**:
- `Set: BOOL` - 置位输入
- `Reset: BOOL` - 复位输入  
**返回**: `BOOL` - 触发器状态
**特性**: Reset信号优先于Set信号

```st
FUNCTION RS_Trigger : BOOL
VAR_INPUT
    Set : BOOL;
    Reset : BOOL;
END_VAR
```

**使用示例**:
```st
output := RS_Trigger(start_button, stop_button);
```

---

### 2. SR触发器 (SR_Trigger)

**功能**: Set优先的SR触发器  
**参数**:
- `Set: BOOL` - 置位输入
- `Reset: BOOL` - 复位输入  
**返回**: `BOOL` - 触发器状态  
**特性**: Set信号优先于Reset信号

---

### 3. 平滑滤波器 (SmoothFilter)

**功能**: 一阶低通滤波，平滑输入信号  
**算法**: Y(n) = α×X(n) + (1-α)×Y(n-1)  
**参数**:
- `input: REAL` - 当前输入值
- `alpha: REAL` - 平滑系数 [0.0-1.0]，值越大响应越快  
**返回**: `REAL` - 滤波后的值

**应用场景**:
- 传感器信号降噪
- 温度/压力平滑
- PID控制输入预处理

**使用示例**:
```st
(* alpha=0.1 强平滑, alpha=0.9 快速响应 *)
smooth_temp := SmoothFilter(raw_temp, 0.2);
```

---

### 4. 1oo2投票逻辑 (Vote_1oo2)

**功能**: 两个输入中至少一个为TRUE  
**参数**:
- `input1: BOOL` - 通道1
- `input2: BOOL` - 通道2  
**返回**: `BOOL` - 投票结果

**应用场景**:
- 双冗余传感器容错
- 允许一个传感器故障
- 危险检测（任一触发即报警）

**真值表**:
```
input1 | input2 | output
-------|--------|-------
   F   |   F    |   F
   F   |   T    |   T
   T   |   F    |   T
   T   |   T    |   T
```

---

### 5. 2oo3投票逻辑 (Vote_2oo3)

**功能**: 三个输入中至少两个为TRUE  
**参数**:
- `input1, input2, input3: BOOL` - 三个输入通道  
**返回**: `BOOL` - 投票结果

**应用场景**:
- 三重冗余系统（TMR）
- 容忍一个传感器故障
- 安全关键系统

**特点**:
- 最常用的冗余配置
- 平衡可靠性和成本
- 容错能力强

---

### 6. 3oo4投票逻辑 (Vote_3oo4)

**功能**: 四个输入中至少三个为TRUE  
**参数**:
- `input1, input2, input3, input4: BOOL` - 四个输入通道  
**返回**: `BOOL` - 投票结果

**应用场景**:
- 四重冗余系统
- 极高可靠性要求
- 核电、航空航天等领域

---

### 7. 限幅器 (Limiter)

**功能**: 将信号限制在指定范围内  
**参数**:
- `input: REAL` - 输入值
- `min_val: REAL` - 最小值
- `max_val: REAL` - 最大值  
**返回**: `REAL` - 限幅后的值

**特性**:
- 自动处理min>max的情况
- 保护执行器不过载
- 防止控制信号饱和

**使用示例**:
```st
(* 阀门开度限制在0-100% *)
valve_pos := Limiter(pid_output, 0.0, 100.0);
```

---

### 8. 死区处理器 (DeadBand)

**功能**: 小信号抑制，避免抖动  
**参数**:
- `input: REAL` - 输入值
- `threshold: REAL` - 死区阈值（正值）  
**返回**: `REAL` - 处理后的值

**算法**:
- 若 |input| < threshold，输出0
- 若 input > threshold，输出 input - threshold
- 若 input < -threshold，输出 input + threshold

**应用场景**:
- 避免小幅振荡
- 减少执行器磨损
- 控制系统稳定

**使用示例**:
```st
(* 小于±5的偏差不调整 *)
error := DeadBand(setpoint - actual, 5.0);
```

---

### 9. 滞环比较器 (Hysteresis)

**功能**: 带滞环的比较器，防止临界抖动  
**参数**:
- `input: REAL` - 输入值
- `high_th: REAL` - 高阈值（上升沿触发）
- `low_th: REAL` - 低阈值（下降沿触发）  
**返回**: `BOOL` - 比较结果

**工作原理**:
- 输入 >= high_th → 输出TRUE
- 输入 <= low_th → 输出FALSE
- 在两阈值之间 → 保持原状态

**应用场景**:
- 温度控制（避免频繁开关）
- 液位控制
- 压力开关

**使用示例**:
```st
(* 温度高于80℃开风扇，低于70℃关闭 *)
fan_on := Hysteresis(temperature, 80.0, 70.0);
```

---

## 测试结果

运行 `engineering_basic.st` 测试：

```
=== Engineering Library Basic Test ===

[Test 1] RS Trigger
  1.1 Init: PASS
  1.2 Set: PASS
  1.3 Reset: PASS

[Test 2] Limiter
  2.1 In range: PASS
  2.2 Above max: PASS
  2.3 Below min: PASS

[Test 3] 1oo2 Voting
  3.1 (F,F): PASS
  3.2 (T,F): PASS

[Test 4] 2oo3 Voting
  4.1 (T,F,F): PASS
  4.2 (T,T,F): PASS

=== Summary ===
Passed: 10/10 tests
Status: ALL TESTS PASSED!
```

## 文件列表

1. **engineering_lib.st** - 完整库源码（包含所有函数和文档）
2. **engineering_basic.st** - 基础测试用例（通过验证）
3. **ENGINEERING_LIB_DOC.md** - 本文档

## 使用建议

### 传感器冗余配置
- **关键安全**: 使用2oo3或3oo4
- **一般可靠**: 使用1oo2
- **成本敏感**: 单传感器+滤波

### 滤波器选择
- **慢变信号** (温度): alpha = 0.1-0.3
- **中速信号** (压力): alpha = 0.3-0.6
- **快速信号** (流量): alpha = 0.6-0.9

### 触发器选择
- **紧急停止**: RS_Trigger (Reset优先)
- **启动控制**: SR_Trigger (Set优先)

## 工程应用示例

### 1. 温度控制系统
```st
(* 三重冗余温度传感器 *)
sensor_ok := Vote_2oo3(temp1_ok, temp2_ok, temp3_ok);

(* 平滑滤波 *)
smooth_temp := SmoothFilter(raw_temp, 0.2);

(* 滞环控制加热器 *)
heater_on := Hysteresis(smooth_temp, 75.0, 72.0);
```

### 2. 安全连锁系统
```st
(* 任一紧急按钮触发停机 *)
emergency_stop := Vote_1oo2(e_stop1, e_stop2);

(* 三重冗余安全传感器 *)
safe_condition := Vote_2oo3(sensor1, sensor2, sensor3);

(* RS触发器控制运行状态 *)
running := RS_Trigger(start_btn AND safe_condition, 
                      emergency_stop);
```

### 3. PID控制器保护
```st
(* 限幅PID输出 *)
limited_output := Limiter(pid_output, 0.0, 100.0);

(* 死区避免小幅振荡 *)
error := DeadBand(setpoint - measured, 2.0);
```

## 版本信息

- **版本**: 1.0.0
- **日期**: 2025-10-19
- **状态**: 已测试通过
- **兼容**: STVM v1.x

## 注意事项

1. 函数内部带 `VAR` 块的变量为全局变量（保持状态）
2. 触发器和滤波器需要保持内部状态
3. 投票逻辑为纯函数（无状态）
4. 建议在主循环中周期调用

---

**作者**: STVM Engineering Team  
**联系**: GitHub.com/Bentusi/STVM
