/**
 * @file debugger.c
 * @brief 调试器实现 - 支持断点、单步执行、变量查看等
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "debugger.h"
#include "mmgr.h"
#include "error.h"
#include "bytecode_io.h"

/**
 * @brief 创建调试器
 */
Debugger* debugger_create(VM* vm) {
    if (!vm) {
        return NULL;
    }
    
    Debugger* dbg = (Debugger*)mmgr_alloc(sizeof(Debugger));
    if (!dbg) {
        return NULL;
    }
    
    memset(dbg, 0, sizeof(Debugger));
    dbg->vm = vm;
    dbg->state = DEBUG_STOPPED;
    dbg->show_disasm = true;
    
    return dbg;
}

/**
 * @brief 释放调试器
 */
void debugger_free(Debugger* dbg) {
    if (!dbg) {
        return;
    }
    
    // 释放所有断点
    Breakpoint* bp = dbg->breakpoints;
    while (bp) {
        Breakpoint* next = bp->next;
        mmgr_free(bp);
        bp = next;
    }
    
    mmgr_free(dbg);
}

/**
 * @brief 添加断点
 */
int debugger_add_breakpoint(Debugger* dbg, uint32_t address) {
    if (!dbg) {
        return -1;
    }
    
    // 检查是否已存在
    Breakpoint* bp = dbg->breakpoints;
    while (bp) {
        if (bp->address == address) {
            printf("断点已存在于地址 %u\n", address);
            return -1;
        }
        bp = bp->next;
    }
    
    // 创建新断点
    bp = (Breakpoint*)mmgr_alloc(sizeof(Breakpoint));
    if (!bp) {
        return ERR_OUT_OF_MEMORY;
    }
    
    bp->address = address;
    bp->enabled = true;
    bp->hit_count = 0;
    bp->condition[0] = '\0';
    bp->next = dbg->breakpoints;
    dbg->breakpoints = bp;
    dbg->breakpoint_count++;
    
    printf("断点 #%d 设置于地址 %u\n", dbg->breakpoint_count, address);
    return dbg->breakpoint_count;
}

/**
 * @brief 删除断点
 */
bool debugger_remove_breakpoint(Debugger* dbg, uint32_t address) {
    if (!dbg) {
        return false;
    }
    
    Breakpoint** pp = &dbg->breakpoints;
    while (*pp) {
        if ((*pp)->address == address) {
            Breakpoint* to_remove = *pp;
            *pp = to_remove->next;
            mmgr_free(to_remove);
            dbg->breakpoint_count--;
            printf("断点已删除于地址 %u\n", address);
            return true;
        }
        pp = &(*pp)->next;
    }
    
    printf("未找到地址 %u 的断点\n", address);
    return false;
}

/**
 * @brief 启用/禁用断点
 */
bool debugger_toggle_breakpoint(Debugger* dbg, uint32_t address, bool enabled) {
    if (!dbg) {
        return false;
    }
    
    Breakpoint* bp = dbg->breakpoints;
    while (bp) {
        if (bp->address == address) {
            bp->enabled = enabled;
            printf("断点 %u %s\n", address, enabled ? "已启用" : "已禁用");
            return true;
        }
        bp = bp->next;
    }
    
    printf("未找到地址 %u 的断点\n", address);
    return false;
}

/**
 * @brief 列出所有断点
 */
void debugger_list_breakpoints(Debugger* dbg) {
    if (!dbg || !dbg->breakpoints) {
        printf("没有断点\n");
        return;
    }
    
    printf("断点列表:\n");
    int id = 1;
    Breakpoint* bp = dbg->breakpoints;
    while (bp) {
        printf("  #%d: 地址 %u [%s] (命中 %d 次)\n",
               id, bp->address,
               bp->enabled ? "启用" : "禁用",
               bp->hit_count);
        bp = bp->next;
        id++;
    }
}

/**
 * @brief 检查是否命中断点
 */
bool debugger_check_breakpoint(Debugger* dbg, uint32_t address) {
    if (!dbg) {
        return false;
    }
    
    Breakpoint* bp = dbg->breakpoints;
    while (bp) {
        if (bp->address == address && bp->enabled) {
            bp->hit_count++;
            return true;
        }
        bp = bp->next;
    }
    
    return false;
}

/**
 * @brief 打印当前栈帧信息
 */
