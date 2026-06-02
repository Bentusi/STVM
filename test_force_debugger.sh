#!/bin/bash
# 自动化测试Force调试器命令

echo "=== Force调试器命令自动测试 ==="
echo ""

# 创建测试命令文件
cat > /tmp/force_test_commands.txt << 'EOF'
help
force counter int 100
force temperature real 50.5 persistent
force alarm bool true
force_status
n
info globals
force_enable off
force_status
force_enable on
unforce counter
force_status
unforce all
force_status
q
EOF

echo "测试命令序列:"
cat /tmp/force_test_commands.txt
echo ""
echo ">>> 运行调试器..."
echo ""

# 运行调试器并输入命令
./build/bin/stvm -d examples/debug_force_test.stbc < /tmp/force_test_commands.txt

echo ""
echo "=== 测试完成 ==="
