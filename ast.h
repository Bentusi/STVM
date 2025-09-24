/*
 * IEC61131 抽象语法树 (AST) 定义
 * 定义语法树节点类型和操作函数
 */

#ifndef AST_H
#define AST_H

#include "symbol_table.h"
#include <stdint.h>
#include <stdbool.h>

/* 前向声明 */
struct type_info;
struct symbol;

/* AST节点类型枚举 - 完整ST语言支持 */
typedef enum {
    /* 顶层节点 */
    AST_PROGRAM,            // 程序根节点
    AST_LIBRARY,            // 库节点
    AST_IMPORT,             // 导入节点
    AST_IMPORT_LIST,        // 导入列表
    
    /* 声明节点 */
    AST_VAR_DECL,           // 变量声明
    AST_VAR_ITEM,           // 变量项
    AST_FUNCTION_DECL,      // 函数声明
    AST_FUNCTION_BLOCK_DECL,// 函数块声明
    AST_PARAM_DECL,         // 参数声明
    AST_TYPE_DECL,          // 类型声明
    
    /* 语句节点 */
    AST_ASSIGNMENT,         // 赋值语句
    AST_IF_STMT,            // 条件语句
    AST_ELSIF_STMT,         // ELSIF语句
    AST_CASE_STMT,          // CASE语句
    AST_CASE_ITEM,          // CASE项
    AST_FOR_STMT,           // FOR循环
    AST_WHILE_STMT,         // WHILE循环
    AST_REPEAT_STMT,        // REPEAT循环
    AST_RETURN_STMT,        // RETURN语句
    AST_EXIT_STMT,          // EXIT语句
    AST_CONTINUE_STMT,      // CONTINUE语句
    AST_EXPRESSION_STMT,    // 表达式语句
    
    /* 表达式节点 */
    AST_BINARY_OP,          // 二元操作
    AST_UNARY_OP,           // 一元操作
    AST_FUNCTION_CALL,      // 函数调用
    AST_ARRAY_ACCESS,       // 数组访问
    AST_MEMBER_ACCESS,      // 成员访问
    AST_LITERAL,            // 字面量
    AST_IDENTIFIER,         // 标识符
    
    /* 类型节点 */
    AST_BASIC_TYPE,         // 基本类型
    AST_ARRAY_TYPE,         // 数组类型
    AST_STRUCT_TYPE,        // 结构体类型
    AST_ENUM_TYPE,          // 枚举类型
    AST_USER_TYPE,          // 用户定义类型
    
    /* 列表节点 */
    AST_DECLARATION_LIST,   // 声明列表
    AST_STATEMENT_LIST,     // 语句列表
    AST_EXPRESSION_LIST,    // 表达式列表
    AST_PARAMETER_LIST,     // 参数列表
    AST_CASE_LIST,          // CASE列表
    AST_ELSIF_LIST,         // ELSIF列表
    AST_STRUCT_MEMBER_LIST, // 结构体成员列表
    AST_ENUM_VALUE_LIST     // 枚举值列表
} ast_node_type_t;

/* 操作符类型 - 完整运算符支持 */
typedef enum {
    /* 算术运算 */
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, OP_POWER,
    /* 逻辑运算 */
    OP_AND, OP_OR, OP_XOR, OP_NOT,
    /* 比较运算 */
    OP_EQ, OP_NE, OP_LT, OP_LE, OP_GT, OP_GE,
    /* 一元运算 */
    OP_NEG, OP_POS,
    /* 赋值运算 */
    OP_ASSIGN
} operator_type_t;

/* 字面量类型 */
typedef enum { 
    LITERAL_INT, 
    LITERAL_REAL, 
    LITERAL_STRING, 
    LITERAL_WSTRING,
    LITERAL_BOOL,
    LITERAL_TIME,
    LITERAL_DATE,
    LITERAL_TOD,
    LITERAL_DT
} literal_type_t;

