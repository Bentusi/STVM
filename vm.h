/*
 * IEC61131 虚拟机 (VM) 定义
 * 平台无关的字节码执行引擎
 */

#ifndef VM_H
#define VM_H

#include "bytecode.h"
#include "ms_sync.h"
#include <stdint.h>
#include <stdbool.h>

/* 前向声明 */
struct master_slave_sync;
struct library_manager;

/* 虚拟机运行时值类型 */
typedef enum {
    VAL_BOOL,           // 布尔值
    VAL_INT,            // 整数值
    VAL_DINT,           // 双精度整数
    VAL_REAL,           // 实数值
    VAL_STRING,         // 字符串值
    VAL_TIME,           // 时间值
    VAL_UNDEFINED       // 未定义值
} value_type_t;

/* 虚拟机运行时值 */
typedef struct vm_value {
    value_type_t type;
    union {
        bool bool_val;          // 布尔值
        int32_t int_val;        // 整数值
        int64_t dint_val;       // 双精度整数值
        double real_val;        // 实数值
        char *string_val;       // 字符串值（由MMGR管理）
        uint64_t time_val;      // 时间值（毫秒）
    } data;
} vm_value_t;

/* 虚拟机执行栈 - 静态分配 */
#define MAX_STACK_SIZE 1000
typedef struct vm_stack {
    vm_value_t data[MAX_STACK_SIZE];    // 静态栈数据
    int32_t top;                        // 栈顶指针
    int32_t capacity;                   // 栈容量
} vm_stack_t;

/* 函数调用帧 - 静态分配 */
#define MAX_CALL_FRAMES 100
typedef struct call_frame {
    uint32_t return_address;            // 返回地址
    int32_t local_var_base;             // 局部变量基地址
    int32_t param_base;                 // 参数基地址
    uint32_t param_count;               // 参数数量
    char *function_name;                // 函数名（调试用）
} call_frame_t;

/* 虚拟机状态 */
typedef enum {
    VM_STATE_INIT,          // 初始化状态
    VM_STATE_RUNNING,       // 正在运行
    VM_STATE_PAUSED,        // 暂停（调试）
    VM_STATE_STOPPED,       // 停止
    VM_STATE_ERROR,         // 错误状态
    VM_STATE_SYNC_WAIT      // 等待同步
} vm_state_t;

/* 虚拟机同步模式 */
typedef enum {
    VM_SYNC_NONE,           // 无同步
    VM_SYNC_PRIMARY,        // 主机模式
    VM_SYNC_SECONDARY       // 从机模式
} vm_sync_mode_t;

/* 虚拟机调试信息 */
typedef struct vm_debug_info {
    bool debug_enabled;                 // 是否启用调试
    uint32_t *breakpoints;              // 断点数组
    uint32_t breakpoint_count;          // 断点数量
    uint32_t current_line;              // 当前源码行号
    uint32_t current_column;            // 当前源码列号
    bool step_mode;                     // 单步模式
} vm_debug_info_t;

/* 虚拟机统计信息 */
typedef struct vm_statistics {
    uint64_t instructions_executed;     // 执行指令数
    uint64_t function_calls;            // 函数调用数
    uint64_t library_calls;             // 库函数调用数
    uint64_t sync_operations;           // 同步操作数
    uint64_t runtime_errors;            // 运行时错误数
    uint64_t execution_time_ms;         // 执行时间（毫秒）
} vm_statistics_t;

