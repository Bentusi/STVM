/**
 * @file vm.c
 * @brief 虚拟机实现 - 字节码解释执行引擎
 * 
 * 功能特性：
 * 1. 栈式计算模型
 * 2. 调用栈管理
 * 3. 全局/局部变量访问
 * 4. 完整的28指令集支持
 * 5. 类型检查和错误处理
 */

#include "vm.h"
#include "mmgr.h"
#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// 默认栈大小
#define VM_STACK_DEFAULT_SIZE 4096
#define VM_CALLSTACK_DEFAULT_SIZE 256

// 辅助宏
#define PUSH(v) do { \
    if (vm->sp + 1 >= vm->stack_size) { \
        vm->error_code = ERR_STACK_OVERFLOW; \
        snprintf(vm->error_msg, sizeof(vm->error_msg), "Stack overflow at PC=%u", vm->pc); \
        return ERR_STACK_OVERFLOW; \
    } \
    vm->stack[++vm->sp] = (v); \
} while(0)

#define POP() (vm->sp < 0 ? (Value){.type=TYPE_VOID} : vm->stack[vm->sp--])

#define PEEK() (vm->sp < 0 ? (Value){.type=TYPE_VOID} : vm->stack[vm->sp])

#define CHECK_STACK(n) do { \
    if (vm->sp < (n) - 1) { \
        vm->error_code = ERR_STACK_UNDERFLOW; \
        snprintf(vm->error_msg, sizeof(vm->error_msg), "Stack underflow at PC=%u", vm->pc); \
        return ERR_STACK_UNDERFLOW; \
    } \
} while(0)

// 前向声明
static ExternalFunction* find_external_function(VM* vm, const char* name);

/**
 * @brief 创建虚拟机实例
 */
VM* vm_create(BytecodeModule* module) {
    if (!module) return NULL;
    
    VM* vm = (VM*)mmgr_alloc(sizeof(VM));
    if (!vm) return NULL;
    
    memset(vm, 0, sizeof(VM));
    vm->module = module;
    
    // 初始化操作数栈
    vm->stack_size = VM_STACK_DEFAULT_SIZE;
    vm->stack = (Value*)mmgr_alloc(sizeof(Value) * vm->stack_size);
    if (!vm->stack) {
        mmgr_free(vm);
        return NULL;
    }
    vm->sp = -1;
    
    // 初始化调用栈
    vm->call_stack_size = VM_CALLSTACK_DEFAULT_SIZE;
    vm->call_stack = (CallFrame*)mmgr_alloc(sizeof(CallFrame) * vm->call_stack_size);
    if (!vm->call_stack) {
        mmgr_free(vm->stack);
        mmgr_free(vm);
        return NULL;
    }
    vm->call_sp = -1;
    
    // 初始化全局变量
    vm->global_count = module->global_count;
    if (vm->global_count > 0) {
        vm->globals = (Value*)mmgr_alloc(sizeof(Value) * vm->global_count);
        if (!vm->globals) {
            mmgr_free(vm->call_stack);
            mmgr_free(vm->stack);
            mmgr_free(vm);
            return NULL;
        }
        memset(vm->globals, 0, sizeof(Value) * vm->global_count);
    }
    
    vm->pc = 0;
    vm->running = false;
    vm->error_code = OK;
    vm->instruction_count = 0;
    
    // 初始化外部函数表
    vm->external_functions = NULL;
    vm->external_function_count = 0;
    
    // 注册内置函数
    if (!builtins_register_all(vm)) {
        fprintf(stderr, "警告：内置函数注册失败\n");
    }
    
    return vm;
}

/**
 * @brief 释放虚拟机实例
 */
void vm_free(VM* vm) {
    if (!vm) return;
    
    if (vm->stack) mmgr_free(vm->stack);
    if (vm->call_stack) mmgr_free(vm->call_stack);
    if (vm->globals) mmgr_free(vm->globals);
    if (vm->jump_table) mmgr_free(vm->jump_table);
    
    // 释放外部函数表
    if (vm->external_functions) {
        for (int32_t i = 0; i < vm->external_function_count; i++) {
            if (vm->external_functions[i].name) {
                mmgr_free(vm->external_functions[i].name);
            }
        }
        mmgr_free(vm->external_functions);
    }
    
    mmgr_free(vm);
}

/**
 * @brief 重置虚拟机状态
 */
void vm_reset(VM* vm) {
    if (!vm) return;
    
    vm->sp = -1;
    vm->call_sp = -1;
    vm->pc = 0;
    vm->running = false;
    vm->error_code = OK;
    vm->error_msg[0] = '\0';
    vm->instruction_count = 0;
    
    if (vm->globals && vm->global_count > 0) {
        memset(vm->globals, 0, sizeof(Value) * vm->global_count);
    }
}

/**
 * @brief 获取局部变量或全局变量
 */
