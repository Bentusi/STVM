#!/bin/bash
# 测试Force文件加载功能

echo "=== Force文件加载功能测试 ==="
echo ""

# 创建测试Force配置文件
cat > /tmp/test_forces.force << 'EOF'
# STVM Force File
# Test configuration
force_enabled=1
force_count=3

force_var=counter
type=int
value=100
persistent=0

force_var=temperature
type=real
value=25.5
persistent=1

force_var=alarm
type=bool
value=1
persistent=0

EOF

echo ">>> 创建的测试Force配置文件:"
cat /tmp/test_forces.force
echo ""

# 创建调试命令序列
cat > /tmp/force_load_test_cmds.txt << 'EOF'
help
info globals
force_load /tmp/test_forces.force
force_status
info globals
n
info globals
force_save /tmp/saved_forces.force
unforce all
force_status
force_load /tmp/saved_forces.force
force_status
q
EOF

echo ">>> 测试命令序列:"
cat /tmp/force_load_test_cmds.txt
echo ""

echo ">>> 运行调试器测试..."
./build/bin/stvm -d examples/debug_force_simple.stbc < /tmp/force_load_test_cmds.txt

echo ""
echo ">>> 验证保存的文件:"
if [ -f /tmp/saved_forces.force ]; then
    echo "保存的Force配置:"
    cat /tmp/saved_forces.force
else
    echo "未找到保存的文件"
fi

echo ""
echo "=== 测试完成 ==="