/* 参数类型 */
typedef enum {
    PARAM_INPUT,            // 输入参数
    PARAM_OUTPUT,           // 输出参数
    PARAM_IN_OUT            // 输入输出参数
} param_category_t;

/* 源码位置信息 */
typedef struct source_location {
    int line;               // 行号
    int column;             // 列号
    char *filename;         // 文件名
} source_location_t;

/* AST节点基础结构 */
typedef struct ast_node {
    ast_node_type_t type;           // 节点类型
    source_location_t *loc;         // 源码位置信息
    struct type_info *data_type;    // 数据类型信息
    
    union {
        /* 程序节点 */
        struct {
            char *name;                     // 程序名
            struct ast_node *imports;       // 导入列表
            struct ast_node *declarations;  // 声明列表
            struct ast_node *statements;    // 语句列表
        } program;
        
        /* 库节点 */
        struct {
            char *name;                     // 库名
            char *version;                  // 版本
            struct ast_node *declarations;  // 声明列表
            struct ast_node *exports;       // 导出列表
        } library;
        
        /* 导入节点 */
        struct {
            char *library_name;             // 库名
            char *alias;                    // 别名
            char *path;                     // 路径
            struct ast_node *symbols;       // 符号列表
            bool is_selective;              // 是否选择性导入
        } import;
        
        /* 变量声明节点 */
        struct {
            var_category_t category;        // 变量类别
            struct ast_node *var_list;      // 变量列表
        } var_decl;
        
        /* 变量项节点 */
        struct {
            char *name;                     // 变量名
            struct type_info *type;         // 类型
            struct ast_node *init_value;    // 初始值
            struct ast_node *array_bounds;  // 数组边界
            bool is_constant;               // 是否常量
            bool is_retain;                 // 是否保持
        } var_item;
        
        /* 函数声明节点 */
        struct {
            char *name;                     // 函数名
            function_category_t func_type;      // 函数类型
            struct ast_node *parameters;    // 参数列表
            struct type_info *return_type;  // 返回类型
            struct ast_node *declarations;  // 局部声明
            struct ast_node *statements;    // 语句列表
            bool is_exported;               // 是否导出
            void *builtin_impl;             // 内置函数实现
        } function_decl;
        
        /* 参数声明节点 */
        struct {
            char *name;                     // 参数名
            struct type_info *type;         // 参数类型
            param_category_t category;      // 参数类别
            struct ast_node *default_value; // 默认值
        } param_decl;
        
        /* 类型声明节点 */
        struct {
            char *name;                     // 类型名
            struct type_info *type_def;     // 类型定义
        } type_decl;
        
        /* 赋值语句节点 */
        struct {
            struct ast_node *target;        // 赋值目标
            struct ast_node *value;         // 赋值值
        } assignment;
        
        /* 条件语句节点 */
        struct {
            struct ast_node *condition;     // 条件表达式
            struct ast_node *then_stmt;     // THEN语句
            struct ast_node *elsif_list;    // ELSIF列表
            struct ast_node *else_stmt;     // ELSE语句
        } if_stmt;
        
        /* ELSIF语句节点 */
        struct {
            struct ast_node *condition;     // 条件表达式
            struct ast_node *statements;    // 语句列表
        } elsif_stmt;
        
        /* CASE语句节点 */
        struct {
            struct ast_node *selector;      // 选择表达式
            struct ast_node *case_list;     // CASE项列表
            struct ast_node *else_stmt;     // ELSE语句
        } case_stmt;
        
        /* CASE项节点 */
        struct {
            struct ast_node *values;        // 匹配值列表
            struct ast_node *statements;    // 执行语句
        } case_item;
        
        /* FOR循环节点 */
        struct {
            char *var_name;                 // 循环变量
            struct ast_node *start_value;   // 起始值
            struct ast_node *end_value;     // 结束值
            struct ast_node *by_value;      // 步长值
            struct ast_node *statements;    // 循环体
        } for_stmt;
        
        /* WHILE循环节点 */
        struct {
            struct ast_node *condition;     // 循环条件
            struct ast_node *statements;    // 循环体
        } while_stmt;
        
        /* REPEAT循环节点 */
        struct {
            struct ast_node *statements;    // 循环体
            struct ast_node *condition;     // 终止条件
        } repeat_stmt;
        
        /* RETURN语句节点 */
        struct {
            struct ast_node *return_value;  // 返回值
        } return_stmt;
        
        /* 二元操作节点 */
        struct {
            operator_type_t op;             // 操作符
            struct ast_node *left;          // 左操作数  
            struct ast_node *right;         // 右操作数
        } binary_op;
        
        /* 一元操作节点 */
        struct {
            operator_type_t op;             // 操作符
            struct ast_node *operand;       // 操作数
        } unary_op;
        
        /* 函数调用节点 */
        struct {
            char *function_name;            // 函数名
            char *library_name;             // 库名（可选）
            struct ast_node *arguments;     // 参数列表
            struct symbol *symbol_ref;      // 符号引用
        } function_call;
        
        /* 数组访问节点 */
        struct {
            struct ast_node *array;         // 数组表达式
            struct ast_node *indices;       // 索引列表
        } array_access;
        
        /* 成员访问节点 */
        struct {
            struct ast_node *object;        // 对象表达式
            char *member_name;              // 成员名
        } member_access;
        
        /* 字面量节点 */
        struct {
            literal_type_t literal_type;    // 字面量类型
            union {
                int32_t int_val;            // 整数值
                double real_val;            // 实数值
                char *string_val;           // 字符串值
                bool bool_val;              // 布尔值
                uint64_t time_val;          // 时间值（毫秒）
                uint32_t date_val;          // 日期值
            } value;
        } literal;
        
        /* 标识符节点 */
        struct {
            char *name;                     // 标识符名
            struct symbol *symbol_ref;      // 符号引用
        } identifier;
        
        /* 数组类型节点 */
        struct {
            struct type_info *element_type; // 元素类型
            struct ast_node *bounds;        // 边界列表
        } array_type;
        
        /* 结构体类型节点 */
        struct {
            struct ast_node *members;       // 成员列表
        } struct_type;
        
        /* 枚举类型节点 */
        struct {
            struct ast_node *values;        // 枚举值列表
        } enum_type;
        
        /* 列表节点 */
        struct {
            struct ast_node **items;        // 项目数组
            uint32_t count;                 // 项目数量
            uint32_t capacity;              // 容量
        } list;
        
    } data;
    
    struct ast_node *next;          // 链表下一个节点
} ast_node_t;

