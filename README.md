# IEC61131 结构化文本编译器

一个完整的IEC61131结构化文本语言编译器和虚拟机实现，使用Flex和Bison构建，支持平台无关的字节码执行。

## 特性

- **完整的词法分析**：支持IEC61131标准的关键字、标识符、字面量和运算符
- **语法分析**：使用Bison实现的语法分析器，生成抽象语法树(AST)
- **虚拟机执行**：平台无关的字节码虚拟机，支持控制流和表达式计算
- **数据类型支持**：BOOL、INT、REAL、STRING基本数据类型
- **控制结构**：IF-THEN-ELSE、FOR循环、WHILE循环、CASE分支
- **函数支持**：函数定义、函数调用、参数传递、返回值处理
- **模块化系统**：库导入、命名空间、第三方函数库支持
- **标准库集成**：数学、字符串、时间、I/O等标准函数库
- **表达式计算**：算术运算、比较运算、逻辑运算

## 构建要求

- GCC编译器
- Flex (词法分析器生成器)
- Bison (语法分析器生成器)
- Make构建工具

## 安装依赖

### Ubuntu/Debian
```bash
sudo apt-get install gcc flex bison make
```

### CentOS/RHEL
```bash
sudo yum install gcc flex bison make
```

### macOS
```bash
brew install flex bison gcc make
```

## 构建

```bash
# 构建编译器
make

# 运行测试
make test

# 清理生成文件
make clean

# 安装到系统
make install
```

## 使用方法

### 1. 编译文件
```bash
./stc program.st
```

### 2. 交互模式
```bash
./stc -i
```

### 3. 帮助信息
```bash
./stc -h
```

## 语法支持

### 程序结构
```
PROGRAM MyProgram

(* 库导入声明 *)
IMPORT MathLib;
IMPORT StringLib AS Str;
IMPORT MyCustomLib FROM "libs/custom.st";

VAR
    variable_declarations
END_VAR

    function_declarations
    
    statements

END_PROGRAM
```

### 库导入语法

#### 基本导入
```
(* 导入整个库 *)
IMPORT MathLib;

(* 使用别名导入 *)
IMPORT StringLibrary AS Str;

(* 从指定路径导入 *)
IMPORT CustomFunctions FROM "libs/custom.st";

(* 导入特定函数 *)
IMPORT (Sin, Cos, Tan) FROM MathLib;

(* 导入并使用别名 *)
IMPORT (Sin AS Sine, Cos AS Cosine) FROM MathLib;
```

#### 条件导入
```
(* 根据编译条件导入不同库 *)
{$IFDEF SIMULATION}
    IMPORT SimulationLib;
{$ELSE}
    IMPORT HardwareLib;
{$ENDIF}

(* 版本检查导入 *)
IMPORT MathLib VERSION >= "2.0";
```

### 库定义语法

#### 创建函数库文件 (MathLib.st)
```
LIBRARY MathLib VERSION "2.1.0"

(* 库级常量定义 *)
VAR_CONSTANT
    PI : REAL := 3.14159265359;
    E : REAL := 2.71828182846;
END_VAR

(* 导出函数声明 *)
FUNCTION Sin : REAL
VAR_INPUT
    angle : REAL;
END_VAR
EXPORT;  (* 标记为可导出 *)

FUNCTION Cos : REAL  
VAR_INPUT
    angle : REAL;
END_VAR
EXPORT;

FUNCTION Sqrt : REAL
VAR_INPUT
    value : REAL;
END_VAR
EXPORT;

(* 内部辅助函数，不导出 *)
FUNCTION NormalizeAngle : REAL
VAR_INPUT
    angle : REAL;
END_VAR
(* 无EXPORT标记，内部使用 *)

(* 函数实现 *)
Sin := SIN_BUILTIN(NormalizeAngle(angle));
Cos := COS_BUILTIN(NormalizeAngle(angle));
Sqrt := SQRT_BUILTIN(value);

NormalizeAngle := angle - (FLOOR(angle / (2.0 * PI)) * 2.0 * PI);

END_LIBRARY
```

#### 字符串处理库示例 (StringLib.st)
```
LIBRARY StringLib VERSION "1.5.2"

FUNCTION Length : INT
VAR_INPUT
    str : STRING;
END_VAR
EXPORT;

FUNCTION Concat : STRING
VAR_INPUT
    str1 : STRING;
    str2 : STRING;
END_VAR
EXPORT;

FUNCTION Substring : STRING
VAR_INPUT
    str : STRING;
    start : INT;
    length : INT;
END_VAR
EXPORT;

FUNCTION ToUpper : STRING
VAR_INPUT
    str : STRING;
END_VAR
EXPORT;

FUNCTION IndexOf : INT
VAR_INPUT
    str : STRING;
    search : STRING;
END_VAR
EXPORT;

(* 实现省略... *)

END_LIBRARY
```

