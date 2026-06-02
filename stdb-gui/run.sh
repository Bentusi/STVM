#!/bin/bash
#
# STDB-GUI 运行脚本
# =================
#
# 启动 STVM 图形化调试器

set -e

cd "$(dirname "$0")"

echo "=========================================="
echo "STDB-GUI - STVM Graphical Debugger"
echo "=========================================="
echo ""

# 检查依赖
if ! python3 -c "import PySide6" 2>/dev/null; then
    echo "Error: Dependencies not installed"
    echo "Please run: ./install.sh"
    exit 1
fi

# 检查 STVM 库
STVM_LIB="../build/lib/libstvm.so"
if [ -f "$STVM_LIB" ]; then
    echo "✓ STVM library found: $STVM_LIB"
    echo "  Running in REAL mode"
else
    echo "✗ STVM library not found: $STVM_LIB"
    echo "  Running in STUB mode (UI testing only)"
    echo ""
    echo "  To use real STVM, compile the library:"
    echo "    cd /home/wei/stvm && make"
fi

echo ""
echo "Starting STDB-GUI..."
echo ""

# 运行应用
python3 main.py "$@"
