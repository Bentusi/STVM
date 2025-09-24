/*
 * IEC61131 抽象语法树 (AST) 定义
 * 定义语法树节点类型和操作函数
 */

#ifndef AST_H
#define AST_H

#include <stddef.h>
#include "common.h"

/* 数据类型枚举 */
typedef enum {
    TYPE_INVALID,
    TYPE_IDENTIFIER,
    TYPE_VOID,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_REAL,
    TYPE_STRING,
    TYPE_ARRAY
} DataType;

/* 操作符类型 */
typedef enum {
    OP_INVALID,
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_EQ, OP_NE, OP_LT, OP_LE, OP_GT, OP_GE,
    OP_AND, OP_OR, OP_NOT, OP_NEG
} OpType;

/* AST节点类型 */
typedef enum {
    NODE_INVALID,
    NODE_PROGRAM,
    NODE_FUNCTION,
    NODE_STATEMENT_LIST,
    NODE_ASSIGN,
    NODE_IF,
    NODE_FOR,
    NODE_WHILE,
    NODE_CASE,
    NODE_CASE_LIST,
    NODE_CASE_ITEM,
    NODE_RETURN,
    NODE_IDENTIFIER,
    NODE_INT_LITERAL,
    NODE_REAL_LITERAL,
    NODE_BOOL_LITERAL,
    NODE_STRING_LITERAL,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_FUNCTION_CALL,
    NODE_ARGUMENT_LIST,
    NODE_COMPILATION_UNIT
} NodeType;

/* 值联合体 */
typedef union {
    int int_val;
    double real_val;
    int bool_val;
    char *str_val;
} Value;

/* 变量声明结构 */
typedef struct VarDecl {
    char *name;
    DataType type;
    Value value;
    struct VarDecl *next;
} VarDecl;

/* 局部变量作用域栈 */
typedef struct LocalScope {
    VarDecl *local_vars;
    struct LocalScope *parent;
} LocalScope;

/* AST节点结构 */
typedef struct ASTNode {
    char *identifier;
    NodeType type;
    DataType data_type;
    Value value;
    OpType op_type;
    // 函数相关
    VarDecl *param_list;        // 参数列表
    VarDecl *local_vars;        // 函数内局部变量列表
    VarDecl *global_vars;       // 函数内使用的全局变量
    DataType return_type;
    struct ASTNode *condition;
    struct ASTNode *statements;
    struct ASTNode *else_statements;
    struct ASTNode *case_value;
    struct ASTNode *case_list;
    struct ASTNode *func_list;
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *next;
} ASTNode;

/* 函数声明 */

/* AST节点创建函数 */
ASTNode *create_program_node(char *name, ASTNode *statements);
ASTNode *create_statement_list(ASTNode *statement);
ASTNode *add_statement(ASTNode *list, ASTNode *statement);
ASTNode *create_assign_node(char *var_name, ASTNode *expr);
ASTNode *create_if_node(ASTNode *condition, ASTNode *then_stmt, ASTNode *else_stmt);
ASTNode *create_for_node(char *var_name, ASTNode *start, ASTNode *end, ASTNode *statements);
ASTNode *create_while_node(ASTNode *condition, ASTNode *statements);
ASTNode *create_case_node(ASTNode *expression, ASTNode *case_list);
ASTNode *create_case_list(ASTNode *case_item, ASTNode *next);
ASTNode *create_case_item(ASTNode *value, ASTNode *statements);
ASTNode *create_binary_op_node(OpType op, ASTNode *left, ASTNode *right);
ASTNode *create_unary_op_node(OpType op, ASTNode *operand);
ASTNode *create_identifier_node(char *name);
ASTNode *create_int_literal_node(int value);
ASTNode *create_real_literal_node(double value);
ASTNode *create_bool_literal_node(int value);
ASTNode *create_string_literal_node(char *value);
ASTNode *create_compilation_unit_node(ASTNode *func_list, ASTNode *program);
ASTNode *create_function_node(char *func_name, DataType return_type, VarDecl *param_list, VarDecl *global_var, VarDecl *local_vars, ASTNode *statements);
ASTNode *create_function_block_node(char *fb_name, VarDecl *param_list, ASTNode *statements);
ASTNode *create_return_node(ASTNode *return_value);

/* 变量管理函数 */
VarDecl *create_var_decl(char *name, DataType type);
VarDecl *create_var_decl_from_identifier(char *name);
int add_global_variable(VarDecl *var);
VarDecl *find_variable(char *name);
VarDecl *find_all_global_variables(void);
void print_all_global_variables(void);
void print_global_variables();
void clear_global_variables();

/* 函数调用验证和作用域管理函数声明 */
int validate_function_call(char *func_name, VarDecl *args);
ASTNode *create_function_call_node(char *func_name, VarDecl *args);
VarDecl *create_argument_list(ASTNode *argument);
VarDecl *add_argument_to_list(VarDecl *list, VarDecl *argument);
VarDecl *add_parameter(VarDecl *list, VarDecl *param);
int count_arguments(ASTNode *args);
int add_function_var_decl(ASTNode *func, VarDecl *var);
ASTNode *find_global_function(char *name);
VarDecl *find_local_variable(char *name);
void push_local_scope(void);
void pop_local_scope(void);
int get_function_param_count(ASTNode *func);
int enter_function_scope(ASTNode *func);
void exit_function_scope(void);
VarDecl *ast_get_global_var_table();
ASTNode *ast_get_global_function_table();
LocalScope *ast_get_current_scope();
int add_global_function(ASTNode *func);

/* AST遍历和打印函数 */
void print_ast(ASTNode *node, int indent);
void print_var_list(VarDecl *list, int indent);
void free_ast(ASTNode *node);
void clear_global_functions(void);
void print_function_info();

#endif // AST_H