# IEC61131 ST语言编译器和虚拟机构建脚本
# 支持库管理、主从同步和完整的模块化架构

# 编译器配置
CC = gcc
FLEX = flex
BISON = bison

# 编译选项
CFLAGS = -Wall -Wextra -g -O2 -DDEBUG
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

# 主要可执行文件
COMPILER = stc
VM_RUNNER = stvm

# 核心源文件
CORE_SOURCES = \
    st_lexer.c \
    st_parser.c \
    ast.c \
    symbol_table.c \
    mmgr.c \
    bytecode_generator.c \
    vm.c \
    libmgr.c

# 内置库源文件
BUILTIN_SOURCES = \
    builtin_math.c \
    builtin_string.c \
    builtin_io.c

# 工具源文件
TOOL_SOURCES = \
    main.c \
    vm_main.c 

# 生成的源文件
GENERATED_SOURCES = \
    st_lexer.c \
    st_parser.c \
    st_parser.h

# 目标文件
CORE_OBJECTS = $(CORE_SOURCES:.c=.o)
BUILTIN_OBJECTS = $(BUILTIN_SOURCES:.c=.o)
TOOL_OBJECTS = $(TOOL_SOURCES:.c=.o)

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
all: directories $(COMPILER) $(VM_RUNNER) $(LIBST) $(LIBBUILTIN)

# 创建必要目录
directories:
	@mkdir -p $(BUILD_DIR)

# 编译器主程序
$(COMPILER): main.o $(CORE_OBJECTS) $(BUILTIN_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "编译器构建完成: $(COMPILER)"

# 虚拟机运行器
$(VM_RUNNER): vm_main.o $(CORE_OBJECTS) $(BUILTIN_OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "虚拟机运行器构建完成: $(VM_RUNNER)"

# Flex词法分析器生成
st_lexer.c: st_lexer.lex st_parser.h
	$(FLEX) -o $@ $<
	@echo "词法分析器生成完成"

# Bison语法分析器生成
st_parser.c st_parser.h: st_parser.y
	$(BISON) -d -v -o st_parser.c $<
	@echo "语法分析器生成完成"

# 核心模块编译规则
ast.o: ast.c ast.h mmgr.h
	$(CC) $(CFLAGS) -c -o $@ $<

symbol_table.o: symbol_table.c symbol_table.h mmgr.h ast.h
	$(CC) $(CFLAGS) -c -o $@ $<

mmgr.o: mmgr.c mmgr.h
	$(CC) $(CFLAGS) -c -o $@ $<

bytecode_generator.o: bytecode_generator.c bytecode_generator.h mmgr.h
	$(CC) $(CFLAGS) -c -o $@ $<

vm.o: vm.c vm.h bytecode_generator.h mmgr.h libmgr.h ms_sync.h
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

# 主程序编译规则
main.o: main.c ast.h symbol_table.h bytecode_generator.h vm.h libmgr.h
	$(CC) $(CFLAGS) -c -o $@ $<

vm_main.o: vm_main.c vm.h bytecode_generator.h
	$(CC) $(CFLAGS) -c -o $@ $<

# 通用编译规则
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<


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

# 清理
clean:
	@echo "清理生成文件..."
	@rm -f *.o
	@rm -f $(GENERATED_SOURCES)
	@rm -f st_parser.h st_parser.output
	@rm -f $(COMPILER) $(VM_RUNNER)
	@rm -f $(LIBST) $(LIBBUILTIN)
	@rm -f *.stbc
	@rm -f tests/test_*
	@rm -f *.gcov *.gcda *.gcno
	@rm -rf $(BUILD_DIR)
	@echo "清理完成"

# 声明伪目标
.PHONY: all clean test safety static-analysis coverage

# 依赖关系声明
st_parser.o: st_parser.c st_parser.h ast.h symbol_table.h
st_lexer.o: st_lexer.c st_parser.h ast.h