### 命名空间和函数调用

#### 完全限定名调用
```
VAR
    result : REAL;
    text : STRING;
    length : INT;
END_VAR

(* 使用库名前缀调用 *)
result := MathLib.Sin(1.57);
result := MathLib.Sqrt(16.0);

(* 使用别名调用 *)
length := Str.Length("Hello World");
text := Str.ToUpper("hello");
```

#### 命名空间导入后直接调用
```
(* 导入特定函数后可直接调用 *)
IMPORT (Sin, Cos, Sqrt) FROM MathLib;
IMPORT (Length, ToUpper) FROM StringLib;

VAR
    angle : REAL := 1.57;
    text : STRING := "hello";
END_VAR

(* 直接调用，无需前缀 *)
result := Sin(angle);
result := Sqrt(25.0);
length := Length(text);
text := ToUpper(text);
```

### 标准库函数

#### 数学库 (MathLib)
```
PROGRAM MathExample
IMPORT MathLib;

VAR
    x : REAL := 45.0;
    y : REAL;
    z : REAL;
END_VAR

(* 三角函数 *)
y := MathLib.Sin(MathLib.DegToRad(x));
y := MathLib.Cos(MathLib.DegToRad(x));
y := MathLib.Tan(MathLib.DegToRad(x));

(* 反三角函数 *)
x := MathLib.RadToDeg(MathLib.ArcSin(0.5));

(* 指数和对数 *)
y := MathLib.Exp(2.0);        (* e^2 *)
y := MathLib.Log(10.0);       (* ln(10) *)
y := MathLib.Log10(100.0);    (* log10(100) *)
y := MathLib.Power(2.0, 8.0); (* 2^8 *)

(* 其他数学函数 *)
y := MathLib.Sqrt(16.0);      (* 平方根 *)
y := MathLib.Abs(-5.5);       (* 绝对值 *)
y := MathLib.Floor(3.7);      (* 向下取整 *)
y := MathLib.Ceil(3.2);       (* 向上取整 *)
y := MathLib.Round(3.6);      (* 四舍五入 *)

END_PROGRAM
```

#### 字符串库 (StringLib)
```
PROGRAM StringExample
IMPORT StringLib AS Str;

VAR
    text1 : STRING := "Hello";
    text2 : STRING := "World";
    result : STRING;
    pos : INT;
    len : INT;
END_VAR

(* 字符串操作 *)
result := Str.Concat(text1, " ");
result := Str.Concat(result, text2);     (* "Hello World" *)

len := Str.Length(result);               (* 11 *)
result := Str.Substring(result, 1, 5);   (* "Hello" *)
result := Str.ToUpper(result);           (* "HELLO" *)
result := Str.ToLower(result);           (* "hello" *)

(* 字符串查找 *)
pos := Str.IndexOf(result, "ll");        (* 2 *)
pos := Str.LastIndexOf(result, "l");     (* 3 *)

(* 字符串判断 *)
IF Str.StartsWith(result, "hel") THEN
    result := Str.Replace(result, "hel", "wel");
END_IF

(* 字符串分割和连接 *)
IMPORT StringArray FROM StringLib;
VAR
    parts : StringArray;
    joined : STRING;
END_VAR

parts := Str.Split("a,b,c,d", ",");
joined := Str.Join(parts, "-");          (* "a-b-c-d" *)

END_PROGRAM
```

#### 时间库 (TimeLib)
```
PROGRAM TimeExample
IMPORT TimeLib;

VAR
    current_time : TIME;
    timestamp : DINT;
    formatted : STRING;
END_VAR

(* 获取当前时间 *)
current_time := TimeLib.Now();
timestamp := TimeLib.GetTimestamp();

(* 时间格式化 *)
formatted := TimeLib.Format(current_time, "YYYY-MM-DD HH:mm:ss");

(* 时间运算 *)
current_time := TimeLib.AddSeconds(current_time, 3600);  (* 加1小时 *)
current_time := TimeLib.AddDays(current_time, 1);        (* 加1天 *)

(* 时间比较 *)
IF TimeLib.IsAfter(current_time, TimeLib.ParseTime("2024-01-01")) THEN
    (* 时间在2024年之后 *)
END_IF

END_PROGRAM
```

