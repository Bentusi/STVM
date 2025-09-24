#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "bytecode.h"
#include "symbol_table.h"
#include "libmgr.h"
#include "ms_sync.h"
#include <stdint.h>
#include <stdbool.h>

/* 代码生成器状态 */
typedef struct codegen_context {
    bytecode_generator_t *bc_gen;       // 字节码生成器
    symbol_table_manager_t *sym_mgr;    // 符号表管理器
    library_manager_t *lib_mgr;         // 库管理器
    master_slave_sync_t *sync_mgr;      // 主从同步管理器
    
    /* 当前函数上下文 */
    struct {
        char *name;                     // 当前函数名
        uint32_t param_count;           // 参数数量
        type_info_t *return_type;       // 返回类型
        uint32_t local_var_offset;      // 局部变量偏移
        bool has_return_value;          // 是否有返回值
        uint32_t entry_label;           // 函数入口标签
    } current_function;
    
    /* 变量地址分配 */
    struct {
        uint32_t global_offset;         // 全局变量偏移
        uint32_t local_offset;          // 局部变量偏移
        uint32_t param_offset;          // 参数偏移
    } var_allocation;
    
    /* 控制流管理 */
    struct {
        uint32_t *break_labels;         // break标签栈
        uint32_t *continue_labels;      // continue标签栈
        uint32_t label_stack_depth;     // 标签栈深度
        uint32_t next_temp_label;       // 下一个临时标签编号
        uint32_t max_stack_depth;       // 最大栈深度
    } control_flow;
    
    /* 主从同步变量管理 */
    struct {
        char **sync_var_names;          // 同步变量名列表
        uint32_t *sync_var_indices;     // 同步变量VM索引
        uint32_t *sync_var_types;       // 同步变量类型
        bool *sync_var_dirty_flags;     // 同步变量脏标记
        uint32_t sync_var_count;        // 同步变量数量
        uint32_t sync_var_capacity;     // 同步变量容量
        bool sync_enabled;              // 是否启用同步
        uint32_t checkpoint_interval;   // 检查点间隔（指令数）
        uint32_t last_checkpoint;       // 上次检查点位置
    } sync_info;
    
    /* 错误处理 */
    char last_error[512];               // 最后错误信息
    bool has_error;                     // 是否有错误
    
    /* 统计信息 */
    struct {
        uint32_t instructions_generated; // 生成的指令数
        uint32_t functions_compiled;     // 编译的函数数
        uint32_t variables_allocated;    // 分配的变量数
        uint32_t library_calls;          // 库函数调用数
        uint32_t sync_instructions;      // 同步指令数
        uint32_t checkpoints_generated;  // 生成的检查点数
    } statistics;
    
} codegen_context_t;

/* 变量访问信息 */
typedef struct var_access_info {
    enum {
        VAR_ACCESS_GLOBAL,              // 全局变量
        VAR_ACCESS_LOCAL,               // 局部变量
        VAR_ACCESS_PARAM,               // 函数参数
        VAR_ACCESS_LIBRARY              // 库变量
    } access_type;
    
    uint32_t offset;                    // 变量偏移
    type_info_t *type;                  // 变量类型
    char *library_name;                 // 库名（库变量）
    bool is_sync;                       // 是否同步变量
    uint32_t vm_index;                  // VM变量索引
    uint32_t sync_index;                // 同步变量索引（如果是同步变量）
} var_access_info_t;

/* 函数调用信息 */
typedef struct function_call_info {
    enum {
        FUNC_CALL_USER,                 // 用户函数
        FUNC_CALL_BUILTIN,              // 内置函数
        FUNC_CALL_LIBRARY               // 库函数
    } call_type;
    
    char *function_name;                // 函数名
    char *library_name;                 // 库名（库函数）
    uint32_t param_count;               // 参数数量
    type_info_t *return_type;           // 返回类型
    void *builtin_impl;                 // 内置函数实现（内置函数）
    uint32_t function_address;          // 函数地址
} function_call_info_t;

/* 表达式求值结果 */
typedef struct expr_result {
    type_info_t *type;                  // 表达式类型
    bool is_constant;                   // 是否常量
    bool affects_sync_var;              // 是否影响同步变量
    union {
        int32_t int_val;                // 整数常量值
        double real_val;                // 实数常量值
        bool bool_val;                  // 布尔常量值
        char *string_val;               // 字符串常量值
    } const_value;
} expr_result_t;

/* 同步变量配置 */
typedef struct sync_var_config {
    char *var_name;                     // 变量名
    uint32_t vm_index;                  // VM变量索引
    uint32_t var_type;                  // 变量类型
    uint32_t var_size;                  // 变量大小
    bool auto_sync;                     // 是否自动同步
    uint32_t sync_priority;             // 同步优先级
} sync_var_config_t;

/* ========== 代码生成器管理函数 ========== */