/* 虚拟机实例 - 静态内存分配 */
#define MAX_GLOBAL_VARS 500
#define MAX_LOCAL_VARS 100
typedef struct st_vm {
    /* 字节码和常量 */
    bytecode_instr_t *instructions;     // 指令序列
    uint32_t instr_count;               // 指令数量
    const_pool_item_t *const_pool;      // 常量池
    uint32_t const_pool_size;           // 常量池大小
    
    /* 运行时状态 */
    uint32_t pc;                        // 程序计数器
    vm_state_t state;                   // 虚拟机状态
    vm_stack_t operand_stack;           // 操作数栈（静态）
    call_frame_t call_stack[MAX_CALL_FRAMES]; // 调用栈（静态）
    int32_t call_stack_top;             // 调用栈顶
    
    /* 内存区域 - 静态分配 */
    vm_value_t global_vars[MAX_GLOBAL_VARS];    // 全局变量区（静态）
    uint32_t global_var_count;          // 全局变量数量
    vm_value_t local_vars[MAX_LOCAL_VARS];      // 局部变量区（静态）
    uint32_t local_var_count;           // 局部变量数量
    
    /* 库管理 */
    struct library_manager *lib_mgr;    // 库管理器
    
    /* 主从同步支持 */
    struct master_slave_sync *sync_mgr; // 同步管理器
    vm_sync_mode_t sync_mode;           // 同步模式
    bool sync_enabled;                  // 是否启用同步
    uint32_t *sync_var_indices;         // 同步变量索引数组
    uint32_t sync_var_count;            // 同步变量数量
    
    /* 调试支持 */
    vm_debug_info_t debug_info;         // 调试信息
    
    /* 统计信息 */
    vm_statistics_t statistics;         // 统计信息
    
    /* 错误处理 */
    char last_error[256];               // 最后错误信息
    bool has_error;                     // 是否有错误
    
} st_vm_t;

/* 虚拟机配置 */
typedef struct vm_config {
    bool enable_debug;                  // 启用调试
    bool enable_statistics;             // 启用统计
    uint32_t max_execution_time_ms;     // 最大执行时间
    bool enable_sync;                   // 启用同步
    vm_sync_mode_t sync_mode;           // 同步模式
} vm_config_t;

/* ========== 虚拟机生命周期管理 ========== */

/* 初始化和清理 */
st_vm_t* vm_create(void);
void vm_destroy(st_vm_t *vm);
int vm_init(st_vm_t *vm, const vm_config_t *config);
void vm_reset(st_vm_t *vm);
void vm_cleanup(st_vm_t *vm);

/* ========== 字节码加载和执行 ========== */

/* 字节码加载 */
int vm_load_bytecode_file(st_vm_t *vm, const char *filename);
int vm_load_bytecode(st_vm_t *vm, const bytecode_file_t *bc_file);
int vm_verify_bytecode(st_vm_t *vm);

/* 执行控制 */
int vm_execute(st_vm_t *vm);
int vm_execute_single_step(st_vm_t *vm);
int vm_execute_until_breakpoint(st_vm_t *vm);
void vm_pause(st_vm_t *vm);
void vm_resume(st_vm_t *vm);
void vm_stop(st_vm_t *vm);

/* 程序计数器操作 */
void vm_set_pc(st_vm_t *vm, uint32_t pc);
uint32_t vm_get_pc(const st_vm_t *vm);
void vm_jump_to(st_vm_t *vm, uint32_t address);

/* ========== 栈操作函数 ========== */

/* 操作数栈操作 */
int vm_stack_push(st_vm_t *vm, const vm_value_t *value);
vm_value_t* vm_stack_pop(st_vm_t *vm);
vm_value_t* vm_stack_top(st_vm_t *vm);
vm_value_t* vm_stack_peek(st_vm_t *vm, int32_t offset);
void vm_stack_clear(st_vm_t *vm);
bool vm_stack_is_empty(const st_vm_t *vm);
int32_t vm_stack_size(const st_vm_t *vm);

/* 调用栈操作 */
int vm_call_stack_push(st_vm_t *vm, uint32_t return_addr, const char *func_name);
call_frame_t* vm_call_stack_pop(st_vm_t *vm);
call_frame_t* vm_call_stack_top(st_vm_t *vm);
int32_t vm_call_stack_depth(const st_vm_t *vm);

/* ========== 变量操作 ========== */

