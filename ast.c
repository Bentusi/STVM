/*
 * IEC61131 抽象语法树 (AST) 实现
 * 实现语法树节点创建、管理和操作函数
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "ast.h"

/* 全局变量表 */
static VarDecl *variable_table = NULL;
static ASTNode *function_table = NULL;

/* 创建程序节点 */
ASTNode *create_program_node(char *name, ASTNode *statements) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_PROGRAM;
    node->identifier = strdup(name);
    node->statements = statements;
    node->left = node->right = node->condition = node->else_statements = node->next = NULL;
    return node;
}

/* 创建语句列表 */
ASTNode *create_statement_list(ASTNode *statement) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_STATEMENT_LIST;
    node->left = statement;
    node->right = node->condition = node->statements = node->else_statements = node->next = NULL;
    node->identifier = NULL;
    return node;
}

/* 添加语句到列表 */
ASTNode *add_statement(ASTNode *list, ASTNode *statement) {
    if (list == NULL) {
        return create_statement_list(statement);
    }
    
    ASTNode *current = list;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = create_statement_list(statement);
    return list;
}

/* 创建编译单元 */
ASTNode *create_compilation_unit_node(ASTNode *func_list, ASTNode *program) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_PROGRAM;  // 使用程序节点类型表示编译单元
    node->statements = program;
    node->left = func_list;  // 函数列表作为左子节点
    node->right = node->condition = node->else_statements = node->next = NULL;
    node->identifier = NULL;
    return node;
}

/* 创建函数节点 */
ASTNode *create_function_node(char *func_name, DataType return_type, VarDecl *param_list, ASTNode *statements) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION;
    node->identifier = strdup(func_name);
    node->param_list = param_list;      // 参数列表
    node->return_type = return_type;    // 返回值类型
    node->statements = statements; // 函数体语句
    node->left = node->right = NULL;
    node->condition = node->else_statements = node->next = NULL;
    return node;
}

/* 创建函数块节点 */
ASTNode *create_function_block_node(char *fb_name, VarDecl *param_list, ASTNode *statements) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION_BLOCK;
    node->identifier = strdup(fb_name);
    node->param_list = param_list;      // 参数列表
    node->statements = statements; // 函数块体语句
    node->left = node->right = NULL;
    node->condition = node->else_statements = node->next = NULL;
    return node;
}

/* 创建返回值节点 */
ASTNode *create_return_node(ASTNode *return_value) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_RETURN;
    node->left = return_value;  // 返回值表达式
    node->return_type = return_value->data_type; // 设置返回值类型
    node->right = node->condition = node->statements = node->else_statements = node->next = NULL;
    node->identifier = NULL;
    return node;
}

/* 创建赋值节点 */
ASTNode *create_assign_node(char *var_name, ASTNode *expr) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_ASSIGN;
    node->identifier = strdup(var_name);
    node->right = expr;
    node->left = node->condition = node->statements = node->else_statements = node->next = NULL;
    return node;
}

/* 创建IF节点 */
ASTNode *create_if_node(ASTNode *condition, ASTNode *then_stmt, ASTNode *else_stmt) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_IF;
    node->condition = condition;
    node->statements = then_stmt;
    node->else_statements = else_stmt;
    node->left = node->right = node->next = NULL;
    node->identifier = NULL;
    return node;
}

/* 创建FOR循环节点 */
ASTNode *create_for_node(char *var_name, ASTNode *start, ASTNode *end, ASTNode *statements) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_FOR;
    node->identifier = strdup(var_name);
    node->left = start;
    node->right = end;
    node->statements = statements;
    node->condition = node->else_statements = node->next = NULL;
    return node;
}

/* 创建WHILE循环节点 */
ASTNode *create_while_node(ASTNode *condition, ASTNode *statements) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_WHILE;
    node->condition = condition;
    node->statements = statements;
    node->left = node->right = node->else_statements = node->next = NULL;
    node->identifier = NULL;
    return node;
}

