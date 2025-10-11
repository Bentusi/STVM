/**
 * @file test_parser.c
 * @brief 语法分析器测试（使用Flex/Bison）
 */

#include <stdio.h>
#include <assert.h>
#include "ast.h"
#include "mmgr.h"
#include "typecheck.h"
#include "symtbl.h"

/* 外部声明 */
extern ASTNode* parse_string(const char* source);
extern ASTNode* parse_file(const char* filename);

void test_simple_program(void) {
    printf("Testing simple program...\n");
    
    const char* source =
        "PROGRAM Test\n"
        "VAR\n"
        "  x : INT;\n"
        "END_VAR\n"
        "  x := 42;\n"
        "END_PROGRAM\n";
    
    ASTNode* program = parse_string(source);
    assert(program != NULL);
    assert(program->type == AST_PROGRAM);
    assert(program->data.program.var_decls != NULL);
    assert(program->data.program.body != NULL);
    
    printf("  Simple program test passed ✓\n");
    
    ast_free(program);
}

void test_expressions(void) {
    printf("Testing expressions...\n");
    
    const char* source =
        "PROGRAM Test\n"
        "VAR\n"
        "  result : INT;\n"
        "END_VAR\n"
        "  result := 1 + 2 * 3;\n"
        "END_PROGRAM\n";
    
    ASTNode* program = parse_string(source);
    assert(program != NULL);
    
    // 检查表达式结构
    ASTNode* stmt = program->data.program.body;
    assert(stmt->type == AST_ASSIGNMENT);
    assert(stmt->data.assignment.value->type == AST_BINARY_OP);
    
    printf("  Expressions test passed ✓\n");
    
    ast_free(program);
}

void test_if_statement(void) {
    printf("Testing if statement...\n");
    
    const char* source =
        "PROGRAM Test\n"
        "VAR\n"
        "  x : INT;\n"
        "  y : INT;\n"
        "END_VAR\n"
        "  IF x > 0 THEN\n"
        "    y := 1;\n"
        "  ELSE\n"
        "    y := 0;\n"
        "  END_IF\n"
        "END_PROGRAM\n";
    
    ASTNode* program = parse_string(source);
    assert(program != NULL);
    
    ASTNode* stmt = program->data.program.body;
    assert(stmt->type == AST_IF_STMT);
    assert(stmt->data.if_stmt.condition != NULL);
    assert(stmt->data.if_stmt.then_branch != NULL);
    assert(stmt->data.if_stmt.else_branch != NULL);
    
    printf("  If statement test passed ✓\n");
    
    ast_free(program);
}

void test_while_loop(void) {
    printf("Testing while loop...\n");
    
    const char* source =
        "PROGRAM Test\n"
        "VAR\n"
        "  x : INT := 0;\n"
        "END_VAR\n"
        "  WHILE x < 10 DO\n"
        "    x := x + 1;\n"
        "  END_WHILE\n"
        "END_PROGRAM\n";
    
    ASTNode* program = parse_string(source);
    assert(program != NULL);
    
    ASTNode* stmt = program->data.program.body;
    assert(stmt->type == AST_WHILE_STMT);
    assert(stmt->data.while_stmt.condition != NULL);
    assert(stmt->data.while_stmt.body != NULL);
    
    printf("  While loop test passed ✓\n");
    
    ast_free(program);
}

void test_for_loop(void) {
    printf("Testing for loop...\n");
    
    const char* source =
        "PROGRAM Test\n"
        "VAR\n"
        "  i : INT;\n"
        "  sum : INT := 0;\n"
        "END_VAR\n"
        "  FOR i := 1 TO 10 DO\n"
        "    sum := sum + i;\n"
        "  END_FOR\n"
        "END_PROGRAM\n";
    
    ASTNode* program = parse_string(source);
    assert(program != NULL);
    
    ASTNode* stmt = program->data.program.body;
    assert(stmt->type == AST_FOR_STMT);
    assert(stmt->data.for_stmt.init != NULL);
    assert(stmt->data.for_stmt.end_value != NULL);
    assert(stmt->data.for_stmt.body != NULL);
    
    printf("  For loop test passed ✓\n");
    
    ast_free(program);
}

void test_function_declaration(void) {
    printf("Testing function declaration...\n");
    
    const char* source =
        "PROGRAM Test\n"
        "FUNCTION Add\n"
        "VAR_INPUT\n"
        "  a : INT;\n"
        "  b : INT;\n"
        "END_VAR\n"
        ": INT\n"
        "  RETURN a + b;\n"
        "END_FUNCTION\n"
        "END_PROGRAM\n";
    
    ASTNode* program = parse_string(source);
    assert(program != NULL);
    
    ASTNode* func = program->data.program.functions;
    assert(func != NULL);
    assert(func->type == AST_FUNCTION_DECL);
    assert(func->data.function_decl.params != NULL);
    assert(func->data.function_decl.return_type != NULL);
    assert(func->data.function_decl.body != NULL);
    
    printf("  Function declaration test passed ✓\n");
    
    ast_free(program);
}

void test_typecheck(void) {
    printf("Testing type checking...\n");
    
    const char* source =
        "PROGRAM Test\n"
        "VAR\n"
        "  x : INT;\n"
        "  y : INT;\n"
        "END_VAR\n"
        "  x := 42;\n"
        "  y := x + 10;\n"
        "END_PROGRAM\n";
    
    ASTNode* program = parse_string(source);
    assert(program != NULL);
    
    SymbolTable* symtbl = symtbl_create();
    assert(symtbl != NULL);
    
    TypeChecker checker;
    assert(typecheck_init(&checker, symtbl) == ERR_OK);
    
    ErrorCode err = typecheck_program(&checker, program);
    assert(err == ERR_OK);
    assert(!checker.had_error);
    
    printf("  Type checking test passed ✓\n");
    
    typecheck_cleanup(&checker);
    symtbl_destroy(symtbl);
    ast_free(program);
}

void test_type_error_detection(void) {
    printf("Testing type error detection...\n");
    
    const char* source =
        "PROGRAM Test\n"
        "VAR\n"
        "  x : INT;\n"
        "  b : BOOL;\n"
        "END_VAR\n"
        "  x := TRUE;\n"  // 类型不匹配
        "END_PROGRAM\n";
    
    ASTNode* program = parse_string(source);
    assert(program != NULL);
    
    SymbolTable* symtbl = symtbl_create();
    assert(symtbl != NULL);
    
    TypeChecker checker;
    assert(typecheck_init(&checker, symtbl) == ERR_OK);
    
    typecheck_program(&checker, program);
    assert(checker.had_error);  // 应该检测到错误
    
    printf("  Type error detection test passed ✓\n");
    
    typecheck_cleanup(&checker);
    symtbl_destroy(symtbl);
    ast_free(program);
}

int main(void) {
    printf("=== Parser and Type Checker Tests (Flex/Bison) ===\n\n");
    
    // 初始化内存管理器
    assert(mmgr_init() == ERR_OK);
    
    test_simple_program();
    test_expressions();
    test_if_statement();
    test_while_loop();
    test_for_loop();
    test_function_declaration();
    test_typecheck();
    test_type_error_detection();
    
    // 清理内存管理器
    mmgr_cleanup();
    
    printf("\nAll Tests Passed! ✓\n");
    
    return 0;
}
