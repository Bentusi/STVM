# Makefile for STVM - ST Language Compiler and Virtual Machine
# Phase 1, 2 & 3: Infrastructure + Data Structures + Compiler Frontend

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I./src/include -I./build
LDFLAGS =

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
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin

# Source files (exclude .l and .y files, they're generated)
CORE_SRCS = $(filter-out $(CORE_DIR)/st_lexer.c $(CORE_DIR)/st_parser.c,$(wildcard $(CORE_DIR)/*.c))
CORE_OBJS = $(patsubst $(CORE_DIR)/%.c,$(OBJ_DIR)/%.o,$(CORE_SRCS))

# Generated parser files
PARSER_C = $(BUILD_DIR)/st_parser.tab.c
PARSER_H = $(BUILD_DIR)/st_parser.tab.h
LEXER_C = $(BUILD_DIR)/lex.yy.c
PARSER_OBJ = $(OBJ_DIR)/st_parser.tab.o
LEXER_OBJ = $(OBJ_DIR)/lex.yy.o

# Header files
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)

# Targets
.PHONY: all clean dirs test release parser

all: dirs parser test_mmgr test_types test_bytecode test_ast test_symtbl test_parser

# Create necessary directories
dirs:
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)

# Generate parser and lexer
parser: dirs $(PARSER_C) $(LEXER_C)

$(PARSER_C) $(PARSER_H): $(CORE_DIR)/st_parser.y
	@echo "Generating parser..."
	$(YACC) $(YFLAGS) -o $(PARSER_C) $<

$(LEXER_C): $(CORE_DIR)/st_lexer.l $(PARSER_H)
	@echo "Generating lexer..."
	$(LEX) -o $(LEXER_C) $<

# Compile generated parser and lexer
$(PARSER_OBJ): $(PARSER_C) $(HEADERS)
	@echo "Compiling parser..."
	$(CC) $(CFLAGS) -Wno-unused-function -c $< -o $@

$(LEXER_OBJ): $(LEXER_C) $(HEADERS)
	@echo "Compiling lexer..."
	$(CC) $(CFLAGS) -Wno-unused-function -c $< -o $@

# Compile object files
$(OBJ_DIR)/%.o: $(CORE_DIR)/%.c $(HEADERS)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Test programs
test_mmgr: dirs $(OBJ_DIR)/mmgr.o $(OBJ_DIR)/types.o
	@echo "Building test_mmgr..."
	$(CC) $(CFLAGS) -o $(BIN_DIR)/test_mmgr tests/test_mmgr.c $(OBJ_DIR)/mmgr.o $(OBJ_DIR)/types.o $(LDFLAGS)

test_types: dirs $(OBJ_DIR)/mmgr.o $(OBJ_DIR)/types.o
	@echo "Building test_types..."
	$(CC) $(CFLAGS) -o $(BIN_DIR)/test_types tests/test_types.c $(OBJ_DIR)/mmgr.o $(OBJ_DIR)/types.o $(LDFLAGS)

test_bytecode: dirs $(CORE_OBJS)
	@echo "Building test_bytecode..."
	$(CC) $(CFLAGS) -o $(BIN_DIR)/test_bytecode tests/test_bytecode.c $(CORE_OBJS) $(LDFLAGS)

test_ast: dirs $(CORE_OBJS)
	@echo "Building test_ast..."
	$(CC) $(CFLAGS) -o $(BIN_DIR)/test_ast tests/test_ast.c $(CORE_OBJS) $(LDFLAGS)

test_symtbl: dirs $(CORE_OBJS)
	@echo "Building test_symtbl..."
	$(CC) $(CFLAGS) -o $(BIN_DIR)/test_symtbl tests/test_symtbl.c $(CORE_OBJS) $(LDFLAGS)

test_parser: dirs parser $(CORE_OBJS) $(PARSER_OBJ) $(LEXER_OBJ)
	@echo "Building test_parser..."
	$(CC) $(CFLAGS) -o $(BIN_DIR)/test_parser tests/test_parser.c $(CORE_OBJS) $(PARSER_OBJ) $(LEXER_OBJ) $(LDFLAGS)

# Release build
release: CFLAGS := $(filter-out $(DEBUG_FLAGS),$(CFLAGS))
release: CFLAGS += $(RELEASE_FLAGS)
release: clean all

# Run tests
test: test_mmgr test_types test_bytecode test_ast test_symtbl test_parser
	@echo "\n=== Running Memory Manager Tests ==="
	@./$(BIN_DIR)/test_mmgr
	@echo "\n=== Running Types Tests ==="
	@-./$(BIN_DIR)/test_types
	@echo "\n=== Running Bytecode Tests ==="
	@./$(BIN_DIR)/test_bytecode
	@echo "\n=== Running AST Tests ==="
	@./$(BIN_DIR)/test_ast
	@echo "\n=== Running Symbol Table Tests ==="
	@./$(BIN_DIR)/test_symtbl
	@echo "\n=== Running Parser and Type Checker Tests ==="
	@./$(BIN_DIR)/test_parser

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)
	@rm -f $(CORE_DIR)/st_parser.tab.* $(CORE_DIR)/lex.yy.c $(CORE_DIR)/st_parser.output

# Help
help:
	@echo "STVM Makefile - Available targets:"
	@echo "  all           - Build all test programs (default)"
	@echo "  parser        - Generate lexer and parser from Flex/Bison"
	@echo "  test_mmgr     - Build memory manager test"
	@echo "  test_types    - Build types test"
	@echo "  test_bytecode - Build bytecode test"
	@echo "  test_ast      - Build AST test"
	@echo "  test_symtbl   - Build symbol table test"
	@echo "  test_parser   - Build parser and type checker test"
	@echo "  test          - Build and run all tests"
	@echo "  release       - Build optimized release version"
	@echo "  clean         - Remove build artifacts"
	@echo "  help          - Show this help message"