static inline Value* vm_get_variable(VM* vm, uint16_t index, bool is_global) {
    if (is_global) {
        if (index >= vm->global_count) {
            vm->error_code = ERR_OUT_OF_BOUNDS;
            return NULL;
        }
        return &vm->globals[index];
    } else {
        // 局部变量：基于当前调用帧的栈帧
        if (vm->call_sp < 0) {
            vm->error_code = ERR_RUNTIME;
            return NULL;
        }
        CallFrame* frame = &vm->call_stack[vm->call_sp];
        int32_t var_index = frame->base_pointer + index;
        if (var_index < 0 || var_index >= vm->sp + 1) {
            vm->error_code = ERR_OUT_OF_BOUNDS;
            return NULL;
        }
        return &vm->stack[var_index];
    }
}

/**
 * @brief 执行算术运算
 */
static ErrorCode vm_execute_arithmetic(VM* vm, Opcode op) {
    CHECK_STACK(2);
    
    Value b = POP();
    Value a = POP();
    Value result;
    result.type = a.type;
    
    // 类型转换：如果一个是REAL，结果就是REAL
    if (a.type == TYPE_REAL || b.type == TYPE_REAL) {
        double left = (a.type == TYPE_REAL) ? a.real_val : (double)a.int_val;
        double right = (b.type == TYPE_REAL) ? b.real_val : (double)b.int_val;
        result.type = TYPE_REAL;
        
        switch (op) {
            case OP_ADD: result.real_val = left + right; break;
            case OP_SUB: result.real_val = left - right; break;
            case OP_MUL: result.real_val = left * right; break;
            case OP_DIV:
                if (right == 0.0) {
                    vm->error_code = ERR_DIV_ZERO;
                    snprintf(vm->error_msg, sizeof(vm->error_msg), "Division by zero at PC=%u", vm->pc);
                    return ERR_DIV_ZERO;
                }
                result.real_val = left / right;
                break;
            default:
                return ERR_INVALID_INSTRUCTION;
        }
    } else {
        // 整数运算
        switch (op) {
            case OP_ADD: result.int_val = a.int_val + b.int_val; break;
            case OP_SUB: result.int_val = a.int_val - b.int_val; break;
            case OP_MUL: result.int_val = a.int_val * b.int_val; break;
            case OP_DIV:
                if (b.int_val == 0) {
                    vm->error_code = ERR_DIV_ZERO;
                    snprintf(vm->error_msg, sizeof(vm->error_msg), "Division by zero at PC=%u", vm->pc);
                    return ERR_DIV_ZERO;
                }
                result.int_val = a.int_val / b.int_val;
                break;
            case OP_MOD:
                if (b.int_val == 0) {
                    vm->error_code = ERR_DIV_ZERO;
                    return ERR_DIV_ZERO;
                }
                result.int_val = a.int_val % b.int_val;
                break;
            default:
                return ERR_INVALID_INSTRUCTION;
        }
    }
    
    PUSH(result);
    return OK;
}

/**
 * @brief 执行比较运算
 */
static ErrorCode vm_execute_comparison(VM* vm, Opcode op) {
    CHECK_STACK(2);
    
    Value b = POP();
    Value a = POP();
    Value result;
    result.type = TYPE_BOOL;
    
    // 类型转换
    if (a.type == TYPE_REAL || b.type == TYPE_REAL) {
        double left = (a.type == TYPE_REAL) ? a.real_val : (double)a.int_val;
        double right = (b.type == TYPE_REAL) ? b.real_val : (double)b.int_val;
        
        switch (op) {
            case OP_EQ: result.bool_val = (left == right); break;
            case OP_NE: result.bool_val = (left != right); break;
            case OP_LT: result.bool_val = (left < right); break;
            case OP_LE: result.bool_val = (left <= right); break;
            case OP_GT: result.bool_val = (left > right); break;
            case OP_GE: result.bool_val = (left >= right); break;
            default: return ERR_INVALID_INSTRUCTION;
        }
    } else {
        switch (op) {
            case OP_EQ: result.bool_val = (a.int_val == b.int_val); break;
            case OP_NE: result.bool_val = (a.int_val != b.int_val); break;
            case OP_LT: result.bool_val = (a.int_val < b.int_val); break;
            case OP_LE: result.bool_val = (a.int_val <= b.int_val); break;
            case OP_GT: result.bool_val = (a.int_val > b.int_val); break;
            case OP_GE: result.bool_val = (a.int_val >= b.int_val); break;
            default: return ERR_INVALID_INSTRUCTION;
        }
    }
    
    PUSH(result);
    return OK;
}

/**
 * @brief 执行逻辑运算
 */
