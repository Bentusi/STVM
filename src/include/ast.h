/**
 * @file ast.h
 * @brief 抽象语法树（AST）定义
 */

#ifndef STVM_AST_H
#define STVM_AST_H

#include <stdint.h>
#include <stdbool.h>
#include "types.h"

/**
 * @brief AST节点类型
 */
typedef enum {
    // === 程序结构 ===
    AST_PROGRAM,            // 程序根节点
    AST_VAR_DECL,           // 变量声明
    AST_FUNCTION_DECL,      // 函数声明
    AST_IMPORT,             // 导入语句
    
    // === 语句 ===
    AST_ASSIGN,             // 赋值语句
    AST_IF,                 // if语句
    AST_WHILE,              // while循环
    AST_FOR,                // for循环
    AST_REPEAT,             // repeat循环
    AST_CASE,               // case语句
    AST_CASE_ELEMENT,       // case分支元素
    AST_RETURN,             // return语句
    AST_BLOCK,              // 语句块
    AST_EXPR_STMT,          // 表达式语句
    
    // === 表达式 ===
    AST_BINARY_OP,          // 二元运算
    AST_UNARY_OP,           // 一元运算
    AST_LITERAL,            // 字面量
    AST_IDENTIFIER,         // 标识符
    AST_FUNCTION_CALL,      // 函数调用
    AST_ARRAY_ACCESS,       // 数组访问
    AST_MEMBER_ACCESS,      // 成员访问（质量化类型的.VAL/.QUALITY）
    
    // === 类型 ===
    AST_TYPE_SPEC,          // 类型规范
    AST_ARRAY_TYPE          // 数组类型
} ASTNodeType;

/**
 * @brief 二元运算符
 */
typedef enum {
    // 算术运算
    BINOP_ADD,      // +
    BINOP_SUB,      // -
    BINOP_MUL,      // *
    BINOP_DIV,      // /
    BINOP_MOD,      // MOD
    
    // 比较运算
    BINOP_EQ,       // =
    BINOP_NE,       // <>
    BINOP_LT,       // <
    BINOP_LE,       // <=
    BINOP_GT,       // >
    BINOP_GE,       // >=
    
    // 逻辑运算
    BINOP_AND,      // AND（逻辑与）
    BINOP_OR,       // OR（逻辑或）
    BINOP_XOR,      // XOR（逻辑异或）
    
    // 位运算（IEC 61131-3）
    BINOP_BIT_AND,  // &（位与）
    BINOP_BIT_OR,   // |（位或）
    BINOP_BIT_XOR,  // ^（位异或）
    
    // 移位运算
    BINOP_SHL,      // SHL（左移）
    BINOP_SHR       // SHR（右移）
} BinaryOp;

/**
 * @brief 一元运算符
 */
typedef enum {
    UNOP_NEG,       // - (取负)
    UNOP_NOT,       // NOT (逻辑非)
    UNOP_BIT_NOT    // ~（位取反）
} UnaryOp;

/**
 * @brief 成员访问类型（质量化类型的成员）
 */
typedef enum {
    MEMBER_VAL,     // .VAL - 访问值部分
    MEMBER_QUALITY  // .QUALITY - 访问质量位部分
} MemberType;

/**
 * @brief 源码位置信息
 */
typedef struct {
    const char* filename;   // 文件名
    int line;               // 行号
    int column;             // 列号
} SourceLocation;

// 前向声明
typedef struct ASTNode ASTNode;

/**
 * @brief AST节点数据联合体
 */
