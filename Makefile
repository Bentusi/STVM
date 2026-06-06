# Makefile for STVM - ST Language Compiler and Virtual Machine
# Phase 1, 2 & 3: Infrastructure + Data Structures + Compiler Frontend

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I./src/include -I./build -D_POSIX_C_SOURCE=200809L 
LDFLAGS = -lm -lc

# Flex and Bison
LEX = flex
YACC = bison
YFLAGS = -d -v

# Debug flags
DEBUG_FLAGS = -g -DDEBUG -O0
RELEASE_FLAGS = -O2 -DNDEBUG

# Default to debug build
CFLAGS += $(DEBUG_FLAGS)

# Directories
SRC_DIR = src
INCLUDE_DIR = $(SRC_DIR)/include
CORE_DIR = $(SRC_DIR)/core
TESTS_DIR = tests
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin

# Source files (exclude .l and .y files, they're generated)
CORE_SRCS = $(filter-out $(CORE_DIR)/st_lexer.c $(CORE_DIR)/st_parser.c,$(wildcard $(CORE_DIR)/*.c))
CORE_OBJS = $(patsubst $(CORE_DIR)/%.c,$(OBJ_DIR)/%.o,$(CORE_SRCS))

# Test source files
TEST_SRCS = $(wildcard $(TESTS_DIR)/*.c)

# Generated parser files
PARSER_C = $(BUILD_DIR)/st_parser.tab.c
PARSER_H = $(BUILD_DIR)/st_parser.tab.h
LEXER_C = $(BUILD_DIR)/lex.yy.c
PARSER_OBJ = $(OBJ_DIR)/st_parser.tab.o
LEXER_OBJ = $(OBJ_DIR)/lex.yy.o

# Header files
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)

# Targets
.PHONY: all clean dirs test release parser stvm help doc

all: stvm

# Create necessary directories
dirs:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

# Generate parser and lexer
parser: $(PARSER_C) $(LEXER_C)

$(PARSER_C) $(PARSER_H): $(CORE_DIR)/st_parser.y | dirs
	@echo "Generating parser..."
	$(YACC) $(YFLAGS) -o $(PARSER_C) $<

$(LEXER_C): $(CORE_DIR)/st_lexer.l $(PARSER_H) | dirs
	@echo "Generating lexer..."
	$(LEX) -o $(LEXER_C) $<

# Compile generated parser and lexer
$(PARSER_OBJ): $(PARSER_C) $(PARSER_H) $(HEADERS) | dirs
	@echo "Compiling parser..."
	$(CC) $(CFLAGS) -Wno-unused-function -c $(PARSER_C) -o $@

$(LEXER_OBJ): $(LEXER_C) $(PARSER_H) $(HEADERS) | dirs
	@echo "Compiling lexer..."
	$(CC) $(CFLAGS) -Wno-unused-function -D_POSIX_C_SOURCE=200809L -c $(LEXER_C) -o $@

# Compile object files with proper dependencies
$(OBJ_DIR)/%.o: $(CORE_DIR)/%.c $(HEADERS) | dirs
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Test programs with proper dependencies
test_mmgr: $(BIN_DIR)/test_mmgr

$(BIN_DIR)/test_mmgr: $(TESTS_DIR)/test_mmgr.c $(OBJ_DIR)/mmgr.o $(OBJ_DIR)/types.o | dirs
	@echo "Building test_mmgr..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_types: $(BIN_DIR)/test_types

$(BIN_DIR)/test_types: $(TESTS_DIR)/test_types.c $(OBJ_DIR)/mmgr.o $(OBJ_DIR)/types.o | dirs
	@echo "Building test_types..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_bytecode: $(BIN_DIR)/test_bytecode

$(BIN_DIR)/test_bytecode: $(TESTS_DIR)/test_bytecode.c $(filter-out $(OBJ_DIR)/main.o,$(CORE_OBJS)) $(PARSER_OBJ) $(LEXER_OBJ) | dirs
	@echo "Building test_bytecode..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_ast: $(BIN_DIR)/test_ast

$(BIN_DIR)/test_ast: $(TESTS_DIR)/test_ast.c $(filter-out $(OBJ_DIR)/main.o,$(CORE_OBJS)) $(PARSER_OBJ) $(LEXER_OBJ) | dirs
	@echo "Building test_ast..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_symtbl: $(BIN_DIR)/test_symtbl

$(BIN_DIR)/test_symtbl: $(TESTS_DIR)/test_symtbl.c $(filter-out $(OBJ_DIR)/main.o,$(CORE_OBJS)) | dirs
	@echo "Building test_symtbl..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_parser: $(BIN_DIR)/test_parser

$(BIN_DIR)/test_parser: $(TESTS_DIR)/test_parser.c $(filter-out $(OBJ_DIR)/main.o,$(CORE_OBJS)) $(PARSER_OBJ) $(LEXER_OBJ) | dirs
	@echo "Building test_parser..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_codegen: $(BIN_DIR)/test_codegen

$(BIN_DIR)/test_codegen: $(TESTS_DIR)/test_codegen.c $(filter-out $(OBJ_DIR)/main.o,$(CORE_OBJS)) $(PARSER_OBJ) $(LEXER_OBJ) | dirs
	@echo "Building test_codegen..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_vm: $(BIN_DIR)/test_vm

$(BIN_DIR)/test_vm: $(TESTS_DIR)/test_vm.c $(filter-out $(OBJ_DIR)/main.o,$(CORE_OBJS)) | dirs
	@echo "Building test_vm..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_libmgr: $(BIN_DIR)/test_libmgr

$(BIN_DIR)/test_libmgr: $(TESTS_DIR)/test_libmgr.c $(filter-out $(OBJ_DIR)/main.o,$(CORE_OBJS)) | dirs
	@echo "Building test_libmgr..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_bitops: $(BIN_DIR)/test_bitops

$(BIN_DIR)/test_bitops: $(TESTS_DIR)/test_bitops.c $(filter-out $(OBJ_DIR)/main.o,$(CORE_OBJS)) | dirs
	@echo "Building test_bitops..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_hotreload: $(BIN_DIR)/test_hotreload

$(BIN_DIR)/test_hotreload: $(TESTS_DIR)/test_hotreload.c $(filter-out $(OBJ_DIR)/main.o,$(CORE_OBJS)) $(PARSER_OBJ) $(LEXER_OBJ) | dirs
	@echo "Building test_hotreload..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_io_manager: $(BIN_DIR)/test_io_manager

$(BIN_DIR)/test_io_manager: $(TESTS_DIR)/test_io_manager_simple.c $(OBJ_DIR)/iomgr.o $(OBJ_DIR)/io_adapter_sim.o $(OBJ_DIR)/mmgr.o $(OBJ_DIR)/types.o | dirs
	@echo "Building test_io_manager..."
	$(CC) $(CFLAGS) -pthread -o $@ $^ $(LDFLAGS)

test_io_integration: $(BIN_DIR)/test_io_integration

$(BIN_DIR)/test_io_integration: $(TESTS_DIR)/test_io_integration.c $(filter-out $(OBJ_DIR)/main.o,$(CORE_OBJS)) $(PARSER_OBJ) $(LEXER_OBJ) | dirs
	@echo "Building test_io_integration..."
	$(CC) $(CFLAGS) -pthread -o $@ $^ $(LDFLAGS)

test_hotreload_integration: $(BIN_DIR)/test_hotreload_integration

$(BIN_DIR)/test_hotreload_integration: $(TESTS_DIR)/test_hotreload_integration.c $(filter-out $(OBJ_DIR)/main.o,$(CORE_OBJS)) $(PARSER_OBJ) $(LEXER_OBJ) | dirs
	@echo "Building test_hotreload_integration..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_force: $(BIN_DIR)/test_force

$(BIN_DIR)/test_force: $(TESTS_DIR)/test_force.c $(OBJ_DIR)/force.o $(OBJ_DIR)/mmgr.o $(OBJ_DIR)/types.o | dirs
	@echo "Building test_force..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_force_file: $(BIN_DIR)/test_force_file

$(BIN_DIR)/test_force_file: $(TESTS_DIR)/test_force_file.c $(OBJ_DIR)/force.o $(OBJ_DIR)/mmgr.o $(OBJ_DIR)/types.o | dirs
	@echo "Building test_force_file..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_force_quality: $(BIN_DIR)/test_force_quality

$(BIN_DIR)/test_force_quality: $(TESTS_DIR)/test_force_quality.c $(OBJ_DIR)/force.o $(OBJ_DIR)/mmgr.o $(OBJ_DIR)/types.o | dirs
	@echo "Building test_force_quality..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_wcet: $(BIN_DIR)/test_wcet

$(BIN_DIR)/test_wcet: $(TESTS_DIR)/test_wcet.c $(OBJ_DIR)/wcet.o $(OBJ_DIR)/mmgr.o $(OBJ_DIR)/types.o $(OBJ_DIR)/bytecode.o $(OBJ_DIR)/bytecode_io.o | dirs
	@echo "Building test_wcet..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Main programs
stvm: $(BIN_DIR)/stvm

$(BIN_DIR)/stvm: $(CORE_OBJS) $(PARSER_OBJ) $(LEXER_OBJ) | dirs
	@echo "Building stvm (compiler and VM)..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Release build
release: CFLAGS := $(filter-out $(DEBUG_FLAGS),$(CFLAGS))
release: CFLAGS += $(RELEASE_FLAGS)
release: clean all

# Run all tests
test: test_mmgr test_types test_bytecode test_ast test_symtbl test_parser test_codegen test_vm test_libmgr test_bitops test_hotreload test_io_manager
	@echo ""
	@echo "=== Running Memory Manager Tests ==="
	@./$(BIN_DIR)/test_mmgr
	@echo ""
	@echo "=== Running Types Tests ==="
	@-./$(BIN_DIR)/test_types
	@echo ""
	@echo "=== Running Bytecode Tests ==="
	@./$(BIN_DIR)/test_bytecode
	@echo ""
	@echo "=== Running AST Tests ==="
	@./$(BIN_DIR)/test_ast
	@echo ""
	@echo "=== Running Symbol Table Tests ==="
	@./$(BIN_DIR)/test_symtbl
	@echo ""
	@echo "=== Running Parser and Type Checker Tests ==="
	@-./$(BIN_DIR)/test_parser
	@echo ""
	@echo "=== Running Code Generator Tests ==="
	@./$(BIN_DIR)/test_codegen
	@echo ""
	@echo "=== Running VM Tests ==="
	@./$(BIN_DIR)/test_vm
	@echo ""
	@echo "=== Running Library Manager Tests ==="
	@./$(BIN_DIR)/test_libmgr
	@echo ""
	@echo "=== Running Bitwise Operations Tests ==="
	@./$(BIN_DIR)/test_bitops
	@echo ""
	@echo "=== Running Hot Reload Tests ==="
	@if [ -f examples/hotreload_v1.stbc ] && [ -f examples/hotreload_v2.stbc ]; then \
		./$(BIN_DIR)/test_hotreload examples/hotreload_v1.stbc examples/hotreload_v2.stbc; \
	else \
		echo "Skipping hot reload tests (bytecode files not found)"; \
		echo "Run: ./build/bin/stvm -c examples/hotreload_v1.st -o examples/hotreload_v1.stbc"; \
		echo "     ./build/bin/stvm -c examples/hotreload_v2.st -o examples/hotreload_v2.stbc"; \
	fi
	@echo ""
	@echo "=== Running I/O Manager Tests ==="
	@./$(BIN_DIR)/test_io_manager
	@echo ""
	@echo "=== Running ST Examples Test ==="
	@./test_examples_enhanced.sh

doc:
	@echo ""
	@echo "=== Generating Documentation ==="
	@pandoc STVM语法说明.md -o STVM语法说明.html

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)
	@rm -f $(CORE_DIR)/st_parser.tab.* $(CORE_DIR)/lex.yy.c $(CORE_DIR)/st_parser.output

# Help
help:
	@echo "STVM Makefile - Available targets:"
	@echo ""
	@echo "Main programs:"
	@echo "  stvm          - Build STVM compiler and virtual machine"
	@echo ""
	@echo "Build targets:"
	@echo "  all           - Build all programs (default)"
	@echo "  parser        - Generate lexer and parser from Flex/Bison"
	@echo "  release       - Build optimized release version"
	@echo "  clean         - Remove build artifacts"
	@echo ""
	@echo "Test programs:"
	@echo "  test_mmgr     - Build memory manager test"
	@echo "  test_types    - Build types test"
	@echo "  test_bytecode - Build bytecode test"
	@echo "  test_ast      - Build AST test"
	@echo "  test_symtbl   - Build symbol table test"
	@echo "  test_parser   - Build parser and type checker test"
	@echo "  test_codegen  - Build code generator test"
	@echo "  test_vm       - Build VM test"
	@echo "  test_libmgr   - Build library manager test"
	@echo "  test_bitops   - Build bitwise operations test"
	@echo "  test_hotreload- Build hot reload test"
	@echo "  test_io_manager - Build I/O manager test"
	@echo "  test_wcet    - Build WCET analyzer test"
	@echo "  test          - Build and run all tests"
	@echo ""
	@echo "Usage examples:"
	@echo "  make stvm                           # Build STVM"
	@echo "  ./build/bin/stvm -c examples/hello.st   # Compile"
	@echo "  ./build/bin/stvm -r examples/hello.stbc # Run"
	@echo "  ./build/bin/stvm -r hello.stbc -d       # Debug"
	@echo "  ./build/bin/stvm -i                     # REPL"
	@echo "  ./build/bin/stvm --help                 # Show help"
