#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include "ast.h"
#include "symbol_table.h"
#include <stdint.h>
#include <stdbool.h>

/* 类型检查错误码 */
typedef enum {
    TYPE_CHECK_SUCCESS = 0,         // 检查成功
    TYPE_CHECK_TYPE_MISMATCH,       // 类型不匹配
    TYPE_CHECK_UNDEFINED_SYMBOL,    // 未定义符号
    TYPE_CHECK_REDEFINED_SYMBOL,    // 重定义符号
    TYPE_CHECK_INVALID_OPERATION,   // 无效操作
    TYPE_CHECK_INCOMPATIBLE_TYPES,  // 不兼容类型
    TYPE_CHECK_INVALID_ASSIGNMENT,  // 无效赋值
    TYPE_CHECK_FUNCTION_MISMATCH,   // 函数签名不匹配
    TYPE_CHECK_RETURN_TYPE_MISMATCH, // 返回类型不匹配
    TYPE_CHECK_ARRAY_INDEX_ERROR,   // 数组索引错误
    TYPE_CHECK_CONST_ASSIGNMENT,    // 常量赋值错误
    TYPE_CHECK_MEMORY_ERROR         // 内存错误
} type_check_error_t;

/* 类型检查严重级别 */
typedef enum {
    TYPE_SEVERITY_ERROR,            // 错误（必须修复）
    TYPE_SEVERITY_WARNING,          // 警告（建议修复）
    TYPE_SEVERITY_INFO             // 信息（提示）
} type_severity_t;

/* 类型检查消息 */
typedef struct type_check_message {
    type_check_error_t error_code;  // 错误码
    type_severity_t severity;       // 严重级别
    char *message;              // 错误消息
    source_location_t *location;    // 错误位置
    struct type_check_message *next; // 下一个消息
} type_check_message_t;

/* 类型兼容性规则 */
typedef struct type_compatibility {
    type_info_t *from_type;         // 源类型
    type_info_t *to_type;           // 目标类型
    bool is_implicit_allowed;       // 是否允许隐式转换
    bool is_explicit_allowed;       // 是否允许显式转换
} type_compatibility_t;

/* 函数签名信息 */
typedef struct function_signature {
    char *name;                     // 函数名
    type_info_t *return_type;       // 返回类型
    type_info_t **param_types;      // 参数类型数组
    char **param_names;             // 参数名数组
    uint32_t param_count;           // 参数数量
    bool is_variadic;               // 是否可变参数
    bool is_builtin;                // 是否内置函数
} function_signature_t;

/* 类型检查上下文 */
typedef struct type_checker_context {
    symbol_table_manager_t *sym_mgr; // 符号表管理器
    
    /* 当前检查状态 */
    struct {
        function_signature_t *current_function; // 当前函数
        type_info_t *expected_return_type;      // 期望返回类型
        uint32_t scope_depth;                   // 作用域深度
        bool in_loop;                           // 是否在循环中
        bool in_function;                       // 是否在函数中
    } current_context;
    
    /* 错误和警告收集 */
    type_check_message_t *messages; // 消息链表
    uint32_t error_count;           // 错误数量
    uint32_t warning_count;         // 警告数量
    
    /* 类型兼容性表 */
    type_compatibility_t *compatibility_table; // 兼容性规则
    uint32_t compatibility_count;   // 规则数量
    
    /* 内置类型定义 */
    type_info_t *builtin_types[TYPE_MAX_ID]; // 内置类型数组
    uint32_t builtin_type_count;    // 内置类型数量
    
    /* 函数签名表 */
    function_signature_t *function_signatures; // 函数签名
    uint32_t function_count;        // 函数数量
    
    /* 配置选项 */
    struct {
        bool strict_mode;           // 严格模式
        bool warnings_as_errors;    // 警告视为错误
        bool check_unused_vars;     // 检查未使用变量
        bool check_uninitialized;   // 检查未初始化变量
        bool allow_implicit_conv;   // 允许隐式转换
    } options;
    
    /* 统计信息 */
    struct {
        uint32_t nodes_checked;     // 检查的节点数
        uint32_t symbols_resolved;  // 解析的符号数
        uint32_t types_inferred;    // 推断的类型数
    } statistics;
    
} type_checker_context_t;

/* 类型推断结果 */
typedef struct type_inference_result {
    type_info_t *inferred_type;     // 推断出的类型
    bool is_constant;               // 是否常量
    bool is_lvalue;                 // 是否左值
    union {
        int32_t int_val;            // 常量整数值
        double real_val;            // 常量实数值
        bool bool_val;              // 常量布尔值
        char *string_val;           // 常量字符串值
    } const_value;
} type_inference_result_t;

/* ========== 类型检查器管理函数 ========== */

/* 初始化和清理 */
type_checker_context_t* type_checker_create(void);
void type_checker_destroy(type_checker_context_t *ctx);
int type_checker_init(type_checker_context_t *ctx, symbol_table_manager_t *sym_mgr);