#### I/O库 (IOLib)
```
PROGRAM IOExample
IMPORT IOLib;

VAR
    sensor_value : REAL;
    output_state : BOOL;
END_VAR

(* 模拟量输入输出 *)
sensor_value := IOLib.ReadAnalogInput(0);    (* 读取AI0通道 *)
IOLib.WriteAnalogOutput(0, 4.5);             (* 写入AO0通道 *)

(* 数字量输入输出 *)
output_state := IOLib.ReadDigitalInput(1);   (* 读取DI1通道 *)
IOLib.WriteDigitalOutput(1, TRUE);           (* 写入DO1通道 *)

(* 脉冲输出 *)
IOLib.GeneratePulse(2, 1000);                (* 通道2输出1000ms脉冲 *)

END_PROGRAM
```

#### 自定义库开发

#### 创建自定义控制库
```
LIBRARY ControlLib VERSION "1.0.0"
AUTHOR "Control Systems Inc."
DESCRIPTION "PID控制和滤波算法库"

(* PID控制器结构体 *)
TYPE PID_Controller : STRUCT
    Kp : REAL;           (* 比例增益 *)
    Ki : REAL;           (* 积分增益 *)
    Kd : REAL;           (* 微分增益 *)
    SetPoint : REAL;     (* 设定值 *)
    LastError : REAL;    (* 上次误差 *)
    Integral : REAL;     (* 积分累积 *)
    Output : REAL;       (* 输出值 *)
    OutputMin : REAL;    (* 输出下限 *)
    OutputMax : REAL;    (* 输出上限 *)
END_STRUCT;

(* PID控制器初始化 *)
FUNCTION InitPID : PID_Controller
VAR_INPUT
    Kp, Ki, Kd : REAL;
    OutputMin, OutputMax : REAL;
END_VAR
EXPORT;

(* PID控制器计算 *)
FUNCTION UpdatePID : REAL
VAR_IN_OUT
    controller : PID_Controller;
END_VAR
VAR_INPUT
    ProcessValue : REAL;
    DeltaTime : REAL;
END_VAR
EXPORT;

(* 低通滤波器 *)
FUNCTION LowPassFilter : REAL
VAR_INPUT
    input : REAL;
    alpha : REAL;        (* 滤波系数 0-1 *)
END_VAR
VAR_IN_OUT
    last_output : REAL;
END_VAR
EXPORT;

(* 实现 *)
InitPID.Kp := Kp;
InitPID.Ki := Ki;
InitPID.Kd := Kd;
InitPID.OutputMin := OutputMin;
InitPID.OutputMax := OutputMax;
InitPID.Integral := 0.0;
InitPID.LastError := 0.0;

UpdatePID := PID_Calculate(controller, ProcessValue, DeltaTime);

LowPassFilter := alpha * input + (1.0 - alpha) * last_output;
last_output := LowPassFilter;

END_LIBRARY
```

### 库的使用示例

#### 完整的温度控制系统
```
PROGRAM AdvancedTemperatureControl
(* 导入所需库 *)
IMPORT MathLib;
IMPORT TimeLib;
IMPORT IOLib;
IMPORT ControlLib;
IMPORT StringLib AS Str;

VAR
    (* 控制器变量 *)
    pid_controller : ControlLib.PID_Controller;
    current_temp : REAL;
    target_temp : REAL := 25.0;
    heater_output : REAL;
    
    (* 时间变量 *)
    last_time : TIME;
    current_time : TIME;
    delta_time : REAL;
    
    (* 滤波变量 *)
    filtered_temp : REAL := 0.0;
    
    (* 状态和日志 *)
    system_status : STRING;
    log_message : STRING;
END_VAR

(* 初始化PID控制器 *)
pid_controller := ControlLib.InitPID(
    Kp := 2.0,
    Ki := 0.1,
    Kd := 0.5,
    OutputMin := 0.0,
    OutputMax := 100.0
);

pid_controller.SetPoint := target_temp;
last_time := TimeLib.Now();

(* 主控制循环 *)
WHILE TRUE DO
    (* 获取当前时间和计算时间差 *)
    current_time := TimeLib.Now();
    delta_time := TimeLib.DiffInSeconds(current_time, last_time);
    
    (* 读取温度传感器 *)
    current_temp := IOLib.ReadAnalogInput(0) * 100.0;  (* 假设传感器输出0-1V对应0-100°C *)
    
    (* 温度滤波 *)
    filtered_temp := ControlLib.LowPassFilter(
        input := current_temp,
        alpha := 0.1,
        last_output := filtered_temp
    );
    
    (* PID控制计算 *)
    heater_output := ControlLib.UpdatePID(
        controller := pid_controller,
        ProcessValue := filtered_temp,
        DeltaTime := delta_time
    );
    
    (* 输出控制信号 *)
    IOLib.WriteAnalogOutput(0, heater_output / 100.0);  (* 输出0-1V对应0-100%功率 *)
    
    (* 状态监控和日志 *)
    IF MathLib.Abs(filtered_temp - target_temp) < 0.5 THEN
        system_status := "Temperature Stable";
    ELSIF filtered_temp < target_temp THEN
        system_status := "Heating";
    ELSE
        system_status := "Cooling";
    END_IF
    
    (* 生成日志消息 *)
    log_message := Str.Concat("Time: ", TimeLib.Format(current_time, "HH:mm:ss"));
    log_message := Str.Concat(log_message, " Temp: ");
    log_message := Str.Concat(log_message, REAL_TO_STRING(filtered_temp, 2));
    log_message := Str.Concat(log_message, " Output: ");
    log_message := Str.Concat(log_message, REAL_TO_STRING(heater_output, 1));
    log_message := Str.Concat(log_message, "% Status: ");
    log_message := Str.Concat(log_message, system_status);
    
    (* 每秒输出一次日志 *)
    IF delta_time >= 1.0 THEN
        (* 输出日志到控制台或文件 *)
        IOLib.WriteLine(log_message);
        last_time := current_time;
    END_IF
    
    (* 控制循环延时 *)
    TimeLib.Sleep(100);  (* 100ms循环周期 *)
END_WHILE

END_PROGRAM
```

