/*
 * IEC61131 ST语言虚拟机实现
 * 支持栈式执行、库函数调用和主从同步
 */

#include "symbol_table.h"
#include "vm.h"
#include "mmgr.h"
#include "libmgr.h"
#include "ms_sync.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/* 内部辅助函数声明 */
static int vm_load_bytecode_from_file(st_vm_t *vm, const bytecode_file_t *bc_file);
static int vm_execute_main_loop(st_vm_t *vm);
static int vm_handle_sync_operations(st_vm_t *vm);
static void vm_update_statistics(st_vm_t *vm, opcode_t opcode);
static bool vm_check_execution_limits(st_vm_t *vm);

/* ========== 虚拟机生命周期管理 ========== */

/* 创建虚拟机实例 */
st_vm_t* vm_create(void) {
    st_vm_t *vm = (st_vm_t*)mmgr_alloc_general(sizeof(st_vm_t));
    if (!vm) {
        return NULL;
    }

    memset(vm, 0, sizeof(st_vm_t));

    /* 初始化栈 */
    vm->operand_stack.top = -1;
    vm->operand_stack.capacity = MAX_STACK_SIZE;
    vm->call_stack_top = -1;

    /* 初始化状态 */
    vm->state = VM_STATE_INIT;
    vm->pc = 0;
    vm->sync_mode = VM_SYNC_NONE;
    vm->sync_enabled = false;

    /* 初始化调试信息 */
    vm->debug_info.debug_enabled = false;
    vm->debug_info.step_mode = false;
    vm->debug_info.breakpoint_count = 0;
    vm->debug_info.breakpoints = (uint32_t*)mmgr_alloc_general(sizeof(uint32_t) * 100);

    /* 初始化同步变量数组 */
    vm->sync_var_indices = (uint32_t*)mmgr_alloc_general(sizeof(uint32_t) * MAX_SYNC_VARIABLES);

    if (!vm->debug_info.breakpoints || !vm->sync_var_indices) {
        return NULL;
    }

    return vm;
}

/* 销毁虚拟机实例 */
void vm_destroy(st_vm_t *vm) {
    if (!vm) return;

    vm_cleanup(vm);
    /* 内存由MMGR管理，不需要显式释放 */
}

/* 初始化虚拟机 */
int vm_init(st_vm_t *vm, const vm_config_t *config) {
    if (!vm) {
        return -1;
    }

    /* 应用配置 */
    if (config) {
        vm->debug_info.debug_enabled = config->enable_debug;
        vm->sync_enabled = config->enable_sync;
        vm->sync_mode = config->sync_mode;
    }

    /* 重置统计信息 */
    vm_reset_statistics(vm);

    /* 初始化全局变量区 */
    vm_init_global_vars(vm, MAX_GLOBAL_VARS);

    vm->state = VM_STATE_INIT;

    return 0;
}

/* 重置虚拟机状态 */
void vm_reset(st_vm_t *vm) {
    if (!vm) return;

    /* 重置程序计数器和栈 */
    vm->pc = 0;
    vm_stack_clear(vm);
    vm->call_stack_top = -1;

    /* 重置状态 */
    vm->state = VM_STATE_INIT;
    vm_clear_error(vm);

    /* 重置调试状态 */
    vm->debug_info.current_line = 0;
    vm->debug_info.current_column = 0;
    vm->debug_info.step_mode = false;

    /* 清理变量区 */
    memset(vm->global_vars, 0, sizeof(vm_value_t) * MAX_GLOBAL_VARS);
    memset(vm->local_vars, 0, sizeof(vm_value_t) * MAX_LOCAL_VARS);
}

/* 清理虚拟机资源 */
void vm_cleanup(st_vm_t *vm) {
    if (!vm) return;

    /* 清理字符串值 */
    for (uint32_t i = 0; i < vm->global_var_count; i++) {
        vm_value_free(&vm->global_vars[i]);
    }

    for (uint32_t i = 0; i < vm->local_var_count; i++) {
        vm_value_free(&vm->local_vars[i]);
    }

    /* 清理栈中的字符串值 */
    vm_stack_clear(vm);

    vm->state = VM_STATE_STOPPED;
}

/* ========== 字节码加载和执行 ========== */

/* 从文件加载字节码 */
int vm_load_bytecode_file(st_vm_t *vm, const char *filename) {
    if (!vm || !filename) {
        vm_set_error(vm, "Invalid parameters for bytecode loading");
        return -1;
    }

    bytecode_file_t bc_file;
    if (bytecode_load_file(&bc_file, filename) != 0) {
        vm_set_error(vm, "Failed to load bytecode file");
        return -1;
    }

    if (!bytecode_verify_file(&bc_file)) {
        vm_set_error(vm, "Invalid bytecode file format");
        return -1;
    }

    return vm_load_bytecode_from_file(vm, &bc_file);
}

/* 加载字节码数据 */
int vm_load_bytecode(st_vm_t *vm, const bytecode_file_t *bc_file) {
    if (!vm || !bc_file) {
        vm_set_error(vm, "Invalid parameters for bytecode loading");
        return -1;
    }

    return vm_load_bytecode_from_file(vm, bc_file);
}

