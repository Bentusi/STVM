#!/bin/bash
# SYSTEM 函数验证脚本

echo "====================================="
echo "  SYSTEM 函数验证脚本"
echo "====================================="
echo ""

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 计数器
TOTAL=0
PASSED=0
FAILED=0

# 测试函数
run_test() {
    local test_name="$1"
    local test_file="$2"
    
    echo -e "${YELLOW}测试:${NC} $test_name"
    echo "文件: $test_file"
    
    TOTAL=$((TOTAL + 1))
    
    if [ ! -f "$test_file" ]; then
        echo -e "${RED}✗ 失败${NC}: 文件不存在"
        FAILED=$((FAILED + 1))
        echo ""
        return 1
    fi
    
    if ./build/bin/stvm "$test_file" > /tmp/stvm_test_output.txt 2>&1; then
        echo -e "${GREEN}✓ 通过${NC}"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}✗ 失败${NC}"
        echo "错误输出:"
        cat /tmp/stvm_test_output.txt | head -20
        FAILED=$((FAILED + 1))
    fi
    echo ""
}

# 检查 STVM 是否已构建
if [ ! -f "./build/bin/stvm" ]; then
    echo -e "${RED}错误:${NC} STVM 未构建"
    echo "请先运行: make"
    exit 1
fi

echo "STVM 版本信息:"
./build/bin/stvm --version
echo ""

# 运行测试
echo "====================================="
echo "  开始测试"
echo "====================================="
echo ""

run_test "完整测试套件 (system_test.st)" "examples/system_test.st"
run_test "简单演示 (system_demo.st)" "examples/system_demo.st"

# 创建即时测试
echo "====================================="
echo "  即时功能测试"
echo "====================================="
echo ""

# 测试 1: 基本 SYSTEM 调用
cat > /tmp/test_basic_system.st << 'EOF'
PROGRAM TestBasicSystem
VAR
    r: INT;
END_VAR
PRINT('Test: Basic SYSTEM call\n');
r := SYSTEM('echo "SYSTEM works!"');
PRINT('Exit code: %d\n', r);
IF r = 0 THEN
    PRINT('PASS\n');
END_IF;
END_PROGRAM
EOF

echo -e "${YELLOW}即时测试 1:${NC} 基本 SYSTEM 调用"
if ./build/bin/stvm /tmp/test_basic_system.st 2>&1 | grep -q "SYSTEM works!"; then
    echo -e "${GREEN}✓ 通过${NC}"
    PASSED=$((PASSED + 1))
else
    echo -e "${RED}✗ 失败${NC}"
    FAILED=$((FAILED + 1))
fi
TOTAL=$((TOTAL + 1))
echo ""

# 测试 2: 退出码检查
cat > /tmp/test_exit_code.st << 'EOF'
PROGRAM TestExitCode
VAR
    success: INT;
    failure: INT;
END_VAR
PRINT('Test: Exit code handling\n');
success := SYSTEM('true');
failure := SYSTEM('false');
PRINT('Success code: %d\n', success);
PRINT('Failure code: %d\n', failure);
IF success = 0 AND failure <> 0 THEN
    PRINT('PASS\n');
END_IF;
END_PROGRAM
EOF

echo -e "${YELLOW}即时测试 2:${NC} 退出码处理"
if ./build/bin/stvm /tmp/test_exit_code.st 2>&1 | grep -q "PASS"; then
    echo -e "${GREEN}✓ 通过${NC}"
    PASSED=$((PASSED + 1))
else
    echo -e "${RED}✗ 失败${NC}"
    FAILED=$((FAILED + 1))
fi
TOTAL=$((TOTAL + 1))
echo ""

# 测试 3: 文件操作
cat > /tmp/test_file_ops.st << 'EOF'
PROGRAM TestFileOps
VAR
    r: INT;
END_VAR
PRINT('Test: File operations\n');
r := SYSTEM('echo "test data" > /tmp/stvm_verify_test.txt');
r := SYSTEM('cat /tmp/stvm_verify_test.txt');
r := SYSTEM('rm /tmp/stvm_verify_test.txt');
IF r = 0 THEN
    PRINT('PASS\n');
END_IF;
END_PROGRAM
EOF

echo -e "${YELLOW}即时测试 3:${NC} 文件操作"
if ./build/bin/stvm /tmp/test_file_ops.st 2>&1 | grep -q "test data"; then
    echo -e "${GREEN}✓ 通过${NC}"
    PASSED=$((PASSED + 1))
else
    echo -e "${RED}✗ 失败${NC}"
    FAILED=$((FAILED + 1))
fi
TOTAL=$((TOTAL + 1))
echo ""

# 清理
rm -f /tmp/test_*.st /tmp/stvm_test_output.txt /tmp/stvm_verify_test.txt

# 总结
echo "====================================="
echo "  测试总结"
echo "====================================="
echo ""
echo "总计: $TOTAL 个测试"
echo -e "${GREEN}通过: $PASSED${NC}"
if [ $FAILED -gt 0 ]; then
    echo -e "${RED}失败: $FAILED${NC}"
else
    echo "失败: 0"
fi
echo ""

# 计算百分比
if [ $TOTAL -gt 0 ]; then
    PERCENTAGE=$((PASSED * 100 / TOTAL))
    echo "通过率: $PERCENTAGE%"
fi
echo ""

# 最终结果
if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}====================================="
    echo "  所有测试通过！"
    echo "  SYSTEM 函数工作正常 ✓"
    echo "=====================================${NC}"
    exit 0
else
    echo -e "${RED}====================================="
    echo "  有测试失败"
    echo "  请检查输出"
    echo "=====================================${NC}"
    exit 1
fi
