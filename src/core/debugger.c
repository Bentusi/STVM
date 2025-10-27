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
#include "force.h"

// 辅助函数：将字符串解析为Value
static bool parse_value_from_string(const char* type_str, const char* value_str, Value* out_value);

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
    
    // 如果VM还没有启动，需要启动它
    if (!vm->running) {
        vm->running = true;
        vm->error_code = OK;
    }
    
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
    if (!dbg || !dbg->vm) {
        return ERR_RUNTIME;
    }
    
    dbg->state = DEBUG_STEP_INTO;
    VM* vm = dbg->vm;
    
    // 如果VM还没有启动，需要启动它
    if (!vm->running) {
        vm->running = true;
        vm->error_code = OK;
    }
    
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
    
    // 如果VM还没有启动，需要启动它
    if (!vm->running) {
        vm->running = true;
        vm->error_code = OK;
    }
    
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
    
    // 如果VM还没有启动，需要启动它
    if (!vm->running) {
        vm->running = true;
        vm->error_code = OK;
    }
    
    while (vm->running) {
        // 检查断点
        if (debugger_check_breakpoint(dbg, vm->pc)) {
            printf("\n断点命中于地址 %u\n", vm->pc);
            debugger_print_frame(dbg);
            dbg->state = DEBUG_PAUSED;
            return OK;
        }
        
        ErrorCode err = debugger_step_instruction(dbg);
        if (err != OK) {
            return err;
        }
    }
    
    printf("程序正常结束\n");
    dbg->state = DEBUG_STOPPED;
    return OK;
}

// ========================= Force 命令辅助函数 =========================

/**
 * @brief 将字符串解析为Value
 * @param type_str 类型字符串 ("int", "real", "bool", "string")
 * @param value_str 值字符串
 * @param out_value 输出Value
 * @return 成功返回true
 */
static bool parse_value_from_string(const char* type_str, const char* value_str, Value* out_value) {
    if (!type_str || !value_str || !out_value) {
        return false;
    }
    
    // INT
    if (strcasecmp(type_str, "int") == 0) {
        out_value->type = TYPE_INT;
        out_value->int_val = atoi(value_str);
        out_value->quality = QUALITY_GOOD;
        return true;
    }
    // REAL
    else if (strcasecmp(type_str, "real") == 0 || strcasecmp(type_str, "float") == 0) {
        out_value->type = TYPE_REAL;
        out_value->real_val = atof(value_str);
        out_value->quality = QUALITY_GOOD;
        return true;
    }
    // BOOL
    else if (strcasecmp(type_str, "bool") == 0) {
        out_value->type = TYPE_BOOL;
        if (strcasecmp(value_str, "true") == 0 || strcmp(value_str, "1") == 0) {
            out_value->bool_val = true;
        } else if (strcasecmp(value_str, "false") == 0 || strcmp(value_str, "0") == 0) {
            out_value->bool_val = false;
        } else {
            return false;
        }
        out_value->quality = QUALITY_GOOD;
        return true;
    }
    // STRING
    else if (strcasecmp(type_str, "string") == 0) {
        out_value->type = TYPE_STRING;
        strncpy(out_value->string_val, value_str, sizeof(out_value->string_val) - 1);
        out_value->string_val[sizeof(out_value->string_val) - 1] = '\0';
        out_value->quality = QUALITY_GOOD;
        return true;
    }
    // 质量化类型
    else if (strcasecmp(type_str, "qint") == 0) {
        out_value->type = TYPE_QINT;
        out_value->int_val = atoi(value_str);
        out_value->quality = QUALITY_GOOD;
        return true;
    }
    else if (strcasecmp(type_str, "qreal") == 0) {
        out_value->type = TYPE_QREAL;
        out_value->real_val = atof(value_str);
        out_value->quality = QUALITY_GOOD;
        return true;
    }
    else if (strcasecmp(type_str, "qbool") == 0) {
        out_value->type = TYPE_QBOOL;
        out_value->bool_val = (strcasecmp(value_str, "true") == 0 || strcmp(value_str, "1") == 0);
        out_value->quality = QUALITY_GOOD;
        return true;
    }
    
    return false;
}

/**
 * @brief 处理 force 命令
 * 用法: force <var_name> <type> <value> [persistent]
 * 例如: force counter int 100
 *       force temperature real 25.5 persistent
 */
