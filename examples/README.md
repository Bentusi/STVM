# STVM 示例程序

本目录包含一些 ST 语言示例程序，用于演示 STVM 编译器和虚拟机的功能。

## 示例列表

### 1. hello.st - Hello World
最简单的示例，演示：
- 变量声明和初始化
- 基本类型（INT, REAL, STRING）
- 简单赋值和运算

**编译和运行：**
```bash
make stvm
./build/bin/stvm -c examples/hello.st
./build/bin/stvm -r examples/hello.stbc
```

### 2. control_flow.st - 控制流
演示控制流结构：
- IF-THEN-ELSE 条件语句
- WHILE 循环
- FOR 循环

**编译和运行：**
```bash
./build/bin/stvm -c examples/control_flow.st
./build/bin/stvm -r examples/control_flow.stbc
```

### 3. functions.st - 函数定义和调用
演示函数功能：
- 函数定义（FUNCTION）
- 输入参数（VAR_INPUT）
- 函数返回值
- 递归计算（阶乘）

**编译和运行：**
```bash
./build/bin/stvm -c examples/functions.st
./build/bin/stvm -r examples/functions.stbc
```

### 4. arrays.st - 数组操作
演示数组功能：
- 数组声明
- 数组元素访问
- 数组遍历
- 数组算法（求和、查找最大值）

**编译和运行：**
```bash
./build/bin/stvm -c examples/arrays.st
./build/bin/stvm -r examples/arrays.stbc
```

### 5. case.st - CASE 语句
演示 CASE 选择结构：
- 简单 CASE 语句
- 多值匹配
- ELSE 默认分支

**编译和运行：**
```bash
./build/bin/stvm -c examples/case.st
./build/bin/stvm -r examples/case.stbc
```

## 调试示例

使用调试器运行任何示例：

```bash
./build/bin/stvm -r examples/hello.stbc -d
```

调试器命令：
- `b <addr>` - 设置断点
- `s` - 单步执行（进入函数）
- `n` - 单步执行（跳过函数）
- `c` - 继续执行
- `info locals` - 显示局部变量
- `info globals` - 显示全局变量
- `bt` - 显示调用栈
- `help` - 显示所有命令

## 高级选项

### 显示 AST
```bash
./build/bin/stvm -c examples/hello.st --dump-ast
```

### 显示字节码
```bash
./build/bin/stvm -c examples/hello.st --dump-bytecode
```

### 详细输出
```bash
./build/bin/stvm -c examples/hello.st -V
```

### 统计信息
```bash
./build/bin/stvm -c examples/hello.st -s
./build/bin/stvm -r examples/hello.stbc -s
```

## 创建自己的程序

参考这些示例创建您自己的 ST 程序。ST 语言的基本结构：

```st
PROGRAM ProgramName
VAR
    (* 变量声明 *)
    variable_name : TYPE;
END_VAR

BEGIN
    (* 程序代码 *)
END_PROGRAM
```

支持的数据类型：
- `BOOL` - 布尔类型
- `INT` - 整数类型
- `REAL` - 实数类型
- `STRING` - 字符串类型
- `ARRAY[lower..upper] OF TYPE` - 数组类型

控制流语句：
- `IF ... THEN ... ELSIF ... ELSE ... END_IF`
- `WHILE ... DO ... END_WHILE`
- `FOR ... TO ... DO ... END_FOR`
- `CASE ... OF ... END_CASE`
- `REPEAT ... UNTIL ...`
