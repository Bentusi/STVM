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

/* 全局函数表 */
static FunctionDecl *function_table = NULL;

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

/* 创建函数节点 - 修正签名 */
ASTNode *create_function_node(char *name, ParamDecl *params, DataType return_type, ASTNode *body) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION;
    node->identifier = strdup(name);
    node->return_type = return_type;
    node->params = params;
    node->statements = body;
    node->left = node->right = node->condition = node->else_statements = node->next = NULL;
    node->arguments = node->return_value = NULL;
    return node;
}

/* 创建函数块节点 */
ASTNode *create_function_block_node(char *name, DataType return_type, ASTNode *body) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION_BLOCK;
    node->identifier = strdup(name);
    node->return_type = return_type;
    node->statements = body;
    node->params = NULL;
    node->left = node->right = node->condition = node->else_statements = node->next = NULL;
    node->arguments = node->return_value = NULL;
    return node;
}

/* 创建函数调用节点 */
ASTNode *create_function_call_node(char *name, ASTNode *arguments) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_FUNCTION_CALL;
    node->identifier = strdup(name);
    node->arguments = arguments;
    node->left = node->right = node->condition = node->statements = node->else_statements = node->next = NULL;
    node->params = NULL;
    node->return_value = NULL;
    return node;
}

/* 创建返回语句节点 */
ASTNode *create_return_node(ASTNode *value) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    node->type = NODE_RETURN;
    node->return_value = value;
    node->left = node->right = node->condition = node->statements = node->else_statements = node->next = NULL;
    node->identifier = NULL;
    node->params = NULL;
    node->arguments = NULL;
    return node;
}

/* 创建参数节点 */
ParamDecl *create_param_node(char *name, DataType type) {
    ParamDecl *param = (ParamDecl *)malloc(sizeof(ParamDecl));
    param->name = strdup(name);
    param->type = type;
    param->direction = PARAM_VALUE;
    param->next = NULL;
    return param;
}

/* 创建输入参数节点 */
ParamDecl *create_input_param_node(char *name, DataType type) {
    ParamDecl *param = create_param_node(name, type);
    param->direction = PARAM_INPUT;
    return param;
}

/* 创建输出参数节点 */
ParamDecl *create_output_param_node(char *name, DataType type) {
    ParamDecl *param = create_param_node(name, type);
    param->direction = PARAM_OUTPUT;
    return param;
}

/* 创建输入输出参数节点 */
ParamDecl *create_inout_param_node(char *name, DataType type) {
    ParamDecl *param = create_param_node(name, type);
    param->direction = PARAM_INOUT;
    return param;
}

/* 添加参数到参数列表 */
ParamDecl *add_param(ParamDecl *list, ParamDecl *param) {
    if (list == NULL) {
        return param;
    }
    
    ParamDecl *current = list;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = param;
    return list;
}

/* 创建函数声明 */
FunctionDecl *create_function_decl(char *name, DataType return_type, ParamDecl *params, ASTNode *body) {
    FunctionDecl *func = (FunctionDecl *)malloc(sizeof(FunctionDecl));
    func->name = strdup(name);
    func->return_type = return_type;
    func->params = params;
    func->body = body;
    func->next = NULL;
    return func;
}

/* 添加函数到函数表 */
void add_function(FunctionDecl *func) {
    if (function_table == NULL) {
        function_table = func;
    } else {
        FunctionDecl *current = function_table;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = func;
    }
}