void debugger_print_frame(Debugger* dbg) {
    if (!dbg || !dbg->vm) {
        return;
    }
    
    VM* vm = dbg->vm;
    
    printf("=== 当前栈帧 ===\n");
    printf("PC: %u\n", vm->pc);
    printf("SP: %d\n", vm->sp);
    
    if (vm->call_sp >= 0) {
        CallFrame* frame = &vm->call_stack[vm->call_sp];
        printf("当前函数: %s\n", frame->function ? frame->function->name : "<main>");
        printf("返回地址: %u\n", frame->return_address);
        printf("基址指针: %d\n", frame->base_pointer);
        printf("局部变量数: %d\n", frame->local_count);
    } else {
        printf("当前函数: <main>\n");
    }
    
    // 显示当前指令
    if (vm->pc < vm->module->instruction_count) {
        printf("\n当前指令:\n");
        bytecode_print_module(vm->module);
    }
}

/**
 * @brief 打印调用栈
 */
void debugger_print_backtrace(Debugger* dbg) {
    if (!dbg || !dbg->vm) {
        return;
    }
    
    VM* vm = dbg->vm;
    
    printf("=== 调用栈 ===\n");
    
    if (vm->call_sp < 0) {
        printf("#0  <main> at pc=%u\n", vm->pc);
        return;
    }
    
    for (int i = vm->call_sp; i >= 0; i--) {
        CallFrame* frame = &vm->call_stack[i];
        printf("#%d  %s at pc=%u (bp=%d)\n",
               vm->call_sp - i,
               frame->function ? frame->function->name : "<unknown>",
               frame->return_address,
               frame->base_pointer);
    }
    
    printf("#%d  <main>\n", vm->call_sp + 1);
}

/**
 * @brief 打印局部变量
 */
void debugger_print_locals(Debugger* dbg) {
    if (!dbg || !dbg->vm) {
        return;
    }
    
    VM* vm = dbg->vm;
    
    printf("=== 局部变量 ===\n");
    
    if (vm->call_sp < 0) {
        printf("无局部变量（全局作用域）\n");
        return;
    }
    
    CallFrame* frame = &vm->call_stack[vm->call_sp];
    int32_t bp = frame->base_pointer;
    
    for (int i = 0; i < frame->local_count; i++) {
        if (bp + i <= vm->sp) {
            Value* val = &vm->stack[bp + i];
            printf("  [%d] ", i);
            value_print(val);
            printf("\n");
        }
    }
}

/**
 * @brief 打印全局变量
 */
void debugger_print_globals(Debugger* dbg) {
    if (!dbg || !dbg->vm) {
        return;
    }
    
    VM* vm = dbg->vm;
    
    printf("=== 全局变量 ===\n");
    
    for (int i = 0; i < vm->global_count; i++) {
        printf("  [%d] ", i);
        value_print(&vm->globals[i]);
        printf("\n");
    }
}

/**
 * @brief 打印操作数栈
 */
void debugger_print_stack(Debugger* dbg) {
    if (!dbg || !dbg->vm) {
        return;
    }
    
    VM* vm = dbg->vm;
    
    printf("=== 操作数栈 ===\n");
    
    if (vm->sp < 0) {
        printf("栈为空\n");
        return;
    }
    
    printf("栈顶 -> 栈底:\n");
    for (int i = vm->sp; i >= 0; i--) {
        printf("  [%d] ", i);
        value_print(&vm->stack[i]);
        if (i == vm->sp) {
            printf(" <- SP");
        }
        if (vm->call_sp >= 0 && i == vm->call_stack[vm->call_sp].base_pointer) {
            printf(" <- BP");
        }
        printf("\n");
    }
}

/**
 * @brief 反汇编指定范围的字节码
 */
void debugger_disassemble(Debugger* dbg, uint32_t start, int count) {
    if (!dbg || !dbg->vm || !dbg->vm->module) {
        return;
    }
    
    // TODO: 实现指定范围的反汇编，当前忽略 start 和 count 参数
    (void)start;
    (void)count;
    
    bytecode_print_module(dbg->vm->module);
}

/**
 * @brief 单步执行一条指令
 */
static ErrorCode debugger_step_instruction(Debugger* dbg) {
    if (!dbg || !dbg->vm) {
        return ERR_RUNTIME;
    }
    
    VM* vm = dbg->vm;
    
    // 保存当前状态
    dbg->last_pc = vm->pc;
    
    // 执行一条指令
    ErrorCode err = vm_step(vm);
    if (err != OK) {
        return err;
    }
    
    // 显示反汇编（如果启用）
    // TODO: 实现 bytecode_disassemble 函数以显示单条指令的反汇编
    /*
    if (dbg->show_disasm) {
        bytecode_disassemble(vm->module, dbg->last_pc, 1);
    }
    */
    
    return OK;
}

