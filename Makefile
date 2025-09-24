# IEC61131 ST语言编译器和虚拟机构建脚本
# 支持库管理、主从同步和完整的模块化架构

# 编译器配置
CC = gcc
FLEX = flex
BISON = bison

# 编译选项
CFLAGS = -Wall -Wextra -std=c99 -g -O2 -DDEBUG
LDFLAGS = -lm

# 安全级编译选项 (MISRA合规)
SAFETY_CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99 \
                -Wstrict-prototypes -Wmissing-prototypes \
                -Wold-style-definition -Wmissing-declarations \
                -Wredundant-decls -Wnested-externs

# 目录配置
SRC_DIR = .
BUILD_DIR = build
LIB_DIR = libs
TEST_DIR = tests
DOC_DIR = docs

# 主要可执行文件
COMPILER = stc
VM_RUNNER = stvm
DEBUGGER = stdbg

# 核心源文件
CORE_SOURCES = \
    st_lexer.c \
    st_parser.c \
    ast.c \
    symbol_table.c \
    type_checker.c \
    mmgr.c \
    bytecode.c \
    codegen.c \
    vm.c \
    ms_sync.c \
    libmgr.c

# 内置库源文件
BUILTIN_SOURCES = \
    builtin_math.c \
    builtin_string.c \
    builtin_io.c \
    builtin_time.c

# 工具源文件
TOOL_SOURCES = \
    main.c \
    vm_main.c \
    debugger.c

# 测试源文件
TEST_SOURCES = \
    tests/test_lexer.c \
    tests/test_parser.c \
    tests/test_ast.c \
    tests/test_vm.c \
    tests/test_sync.c \
    tests/test_integration.c

# 生成的源文件
GENERATED_SOURCES = \
    st_lexer.c \
    st_parser.c \
    st_parser.tab.h

# 目标文件
CORE_OBJECTS = $(CORE_SOURCES:.c=.o)
BUILTIN_OBJECTS = $(BUILTIN_SOURCES:.c=.o)
TOOL_OBJECTS = $(TOOL_SOURCES:.c=.o)
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)

ALL_OBJECTS = $(CORE_OBJECTS) $(BUILTIN_OBJECTS) $(TOOL_OBJECTS)

# 库文件
LIBST = libst.a
LIBBUILTIN = libbuiltin.a

# 示例程序
EXAMPLES = \
    examples/hello_world.st \
    examples/factorial.st \
    examples/temperature_control.st \
    examples/sync_example.st

# 默认目标
all: directories $(COMPILER) $(VM_RUNNER) $(DEBUGGER) $(LIBST) $(LIBBUILTIN)

# 创建必要目录
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(LIB_DIR)
	@mkdir -p $(TEST_DIR)
	@mkdir -p $(DOC_DIR)
	@mkdir -p examples