/* 查找函数 */
FunctionDecl *find_function(char *name) {
    FunctionDecl *current = function_table;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
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

/* 添加变量到符号表 */
void add_variable(VarDecl *var) {
    if (variable_table == NULL) {
        variable_table = var;
    } else {
        VarDecl *current = variable_table;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = var;
    }
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
        case NODE_FUNCTION:
            printf("函数: %s (返回类型: %d)\n", node->identifier, node->return_type);
            if (node->params) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("参数列表:\n");
                print_params(node->params, indent + 2);
            }
            print_ast(node->statements, indent + 1);
            break;
        case NODE_FUNCTION_CALL:
            printf("函数调用: %s\n", node->identifier);
            if (node->arguments) {
                for (int i = 0; i < indent + 1; i++) printf("  ");
                printf("参数:\n");
                print_ast(node->arguments, indent + 2);
            }
            break;
        case NODE_RETURN:
            printf("返回语句\n");
            if (node->return_value) {
                print_ast(node->return_value, indent + 1);
            }
            break;
        default:
            printf("未知节点类型\n");
            break;
    }
}

/* 打印参数列表 */
void print_params(ParamDecl *params, int indent) {
    ParamDecl *current = params;
    while (current != NULL) {
        for (int i = 0; i < indent; i++) printf("  ");
        printf("参数: %s (类型: %d, 方向: %d)\n", 
               current->name, current->type, current->direction);
        current = current->next;
    }
}

/* 释放参数列表内存 */
void free_params(ParamDecl *params) {
    ParamDecl *current = params;
    while (current != NULL) {
        ParamDecl *next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
}

/* 扩展释放AST内存函数 */
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
    
    /* 释放函数相关内存 */
    if (node->params) {
        free_params(node->params);
    }
    free_ast(node->arguments);
    free_ast(node->return_value);
    
    free(node);
}

/* 类型检查函数 */
DataType get_expression_type(ASTNode *expr) {
    if (expr == NULL) return TYPE_VOID;
    
    switch (expr->type) {
        case NODE_INT_LITERAL:
            return TYPE_INT;
        case NODE_REAL_LITERAL:
            return TYPE_REAL;
        case NODE_BOOL_LITERAL:
            return TYPE_BOOL;
        case NODE_STRING_LITERAL:
            return TYPE_STRING;
        case NODE_IDENTIFIER: {
            VarDecl *var = find_variable(expr->identifier);
            return var ? var->type : TYPE_VOID;
        }
        case NODE_FUNCTION_CALL: {
            FunctionDecl *func = find_function(expr->identifier);
            return func ? func->return_type : TYPE_VOID;
        }
        case NODE_BINARY_OP:
            // 根据操作符类型推断返回类型
            switch (expr->op_type) {
                case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV: case OP_MOD:
                    return get_expression_type(expr->left);
                case OP_EQ: case OP_NE: case OP_LT: case OP_LE: case OP_GT: case OP_GE:
                case OP_AND: case OP_OR:
                    return TYPE_BOOL;
                default:
                    return TYPE_VOID;
            }
        default:
            return TYPE_VOID;
    }
}

/* 检查函数调用参数匹配 */
int check_function_call(char *func_name, ASTNode *arguments) {
    FunctionDecl *func = find_function(func_name);
    if (func == NULL) {
        printf("错误: 未定义的函数 '%s'\n", func_name);
        return 0;
    }
    
    ParamDecl *param = func->params;
    ASTNode *arg = arguments;
    int param_count = 0, arg_count = 0;
    
    // 计算参数个数
    while (param != NULL) {
        param_count++;
        param = param->next;
    }
    
    while (arg != NULL) {
        arg_count++;
        arg = arg->next;
    }
    
    if (param_count != arg_count) {
        printf("错误: 函数 '%s' 参数个数不匹配，期望 %d 个，实际 %d 个\n", 
               func_name, param_count, arg_count);
        return 0;
    }
    
    // 检查参数类型匹配
    param = func->params;
    arg = arguments;
    while (param != NULL && arg != NULL) {
        DataType arg_type = get_expression_type(arg);
        if (param->type != arg_type) {
            printf("错误: 函数 '%s' 参数 '%s' 类型不匹配\n", 
                   func_name, param->name);
            return 0;
        }
        param = param->next;
        arg = arg->next;
    }
    
    return 1;
}