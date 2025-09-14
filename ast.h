/*
 * IEC61131 抽象语法树 (AST) 定义
 * 定义语法树节点类型和操作函数
 */

#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>

/* 数据类型枚举 */
typedef enum {
    TYPE_BOOL,
    TYPE_INT,
    TYPE_REAL,
    TYPE_STRING,
    TYPE_VOID
} DataType;

/* AST节点类型 */
typedef enum {
    NODE_COMPILATION_UNIT,
    NODE_PROGRAM,
    NODE_FUNCTION,
    NODE_FUNCTION_BLOCK,
    NODE_FUNCTION_CALL,
    NODE_VAR_SECTION,
    NODE_PARAM,
    NODE_PARAM_LIST,
    NODE_RETURN,
    NODE_STATEMENT_LIST,
    NODE_ASSIGN,
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

/* 操作符类型 */
typedef enum {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_AND, OP_OR, OP_NOT, OP_NEG,
    OP_EQ, OP_NE, OP_LT, OP_LE, OP_GT, OP_GE
} OpType;

/* 参数方向 */
typedef enum {
    PARAM_INPUT,
    PARAM_OUTPUT,
    PARAM_INOUT,
    PARAM_VALUE
} ParamDirection;

/* 前向声明 */
typedef struct ASTNode ASTNode;
typedef struct VarDecl VarDecl;
typedef struct FunctionDecl FunctionDecl;
typedef struct ParamDecl ParamDecl;

/* 参数声明结构 */
struct ParamDecl {
    char *name;
    DataType type;
    ParamDirection direction;
    ASTNode *default_value;  /* 新增：默认值 */
    struct ParamDecl *next;
};

/* 函数声明结构 */
struct FunctionDecl {
    char *name;
    DataType return_type;
    ParamDecl *params;
    ASTNode *body;
    struct FunctionDecl *next;
};

/* 变量声明结构 */
struct VarDecl {
    char *name;
    DataType type;
    struct VarDecl *next;
};

/* 值联合体声明 */
typedef union {
    int int_val;
    double real_val;
    int bool_val;
    char *str_val;
} Value;

/* AST节点结构 */
struct ASTNode {
    NodeType type;
    DataType data_type;
    
    /* 基本字段 */
    char *identifier;
    OpType op_type;
    
    /* 值联合体 */
    Value value;
    
    /* 函数相关字段 */
    DataType return_type;
    ParamDecl *params;
    VarDecl *var_decls;
    VarDecl *local_vars;
    ASTNode *arguments;
    ASTNode *return_value;
    
    /* 子节点指针 */
    ASTNode *left;
    ASTNode *right;
    ASTNode *condition;
    ASTNode *statements;
    ASTNode *else_statements;
    ASTNode *next;
};

/* 函数声明 */

/* AST节点创建函数 */
ASTNode *create_compilation_unit_node(ASTNode *declarations, ASTNode *program);
ASTNode *create_program_node(char *name, ASTNode *statements);
ASTNode *create_statement_list(ASTNode *statement);
ASTNode *add_statement(ASTNode *list, ASTNode *statement);

/* 函数相关创建函数 - 修正签名 */
ASTNode *create_function_node(char *name, ParamDecl *params, DataType return_type, ASTNode *body);
ASTNode *create_function_block_node(char *name, DataType return_type, ASTNode *body);
ASTNode *create_function_call_node(char *name, ASTNode *arguments);
ASTNode *create_return_node(ASTNode *value);
ASTNode *create_function_node_complete(char *name, ParamDecl *params, DataType return_type, VarDecl *local_vars, ASTNode *body);
ASTNode *create_function_block_node_complete(char *name, ParamDecl *params, VarDecl *local_vars, ASTNode *body);
ASTNode *create_var_section_node(VarDecl *var_list);
ParamDecl *add_param(ParamDecl *list, ParamDecl *param);
ParamDecl *append_param_list(ParamDecl *list1, ParamDecl *list2);
VarDecl *append_var_list(VarDecl *list1, VarDecl *list2);

/* 参数相关函数 */
ParamDecl *create_param_node(char *name, DataType type);
ParamDecl *create_input_param_node(char *name, DataType type);
ParamDecl *create_output_param_node(char *name, DataType type);
ParamDecl *create_inout_param_node(char *name, DataType type);
ParamDecl *add_param(ParamDecl *list, ParamDecl *param);

/* 其他节点创建函数 */
ASTNode *create_assign_node(char *var_name, ASTNode *expr);
ASTNode *create_if_node(ASTNode *condition, ASTNode *then_stmt, ASTNode *else_stmt);
ASTNode *create_for_node(char *var_name, ASTNode *start, ASTNode *end, ASTNode *statements);
ASTNode *create_while_node(ASTNode *condition, ASTNode *statements);
ASTNode *create_case_node(ASTNode *expression, ASTNode *case_list);
ASTNode *create_case_item(ASTNode *value, ASTNode *statements);
ASTNode *create_binary_op_node(OpType op, ASTNode *left, ASTNode *right);
ASTNode *create_unary_op_node(OpType op, ASTNode *operand);

/* 字面量节点创建函数 */
ASTNode *create_identifier_node(char *name);
ASTNode *create_int_literal_node(int value);
ASTNode *create_real_literal_node(double value);
ASTNode *create_bool_literal_node(int value);
ASTNode *create_string_literal_node(char *value);

/* 变量相关函数 */
VarDecl *create_var_decl(char *name, DataType type);
void add_variable(VarDecl *var);
VarDecl *find_variable(char *name);

/* 函数表管理 */
void add_function(FunctionDecl *func);
FunctionDecl *find_function(char *name);
FunctionDecl *create_function_decl(char *name, DataType return_type, ParamDecl *params, ASTNode *body);

/* 工具函数 */
void print_ast(ASTNode *node, int indent);
void print_params(ParamDecl *params, int indent);
void free_ast(ASTNode *node);
void free_params(ParamDecl *params);
void print_vars(VarDecl *vars, int indent); 
void free_vars(VarDecl *vars);
void print_functions();

/* 类型检查函数 */
DataType get_expression_type(ASTNode *expr);
int check_function_call(char *func_name, ASTNode *arguments);

#endif // AST_H