static void cmd_force(Debugger* dbg, char* args) {
    if (!dbg || !dbg->vm) {
        printf("错误：无效的调试器或VM实例\n");
        return;
    }
    
    ForceManager* mgr = vm_get_force_manager(dbg->vm);
    if (!mgr) {
        printf("错误：Force Manager 未初始化\n");
        return;
    }
    
    // 解析参数
    char* var_name = strtok(args, " \t");
    char* type_str = strtok(NULL, " \t");
    char* value_str = strtok(NULL, " \t");
    char* next_arg = strtok(NULL, " \t");
    
    if (!var_name || !type_str || !value_str) {
        printf("用法: force <var_name> <type> <value> [quality] [persistent]\n");
        printf("类型: int, real, bool, string, qint, qreal, qbool, qstring\n");
        printf("质量位: good, uncertain, bad, error (仅对质量化类型有效)\n");
        printf("示例: force counter int 100\n");
        printf("      force temp qreal 25.5 bad persistent\n");
        printf("      force alarm qbool 1 uncertain\n");
        return;
    }
    
    // 解析值
    Value value;
    if (!parse_value_from_string(type_str, value_str, &value)) {
        printf("错误：无法解析类型 '%s' 的值 '%s'\n", type_str, value_str);
        return;
    }
    
    // 解析可选参数（质量位和persistent）
    bool persistent = false;
    QualityFlag quality = QUALITY_GOOD;
    
    while (next_arg) {
        if (strcasecmp(next_arg, "persistent") == 0) {
            persistent = true;
        } else if (strcasecmp(next_arg, "good") == 0 || strcmp(next_arg, "0") == 0) {
            quality = QUALITY_GOOD;
        } else if (strcasecmp(next_arg, "uncertain") == 0 || strcmp(next_arg, "1") == 0) {
            quality = QUALITY_UNCERTAIN;
        } else if (strcasecmp(next_arg, "bad") == 0 || strcmp(next_arg, "2") == 0) {
            quality = QUALITY_BAD;
        } else if (strcasecmp(next_arg, "error") == 0 || strcmp(next_arg, "3") == 0) {
            quality = QUALITY_ERROR;
        } else {
            printf("警告：忽略未知参数 '%s'\n", next_arg);
        }
        next_arg = strtok(NULL, " \t");
    }
    
    // 对于质量化类型，设置质量位
    if (value.type == TYPE_QINT || value.type == TYPE_QREAL || 
        value.type == TYPE_QBOOL || value.type == TYPE_QSTRING) {
        value.quality = quality;
    }
    
    // 强制变量
    if (vm_force_variable(dbg->vm, var_name, value, persistent)) {
        printf("已强制变量 '%s' = ", var_name);
        switch (value.type) {
            case TYPE_INT:
            case TYPE_QINT:
                printf("%d", value.int_val);
                break;
            case TYPE_REAL:
            case TYPE_QREAL:
                printf("%f", value.real_val);
                break;
            case TYPE_BOOL:
            case TYPE_QBOOL:
                printf("%s", value.bool_val ? "TRUE" : "FALSE");
                break;
            case TYPE_STRING:
            case TYPE_QSTRING:
                printf("\"%s\"", value.string_val);
                break;
            default:
                printf("?");
        }
        
        // 显示质量位
        if (value.type == TYPE_QINT || value.type == TYPE_QREAL || 
            value.type == TYPE_QBOOL || value.type == TYPE_QSTRING) {
            printf(" [%s]", quality_to_string(value.quality));
        }
        
        printf(" (%s)\n", persistent ? "持久化" : "临时");
    } else {
        printf("错误：无法强制变量 '%s'\n", var_name);
    }
}

/**
 * @brief 处理 unforce 命令
 * 用法: unforce <var_name> | all
 */
static void cmd_unforce(Debugger* dbg, char* args) {
    if (!dbg || !dbg->vm) {
        printf("错误：无效的调试器或VM实例\n");
        return;
    }
    
    ForceManager* mgr = vm_get_force_manager(dbg->vm);
    if (!mgr) {
        printf("错误：Force Manager 未初始化\n");
        return;
    }
    
    char* var_name = strtok(args, " \t");
    if (!var_name) {
        printf("用法: unforce <var_name> | all\n");
        printf("示例: unforce counter\n");
        printf("      unforce all\n");
        return;
    }
    
    // 取消全部
    if (strcasecmp(var_name, "all") == 0) {
        int32_t count = vm_unforce_all(dbg->vm);
        printf("已取消 %d 个强制变量\n", count);
    }
    // 取消单个
    else {
        if (vm_unforce_variable(dbg->vm, var_name)) {
            printf("已取消变量 '%s' 的强制\n", var_name);
        } else {
            printf("变量 '%s' 未被强制\n", var_name);
        }
    }
}

/**
 * @brief 处理 force_status 命令
 * 显示所有强制变量的状态
 */
static void cmd_force_status(Debugger* dbg) {
    if (!dbg || !dbg->vm) {
        printf("错误：无效的调试器或VM实例\n");
        return;
    }
    
    vm_print_force_status(dbg->vm);
}

/**
 * @brief 处理 force_enable 命令
 * 用法: force_enable on | off
 */