/* 初始化和清理 */
codegen_context_t* codegen_create_context(void);
void codegen_destroy_context(codegen_context_t *ctx);
int codegen_init_context(codegen_context_t *ctx, symbol_table_manager_t *sym_mgr,
                         library_manager_t *lib_mgr, master_slave_sync_t *sync_mgr);

/* ========== AST编译函数 ========== */

/* 顶层编译 */
int codegen_compile_program(codegen_context_t *ctx, ast_node_t *program_node,
                           bytecode_file_t *output_file);
int codegen_compile_ast(codegen_context_t *ctx, ast_node_t *node);

/* 声明编译 */
int codegen_compile_var_declaration(codegen_context_t *ctx, ast_node_t *var_decl);
int codegen_compile_function_declaration(codegen_context_t *ctx, ast_node_t *func_decl);

/* 语句编译 */
int codegen_compile_statement_list(codegen_context_t *ctx, ast_node_t *stmt_list);
int codegen_compile_assignment(codegen_context_t *ctx, ast_node_t *assignment);
int codegen_compile_if_statement(codegen_context_t *ctx, ast_node_t *if_stmt);
int codegen_compile_for_statement(codegen_context_t *ctx, ast_node_t *for_stmt);
int codegen_compile_while_statement(codegen_context_t *ctx, ast_node_t *while_stmt);
int codegen_compile_case_statement(codegen_context_t *ctx, ast_node_t *case_stmt);
int codegen_compile_return_statement(codegen_context_t *ctx, ast_node_t *return_stmt);

/* 表达式编译 */
int codegen_compile_expression(codegen_context_t *ctx, ast_node_t *expr);
int codegen_compile_binary_operation(codegen_context_t *ctx, ast_node_t *binary_op);
int codegen_compile_unary_operation(codegen_context_t *ctx, ast_node_t *unary_op);
int codegen_compile_function_call(codegen_context_t *ctx, ast_node_t *func_call);
int codegen_compile_identifier(codegen_context_t *ctx, ast_node_t *identifier);
int codegen_compile_literal(codegen_context_t *ctx, ast_node_t *literal);

/* ========== 变量和函数管理 ========== */

/* 变量分配和访问 */
int codegen_allocate_variable(codegen_context_t *ctx, const char *name,
                             type_info_t *type, bool is_global, bool is_sync);
var_access_info_t* codegen_get_variable_access(codegen_context_t *ctx, const char *name);
int codegen_generate_variable_load(codegen_context_t *ctx, const var_access_info_t *access);
int codegen_generate_variable_store(codegen_context_t *ctx, const var_access_info_t *access);

/* 函数管理 */
int codegen_register_function(codegen_context_t *ctx, const char *name,
                             type_info_t *return_type, uint32_t param_count);
function_call_info_t* codegen_resolve_function_call(codegen_context_t *ctx,
                                                   const char *function_name,
                                                   const char *library_name);
int codegen_generate_function_call(codegen_context_t *ctx, const function_call_info_t *call_info,
                                  uint32_t arg_count);

/* ========== 库函数调用支持 ========== */

/* 库函数解析 */
int codegen_resolve_library_function(codegen_context_t *ctx, const char *library_name,
                                     const char *function_name, function_call_info_t *call_info);
int codegen_generate_library_call(codegen_context_t *ctx, const char *library_name,
                                 const char *function_name, uint32_t arg_count);

/* 内置函数支持 */
int codegen_generate_builtin_call(codegen_context_t *ctx, const char *function_name,
                                 uint32_t arg_count);

/* ========== 主从同步支持函数 ========== */

/* 同步系统初始化和管理 */
int codegen_enable_sync(codegen_context_t *ctx);
int codegen_disable_sync(codegen_context_t *ctx);
int codegen_set_sync_checkpoint_interval(codegen_context_t *ctx, uint32_t interval);

/* 同步变量注册和管理 */
int codegen_register_sync_variable(codegen_context_t *ctx, const char *var_name, 
                                  uint32_t vm_index, const sync_var_config_t *config);
int codegen_unregister_sync_variable(codegen_context_t *ctx, const char *var_name);
int codegen_mark_sync_variable_dirty(codegen_context_t *ctx, const char *var_name);
bool codegen_is_sync_variable(codegen_context_t *ctx, const char *var_name);

/* 同步指令生成 */
int codegen_generate_sync_instruction(codegen_context_t *ctx, const char *var_name);
int codegen_generate_sync_checkpoint(codegen_context_t *ctx);
int codegen_generate_sync_barrier(codegen_context_t *ctx);
int codegen_generate_conditional_sync(codegen_context_t *ctx, const char *var_name, 
                                     const char *condition_label);

/* 自动同步检测和生成 */
int codegen_auto_insert_sync_points(codegen_context_t *ctx);
int codegen_check_sync_requirements(codegen_context_t *ctx, ast_node_t *node);
bool codegen_should_insert_checkpoint(codegen_context_t *ctx);

