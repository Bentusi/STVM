#!/bin/bash

# STVM 热更新功能测试脚本
# 快速验证热更新功能是否正常工作

set -e  # 遇到错误立即退出

echo "╔════════════════════════════════════════════════════════╗"
echo "║          STVM 热更新功能验证脚本                       ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""

# 检查构建目录
if [ ! -f "build/bin/stvm" ]; then
    echo "❌ STVM 可执行文件不存在，开始构建..."
    make clean && make
    echo "✓ 构建完成"
fi

# 检查测试程序
if [ ! -f "build/bin/test_hotreload" ]; then
    echo "❌ 热更新测试程序不存在，开始构建..."
    make test_hotreload
    echo "✓ 测试程序构建完成"
fi

echo ""
echo "▶ Step 1: 编译测试样例 V1..."
./build/bin/stvm -c examples/hotreload_v1.st -o examples/hotreload_v1.stbc
echo "✓ V1 编译完成"

echo ""
echo "▶ Step 2: 编译测试样例 V2..."
./build/bin/stvm -c examples/hotreload_v2.st -o examples/hotreload_v2.stbc
echo "✓ V2 编译完成"

echo ""
echo "▶ Step 3: 运行热更新测试..."
echo ""
./build/bin/test_hotreload examples/hotreload_v1.stbc examples/hotreload_v2.stbc

echo ""
echo "╔════════════════════════════════════════════════════════╗"
echo "║                验证完成！                              ║"
echo "╠════════════════════════════════════════════════════════╣"
echo "║ 如果看到上方测试成功输出，说明热更新功能正常！        ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""
echo "📖 更多信息："
echo "   - 快速指南: HOT_RELOAD_QUICKSTART.md"
echo "   - 实现报告: HOT_RELOAD_IMPLEMENTATION.md"
echo "   - 完成总结: HOT_RELOAD_COMPLETION_SUMMARY.md"
echo ""