/**
 * @brief 单步执行（跳过函数调用）
 */
ErrorCode debugger_step_over(Debugger* dbg) {
    if (!dbg || !dbg->vm) {
        return ERR_RUNTIME;
    }
    
    VM* vm = dbg->vm;
    dbg->state = DEBUG_STEP_OVER;
    dbg->step_frame_level = vm->call_sp;
    
    // 检查下一条指令是否是CALL
    if (vm->pc < vm->module->instruction_count) {
        Instruction instr = vm->module->instructions[vm->pc];
        if (instr.opcode == OP_CALL || instr.opcode == OP_CALL_EXT) {
            // 设置临时断点在下一条指令
            uint32_t return_address = vm->pc + 1;
            
            // 执行直到返回
            while (vm->running && vm->pc != return_address) {
                ErrorCode err = debugger_step_instruction(dbg);
                if (err != OK) {
                    return err;
                }
            }
            
            return OK;
        }
    }
    
    // 普通单步
    return debugger_step_instruction(dbg);
}

/**
 * @brief 单步执行（进入函数）
 */
ErrorCode debugger_step_into(Debugger* dbg) {
    if (!dbg) {
        return ERR_RUNTIME;
    }
    
    dbg->state = DEBUG_STEP_INTO;
    return debugger_step_instruction(dbg);
}

/**
 * @brief 单步跳出（执行到函数返回）
 */
ErrorCode debugger_step_out(Debugger* dbg) {
    if (!dbg || !dbg->vm) {
        return ERR_RUNTIME;
    }
    
    VM* vm = dbg->vm;
    
    if (vm->call_sp < 0) {
        printf("已在最顶层\n");
        return OK;
    }
    
    dbg->state = DEBUG_STEP_OUT;
    int32_t target_level = vm->call_sp - 1;
    
    // 执行直到栈帧返回
    while (vm->running && vm->call_sp > target_level) {
        ErrorCode err = debugger_step_instruction(dbg);
        if (err != OK) {
            return err;
        }
    }
    
    return OK;
}

/**
 * @brief 继续执行直到下一个断点
 */
ErrorCode debugger_continue(Debugger* dbg) {
    if (!dbg || !dbg->vm) {
        return ERR_RUNTIME;
    }
    
    dbg->state = DEBUG_RUNNING;
    VM* vm = dbg->vm;
    
    while (vm->running) {
        // 检查断点
        if (debugger_check_breakpoint(dbg, vm->pc)) {
            printf("\n断点命中于地址 %u\n", vm->pc);
            debugger_print_frame(dbg);
            return OK;
        }
        
        ErrorCode err = debugger_step_instruction(dbg);
        if (err != OK) {
            return err;
        }
    }
    
    printf("程序正常结束\n");
    return OK;
}

/**
 * @brief 打印调试帮助信息
 */
void debugger_print_help(void) {
    printf("调试器命令:\n");
    printf("  h, help              显示帮助\n");
    printf("  r, run               开始执行\n");
    printf("  c, continue          继续执行\n");
    printf("  s, step              单步执行（进入函数）\n");
    printf("  n, next              单步执行（跳过函数）\n");
    printf("  f, finish            执行到函数返回\n");
    printf("  b <addr>             在地址设置断点\n");
    printf("  d <addr>             删除断点\n");
    printf("  info breakpoints     列出所有断点\n");
    printf("  info frame           显示当前栈帧\n");
    printf("  info locals          显示局部变量\n");
    printf("  info globals         显示全局变量\n");
    printf("  backtrace, bt        显示调用栈\n");
    printf("  print <name>         打印变量\n");
    printf("  stack                显示操作数栈\n");
    printf("  disasm [addr] [n]    反汇编代码\n");
    printf("  q, quit              退出调试器\n");
}

/**
 * @brief 处理调试命令
 */