# 编译器主程序
$(COMPILER): main.o $(CORE_OBJECTS) $(BUILTIN_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "编译器构建完成: $(COMPILER)"

# 虚拟机运行器
$(VM_RUNNER): vm_main.o $(CORE_OBJECTS) $(BUILTIN_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "虚拟机运行器构建完成: $(VM_RUNNER)"

# 调试器
$(DEBUGGER): debugger.o $(CORE_OBJECTS) $(BUILTIN_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "调试器构建完成: $(DEBUGGER)"

# 核心库
$(LIBST): $(CORE_OBJECTS)
	ar rcs $@ $^
	@echo "核心库构建完成: $(LIBST)"

# 内置函数库
$(LIBBUILTIN): $(BUILTIN_OBJECTS)
	ar rcs $@ $^
	@echo "内置函数库构建完成: $(LIBBUILTIN)"

# Flex词法分析器生成
st_lexer.c: st_lexer.lex st_parser.tab.h
	$(FLEX) -o $@ $<
	@echo "词法分析器生成完成"

# Bison语法分析器生成
st_parser.c st_parser.tab.h: st_parser.y
	$(BISON) -d -v -o st_parser.c $<
	@echo "语法分析器生成完成"

# 核心模块编译规则
ast.o: ast.c ast.h mmgr.h
	$(CC) $(CFLAGS) -c -o $@ $<

symbol_table.o: symbol_table.c symbol_table.h mmgr.h ast.h
	$(CC) $(CFLAGS) -c -o $@ $<

type_checker.o: type_checker.c type_checker.h ast.h symbol_table.h
	$(CC) $(CFLAGS) -c -o $@ $<

mmgr.o: mmgr.c mmgr.h
	$(CC) $(CFLAGS) -c -o $@ $<

bytecode.o: bytecode.c bytecode.h mmgr.h
	$(CC) $(CFLAGS) -c -o $@ $<

codegen.o: codegen.c codegen.h ast.h bytecode.h symbol_table.h libmgr.h ms_sync.h
	$(CC) $(CFLAGS) -c -o $@ $<

vm.o: vm.c vm.h bytecode.h mmgr.h libmgr.h ms_sync.h
	$(CC) $(CFLAGS) -c -o $@ $<

ms_sync.o: ms_sync.c ms_sync.h vm.h mmgr.h
	$(CC) $(CFLAGS) -c -o $@ $<

libmgr.o: libmgr.c libmgr.h mmgr.h symbol_table.h
	$(CC) $(CFLAGS) -c -o $@ $<

# 内置库编译规则
builtin_math.o: builtin_math.c libmgr.h vm.h
	$(CC) $(CFLAGS) -c -o $@ $<

builtin_string.o: builtin_string.c libmgr.h vm.h mmgr.h
	$(CC) $(CFLAGS) -c -o $@ $<

builtin_io.o: builtin_io.c libmgr.h vm.h
	$(CC) $(CFLAGS) -c -o $@ $<

builtin_time.o: builtin_time.c libmgr.h vm.h
	$(CC) $(CFLAGS) -c -o $@ $<

# 主程序编译规则
main.o: main.c ast.h symbol_table.h codegen.h vm.h libmgr.h
	$(CC) $(CFLAGS) -c -o $@ $<

vm_main.o: vm_main.c vm.h bytecode.h ms_sync.h
	$(CC) $(CFLAGS) -c -o $@ $<

debugger.o: debugger.c vm.h bytecode.h
	$(CC) $(CFLAGS) -c -o $@ $<

# 通用编译规则
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# 测试目标
test: test-lexer test-parser test-ast test-vm test-sync test-integration
	@echo "所有测试完成"

test-lexer: tests/test_lexer
	@echo "运行词法分析器测试..."
	./tests/test_lexer
	@echo "词法分析器测试通过"

test-parser: tests/test_parser
	@echo "运行语法分析器测试..."
	./tests/test_parser
	@echo "语法分析器测试通过"

test-ast: tests/test_ast
	@echo "运行AST测试..."
	./tests/test_ast
	@echo "AST测试通过"

test-vm: tests/test_vm
	@echo "运行虚拟机测试..."
	./tests/test_vm
	@echo "虚拟机测试通过"

test-sync: tests/test_sync
	@echo "运行同步系统测试..."
	./tests/test_sync
	@echo "同步系统测试通过"

test-integration: tests/test_integration
	@echo "运行集成测试..."
	./tests/test_integration
	@echo "集成测试通过"

# 测试程序构建
tests/test_lexer: tests/test_lexer.o st_lexer.o mmgr.o
	@mkdir -p tests
	$(CC) -o $@ $^ $(LDFLAGS)

tests/test_parser: tests/test_parser.o st_parser.o st_lexer.o ast.o mmgr.o symbol_table.o
	@mkdir -p tests
	$(CC) -o $@ $^ $(LDFLAGS)

tests/test_ast: tests/test_ast.o ast.o mmgr.o
	@mkdir -p tests
	$(CC) -o $@ $^ $(LDFLAGS)

tests/test_vm: tests/test_vm.o $(CORE_OBJECTS) $(BUILTIN_OBJECTS)
	@mkdir -p tests
	$(CC) -o $@ $^ $(LDFLAGS)

tests/test_sync: tests/test_sync.o ms_sync.o vm.o mmgr.o bytecode.o
	@mkdir -p tests
	$(CC) -o $@ $^ $(LDFLAGS)

tests/test_integration: tests/test_integration.o $(CORE_OBJECTS) $(BUILTIN_OBJECTS)
	@mkdir -p tests
	$(CC) -o $@ $^ $(LDFLAGS)

# 示例程序测试
test-examples: $(COMPILER) $(VM_RUNNER)
	@echo "测试示例程序..."
	@for example in $(EXAMPLES); do \
		if [ -f $$example ]; then \
			echo "编译 $$example..."; \
			./$(COMPILER) $$example -o $${example%.st}.stbc || exit 1; \
			echo "运行 $$example..."; \
			./$(VM_RUNNER) $${example%.st}.stbc || exit 1; \
		fi; \
	done
	@echo "示例程序测试完成"

# 性能测试
benchmark: $(COMPILER) $(VM_RUNNER)
	@echo "运行性能测试..."
	@./$(COMPILER) examples/factorial.st -o factorial.stbc
	@time ./$(VM_RUNNER) factorial.stbc
	@echo "性能测试完成"

# 主从同步测试
test-sync-demo: $(VM_RUNNER)
	@echo "启动主从同步演示..."
	@./$(VM_RUNNER) --sync-primary --ip 127.0.0.1 --port 8888 examples/sync_example.stbc &
	@sleep 1
	@./$(VM_RUNNER) --sync-secondary --ip 127.0.0.1 --port 8888 examples/sync_example.stbc &
	@echo "主从同步演示启动完成（后台运行）"

# 内存检查
memcheck: $(COMPILER) $(VM_RUNNER)
	@echo "运行内存检查..."
	@if command -v valgrind >/dev/null 2>&1; then \
		valgrind --leak-check=full ./$(COMPILER) examples/hello_world.st; \
		valgrind --leak-check=full ./$(VM_RUNNER) hello_world.stbc; \
	else \
		echo "Valgrind未安装，跳过内存检查"; \
	fi

# 安全级编译（MISRA合规）
safety: CFLAGS = $(SAFETY_CFLAGS)
safety: clean all
	@echo "安全级编译完成（MISRA合规）"

# 静态分析
static-analysis:
	@echo "运行静态代码分析..."
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=all --std=c99 $(CORE_SOURCES) $(BUILTIN_SOURCES); \
	else \
		echo "cppcheck未安装，跳过静态分析"; \
	fi

# 代码覆盖率
coverage: CFLAGS += --coverage
coverage: LDFLAGS += --coverage
coverage: clean test
	@echo "生成代码覆盖率报告..."
	@if command -v gcov >/dev/null 2>&1; then \
		gcov $(CORE_SOURCES); \
		echo "覆盖率报告已生成（*.gcov文件）"; \
	else \
		echo "gcov未安装，跳过覆盖率分析"; \
	fi

# 文档生成
doc:
	@echo "生成API文档..."
	@mkdir -p $(DOC_DIR)
	@if command -v doxygen >/dev/null 2>&1; then \
		doxygen Doxyfile; \
	else \
		echo "Doxygen未安装，跳过文档生成"; \
	fi

# 安装
install: all
	@echo "安装编译器系统..."
	@mkdir -p /usr/local/bin
	@mkdir -p /usr/local/lib
	@mkdir -p /usr/local/include/st
	@cp $(COMPILER) $(VM_RUNNER) $(DEBUGGER) /usr/local/bin/
	@cp $(LIBST) $(LIBBUILTIN) /usr/local/lib/
	@cp *.h /usr/local/include/st/
	@echo "安装完成"

# 卸载
uninstall:
	@echo "卸载编译器系统..."
	@rm -f /usr/local/bin/$(COMPILER)
	@rm -f /usr/local/bin/$(VM_RUNNER)
	@rm -f /usr/local/bin/$(DEBUGGER)
	@rm -f /usr/local/lib/$(LIBST)
	@rm -f /usr/local/lib/$(LIBBUILTIN)
	@rm -rf /usr/local/include/st
	@echo "卸载完成"

# 打包发布
package: clean
	@echo "创建发布包..."
	@tar czf st-compiler-$(shell date +%Y%m%d).tar.gz \
		*.c *.h *.lex *.y Makefile CLAUDE.md LICENSE README.md \
		examples/ tests/ docs/
	@echo "发布包创建完成: st-compiler-$(shell date +%Y%m%d).tar.gz"

# 清理
clean:
	@echo "清理生成文件..."
	@rm -f *.o
	@rm -f $(GENERATED_SOURCES)
	@rm -f st_parser.tab.h st_parser.output
	@rm -f $(COMPILER) $(VM_RUNNER) $(DEBUGGER)
	@rm -f $(LIBST) $(LIBBUILTIN)
	@rm -f *.stbc
	@rm -f tests/test_*
	@rm -f *.gcov *.gcda *.gcno
	@rm -rf $(BUILD_DIR)
	@echo "清理完成"

# 深度清理
distclean: clean
	@echo "深度清理..."
	@rm -rf $(DOC_DIR)/html
	@rm -f *.tar.gz
	@echo "深度清理完成"

# 帮助信息
help:
	@echo "IEC61131 ST语言编译器构建系统"
	@echo ""
	@echo "主要目标:"
	@echo "  all          - 构建所有组件（默认）"
	@echo "  $(COMPILER)  - 构建编译器"
	@echo "  $(VM_RUNNER) - 构建虚拟机运行器"
	@echo "  $(DEBUGGER)  - 构建调试器"
	@echo ""
	@echo "测试目标:"
	@echo "  test         - 运行所有测试"
	@echo "  test-examples- 测试示例程序"
	@echo "  test-sync-demo- 主从同步演示"
	@echo "  benchmark    - 性能测试"
	@echo "  memcheck     - 内存检查（需要Valgrind）"
	@echo ""
	@echo "质量保证:"
	@echo "  safety       - 安全级编译（MISRA合规）"
	@echo "  static-analysis - 静态代码分析"
	@echo "  coverage     - 代码覆盖率分析"
	@echo ""
	@echo "文档和发布:"
	@echo "  doc          - 生成API文档"
	@echo "  package      - 创建发布包"
	@echo ""
	@echo "系统管理:"
	@echo "  install      - 安装到系统"
	@echo "  uninstall    - 从系统卸载"
	@echo "  clean        - 清理生成文件"
	@echo "  distclean    - 深度清理"
	@echo ""
	@echo "示例用法:"
	@echo "  make                    # 构建所有组件"
	@echo "  make test               # 运行测试"
	@echo "  make safety             # 安全级编译"
	@echo "  make install            # 安装到系统"

# 声明伪目标
.PHONY: all clean distclean test test-examples test-sync-demo \
        benchmark memcheck safety static-analysis coverage \
        doc install uninstall package help directories

# 依赖关系声明
st_parser.o: st_parser.c st_parser.tab.h ast.h symbol_table.h
st_lexer.o: st_lexer.c st_parser.tab.h ast.h