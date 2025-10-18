#!/bin/bash
# STVM 静态链接功能演示脚本

set -e

echo "=== STVM 静态链接功能演示 ==="
echo

# 清理之前的输出
rm -f /tmp/test_dynamic.stbc /tmp/test_static.stbc

echo "1. 编译动态链接版本..."
build/bin/stvm -c examples/test_mathlib_demo.st -o /tmp/test_dynamic.stbc
echo "   ✓ 生成: /tmp/test_dynamic.stbc"

echo
echo "2. 编译静态链接版本..."
build/bin/stvm -c examples/test_mathlib_demo.st -o /tmp/test_static.stbc --static
echo "   ✓ 生成: /tmp/test_static.stbc"

echo
echo "3. 文件大小对比:"
echo "   库文件 (mathlib.stbc):"
ls -lh examples/mathlib.stbc | awk '{print "      ", $5, $9}'
echo "   动态链接版本:"
ls -lh /tmp/test_dynamic.stbc | awk '{print "      ", $5, $9}'
echo "   静态链接版本:"
ls -lh /tmp/test_static.stbc | awk '{print "      ", $5, $9}'

echo
echo "4. 测试动态链接版本(需要库文件)..."
build/bin/stvm examples/test_mathlib_demo.st

echo
echo "5. 备份并删除库文件..."
cp examples/mathlib.stbc /tmp/mathlib.stbc.backup
rm examples/mathlib.stbc
echo "   ✓ 库文件已删除"

echo
echo "6. 测试动态链接版本(应该失败)..."
if build/bin/stvm -r /tmp/test_dynamic.stbc 2>&1; then
    echo "   ✗ 意外成功(应该因为找不到库而失败)"
else
    echo "   ✓ 如预期失败(无法加载库)"
fi

echo
echo "7. 测试静态链接版本(无需库文件)..."
build/bin/stvm -r /tmp/test_static.stbc
echo "   ✓ 静态链接版本成功运行!"

echo
echo "8. 恢复库文件..."
mv /tmp/mathlib.stbc.backup examples/mathlib.stbc
echo "   ✓ 库文件已恢复"

echo
echo "=== 演示完成 ==="
echo
echo "总结:"
echo "  • 动态链接: 文件小,需要运行时库支持"
echo "  • 静态链接: 文件大,完全独立,无需外部依赖"
echo "  • 使用 --static 选项生成静态链接版本"