/* 内部字节码加载函数 */
static int vm_load_bytecode_from_file(st_vm_t *vm, const bytecode_file_t *bc_file) {
    /* 复制指令序列 */
    vm->instr_count = bc_file->header.instr_count;
    vm->instructions = bc_file->instructions;

    /* 复制常量池 */
    vm->const_pool = bc_file->const_pool;
    vm->const_pool_size = bc_file->header.const_pool_size;

    /* 设置程序计数器到入口点 */
    vm->pc = bc_file->header.entry_point;

    /* 初始化全局变量区 */
    vm->global_var_count = bc_file->header.var_count;
    if (vm->global_var_count > MAX_GLOBAL_VARS) {
        vm_set_error(vm, "Too many global variables");
        return -1;
    }

    /* 从变量表初始化全局变量 */
    for (uint32_t i = 0; i < vm->global_var_count && i < MAX_GLOBAL_VARS; i++) {
        if (bc_file->var_table) {
            const var_descriptor_t *var_desc = &bc_file->var_table[i];
            vm->global_vars[i] = vm_create_undefined_value();

            /* 根据类型设置默认值 */
            switch (var_desc->type) {
            case TYPE_BOOL_ID:
                vm->global_vars[i] = vm_create_bool_value(false);
                break;
            case TYPE_INT_ID:
            case TYPE_DINT_ID:
                vm->global_vars[i] = vm_create_int_value(0);
                break;
            case TYPE_REAL_ID:
                vm->global_vars[i] = vm_create_real_value(0.0);
                break;
            case TYPE_STRING_ID:
                vm->global_vars[i] = vm_create_string_value("");
                break;
            }
        }
    }

    vm->state = VM_STATE_STOPPED;
    return 0;
}

/* 验证字节码 */
int vm_verify_bytecode(st_vm_t *vm) {
    if (!vm || !vm->instructions) {
        return -1;
    }

    /* 简单验证：检查所有跳转指令的目标地址 */
    for (uint32_t i = 0; i < vm->instr_count; i++) {
        const bytecode_instr_t *instr = &vm->instructions[i];

        switch (instr->opcode) {
        case OP_JMP:
        case OP_JMP_TRUE:
        case OP_JMP_FALSE:
        case OP_CALL:
            if (instr->operand.addr_operand >= vm->instr_count) {
                vm_set_error(vm, "Invalid jump target address");
                return -1;
            }
            break;

        default:
            break;
        }
    }

    return 0;
}

/* 执行字节码 */
int vm_execute(st_vm_t *vm) {
    if (!vm || vm->state == VM_STATE_ERROR) {
        return -1;
    }

    if (!vm->instructions) {
        vm_set_error(vm, "No bytecode loaded");
        return -1;
    }

    vm->state = VM_STATE_RUNNING;
    vm->statistics.execution_time_ms = clock();

    int result = vm_execute_main_loop(vm);

    vm->statistics.execution_time_ms = clock() - vm->statistics.execution_time_ms;
    vm->statistics.execution_time_ms = (vm->statistics.execution_time_ms * 1000) / CLOCKS_PER_SEC;

    return result;
}

/* 主执行循环 */
static int vm_execute_main_loop(st_vm_t *vm) {
    while (vm->state == VM_STATE_RUNNING && vm->pc < vm->instr_count) {
        /* 检查执行限制 */
        if (!vm_check_execution_limits(vm)) {
            vm_set_error(vm, "Execution limit exceeded");
            vm->state = VM_STATE_ERROR;
            return -1;
        }

        /* 处理同步操作 */
        if (vm->sync_enabled) {
            vm_handle_sync_operations(vm);
        }

        /* 检查断点 */
        if (vm_is_breakpoint(vm, vm->pc)) {
            vm->state = VM_STATE_PAUSED;
            break;
        }

        /* 执行单条指令 */
        if (vm_execute_single_step(vm) != 0) {
            return -1;
        }

        /* 单步模式检查 */
        if (vm->debug_info.step_mode) {
            vm->state = VM_STATE_PAUSED;
            break;
        }
    }

    if (vm->pc >= vm->instr_count && vm->state == VM_STATE_RUNNING) {
        vm->state = VM_STATE_STOPPED;
    }

    return 0;
}

/* 执行单步 */
int vm_execute_single_step(st_vm_t *vm) {
    if (!vm || vm->pc >= vm->instr_count) {
        return -1;
    }

    const bytecode_instr_t *instr = &vm->instructions[vm->pc];

    /* 更新调试信息 */
    vm->debug_info.current_line = instr->source_line;
    vm->debug_info.current_column = instr->source_column;

    /* 执行指令 */
    int result = vm_execute_instruction(vm, instr);

    /* 更新统计信息 */
    if (result == 0) {
        vm_update_statistics(vm, instr->opcode);
    } else {
        vm->statistics.runtime_errors++;
    }

    return result;
}

/* ========== 栈操作函数 ========== */