/* AST节点创建函数声明 - 顶层节点 */
ast_node_t* ast_create_program(const char *name, ast_node_t *imports, 
                              ast_node_t *declarations, ast_node_t *statements);
ast_node_t* ast_create_library(const char *name, const char *version, 
                              ast_node_t *declarations, ast_node_t *exports);
ast_node_t* ast_create_import(const char *library_name, const char *alias, 
                             const char *path, ast_node_t *symbols, bool is_selective);

/* 声明节点创建函数 */
ast_node_t* ast_create_var_declaration(var_category_t category, ast_node_t *var_list);
ast_node_t* ast_create_var_item(const char *name, struct type_info *type, 
                                ast_node_t *init_value, ast_node_t *array_bounds);
ast_node_t* ast_create_function_declaration(const char *name, function_category_t func_type,
                                           ast_node_t *parameters, struct type_info *return_type,
                                           ast_node_t *declarations, ast_node_t *statements);
ast_node_t* ast_create_parameter(const char *name, struct type_info *type, 
                                param_category_t category, ast_node_t *default_value);
ast_node_t* ast_create_type_declaration(const char *name, struct type_info *type_def);

/* 语句节点创建函数 */
ast_node_t* ast_create_assignment(ast_node_t *target, ast_node_t *value);
ast_node_t* ast_create_if_statement(ast_node_t *condition, ast_node_t *then_stmt,
                                   ast_node_t *elsif_list, ast_node_t *else_stmt);
