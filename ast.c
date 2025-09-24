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
static LocalScope *current_scope = NULL;

/* 获取全局变量表 */
VarDecl *ast_get_global_var_table() {
    return variable_table;
}

/* 获取全局函数表 */
ASTNode *ast_get_global_function_table() {
    return function_table;
}

/* 获取当前作用域 */
LocalScope *ast_get_current_scope() {
    return current_scope;
}

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

/* 创建函数调用节点 */
ASTNode *create_function_call_node(char *func_name, VarDecl *params) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION_CALL;
    node->identifier = strdup(func_name);
    node->param_list = params;
    node->left = node->right = node->condition = node->statements = node->else_statements = node->next = NULL;
    return node;
}

/* 创建参数列表节点 */
VarDecl *create_argument_list(ASTNode *argument) {
    VarDecl *list = NULL;
    if (argument) {
        list = create_var_decl(argument->identifier, argument->data_type);
    }
    return list;
}

/* 添加参数到参数列表 */
VarDecl *add_argument_to_list(VarDecl *list, VarDecl *argument) {
    if (argument == NULL) {
        return list;
    }

    if (list == NULL) {
        return argument;
    }

    VarDecl *current = list;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = argument;
    argument->next = NULL;
    return list;
}

/* 创建编译单元 */
ASTNode *create_compilation_unit_node(ASTNode *func_list, ASTNode *program) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_COMPILATION_UNIT;
    node->left = func_list;     // 函数列表作为左子节点
    node->right = program;      // 程序作为右子节点，不是statements
    node->statements = NULL;    // 避免重复引用
    node->condition = node->else_statements = node->next = NULL;
    node->identifier = NULL;
    return node;
}

/* 创建带变量的函数节点 */
ASTNode *create_function_node(char *func_name, DataType return_type, VarDecl *param_list, VarDecl *global_var, VarDecl *local_vars, ASTNode *statements) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION;
    node->identifier = strdup(func_name);
    node->param_list = param_list;      // 参数列表
    node->return_type = return_type;    // 返回值类型
    node->statements = statements; // 函数体语句
    node->left = node->right = NULL;
    node->condition = node->else_statements = node->next = NULL;
    
    node->global_vars = global_var; // 全局变量列表
    node->local_vars = local_vars;  // 局部变量列表

    // 设置函数作用域
    if (setup_function_scope(node) != 0) {
        printf("错误: 无法为函数 '%s' 设置作用域\n", func_name);
        free(node->identifier);
        free(node);
        return NULL;
    }

    return node;
}

/* 为函数创建独立作用域 */
/* 调用时机: 函数定义时 */
int setup_function_scope(ASTNode *func) {
    if (!func || func->type != NODE_FUNCTION) {
        return -1;
    }
    
    // 创建函数作用域
    push_local_scope();
    
    // 添加参数到作用域
    VarDecl *param = func->param_list;
    while (param) {
        VarDecl *local_param = create_var_decl(param->name, param->type);
        if (add_local_variable(local_param) != 0) {
            free(local_param->name);
            free(local_param);
            return -1;
        }
        param = param->next;
    }
    
    // 添加局部变量到作用域
    VarDecl *local_var = func->local_vars;
    while (local_var) {
        VarDecl *scope_var = create_var_decl(local_var->name, local_var->type);
        if (add_local_variable(scope_var) != 0) {
            free(scope_var->name);
            free(scope_var);
            return -1;
        }
        local_var = local_var->next;
    }
    
    return 0;
}

/* 向函数添加局部变量 */
int add_function_local_var(ASTNode *func, VarDecl *var) {
    if (!func || func->type != NODE_FUNCTION || !var) {
        return -1;
    }
    
    // 检查是否与参数重名
    VarDecl *param = func->param_list;
    while (param) {
        if (strcmp(param->name, var->name) == 0) {
            printf("错误: 变量 '%s' 与参数重名\n", var->name);
            return -2; // 与参数重名
        }
        param = param->next;
    }
    
    // 检查是否与现有局部变量重名
    VarDecl *local = func->local_vars;
    while (local) {
        if (strcmp(local->name, var->name) == 0) {
            printf("错误: 变量 '%s' 与现有局部变量重名\n", var->name);
            return -3; // 局部变量重名
        }
        local = local->next;
    }
    
    // 添加到局部变量链表
    var->next = func->local_vars;
    func->local_vars = var;
    
    return 0;
}

