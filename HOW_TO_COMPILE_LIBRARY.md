# 如何编译和使用库文件

## 概述

在 STVM 中,**库文件就是普通的字节码文件**,只是它主要包含函数定义而没有主执行逻辑。任何编译后的 `.stbc` 文件都可以作为库被其他程序导入。

## 编译库文件

### 方法 1: 使用普通编译命令

```bash
# 编译库文件 (与编译普通程序相同)
stvm -c mathlib.st -o mathlib.stbc

# 或者让编译器自动生成输出文件名
stvm -c mathlib.st  # 生成 mathlib.stbc
```

**说明**: 不需要特殊的 `--compile-library` 选项,因为库文件和普通程序使用相同的字节码格式。

### 方法 2: 直接运行生成字节码

```bash
# 这也会生成字节码文件 (如果不存在或源文件较新)
stvm mathlib.st
```

## 库文件的结构

### 标准库文件格式 (mathlib.st)

```st
(* 库文件也是一个标准的 PROGRAM *)
PROGRAM MathLib

(* 定义函数 - 这些函数可以被导入 *)
FUNCTION Power : INT
VAR_INPUT
    base : INT;
    exponent : INT;
END_VAR
VAR
    result : INT;
    i : INT;
END_VAR
    result := 1;
    FOR i := 1 TO exponent DO
        result := result * base;
    END_FOR;
    Power := result;
END_FUNCTION

(* 可以定义多个函数 *)
FUNCTION Factorial : INT
VAR_INPUT
    n : INT;
END_VAR
VAR
    result : INT;
    i : INT;
END_VAR
    result := 1;
    FOR i := 1 TO n DO
        result := result * i;
    END_FOR;
    Factorial := result;
END_FUNCTION

(* 库可以没有主执行代码,或者包含简单的测试代码 *)

END_PROGRAM
```

## 使用库文件

### 导入并使用库函数

创建主程序 `main.st`:

```st
PROGRAM Main

(* 导入特定函数 *)
IMPORT Power FROM 'mathlib.stbc';
IMPORT Factorial FROM 'mathlib.stbc';

VAR
    result : INT;
END_VAR

(* 使用导入的函数 *)
result := Power(2, 10);
PRINT('2^10 = %d\n', result);

result := Factorial(5);
PRINT('5! = %d\n', result);

END_PROGRAM
```

### 编译和运行

```bash
# 方法 1: 动态链接 (默认)
stvm main.st
# 输出:
# 2^10 = 1024
# 5! = 120

# 方法 2: 编译后运行
stvm -c main.st -o main.stbc
stvm -r main.stbc

# 方法 3: 静态链接 (生成独立可执行文件)
stvm -c main.st -o main_static.stbc --static
stvm -r main_static.stbc  # 不需要 mathlib.stbc
```

## 完整工作流示例

### 1. 创建库文件

创建 `mylib.st`:

```st
PROGRAM MyLib

FUNCTION Add : INT
VAR_INPUT
    a : INT;
    b : INT;
END_VAR
    Add := a + b;
END_FUNCTION

FUNCTION Multiply : INT
VAR_INPUT
    a : INT;
    b : INT;
END_VAR
    Multiply := a * b;
END_FUNCTION

END_PROGRAM
```

### 2. 编译库

```bash
stvm -c mylib.st
# 生成: mylib.stbc
```

### 3. 创建主程序

创建 `app.st`:

```st
PROGRAM App

IMPORT Add, Multiply FROM 'mylib.stbc';

VAR
    x : INT;
    y : INT;
END_VAR

x := Add(10, 20);
PRINT('10 + 20 = %d\n', x);

y := Multiply(5, 6);
PRINT('5 * 6 = %d\n', y);

END_PROGRAM
```

### 4. 编译和运行主程序

```bash
# 动态链接方式
stvm app.st

# 或静态链接方式 (生成独立可执行文件)
stvm -c app.st -o app.stbc --static
```

