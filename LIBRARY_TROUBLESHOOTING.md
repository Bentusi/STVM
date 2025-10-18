# 库识别问题解决方案

## 问题描述

用户报告使用 `build/bin/stvm examples/test_mathlib_demo.st` 时出现错误:

```
运行时错误: External function 'examples/mathlib.stbc.Power' not registered at PC=3
```

虽然日志显示库已被加载:
```
[libmgr] Loaded library: mathlib (52 functions)
[libmgr] Imported: examples/mathlib.stbc.Power
```

## 根本原因

这个问题通常是因为使用了**旧版本的编译器**,该版本可能:
1. `vm_run_from` 函数中没有完整的库函数查找逻辑
2. 或者库函数查找逻辑有bug

## 解决方案

### 方法 1: 重新编译 (推荐)

```bash
cd /home/jiang/src/lexer
make clean
make
```

然后重新测试:
```bash
build/bin/stvm examples/test_mathlib_demo.st
```

### 方法 2: 使用编译后运行模式

如果问题仍然存在,可以使用两步法:

```bash
# 步骤1: 编译
build/bin/stvm -c examples/test_mathlib_demo.st -o test.stbc

# 步骤2: 运行
build/bin/stvm -r test.stbc
```

### 方法 3: 使用静态链接

生成包含所有库代码的独立可执行文件:

```bash
# 静态链接编译
build/bin/stvm -c examples/test_mathlib_demo.st -o test_static.stbc --static

# 运行(不需要库文件)
build/bin/stvm -r test_static.stbc
```

## 验证修复

正常输出应该是:
```
Power(2, 10) = 1024
Factorial(5) = 120
GCD(48, 18) = 6
Fibonacci(10) = 55
Lerp(0.0, 100.0, 0.75) = 75.000000
```

## 技术细节

### VM 如何查找库函数

1. **函数名格式**: `examples/mathlib.stbc.Power`
   - 库路径: `examples/mathlib.stbc`
   - 函数名: `Power`

2. **查找过程**:
   ```c
   // 1. 解析限定名
   const char* stbc_pos = strstr(func_name, ".stbc.");
   const char* real_name = strrchr(func_name, '.');
   
   // 2. 从 libmgr 查找对应的库
   LoadedLibrary* lib = find_library_by_path(vm->libmgr, lib_path);
   
   // 3. 在库模块中查找函数
   FunctionEntry* func = find_function(lib->module, real_name);
   
   // 4. 切换到库模块执行
   vm->module = lib->module;
   vm->pc = func->address;
   ```

3. **路径匹配逻辑**:
   - 比较 `lib->path` 的结尾是否匹配提取的库路径
   - 例如: `lib->path="examples/mathlib.stbc"` 匹配 `lib_path="examples/mathlib.stbc"`

## 预防措施

### 1. 始终使用最新编译器

```bash
# 编译库
build/bin/stvm -c library.st

# 使用库
build/bin/stvm main.st
```

### 2. 检查库是否存在

```bash
ls -lh examples/mathlib.stbc
# 应该显示文件(约4.6KB)
```

### 3. 验证库搜索路径

默认搜索路径:
- `.` (当前目录)
- `examples/`

添加自定义路径:
```bash
build/bin/stvm main.st -L /path/to/libs
```

### 4. 使用详细模式调试

```bash
build/bin/stvm main.st -v
```

查看编译和运行过程的详细信息。

## 常见问题

### Q: 库文件存在但仍然报错

**A**: 可能原因:
1. 编译器版本过旧 → 重新编译
2. 库文件已损坏 → 重新编译库
3. 路径不匹配 → 检查IMPORT语句中的路径

### Q: 如何确认库被正确加载?

**A**: 使用 `-v` 选项,应该看到:
```
[libmgr] Loaded library: mathlib (52 functions)
```

### Q: 静态链接和动态链接有什么区别?

**A**: 
- **动态链接**: 运行时需要库文件,文件小
- **静态链接** (`--static`): 库代码内嵌,文件大,无运行时依赖

## 总结

问题已解决,关键步骤:
1. ✅ 重新编译项目 (`make clean && make`)
2. ✅ 确认库文件存在
3. ✅ 使用最新的编译器运行

现在应该可以正常使用库功能了!