typedef union {
    // 程序节点
    struct {
        char* name;                 // 程序名
        ASTNode* imports;           // 导入语句列表
        ASTNode* var_decls;         // 变量声明列表
        ASTNode* functions;         // 函数列表
        ASTNode* body;              // 主程序语句
    } program;
    
    // 变量声明
    struct {
        char* name;                 // 变量名
        TypeInfo* type;             // 类型信息
        ASTNode* initializer;       // 初始化表达式（可选）
        bool is_const;              // 是否为常量
        bool is_global;             // 是否为全局变量（函数内VAR块）
        bool is_external;           // 是否为外部 I/O 变量
        char* io_address;           // I/O 地址字符串（如 "%IX0.0"）
    } var_decl;
    
    // 函数声明
    struct {
        char* name;                 // 函数名
        ASTNode* params;        // 参数列表
        TypeInfo* return_type;      // 返回类型
        ASTNode* declarations;      // 局部变量声明
        ASTNode* body;              // 函数体
    } function_decl;
    
    // 导入语句
    struct {
        char* module_name;          // 模块名
        char** symbols;             // 导入的符号列表（NULL表示全部导入）
        int symbol_count;           // 符号数量
        char** aliases;             // 别名列表（可选）
    } import;
    
    // 赋值语句
    struct {
        ASTNode* target;            // 赋值目标（标识符或数组访问）
        ASTNode* value;             // 赋值值
    } assign;
    
    // if语句
    struct {
        ASTNode* condition;         // 条件表达式
        ASTNode* then_branch;       // then分支
        ASTNode* else_branch;       // else分支（可选）
    } if_stmt;
    
    // while循环
    struct {
        ASTNode* condition;         // 循环条件
        ASTNode* body;              // 循环体
    } while_stmt;
    
    // for循环
    struct {
        char* variable;             // 循环变量
        ASTNode* start;             // 起始值
        ASTNode* end;               // 结束值
        ASTNode* step;              // 步长（可选，默认1）
        ASTNode* body;              // 循环体
    } for_stmt;
    
    // repeat循环
    struct {
        ASTNode* body;              // 循环体
        ASTNode* condition;         // 终止条件（UNTIL）
    } repeat_stmt;
    
    // case语句
    struct {
        ASTNode* expression;        // 选择表达式
        ASTNode** cases;            // case分支数组
        int case_count;             // case数量
        ASTNode* default_case;      // 默认分支（可选）
    } case_stmt;
    
    // case分支元素
    struct {
        ASTNode* labels;            // 标签列表（链表）
        ASTNode* statements;        // 语句列表
    } case_element;
    
    // return语句
    struct {
        ASTNode* value;             // 返回值（可选）
    } return_stmt;
    
    // 二元运算
    struct {
        BinaryOp op;                // 运算符
        ASTNode* left;              // 左操作数
        ASTNode* right;             // 右操作数
    } binary_op;
    
    // 一元运算
    struct {
        UnaryOp op;                 // 运算符
        ASTNode* operand;           // 操作数
    } unary_op;
    
    // 字面量
    struct {
        Value value;                // 值
    } literal;
    
    // 标识符
    struct {
        char* name;                 // 标识符名称
    } identifier;
    
    // 函数调用
    struct {
        char* name;                 // 函数名
        ASTNode** arguments;        // 参数列表
        int arg_count;              // 参数个数
    } function_call;
    
    // 数组访问
    struct {
        ASTNode* array;             // 数组表达式
        ASTNode* index;             // 索引表达式
    } array_access;
    
    // 成员访问（质量化类型的.VAL/.QUALITY）
    struct {
        ASTNode* object;            // 对象表达式（质量化类型变量）
        MemberType member;          // 成员类型（VAL或QUALITY）
    } member_access;
    
    // 类型规范
    struct {
        TypeInfo* type_info;        // 类型信息
    } type_spec;
} ASTNodeData;

/**
 * @brief AST节点结构
 */
struct ASTNode {
    ASTNodeType type;               // 节点类型
    SourceLocation location;        // 源码位置
    ASTNodeData data;               // 节点数据
    ASTNode* next;                  // 链表指针（用于列表节点）
    
    // 类型检查后的信息
    TypeInfo* resolved_type;        // 解析后的类型（表达式用）
};

/**
 * @brief 创建程序节点
 */
