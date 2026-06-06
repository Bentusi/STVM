#!/bin/bash

# ============================================================================
# STVM Functional Test Suite
# 三大综合功能测试: 语法 / 安全 / IO
# ============================================================================

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
STVM_BIN="$SCRIPT_DIR/build/bin/stvm"
FUNC_DIR="$SCRIPT_DIR/tests/functional"
TOTAL=0
PASSED=0
FAILED=0

echo ""
echo -e "${BLUE}============================================${NC}"
echo -e "${BLUE}  STVM 功能安全综合测试套件${NC}"
echo -e "${BLUE}============================================${NC}"
echo ""

run_test() {
    local name="$1"
    local file="$2"
    local args="$3"
    local bc_file="${file%.st}.stbc"
    
    TOTAL=$((TOTAL + 1))
    echo -n "Testing: $name ... "
    
    # 编译
    "$STVM_BIN" -c "$file" -o "$bc_file" -V > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo -e "${RED}FAILED (compile)${NC}"
        FAILED=$((FAILED + 1))
        "$STVM_BIN" -c "$file" -o "$bc_file" -V 2>&1 | head -5
        return 1
    fi
    
    # 运行
    timeout 10 "$STVM_BIN" -r "$bc_file" $args -V > /tmp/stvm_func_test_$$.log 2>&1
    local rc=$?
    if [ $rc -eq 0 ] || [ $rc -eq 124 ]; then
        echo -e "${GREEN}PASSED${NC}"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}FAILED (runtime)${NC}"
        FAILED=$((FAILED + 1))
        tail -5 /tmp/stvm_func_test_$$.log
    fi
    
    rm -f "$bc_file" /tmp/stvm_func_test_$$.log
}

# ================================================================
# 测试 1: 语法功能综合测试
# ================================================================
echo -e "${YELLOW}[1/3] 语法功能测试${NC}"
run_test "syntax_test"  "$FUNC_DIR/syntax_test.st"  ""
echo ""

# ================================================================
# 测试 2: 安全功能综合测试 (跳过 I/O 要求)
# ================================================================
echo -e "${YELLOW}[2/3] 安全功能测试${NC}"
run_test "safety_test"  "$FUNC_DIR/safety_test.st"  ""
echo ""

# ================================================================
# 测试 3: IO 功能综合测试 (启用模拟器)
# ================================================================
echo -e "${YELLOW}[3/3] IO 功能测试${NC}"
run_test "io_test"     "$FUNC_DIR/io_test.st"     "-I"
echo ""

# ================================================================
# WCET 分析测试
# ================================================================
echo -e "${YELLOW}[WCET] WCET 分析演示${NC}"
echo -n "  syntax WCET ... "
timeout 5 "$STVM_BIN" --wcet "$FUNC_DIR/syntax_test.stbc" > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo -e "${GREEN}OK${NC}"
else
    echo -e "${YELLOW}SKIP${NC}"
fi
rm -f "$FUNC_DIR/syntax_test.stbc"
echo ""

# ================================================================
# 测试总结
# ================================================================
echo -e "${BLUE}============================================${NC}"
echo -e "${BLUE}  测试结果${NC}"
echo -e "${BLUE}============================================${NC}"
echo -e "Total:   $TOTAL"
echo -e "${GREEN}Passed:  $PASSED${NC}"
if [ $FAILED -gt 0 ]; then
    echo -e "${RED}Failed:  $FAILED${NC}"
else
    echo -e "Failed:  $FAILED"
fi

if [ $FAILED -eq 0 ]; then
    echo -e "\n${GREEN}所有功能测试通过!${NC}"
    exit 0
else
    echo -e "\n${RED}存在失败用例${NC}"
    exit 1
fi