/* 压栈 */
int vm_stack_push(st_vm_t *vm, const vm_value_t *value) {
    if (!vm || !value || vm->operand_stack.top >= MAX_STACK_SIZE - 1) {
        return -1;
    }

    vm->operand_stack.top++;
    vm_value_copy(value, &vm->operand_stack.data[vm->operand_stack.top]);

    return 0;
}

/* 弹栈 */
vm_value_t* vm_stack_pop(st_vm_t *vm) {
    if (!vm || vm->operand_stack.top < 0) {
        return NULL;
    }

    vm_value_t *value = &vm->operand_stack.data[vm->operand_stack.top];
    vm->operand_stack.top--;

    return value;
}

/* 获取栈顶 */
vm_value_t* vm_stack_top(st_vm_t *vm) {
    if (!vm || vm->operand_stack.top < 0) {
        return NULL;
    }

    return &vm->operand_stack.data[vm->operand_stack.top];
}

/* 清空栈 */
void vm_stack_clear(st_vm_t *vm) {
    if (!vm) return;

    /* 释放栈中的字符串值 */
    for (int32_t i = 0; i <= vm->operand_stack.top; i++) {
        vm_value_free(&vm->operand_stack.data[i]);
    }

    vm->operand_stack.top = -1;
}

/* 检查栈是否为空 */
bool vm_stack_is_empty(const st_vm_t *vm) {
    return vm ? (vm->operand_stack.top < 0) : true;
}

/* 获取栈大小 */
int32_t vm_stack_size(const st_vm_t *vm) {
    return vm ? (vm->operand_stack.top + 1) : 0;
}

/* ========== 调用栈操作（补充完整） ========== */

/* 压入调用帧 */
int vm_call_stack_push(st_vm_t *vm, uint32_t return_addr, const char *func_name) {
    if (!vm || vm->call_stack_top >= MAX_CALL_FRAMES - 1) {
        return -1;
    }

    vm->call_stack_top++;
    call_frame_t *frame = &vm->call_stack[vm->call_stack_top];

    frame->return_address = return_addr;
    frame->local_var_base = vm->operand_stack.top + 1;
    frame->param_base = vm->operand_stack.top;
    frame->param_count = 0;
    frame->function_name = func_name ? mmgr_alloc_string(func_name) : NULL;

    return 0;
}

/* 弹出调用帧 */
call_frame_t* vm_call_stack_pop(st_vm_t *vm) {
    if (!vm || vm->call_stack_top < 0) {
        return NULL;
    }

    call_frame_t *frame = &vm->call_stack[vm->call_stack_top];
    vm->call_stack_top--;

    return frame;
}

/* 获取当前调用帧 */
call_frame_t* vm_call_stack_top(st_vm_t *vm) {
    if (!vm || vm->call_stack_top < 0) {
        return NULL;
    }

    return &vm->call_stack[vm->call_stack_top];
}

/* 获取调用栈深度 */
int32_t vm_call_stack_depth(const st_vm_t *vm) {
    return vm ? (vm->call_stack_top + 1) : 0;
}

/* ========== 变量操作（补充完整） ========== */

/* 设置局部变量 */
int vm_set_local_var(st_vm_t *vm, uint32_t index, const vm_value_t *value) {
    if (!vm || !value || index >= MAX_LOCAL_VARS) {
        return -1;
    }

    vm_value_free(&vm->local_vars[index]);
    vm_value_copy(value, &vm->local_vars[index]);

    return 0;
}

/* 获取局部变量 */
vm_value_t* vm_get_local_var(st_vm_t *vm, uint32_t index) {
    if (!vm || index >= MAX_LOCAL_VARS) {
        return NULL;
    }

    return &vm->local_vars[index];
}

/* 初始化局部变量区 */
int vm_init_local_vars(st_vm_t *vm, uint32_t count) {
    if (!vm || count > MAX_LOCAL_VARS) {
        return -1;
    }

    vm->local_var_count = count;

    for (uint32_t i = 0; i < count; i++) {
        vm->local_vars[i] = vm_create_undefined_value();
    }

    return 0;
}

/* 设置参数 */
int vm_set_param(st_vm_t *vm, uint32_t index, const vm_value_t *value) {
    if (!vm || !value) {
        return -1;
    }

    call_frame_t *frame = vm_call_stack_top(vm);
    if (!frame || index >= frame->param_count) {
        return -1;
    }

    /* 参数存储在栈中 */
    int32_t param_index = frame->param_base - frame->param_count + index + 1;
    if (param_index < 0 || param_index > vm->operand_stack.top) {
        return -1;
    }

    vm_value_copy(value, &vm->operand_stack.data[param_index]);

    return 0;
}

/* 获取参数 */
vm_value_t* vm_get_param(st_vm_t *vm, uint32_t index) {
    if (!vm) return NULL;

    call_frame_t *frame = vm_call_stack_top(vm);
    if (!frame || index >= frame->param_count) {
        return NULL;
    }

    int32_t param_index = frame->param_base - frame->param_count + index + 1;
    if (param_index < 0 || param_index > vm->operand_stack.top) {
        return NULL;
    }

    return &vm->operand_stack.data[param_index];
}