static ErrorCode vm_execute_logical(VM* vm, Opcode op) {
    Value result;
    result.type = TYPE_BOOL;
    
    if (op == OP_NOT) {
        CHECK_STACK(1);
        Value a = POP();
        result.bool_val = !a.bool_val;
    } else {
        CHECK_STACK(2);
        Value b = POP();
        Value a = POP();
        
        switch (op) {
            case OP_AND: result.bool_val = a.bool_val && b.bool_val; break;
            case OP_OR:  result.bool_val = a.bool_val || b.bool_val; break;
            default: return ERR_INVALID_INSTRUCTION;
        }
    }
    
    PUSH(result);
    return OK;
}

/**
 * @brief 主解释循环
 */
ErrorCode vm_run(VM* vm) {
    // 从指令0开始执行（包括全局变量初始化）
    return vm_run_from(vm, 0);
}

/**
 * @brief 单步执行一条指令（用于调试）
 */
ErrorCode vm_step(VM* vm) {
    if (!vm || !vm->module) return ERR_RUNTIME;
    
    // 检查是否还有指令可执行
    if (!vm->running || vm->pc >= vm->module->instruction_count) {
        vm->running = false;
        return OK;
    }
    
    Instruction* code = vm->module->instructions;
    Instruction instr = code[vm->pc++];
    vm->instruction_count++;
    
    // 执行单条指令
    switch (instr.opcode) {
        // === 栈操作 ===
        case OP_PUSH: {
            if (instr.operand >= vm->module->const_count) {
                vm->error_code = ERR_OUT_OF_BOUNDS;
                snprintf(vm->error_msg, sizeof(vm->error_msg), 
                        "Invalid constant index %u at PC=%u", instr.operand, vm->pc-1);
                return ERR_OUT_OF_BOUNDS;
            }
            Constant* c = &vm->module->constants[instr.operand];
            Value v;
            switch (c->type) {
                case CONST_INT: 
                    v.type = TYPE_INT;
                    v.int_val = c->int_val; 
                    break;
                case CONST_REAL: 
                    v.type = TYPE_REAL;
                    v.real_val = c->real_val; 
                    break;
                case CONST_BOOL: 
                    v.type = TYPE_BOOL;
                    v.bool_val = c->bool_val; 
                    break;
                case CONST_STRING: 
                    v.type = TYPE_STRING;
                    v.string_val = c->string_val; 
                    break;
                default:
                    v.type = TYPE_VOID;
                    break;
            }
            PUSH(v);
            break;
        }
        
        case OP_POP:
            CHECK_STACK(1);
            POP();
            break;
        
        case OP_DUP:
            CHECK_STACK(1);
            {
                Value top = PEEK();
                PUSH(top);
            }
            break;
        
        case OP_LOAD: {
            bool is_global = (instr.flags & FLAG_GLOBAL) != 0;
            Value* var = vm_get_variable(vm, instr.operand, is_global);
            if (!var) return vm->error_code;
            PUSH(*var);
            break;
        }
        
        case OP_STORE: {
            CHECK_STACK(1);
            bool is_global = (instr.flags & FLAG_GLOBAL) != 0;
            Value* var = vm_get_variable(vm, instr.operand, is_global);
            if (!var) return vm->error_code;
            *var = POP();
            break;
        }
        
        // === 算术运算 ===
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_MOD: {
            ErrorCode err = vm_execute_arithmetic(vm, instr.opcode);
            if (err != OK) return err;
            break;
        }
        
        case OP_NEG: {
            CHECK_STACK(1);
            Value a = POP();
            if (a.type == TYPE_REAL) {
                a.real_val = -a.real_val;
            } else {
                a.int_val = -a.int_val;
            }
            PUSH(a);
            break;
        }
        
        // === 比较运算 ===
        case OP_EQ:
        case OP_NE:
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE: {
            ErrorCode err = vm_execute_comparison(vm, instr.opcode);
            if (err != OK) return err;
            break;
        }
        
        // === 逻辑运算 ===
        case OP_AND:
        case OP_OR:
        case OP_NOT: {
            ErrorCode err = vm_execute_logical(vm, instr.opcode);
            if (err != OK) return err;
            break;
        }
        
        // === 控制流 ===
        case OP_JMP:
            vm->pc = instr.operand;
            break;
        
        case OP_JZ: {
            CHECK_STACK(1);
            Value cond = POP();
            // 跳转如果为假：检查bool_val或int_val
            bool is_false = (cond.type == TYPE_BOOL) ? !cond.bool_val : (cond.int_val == 0);
            if (is_false) {
                vm->pc = instr.operand;
            }
            break;
        }
        
        case OP_JNZ: {
            CHECK_STACK(1);
            Value cond = POP();
            // 跳转如果为真：检查bool_val或int_val
            bool is_true = (cond.type == TYPE_BOOL) ? cond.bool_val : (cond.int_val != 0);
            if (is_true) {
                vm->pc = instr.operand;
            }
            break;
        }
        
        case OP_CALL: {
            // 函数调用
            if (vm->call_sp + 1 >= vm->call_stack_size) {
                vm->error_code = ERR_CALL_STACK_OVERFLOW;
                return ERR_CALL_STACK_OVERFLOW;
            }
            
            // operand是函数索引（不是地址）
            uint32_t func_idx = instr.operand;
            if (func_idx >= vm->module->function_count) {
                vm->error_code = ERR_OUT_OF_BOUNDS;
                snprintf(vm->error_msg, sizeof(vm->error_msg),
                        "Invalid function index %u at PC=%u", func_idx, vm->pc-1);
                return ERR_OUT_OF_BOUNDS;
            }
            
            FunctionEntry* func = &vm->module->functions[func_idx];
            
            CallFrame frame;
            frame.return_address = vm->pc;
            frame.base_pointer = vm->sp - func->param_count + 1;
            frame.local_count = func->local_count;
            frame.function = func;
            
            vm->call_stack[++vm->call_sp] = frame;
            
            // 为局部变量（参数之外的）在栈上分配空间
            // local_count 包括参数和局部变量，参数已经在栈上了
            int32_t locals_only = func->local_count - func->param_count;
            for (int32_t i = 0; i < locals_only; i++) {
                Value v = {.type = TYPE_VOID};
                PUSH(v);
            }
            
            vm->pc = func->address;
            break;
        }
        
        case OP_RET: {
            if (vm->call_sp < 0) {
                // 主程序返回，停止执行
                vm->running = false;
                break;
            }
            
            CallFrame frame = vm->call_stack[vm->call_sp--];
            vm->pc = frame.return_address;
            
            // 清理局部变量（保留返回值）
            Value return_val = (vm->sp >= 0) ? POP() : (Value){.type=TYPE_VOID};
            vm->sp = frame.base_pointer - 1;
            
            if (return_val.type != TYPE_VOID) {
                PUSH(return_val);
            }
            break;
        }
        
        // === 其他 ===
        case OP_HALT:
            vm->running = false;
            break;
        
        case OP_CALL_EXT: {
            // 外部函数调用
            uint32_t func_idx = instr.operand;
            int32_t argc = instr.flags;
            
            if (func_idx >= vm->module->function_count) {
                vm->error_code = ERR_OUT_OF_BOUNDS;
                snprintf(vm->error_msg, sizeof(vm->error_msg), 
                        "Invalid function index %u at PC=%u", func_idx, vm->pc-1);
                return ERR_OUT_OF_BOUNDS;
            }
            
            const char* func_name = vm->module->functions[func_idx].name;
            ExternalFunction* ext_func = find_external_function(vm, func_name);
            
            if (!ext_func) {
                vm->error_code = ERR_RUNTIME;
                snprintf(vm->error_msg, sizeof(vm->error_msg), 
                        "External function '%s' not registered at PC=%u", func_name, vm->pc-1);
                return ERR_RUNTIME;
            }
            
            // 检查参数个数
            if (ext_func->param_count >= 0 && argc != ext_func->param_count) {
                vm->error_code = ERR_RUNTIME;
                snprintf(vm->error_msg, sizeof(vm->error_msg), 
                        "Function '%s' expects %d args, got %d at PC=%u",
                        func_name, ext_func->param_count, argc, vm->pc-1);
                return ERR_RUNTIME;
            }
            
            CHECK_STACK(argc);
            
            // 调用外部函数
            Value result = ext_func->callback(vm, argc);
            
            // 弹出参数
            for (int32_t i = 0; i < argc; i++) {
                POP();
            }
            
            // 压入返回值
            if (result.type != TYPE_VOID) {
                PUSH(result);
            }
            
            break;
        }
        
        case OP_NOP:
            // 空操作
            break;
        
        case OP_LOAD_INDEXED: {
            // 从栈顶弹出索引，使用 operand 作为基地址，压入数组元素
            if (vm->sp < 1) {
                vm->error_code = ERR_STACK_UNDERFLOW;
                snprintf(vm->error_msg, sizeof(vm->error_msg), "Stack underflow in LOAD_INDEXED");
                return ERR_STACK_UNDERFLOW;
            }
            
            Value index_val = vm->stack[--vm->sp];
            if (index_val.type != TYPE_INT) {
                vm->error_code = ERR_TYPE;
                snprintf(vm->error_msg, sizeof(vm->error_msg), "Array index must be INT");
                return ERR_TYPE;
            }
            
            int32_t index = index_val.int_val;
            int32_t base_offset = instr.operand;
            int32_t actual_offset = base_offset + index;
            
            // 根据标志位确定是全局还是局部变量
            if (instr.flags & FLAG_GLOBAL) {
                if (actual_offset >= vm->global_count) {
                    vm->error_code = ERR_OUT_OF_BOUNDS;
                    snprintf(vm->error_msg, sizeof(vm->error_msg), 
                            "Global array index out of bounds: %d", actual_offset);
                    return ERR_OUT_OF_BOUNDS;
                }
                vm->stack[vm->sp++] = vm->globals[actual_offset];
            } else {
                // 获取当前调用栈帧的基指针
                int32_t bp = (vm->call_sp >= 0) ? vm->call_stack[vm->call_sp].base_pointer : 0;
                int32_t local_addr = bp + actual_offset;
                if (local_addr < 0 || local_addr >= vm->sp) {
                    vm->error_code = ERR_OUT_OF_BOUNDS;
                    snprintf(vm->error_msg, sizeof(vm->error_msg), 
                            "Local array index out of bounds: %d", local_addr);
                    return ERR_OUT_OF_BOUNDS;
                }
                vm->stack[vm->sp++] = vm->stack[local_addr];
            }
            break;
        }
        
        case OP_STORE_INDEXED: {
            // 从栈顶弹出值和索引，使用 operand 作为基地址，存储到数组元素
            if (vm->sp < 2) {
                vm->error_code = ERR_STACK_UNDERFLOW;
                snprintf(vm->error_msg, sizeof(vm->error_msg), "Stack underflow in STORE_INDEXED");
                return ERR_STACK_UNDERFLOW;
            }
            
            Value index_val = vm->stack[--vm->sp];
            Value value = vm->stack[--vm->sp];
            
            if (index_val.type != TYPE_INT) {
                vm->error_code = ERR_TYPE;
                snprintf(vm->error_msg, sizeof(vm->error_msg), "Array index must be INT");
                return ERR_TYPE;
            }
            
            int32_t index = index_val.int_val;
            int32_t base_offset = instr.operand;
            int32_t actual_offset = base_offset + index;
            
            // 根据标志位确定是全局还是局部变量
            if (instr.flags & FLAG_GLOBAL) {
                if (actual_offset >= vm->global_count) {
                    vm->error_code = ERR_OUT_OF_BOUNDS;
                    snprintf(vm->error_msg, sizeof(vm->error_msg), 
                            "Global array index out of bounds: %d", actual_offset);
                    return ERR_OUT_OF_BOUNDS;
                }
                vm->globals[actual_offset] = value;
            } else {
                // 获取当前调用栈帧的基指针
                int32_t bp = (vm->call_sp >= 0) ? vm->call_stack[vm->call_sp].base_pointer : 0;
                int32_t local_addr = bp + actual_offset;
                if (local_addr < 0 || local_addr >= vm->sp) {
                    vm->error_code = ERR_OUT_OF_BOUNDS;
                    snprintf(vm->error_msg, sizeof(vm->error_msg), 
                            "Local array index out of bounds: %d", local_addr);
                    return ERR_OUT_OF_BOUNDS;
                }
                vm->stack[local_addr] = value;
            }
            break;
        }
        
        default:
            vm->error_code = ERR_INVALID_INSTRUCTION;
            snprintf(vm->error_msg, sizeof(vm->error_msg), 
                    "Invalid opcode %u at PC=%u", instr.opcode, vm->pc-1);
            return ERR_INVALID_INSTRUCTION;
    }
    
    return OK;
}

