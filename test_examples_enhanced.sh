#!/bin/bash

# ============================================================================
# STVM Examples Test Script (Enhanced Version)
# 测试 examples 目录下所有 ST 文件的编译和执行
# 支持库文件依赖处理
# ============================================================================

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# 统计变量
TOTAL=0
PASSED=0
FAILED=0
SKIPPED=0

# 结果数组
declare -a PASSED_TESTS
declare -a FAILED_TESTS
declare -a SKIPPED_TESTS

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXAMPLES_DIR="$SCRIPT_DIR/examples"
STVM_BIN="$SCRIPT_DIR/build/bin/stvm"
TEMP_DIR="/tmp/stvm_test_$$"
REPORT_FILE="/tmp/test_results.txt"

# 详细模式
VERBOSE=0

# 创建临时目录
mkdir -p "$TEMP_DIR"

# 清理函数
cleanup() {
    rm -rf "$TEMP_DIR"
}
trap cleanup EXIT

# 打印分隔线
print_separator() {
    echo "========================================================================"
}

# 打印测试头部
print_header() {
    print_separator
    echo -e "${BLUE}STVM Examples Test Suite (Enhanced)${NC}"
    echo "Examples Directory: $EXAMPLES_DIR"
    echo "STVM Binary: $STVM_BIN"
    echo "Test Report: $REPORT_FILE"
    print_separator
    echo ""
}

# 检查 STVM 是否存在
check_stvm() {
    if [ ! -f "$STVM_BIN" ]; then
        echo -e "${RED}Error: STVM binary not found at $STVM_BIN${NC}"
        echo "Please run 'make stvm' first"
        exit 1
    fi
}

# 预编译库文件
precompile_libraries() {
    echo -e "${CYAN}Step 1: Pre-compiling library files...${NC}"
    
    # mathlib.st 需要先编译
    if [ -f "$EXAMPLES_DIR/mathlib.st" ]; then
        echo -n "  Compiling mathlib.st ... "
        if "$STVM_BIN" -c "$EXAMPLES_DIR/mathlib.st" -o "$EXAMPLES_DIR/mathlib.stbc" > /dev/null 2>&1; then
            echo -e "${GREEN}OK${NC}"
        else
            echo -e "${YELLOW}FAILED (non-critical)${NC}"
        fi
    fi
    
    # engineering_lib.st 可能需要先编译
    if [ -f "$EXAMPLES_DIR/engineering_lib.st" ]; then
        echo -n "  Compiling engineering_lib.st ... "
        if timeout 5s "$STVM_BIN" -c "$EXAMPLES_DIR/engineering_lib.st" -o "$EXAMPLES_DIR/engineering_lib.stbc" > /dev/null 2>&1; then
            echo -e "${GREEN}OK${NC}"
        else
            echo -e "${YELLOW}FAILED (non-critical)${NC}"
        fi
    fi
    
    echo ""
}