/* ========== 值类型操作 ========== */

/* 创建各种类型的值 */
vm_value_t vm_create_bool_value(bool value) {
    vm_value_t val;
    val.type = VAL_BOOL;
    val.data.bool_val = value;
    return val;
}

vm_value_t vm_create_int_value(int32_t value) {
    vm_value_t val;
    val.type = VAL_INT;
    val.data.int_val = value;
    return val;
}

vm_value_t vm_create_real_value(double value) {
    vm_value_t val;
    val.type = VAL_REAL;
    val.data.real_val = value;
    return val;
}

vm_value_t vm_create_string_value(const char *value) {
    vm_value_t val;
    val.type = VAL_STRING;
    val.data.string_val = value ? mmgr_alloc_string(value) : NULL;
    return val;
}

vm_value_t vm_create_undefined_value(void) {
    vm_value_t val;
    val.type = VAL_UNDEFINED;
    memset(&val.data, 0, sizeof(val.data));
    return val;
}

/* 值复制 */
void vm_value_copy(const vm_value_t *src, vm_value_t *dst) {
    if (!src || !dst) return;

    /* 先释放目标的资源 */
    vm_value_free(dst);

    dst->type = src->type;

    switch (src->type) {
    case VAL_BOOL:
        dst->data.bool_val = src->data.bool_val;
        break;
    case VAL_INT:
        dst->data.int_val = src->data.int_val;
        break;
    case VAL_REAL:
        dst->data.real_val = src->data.real_val;
        break;
    case VAL_STRING:
        dst->data.string_val = src->data.string_val ? mmgr_alloc_string(src->data.string_val) : NULL;
        break;
    default:
        memset(&dst->data, 0, sizeof(dst->data));
        break;
    }
}

/* 释放值资源 */
void vm_value_free(vm_value_t *value) {
    if (!value) return;

    if (value->type == VAL_STRING && value->data.string_val) {
        /* 字符串由MMGR管理，不需要显式释放 */
        value->data.string_val = NULL;
    }

    value->type = VAL_UNDEFINED;
    memset(&value->data, 0, sizeof(value->data));
}

/* ========== 指令执行 ========== */

/* 执行单条指令 */
int vm_execute_instruction(st_vm_t *vm, const bytecode_instr_t *instr) {
    if (!vm || !instr) {
        return -1;
    }

    switch (instr->opcode) {
    case OP_NOP:
        break;

    case OP_HALT:
        vm->state = VM_STATE_STOPPED;
        return 0;

    case OP_LOAD_CONST_INT: {
        vm_value_t value = vm_create_int_value(instr->operand.int_operand);
        return vm_stack_push(vm, &value);
    }

    case OP_LOAD_CONST_REAL: {
        vm_value_t value = vm_create_real_value(instr->operand.real_operand);
        return vm_stack_push(vm, &value);
    }

    case OP_LOAD_CONST_BOOL: {
        vm_value_t value = vm_create_bool_value(instr->operand.int_operand != 0);
        return vm_stack_push(vm, &value);
    }

    case OP_LOAD_GLOBAL: {
        vm_value_t *var = vm_get_global_var(vm, instr->operand.int_operand);
        if (!var) {
            vm_set_error(vm, "Invalid global variable index");
            return -1;
        }
        return vm_stack_push(vm, var);
    }

    case OP_STORE_GLOBAL: {
        vm_value_t *value = vm_stack_pop(vm);
        if (!value) {
            vm_set_error(vm, "Stack underflow");
            return -1;
        }
        return vm_set_global_var(vm, instr->operand.int_operand, value);
    }

        /* 算术运算指令 */
    case OP_ADD_INT:
    case OP_ADD_REAL:
        return vm_exec_add(vm, (instr->opcode == OP_ADD_INT) ? VAL_INT : VAL_REAL);

    case OP_SUB_INT:
    case OP_SUB_REAL:
        return vm_exec_sub(vm, (instr->opcode == OP_SUB_INT) ? VAL_INT : VAL_REAL);

    case OP_MUL_INT:
    case OP_MUL_REAL:
        return vm_exec_mul(vm, (instr->opcode == OP_MUL_INT) ? VAL_INT : VAL_REAL);

    case OP_DIV_INT:
    case OP_DIV_REAL:
        return vm_exec_div(vm, (instr->opcode == OP_DIV_INT) ? VAL_INT : VAL_REAL);

    case OP_MOD_INT:
        return vm_exec_mod(vm);

    case OP_NEG_INT:
    case OP_NEG_REAL:
        return vm_exec_neg(vm, (instr->opcode == OP_NEG_INT) ? VAL_INT : VAL_REAL);

        /* 控制流指令 */
    case OP_JMP:
        return vm_exec_jump(vm, instr->operand.addr_operand);

    case OP_JMP_TRUE: {
        vm_value_t *cond = vm_stack_pop(vm);
        if (!cond) {
            vm_set_error(vm, "Stack underflow");
            return -1;
        }

        bool condition = false;
        if (cond->type == VAL_BOOL) {
            condition = cond->data.bool_val;
        }

        return vm_exec_jump_conditional(vm, instr->operand.addr_operand, condition);
    }

    case OP_JMP_FALSE: {
        vm_value_t *cond = vm_stack_pop(vm);
        if (!cond) {
            vm_set_error(vm, "Stack underflow");
            return -1;
        }

        bool condition = true;
        if (cond->type == VAL_BOOL) {
            condition = !cond->data.bool_val;
        }

        return vm_exec_jump_conditional(vm, instr->operand.addr_operand, condition);
    }

        /* 同步指令 */
    case OP_SYNC_VAR:
        if (vm->sync_enabled) {
            return vm_sync_variable(vm, instr->operand.int_operand);
        }
        break;

    case OP_SYNC_CHECKPOINT:
        if (vm->sync_enabled) {
            return vm_send_sync_checkpoint(vm);
        }
        break;

    default:
        vm_set_error(vm, "Unknown instruction");
        return -1;
    }

    /* 移动到下一条指令 */
    vm->pc++;

    return 0;
}