/* 创建CASE节点 */
ASTNode *create_case_node(ASTNode *expression, ASTNode *case_list) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_CASE;
    node->left = expression;  // CASE表达式
    node->statements = case_list;  // CASE项列表
    node->condition = node->right = node->else_statements = node->next = NULL;
    node->identifier = NULL;
    return node;
}

/* 创建CASE列表节点 */
ASTNode *create_case_list(ASTNode *case_item, ASTNode *next) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_CASE_LIST;
    node->left = case_item;
    node->next = next;
    node->condition = node->right = node->statements = node->else_statements = NULL;
    node->identifier = NULL;
    return node;
}

/* 创建CASE项节点 */
ASTNode *create_case_item(ASTNode *value, ASTNode *statements) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_CASE_ITEM;
    node->left = value;  // CASE值
    node->statements = statements;  // 执行语句
    node->condition = node->right = node->else_statements = NULL;
    node->next = NULL;  // 初始化next指针
    node->identifier = NULL;
    return node;
}

/* 创建二元操作节点 */
ASTNode *create_binary_op_node(OpType op, ASTNode *left, ASTNode *right) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    memset(node, 0, sizeof(ASTNode));
    node->type = NODE_BINARY_OP;
    node->op_type = op;
    node->left = left;
    node->right = right;
    node->condition = node->statements = node->else_statements = node->next = NULL;
    node->identifier = NULL;
    return node;
}

/* 创建一元操作节点 */
ASTNode *create_unary_op_node(OpType op, ASTNode *operand) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_UNARY_OP;
    node->op_type = op;
    node->left = operand;
    node->right = node->condition = node->statements = node->else_statements = node->next = NULL;
    node->identifier = NULL;
    return node;
}

/* 创建标识符节点 */
ASTNode *create_identifier_node(char *name) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_IDENTIFIER;
    node->identifier = strdup(name);
    node->left = node->right = node->condition = node->statements = node->else_statements = node->next = NULL;
    return node;
}

/* 创建整数字面量节点 */
ASTNode *create_int_literal_node(int value) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_INT_LITERAL;
    node->data_type = TYPE_INT;
    node->value.int_val = value;
    node->left = node->right = node->condition = node->statements = node->else_statements = node->next = NULL;
    node->identifier = NULL;
    return node;
}

/* 创建实数字面量节点 */
ASTNode *create_real_literal_node(double value) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_REAL_LITERAL;
    node->data_type = TYPE_REAL;
    node->value.real_val = value;
    node->left = node->right = node->condition = node->statements = node->else_statements = node->next = NULL;
    node->identifier = NULL;
    return node;
}

/* 创建布尔字面量节点 */
ASTNode *create_bool_literal_node(int value) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_BOOL_LITERAL;
    node->data_type = TYPE_BOOL;
    node->value.bool_val = value;
    node->left = node->right = node->condition = node->statements = node->else_statements = node->next = NULL;
    node->identifier = NULL;
    return node;
}

/* 创建字符串字面量节点 */
ASTNode *create_string_literal_node(char *value) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_STRING_LITERAL;
    node->data_type = TYPE_STRING;
    node->value.str_val = strdup(value);
    node->left = node->right = node->condition = node->statements = node->else_statements = node->next = NULL;
    node->identifier = NULL;
    return node;
}

/* 创建变量声明 */
VarDecl *create_var_decl(char *name, DataType type) {
    VarDecl *var = (VarDecl *)malloc(sizeof(VarDecl));
    var->name = strdup(name);
    var->type = type;
    var->next = NULL;
    return var;
}

/* 添加函数到函数表 */
int add_global_function(ASTNode *func) {
    // 检查函数是否已存在，避免重复声明
    ASTNode *existing = find_global_function(func->identifier);
    if (existing != NULL) {
        free(func->identifier);
        free(func);
        return -1; // 或者返回错误码
    }
    
    // 将新函数添加到链表头部
    func->next = function_table;
    function_table = func;
    return 0; // 成功
}

