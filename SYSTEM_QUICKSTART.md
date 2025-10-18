# SYSTEM 内置函数 - 快速开始

## 简介

SYSTEM 是 STVM 的内置函数，允许 ST 程序调用操作系统命令。

## 基本用法

```st
PROGRAM Example
VAR
    result: INT;
END_VAR

result := SYSTEM('echo Hello World');
PRINT('Exit code: %d\n', result);

END_PROGRAM
```

## 常用示例

### 1. 文件操作

```st
VAR
    status: INT;
END_VAR

(* 创建目录 *)
status := SYSTEM('mkdir -p output');

(* 写入文件 *)
status := SYSTEM('echo "data" > output/file.txt');

(* 读取文件 *)
status := SYSTEM('cat output/file.txt');

(* 删除文件 *)
status := SYSTEM('rm output/file.txt');
```

### 2. 系统信息

```st
VAR
    r: INT;
END_VAR

r := SYSTEM('date');      (* 显示日期 *)
r := SYSTEM('whoami');    (* 显示用户名 *)
r := SYSTEM('uname -a');  (* 系统信息 *)
r := SYSTEM('pwd');       (* 当前目录 *)
```

### 3. 条件执行

```st
VAR
    check: INT;
END_VAR

check := SYSTEM('test -f config.txt');
IF check = 0 THEN
    PRINT('Config file exists\n');
    check := SYSTEM('cat config.txt');
ELSE
    PRINT('Config file not found\n');
    check := SYSTEM('echo "default" > config.txt');
END_IF;
```

### 4. 管道和重定向

```st
VAR
    ret: INT;
END_VAR

(* 管道 *)
ret := SYSTEM('ls -l | grep .st');

(* 输出重定向 *)
ret := SYSTEM('date > timestamp.txt');

(* 命令组合 *)
ret := SYSTEM('mkdir -p logs && echo "started" > logs/status.txt');
```

## 退出码

| 值 | 含义 |
|----|------|
| 0 | 成功 |
| 非零 | 失败（具体含义取决于命令）|
| -1 | SYSTEM 调用失败 |

## 错误处理

```st
VAR
    exit_code: INT;
END_VAR

exit_code := SYSTEM('some_command');

IF exit_code = 0 THEN
    PRINT('Success!\n');
ELSIF exit_code = 127 THEN
    PRINT('Command not found\n');
ELSE
    PRINT('Error: %d\n', exit_code);
END_IF;
```

## 安全提示

⚠️ **重要**:
- 不要使用不可信的输入构造命令
- 命令以当前用户权限运行
- 注意跨平台命令差异

## 测试程序

```bash
# 运行完整测试
./build/bin/stvm examples/system_test.st

# 运行简单演示
./build/bin/stvm examples/system_demo.st
```

## 更多信息

- 完整文档: [SYSTEM_FUNCTION_COMPLETION.md](SYSTEM_FUNCTION_COMPLETION.md)
- 主文档: [README.md](README.md)
- 示例: `examples/system_*.st`

---

**快速参考**: `result := SYSTEM('command');` 返回退出码，0表示成功。