/* 配置管理 */
int type_checker_set_strict_mode(type_checker_context_t *ctx, bool enabled);
int type_checker_set_warnings_as_errors(type_checker_context_t *ctx, bool enabled);
int type_checker_enable_unused_check(type_checker_context_t *ctx, bool enabled);
int type_checker_allow_implicit_conversion(type_checker_context_t *ctx, bool enabled);

/* ========== 顶层检查函数 ========== */

/* 程序检查 */
int type_checker_check_program(type_checker_context_t *ctx, ast_node_t *program);
int type_checker_check_declarations(type_checker_context_t *ctx, ast_node_t *decl_list);
int type_checker_check_statements(type_checker_context_t *ctx, ast_node_t *stmt_list);

/* AST节点检查 */
int type_checker_check_node(type_checker_context_t *ctx, ast_node_t *node);

/* ========== 声明检查函数 ========== */

/* 变量声明检查 */
int type_checker_check_var_declaration(type_checker_context_t *ctx, ast_node_t *var_decl);
int type_checker_check_var_item(type_checker_context_t *ctx, ast_node_t *var_item);

/* 函数声明检查 */
int type_checker_check_function_declaration(type_checker_context_t *ctx, ast_node_t *func_decl);
int type_checker_check_function_parameters(type_checker_context_t *ctx, ast_node_t *params);
int type_checker_check_function_return(type_checker_context_t *ctx, ast_node_t *return_stmt);

/* ========== 语句检查函数 ========== */

/* 赋值语句检查 */
int type_checker_check_assignment(type_checker_context_t *ctx, ast_node_t *assignment);
int type_checker_check_lvalue(type_checker_context_t *ctx, ast_node_t *lvalue);

/* 控制流语句检查 */
int type_checker_check_if_statement(type_checker_context_t *ctx, ast_node_t *if_stmt);
int type_checker_check_for_statement(type_checker_context_t *ctx, ast_node_t *for_stmt);
int type_checker_check_while_statement(type_checker_context_t *ctx, ast_node_t *while_stmt);
int type_checker_check_case_statement(type_checker_context_t *ctx, ast_node_t *case_stmt);
int type_checker_check_return_statement(type_checker_context_t *ctx, ast_node_t *return_stmt);

/* ========== 表达式检查函数 ========== */

/* 表达式类型推断 */
type_inference_result_t* type_checker_infer_expression(type_checker_context_t *ctx, 
                                                      ast_node_t *expr);
type_inference_result_t* type_checker_infer_binary_op(type_checker_context_t *ctx, 
                                                     ast_node_t *binary_op);
type_inference_result_t* type_checker_infer_unary_op(type_checker_context_t *ctx, 
                                                    ast_node_t *unary_op);
type_inference_result_t* type_checker_infer_function_call(type_checker_context_t *ctx, 
                                                         ast_node_t *func_call);
type_inference_result_t* type_checker_infer_identifier(type_checker_context_t *ctx, 
                                                      ast_node_t *identifier);
type_inference_result_t* type_checker_infer_literal(type_checker_context_t *ctx, 
                                                   ast_node_t *literal);

/* 表达式验证 */
int type_checker_check_expression(type_checker_context_t *ctx, ast_node_t *expr, 
                                 type_info_t *expected_type);
int type_checker_check_condition(type_checker_context_t *ctx, ast_node_t *condition);

/* ========== 类型系统函数 ========== */

/* 类型兼容性检查 */
bool type_checker_is_compatible(type_checker_context_t *ctx, type_info_t *from_type, 
                               type_info_t *to_type);
bool type_checker_is_implicitly_convertible(type_checker_context_t *ctx, 
                                           type_info_t *from_type, type_info_t *to_type);
bool type_checker_is_explicitly_convertible(type_checker_context_t *ctx, 
                                           type_info_t *from_type, type_info_t *to_type);

/* 类型推导 */
type_info_t* type_checker_deduce_binary_result_type(type_checker_context_t *ctx,
                                                   operator_type_t op,
                                                   type_info_t *left_type,
                                                   type_info_t *right_type);
type_info_t* type_checker_deduce_unary_result_type(type_checker_context_t *ctx,
                                                  operator_type_t op,
                                                  type_info_t *operand_type);

/* 类型验证 */
bool type_checker_validate_binary_operation(type_checker_context_t *ctx,
                                           operator_type_t op,
                                           type_info_t *left_type,
                                           type_info_t *right_type);
bool type_checker_validate_unary_operation(type_checker_context_t *ctx,
                                          operator_type_t op,
                                          type_info_t *operand_type);

/* ========== 符号管理函数 ========== */

/* 符号解析 */
symbol_t* type_checker_resolve_symbol(type_checker_context_t *ctx, const char *name);
function_signature_t* type_checker_resolve_function(type_checker_context_t *ctx, 
                                                   const char *name);