/* 添加变量到符号表 */
int add_global_variable(VarDecl *var) {
    // 检查变量是否已存在，避免重复声明
    VarDecl *existing = find_variable(var->name);
    if (existing != NULL) {
        // 如果变量已存在，释放新变量并返回错误
        free(var->name);
        free(var);
        return -1; // 或者可以返回错误码
    }
    
    // 将新变量添加到链表头部
    var->next = variable_table;
    variable_table = var;
    return 0; // 成功
}

/* 查找函数 */
ASTNode *find_global_function(char *name) {
    ASTNode *current = function_table;
    while (current != NULL) {
        if (strcmp(current->identifier, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/* 查找变量 */
VarDecl *find_variable(char *name) {
    VarDecl *current = variable_table;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/* 打印AST结构（调试用） */
void print_ast(ASTNode *node, int indent) {
    if (node == NULL) return;
    
    for (int i = 0; i < indent; i++) printf("  ");
    
    switch (node->type) {
        case NODE_PROGRAM:
            printf("程序: %s\n", node->identifier);
            print_ast(node->statements, indent + 1);
            break;
        case NODE_STATEMENT_LIST:
            printf("语句列表\n");
            print_ast(node->left, indent + 1);
            print_ast(node->next, indent);
            break;
        case NODE_ASSIGN:
            printf("赋值: %s\n", node->identifier);
            print_ast(node->right, indent + 1);
            break;
        case NODE_IF:
            printf("IF语句\n");
            print_ast(node->condition, indent + 1);
            print_ast(node->statements, indent + 1);
            if (node->else_statements) {
                for (int i = 0; i < indent; i++) printf("  ");
                printf("ELSE\n");
                print_ast(node->else_statements, indent + 1);
            }
            break;
        case NODE_BINARY_OP:
            printf("二元操作: %d\n", node->op_type);
            print_ast(node->left, indent + 1);
            print_ast(node->right, indent + 1);
            break;
        case NODE_CASE:
            printf("CASE语句\n");
            for (int i = 0; i < indent; i++) printf("  ");
            printf("表达式:\n");
            print_ast(node->left, indent + 1);
            for (int i = 0; i < indent; i++) printf("  ");
            printf("CASE项:\n");
            print_ast(node->statements, indent + 1);
            break;
        case NODE_CASE_LIST:
            printf("CASE列表\n");
            print_ast(node->left, indent + 1);
            if (node->next) print_ast(node->next, indent);
            break;
        case NODE_CASE_ITEM:
            printf("CASE项: ");
            print_ast(node->left, 0);  // 值不需要缩进
            printf(" -> \n");
            print_ast(node->statements, indent + 1);
            if (node->next) print_ast(node->next, indent);
            break;
        case NODE_INT_LITERAL:
            printf("整数: %d\n", node->value.int_val);
            break;
        case NODE_REAL_LITERAL:
            printf("实数: %f\n", node->value.real_val);
            break;
        case NODE_BOOL_LITERAL:
            printf("布尔: %s\n", node->value.bool_val ? "TRUE" : "FALSE" );
            break;
        case NODE_IDENTIFIER:
            printf("标识符: %s\n", node->identifier);
            break;
        default:
            printf("未知节点类型\n");
            break;
    }
}

/* 释放AST内存 */
void free_ast(ASTNode *node) {
    if (node == NULL) return;
    
    free_ast(node->left);
    free_ast(node->right);
    free_ast(node->condition);
    free_ast(node->statements);
    free_ast(node->else_statements);
    free_ast(node->next);
    
    if (node->identifier) free(node->identifier);
    if (node->type == NODE_STRING_LITERAL && node->value.str_val) {
        free(node->value.str_val);
    }
    
    free(node);
}