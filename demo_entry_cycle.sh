#!/bin/bash

# STVM 入口函数和执行周期功能演示脚本

echo "=== STVM 入口函数和执行周期功能演示 ==="
echo

# 显示可用函数
echo "1. 查看functions.stbc中的可用函数："
./build/bin/stvm examples/functions.stbc -e nonexistent 2>&1 | grep -A 10 "可用函数列表"
echo

# 测试默认main函数（不存在）
echo "2. 测试默认main函数（期望失败）："
./build/bin/stvm examples/functions.stbc 2>&1 | grep "找不到入口函数"
echo

# 测试指定存在的函数（单次执行）
echo "3. 测试单次执行Add函数："
./build/bin/stvm examples/functions.stbc -e Add -C 0 -V
echo

# 测试不区分大小写
echo "4. 测试不区分大小写的函数名："
./build/bin/stvm examples/functions.stbc -e add -C 0 -V
echo

# 测试短时间周期性执行
echo "5. 测试周期性执行（每500毫秒执行一次，总共3秒）："
timeout 3s ./build/bin/stvm examples/functions.stbc -e Factorial -C 500 -V
echo
echo "（已自动停止）"
echo

# 测试错误处理
echo "6. 测试错误处理："
echo "   a) 无效的函数名："
./build/bin/stvm examples/functions.stbc -e invalid_func 2>&1 | head -3
echo

echo "   b) 无效的执行周期："
./build/bin/stvm examples/functions.stbc -C -5 2>&1 | head -1
echo

echo "   c) 过大的执行周期："
./build/bin/stvm examples/functions.stbc -C 4000000 2>&1 | head -1
echo

# 显示帮助信息
echo "7. 新增的命令行选项帮助："
./build/bin/stvm --help | grep -A 5 "运行模式专用选项"
echo

echo "=== 演示完成 ==="
echo "新功能总结："
echo "- 支持 -e/--entry 指定入口函数（默认main，不区分大小写）"
echo "- 支持 -C/--cycle 指定执行周期（默认1000毫秒，0表示单次执行）"
echo "- 在运行和编译运行模式下都可使用"
echo "- 包含完整的错误处理和边界检查"