/* 查找函数内的局部变量 */
VarDecl *find_function_local_var(ASTNode *func, char *name) {
    if (!func || func->type != NODE_FUNCTION || !name) {
        return NULL;
    }
    
    // 先查找参数
    VarDecl *param = func->param_list;
    while (param) {
        if (strcmp(param->name, name) == 0) {
            return param;
        }
        param = param->next;
    }
    
    // 再查找局部变量
    VarDecl *local = func->local_vars;
    while (local) {
        if (strcmp(local->name, name) == 0) {
            return local;
        }
        local = local->next;
    }
    
    return NULL;
}

/* 获取函数所有变量数量 */
int get_function_var_count(ASTNode *func) {
    if (!func || func->type != NODE_FUNCTION) {
        return 0;
    }
    
    int count = 0;
    
    // 计算参数数量
    VarDecl *param = func->param_list;
    while (param) {
        count++;
        param = param->next;
    }
    
    // 计算局部变量数量
    VarDecl *local = func->local_vars;
    while (local) {
        count++;
        local = local->next;
    }
    
    return count;
}

/* 打印函数详细信息 */
void print_function_details(ASTNode *func) {
    if (!func || func->type != NODE_FUNCTION) {
        return;
    }
    
    printf("=== 函数详情: %s ===\n", func->identifier);
    printf("返回类型: %d\n", func->return_type);
    
    // 打印参数
    printf("参数列表:\n");
    VarDecl *param = func->param_list;
    int param_count = 0;
    while (param) {
        printf("  [%d] %s: %d\n", param_count++, param->name, param->type);
        param = param->next;
    }
    
    // 打印局部变量
    printf("局部变量:\n");
    VarDecl *local = func->local_vars;
    int local_count = 0;
    while (local) {
        printf("  [%d] %s: %d\n", local_count++, local->name, local->type);
        local = local->next;
    }
    
    printf("总变量数量: %d\n", param_count + local_count);
    printf("========================\n");
}

/* 释放函数变量 */
void free_function_vars(ASTNode *func) {
    if (!func || func->type != NODE_FUNCTION) {
        return;
    }
    
    // 释放参数列表
    free_param_list(func->param_list);
    func->param_list = NULL;
    
    // 释放局部变量列表
    free_param_list(func->local_vars);
    func->local_vars = NULL;
    
    // 释放全局变量引用列表
    free_param_list(func->global_vars);
    func->global_vars = NULL;
}

/* 创建返回值节点 */
ASTNode *create_return_node(ASTNode *return_value) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_RETURN;
    node->left = return_value;  // 返回值表达式
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
    node->condition = expression; // CASE表达式
    node->case_list = case_list;  // CASE项列表
    node->left = node->right = node->else_statements = node->next = NULL;
    node->identifier = NULL;
    return node;
}

/* CASE项加入到CASE列表 */
ASTNode *create_case_list(ASTNode *case_list, ASTNode *case_item) {
    ASTNode *current = case_list;
    while (current->next != NULL) {
        current = current->next;
    }

    current->next = case_item; // 添加新项
    return case_list;
}

/* 创建CASE项节点 */
ASTNode *create_case_item(ASTNode *value, ASTNode *statements) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_CASE_ITEM;
    node->case_value = value;
    node->statements = statements; // 执行语句
    node->case_list = node->condition = node->right = node->else_statements = NULL;
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
    var->name = strdup(name ? name : "Anonymous"); // 处理NULL名称
    var->type = type;
    var->next = NULL;
    return var;
}