/**
 * @brief 从指定入口点执行
 */
ErrorCode vm_run_from(VM* vm, uint32_t entry_point) {
    if (!vm || !vm->module) return ERR_RUNTIME;
    
    vm->pc = entry_point;
    vm->running = true;
    vm->error_code = OK;
    
    Instruction* code = vm->module->instructions;
    uint32_t code_size = vm->module->instruction_count;
    
    // 主解释循环
    while (vm->running && vm->pc < code_size) {
        Instruction instr = code[vm->pc++];
        vm->instruction_count++;
        
        switch (instr.opcode) {
            // === 栈操作 ===
            case OP_PUSH: {
                if (instr.operand >= vm->module->const_count) {
                    vm->error_code = ERR_OUT_OF_BOUNDS;
                    snprintf(vm->error_msg, sizeof(vm->error_msg), 
                            "Invalid constant index %u at PC=%u", instr.operand, vm->pc-1);
                    return ERR_OUT_OF_BOUNDS;
                }
                Constant* c = &vm->module->constants[instr.operand];
                Value v;
                // 正确映射ConstantType到DataType
                switch (c->type) {
                    case CONST_INT: 
                        v.type = TYPE_INT;
                        v.int_val = c->int_val; 
                        break;
                    case CONST_REAL: 
                        v.type = TYPE_REAL;
                        v.real_val = c->real_val; 
                        break;
                    case CONST_BOOL: 
                        v.type = TYPE_BOOL;
                        v.bool_val = c->bool_val; 
                        break;
                    case CONST_STRING: 
                        v.type = TYPE_STRING;
                        v.string_val = c->string_val; 
                        break;
                    default:
                        v.type = TYPE_VOID;
                        break;
                }
                PUSH(v);
                break;
            }
            
            case OP_POP:
                CHECK_STACK(1);
                POP();
                break;
            
            case OP_DUP:
                CHECK_STACK(1);
                {
                    Value top = PEEK();
                    PUSH(top);
                }
                break;
            
            case OP_LOAD: {
                bool is_global = (instr.flags & FLAG_GLOBAL) != 0;
                Value* var = vm_get_variable(vm, instr.operand, is_global);
                if (!var) return vm->error_code;
                PUSH(*var);
                break;
            }
            
            case OP_STORE: {
                CHECK_STACK(1);
                bool is_global = (instr.flags & FLAG_GLOBAL) != 0;
                Value* var = vm_get_variable(vm, instr.operand, is_global);
                if (!var) return vm->error_code;
                *var = POP();
                break;
            }
            
            // === 算术运算 ===
            case OP_ADD:
            case OP_SUB:
            case OP_MUL:
            case OP_DIV:
            case OP_MOD: {
                ErrorCode err = vm_execute_arithmetic(vm, instr.opcode);
                if (err != OK) return err;
                break;
            }
            
            case OP_NEG: {
                CHECK_STACK(1);
                Value a = POP();
                if (a.type == TYPE_REAL) {
                    a.real_val = -a.real_val;
                } else {
                    a.int_val = -a.int_val;
                }
                PUSH(a);
                break;
            }
            
            // === 比较运算 ===
            case OP_EQ:
            case OP_NE:
            case OP_LT:
            case OP_LE:
            case OP_GT:
            case OP_GE: {
                ErrorCode err = vm_execute_comparison(vm, instr.opcode);
                if (err != OK) return err;
                break;
            }
            
            // === 逻辑运算 ===
            case OP_AND:
            case OP_OR:
            case OP_NOT: {
                ErrorCode err = vm_execute_logical(vm, instr.opcode);
                if (err != OK) return err;
                break;
            }
            
            // === 控制流 ===
            case OP_JMP:
                vm->pc = instr.operand;
                break;
            
            case OP_JZ: {
                CHECK_STACK(1);
                Value cond = POP();
                // 跳转如果为假：检查bool_val或int_val
                bool is_false = (cond.type == TYPE_BOOL) ? !cond.bool_val : (cond.int_val == 0);
                if (is_false) {
                    vm->pc = instr.operand;
                }
                break;
            }
            
            case OP_JNZ: {
                CHECK_STACK(1);
                Value cond = POP();
                // 跳转如果为真：检查bool_val或int_val
                bool is_true = (cond.type == TYPE_BOOL) ? cond.bool_val : (cond.int_val != 0);
                if (is_true) {
                    vm->pc = instr.operand;
                }
                break;
            }
            
            case OP_CALL: {
                // 函数调用
                if (vm->call_sp + 1 >= vm->call_stack_size) {
                    vm->error_code = ERR_CALL_STACK_OVERFLOW;
                    return ERR_CALL_STACK_OVERFLOW;
                }
                
                // operand是函数索引（不是地址）
                uint32_t func_idx = instr.operand;
                if (func_idx >= vm->module->function_count) {
                    vm->error_code = ERR_OUT_OF_BOUNDS;
                    snprintf(vm->error_msg, sizeof(vm->error_msg),
                            "Invalid function index %u at PC=%u", func_idx, vm->pc-1);
                    return ERR_OUT_OF_BOUNDS;
                }
                
                FunctionEntry* func = &vm->module->functions[func_idx];
                
                CallFrame frame;
                frame.return_address = vm->pc;
                frame.base_pointer = vm->sp - func->param_count + 1;
                frame.local_count = func->local_count;
                frame.function = func;
                
                vm->call_stack[++vm->call_sp] = frame;
                
                // 为局部变量（参数之外的）在栈上分配空间
                int32_t locals_only = func->local_count - func->param_count;
                for (int32_t i = 0; i < locals_only; i++) {
                    Value v = {.type = TYPE_VOID};
                    PUSH(v);
                }
                
                vm->pc = func->address;
                break;
            }
            
            case OP_RET: {
                if (vm->call_sp < 0) {
                    // 主程序返回，停止执行
                    vm->running = false;
                    break;
                }
                
                CallFrame frame = vm->call_stack[vm->call_sp--];
                vm->pc = frame.return_address;
                
                // 清理局部变量（保留返回值）
                Value return_val = (vm->sp >= 0) ? POP() : (Value){.type=TYPE_VOID};
                vm->sp = frame.base_pointer - 1;
                
                if (return_val.type != TYPE_VOID) {
                    PUSH(return_val);
                }
                break;
            }
            
            // === 其他 ===
            case OP_HALT:
                vm->running = false;
                break;
            
            case OP_CALL_EXT: {
                // 外部函数调用
                // operand是函数索引（用于查找），flags包含参数个数
                uint32_t func_idx = instr.operand;
                int32_t argc = instr.flags;
                
                // 通过函数表查找函数名
                if (func_idx >= vm->module->function_count) {
                    vm->error_code = ERR_OUT_OF_BOUNDS;
                    snprintf(vm->error_msg, sizeof(vm->error_msg), 
                            "Invalid function index %u at PC=%u", func_idx, vm->pc-1);
                    return ERR_OUT_OF_BOUNDS;
                }
                
                const char* func_name = vm->module->functions[func_idx].name;
                ExternalFunction* ext_func = find_external_function(vm, func_name);
                
                if (!ext_func) {
                    vm->error_code = ERR_RUNTIME;
                    snprintf(vm->error_msg, sizeof(vm->error_msg), 
                            "External function '%s' not registered at PC=%u", func_name, vm->pc-1);
                    return ERR_RUNTIME;
                }
                
                // 检查参数个数
                if (ext_func->param_count >= 0 && argc != ext_func->param_count) {
                    vm->error_code = ERR_RUNTIME;
                    snprintf(vm->error_msg, sizeof(vm->error_msg), 
                            "Function '%s' expects %d args, got %d at PC=%u",
                            func_name, ext_func->param_count, argc, vm->pc-1);
                    return ERR_RUNTIME;
                }
                
                // 检查栈上是否有足够的参数
                CHECK_STACK(argc);
                
                // 调用外部函数
                Value result = ext_func->callback(vm, argc);
                
                // 弹出参数
                for (int32_t i = 0; i < argc; i++) {
                    POP();
                }
                
                // 压入返回值
                if (result.type != TYPE_VOID) {
                    PUSH(result);
                }
                
                break;
            }
            
            case OP_NOP:
                // 空操作
                break;
            
            case OP_LOAD_INDEXED: {
                // 从栈顶弹出索引，使用 operand 作为基地址，压入数组元素
                if (vm->sp < 0) {
                    vm->error_code = ERR_STACK_UNDERFLOW;
                    snprintf(vm->error_msg, sizeof(vm->error_msg), "Stack underflow in LOAD_INDEXED");
                    return ERR_STACK_UNDERFLOW;
                }
                
                Value index_val = POP();
                if (index_val.type != TYPE_INT) {
                    vm->error_code = ERR_TYPE;
                    snprintf(vm->error_msg, sizeof(vm->error_msg), "Array index must be INT");
                    return ERR_TYPE;
                }
                
                int32_t index = index_val.int_val;
                int32_t base_offset = instr.operand;
                int32_t actual_offset = base_offset + index;
                
                // 根据标志位确定是全局还是局部变量
                Value elem_val;
                elem_val.type = TYPE_INT;  // 目前只支持 INT 数组
                
                if (instr.flags & FLAG_GLOBAL) {
                    if (actual_offset >= vm->global_count) {
                        vm->error_code = ERR_OUT_OF_BOUNDS;
                        snprintf(vm->error_msg, sizeof(vm->error_msg), 
                                "Global array index out of bounds: %d", actual_offset);
                        return ERR_OUT_OF_BOUNDS;
                    }
                    elem_val.int_val = vm->globals[actual_offset].int_val;
                } else {
                    // 获取当前调用栈帧的基指针
                    int32_t bp = (vm->call_sp >= 0) ? vm->call_stack[vm->call_sp].base_pointer : 0;
                    int32_t local_addr = bp + actual_offset;
                    if (local_addr < 0 || local_addr > vm->sp) {
                        vm->error_code = ERR_OUT_OF_BOUNDS;
                        snprintf(vm->error_msg, sizeof(vm->error_msg), 
                                "Local array index out of bounds: %d", local_addr);
                        return ERR_OUT_OF_BOUNDS;
                    }
                    elem_val.int_val = vm->stack[local_addr].int_val;
                }
                
                PUSH(elem_val);
                break;
            }
            
            case OP_STORE_INDEXED: {
                // 从栈顶弹出值和索引，使用 operand 作为基地址，存储到数组元素
                if (vm->sp < 1) {
                    vm->error_code = ERR_STACK_UNDERFLOW;
                    snprintf(vm->error_msg, sizeof(vm->error_msg), "Stack underflow in STORE_INDEXED");
                    return ERR_STACK_UNDERFLOW;
                }
                
                Value index_val = POP();
                Value value_val = POP();
                
                if (index_val.type != TYPE_INT) {
                    vm->error_code = ERR_TYPE;
                    snprintf(vm->error_msg, sizeof(vm->error_msg), "Array index must be INT");
                    return ERR_TYPE;
                }
                
                if (value_val.type != TYPE_INT) {
                    vm->error_code = ERR_TYPE;
                    snprintf(vm->error_msg, sizeof(vm->error_msg), "Array element must be INT");
                    return ERR_TYPE;
                }
                
                int32_t index = index_val.int_val;
                int32_t base_offset = instr.operand;
                int32_t actual_offset = base_offset + index;
                
                // 根据标志位确定是全局还是局部变量
                if (instr.flags & FLAG_GLOBAL) {
                    if (actual_offset >= vm->global_count) {
                        vm->error_code = ERR_OUT_OF_BOUNDS;
                        snprintf(vm->error_msg, sizeof(vm->error_msg), 
                                "Global array index out of bounds: %d", actual_offset);
                        return ERR_OUT_OF_BOUNDS;
                    }
                    vm->globals[actual_offset] = value_val;
                } else {
                    // 获取当前调用栈帧的基指针
                    int32_t bp = (vm->call_sp >= 0) ? vm->call_stack[vm->call_sp].base_pointer : 0;
                    int32_t local_addr = bp + actual_offset;
                    if (local_addr < 0 || local_addr > vm->sp) {
                        vm->error_code = ERR_OUT_OF_BOUNDS;
                        snprintf(vm->error_msg, sizeof(vm->error_msg), 
                                "Local array index out of bounds: %d", local_addr);
                        return ERR_OUT_OF_BOUNDS;
                    }
                    vm->stack[local_addr] = value_val;
                }
                
                break;
            }
            
            default:
                vm->error_code = ERR_INVALID_INSTRUCTION;
                snprintf(vm->error_msg, sizeof(vm->error_msg), 
                        "Invalid opcode %u at PC=%u", instr.opcode, vm->pc-1);
                return ERR_INVALID_INSTRUCTION;
        }
    }
    
    return vm->error_code;
}

