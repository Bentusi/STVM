#!/bin/bash
# 交互式Force调试器测试

cat > /tmp/force_cmds.txt << 'EOF'
help
info globals
force counter int 100
force temperature real 99.9 persistent
force alarm bool true
force_status
info globals
n
info globals
unforce counter
force_status
info globals
n
info globals
force_enable off
info globals
n
force_enable on
unforce all
force_status
q
EOF

echo "=== 交互式Force调试器测试 ==="
echo ""
echo "使用程序: examples/debug_force_simple.stbc"
echo ""

./build/bin/stvm -d examples/debug_force_simple.stbc < /tmp/force_cmds.txt