/* 通过标识名称符创建变量声明 */
/* 先在函数局部变量表中查找变量，没有再从全局变量表中查找 */
VarDecl *create_var_decl_from_identifier(char *name) {
    if (name == NULL) {
        return NULL;
    }
    
    VarDecl *local_var = find_local_variable(name);
    if (local_var != NULL) {
        return create_var_decl(local_var->name, local_var->type);
    }
    VarDecl *global_var = find_variable(name);
    if (global_var != NULL) {
        return create_var_decl(global_var->name, global_var->type);
    }
    
    // 如果找不到变量，返回NULL而不是创建未定义的变量
    return NULL;
}

/* 添加变量到参数列表 */
VarDecl *add_parameter(VarDecl *list, VarDecl *param) {
    if (list == NULL) {
        return param;
    }
    
    VarDecl *current = list;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = param;
    return list;
}

/* 创建新的局部作用域 */
void push_local_scope() {
    LocalScope *new_scope = (LocalScope *)malloc(sizeof(LocalScope));
    new_scope->local_vars = NULL;
    new_scope->parent = current_scope;
    current_scope = new_scope;
}

/* 弹出当前局部作用域 */
void pop_local_scope() {
    if (current_scope == NULL) return;
    
    LocalScope *old_scope = current_scope;
    current_scope = current_scope->parent;
    
    // 释放局部变量
    VarDecl *var = old_scope->local_vars;
    while (var != NULL) {
        VarDecl *next = var->next;
        free(var->name);
        free(var);
        var = next;
    }
    
    free(old_scope);
}

/* 添加局部变量 */
int add_local_variable(VarDecl *var) {
    if (current_scope == NULL) {
        return -1; // 没有局部作用域
    }
    
    // 检查当前作用域中是否已存在同名变量
    VarDecl *existing = find_local_variable(var->name);
    if (existing != NULL) {
        return -1; // 变量已存在
    }
    
    // 添加到当前作用域
    var->next = current_scope->local_vars;
    current_scope->local_vars = var;
    return 0;
}

/* 查找局部变量（包括所有上级作用域） */
VarDecl *find_local_variable(char *name) {
    LocalScope *scope = current_scope;
    while (scope != NULL) {
        VarDecl *var = scope->local_vars;
        while (var != NULL) {
            if (strcmp(var->name, name) == 0) {
                return var;
            }
            var = var->next;
        }
        scope = scope->parent;
    }
    return NULL;
}

/* 查找变量（先查找局部变量，再查找全局变量） */
VarDecl *find_variable_in_scope(char *name) {
    // 先查找局部变量
    VarDecl *local_var = find_local_variable(name);
    if (local_var != NULL) {
        return local_var;
    }
    
    // 再查找全局变量
    return find_variable(name);
}

/* 获取函数表 */
ASTNode *get_function_table() {
    return function_table;
}

/* 添加函数到函数表 */
int add_global_function(ASTNode *func) {
    // 检查函数是否已存在，避免重复声明
    ASTNode *existing = find_global_function(func->identifier);
    if (existing != NULL) {
        return -1; // 返回错误码，但不释放节点
    }
    
    // 创建函数节点的副本用于函数表
    ASTNode *func_copy = (ASTNode *)malloc(sizeof(ASTNode));
    *func_copy = *func; // 复制内容
    func_copy->identifier = strdup(func->identifier); // 复制标识符
    func_copy->next = function_table; // 链接到函数表
    function_table = func_copy;
    return 0; // 成功
}

/* 添加变量到符号表 */
int add_global_variable(VarDecl *var) {
    // 检查变量是否已存在，避免重复声明
    VarDecl *existing = find_variable(var->name);
    if (existing != NULL) {
        // 如果变量已存在，返回错误
        return -1;
    }
    
    // 将新变量添加到链表头部
    var->next = variable_table;
    variable_table = var;
    return 0; // 成功
}

