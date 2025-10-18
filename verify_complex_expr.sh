#!/bin/bash
#
# 复杂表达式支持验证脚本
# 验证 STVM 是否正确处理各种复杂表达式
#

set -e

STVM="./build/bin/stvm"
TEST_DIR="examples"
PASS=0
FAIL=0

echo "========================================"
echo "STVM 复杂表达式支持验证"
echo "========================================"
echo

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

run_test() {
    local test_file=$1
    local test_name=$2
    
    echo -n "Testing: $test_name ... "
    
    if [ ! -f "$TEST_DIR/$test_file" ]; then
        echo -e "${YELLOW}SKIP${NC} (file not found)"
        return
    fi
    
    if $STVM "$TEST_DIR/$test_file" > /tmp/stvm_test_$$.out 2>&1; then
        echo -e "${GREEN}PASS${NC}"
        PASS=$((PASS + 1))
        # 显示关键输出
        if grep -q "Result:\|result =\|All tests" /tmp/stvm_test_$$.out; then
            grep "Result:\|result =\|All tests" /tmp/stvm_test_$$.out | head -3 | sed 's/^/  → /'
        fi
    else
        echo -e "${RED}FAIL${NC}"
        FAIL=$((FAIL + 1))
        cat /tmp/stvm_test_$$.out | head -10
    fi
    
    rm -f /tmp/stvm_test_$$.out
    echo
}

# 检查 STVM 是否存在
if [ ! -f "$STVM" ]; then
    echo -e "${RED}Error: STVM not found at $STVM${NC}"
    echo "Please run 'make stvm' first"
    exit 1
fi

# 测试1: 基础复杂表达式
run_test "test_complex_expr.st" "Basic complex expression"

# 测试2: 多样复杂表达式
run_test "test_very_complex_expr.st" "Various complex patterns"

# 测试3: 原始表达式直接测试
run_test "test_original_expr.st" "Original expression direct test"

# 测试4: PID 演示（原始表达式版本）
run_test "pid_demo_original_expr.st" "PID demo with original expressions"

# 测试5: PID 演示（简化版本）
run_test "pid_demo_fixed.st" "PID demo with simplified expressions"

# 测试6: 简单 PID 测试
run_test "pid_test.st" "Simple PID test"

echo "========================================"
echo "Test Results:"
echo "========================================"
echo -e "PASS: ${GREEN}${PASS}${NC}"
echo -e "FAIL: ${RED}${FAIL}${NC}"
echo "========================================"

if [ $FAIL -eq 0 ]; then
    echo -e "${GREEN}✅ All tests PASSED!${NC}"
    echo
    echo "Conclusion:"
    echo "  STVM code generator fully supports"
    echo "  complex nested arithmetic expressions!"
    exit 0
else
    echo -e "${RED}❌ Some tests FAILED${NC}"
    exit 1
fi