/* 全局变量操作 */
int vm_set_global_var(st_vm_t *vm, uint32_t index, const vm_value_t *value);
vm_value_t* vm_get_global_var(st_vm_t *vm, uint32_t index);
int vm_init_global_vars(st_vm_t *vm, uint32_t count);

/* 局部变量操作 */
int vm_set_local_var(st_vm_t *vm, uint32_t index, const vm_value_t *value);
vm_value_t* vm_get_local_var(st_vm_t *vm, uint32_t index);
int vm_init_local_vars(st_vm_t *vm, uint32_t count);

/* 参数操作 */
int vm_set_param(st_vm_t *vm, uint32_t index, const vm_value_t *value);
vm_value_t* vm_get_param(st_vm_t *vm, uint32_t index);

/* ========== 值类型操作 ========== */

/* 值创建和转换 */
vm_value_t vm_create_bool_value(bool value);
vm_value_t vm_create_int_value(int32_t value);
vm_value_t vm_create_real_value(double value);
vm_value_t vm_create_string_value(const char *value);
vm_value_t vm_create_undefined_value(void);

/* 值类型转换 */
int vm_convert_to_bool(const vm_value_t *src, vm_value_t *dst);
int vm_convert_to_int(const vm_value_t *src, vm_value_t *dst);
int vm_convert_to_real(const vm_value_t *src, vm_value_t *dst);
int vm_convert_to_string(const vm_value_t *src, vm_value_t *dst);

/* 值比较和运算 */
bool vm_value_equals(const vm_value_t *a, const vm_value_t *b);
int vm_value_compare(const vm_value_t *a, const vm_value_t *b);
void vm_value_copy(const vm_value_t *src, vm_value_t *dst);
void vm_value_free(vm_value_t *value);

/* ========== 指令执行 ========== */

/* 核心指令执行器 */
int vm_execute_instruction(st_vm_t *vm, const bytecode_instr_t *instr);

/* 算术运算指令 */
int vm_exec_add(st_vm_t *vm, value_type_t type);
int vm_exec_sub(st_vm_t *vm, value_type_t type);
int vm_exec_mul(st_vm_t *vm, value_type_t type);
int vm_exec_div(st_vm_t *vm, value_type_t type);
int vm_exec_mod(st_vm_t *vm);
int vm_exec_neg(st_vm_t *vm, value_type_t type);

/* 逻辑运算指令 */
int vm_exec_and(st_vm_t *vm);
int vm_exec_or(st_vm_t *vm);
int vm_exec_not(st_vm_t *vm);

/* 比较运算指令 */
int vm_exec_compare(st_vm_t *vm, int compare_op, value_type_t type);

/* 控制流指令 */
int vm_exec_jump(st_vm_t *vm, uint32_t address);
int vm_exec_jump_conditional(st_vm_t *vm, uint32_t address, bool condition);

/* 函数调用指令 */
int vm_exec_call(st_vm_t *vm, uint32_t address, uint32_t param_count);
int vm_exec_call_builtin(st_vm_t *vm, uint32_t builtin_id, uint32_t param_count);
int vm_exec_call_library(st_vm_t *vm, uint32_t lib_func_id, uint32_t param_count);
int vm_exec_return(st_vm_t *vm, bool has_return_value);

/* ========== 主从同步接口 ========== */

/* 同步系统管理 */
int vm_enable_sync(st_vm_t *vm, const sync_config_t *config, vm_sync_mode_t mode);
int vm_disable_sync(st_vm_t *vm);
int vm_set_sync_mode(st_vm_t *vm, vm_sync_mode_t mode);
vm_sync_mode_t vm_get_sync_mode(const st_vm_t *vm);

/* 同步变量管理 */
int vm_register_sync_variable(st_vm_t *vm, uint32_t var_index, const char *var_name);
int vm_unregister_sync_variable(st_vm_t *vm, uint32_t var_index);
bool vm_is_sync_variable(const st_vm_t *vm, uint32_t var_index);