/**
 * @brief 获取执行结果
 */
ErrorCode vm_get_result(VM* vm, Value* result) {
    if (!vm || !result) return ERR_RUNTIME;
    
    if (vm->sp >= 0) {
        *result = vm->stack[vm->sp];
        return OK;
    }
    
    result->type = TYPE_VOID;
    return OK;
}

/**
 * @brief 打印虚拟机状态（调试用）
 */
void vm_dump_state(VM* vm) {
    if (!vm) return;
    
    printf("=== VM State ===\n");
    printf("PC: %u\n", vm->pc);
    printf("SP: %d\n", vm->sp);
    printf("Call SP: %d\n", vm->call_sp);
    printf("Instructions executed: %llu\n", (unsigned long long)vm->instruction_count);
    printf("Error: %s\n", vm->error_msg[0] ? vm->error_msg : "None");
    
    printf("\n--- Stack (top %d) ---\n", vm->sp >= 0 ? vm->sp + 1 : 0);
    for (int32_t i = vm->sp; i >= 0 && i >= vm->sp - 10; i--) {
        printf("[%d] ", i);
        value_print(&vm->stack[i]);
        printf("\n");
    }
    
    printf("\n--- Globals ---\n");
    for (int32_t i = 0; i < vm->global_count; i++) {
        printf("[%d] ", i);
        value_print(&vm->globals[i]);
        printf("\n");
    }
    printf("================\n");
}