/* ========== 算术运算指令实现 ========== */

int vm_exec_add(st_vm_t *vm, value_type_t type) {
    vm_value_t *b = vm_stack_pop(vm);
    vm_value_t *a = vm_stack_pop(vm);

    if (!a || !b) {
        vm_set_error(vm, "Stack underflow in ADD");
        return -1;
    }

    vm_value_t result;

    if (type == VAL_INT) {
        if (a->type != VAL_INT || b->type != VAL_INT) {
            vm_set_error(vm, "Type mismatch in ADD_INT");
            return -1;
        }
        result = vm_create_int_value(a->data.int_val + b->data.int_val);
    } else {
        if (a->type != VAL_REAL || b->type != VAL_REAL) {
            vm_set_error(vm, "Type mismatch in ADD_REAL");
            return -1;
        }
        result = vm_create_real_value(a->data.real_val + b->data.real_val);
    }

    return vm_stack_push(vm, &result);
}

int vm_exec_sub(st_vm_t *vm, value_type_t type) {
    vm_value_t *b = vm_stack_pop(vm);
    vm_value_t *a = vm_stack_pop(vm);

    if (!a || !b) {
        vm_set_error(vm, "Stack underflow in SUB");
        return -1;
    }

    vm_value_t result;

    if (type == VAL_INT) {
        result = vm_create_int_value(a->data.int_val - b->data.int_val);
    } else {
        result = vm_create_real_value(a->data.real_val - b->data.real_val);
    }

    return vm_stack_push(vm, &result);
}

int vm_exec_mul(st_vm_t *vm, value_type_t type) {
    vm_value_t *b = vm_stack_pop(vm);
    vm_value_t *a = vm_stack_pop(vm);

    if (!a || !b) {
        vm_set_error(vm, "Stack underflow in MUL");
        return -1;
    }

    vm_value_t result;

    if (type == VAL_INT) {
        result = vm_create_int_value(a->data.int_val * b->data.int_val);
    } else {
        result = vm_create_real_value(a->data.real_val * b->data.real_val);
    }

    return vm_stack_push(vm, &result);
}

int vm_exec_div(st_vm_t *vm, value_type_t type) {
    vm_value_t *b = vm_stack_pop(vm);
    vm_value_t *a = vm_stack_pop(vm);

    if (!a || !b) {
        vm_set_error(vm, "Stack underflow in DIV");
        return -1;
    }

    vm_value_t result;

    if (type == VAL_INT) {
        if (b->data.int_val == 0) {
            vm_set_error(vm, "Division by zero");
            return -1;
        }
        result = vm_create_int_value(a->data.int_val / b->data.int_val);
    } else {
        if (b->data.real_val == 0.0) {
            vm_set_error(vm, "Division by zero");
            return -1;
        }
        result = vm_create_real_value(a->data.real_val / b->data.real_val);
    }

    return vm_stack_push(vm, &result);
}

int vm_exec_mod(st_vm_t *vm) {
    vm_value_t *b = vm_stack_pop(vm);
    vm_value_t *a = vm_stack_pop(vm);

    if (!a || !b || a->type != VAL_INT || b->type != VAL_INT) {
        vm_set_error(vm, "Invalid operands for MOD");
        return -1;
    }

    if (b->data.int_val == 0) {
        vm_set_error(vm, "Division by zero in MOD");
        return -1;
    }

    vm_value_t result = vm_create_int_value(a->data.int_val % b->data.int_val);
    return vm_stack_push(vm, &result);
}

int vm_exec_neg(st_vm_t *vm, value_type_t type) {
    vm_value_t *a = vm_stack_pop(vm);

    if (!a) {
        vm_set_error(vm, "Stack underflow in NEG");
        return -1;
    }

    vm_value_t result;

    if (type == VAL_INT) {
        result = vm_create_int_value(-a->data.int_val);
    } else {
        result = vm_create_real_value(-a->data.real_val);
    }

    return vm_stack_push(vm, &result);
}

/* ========== 控制流指令实现 ========== */

