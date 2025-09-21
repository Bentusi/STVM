# IEC61131 结构化文本编译器 Makefile
# 使用 Flex 和 Bison 生成词法和语法分析器

CC = gcc
CFLAGS = -g -Wall -Wextra
LIBS = -lm -ll
FLEX = flex
BISON = bison

# 目标文件
TARGET = stvm
OBJS = main.o ast.o vm.o st_lexer.o st_parser.tab.o

# 生成的文件
GENERATED = st_lexer.c st_parser.tab.c st_parser.tab.h

.PHONY: all clean install test

all: $(TARGET)

# 主目标
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# 编译目标文件
main.o: main.c ast.h vm.h st_parser.tab.h
	$(CC) $(CFLAGS) -c $<

ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c $<

vm.o: vm.c vm.h ast.h
	$(CC) $(CFLAGS) -c $<

st_lexer.o: st_lexer.c st_parser.tab.h
	$(CC) $(CFLAGS) -c $<

st_parser.tab.o: st_parser.tab.c ast.h vm.h
	$(CC) $(CFLAGS) -c $<

# 使用 Bison 生成语法分析器
st_parser.tab.c st_parser.tab.h: st_parser.y
	$(BISON) -d $<

# 使用 Flex 生成词法分析器
st_lexer.c: st_lexer.lex st_parser.tab.h
	$(FLEX) -o $@ $<

# 清理生成的文件
clean:
	rm -f $(OBJS) $(TARGET) $(GENERATED)

# 安装到系统
install: $(TARGET)
	@echo "安装 $(TARGET) 到 /usr/local/bin/"
	sudo cp $(TARGET) /usr/local/bin/
	@echo "安装完成！"

# 测试程序
test: $(TARGET)
	@echo "创建测试文件..."
	@echo 'PROGRAM TestProgram' > test.st
	@echo 'VAR' >> test.st
	@echo '    i : INT;' >> test.st
	@echo '    sum : INT;' >> test.st
	@echo 'END_VAR' >> test.st
	@echo '' >> test.st
	@echo 'sum := 0;' >> test.st
	@echo 'FOR i := 1 TO 10 DO' >> test.st
	@echo '    sum := sum + i;' >> test.st
	@echo 'END_FOR' >> test.st
	@echo 'END_PROGRAM' >> test.st
	@echo ""
	@echo "测试文件内容:"
	@cat test.st
	@echo ""
	@echo "运行测试:"
	./$(TARGET) test.st
	@echo ""
	@echo "测试完成！"

# 帮助信息
help:
	@echo "IEC61131 结构化文本编译器构建系统"
	@echo ""
	@echo "可用目标:"
	@echo "  all      - 构建编译器 (默认)"
	@echo "  clean    - 清理生成的文件"
	@echo "  install  - 安装到系统路径"
	@echo "  test     - 运行测试程序"
	@echo "  help     - 显示此帮助信息"
	@echo ""
	@echo "依赖："
	@echo "  - flex   (词法分析器生成器)"
	@echo "  - bison  (语法分析器生成器)"
	@echo "  - gcc    (C编译器)"
	@echo ""
	@echo "使用示例："
	@echo "  make           # 构建编译器"
	@echo "  make test      # 运行测试"
	@echo "  make clean     # 清理文件"