static void cmd_force_enable(Debugger* dbg, char* args) {
    if (!dbg || !dbg->vm) {
        printf("错误：无效的调试器或VM实例\n");
        return;
    }
    
    ForceManager* mgr = vm_get_force_manager(dbg->vm);
    if (!mgr) {
        printf("错误：Force Manager 未初始化\n");
        return;
    }
    
    char* state_str = strtok(args, " \t");
    if (!state_str) {
        // 显示当前状态
        bool enabled = vm_is_force_enabled(dbg->vm);
        printf("Force 功能当前状态: %s\n", enabled ? "启用" : "禁用");
        return;
    }
    
    // 设置状态
    if (strcasecmp(state_str, "on") == 0 || strcasecmp(state_str, "1") == 0 || 
        strcasecmp(state_str, "true") == 0) {
        vm_force_enable(dbg->vm, true);
        printf("Force 功能已启用\n");
    } else if (strcasecmp(state_str, "off") == 0 || strcasecmp(state_str, "0") == 0 || 
               strcasecmp(state_str, "false") == 0) {
        vm_force_enable(dbg->vm, false);
        printf("Force 功能已禁用\n");
    } else {
        printf("用法: force_enable [on|off]\n");
        printf("示例: force_enable on\n");
        printf("      force_enable off\n");
    }
}

/**
 * @brief 处理 force_save 命令
 * 用法: force_save <filename>
 */
static void cmd_force_save(Debugger* dbg, char* args) {
    if (!dbg || !dbg->vm) {
        printf("错误：无效的调试器或VM实例\n");
        return;
    }
    
    ForceManager* mgr = vm_get_force_manager(dbg->vm);
    if (!mgr) {
        printf("错误：Force Manager 未初始化\n");
        return;
    }
    
    char* filename = strtok(args, " \t");
    if (!filename) {
        printf("用法: force_save <filename>\n");
        printf("示例: force_save debug.force\n");
        printf("      force_save /tmp/my_forces.txt\n");
        return;
    }
    
    if (force_save_to_file(mgr, filename)) {
        printf("Force 配置已保存到 '%s'\n", filename);
    } else {
        printf("错误：无法保存到 '%s'\n", filename);
    }
}

/**
 * @brief 处理 force_load 命令
 * 用法: force_load <filename>
 */
static void cmd_force_load(Debugger* dbg, char* args) {
    if (!dbg || !dbg->vm) {
        printf("错误：无效的调试器或VM实例\n");
        return;
    }
    
    ForceManager* mgr = vm_get_force_manager(dbg->vm);
    if (!mgr) {
        printf("错误：Force Manager 未初始化\n");
        return;
    }
    
    char* filename = strtok(args, " \t");
    if (!filename) {
        printf("用法: force_load <filename>\n");
        printf("示例: force_load debug.force\n");
        printf("      force_load /tmp/my_forces.txt\n");
        return;
    }
    
    if (force_load_from_file(mgr, filename)) {
        printf("Force 配置已从 '%s' 加载\n", filename);
    } else {
        printf("错误：无法从 '%s' 加载\n", filename);
    }
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
    printf("\n变量强制命令 (Force):\n");
    printf("  force <var> <type> <val> [persistent]  强制变量值\n");
    printf("                       类型: int, real, bool, string\n");
    printf("                       示例: force counter int 100\n");
    printf("                             force temp real 25.5 persistent\n");
    printf("  unforce <var>        取消变量强制\n");
    printf("  unforce all          取消所有强制\n");
    printf("  force_status         显示所有强制变量状态\n");
    printf("  force_enable [on|off] 启用/禁用Force功能\n");
    printf("  force_save <file>    保存Force配置到文件\n");
    printf("  force_load <file>    从文件加载Force配置\n");
    printf("\n");
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
    // Force 命令
    else if (strcmp(token, "force") == 0) {
        char* remaining = strtok(NULL, "");  // 获取剩余所有参数
        cmd_force(dbg, remaining);
    }
    else if (strcmp(token, "unforce") == 0) {
        char* remaining = strtok(NULL, "");
        cmd_unforce(dbg, remaining);
    }
    else if (strcmp(token, "force_status") == 0) {
        cmd_force_status(dbg);
    }
    else if (strcmp(token, "force_enable") == 0) {
        char* remaining = strtok(NULL, "");
        cmd_force_enable(dbg, remaining);
    }
    else if (strcmp(token, "force_save") == 0) {
        char* remaining = strtok(NULL, "");
        cmd_force_save(dbg, remaining);
    }
    else if (strcmp(token, "force_load") == 0) {
        char* remaining = strtok(NULL, "");
        cmd_force_load(dbg, remaining);
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
        printf("(stdb) ");
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