## 库搜索路径

### 默认搜索路径

STVM 默认在以下位置查找库文件:
1. 当前目录 (`.`)
2. `examples` 目录

### 添加自定义搜索路径

```bash
# 使用 -L 选项添加库搜索路径
stvm -c main.st -L /path/to/libs -L /another/path

# 示例:
stvm -c main.st -L ~/mylibs -L /usr/local/stvm/lib
```

## 验证库文件

### 查看库内容

```bash
# 反汇编查看库中的函数
stvm -c mylib.st --dump-bytecode

# 输出会显示:
# - 函数列表
# - 每个函数的参数和返回类型
# - 字节码指令
```

### 示例输出

```
=== Bytecode Module ===
Entry point: 0
Global variables: 0

--- Functions (2) ---
  [0] Add(@0): params=2, locals=2, return=int
  [1] Multiply(@10): params=2, locals=2, return=int

--- Instructions (20) ---
  ...
```

## 最佳实践

### 1. 库文件命名

- 使用描述性名称: `mathlib.st`, `stringlib.st`, `iolib.st`
- 编译后自动生成 `.stbc` 扩展名

### 2. 库文件组织

```
project/
  ├── src/
  │   ├── main.st          # 主程序
  │   └── utils.st         # 工具函数
  ├── lib/
  │   ├── mathlib.st       # 数学库源码
  │   ├── mathlib.stbc     # 数学库字节码
  │   ├── stringlib.st     # 字符串库源码
  │   └── stringlib.stbc   # 字符串库字节码
  └── build/
      └── app.stbc         # 最终程序
```

### 3. 编译脚本

创建 `build.sh`:

```bash
#!/bin/bash

# 编译所有库
echo "编译库文件..."
stvm -c lib/mathlib.st -o lib/mathlib.stbc
stvm -c lib/stringlib.st -o lib/stringlib.stbc

# 编译主程序 (动态链接)
echo "编译主程序..."
stvm -c src/main.st -o build/app.stbc -L lib

# 或者编译为静态链接版本
echo "编译静态链接版本..."
stvm -c src/main.st -o build/app_static.stbc -L lib --static

echo "完成!"
```

## 常见问题

### Q: 库文件和普通程序有什么区别?

**A**: 在字节码层面没有区别。库文件通常:
- 只包含函数定义
- 没有或很少的主执行代码
- 被设计为可重用的模块

### Q: 是否需要特殊的编译选项?

**A**: 不需要。使用普通的 `-c` 选项编译即可:
```bash
stvm -c library.st
```

### Q: 如何知道库中有哪些函数?

**A**: 使用 `--dump-bytecode` 选项:
```bash
stvm -c library.st --dump-bytecode | grep "^\s*\["
```

### Q: 库文件可以导入其他库吗?

**A**: 可以。库可以相互导入:

```st
(* advanced_math.st *)
PROGRAM AdvancedMath

IMPORT Power FROM 'mathlib.stbc';

FUNCTION SquarePower : INT
VAR_INPUT
    base : INT;
    exp : INT;
END_VAR
    SquarePower := Power(Power(base, 2), exp);
END_FUNCTION

END_PROGRAM
```

## 总结

| 操作 | 命令 |
|------|------|
| 编译库 | `stvm -c library.st` |
| 编译主程序(动态) | `stvm -c main.st` |
| 编译主程序(静态) | `stvm -c main.st --static` |
| 运行程序 | `stvm main.st` 或 `stvm -r main.stbc` |
| 查看库内容 | `stvm -c library.st --dump-bytecode` |
| 添加库路径 | `stvm -c main.st -L /path/to/libs` |

**核心要点**: 
- ✅ 库文件 = 普通字节码文件
- ✅ 使用相同的编译命令
- ✅ 无需特殊选项
- ✅ 通过 IMPORT 语句使用