### 文件结构
```
/home/jiang/src/lexer/
├── st_lexer.lex         # 词法规则定义
├── st_parser.y          # 语法规则定义
├── ast.h/ast.c          # 抽象语法树
├── symbol_table.h/c     # 符号表管理
├── function.h/c         # 函数定义和调用处理
├── scope.h/c            # 作用域管理
├── library_manager.h/c  # 库管理器
├── import_resolver.h/c  # 导入解析器
├── namespace.h/c        # 命名空间管理
├── vm.h/vm.c           # 虚拟机实现
├── main.c              # 主程序
├── Makefile            # 构建脚本
├── library_config.json # 库配置文件
├── libs/               # 标准库目录
│   ├── standard/       # 标准库
│   │   ├── MathLib.st
│   │   ├── StringLib.st
│   │   ├── TimeLib.st
│   │   └── IOLib.st
│   └── custom/         # 自定义库
│       └── ControlLib.st
└── README.md           # 说明文档
```

### 虚拟机指令集
- **数据操作**：LOAD_INT, LOAD_REAL, LOAD_VAR, STORE_VAR
- **算术运算**：ADD, SUB, MUL, DIV, NEG
- **比较运算**：EQ, NE, LT, LE, GT, GE
- **逻辑运算**：AND, OR, NOT
- **控制流**：JMP, JZ, JNZ
- **函数调用**：CALL, RET, PUSH_PARAM, POP_PARAM
- **栈操作**：PUSH_FRAME, POP_FRAME, ALLOC_LOCAL
- **系统**：HALT, NOP
- **模块操作**：LOAD_LIB, RESOLVE_SYMBOL, CALL_EXTERNAL
- **命名空间**：ENTER_NS, EXIT_NS, RESOLVE_NS

### 库加载和链接机制

#### 编译时库处理
1. **导入解析**：解析IMPORT语句，确定需要加载的库
2. **依赖检查**：验证库版本兼容性和依赖关系
3. **符号解析**：将库中的符号加入当前命名空间
4. **类型检查**：验证库函数调用的参数类型匹配
5. **代码生成**：生成调用外部库函数的字节码

#### 运行时库加载
1. **动态加载**：运行时按需加载库文件
2. **符号绑定**：将函数调用绑定到实际的库函数地址
3. **内存管理**：管理库的内存分配和释放
4. **错误处理**：处理库加载失败和运行时错误

#### 调用栈管理
```
调用栈帧结构：
+------------------+
| 返回地址         |
+------------------+
| 上一帧指针       |
+------------------+
| 局部变量1        |
+------------------+
| 局部变量2        |
+------------------+
| ...              |
+------------------+
| 参数N            |
+------------------+
| 参数2            |
+------------------+
| 参数1            |
+------------------+
```

#### 参数传递协议
1. **函数调用前**：将参数按顺序压入操作数栈
2. **CALL指令**：创建新栈帧，设置返回地址
3. **函数执行**：在新栈帧中执行函数体
4. **RET指令**：恢复上一栈帧，返回结果

## 扩展功能

未来可扩展的功能：
- 标准库函数（数学、字符串、时间等）
- 数组和结构体数据类型
- 指针和引用类型
- 异常处理机制
- 模块化和命名空间
- 调试和单步执行功能
- 优化编译器后端

## 错误处理

编译器提供详细的错误信息：
- 词法错误：未识别的字符或标记
- 语法错误：不符合语法规则的构造
- 语义错误：类型不匹配、未定义变量、函数签名错误等
- 运行时错误：栈溢出、除零、函数调用错误等

## 许可证

MIT License - 详见LICENSE文件

## 贡献

欢迎提交Issue和Pull Request来改进这个项目。