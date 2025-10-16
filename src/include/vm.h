#ifndef STVM_VM_H
#define STVM_VM_H

#include "bytecode.h"
#include "types.h"
#include "error.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 调用帧结构 - 保存函数调用上下文
 */
typedef struct CallFrame {
    uint32_t return_address;    // 返回地址
    int32_t base_pointer;       // 栈帧基址指针
    int32_t local_count;        // 局部变量数量
    FunctionEntry* function;    // 当前函数信息
} CallFrame;

/**
 * @brief 虚拟机主结构
 */
typedef struct VM {
    // 字节码模块
    BytecodeModule* module;
    
    // 操作数栈
    Value* stack;
    int32_t stack_size;
    int32_t sp;                 // 栈顶指针 (-1表示空栈)
    
    // 调用栈
    CallFrame* call_stack;
    int32_t call_stack_size;
    int32_t call_sp;            // 调用栈指针 (-1表示空)
    
    // 全局变量
    Value* globals;
    int32_t global_count;
    
    // 程序计数器
    uint32_t pc;
    
    // 运行状态
    bool running;
    ErrorCode error_code;
    char error_msg[256];
    
    // 性能统计（可选）
    uint64_t instruction_count;
    
    // 跳转表（用于直接跳转优化）
    void** jump_table;
    
    // 外部函数表
    struct ExternalFunction* external_functions;
    int32_t external_function_count;
} VM;

/**
 * @brief 外部函数回调类型
 * @param vm 虚拟机实例
 * @param argc 参数个数
 * @return 返回值
 */
typedef Value (*ExternalFunctionCallback)(VM* vm, int32_t argc);

/**
 * @brief 外部函数注册结构
 */
typedef struct ExternalFunction {
    char* name;                     // 函数名
    ExternalFunctionCallback callback; // 回调函数
    int32_t param_count;            // 参数个数（-1表示可变参数）
} ExternalFunction;

/**
 * @brief 创建虚拟机实例
 * @param module 要执行的字节码模块
 * @return 虚拟机实例，失败返回NULL
 */
VM* vm_create(BytecodeModule* module);

/**
 * @brief 释放虚拟机实例
 * @param vm 虚拟机实例
 */
void vm_free(VM* vm);

/**
 * @brief 执行字节码
 * @param vm 虚拟机实例
 * @return 错误码
 */
ErrorCode vm_run(VM* vm);

/**
 * @brief 执行字节码（从指定入口点开始）
 * @param vm 虚拟机实例
 * @param entry_point 入口地址
 * @return 错误码
 */
ErrorCode vm_run_from(VM* vm, uint32_t entry_point);

/**
 * @brief 单步执行一条指令（用于调试）
 * @param vm 虚拟机实例
 * @return 错误码
 */
ErrorCode vm_step(VM* vm);

/**
 * @brief 获取栈顶值
 * @param vm 虚拟机实例
 * @param result 输出参数，保存栈顶值
 * @return 错误码
 */
ErrorCode vm_get_result(VM* vm, Value* result);

/**
 * @brief 重置虚拟机状态
 * @param vm 虚拟机实例
 */
void vm_reset(VM* vm);

/**
 * @brief 打印虚拟机状态（调试用）
 * @param vm 虚拟机实例
 */
void vm_dump_state(VM* vm);

/**
 * @brief 打印调用栈（调试用）
 * @param vm 虚拟机实例
 */
void vm_dump_call_stack(VM* vm);

/**
 * @brief 注册外部函数
 * @param vm 虚拟机实例
 * @param name 函数名
 * @param callback 回调函数
 * @param param_count 参数个数（-1表示可变参数）
 * @return 成功返回true
 */
bool vm_register_external_function(VM* vm, const char* name, 
                                    ExternalFunctionCallback callback,
                                    int32_t param_count);

/**
 * @brief 获取外部函数参数
 * @param vm 虚拟机实例
 * @param index 参数索引（0表示第一个参数）
 * @return 参数值
 */
Value vm_get_arg(VM* vm, int32_t index);

#endif // STVM_VM_H