int vm_exec_jump(st_vm_t *vm, uint32_t address) {
    if (!vm_is_valid_pc(vm, address)) {
        vm_set_error(vm, "Invalid jump address");
        return -1;
    }

    vm->pc = address;
    return 0;
}

int vm_exec_jump_conditional(st_vm_t *vm, uint32_t address, bool condition) {
    if (condition) {
        return vm_exec_jump(vm, address);
    } else {
        vm->pc++;  // 继续执行下一条指令
        return 0;
    }
}

/* ========== 主从同步接口实现 ========== */

int vm_enable_sync(st_vm_t *vm, const sync_config_t *config, vm_sync_mode_t mode) {
    if (!vm || !config) {
        return -1;
    }

    /* 创建同步管理器 */
    vm->sync_mgr = ms_sync_create();
    if (!vm->sync_mgr) {
        return -1;
    }

    /* 初始化同步管理器 */
    node_role_t role = (mode == VM_SYNC_PRIMARY) ? NODE_ROLE_PRIMARY : NODE_ROLE_SECONDARY;
    if (ms_sync_init(vm->sync_mgr, config, role, vm) != 0) {
        ms_sync_destroy(vm->sync_mgr);
        vm->sync_mgr = NULL;
        return -1;
    }

    /* 启动网络连接 */
    if (ms_sync_start_network(vm->sync_mgr) != 0) {
        ms_sync_destroy(vm->sync_mgr);
        vm->sync_mgr = NULL;
        return -1;
    }

    vm->sync_enabled = true;
    vm->sync_mode = mode;

    return 0;
}

int vm_disable_sync(st_vm_t *vm) {
    if (!vm) return -1;

    if (vm->sync_mgr) {
        ms_sync_stop_network(vm->sync_mgr);
        ms_sync_destroy(vm->sync_mgr);
        vm->sync_mgr = NULL;
    }

    vm->sync_enabled = false;
    vm->sync_mode = VM_SYNC_NONE;

    return 0;
}

int vm_sync_variable(st_vm_t *vm, uint32_t var_index) {
    if (!vm || !vm->sync_enabled || !vm->sync_mgr) {
        return -1;
    }

    return ms_sync_send_variable_update(vm->sync_mgr, var_index);
}

int vm_process_sync_messages(st_vm_t *vm) {
    if (!vm || !vm->sync_enabled || !vm->sync_mgr) {
        return -1;
    }

    return ms_sync_process_messages(vm->sync_mgr);
}

bool vm_is_sync_variable(const st_vm_t *vm, uint32_t var_index) {
    if (!vm || !vm->sync_enabled) {
        return false;
    }

    for (uint32_t i = 0; i < vm->sync_var_count; i++) {
        if (vm->sync_var_indices[i] == var_index) {
            return true;
        }
    }

    return false;
}

/* ========== 调试支持 ========== */

bool vm_is_breakpoint(const st_vm_t *vm, uint32_t address) {
    if (!vm || !vm->debug_info.debug_enabled) {
        return false;
    }

    for (uint32_t i = 0; i < vm->debug_info.breakpoint_count; i++) {
        if (vm->debug_info.breakpoints[i] == address) {
            return true;
        }
    }

    return false;
}

/* ========== 错误处理 ========== */

void vm_set_error(st_vm_t *vm, const char *error_msg) {
    if (!vm || !error_msg) return;

    strncpy(vm->last_error, error_msg, sizeof(vm->last_error) - 1);
    vm->last_error[sizeof(vm->last_error) - 1] = '\0';
    vm->has_error = true;
    vm->state = VM_STATE_ERROR;
}

const char* vm_get_last_error(const st_vm_t *vm) {
    return vm ? vm->last_error : "Invalid VM instance";
}

void vm_clear_error(st_vm_t *vm) {
    if (!vm) return;

    vm->has_error = false;
    vm->last_error[0] = '\0';
}

/* ========== 辅助函数 ========== */

static int vm_handle_sync_operations(st_vm_t *vm) {
    if (!vm || !vm->sync_enabled) {
        return 0;
    }

    /* 处理同步消息 */
    vm_process_sync_messages(vm);

    return 0;
}

static void vm_update_statistics(st_vm_t *vm, opcode_t opcode) {
    if (!vm) return;

    vm->statistics.instructions_executed++;

    switch (opcode) {
    case OP_CALL:
    case OP_CALL_BUILTIN:
    case OP_CALL_LIBRARY:
        vm->statistics.function_calls++;
        break;
    case OP_SYNC_VAR:
    case OP_SYNC_CHECKPOINT:
        vm->statistics.sync_operations++;
        break;
    default:
        break;
    }
}

static bool vm_check_execution_limits(st_vm_t *vm) {
    if (!vm) return false;

    /* 简单的执行限制检查 */
    return vm->statistics.instructions_executed < 1000000; /* 100万指令限制 */
}

bool vm_is_valid_pc(const st_vm_t *vm, uint32_t pc) {
    return vm && pc < vm->instr_count;
}

void vm_reset_statistics(st_vm_t *vm) {
    if (!vm) return;

    memset(&vm->statistics, 0, sizeof(vm_statistics_t));
}