/* 主从角色相关代码生成 */
int codegen_generate_primary_mode_code(codegen_context_t *ctx);
int codegen_generate_secondary_mode_code(codegen_context_t *ctx);
int codegen_generate_role_switch_code(codegen_context_t *ctx, node_role_t target_role);

/* 故障切换支持 */
int codegen_generate_failover_detection(codegen_context_t *ctx);
int codegen_generate_state_backup(codegen_context_t *ctx);
int codegen_generate_state_recovery(codegen_context_t *ctx);

/* ========== 控制流管理 ========== */

/* 标签管理 */
uint32_t codegen_create_temp_label(codegen_context_t *ctx);
char* codegen_get_temp_label_name(codegen_context_t *ctx, uint32_t label_id);
int codegen_push_break_label(codegen_context_t *ctx, uint32_t label_id);
int codegen_push_continue_label(codegen_context_t *ctx, uint32_t label_id);
uint32_t codegen_pop_break_label(codegen_context_t *ctx);
uint32_t codegen_pop_continue_label(codegen_context_t *ctx);

/* 跳转指令生成 */
uint32_t codegen_emit_jump(codegen_context_t *ctx, const char *label);
uint32_t codegen_emit_jump_if_false(codegen_context_t *ctx, const char *label);
uint32_t codegen_emit_jump_if_true(codegen_context_t *ctx, const char *label);

/* ========== 类型转换和验证 ========== */

/* 类型转换 */
int codegen_generate_type_conversion(codegen_context_t *ctx, type_info_t *from_type,
                                    type_info_t *to_type);
bool codegen_is_implicit_conversion_allowed(type_info_t *from_type, type_info_t *to_type);
int codegen_generate_implicit_conversion(codegen_context_t *ctx, type_info_t *from_type,
                                        type_info_t *to_type);

/* 类型验证 */
bool codegen_validate_binary_operation(operator_type_t op, type_info_t *left_type,
                                      type_info_t *right_type, type_info_t **result_type);
bool codegen_validate_unary_operation(operator_type_t op, type_info_t *operand_type,
                                     type_info_t **result_type);

/* ========== 优化和分析 ========== */

/* 基础优化 */
int codegen_optimize_constants(codegen_context_t *ctx);
int codegen_optimize_dead_code(codegen_context_t *ctx);
int codegen_optimize_jumps(codegen_context_t *ctx);
int codegen_optimize_sync_instructions(codegen_context_t *ctx);

/* 代码分析 */
int codegen_analyze_stack_usage(codegen_context_t *ctx, uint32_t *max_stack_depth);
int codegen_analyze_memory_usage(codegen_context_t *ctx, uint32_t *global_size,
                                uint32_t *max_local_size);
int codegen_analyze_sync_overhead(codegen_context_t *ctx, uint32_t *sync_instruction_count,
                                 uint32_t *checkpoint_count);

/* ========== 调试支持 ========== */

/* 调试信息生成 */
int codegen_generate_line_info(codegen_context_t *ctx, uint32_t line, uint32_t column);
int codegen_generate_breakpoint(codegen_context_t *ctx, uint32_t breakpoint_id);
int codegen_generate_debug_print(codegen_context_t *ctx, const char *message);
int codegen_generate_sync_debug_info(codegen_context_t *ctx, const char *var_name);

/* ========== 错误处理和诊断 ========== */

/* 错误管理 */
void codegen_set_error(codegen_context_t *ctx, const char *error_msg);
const char* codegen_get_last_error(codegen_context_t *ctx);
bool codegen_has_error(codegen_context_t *ctx);
void codegen_clear_error(codegen_context_t *ctx);

/* 诊断信息 */
void codegen_print_statistics(const codegen_context_t *ctx);
void codegen_print_variable_allocation(const codegen_context_t *ctx);
void codegen_print_function_table(const codegen_context_t *ctx);
void codegen_print_sync_variables(const codegen_context_t *ctx);
void codegen_print_sync_statistics(const codegen_context_t *ctx);

/* ========== 辅助函数 ========== */

/* 操作码选择 */
opcode_t codegen_select_load_opcode(type_info_t *type, bool is_constant);
opcode_t codegen_select_store_opcode(type_info_t *type);
opcode_t codegen_select_binary_opcode(operator_type_t op, type_info_t *type);
opcode_t codegen_select_unary_opcode(operator_type_t op, type_info_t *type);
opcode_t codegen_select_compare_opcode(operator_type_t op, type_info_t *type);

/* 类型工具 */
uint32_t codegen_get_type_size(type_info_t *type);
bool codegen_is_numeric_type(type_info_t *type);
bool codegen_is_integer_type(type_info_t *type);
bool codegen_is_real_type(type_info_t *type);

/* 同步辅助函数 */
uint32_t codegen_get_sync_var_index(codegen_context_t *ctx, const char *var_name);
bool codegen_needs_sync(codegen_context_t *ctx, ast_node_t *node);
int codegen_calculate_sync_priority(codegen_context_t *ctx, const char *var_name);

#endif /* CODEGEN_H */
