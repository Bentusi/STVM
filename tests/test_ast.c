/**
 * @file test_ast.c
 * @brief AST模块测试程序
 */

#include "ast.h"
#include "mmgr.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_literal_nodes(void) {
    printf("\n--- Test: Literal Nodes ---\n");
    
    // 整数字面量
    Value int_val = value_create(TYPE_INT);
    int_val.int_val = 42;
    ASTNode* int_node = ast_create_literal(int_val);
    assert(int_node != NULL);
    assert(int_node->type == AST_LITERAL);
    assert(int_node->data.literal.value.type == TYPE_INT);
    assert(int_node->data.literal.value.int_val == 42);
    printf("✓ Created INT literal: 42\n");
    
    // 实数字面量
    Value real_val = value_create(TYPE_REAL);
    real_val.real_val = 3.14;
    ASTNode* real_node = ast_create_literal(real_val);
    assert(real_node->data.literal.value.type == TYPE_REAL);
    assert(real_node->data.literal.value.real_val == 3.14);
    printf("✓ Created REAL literal: 3.14\n");
    
    // 布尔字面量
    Value bool_val = value_create(TYPE_BOOL);
    bool_val.bool_val = true;
    ASTNode* bool_node = ast_create_literal(bool_val);
    assert(bool_node->data.literal.value.type == TYPE_BOOL);
    assert(bool_node->data.literal.value.bool_val == true);
    printf("✓ Created BOOL literal: true\n");
    
    ast_free_node(int_node);
    ast_free_node(real_node);
    ast_free_node(bool_node);
}

void test_identifier_nodes(void) {
    printf("\n--- Test: Identifier Nodes ---\n");
    
    ASTNode* id = ast_create_identifier("myVar");
    assert(id != NULL);
    assert(id->type == AST_IDENTIFIER);
    assert(strcmp(id->data.identifier.name, "myVar") == 0);
    printf("✓ Created identifier: %s\n", id->data.identifier.name);
    
    ast_free_node(id);
}

void test_binary_operations(void) {
    printf("\n--- Test: Binary Operations ---\n");
    
    // 创建 2 + 3
    Value v2 = value_create(TYPE_INT);
    v2.int_val = 2;
    ASTNode* left = ast_create_literal(v2);
    
    Value v3 = value_create(TYPE_INT);
    v3.int_val = 3;
    ASTNode* right = ast_create_literal(v3);
    
    ASTNode* add = ast_create_binary_op(BINOP_ADD, left, right);
    assert(add != NULL);
    assert(add->type == AST_BINARY_OP);
    assert(add->data.binary_op.op == BINOP_ADD);
    assert(add->data.binary_op.left == left);
    assert(add->data.binary_op.right == right);
    printf("✓ Created binary operation: 2 + 3\n");
    
    // 测试运算符字符串
    assert(strcmp(binary_op_to_string(BINOP_ADD), "+") == 0);
    assert(strcmp(binary_op_to_string(BINOP_MUL), "*") == 0);
    assert(strcmp(binary_op_to_string(BINOP_EQ), "=") == 0);
    printf("✓ Binary operator strings correct\n");
    
    ast_free_node(add);
}

void test_unary_operations(void) {
    printf("\n--- Test: Unary Operations ---\n");
    
    // 创建 -5
    Value v5 = value_create(TYPE_INT);
    v5.int_val = 5;
    ASTNode* operand = ast_create_literal(v5);
    
    ASTNode* neg = ast_create_unary_op(UNOP_NEG, operand);
    assert(neg != NULL);
    assert(neg->type == AST_UNARY_OP);
    assert(neg->data.unary_op.op == UNOP_NEG);
    assert(neg->data.unary_op.operand == operand);
    printf("✓ Created unary operation: -5\n");
    
    // 测试运算符字符串
    assert(strcmp(unary_op_to_string(UNOP_NEG), "-") == 0);
    assert(strcmp(unary_op_to_string(UNOP_NOT), "NOT") == 0);
    printf("✓ Unary operator strings correct\n");
    
    ast_free_node(neg);
}

void test_variable_declaration(void) {
    printf("\n--- Test: Variable Declaration ---\n");
    
    TypeInfo* type = type_info_create(TYPE_INT);
    
    // 无初始化器的声明
    ASTNode* var_decl = ast_create_var_decl("count", type, NULL, false);
    assert(var_decl != NULL);
    assert(var_decl->type == AST_VAR_DECL);
    assert(strcmp(var_decl->data.var_decl.name, "count") == 0);
    assert(var_decl->data.var_decl.type == type);
    assert(var_decl->data.var_decl.initializer == NULL);
    assert(var_decl->data.var_decl.is_const == false);
    printf("✓ Created variable declaration: count : INT\n");
    
    // 带初始化器的声明
    Value init_val = value_create(TYPE_INT);
    init_val.int_val = 10;
    ASTNode* initializer = ast_create_literal(init_val);
    
    TypeInfo* type2 = type_info_create(TYPE_INT);
    ASTNode* var_decl2 = ast_create_var_decl("total", type2, initializer, true);
    assert(var_decl2->data.var_decl.initializer == initializer);
    assert(var_decl2->data.var_decl.is_const == true);
    printf("✓ Created constant declaration: total : INT := 10\n");
    
    ast_free_node(var_decl);
    ast_free_node(var_decl2);
    
    // 释放我们创建的类型
    type_info_free(type);
    type_info_free(type2);
}

