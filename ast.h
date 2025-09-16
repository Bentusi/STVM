/*
 * IEC61131 抽象语法树 (AST) 定义
 * 定义语法树节点类型和操作函数
 */

#ifndef AST_H
#define AST_H

#include <stddef.h>

/* 数据类型枚举 */
typedef enum {
    TYPE_INVALID,
    TYPE_VOID,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_REAL,
    TYPE_STRING
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
    NODE_COMPILATION_UNIT,
    NODE_STATEMENT_LIST,
    NODE_ASSIGN,
    NODE_FUNCTION,
    NODE_FUNCTION_BLOCK,
    NODE_RETURN,
    NODE_IF,
    NODE_FOR,
    NODE_WHILE,
    NODE_CASE,
    NODE_CASE_LIST,
    NODE_CASE_ITEM,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_IDENTIFIER,
    NODE_INT_LITERAL,
    NODE_REAL_LITERAL,
    NODE_BOOL_LITERAL,
    NODE_STRING_LITERAL
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

/* AST节点结构 */
typedef struct ASTNode {
    NodeType type;
    DataType data_type;
    Value value;
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *condition;
    struct ASTNode *statements;
    struct ASTNode *else_statements;
    struct ASTNode *case_value;
    struct ASTNode *case_list;
    struct ASTNode *func_list;
    VarDecl *param_list;
    DataType return_type;
    struct ASTNode *next;
    OpType op_type;
    char *identifier;
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
ASTNode *create_function_node(char *func_name, DataType return_type, VarDecl *param_list, ASTNode *statements);
ASTNode *create_function_block_node(char *fb_name, VarDecl *param_list, ASTNode *statements);
ASTNode *create_return_node(ASTNode *return_value);

/* 变量管理函数 */
VarDecl *create_var_decl(char *name, DataType type);
int add_global_variable(VarDecl *var);
int add_global_function(ASTNode *func);
VarDecl *find_variable(char *name);
ASTNode *find_global_function(char *name);
VarDecl *get_variable_table();
ASTNode *get_function_table();

/* AST遍历和打印函数 */
void print_ast(ASTNode *node, int indent);
void free_ast(ASTNode *node);
void clear_global_functions(void);

#endif // AST_H