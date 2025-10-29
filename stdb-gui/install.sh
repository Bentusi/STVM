#!/bin/bash
#
# STDB-GUI 安装脚本
# ==================
#
# 安装所有必需的 Python 依赖

set -e  # Exit on error

echo "============================================"
echo "STDB-GUI Installation Script"
echo "============================================"
echo ""

# 检查 Python 版本
echo "[1/4] Checking Python version..."
python3 --version || { echo "Error: python3 not found"; exit 1; }

# 检查 pip
echo ""
echo "[2/4] Checking pip..."
python3 -m pipx --version || { echo "Error: pip not found"; exit 1; }

# 安装依赖
echo ""
echo "[3/4] Installing dependencies..."
echo "This may take a few minutes and download ~200MB..."
echo ""

python3 -m pipx install --user -r requirements.txt

# 验证安装
echo ""
echo "[4/4] Verifying installation..."
python3 -c "
import sys
try:
    import PySide6
    print('✓ PySide6:', PySide6.__version__)
except ImportError as e:
    print('✗ PySide6: Not installed')
    sys.exit(1)

try:
    import PyQt6.Qsci
    print('✓ QScintilla: OK')
except ImportError:
    print('✗ QScintilla: Not installed')
    sys.exit(1)

try:
    import pyqtgraph
    print('✓ PyQtGraph:', pyqtgraph.__version__)
except ImportError:
    print('✗ PyQtGraph: Not installed')
    sys.exit(1)

try:
    import qtawesome
    print('✓ QtAwesome: OK')
except ImportError:
    print('✗ QtAwesome: Not installed')
    sys.exit(1)

try:
    import qdarkstyle
    print('✓ QDarkStyle: OK')
except ImportError:
    print('✗ QDarkStyle: Not installed')
    sys.exit(1)

try:
    import cffi
    print('✓ cffi:', cffi.__version__)
except ImportError:
    print('✗ cffi: Not installed')
    sys.exit(1)

try:
    import numpy
    print('✓ NumPy:', numpy.__version__)
except ImportError:
    print('✗ NumPy: Not installed')
    sys.exit(1)

print('')
print('All dependencies installed successfully!')
"

echo ""
echo "============================================"
echo "Installation complete!"
echo "============================================"
echo ""
echo "Run the application:"
echo "  cd /home/wei/stvm/stdb-gui"
echo "  python3 main.py"
echo ""
echo "Or use the run script:"
echo "  ./run.sh"
echo ""