/* 同步操作 */
int vm_sync_variable(st_vm_t *vm, uint32_t var_index);
int vm_sync_all_variables(st_vm_t *vm);
int vm_process_sync_messages(st_vm_t *vm);
int vm_send_sync_checkpoint(st_vm_t *vm);

/* 故障切换 */
int vm_initiate_failover(st_vm_t *vm);
bool vm_should_takeover(const st_vm_t *vm);
int vm_become_primary(st_vm_t *vm);
int vm_become_secondary(st_vm_t *vm);

/* ========== 调试支持 ========== */

/* 调试控制 */
int vm_enable_debug(st_vm_t *vm);
int vm_disable_debug(st_vm_t *vm);
bool vm_is_debug_enabled(const st_vm_t *vm);

/* 断点管理 */
int vm_set_breakpoint(st_vm_t *vm, uint32_t address);
int vm_clear_breakpoint(st_vm_t *vm, uint32_t address);
int vm_clear_all_breakpoints(st_vm_t *vm);
bool vm_is_breakpoint(const st_vm_t *vm, uint32_t address);

/* 单步执行 */
int vm_enable_step_mode(st_vm_t *vm);
int vm_disable_step_mode(st_vm_t *vm);
bool vm_is_step_mode(const st_vm_t *vm);

/* 调试信息获取 */
uint32_t vm_get_current_line(const st_vm_t *vm);
uint32_t vm_get_current_column(const st_vm_t *vm);
const char* vm_get_current_function(const st_vm_t *vm);

/* ========== 库函数支持 ========== */

/* 库管理器集成 */
int vm_set_library_manager(st_vm_t *vm, struct library_manager *lib_mgr);
struct library_manager* vm_get_library_manager(const st_vm_t *vm);

/* 库函数调用 */
int vm_call_library_function(st_vm_t *vm, const char *lib_name, 
                             const char *func_name, uint32_t param_count);
int vm_call_builtin_function(st_vm_t *vm, const char *func_name, uint32_t param_count);

/* ========== 错误处理和诊断 ========== */

/* 错误管理 */
void vm_set_error(st_vm_t *vm, const char *error_msg);
const char* vm_get_last_error(const st_vm_t *vm);
bool vm_has_error(const st_vm_t *vm);
void vm_clear_error(st_vm_t *vm);

/* 状态查询 */
vm_state_t vm_get_state(const st_vm_t *vm);
const char* vm_state_to_string(vm_state_t state);

/* 统计信息 */
const vm_statistics_t* vm_get_statistics(const st_vm_t *vm);
void vm_reset_statistics(st_vm_t *vm);
void vm_print_statistics(const st_vm_t *vm);

/* 诊断输出 */
void vm_print_stack(const st_vm_t *vm);
void vm_print_call_stack(const st_vm_t *vm);
void vm_print_variables(const st_vm_t *vm);
void vm_print_sync_status(const st_vm_t *vm);

/* 内存使用信息 */
size_t vm_get_memory_usage(const st_vm_t *vm);
void vm_print_memory_info(const st_vm_t *vm);

/* ========== 工具函数 ========== */

/* 值类型工具 */
const char* vm_value_type_to_string(value_type_t type);
bool vm_is_numeric_type(value_type_t type);
bool vm_is_compatible_types(value_type_t a, value_type_t b);

/* 指令工具 */
const char* vm_opcode_to_string(opcode_t opcode);
bool vm_is_jump_instruction(opcode_t opcode);
bool vm_is_call_instruction(opcode_t opcode);

/* 地址和索引验证 */
bool vm_is_valid_pc(const st_vm_t *vm, uint32_t pc);
bool vm_is_valid_global_var_index(const st_vm_t *vm, uint32_t index);
bool vm_is_valid_local_var_index(const st_vm_t *vm, uint32_t index);

#endif /* VM_H */