/**
 * @brief 打印调用栈（调试用）
 */
void vm_dump_call_stack(VM* vm) {
    if (!vm) return;
    
    printf("=== Call Stack ===\n");
    for (int32_t i = vm->call_sp; i >= 0; i--) {
        CallFrame* frame = &vm->call_stack[i];
        printf("[%d] Function: %s, Return: %u, BP: %d, Locals: %d\n",
               i,
               frame->function ? frame->function->name : "<unknown>",
               frame->return_address,
               frame->base_pointer,
               frame->local_count);
    }
    printf("==================\n");
}

/**
 * @brief 注册外部函数
 */
bool vm_register_external_function(VM* vm, const char* name, 
                                    ExternalFunctionCallback callback,
                                    int32_t param_count) {
    if (!vm || !name || !callback) return false;
    
    // 扩展外部函数数组
    ExternalFunction* new_funcs = (ExternalFunction*)mmgr_realloc(
        vm->external_functions,
        sizeof(ExternalFunction) * (vm->external_function_count + 1)
    );
    if (!new_funcs) return false;
    
    vm->external_functions = new_funcs;
    
    // 添加新函数
    int32_t idx = vm->external_function_count++;
    vm->external_functions[idx].name = mmgr_strdup(name);
    vm->external_functions[idx].callback = callback;
    vm->external_functions[idx].param_count = param_count;
    
    return true;
}

/**
 * @brief 查找外部函数
 */
static ExternalFunction* find_external_function(VM* vm, const char* name) {
    if (!vm || !name) return NULL;
    
    for (int32_t i = 0; i < vm->external_function_count; i++) {
        if (strcmp(vm->external_functions[i].name, name) == 0) {
            return &vm->external_functions[i];
        }
    }
    return NULL;
}

/**
 * @brief 获取外部函数参数（从栈上）
 */
Value vm_get_arg(VM* vm, int32_t index) {
    if (!vm || index < 0) {
        Value v = {.type = TYPE_VOID};
        return v;
    }
    
    // 参数在栈上，从sp向下索引
    int32_t arg_pos = vm->sp - index;
    if (arg_pos < 0 || arg_pos > vm->sp) {
        Value v = {.type = TYPE_VOID};
        return v;
    }
    
    return vm->stack[arg_pos];
}
