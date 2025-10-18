# STVM 库编译快速参考

## 一分钟快速上手

### 编译库文件
```bash
# 基本用法 - 编译库
stvm -c mathlib.st

# 指定输出文件
stvm -c mathlib.st -o mathlib.stbc
```

### 使用库文件
```st
(* 在你的程序中 *)
IMPORT Power, Factorial FROM 'mathlib.stbc';

result := Power(2, 10);  (* 使用导入的函数 *)
```

### 编译使用库的程序
```bash
# 动态链接 (默认,需要库文件)
stvm main.st

# 静态链接 (独立可执行,无需库文件)
stvm -c main.st --static
```

## 常用命令

| 任务 | 命令 |
|------|------|
| 编译库 | `stvm -c library.st` |
| 编译程序(动态) | `stvm -c main.st` |
| 编译程序(静态) | `stvm -c main.st --static` |
| 运行程序 | `stvm main.st` |
| 查看库函数 | `stvm -c lib.st --dump-bytecode \| grep Function` |
| 添加库路径 | `stvm -c main.st -L /path/to/libs` |

## 关键点

1. **库文件 = 普通字节码**  
   不需要特殊编译选项

2. **动态 vs 静态**
   - 动态: 文件小,需要库在运行时可用
   - 静态(`--static`): 文件大,完全独立

3. **导入语法**
   ```st
   IMPORT Function1, Function2 FROM 'library.stbc';
   ```

4. **库搜索路径**
   - 默认: `.` 和 `examples/`
   - 自定义: `-L /custom/path`

---
**详细文档**: 参见 `HOW_TO_COMPILE_LIBRARY.md`