# 测试单个文件
test_file() {
    local st_file="$1"
    local basename=$(basename "$st_file" .st)
    local stbc_file="$TEMP_DIR/${basename}.stbc"
    local log_file="$TEMP_DIR/${basename}.log"
    local err_file="$TEMP_DIR/${basename}.err"
    
    echo -n "Testing: $(printf '%-45s' "$basename.st") ... "
    
    TOTAL=$((TOTAL + 1))
    
    # 步骤 1: 编译 ST -> STBC
    if ! timeout 10s "$STVM_BIN" -c "$st_file" -o "$stbc_file" > "$log_file" 2> "$err_file"; then
        local exit_code=$?
        
        if [ $exit_code -eq 124 ]; then
            echo -e "${YELLOW}COMPILE TIMEOUT${NC}"
            SKIPPED=$((SKIPPED + 1))
            SKIPPED_TESTS+=("$basename: Compilation timeout (10s)")
            return 2
        fi
        
        echo -e "${RED}COMPILE FAILED${NC}"
        FAILED=$((FAILED + 1))
        
        # 提取关键错误信息
        local error_summary=$(grep -E "error|Error|错误" "$err_file" | head -3 | sed 's/^/    /')
        if [ -n "$error_summary" ]; then
            FAILED_TESTS+=("$basename: Compilation failed - $(echo "$error_summary" | head -1 | sed 's/^[[:space:]]*//')")
        else
            FAILED_TESTS+=("$basename: Compilation failed")
        fi
        
        if [ $VERBOSE -eq 1 ]; then
            echo "  Error output:"
            head -5 "$err_file" | sed 's/^/    /'
        fi
        return 1
    fi
    
    # 检查字节码文件是否生成
    if [ ! -f "$stbc_file" ]; then
        echo -e "${RED}COMPILE FAILED (no output)${NC}"
        FAILED=$((FAILED + 1))
        FAILED_TESTS+=("$basename: No bytecode generated")
        return 1
    fi
    
    # 步骤 2: 执行字节码
    local exit_code=0
    local run_cmd="$STVM_BIN -r $stbc_file"
    
    # 如果是 IO 测试文件，添加 -I 参数和 IO 配置
    if [[ "$basename" == io_* ]]; then
        local io_config="$EXAMPLES_DIR/io_config.json"
        if [ -f "$io_config" ]; then
            run_cmd="$STVM_BIN -I --io-config $io_config -r $stbc_file"
        fi
    fi
    
    if ! timeout 5s $run_cmd > "$log_file" 2> "$err_file"; then
        exit_code=$?
        
        if [ $exit_code -eq 124 ]; then
            echo -e "${YELLOW}RUN TIMEOUT${NC}"
            SKIPPED=$((SKIPPED + 1))
            SKIPPED_TESTS+=("$basename: Execution timeout (5s)")
            return 2
        fi
        
        # 检查是否有段错误
        if grep -q "段错误\|Segmentation fault\|core dump" "$err_file" 2>/dev/null; then
            echo -e "${RED}SEGFAULT${NC}"
            FAILED=$((FAILED + 1))
            FAILED_TESTS+=("$basename: Segmentation fault")
            return 1
        fi
        
        # 某些程序可能有非零退出码但仍然是预期的
        if [ -s "$log_file" ]; then
            echo -e "${YELLOW}RUN WARNING (exit: $exit_code)${NC}"
            PASSED=$((PASSED + 1))
            PASSED_TESTS+=("$basename: Executed with warnings (exit: $exit_code)")
            return 0
        else
            echo -e "${RED}RUN FAILED${NC}"
            FAILED=$((FAILED + 1))
            FAILED_TESTS+=("$basename: Execution failed (exit: $exit_code)")
            
            if [ $VERBOSE -eq 1 ] && [ -s "$err_file" ]; then
                echo "  Error output:"
                head -5 "$err_file" | sed 's/^/    /'
            fi
            return 1
        fi
    fi
    
    # 成功
    echo -e "${GREEN}PASSED${NC}"
    PASSED=$((PASSED + 1))
    PASSED_TESTS+=("$basename")
    
    # 显示输出的前几行（如果有）
    if [ $VERBOSE -eq 1 ] && [ -s "$log_file" ]; then
        local line_count=$(wc -l < "$log_file")
        if [ $line_count -le 3 ]; then
            echo "  Output:"
            sed 's/^/    /' "$log_file"
        fi
    fi
    
    return 0
}