ast_node_t* ast_create_elsif_statement(ast_node_t *condition, ast_node_t *statements);
ast_node_t* ast_create_case_statement(ast_node_t *selector, ast_node_t *case_list,
                                     ast_node_t *else_stmt);
ast_node_t* ast_create_case_item(ast_node_t *values, ast_node_t *statements);
ast_node_t* ast_create_for_statement(const char *var_name, ast_node_t *start_value,
                                    ast_node_t *end_value, ast_node_t *by_value,
                                    ast_node_t *statements);
ast_node_t* ast_create_while_statement(ast_node_t *condition, ast_node_t *statements);
ast_node_t* ast_create_repeat_statement(ast_node_t *statements, ast_node_t *condition);
ast_node_t* ast_create_return_statement(ast_node_t *return_value);
ast_node_t* ast_create_exit_statement(void);
ast_node_t* ast_create_continue_statement(void);
ast_node_t* ast_create_expression_statement(ast_node_t *expression);

/* 表达式节点创建函数 */
ast_node_t* ast_create_binary_operation(operator_type_t op, ast_node_t *left, ast_node_t *right);
ast_node_t* ast_create_unary_operation(operator_type_t op, ast_node_t *operand);
ast_node_t* ast_create_function_call(const char *function_name, const char *library_name,
                                    ast_node_t *arguments);
ast_node_t* ast_create_array_access(ast_node_t *array, ast_node_t *indices);
ast_node_t* ast_create_member_access(ast_node_t *object, const char *member_name);
ast_node_t* ast_create_literal(literal_type_t type, const void *value);
ast_node_t* ast_create_identifier(const char *name);

/* 类型节点创建函数 */
ast_node_t* ast_create_array_type(struct type_info *element_type, ast_node_t *bounds);
ast_node_t* ast_create_struct_type(ast_node_t *members);
ast_node_t* ast_create_enum_type(ast_node_t *values);

/* 列表节点操作函数 */
ast_node_t* ast_create_list(ast_node_type_t list_type);
ast_node_t* ast_list_append(ast_node_t *list, ast_node_t *item);
ast_node_t* ast_list_prepend(ast_node_t *list, ast_node_t *item);
uint32_t ast_list_size(ast_node_t *list);
ast_node_t* ast_list_get(ast_node_t *list, uint32_t index);

/* AST遍历和操作函数 */
typedef enum {
    AST_VISIT_CONTINUE,     // 继续遍历
    AST_VISIT_SKIP,         // 跳过子节点
    AST_VISIT_STOP          // 停止遍历
} ast_visit_result_t;

typedef ast_visit_result_t (*ast_visitor_fn)(ast_node_t *node, void *context);

void ast_traverse_preorder(ast_node_t *node, ast_visitor_fn visitor, void *context);
void ast_traverse_postorder(ast_node_t *node, ast_visitor_fn visitor, void *context);

/* AST节点管理函数 */
void ast_free_node(ast_node_t *node);
ast_node_t* ast_clone_node(ast_node_t *node);
bool ast_nodes_equal(ast_node_t *node1, ast_node_t *node2);

/* AST调试和打印函数 */
void ast_print_tree(ast_node_t *node, int indent);
void ast_print_node_info(ast_node_t *node);
const char* ast_node_type_name(ast_node_type_t type);
const char* ast_operator_name(operator_type_t op);

/* AST验证函数 */
bool ast_validate_tree(ast_node_t *root);
bool ast_check_node_consistency(ast_node_t *node);

/* 全局AST根节点管理 */
void ast_set_root(ast_node_t *root);
ast_node_t* ast_get_root(void);

/* AST统计信息 */
typedef struct ast_statistics {
    uint32_t total_nodes;           // 总节点数
    uint32_t node_counts[64];       // 各类型节点数量
    uint32_t max_depth;             // 最大深度
    uint32_t total_memory;          // 总内存使用
} ast_static_t;

ast_static_t* ast_get_static(void);
void ast_reset_static(void);

#endif /* AST_H */