void test_assignment(void) {
    printf("\n--- Test: Assignment Statement ---\n");
    
    ASTNode* target = ast_create_identifier("x");
    
    Value val = value_create(TYPE_INT);
    val.int_val = 100;
    ASTNode* value = ast_create_literal(val);
    
    ASTNode* assign = ast_create_assign(target, value);
    assert(assign != NULL);
    assert(assign->type == AST_ASSIGN);
    assert(assign->data.assign.target == target);
    assert(assign->data.assign.value == value);
    printf("✓ Created assignment: x := 100\n");
    
    ast_free_node(assign);
}

void test_if_statement(void) {
    printf("\n--- Test: If Statement ---\n");
    
    // 条件：x > 0
    ASTNode* x = ast_create_identifier("x");
    Value zero = value_create(TYPE_INT);
    zero.int_val = 0;
    ASTNode* zero_lit = ast_create_literal(zero);
    ASTNode* condition = ast_create_binary_op(BINOP_GT, x, zero_lit);
    
    // then分支：y := 1
    ASTNode* then_target = ast_create_identifier("y");
    Value one = value_create(TYPE_INT);
    one.int_val = 1;
    ASTNode* one_lit = ast_create_literal(one);
    ASTNode* then_branch = ast_create_assign(then_target, one_lit);
    
    // else分支：y := -1
    ASTNode* else_target = ast_create_identifier("y");
    Value minus_one = value_create(TYPE_INT);
    minus_one.int_val = -1;
    ASTNode* minus_one_lit = ast_create_literal(minus_one);
    ASTNode* else_branch = ast_create_assign(else_target, minus_one_lit);
    
    ASTNode* if_stmt = ast_create_if(condition, then_branch, else_branch);
    assert(if_stmt != NULL);
    assert(if_stmt->type == AST_IF);
    assert(if_stmt->data.if_stmt.condition == condition);
    assert(if_stmt->data.if_stmt.then_branch == then_branch);
    assert(if_stmt->data.if_stmt.else_branch == else_branch);
    printf("✓ Created if statement with then and else branches\n");
    
    ast_free_node(if_stmt);
}

void test_while_loop(void) {
    printf("\n--- Test: While Loop ---\n");
    
    // 条件：i < 10
    ASTNode* i = ast_create_identifier("i");
    Value ten = value_create(TYPE_INT);
    ten.int_val = 10;
    ASTNode* ten_lit = ast_create_literal(ten);
    ASTNode* condition = ast_create_binary_op(BINOP_LT, i, ten_lit);
    
    // 循环体：i := i + 1
    ASTNode* i2 = ast_create_identifier("i");
    ASTNode* i3 = ast_create_identifier("i");
    Value one = value_create(TYPE_INT);
    one.int_val = 1;
    ASTNode* one_lit = ast_create_literal(one);
    ASTNode* inc = ast_create_binary_op(BINOP_ADD, i3, one_lit);
    ASTNode* body = ast_create_assign(i2, inc);
    
    ASTNode* while_loop = ast_create_while(condition, body);
    assert(while_loop != NULL);
    assert(while_loop->type == AST_WHILE);
    assert(while_loop->data.while_stmt.condition == condition);
    assert(while_loop->data.while_stmt.body == body);
    printf("✓ Created while loop\n");
    
    ast_free_node(while_loop);
}

void test_for_loop(void) {
    printf("\n--- Test: For Loop ---\n");
    
    // FOR i := 1 TO 10 BY 2 DO ...
    Value start_val = value_create(TYPE_INT);
    start_val.int_val = 1;
    ASTNode* start = ast_create_literal(start_val);
    
    Value end_val = value_create(TYPE_INT);
    end_val.int_val = 10;
    ASTNode* end = ast_create_literal(end_val);
    
    Value step_val = value_create(TYPE_INT);
    step_val.int_val = 2;
    ASTNode* step = ast_create_literal(step_val);
    
    ASTNode* body = ast_create_identifier("dummy"); // 简化的循环体
    
    ASTNode* for_loop = ast_create_for("i", start, end, step, body);
    assert(for_loop != NULL);
    assert(for_loop->type == AST_FOR);
    assert(strcmp(for_loop->data.for_stmt.variable, "i") == 0);
    assert(for_loop->data.for_stmt.start == start);
    assert(for_loop->data.for_stmt.end == end);
    assert(for_loop->data.for_stmt.step == step);
    printf("✓ Created for loop: FOR i := 1 TO 10 BY 2\n");
    
    ast_free_node(for_loop);
}