vm_state_t vm_get_state(const st_vm_t *vm) {
    return vm ? vm->state : VM_STATE_ERROR;
}

const char* vm_state_to_string(vm_state_t state) {
    switch (state) {
    case VM_STATE_INIT: return "INIT";
    case VM_STATE_RUNNING: return "RUNNING";
    case VM_STATE_PAUSED: return "PAUSED";
    case VM_STATE_STOPPED: return "STOPPED";
    case VM_STATE_ERROR: return "ERROR";
    case VM_STATE_SYNC_WAIT: return "SYNC_WAIT";
    default: return "UNKNOWN";
    }
}

/* ========== 库函数支持 ========== */

int vm_set_library_manager(st_vm_t *vm, struct library_manager *lib_mgr) {
    if (!vm) return -1;

    vm->lib_mgr = lib_mgr;
    return 0;
}

struct library_manager* vm_get_library_manager(const st_vm_t *vm) {
    return vm ? vm->lib_mgr : NULL;
}

int vm_call_library_function(st_vm_t *vm, const char *lib_name,
                             const char *func_name, uint32_t param_count) {
    if (!vm || !lib_name || !func_name || !vm->lib_mgr) {
        return -1;
    }

    /* 通过库管理器调用函数 */
    /* 这里需要与libmgr.h中的接口配合 */
    return 0; /* 简化实现 */
}

int vm_call_builtin_function(st_vm_t *vm, const char *func_name, uint32_t param_count) {
    if (!vm || !func_name) {
        return -1;
    }

    /* 调用内置函数 */
    return 0; /* 简化实现 */
}

/* ========== 诊断输出（补充完整） ========== */

void vm_print_stack(const st_vm_t *vm) {
    if (!vm) return;

    printf("=== VM Stack (top = %d) ===\n", vm->operand_stack.top);
    for (int32_t i = vm->operand_stack.top; i >= 0; i--) {
        printf("[%d] ", i);
        vm_print_value(&vm->operand_stack.data[i]);
        printf("\n");
    }
    printf("========================\n");
}

void vm_print_call_stack(const st_vm_t *vm) {
    if (!vm) return;

    printf("=== Call Stack (depth = %d) ===\n", vm->call_stack_top + 1);
    for (int32_t i = vm->call_stack_top; i >= 0; i--) {
        const call_frame_t *frame = &vm->call_stack[i];
        printf("[%d] %s (ret_addr: %u)\n", i,
               frame->function_name ? frame->function_name : "<anonymous>",
               frame->return_address);
    }
    printf("=============================\n");
}

void vm_print_variables(const st_vm_t *vm) {
    if (!vm) return;

    printf("=== Global Variables ===\n");
    for (uint32_t i = 0; i < vm->global_var_count && i < 20; i++) {
        printf("G[%u] ", i);
        vm_print_value(&vm->global_vars[i]);
        printf("\n");
    }

    printf("=== Local Variables ===\n");
    for (uint32_t i = 0; i < vm->local_var_count && i < 10; i++) {
        printf("L[%u] ", i);
        vm_print_value(&vm->local_vars[i]);
        printf("\n");
    }
    printf("=======================\n");
}

void vm_print_sync_status(const st_vm_t *vm) {
    if (!vm) return;

    printf("=== Sync Status ===\n");
    printf("Sync enabled: %s\n", vm->sync_enabled ? "YES" : "NO");
    printf("Sync mode: %s\n",
           (vm->sync_mode == VM_SYNC_PRIMARY) ? "PRIMARY" :
           (vm->sync_mode == VM_SYNC_SECONDARY) ? "SECONDARY" : "NONE");
    printf("Sync variables: %u\n", vm->sync_var_count);

    if (vm->sync_mgr) {
        printf("Peer alive: %s\n", ms_sync_is_peer_alive(vm->sync_mgr) ? "YES" : "NO");
        printf("Connected: %s\n", ms_sync_is_connected(vm->sync_mgr) ? "YES" : "NO");
    }
    printf("==================\n");
}

const vm_statistics_t* vm_get_statistics(const st_vm_t *vm) {
    return vm ? &vm->statistics : NULL;
}

void vm_print_statistics(const st_vm_t *vm) {
    if (!vm) return;

    printf("=== VM Statistics ===\n");
    printf("Instructions executed: %lu\n", vm->statistics.instructions_executed);
    printf("Function calls: %lu\n", vm->statistics.function_calls);
    printf("Library calls: %lu\n", vm->statistics.library_calls);
    printf("Sync operations: %lu\n", vm->statistics.sync_operations);
    printf("Runtime errors: %lu\n", vm->statistics.runtime_errors);
    printf("Execution time: %lu ms\n", vm->statistics.execution_time_ms);
    printf("====================\n");
}

size_t vm_get_memory_usage(const st_vm_t *vm) {
    if (!vm) return 0;

    size_t usage = sizeof(st_vm_t);
    usage += vm->instr_count * sizeof(bytecode_instr_t);
    /* 其他内存使用统计 */

    return usage;
}