bool debugger_handle_command(Debugger* dbg, const char* command) {
    if (!dbg || !command) {
        return true;
    }
    
    char cmd[256];
    strncpy(cmd, command, sizeof(cmd) - 1);
    cmd[sizeof(cmd) - 1] = '\0';
    
    // 移除前后空白
    char* start = cmd;
    while (isspace(*start)) start++;
    
    if (*start == '\0') {
        // 空命令，重复上一条
        if (dbg->last_command[0] != '\0') {
            return debugger_handle_command(dbg, dbg->last_command);
        }
        return true;
    }
    
    // 保存命令
    strncpy(dbg->last_command, start, sizeof(dbg->last_command) - 1);
    
    // 解析命令
    char* token = strtok(start, " \t");
    if (!token) {
        return true;
    }
    
    // 帮助
    if (strcmp(token, "h") == 0 || strcmp(token, "help") == 0) {
        debugger_print_help();
    }
    // 运行/继续
    else if (strcmp(token, "r") == 0 || strcmp(token, "run") == 0 ||
             strcmp(token, "c") == 0 || strcmp(token, "continue") == 0) {
        debugger_continue(dbg);
    }
    // 单步进入
    else if (strcmp(token, "s") == 0 || strcmp(token, "step") == 0) {
        debugger_step_into(dbg);
        debugger_print_frame(dbg);
    }
    // 单步跳过
    else if (strcmp(token, "n") == 0 || strcmp(token, "next") == 0) {
        debugger_step_over(dbg);
        debugger_print_frame(dbg);
    }
    // 单步跳出
    else if (strcmp(token, "f") == 0 || strcmp(token, "finish") == 0) {
        debugger_step_out(dbg);
        debugger_print_frame(dbg);
    }
    // 设置断点
    else if (strcmp(token, "b") == 0 || strcmp(token, "break") == 0) {
        char* arg = strtok(NULL, " \t");
        if (arg) {
            uint32_t addr = (uint32_t)atoi(arg);
            debugger_add_breakpoint(dbg, addr);
        } else {
            printf("用法: b <地址>\n");
        }
    }
    // 删除断点
    else if (strcmp(token, "d") == 0 || strcmp(token, "delete") == 0) {
        char* arg = strtok(NULL, " \t");
        if (arg) {
            uint32_t addr = (uint32_t)atoi(arg);
            debugger_remove_breakpoint(dbg, addr);
        } else {
            printf("用法: d <地址>\n");
        }
    }
    // info 命令
    else if (strcmp(token, "info") == 0) {
        char* arg = strtok(NULL, " \t");
        if (!arg) {
            printf("用法: info [breakpoints|frame|locals|globals]\n");
        } else if (strcmp(arg, "breakpoints") == 0 || strcmp(arg, "b") == 0) {
            debugger_list_breakpoints(dbg);
        } else if (strcmp(arg, "frame") == 0) {
            debugger_print_frame(dbg);
        } else if (strcmp(arg, "locals") == 0) {
            debugger_print_locals(dbg);
        } else if (strcmp(arg, "globals") == 0) {
            debugger_print_globals(dbg);
        } else {
            printf("未知 info 子命令: %s\n", arg);
        }
    }
    // 调用栈
    else if (strcmp(token, "bt") == 0 || strcmp(token, "backtrace") == 0) {
        debugger_print_backtrace(dbg);
    }
    // 操作数栈
    else if (strcmp(token, "stack") == 0) {
        debugger_print_stack(dbg);
    }
    // 反汇编
    else if (strcmp(token, "disasm") == 0) {
        char* addr_str = strtok(NULL, " \t");
        char* count_str = strtok(NULL, " \t");
        
        uint32_t addr = addr_str ? (uint32_t)atoi(addr_str) : dbg->vm->pc;
        int count = count_str ? atoi(count_str) : 10;
        
        debugger_disassemble(dbg, addr, count);
    }
    // 退出
    else if (strcmp(token, "q") == 0 || strcmp(token, "quit") == 0) {
        return false;
    }
    // 未知命令
    else {
        printf("未知命令: %s (输入 'help' 查看帮助)\n", token);
    }
    
    return true;
}

/**
 * @brief 启动调试会话
 */
int debugger_run(Debugger* dbg) {
    if (!dbg || !dbg->vm) {
        return 1;
    }
    
    printf("STVM 调试器\n");
    printf("输入 'help' 查看命令列表\n\n");
    
    // 显示初始状态
    debugger_print_frame(dbg);
    
    char line[512];
    
    while (true) {
        printf("\n(stdb) ");
        fflush(stdout);
        
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        
        // 移除换行符
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        
        if (!debugger_handle_command(dbg, line)) {
            break;
        }
    }
    
    return 0;
}