void test_function_call(void) {
    printf("\n--- Test: Function Call ---\n");
    
    // 创建参数
    Value arg1_val = value_create(TYPE_INT);
    arg1_val.int_val = 5;
    ASTNode* arg1 = ast_create_literal(arg1_val);
    
    Value arg2_val = value_create(TYPE_INT);
    arg2_val.int_val = 10;
    ASTNode* arg2 = ast_create_literal(arg2_val);
    
    ASTNode** args = (ASTNode**)mmgr_alloc(sizeof(ASTNode*) * 2);
    args[0] = arg1;
    args[1] = arg2;
    
    ASTNode* call = ast_create_function_call("add", args, 2);
    assert(call != NULL);
    assert(call->type == AST_FUNCTION_CALL);
    assert(strcmp(call->data.function_call.name, "add") == 0);
    assert(call->data.function_call.arg_count == 2);
    assert(call->data.function_call.arguments[0] == arg1);
    assert(call->data.function_call.arguments[1] == arg2);
    printf("✓ Created function call: add(5, 10)\n");
    
    ast_free_node(call);
}

void test_node_list(void) {
    printf("\n--- Test: Node List ---\n");
    
    ASTNode* node1 = ast_create_identifier("a");
    ASTNode* node2 = ast_create_identifier("b");
    ASTNode* node3 = ast_create_identifier("c");
    
    ASTNode* list = ast_append_node(NULL, node1);
    list = ast_append_node(list, node2);
    list = ast_append_node(list, node3);
    
    assert(list == node1);
    assert(list->next == node2);
    assert(list->next->next == node3);
    assert(list->next->next->next == NULL);
    printf("✓ Created node list: a -> b -> c\n");
    
    ast_free_node(list);
}

void test_ast_print(void) {
    printf("\n--- Test: AST Print ---\n");
    
    // 创建一个简单的表达式树：(2 + 3) * 4
    Value v2 = value_create(TYPE_INT);
    v2.int_val = 2;
    ASTNode* lit2 = ast_create_literal(v2);
    
    Value v3 = value_create(TYPE_INT);
    v3.int_val = 3;
    ASTNode* lit3 = ast_create_literal(v3);
    
    ASTNode* add = ast_create_binary_op(BINOP_ADD, lit2, lit3);
    
    Value v4 = value_create(TYPE_INT);
    v4.int_val = 4;
    ASTNode* lit4 = ast_create_literal(v4);
    
    ASTNode* mul = ast_create_binary_op(BINOP_MUL, add, lit4);
    
    printf("AST for (2 + 3) * 4:\n");
    ast_print(mul, 0);
    
    ast_free_node(mul);
}

void test_source_location(void) {
    printf("\n--- Test: Source Location ---\n");
    
    ASTNode* node = ast_create_identifier("test");
    ast_set_location(node, "test.st", 42, 10);
    
    assert(strcmp(node->location.filename, "test.st") == 0);
    assert(node->location.line == 42);
    assert(node->location.column == 10);
    printf("✓ Set source location: %s:%d:%d\n", 
           node->location.filename, 
           node->location.line, 
           node->location.column);
    
    ast_free_node(node);
}

int main(void) {
    printf("========================================\n");
    printf("  STVM AST Module Test Suite\n");
    printf("========================================\n");
    
    // 初始化内存管理器
    if (!mmgr_init()) {
        fprintf(stderr, "Failed to initialize memory manager\n");
        return 1;
    }
    printf("✓ Memory manager initialized\n");
    
    // 运行测试
    test_literal_nodes();
    test_identifier_nodes();
    test_binary_operations();
    test_unary_operations();
    test_variable_declaration();
    test_assignment();
    test_if_statement();
    test_while_loop();
    test_for_loop();
    test_function_call();
    test_node_list();
    test_ast_print();
    test_source_location();
    
    // 打印统计信息
    mmgr_print_stats();
    
    // 检查内存泄漏
    if (mmgr_check_leaks()) {
        printf("\n❌ Memory leaks detected!\n");
    } else {
        printf("\n✓ No memory leaks detected\n");
    }
    
    // 清理
    mmgr_cleanup();
    printf("✓ Memory manager cleaned up\n");
    
    printf("\n========================================\n");
    printf("  All Tests Passed! ✓\n");
    printf("========================================\n");
    
    return 0;
}