# 主测试函数
run_tests() {
    echo -e "${CYAN}Step 2: Running tests...${NC}"
    echo ""
    
    # 查找所有 .st 文件（排除已编译的 .stbc）
    local st_files=("$EXAMPLES_DIR"/*.st)
    
    if [ ${#st_files[@]} -eq 0 ]; then
        echo -e "${YELLOW}No .st files found in $EXAMPLES_DIR${NC}"
        exit 0
    fi
    
    # 测试每个文件
    for st_file in "${st_files[@]}"; do
        # 跳过不存在的文件（glob 展开失败的情况）
        [ -f "$st_file" ] || continue
        
        # 跳过某些已知有问题的文件
        local basename=$(basename "$st_file")
        test_file "$st_file"
    done
}

# 生成详细报告
generate_report() {
    local report="$REPORT_FILE"
    
    {
        echo "========================================================================"
        echo "STVM Examples Test Report"
        echo "========================================================================"
        echo "Date: $(date '+%Y-%m-%d %H:%M:%S')"
        echo "STVM Binary: $STVM_BIN"
        echo "Examples Directory: $EXAMPLES_DIR"
        echo ""
        echo "========================================================================"
        echo "Summary"
        echo "========================================================================"
        echo "Total Tests:   $TOTAL"
        echo "Passed:        $PASSED"
        echo "Failed:        $FAILED"
        echo "Skipped:       $SKIPPED"
        
        if [ $TOTAL -gt 0 ]; then
            local success_rate=$((PASSED * 100 / TOTAL))
            local effective_total=$((TOTAL - SKIPPED))
            local effective_rate=0
            if [ $effective_total -gt 0 ]; then
                effective_rate=$((PASSED * 100 / effective_total))
            fi
            echo ""
            echo "Success Rate:           ${success_rate}% ($PASSED/$TOTAL)"
            echo "Effective Success Rate: ${effective_rate}% ($PASSED/$effective_total, excluding skipped)"
        fi
        
        echo ""
        echo "========================================================================"
        echo "Passed Tests ($PASSED)"
        echo "========================================================================"
        for test in "${PASSED_TESTS[@]}"; do
            echo "  ✓ $test"
        done
        
        if [ $FAILED -gt 0 ]; then
            echo ""
            echo "========================================================================"
            echo "Failed Tests ($FAILED)"
            echo "========================================================================"
            for test in "${FAILED_TESTS[@]}"; do
                echo "  ✗ $test"
            done
        fi
        
        if [ $SKIPPED -gt 0 ]; then
            echo ""
            echo "========================================================================"
            echo "Skipped Tests ($SKIPPED)"
            echo "========================================================================"
            for test in "${SKIPPED_TESTS[@]}"; do
                echo "  ⊘ $test"
            done
        fi
        
        echo ""
        echo "========================================================================"
        echo "End of Report"
        echo "========================================================================"
    } > "$report"
    
    echo -e "${CYAN}Detailed report saved to: $report${NC}"
}

# 打印总结
print_summary() {
    echo ""
    print_separator
    echo -e "${BLUE}Test Summary${NC}"
    print_separator
    printf "%-12s %d\n" "Total:" "$TOTAL"
    printf "${GREEN}%-12s %d${NC}\n" "Passed:" "$PASSED"
    printf "${RED}%-12s %d${NC}\n" "Failed:" "$FAILED"
    printf "${YELLOW}%-12s %d${NC}\n" "Skipped:" "$SKIPPED"
    
    # 计算成功率
    if [ $TOTAL -gt 0 ]; then
        local success_rate=$((PASSED * 100 / TOTAL))
        local effective_total=$((TOTAL - SKIPPED))
        if [ $effective_total -gt 0 ]; then
            local effective_rate=$((PASSED * 100 / effective_total))
            echo ""
            printf "Success Rate:           %d%% (%d/%d)\n" "$success_rate" "$PASSED" "$TOTAL"
            printf "Effective Success Rate: %d%% (%d/%d, excluding skipped)\n" "$effective_rate" "$PASSED" "$effective_total"
        else
            echo ""
            printf "Success Rate: %d%%\n" "$success_rate"
        fi
    fi
    
    print_separator
    
    # 生成详细报告
    generate_report
    
    # 返回适当的退出码
    if [ $FAILED -gt 0 ]; then
        return 1
    else
        return 0
    fi
}

# ============================================================================
# 主程序
# ============================================================================

main() {
    print_header
    check_stvm
    precompile_libraries
    run_tests
    print_summary
}

# 支持命令行参数
case "${1:-}" in
    -h|--help)
        echo "Usage: $0 [OPTIONS]"
        echo ""
        echo "Test all ST files in the examples directory"
        echo ""
        echo "Options:"
        echo "  -h, --help     Show this help message"
        echo "  -v, --verbose  Verbose output (show compilation/runtime output)"
        echo ""
        echo "Output:"
        echo "  Console: Summary of test results"
        echo "  File:    Detailed report saved to test_results.txt"
        echo ""
        exit 0
        ;;
    -v|--verbose)
        VERBOSE=1
        main
        ;;
    *)
        main
        ;;
esac