ASTNode* ast_create_program(const char* name, ASTNode* imports, ASTNode* declarations, ASTNode* functions, ASTNode* statements);

/**
 * @brief 创建变量声明节点
 */
ASTNode* ast_create_var_decl(const char* name, TypeInfo* type, ASTNode* initializer, bool is_const, bool is_global);

/**
 * @brief 创建外部 I/O 变量声明节点
 */
ASTNode* ast_create_external_var_decl(const char* name, TypeInfo* type, const char* io_address);

/**
 * @brief 创建函数声明节点
 */
ASTNode* ast_create_function_decl(const char* name, ASTNode* parameters, TypeInfo* return_type, 
                                   ASTNode* declarations, ASTNode* body);

/**
 * @brief 创建导入语句节点
 */
ASTNode* ast_create_import(const char* module_name, char** symbols, int symbol_count, char** aliases);

/**
 * @brief 创建赋值语句节点
 */
ASTNode* ast_create_assign(ASTNode* target, ASTNode* value);

/**
 * @brief 创建if语句节点
 */
ASTNode* ast_create_if(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch);

/**
 * @brief 创建while循环节点
 */
ASTNode* ast_create_while(ASTNode* condition, ASTNode* body);

/**
 * @brief 创建for循环节点
 */
ASTNode* ast_create_for(const char* variable, ASTNode* start, ASTNode* end, ASTNode* step, ASTNode* body);

/**
 * @brief 创建repeat循环节点
 */
ASTNode* ast_create_repeat(ASTNode* body, ASTNode* condition);

/**
 * @brief 创建return语句节点
 */
ASTNode* ast_create_return(ASTNode* value);

/**
 * @brief 创建case语句节点
 */
ASTNode* ast_create_case(ASTNode* expression, ASTNode** cases, int case_count, ASTNode* default_case);

/**
 * @brief 创建case分支元素节点
 */
ASTNode* ast_create_case_element(ASTNode* labels, ASTNode* statements);

/**
 * @brief 创建语句块节点
 */
ASTNode* ast_create_block(ASTNode* statements);

/**
 * @brief 创建二元运算节点
 */
ASTNode* ast_create_binary_op(BinaryOp op, ASTNode* left, ASTNode* right);

/**
 * @brief 创建一元运算节点
 */
ASTNode* ast_create_unary_op(UnaryOp op, ASTNode* operand);

/**
 * @brief 创建字面量节点
 */
ASTNode* ast_create_literal(Value value);

/**
 * @brief 创建标识符节点
 */
ASTNode* ast_create_identifier(const char* name);

/**
 * @brief 创建函数调用节点
 */
ASTNode* ast_create_function_call(const char* name, ASTNode** arguments, int arg_count);

/**
 * @brief 创建数组访问节点
 */
ASTNode* ast_create_array_access(ASTNode* array, ASTNode* index);

/**
 * @brief 创建成员访问节点（质量化类型的.VAL/.QUALITY）
 */
ASTNode* ast_create_member_access(ASTNode* object, MemberType member);

/**
 * @brief 将节点添加到列表末尾
 */
ASTNode* ast_append_node(ASTNode* list, ASTNode* node);

/**
 * @brief 释放AST节点及其子节点
 */
void ast_free_node(ASTNode* node);

/**
 * @brief 设置节点的源码位置
 */
void ast_set_location(ASTNode* node, const char* filename, int line, int column);

/**
 * @brief 获取二元运算符的字符串表示
 */
const char* binary_op_to_string(BinaryOp op);

/**
 * @brief 获取一元运算符的字符串表示
 */
const char* unary_op_to_string(UnaryOp op);

/**
 * @brief 打印AST（调试用）
 */
void ast_print(ASTNode* node, int indent);

/**
 * @brief 访问者模式遍历AST
 */
typedef void (*ASTVisitor)(ASTNode* node, void* context);
void ast_visit(ASTNode* node, ASTVisitor visitor, void* context);

#endif // STVM_AST_H