/* 查找函数 */
ASTNode *find_global_function(char *name) {
    if (!name) return NULL;

    ASTNode *current = ast_get_global_function_table();
    while (current) {
        if (current->identifier && strcmp(current->identifier, name) == 0) {
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

/* 获取所有全局变量 */
VarDecl *find_all_global_variables(void) {
    return variable_table;
}

/* 打印全局变量列表（调试用） */
void print_all_global_variables(void) {
    printf("=== AST全局变量列表 ===\n");
    VarDecl *var = variable_table;
    int count = 0;
    
    while (var) {
        printf("变量[%d]: %s (类型: %d)\n", count++, var->name, var->type);
        var = var->next;
    }
    
    if (count == 0) {
        printf("无全局变量\n");
    } else {
        printf("总共 %d 个全局变量\n", count);
    }
    printf("======================\n");
}

/* 打印变量表结构（调试用） */
void print_var_list(VarDecl *list, int indent) {
    VarDecl *current = list;
    while (current != NULL) {
        for (int i = 0; i < indent; i++) printf("  ");
        printf("变量: %s (类型: %d)\n", current->name, current->type);
        current = current->next;
    }
}

/* 打印函数调用参数表（调试用） */
void print_arg_list(VarDecl *list, int indent) {
    VarDecl *current = list;
    printf("(");
    while (current != NULL) {
        for (int i = 0; i < indent; i++) printf("  ");
        switch (current->type)
        {
        case TYPE_IDENTIFIER:
            printf("%s", current->name);
            break;
        case TYPE_INT:
            printf("%s:%d", current->name, current->value.int_val);
            break;
        case TYPE_REAL:
            printf("%s:%f", current->name, current->value.real_val);
            break;
        case TYPE_BOOL:
            printf("%s:%s", current->name, current->value.bool_val ? "TRUE" : "FALSE");
            break;
        case TYPE_STRING:
            printf("%s:%s", current->name, current->value.str_val);
            break;
        default:
            break;
        }
        current = current->next;
        printf(current ? ", " : ")");
    }
}

/* 打印AST结构（调试用） */
void print_ast(ASTNode *node, int indent) {
    if (!node) return;
    
    for (int i = 0; i < indent; i++) printf("  ");
    
    switch (node->type) {
        case NODE_COMPILATION_UNIT:
            printf("编译单元\n");
            if (node->left) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("函数列表:\n");
                print_ast(node->left, indent + 2);
            }
            if (node->right) {
                print_ast(node->right, indent + 1);
            }
            break;
            
        case NODE_FUNCTION:
            printf("函数: %s\n", node->identifier);
            if (node->param_list) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("参数列表:\n");
                VarDecl *param = node->param_list;
                while (param) {
                    for (int i = 0; i < indent + 2; i++) printf("  ");
                    printf("参数: %s (类型: %d)\n", param->name, param->type);
                    param = param->next;
                }
            }
            if (node->statements) {
                print_ast(node->statements, indent + 1);
            }
            // 打印下一个函数
            if (node->next) {
                print_ast(node->next, indent);
            }
            break;
            
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
            print_ast(node->condition, indent + 1);
            for (int i = 0; i < indent; i++) printf("  ");
            printf("CASE项:\n");
            print_ast(node->case_list, indent + 1);
            break;
        case NODE_CASE_LIST:
            printf("CASE列表\n");
            print_ast(node->case_list, indent + 1);
            if (node->next) print_ast(node->next, indent);
            break;
        case NODE_CASE_ITEM:
            printf("CASE项: ");
            print_ast(node->case_value, 0);  // 值不需要缩进
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
        case NODE_FUNCTION_CALL:
            printf("函数调用: %s", node->identifier);
            print_arg_list(node->param_list, 0);
            printf("\n");
            break;
        case NODE_RETURN:
            printf("返回值\n");
            if (node->left) {
                print_ast(node->left, indent + 1);
            }
            break;
        default:
            printf("未知节点类型\n");
            break;
    }
}

/*  函数调用验证参数验证 */
int validate_function_call(char *func_name, VarDecl *args) {
    // 查找函数是否存在
    ASTNode *func = find_global_function(func_name);
    if (func == NULL) {
        return -1; // 函数不存在
    }

    // 计算实参数量
    int arg_count = 0;
    VarDecl *current = args;
    while (current != NULL) {
        arg_count++;
        current = current->next;
    }

    // 获取函数形参数量
    int param_count = get_function_param_count(func);
    
    // 检查参数数量是否匹配
    if (arg_count != param_count) {
        return -2; // 参数数量不匹配
    }

    return 0; // 验证通过
}

/* 打印函数信息 */
void print_function_info() {
    ASTNode *func = get_function_table();
    printf("\n=== 已定义的函数 ===\n");
    while (func != NULL) {
        printf("函数: %s", func->identifier);
        if (func->param_list) {
            printf("(");
            VarDecl *param = func->param_list;
            int first = 1;
            while (param != NULL) {
                if (!first) printf(", ");
                printf("%s: %d", param->name, param->type);
                param = param->next;
                first = 0;
            }
            printf(")");
        } else {
            printf("()");
        }
        printf(" -> %d\n", func->return_type);
        func = func->next;
    }
    printf("===================\n");
}

/* 计算参数个数 */
int count_arguments(ASTNode *args) {
    int count = 0;
    ASTNode *current = args;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

/* 检查参数类型匹配 */
int match_parameter_types(VarDecl *params, ASTNode *args) {
    VarDecl *param = params;
    ASTNode *arg = args;
    
    while (param != NULL && arg != NULL) {
        // 这里可以添加更复杂的类型匹配逻辑
        // 简单的实现：跳过类型检查，仅检查个数
        param = param->next;
        arg = arg->next;
    }
    
    // 确保参数和实参数量匹配
    return (param == NULL && arg == NULL) ? 0 : -1;
}

/* 获取函数参数个数 */
int get_function_param_count(ASTNode *func) {
    if (func == NULL || func->param_list == NULL) {
        return 0;
    }
    
    int count = 0;
    VarDecl *param = func->param_list;
    while (param != NULL) {
        count++;
        param = param->next;
    }
    return count;
}

/* 添加函数变量定义 */
int add_function_var_decl(ASTNode *func, VarDecl *var) {
    if (func == NULL || func->type != NODE_FUNCTION) {
        return -1; // 不是函数节点
    }
    
    // 检查变量是否已存在于函数参数中
    VarDecl *existing = func->param_list;
    while (existing != NULL) {
        if (strcmp(existing->name, var->name) == 0) {
            return -1; // 变量已存在
        }
        existing = existing->next;
    }
    
    // 添加变量到函数参数列表
    var->next = func->param_list;
    func->param_list = var;
    return 0; // 成功
}

/* 释放参数列表 */
void free_param_list(VarDecl *params) {
    VarDecl *current = params;
    while (current != NULL) {
        VarDecl *next = current->next;
        if (current->name) {
            free(current->name);
        }
        free(current);
        current = next;
    }
}

/* 释放局部作用域变量 */
void free_local_variables() {
    while (current_scope != NULL) {
        pop_local_scope();
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

/* 清理全局函数表 */
void clear_global_functions() {
    ASTNode *current = function_table;
    while (current) {
        ASTNode *next = current->next;
        if (current->identifier) {
            free(current->identifier);
        }
        free(current);
        current = next;
    }
    function_table = NULL;
}

/* 清理全局变量表 */
void clear_global_variables() {
    VarDecl *current = variable_table;
    while (current) {
        VarDecl *next = current->next;
        if (current->name) {
            free(current->name);
        }
        free(current);
        current = next;
    }
    variable_table = NULL;
}

/* 获取当前作用域 */
LocalScope *get_current_scope() {
    return current_scope;
}

/* 函数调用时的作用域管理 */
int enter_function_scope(ASTNode *func) {
    if (func == NULL || func->type != NODE_FUNCTION) {
        return -1;
    }
    
    push_local_scope();
    
    // 将函数参数添加到局部作用域
    VarDecl *param = func->param_list;
    while (param != NULL) {
        VarDecl *local_param = create_var_decl(param->name, param->type);
        if (add_local_variable(local_param) != 0) {
            free(local_param->name);
            free(local_param);
            return -1;
        }
        param = param->next;
    }
    
    return 0;
}

/* 退出函数作用域 */
void exit_function_scope() {
    pop_local_scope();
}