/* 符号注册 */
int type_checker_register_symbol(type_checker_context_t *ctx, const char *name, 
                                type_info_t *type, symbol_type_t sym_type);
int type_checker_register_function(type_checker_context_t *ctx, 
                                  const function_signature_t *signature);

/* 作用域管理 */
int type_checker_enter_scope(type_checker_context_t *ctx, const char *scope_name);
int type_checker_exit_scope(type_checker_context_t *ctx);

/* ========== 函数签名管理 ========== */

/* 函数签名创建 */
function_signature_t* type_checker_create_function_signature(const char *name,
                                                           type_info_t *return_type,
                                                           type_info_t **param_types,
                                                           char **param_names,
                                                           uint32_t param_count);
void type_checker_destroy_function_signature(function_signature_t *signature);

/* 函数调用验证 */
int type_checker_check_function_call_compatibility(type_checker_context_t *ctx,
                                                  const function_signature_t *signature,
                                                  ast_node_t *args);
int type_checker_check_parameter_count(type_checker_context_t *ctx,
                                      const function_signature_t *signature,
                                      uint32_t actual_count);

/* ========== 内置类型和函数 ========== */

/* 内置类型初始化 */
int type_checker_init_builtin_types(type_checker_context_t *ctx);
type_info_t* type_checker_get_builtin_type(type_checker_context_t *ctx, int type_id);

/* 内置函数注册 */
int type_checker_register_builtin_functions(type_checker_context_t *ctx);
int type_checker_add_builtin_function(type_checker_context_t *ctx, const char *name,
                                     type_info_t *return_type, uint32_t param_count, ...);

/* ========== 错误和警告管理 ========== */

/* 消息添加 */
void type_checker_add_error(type_checker_context_t *ctx, type_check_error_t error_code,
                           source_location_t *location, const char *format, ...);
void type_checker_add_warning(type_checker_context_t *ctx, type_check_error_t error_code,
                             source_location_t *location, const char *format, ...);
void type_checker_add_info(type_checker_context_t *ctx, const char *format, ...);

/* 消息查询 */
uint32_t type_checker_get_error_count(const type_checker_context_t *ctx);
uint32_t type_checker_get_warning_count(const type_checker_context_t *ctx);
bool type_checker_has_errors(const type_checker_context_t *ctx);

/* 消息输出 */
void type_checker_print_messages(const type_checker_context_t *ctx);
void type_checker_print_summary(const type_checker_context_t *ctx);
char* type_checker_get_message_string(const type_checker_context_t *ctx);

/* 消息清理 */
void type_checker_clear_messages(type_checker_context_t *ctx);

/* ========== 常量求值 ========== */

/* 常量表达式求值 */
bool type_checker_is_constant_expression(type_checker_context_t *ctx, ast_node_t *expr);
type_inference_result_t* type_checker_evaluate_constant(type_checker_context_t *ctx, 
                                                       ast_node_t *expr);

/* 常量折叠 */
ast_node_t* type_checker_fold_constant(type_checker_context_t *ctx, ast_node_t *expr);

/* ========== 语义分析辅助 ========== */

/* 死代码检测 */
int type_checker_check_dead_code(type_checker_context_t *ctx, ast_node_t *node);

/* 未使用变量检测 */
int type_checker_check_unused_variables(type_checker_context_t *ctx);

/* 未初始化变量检测 */
int type_checker_check_uninitialized_variables(type_checker_context_t *ctx, ast_node_t *node);

/* 控制流分析 */
int type_checker_analyze_control_flow(type_checker_context_t *ctx, ast_node_t *node);

/* ========== 统计和诊断 ========== */

/* 统计信息 */
void type_checker_print_statistics(const type_checker_context_t *ctx);
uint32_t type_checker_get_nodes_checked(const type_checker_context_t *ctx);
uint32_t type_checker_get_symbols_resolved(const type_checker_context_t *ctx);

/* 诊断输出 */
void type_checker_dump_symbol_table(const type_checker_context_t *ctx);
void type_checker_dump_type_table(const type_checker_context_t *ctx);
void type_checker_dump_function_signatures(const type_checker_context_t *ctx);

/* ========== 工具函数 ========== */

/* 类型工具 */
const char* type_checker_type_to_string(type_info_t *type);
bool type_checker_is_numeric_type(type_info_t *type);
bool type_checker_is_integer_type(type_info_t *type);
bool type_checker_is_boolean_type(type_info_t *type);
bool type_checker_is_string_type(type_info_t *type);

/* AST工具 */
const char* type_checker_ast_node_to_string(ast_node_type_t node_type);
const char* type_checker_operator_to_string(operator_type_t op);

/* 错误码转换 */
const char* type_checker_error_to_string(type_check_error_t error_code);
const char* type_checker_severity_to_string(type_severity_t severity);

#endif /* TYPE_CHECKER_H */
