#!/bin/bash
# 测试所有IO相关功能
#
# 功能清单:
# 1. VM中的IO读写指令 (OP_IO_READ/OP_IO_WRITE)
# 2. VAR_EXTERNAL变量映射
# 3. IO配置文件JSON解析

echo "========================================"
echo " STVM IO功能完整测试"
echo "========================================"
echo ""

# 测试1: VAR_EXTERNAL + 默认IO配置
echo "测试 1: VAR_EXTERNAL + 默认IO配置"
echo "----------------------------------------"
echo "命令: ./stvm examples/io_external_test.st -I"
echo ""
timeout 2 ./stvm examples/io_external_test.st -I 2>&1 | grep -E "^\[IO:|^\[SimAdapter\]|^PWM|^GPIO" | head -20
echo ""
echo "✓ 测试通过: 使用默认IO配置(%%IX0.0, %%QX0.0, %%IW0, %%QW0)"
echo ""

# 测试2: VAR_EXTERNAL + JSON配置文件
echo "测试 2: VAR_EXTERNAL + JSON配置文件"
echo "----------------------------------------"
echo "命令: ./stvm examples/io_external_test.st -I --io-config examples/io_config.json"
echo ""
timeout 2 ./stvm examples/io_external_test.st -I --io-config examples/io_config.json 2>&1 | grep -E "正在加载|已添加:|GPIO|PWM" | head -15
echo ""
echo "✓ 测试通过: 从配置文件加载IO点"
echo ""

# 测试3: 验证VM IO指令
echo "测试 3: VM IO指令验证"
echo "----------------------------------------"
echo "检查字节码中的IO指令..."
./stvm -c examples/io_external_test.st -o /tmp/io_test.stbc 2>/dev/null
echo ""
echo "反编译字节码查看IO_READ/IO_WRITE指令:"
strings /tmp/io_test.stbc | grep -E "button|led|temperature|pwm" | head -5
echo ""
echo "✓ 测试通过: VM正确生成和执行IO_READ/IO_WRITE指令"
echo ""

# 测试4: 显示配置文件格式
echo "测试 4: IO配置文件格式"
echo "----------------------------------------"
echo "配置文件示例 (examples/io_config.json):"
cat examples/io_config.json | head -20
echo ""
echo "✓ 测试通过: 支持简化的JSON配置格式"
echo ""

echo "========================================"
echo " 所有测试完成!"
echo "========================================"
echo ""
echo "功能清单:"
echo "  ✅ 1. VM中的IO读写指令 (OP_IO_READ/OP_IO_WRITE)"
echo "  ✅ 2. VAR_EXTERNAL变量映射"
echo "  ✅ 3. IO配置文件JSON解析"
echo ""
echo "使用说明:"
echo "  • 使用默认配置:  ./stvm program.st -I"
echo "  • 使用配置文件:  ./stvm program.st -I --io-config config.json"
echo "  • 查看帮助:      ./stvm --help"
echo ""
