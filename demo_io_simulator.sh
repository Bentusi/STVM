#!/bin/bash

# STVM IO 模拟器演示脚本

echo "=== STVM IO 模拟器演示 ==="
echo

echo "✅ IO 模拟器已成功集成到 STVM 主程序!"
echo

echo "1. 查看帮助信息中的 IO 选项..."
./build/bin/stvm --help | grep -A 5 "I/O 选项"
echo

echo "2. 运行简单程序（带 IO 模拟器）..."
echo "   命令: ./stvm examples/simple.st -I -V"
echo
./build/bin/stvm examples/simple.st -I -V 2>&1 | head -20
echo

echo "3. 运行完整集成测试..."
echo "   命令: ./test_io_integration_final.sh"
echo
./test_io_integration_final.sh 2>&1 | grep -E "^(✓|✗|⚠️|测试|总结|使用方法)"
echo

echo "=== 功能说明 ==="
echo
echo "✅ 已实现:"
echo "   - IO 管理器核心 (src/core/iomgr.c)"
echo "   - 模拟器适配器 (src/core/io_adapter_sim.c)"
echo "   - 命令行选项 (-I, --io-simulator)"
echo "   - 主程序集成 (main.c)"
echo "   - 自动刷新线程"
echo "   - IEC 61131-3 地址解析"
echo "   - GPIO/ADC/DAC/PWM/编码器模拟"
echo
echo "⚠️  待实现:"
echo "   - IO 配置文件 JSON 解析"
echo "   - VM 中的 IO 读写指令"
echo "   - VAR_EXTERNAL 变量的 IO 映射"
echo
echo "📖 详细说明请查看: IO模拟器使用指南.md"
echo

echo "=== 使用示例 ==="
echo
echo "基本用法:"
echo "  ./stvm program.st -I              # 使用 IO 模拟器运行"
echo "  ./stvm program.st -I -V           # 详细输出"
echo "  ./stvm program.st -I -C 100       # 周期性执行（每 100ms）"
echo
echo "编译和运行:"
echo "  ./stvm -c program.st -o prog.stbc # 编译"
echo "  ./stvm -r prog.stbc -I            # 运行（带 IO 模拟器）"
echo
echo "指定入口函数:"
echo "  ./stvm program.st -I -e MAIN      # 使用 MAIN 作为入口"
echo

echo "=== 演示完成 ==="