void vm_print_memory_info(const st_vm_t *vm) {
    if (!vm) return;

    printf("=== Memory Usage ===\n");
    printf("VM instance: %zu bytes\n", sizeof(st_vm_t));
    printf("Instructions: %zu bytes\n", vm->instr_count * sizeof(bytecode_instr_t));
    printf("Global vars: %zu bytes\n", vm->global_var_count * sizeof(vm_value_t));
    printf("Stack usage: %d/%d\n", vm->operand_stack.top + 1, MAX_STACK_SIZE);
    printf("Call stack: %d/%d\n", vm->call_stack_top + 1, MAX_CALL_FRAMES);
    printf("Total usage: %zu bytes\n", vm_get_memory_usage(vm));
    printf("===================\n");
}

/* ========== 工具函数（补充完整） ========== */

const char* vm_value_type_to_string(value_type_t type) {
    switch (type) {
    case VAL_BOOL: return "BOOL";
    case VAL_INT: return "INT";
    case VAL_DINT: return "DINT";
    case VAL_REAL: return "REAL";
    case VAL_STRING: return "STRING";
    case VAL_TIME: return "TIME";
    case VAL_UNDEFINED: return "UNDEFINED";
    default: return "UNKNOWN";
    }
}

bool vm_is_numeric_type(value_type_t type) {
    return type == VAL_INT || type == VAL_DINT || type == VAL_REAL;
}

bool vm_is_compatible_types(value_type_t a, value_type_t b) {
    if (a == b) return true;

    /* 数值类型之间可以转换 */
    return vm_is_numeric_type(a) && vm_is_numeric_type(b);
}

const char* vm_opcode_to_string(opcode_t opcode) {
    return bytecode_get_opcode_name(opcode);
}

bool vm_is_jump_instruction(opcode_t opcode) {
    return opcode == OP_JMP || opcode == OP_JMP_TRUE || opcode == OP_JMP_FALSE;
}

bool vm_is_call_instruction(opcode_t opcode) {
    return opcode == OP_CALL || opcode == OP_CALL_BUILTIN || opcode == OP_CALL_LIBRARY;
}

bool vm_is_valid_global_var_index(const st_vm_t *vm, uint32_t index) {
    return vm && index < vm->global_var_count;
}

bool vm_is_valid_local_var_index(const st_vm_t *vm, uint32_t index) {
    return vm && index < vm->local_var_count;
}

/* 打印值的辅助函数 */
void vm_print_value(const vm_value_t *value) {
    if (!value) {
        printf("(null)");
        return;
    }

    switch (value->type) {
    case VAL_BOOL:
        printf("BOOL: %s", value->data.bool_val ? "TRUE" : "FALSE");
        break;
    case VAL_INT:
        printf("INT: %d", value->data.int_val);
        break;
    case VAL_DINT:
        printf("DINT: %ld", value->data.dint_val);
        break;
    case VAL_REAL:
        printf("REAL: %.6f", value->data.real_val);
        break;
    case VAL_STRING:
        printf("STRING: \"%s\"", value->data.string_val ? value->data.string_val : "(null)");
        break;
    case VAL_TIME:
        printf("TIME: %lu ms", value->data.time_val);
        break;
    case VAL_UNDEFINED:
        printf("UNDEFINED");
        break;
    default:
        printf("UNKNOWN_TYPE");
        break;
    }
}

/* 执行直到断点 */
int vm_execute_until_breakpoint(st_vm_t *vm) {
    if (!vm) return -1;

    bool old_step_mode = vm->debug_info.step_mode;
    vm->debug_info.step_mode = false;

    vm->state = VM_STATE_RUNNING;
    int result = vm_execute_main_loop(vm);

    vm->debug_info.step_mode = old_step_mode;

    return result;
}

/* 暂停执行 */
void vm_pause(st_vm_t *vm) {
    if (vm && vm->state == VM_STATE_RUNNING) {
        vm->state = VM_STATE_PAUSED;
    }
}

/* 恢复执行 */
void vm_resume(st_vm_t *vm) {
    if (vm && vm->state == VM_STATE_PAUSED) {
        vm->state = VM_STATE_RUNNING;
    }
}

/* 停止执行 */
void vm_stop(st_vm_t *vm) {
    if (vm) {
        vm->state = VM_STATE_STOPPED;
    }
}

/* 设置和获取程序计数器 */
void vm_set_pc(st_vm_t *vm, uint32_t pc) {
    if (vm && vm_is_valid_pc(vm, pc)) {
        vm->pc = pc;
    }
}

uint32_t vm_get_pc(const st_vm_t *vm) {
    return vm ? vm->pc : 0;
}

void vm_jump_to(st_vm_t *vm, uint32_t address) {
    if (vm && vm_is_valid_pc(vm, address)) {
        vm->pc = address;
    }
}

/* 栈操作（补充） */
vm_value_t* vm_stack_peek(st_vm_t *vm, int32_t offset) {
    if (!vm) return NULL;

    int32_t index = vm->operand_stack.top - offset;
    if (index < 0 || index > vm->operand_stack.top) {
        return NULL;
    }

    return &vm->operand_stack.data[index];
}
