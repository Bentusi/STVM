# IEC61131 结构化文本编译器

一个完整的IEC61131结构化文本语言编译器和虚拟机实现，使用Flex和Bison构建，支持平台无关的字节码执行。

## 特性

- **完整的词法分析**：支持IEC61131标准的关键字、标识符、字面量和运算符
- **语法分析**：使用Bison实现的语法分析器，生成抽象语法树(AST)
- **虚拟机执行**：平台无关的字节码虚拟机，支持控制流和表达式计算
- **数据类型支持**：BOOL、INT、REAL、STRING基本数据类型
- **控制结构**：IF-THEN-ELSE、FOR循环、WHILE循环、CASE分支
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
./st_compiler program.st
```

### 2. 交互模式
```bash
./st_compiler -i
```

### 3. 帮助信息
```bash
./st_compiler -h
```

## 语法支持

### 程序结构
```
PROGRAM MyProgram
VAR
    variable_declarations
END_VAR

    statements

END_PROGRAM
```

### 变量声明
```
VAR
    counter : INT;
    flag : BOOL;
    temperature : REAL;
    message : STRING;
END_VAR
```

### 赋值语句
```
counter := 10;
flag := TRUE;
temperature := 25.5;
```

### 控制结构

#### IF语句
```
IF temperature > 30 THEN
    flag := TRUE;
ELSE
    flag := FALSE;
END_IF
```

#### FOR循环
```
FOR i := 1 TO 10 DO
    sum := sum + i;
END_FOR
```

#### WHILE循环
```
WHILE counter > 0 DO
    counter := counter - 1;
END_WHILE
```

### 表达式
- 算术运算：`+`, `-`, `*`, `/`, `MOD`
- 比较运算：`=`, `<>`, `<`, `<=`, `>`, `>=`
- 逻辑运算：`AND`, `OR`, `NOT`

## 示例程序

### 计算1到10的和
```
PROGRAM SumCalculation
VAR
    i : INT;
    sum : INT;
END_VAR

sum := 0;
FOR i := 1 TO 10 DO
    sum := sum + i;
END_FOR

END_PROGRAM
```

### 阶乘计算
```
PROGRAM Factorial
VAR
    n : INT;
    factorial : INT;
    i : INT;
END_VAR

n := 5;
factorial := 1;
FOR i := 1 TO n DO
    factorial := factorial * i;
END_FOR

END_PROGRAM
```

### 条件判断
```
PROGRAM TemperatureControl
VAR
    temperature : REAL;
    heater_on : BOOL;
    cooler_on : BOOL;
    case : INT;
END_VAR

temperature := 22.5;
case := 1

IF temperature < 20.0 THEN
    heater_on := TRUE;
    cooler_on := FALSE;
ELSIF temperature > 25.0 THEN
    heater_on := FALSE;
    cooler_on := TRUE;
ELSE
    heater_on := FALSE;
    cooler_on := FALSE;
END_IF

CASE case OF
    1 : case := 2;
    2 : case := 1;
END_CASE

END_PROGRAM
```

## 架构说明

### 编译流程
1. **词法分析** (st_lexer.lex) - 将源代码转换为标记流
2. **语法分析** (st_parser.y) - 解析标记并构建AST
3. **代码生成** (vm.c) - 将AST编译为虚拟机字节码
4. **执行** (vm.c) - 在虚拟机中执行字节码

### 文件结构
```
/home/jiang/src/lexer/
├── st_lexer.lex      # 词法规则定义
├── st_parser.y       # 语法规则定义
├── ast.h/ast.c       # 抽象语法树
├── vm.h/vm.c         # 虚拟机实现
├── main.c            # 主程序
├── Makefile          # 构建脚本
└── README.md         # 说明文档
```

### 虚拟机指令集
- **数据操作**：LOAD_INT, LOAD_REAL, LOAD_VAR, STORE_VAR
- **算术运算**：ADD, SUB, MUL, DIV, NEG
- **比较运算**：EQ, NE, LT, LE, GT, GE
- **逻辑运算**：AND, OR, NOT
- **控制流**：JMP, JZ, JNZ
- **系统**：HALT, NOP

## 扩展功能

未来可扩展的功能：
- 函数和过程调用支持
- 数组和结构体数据类型
- 更多IEC61131标准库函数
- 调试和单步执行功能
- 优化编译器后端

## 错误处理

编译器提供详细的错误信息：
- 词法错误：未识别的字符或标记
- 语法错误：不符合语法规则的构造
- 语义错误：类型不匹配、未定义变量等
- 运行时错误：栈溢出、除零等

## 许可证

MIT License - 详见LICENSE文件

## 贡献

欢迎提交Issue和Pull Request来改进这个项目。