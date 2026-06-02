/**
 * @file debugger.h
 * @brief 调试器 - 支持断点、单步执行、变量查看等功能
 */

#ifndef STVM_DEBUGGER_H
#define STVM_DEBUGGER_H

#include "vm.h"
#include "bytecode.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief 断点结构
 */
typedef struct Breakpoint {
    uint32_t address;               // 断点地址
    bool enabled;                   // 是否启用
    int hit_count;                  // 命中次数
    char condition[256];            // 条件表达式（可选）
    struct Breakpoint* next;        // 链表指针
} Breakpoint;

/**
 * @brief 调试器状态
 */
typedef enum {
    DEBUG_STOPPED,      // 停止状态
    DEBUG_RUNNING,      // 运行中
    DEBUG_STEP_OVER,    // 单步跳过
    DEBUG_STEP_INTO,    // 单步进入
    DEBUG_STEP_OUT,     // 单步跳出
    DEBUG_PAUSED        // 暂停
} DebugState;

/**
 * @brief 调试器结构
 */
typedef struct Debugger {
    VM* vm;                         // 关联的虚拟机
    DebugState state;               // 调试状态
    Breakpoint* breakpoints;        // 断点链表
    int breakpoint_count;           // 断点数量
    uint32_t last_pc;               // 上一条指令地址
    int32_t step_frame_level;       // 单步执行的栈帧层级
    bool show_disasm;               // 是否显示反汇编
    char last_command[256];         // 上一条命令（用于重复）
} Debugger;

/**
 * @brief 创建调试器
 * @param vm 虚拟机实例
 * @return 调试器实例
 */
Debugger* debugger_create(VM* vm);

/**
 * @brief 释放调试器
 * @param dbg 调试器实例
 */
void debugger_free(Debugger* dbg);

/**
 * @brief 启动调试会话
 * @param dbg 调试器实例
 * @return 成功返回0，失败返回错误码
 */
int debugger_run(Debugger* dbg);

/**
 * @brief 添加断点
 * @param dbg 调试器实例
 * @param address 断点地址
 * @return 成功返回断点ID，失败返回-1
 */
int debugger_add_breakpoint(Debugger* dbg, uint32_t address);

/**
 * @brief 删除断点
 * @param dbg 调试器实例
 * @param address 断点地址
 * @return 成功返回true
 */
bool debugger_remove_breakpoint(Debugger* dbg, uint32_t address);

/**
 * @brief 启用/禁用断点
 * @param dbg 调试器实例
 * @param address 断点地址
 * @param enabled 是否启用
 * @return 成功返回true
 */
bool debugger_toggle_breakpoint(Debugger* dbg, uint32_t address, bool enabled);

/**
 * @brief 列出所有断点
 * @param dbg 调试器实例
 */
void debugger_list_breakpoints(Debugger* dbg);

/**
 * @brief 单步执行（跳过函数调用）
 * @param dbg 调试器实例
 * @return 错误码
 */
ErrorCode debugger_step_over(Debugger* dbg);

/**
 * @brief 单步执行（进入函数）
 * @param dbg 调试器实例
 * @return 错误码
 */
ErrorCode debugger_step_into(Debugger* dbg);

/**
 * @brief 单步跳出（执行到函数返回）
 * @param dbg 调试器实例
 * @return 错误码
 */
ErrorCode debugger_step_out(Debugger* dbg);

/**
 * @brief 继续执行直到下一个断点
 * @param dbg 调试器实例
 * @return 错误码
 */
ErrorCode debugger_continue(Debugger* dbg);

/**
 * @brief 打印当前栈帧信息
 * @param dbg 调试器实例
 */
void debugger_print_frame(Debugger* dbg);

/**
 * @brief 打印调用栈
 * @param dbg 调试器实例
 */
void debugger_print_backtrace(Debugger* dbg);

/**
 * @brief 打印局部变量
 * @param dbg 调试器实例
 */
void debugger_print_locals(Debugger* dbg);

/**
 * @brief 打印全局变量
 * @param dbg 调试器实例
 */
void debugger_print_globals(Debugger* dbg);

/**
 * @brief 打印操作数栈
 * @param dbg 调试器实例
 */
void debugger_print_stack(Debugger* dbg);

/**
 * @brief 打印指定地址的变量值
 * @param dbg 调试器实例
 * @param name 变量名
 */
void debugger_print_variable(Debugger* dbg, const char* name);

/**
 * @brief 反汇编指定范围的字节码
 * @param dbg 调试器实例
 * @param start 起始地址
 * @param count 指令数量
 */
void debugger_disassemble(Debugger* dbg, uint32_t start, int count);

/**
 * @brief 检查是否命中断点
 * @param dbg 调试器实例
 * @param address 当前地址
 * @return 命中返回true
 */
bool debugger_check_breakpoint(Debugger* dbg, uint32_t address);

/**
 * @brief 处理调试命令
 * @param dbg 调试器实例
 * @param command 命令字符串
 * @return 成功返回true，退出返回false
 */
bool debugger_handle_command(Debugger* dbg, const char* command);

/**
 * @brief 打印调试帮助信息
 */
void debugger_print_help(void);

#endif // STVM_DEBUGGER_H
