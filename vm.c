/*
 * IEC61131 虚拟机 (VM) 实现
 * 平台无关的字节码执行引擎实现
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "vm.h"

#define MAX_STACK_SIZE 1024
#define MAX_CODE_SIZE 4096

/* 创建虚拟机实例 */
VMState *vm_create(void) {
    VMState *vm = (VMState *)malloc(sizeof(VMState));
    if (!vm) return NULL;
    
    vm->code = (VMInstruction *)malloc(MAX_CODE_SIZE * sizeof(VMInstruction));
    vm->stack = (VMValue *)malloc(MAX_STACK_SIZE * sizeof(VMValue));
    
    vm->code_size = 0;
    vm->pc = 0;
    vm->stack_size = MAX_STACK_SIZE;
    vm->sp = 0;
    vm->call_stack = NULL;  // 确保调用栈初始化为NULL
    vm->variables = NULL;
    vm->functions = NULL;
    vm->running = 0;
    vm->error_msg = NULL;
    
    return vm;
}

/* 销毁虚拟机实例 */
void vm_destroy(VMState *vm) {
    if (!vm) return;
    
    free(vm->code);
    free(vm->stack);
    
    /* 释放变量存储 */
    VMVariable *var = vm->variables;
    while (var) {
        VMVariable *next = var->next;
        free(var->name);
        if (var->value.type == TYPE_STRING && var->value.value.str_val) {
            free(var->value.value.str_val);
        }
        free(var);
        var = next;
    }
    
    if (vm->error_msg) free(vm->error_msg);
    free(vm);
}

/* 生成虚拟机指令 */
void vm_emit(VMState *vm, VMOpcode opcode, ...) {
    if (vm->code_size >= MAX_CODE_SIZE) {
        vm_set_error(vm, "代码段溢出");
        return;
    }
    
    va_list args;
    va_start(args, opcode);
    
    VMInstruction *instr = &vm->code[vm->code_size++];
    instr->opcode = opcode;
    
    switch (opcode) {
        case VM_LOAD_INT:
        case VM_LOAD_BOOL:
        case VM_JMP:
        case VM_JZ:
        case VM_JNZ:
        case VM_PUSH_ARGS:  // 添加PUSH_ARGS处理
            instr->int_operand = va_arg(args, int);
            break;
        case VM_LOAD_REAL:
            instr->real_operand = va_arg(args, double);
            break;
        case VM_LOAD_STRING:
        case VM_LOAD_VAR:
        case VM_CALL:
        case VM_STORE_VAR:
            instr->str_operand = strdup(va_arg(args, char *));
            break;
        case VM_POP:
        case VM_DUP:
        case VM_ADD:
        case VM_SUB:
        case VM_MUL:
        case VM_DIV:
        case VM_NEG:
        case VM_EQ:
        case VM_NE:
        case VM_LT:
        case VM_LE:
        case VM_GT:
        case VM_GE:
        case VM_AND:
        case VM_OR:
        case VM_NOT:
        case VM_RET:
        case VM_HALT:
            /* 无操作数指令 */
            break;
        default:
            /* 其他指令无操作数 */
            break;
    }
    
    va_end(args);
}

/* 压入调用栈帧 */
void vm_push_frame(VMState *vm, int param_count) {
    CallFrame *frame = (CallFrame *)malloc(sizeof(CallFrame));
    frame->return_pc = vm->pc + 1;  // 保存返回地址
    frame->frame_pointer = vm->sp - param_count;  // 帧指针指向参数起始位置
    frame->local_count = param_count;
    frame->prev = vm->call_stack;
    vm->call_stack = frame;
    
    printf("  创建栈帧: 返回地址=%d, 帧指针=%d, 参数个数=%d\n", 
           frame->return_pc, frame->frame_pointer, param_count);
}

/* 弹出调用栈帧 */
void vm_pop_frame(VMState *vm) {
    if (!vm->call_stack) {
        vm_set_error(vm, "调用栈为空");
        return;
    }
    
    CallFrame *frame = vm->call_stack;
    vm->pc = frame->return_pc;  // 恢复返回地址
    
    printf("  销毁栈帧: 返回地址=%d\n", frame->return_pc);
    
    vm->call_stack = frame->prev;
    free(frame);
}

/* 编译函数调用 */
void vm_compile_function_call(VMState *vm, ASTNode *node) {
    if (node->type != NODE_FUNCTION_CALL) return;
    
    printf("编译函数调用: %s\n", node->identifier);
    
    // 统计参数个数
    int param_count = 0;
    VarDecl *arg = node->param_list;
    
    // 编译参数表达式（从左到右压栈）
    while (arg) {
        // 目前只支持参数是变量或字面量
        switch (arg->type)
        {
        case TYPE_IDENTIFIER:
            vm_emit(vm, VM_LOAD_VAR, arg->name);
            printf("  生成指令: LOAD_VAR %s\n", arg->name);
            break;
        case TYPE_INT:
            vm_emit(vm, VM_LOAD_INT, arg->value.int_val);
            printf("  生成指令: LOAD_INT %d\n", arg->value.int_val);
            break;
        case TYPE_REAL:
            vm_emit(vm, VM_LOAD_REAL, arg->value.real_val);
            printf("  生成指令: LOAD_REAL %f\n", arg->value.real_val);
            break;
        case TYPE_BOOL:
            vm_emit(vm, VM_LOAD_BOOL, arg->value.bool_val);
            printf("  生成指令: LOAD_BOOL %s\n", arg->value.bool_val ? "TRUE" : "FALSE");
            break;
        case TYPE_STRING:
            vm_emit(vm, VM_LOAD_STRING, arg->value.str_val);
            printf("  生成指令: LOAD_STRING \"%s\"\n", arg->value.str_val);
            break;
        default:
            break;
        }
        param_count++;
        arg = arg->next;
    }

    // 压入参数个数
    vm_emit(vm, VM_PUSH_ARGS, param_count);

    // 生成函数调用指令
    char func_label[256];
    snprintf(func_label, sizeof(func_label), "__func_%s__", node->identifier);
    vm_emit(vm, VM_CALL, func_label);

    printf("  生成调用指令: %s (参数个数: %d)\n", func_label, param_count);
}

/* 编译AST到虚拟机字节码 */
void vm_compile_ast(VMState *vm, ASTNode *ast) {
    if (!ast) return;
    
    switch (ast->type) {
        case NODE_COMPILATION_UNIT: {
            // 首先初始化全局变量
            printf("初始化全局变量...\n");
            vm_initialize_global_variables(vm);
            
            // 编译函数列表
            if (ast->left) {
                printf("编译函数列表...\n");
                vm_compile_function_list(vm, ast->left);
            }
            // 编译程序
            if (ast->right) {
                printf("编译程序...\n");
                vm_compile_ast(vm, ast->right);
            }
            break;
        }
        
        case NODE_PROGRAM: {
            // 确保程序开始时全局变量已初始化
            vm_compile_ast(vm, ast->statements);
            vm_emit(vm, VM_HALT);
            break;
        }
        
        case NODE_STATEMENT_LIST: {
            vm_compile_ast(vm, ast->left);
            if (ast->next) {
                vm_compile_ast(vm, ast->next);
            }
            break;
        }
        
        case NODE_ASSIGN: {
            vm_compile_ast(vm, ast->right);  // 计算右侧表达式
            vm_emit(vm, VM_STORE_VAR, ast->identifier);
            break;
        }

        case NODE_FUNCTION: {
            /* 函数定义处理 */
            int func_start = vm->code_size;
            
            // 统计参数个数
            int param_count = 0;
            VarDecl *param = ast->param_list;
            while (param) {
                param_count++;
                param = param->next;
            }
            
            printf("编译函数: %s (参数个数: %d, 地址: 0x%04x)\n", 
                   ast->identifier, param_count, func_start);
            
            // 编译函数体
            vm_compile_ast(vm, ast->statements);
            
            // 如果函数没有显式返回，添加默认返回
            vm_emit(vm, VM_LOAD_INT, 0);  // 默认返回值0
            vm_emit(vm, VM_RET);
            
            // 注册函数
            char func_label[256];
            snprintf(func_label, sizeof(func_label), "__func_%s__", ast->identifier);
            vm_set_function(vm, func_label, func_start);
            
            // 设置函数参数个数
            VMFunction *func = vm->functions;
            while (func && strcmp(func->name, func_label) != 0) {
                func = func->next;
            }
            if (func) {
                func->param_count = param_count;
                printf("  函数 %s 注册完成\n", func_label);
            }
            break;
        }
        
        case NODE_FUNCTION_CALL: {
            vm_compile_function_call(vm, ast);
            break;
        }
        
        case NODE_RETURN: {
            if (ast->left) {
                // 编译返回值表达式
                vm_compile_ast(vm, ast->left);
            } else {
                // 默认返回值0
                vm_emit(vm, VM_LOAD_INT, 0);
            }
            vm_emit(vm, VM_RET);
            break;
        }
        
        case NODE_IF: {
            vm_compile_ast(vm, ast->condition);
            int jz_addr = vm->code_size;
            vm_emit(vm, VM_JZ, 0);  // 占位符，稍后回填
            
            vm_compile_ast(vm, ast->statements);
            
            if (ast->else_statements) {
                int jmp_addr = vm->code_size;
                vm_emit(vm, VM_JMP, 0);  // 跳过else分支
                
                vm->code[jz_addr].int_operand = vm->code_size;  // 回填条件跳转地址
                vm_compile_ast(vm, ast->else_statements);
                vm->code[jmp_addr].int_operand = vm->code_size;  // 回填无条件跳转地址
            } else {
                vm->code[jz_addr].int_operand = vm->code_size;  // 回填条件跳转地址
            }
            break;
        }

        case NODE_FOR: {
            // FOR循环实现：初始化 -> 条件检查 -> 循环体 -> 增量 -> 跳转
            vm_compile_ast(vm, ast->left);  // 初始值
            vm_emit(vm, VM_STORE_VAR, ast->identifier);
            
            int loop_start = vm->code_size;
            vm_emit(vm, VM_LOAD_VAR, ast->identifier);
            vm_compile_ast(vm, ast->right);  // 结束值
            vm_emit(vm, VM_LE);
            
            int loop_exit = vm->code_size;
            vm_emit(vm, VM_JZ, 0);  // 条件为假时退出循环
            
            vm_compile_ast(vm, ast->statements);  // 循环体
            
            // 增量操作
            vm_emit(vm, VM_LOAD_VAR, ast->identifier);
            vm_emit(vm, VM_LOAD_INT, 1);
            vm_emit(vm, VM_ADD);
            vm_emit(vm, VM_STORE_VAR, ast->identifier);
            
            vm_emit(vm, VM_JMP, loop_start);  // 跳回循环开始
            vm->code[loop_exit].int_operand = vm->code_size;  // 回填退出地址
            break;
        }
        case NODE_WHILE: {
            int while_start = vm->code_size;
            vm_compile_ast(vm, ast->condition);
            
            int while_exit = vm->code_size;
            vm_emit(vm, VM_JZ, 0);
            
            vm_compile_ast(vm, ast->statements);
            vm_emit(vm, VM_JMP, while_start);
            vm->code[while_exit].int_operand = vm->code_size;
            break;
        }

        case NODE_CASE: {
            /* CASE语句编译：计算表达式值，然后与各个CASE项比较 */
            vm_compile_ast(vm, ast->left);  // 编译CASE表达式
            vm_emit(vm, VM_STORE_VAR, "__vm_case_temp__");  // 临时存储CASE值
            
            /* 收集所有需要回填的结束跳转地址 */
            int *case_end_jumps = NULL;
            int end_jump_count = 0;
            int max_jumps = 100;  // 假设最多100个CASE项
            case_end_jumps = (int *)malloc(max_jumps * sizeof(int));
            
            /* 处理CASE项列表 */
            ASTNode *case_item = ast->statements;
            while (case_item != NULL) {
                if (case_item->type == NODE_CASE_ITEM) {
                    /* 比较CASE值 */
                    vm_emit(vm, VM_LOAD_VAR, "__vm_case_temp__");
                    vm_compile_ast(vm, case_item->left);  // CASE项的值
                    vm_emit(vm, VM_EQ);  // 比较
                    
                    int no_match_jump = vm->code_size;
                    vm_emit(vm, VM_JZ, 0);  // 不匹配则跳到下一项
                    
                    /* 执行匹配的语句 */
                    vm_compile_ast(vm, case_item->statements);
                    
                    /* 记录需要跳转到结束的地址 */
                    if (end_jump_count < max_jumps) {
                        case_end_jumps[end_jump_count] = vm->code_size;
                        vm_emit(vm, VM_JMP, 0);  // 跳转到CASE结束，稍后回填
                        end_jump_count++;
                    }
                    
                    /* 回填不匹配的跳转地址到下一个比较 */
                    vm->code[no_match_jump].int_operand = vm->code_size;
                }
                case_item = case_item->next;
            }
            
            /* 回填所有结束跳转地址 */
            for (int i = 0; i < end_jump_count; i++) {
                vm->code[case_end_jumps[i]].int_operand = vm->code_size;
            }
            
            free(case_end_jumps);
            break;
        }
        
        case NODE_CASE_LIST:
        case NODE_CASE_ITEM:
            /* 这些节点在NODE_CASE中处理 */
            break;
            
        case NODE_BINARY_OP:
            vm_compile_ast(vm, ast->left);
            vm_compile_ast(vm, ast->right);
            
            switch (ast->op_type) {
                case OP_ADD: vm_emit(vm, VM_ADD); break;
                case OP_SUB: vm_emit(vm, VM_SUB); break;
                case OP_MUL: vm_emit(vm, VM_MUL); break;
                case OP_DIV: vm_emit(vm, VM_DIV); break;
                case OP_EQ:  vm_emit(vm, VM_EQ); break;
                case OP_NE:  vm_emit(vm, VM_NE); break;
                case OP_LT:  vm_emit(vm, VM_LT); break;
                case OP_LE:  vm_emit(vm, VM_LE); break;
                case OP_GT:  vm_emit(vm, VM_GT); break;
                case OP_GE:  vm_emit(vm, VM_GE); break;
                case OP_AND: vm_emit(vm, VM_AND); break;
                case OP_OR:  vm_emit(vm, VM_OR); break;
                default: vm_set_error(vm, "未知二元操作符"); break;
            }
            break;
            
        case NODE_UNARY_OP:
            vm_compile_ast(vm, ast->left);
            switch (ast->op_type) {
                case OP_NEG: vm_emit(vm, VM_NEG); break;
                case OP_NOT: vm_emit(vm, VM_NOT); break;
                default: vm_set_error(vm, "未知一元操作符"); break;
            }
            break;
            
        case NODE_IDENTIFIER:
            vm_emit(vm, VM_LOAD_VAR, ast->identifier);
            break;
            
        case NODE_INT_LITERAL:
            vm_emit(vm, VM_LOAD_INT, ast->value.int_val);
            break;
            
        case NODE_REAL_LITERAL:
            vm_emit(vm, VM_LOAD_REAL, ast->value.real_val);
            break;
            
        case NODE_BOOL_LITERAL:
            vm_emit(vm, VM_LOAD_BOOL, ast->value.bool_val);
            break;
            
        case NODE_STRING_LITERAL:
            vm_emit(vm, VM_LOAD_STRING, ast->value.str_val);
            break;
            
        case NODE_ARGUMENT_LIST:
            /* 参数列表在函数调用中处理 */
            break;
        default:
            vm_set_error(vm, "未知AST节点类型");
            break;
    }
}

/* 专门编译函数列表的函数 */
void vm_compile_function_list(VMState *vm, ASTNode *func_list) {
    if (!func_list) return;
    
    ASTNode *current = func_list;
    int func_count = 0;
    
    while (current) {
        if (current->type == NODE_FUNCTION) {
            func_count++;
            printf("处理第 %d 个函数: %s\n", func_count, current->identifier);
            vm_compile_ast(vm, current);
        }
        current = current->next;
    }
    
    printf("总共处理了 %d 个函数\n", func_count);
}

/* 初始化全局变量到VM */
void vm_initialize_global_variables(VMState *vm) {
    if (!vm) return;

    VarDecl *var = ast_get_global_var_table();
    while (var) {
        VMValue default_val;
        
        // 根据变量类型设置默认值
        switch (var->type) {
            case TYPE_INT:
                default_val.type = TYPE_INT;
                default_val.value.int_val = 0;
                break;
            case TYPE_REAL:
                default_val.type = TYPE_REAL;
                default_val.value.real_val = 0.0;
                break;
            case TYPE_BOOL:
                default_val.type = TYPE_BOOL;
                default_val.value.bool_val = 0;
                break;
            case TYPE_STRING:
                default_val.type = TYPE_STRING;
                default_val.value.str_val = strdup("");
                break;
            default:
                default_val.type = TYPE_INT;
                default_val.value.int_val = 0;
                break;
        }
        
        // 添加到VM变量列表
        vm_set_variable(vm, var->name, default_val);
        printf("  初始化全局变量: %s (类型: %d)\n", var->name, var->type);
        
        var = var->next;
    }
}

/* 重置虚拟机 */
void vm_reset(VMState *vm) {
    vm->pc = 0;
    vm->sp = 0;
    vm->running = 0;
    if (vm->error_msg) {
        free(vm->error_msg);
        vm->error_msg = NULL;
    }
}

/* 执行虚拟机 */
int vm_run(VMState *vm) {
    vm->running = 1;
    vm->pc = 0;
    vm->sp = 0;
    vm->call_stack = NULL;
    
    while (vm->running && vm->pc < vm->code_size) {
        VMInstruction *instr = &vm->code[vm->pc];
        
        printf("PC=%d: ", vm->pc);
        
        switch (instr->opcode) {
            case VM_LOAD_INT: {
                printf("LOAD_INT %d\n", instr->int_operand);
                VMValue val = {TYPE_INT, {.int_val = instr->int_operand}};
                vm_push(vm, val);
                break;
            }
            
            case VM_LOAD_REAL: {
                printf("LOAD_REAL %f\n", instr->real_operand);
                VMValue val = {TYPE_REAL, {.real_val = instr->real_operand}};
                vm_push(vm, val);
                break;
            }
            
            case VM_LOAD_BOOL: {
                printf("LOAD_BOOL %s\n", instr->bool_operand ? "TRUE" : "FALSE");
                VMValue val = {TYPE_BOOL, {.bool_val = instr->bool_operand}};
                vm_push(vm, val);
                break;
            }
            
            case VM_LOAD_STRING: {
                printf("LOAD_STRING \"%s\"\n", instr->str_operand);
                VMValue val = {TYPE_STRING, {.str_val = strdup(instr->str_operand)}};
                vm_push(vm, val);
                break;
            }
            
            case VM_LOAD_VAR: {
                printf("LOAD_VAR %s\n", instr->str_operand);
                VMValue val = vm_get_variable(vm, instr->str_operand);
                vm_push(vm, val);
                printf("  变量值: ");
                vm_print_value(val);
                printf("\n");
                break;
            }
            
            case VM_STORE_VAR: {
                printf("STORE_VAR %s\n", instr->str_operand);
                VMValue val = vm_pop(vm);
                vm_set_variable(vm, instr->str_operand, val);
                printf("  存储值: ");
                vm_print_value(val);
                printf("\n");
                break;
            }

            case VM_NEG: {
                printf("NEG\n");
                VMValue val = vm_pop(vm);
                if (val.type == TYPE_INT) {
                    val.value.int_val = -val.value.int_val;
                } else if (val.type == TYPE_REAL) {
                    val.value.real_val = -val.value.real_val;
                } else {
                    vm_set_error(vm, "NEG操作数类型错误");
                    return -1;
                }
                vm_push(vm, val);
                break;
            }
            
            case VM_ADD: {
                printf("ADD\n");
                VMValue b = vm_pop(vm);
                VMValue a = vm_pop(vm);
                if (a.type == TYPE_REAL || b.type == TYPE_REAL) {
                    double aval = (a.type == TYPE_REAL) ? a.value.real_val : (double)a.value.int_val;
                    double bval = (b.type == TYPE_REAL) ? b.value.real_val : (double)b.value.int_val;
                    VMValue result = {TYPE_REAL, {.real_val = aval + bval}};
                    vm_push(vm, result);
                    printf("  %f + %f = %f\n", aval, bval, result.value.real_val);
                } else {
                    VMValue result = {TYPE_INT, {.int_val = a.value.int_val + b.value.int_val}};
                    vm_push(vm, result);
                    printf("  %d + %d = %d\n", a.value.int_val, b.value.int_val, result.value.int_val);
                }
                break;
            }
            
            case VM_SUB: {
                printf("SUB\n");
                VMValue b = vm_pop(vm);
                VMValue a = vm_pop(vm);
                if (a.type == TYPE_REAL || b.type == TYPE_REAL) {
                    double aval = (a.type == TYPE_REAL) ? a.value.real_val : (double)a.value.int_val;
                    double bval = (b.type == TYPE_REAL) ? b.value.real_val : (double)b.value.int_val;
                    VMValue result = {TYPE_REAL, {.real_val = aval - bval}};
                    vm_push(vm, result);
                    printf("  %f - %f = %f\n", aval, bval, result.value.real_val);
                } else {
                    VMValue result = {TYPE_INT, {.int_val = a.value.int_val - b.value.int_val}};
                    vm_push(vm, result);
                    printf("  %d - %d = %d\n", a.value.int_val, b.value.int_val, result.value.int_val);
                }
                break;
            }
            
            case VM_MUL: {
                printf("MUL\n");
                VMValue b = vm_pop(vm);
                VMValue a = vm_pop(vm);
                if (a.type == TYPE_REAL || b.type == TYPE_REAL) {
                    double aval = (a.type == TYPE_REAL) ? a.value.real_val : (double)a.value.int_val;
                    double bval = (b.type == TYPE_REAL) ? b.value.real_val : (double)b.value.int_val;
                    VMValue result = {TYPE_REAL, {.real_val = aval * bval}};
                    vm_push(vm, result);
                    printf("  %f * %f = %f\n", aval, bval, result.value.real_val);
                } else {
                    VMValue result = {TYPE_INT, {.int_val = a.value.int_val * b.value.int_val}};
                    vm_push(vm, result);
                    printf("  %d * %d = %d\n", a.value.int_val, b.value.int_val, result.value.int_val);
                }
                break;
            }

            case VM_DIV: {
                printf("DIV\n");
                VMValue b = vm_pop(vm);
                VMValue a = vm_pop(vm);
                
                if (b.type == TYPE_INT && b.value.int_val == 0) {
                    vm_set_error(vm, "除以零错误");
                    return -1;
                }

                if (a.type == TYPE_REAL || b.type == TYPE_REAL) {
                    double aval = (a.type == TYPE_REAL) ? a.value.real_val : (double)a.value.int_val;
                    double bval = (b.type == TYPE_REAL) ? b.value.real_val : (double)b.value.int_val;
                    VMValue result = {TYPE_REAL, {.real_val = aval / bval}};
                    vm_push(vm, result);
                    printf("  %f / %f = %f\n", aval, bval, result.value.real_val);
                } else {
                    VMValue result = {TYPE_INT, {.int_val = a.value.int_val / b.value.int_val}};
                    vm_push(vm, result);
                    printf("  %d / %d = %d\n", a.value.int_val, b.value.int_val, result.value.int_val);
                }
                break;
            }
            
            case VM_GE: {
                printf("GE\n");
                VMValue b = vm_pop(vm);
                VMValue a = vm_pop(vm);
                VMValue result = {TYPE_BOOL, {.bool_val = a.value.int_val >= b.value.int_val}};
                vm_push(vm, result);
                printf("  %d >= %d = %s\n", a.value.int_val, b.value.int_val, 
                       result.value.bool_val ? "TRUE" : "FALSE");
                break;
            }

            case VM_LE: {
                printf("LE\n");
                VMValue b = vm_pop(vm);
                VMValue a = vm_pop(vm);
                VMValue result = {TYPE_BOOL, {.bool_val = a.value.int_val <= b.value.int_val}};
                vm_push(vm, result);
                printf("  %d <= %d = %s\n", a.value.int_val, b.value.int_val, 
                       result.value.bool_val ? "TRUE" : "FALSE");
                break;
            }

            case VM_GT: {
                printf("GT\n");
                VMValue b = vm_pop(vm);
                VMValue a = vm_pop(vm);
                VMValue result = {TYPE_BOOL, {.bool_val = a.value.int_val > b.value.int_val}};
                vm_push(vm, result);
                printf("  %d > %d = %s\n", a.value.int_val, b.value.int_val, 
                       result.value.bool_val ? "TRUE" : "FALSE");
                break;
            }

            case VM_LT: {
                printf("LT\n");
                VMValue b = vm_pop(vm);
                VMValue a = vm_pop(vm);
                VMValue result = {TYPE_BOOL, {.bool_val = a.value.int_val < b.value.int_val}};
                vm_push(vm, result);
                printf("  %d < %d = %s\n", a.value.int_val, b.value.int_val, 
                       result.value.bool_val ? "TRUE" : "FALSE");
                break;
            }

            case VM_EQ: {
                printf("EQ\n");
                VMValue b = vm_pop(vm);
                VMValue a = vm_pop(vm);
                VMValue result = {TYPE_BOOL, {.bool_val = a.value.int_val == b.value.int_val}};
                vm_push(vm, result);
                printf("  %d == %d = %s\n", a.value.int_val, b.value.int_val, 
                       result.value.bool_val ? "TRUE" : "FALSE");
                break;
            }
            
            case VM_NE: {
                printf("NE\n");
                VMValue b = vm_pop(vm);
                VMValue a = vm_pop(vm);
                VMValue result = {TYPE_BOOL, {.bool_val = a.value.int_val != b.value.int_val}};
                vm_push(vm, result);
                printf("  %d != %d = %s\n", a.value.int_val, b.value.int_val, 
                       result.value.bool_val ? "TRUE" : "FALSE");
                break;
            }

            case VM_JZ: {
                printf("JZ %d\n", instr->int_operand);
                VMValue val = vm_pop(vm);
                if (!val.value.bool_val) {
                    printf("  跳转到 PC=0x%04x\n", instr->int_operand);
                    vm->pc = instr->int_operand - 1;
                } else {
                    printf("  条件为真，继续执行\n");
                }
                break;
            }
            
            case VM_JMP: {
                printf("JMP %d\n", instr->int_operand);
                printf("  无条件跳转到 PC=%d\n", instr->int_operand);
                vm->pc = instr->int_operand - 1;  // -1因为循环末尾会+1
                break;
            }

            case VM_POP: {
                printf("POP\n");
                VMValue val = vm_pop(vm);
                printf("  弹出值: ");
                vm_print_value(val);
                printf("\n");
                break;
            }

            case VM_DUP: {
                printf("DUP\n");
                if (vm->sp <= 0) {
                    vm_set_error(vm, "栈下溢");
                    return -1;
                }
                VMValue val = vm->stack[vm->sp - 1];
                vm_push(vm, val);
                printf("  复制栈顶值: ");
                vm_print_value(val);
                printf("\n");
                break;
            }

            case VM_PUSH_ARGS: {
                printf("PUSH_ARGS %d\n", instr->int_operand);
                // 参数个数信息存储在指令中，实际压栈在CALL指令中处理
                break;
            }

            case VM_RET: {
                printf("RET\n");
                
                // 获取返回值（应该在栈顶）
                VMValue return_value = {TYPE_INT, {.int_val = 0}};
                if (vm->sp > 0) {
                    return_value = vm_pop(vm);
                    printf("  返回值: ");
                    vm_print_value(return_value);
                    printf("\n");
                }
                
                // 清理函数参数变量
                vm_cleanup_function_parameters(vm);
                
                // 清理参数（恢复栈指针到调用前状态）
                if (vm->call_stack) {
                    vm->sp = vm->call_stack->frame_pointer;
                    printf("  清理参数, 恢复栈指针到: %d\n", vm->sp);
                }
                
                // 弹出调用栈帧（这会恢复PC）
                if (vm->call_stack) {
                    vm_pop_frame(vm);
                    
                    // 将返回值压回栈顶
                    vm_push(vm, return_value);
                    printf("  返回值已压入栈顶\n");
                } else {
                    // 主程序返回，停止执行
                    printf("  主程序返回，停止执行\n");
                    vm->running = 0;
                }
                break;
            }

            case VM_CALL: {
                printf("CALL %s\n", instr->str_operand);
                
                // 查找函数
                VMFunction *func = vm->functions;
                while (func && strcmp(func->name, instr->str_operand) != 0) {
                    func = func->next;
                }
                
                if (!func) {
                    vm_set_error(vm, "调用未知函数");
                    return -1;
                }
                
                // 获取参数个数（从前一条PUSH_ARGS指令）
                int param_count = 0;
                if (vm->pc > 0 && vm->code[vm->pc - 1].opcode == VM_PUSH_ARGS) {
                    param_count = vm->code[vm->pc - 1].int_operand;
                }
                
                printf("  调用函数: %s, 参数个数: %d\n", func->name, param_count);
                
                // 验证参数个数
                if (param_count != func->param_count) {
                    printf("  警告: 参数个数不匹配 (期望: %d, 实际: %d)\n", 
                           func->param_count, param_count);
                }
                
                // 设置函数参数
                vm_setup_function_parameters(vm, func->name, param_count);
                
                // 创建调用栈帧
                vm_push_frame(vm, param_count);
                
                // 跳转到函数地址
                printf("  跳转到函数地址: 0x%04x\n", func->addr);
                vm->pc = func->addr - 1;  // -1因为循环末尾会+1
                break;
            }

            case VM_HALT: {
                printf("HALT\n");
                printf("  程序正常结束\n");
                vm->running = 0;
                break;
            }

            default:
                printf("UNKNOWN 0x%02x\n", instr->opcode);
                vm_set_error(vm, "未知指令");
                return -1;
        }
        
        vm->pc++;
    }
    
    return 0;
}

/* 设置函数参数 */
void vm_setup_function_parameters(VMState *vm, char *func_name, int param_count) {
    if (!vm || !func_name || param_count <= 0) return;
    
    // 修复函数名匹配：正确去掉"__func_"前缀并添加"__"后缀
    char clean_func_name[256];
    if (strncmp(func_name, "__func_", 7) == 0 && 
        strlen(func_name) > 9 && 
        strcmp(func_name + strlen(func_name) - 2, "__") == 0) {
        // 去掉前缀"__func_"和后缀"__"
        int len = strlen(func_name) - 9; // 7 + 2
        strncpy(clean_func_name, func_name + 7, len);
        clean_func_name[len] = '\0';
    } else {
        strcpy(clean_func_name, func_name);
    }
    
    // 查找函数定义获取参数名称
    ASTNode *func_node = find_global_function(clean_func_name);
    if (!func_node) {
        printf("  警告: 无法找到函数定义 %s (清理后: %s)\n", func_name, clean_func_name);
        return;
    }
    
    printf("  设置函数参数 (函数: %s):\n", clean_func_name);
    
    // 从栈中获取参数值并设置到参数变量
    VMValue *param_values = (VMValue *)malloc(param_count * sizeof(VMValue));
    
    // 逆序弹出参数（因为栈是后进先出）
    for (int i = param_count - 1; i >= 0; i--) {
        param_values[i] = vm_pop(vm);
    }
    
    // 设置参数变量，使用特殊前缀避免与全局变量冲突
    VarDecl *param = func_node->param_list;
    for (int i = 0; i < param_count && param; i++) {
        char param_var_name[512];
        snprintf(param_var_name, sizeof(param_var_name), "__param_%s_%s", clean_func_name, param->name);
        
        vm_set_variable(vm, param_var_name, param_values[i]);
        printf("    参数 %s (%s) = ", param->name, param_var_name);
        vm_print_value(param_values[i]);
        printf("\n");
        param = param->next;
    }
    
    free(param_values);
}

/* 清理函数参数变量 */
void vm_cleanup_function_parameters(VMState *vm) {
    if (!vm) return;
    
    printf("  清理函数参数变量\n");
    
    // 清理所有参数变量
    VMVariable **var_ptr = &vm->variables;
    while (*var_ptr) {
        if (strncmp((*var_ptr)->name, "__param_", 8) == 0) {
            VMVariable *to_delete = *var_ptr;
            *var_ptr = (*var_ptr)->next;
            
            printf("    清理参数变量: %s\n", to_delete->name);
            
            // 释放字符串值
            if (to_delete->value.type == TYPE_STRING && to_delete->value.value.str_val) {
                free(to_delete->value.value.str_val);
            }
            free(to_delete->name);
            free(to_delete);
        } else {
            var_ptr = &(*var_ptr)->next;
        }
    }
}

/* 修复函数设置逻辑 */
void vm_set_function(VMState *vm, const char *name, int addr) {
    // 首先检查函数是否已存在
    VMFunction *existing = vm->functions;
    while (existing) {
        if (strcmp(existing->name, name) == 0) {
            printf("  更新现有函数: %s, 地址: 0x%04x\n", name, addr);
            existing->addr = addr;
            return;
        }
        existing = existing->next;
    }

    /* 创建新函数 */
    VMFunction *func = (VMFunction *)malloc(sizeof(VMFunction));
    func->name = strdup(name);
    func->addr = addr;
    func->param_count = 0;  // 默认参数个数，后续设置
    func->next = vm->functions;
    vm->functions = func;
    
    printf("  注册新函数: %s, 地址: 0x%04x\n", name, addr);
}

/* 设置变量值 */
void vm_set_variable(VMState *vm, const char *name, VMValue value) {
    if (!vm || !name) return;
    
    VMVariable *var = vm->variables;
    
    /* 查找现有变量 */
    while (var) {
        if (strcmp(var->name, name) == 0) {
            // 释放旧的字符串值
            if (var->value.type == TYPE_STRING && var->value.value.str_val) {
                free(var->value.value.str_val);
            }
            var->value = value;
            printf("  更新变量: %s\n", name);
            return;
        }
        var = var->next;
    }
    
    /* 创建新变量 */
    var = (VMVariable *)malloc(sizeof(VMVariable));
    var->name = strdup(name);
    var->value = value;
    var->next = vm->variables;
    vm->variables = var;
    
    printf("  创建新变量: %s\n", name);
}

/* 获取变量值 */
VMValue vm_get_variable(VMState *vm, const char *name) {
    if (!vm || !name) {
        VMValue default_val = {TYPE_INT, {.int_val = 0}};
        return default_val;
    }
    
    // 首先查找当前函数的参数变量
    if (vm->call_stack) {
        // 构造参数变量名 - 需要知道当前函数名
        VMVariable *var = vm->variables;
        while (var) {
            // 检查是否是参数变量且匹配参数名
            if (strncmp(var->name, "__param_", 8) == 0) {
                // 从参数变量名中提取实际参数名
                char *last_underscore = strrchr(var->name, '_');
                if (last_underscore && strcmp(last_underscore + 1, name) == 0) {
                    printf("  找到参数变量: %s -> %s\n", name, var->name);
                    return var->value;
                }
            }
            var = var->next;
        }
    }
    
    // 然后查找普通变量
    VMVariable *var = vm->variables;
    while (var) {
        if (strcmp(var->name, name) == 0) {
            return var->value;
        }
        var = var->next;
    }
    
    /* 变量不存在，返回默认值并报告 */
    printf("  警告: 变量 '%s' 不存在，返回默认值0\n", name);
    VMValue default_val = {TYPE_INT, {.int_val = 0}};
    return default_val;
}

/* 压入栈 */
void vm_push(VMState *vm, VMValue value) {
    if (vm->sp >= vm->stack_size) {
        vm_set_error(vm, "栈溢出");
        return;
    }
    vm->stack[vm->sp++] = value;
}

/* 弹出栈 */
VMValue vm_pop(VMState *vm) {
    if (vm->sp <= 0) {
        vm_set_error(vm, "栈下溢");
        VMValue empty = {TYPE_INT, {.int_val = 0}};
        return empty;
    }
    return vm->stack[--vm->sp];
}

/* 设置错误信息 */
void vm_set_error(VMState *vm, const char *error) {
    if (vm->error_msg) free(vm->error_msg);
    vm->error_msg = strdup(error);
}

/* 获取错误信息 */
const char *vm_get_error(VMState *vm) {
    return vm->error_msg;
}

/* 打印函数列表状态（调试用） */
void vm_print_functions(VMState *vm) {
    printf("=== 函数列表 ===\n");
    VMFunction *func = vm->functions;
    int count = 0;
    
    while (func) {
        printf("0x%04x %s (参数: %d)\n", func->addr, func->name, func->param_count);
        func = func->next;
        count++;
    }
    
    if (count == 0) {
        printf("无函数定义\n");
    } else {
        printf("总共 %d 个函数\n", count);
    }
    printf("================\n");
}

/* 打印变量状态（调试用） */
void vm_print_variables(VMState *vm) {
    if (!vm) {
        printf("=== 变量状态 ===\n");
        printf("VM状态为空\n");
        printf("================\n");
        return;
    }
    
    printf("=== 变量状态 ===\n");
    
    if (!vm->variables) {
        printf("无变量定义\n");
        printf("================\n");
        return;
    }
    
    VMVariable *var = vm->variables;
    int index = 0;
    
    while (var) {
        index++;
        switch (var->value.type) {
            case TYPE_INT:
                printf("0x%04x %s = %d (INT)\n", index, var->name, var->value.value.int_val);
                break;
            case TYPE_REAL:
                printf("0x%04x %s = %f (REAL)\n", index, var->name, var->value.value.real_val);
                break;
            case TYPE_BOOL:
                printf("0x%04x %s = %s (BOOL)\n", index, var->name, 
                       var->value.value.bool_val ? "TRUE" : "FALSE");
                break;
            case TYPE_STRING:
                printf("0x%04x %s = \"%s\" (STRING)\n", index, var->name, 
                       var->value.value.str_val ? var->value.value.str_val : "");
                break;
            default:
                printf("0x%04x %s = <未知类型 %d>\n", index, var->name, var->value.type);
                break;
        }
        var = var->next;
    }
    
    printf("总共 %d 个变量\n", index);
    printf("================\n");
}

/* 打印字节码（调试用） */
void vm_print_code(VMState *vm) {
    printf("=== 字节码 ===\n");
    for (int i = 0; i < vm->code_size; i++) {
        VMInstruction *instr = &vm->code[i];
        printf("0x%04x: ", i);
        
        switch (instr->opcode) {
            case VM_LOAD_INT:
                printf("LOAD_INT %d", instr->int_operand);
                break;
            case VM_LOAD_REAL:
                printf("LOAD_REAL %f", instr->real_operand);
                break;
            case VM_LOAD_BOOL:
                printf("LOAD_BOOL %s", instr->bool_operand ? "TRUE" : "FALSE");
                break;
            case VM_LOAD_VAR:
                printf("LOAD_VAR %s", instr->str_operand);
                break;
            case VM_STORE_VAR:
                printf("STORE_VAR %s", instr->str_operand);
                break;
            case VM_PUSH_ARGS:
                printf("PUSH_ARGS %d", instr->int_operand);
                break;
            case VM_ADD:
                printf("ADD");
                break;
            case VM_SUB:
                printf("SUB");
                break;
            case VM_MUL:
                printf("MUL");
                break;
            case VM_NEG:
                printf("NEG");
                break;
            case VM_DIV:
                printf("DIV");
                break;
            case VM_EQ:
                printf("EQ");
                break;
            case VM_NE:
                printf("NE");
                break;
            case VM_LT:
                printf("LT");
                break;
            case VM_LE:
                printf("LE");
                break;
            case VM_GT:
                printf("GT");
                break;
            case VM_GE:
                printf("GE");
                break;
            case VM_JMP:
                printf("JMP %d", instr->int_operand);
                break;
            case VM_JZ:
                printf("JZ %d", instr->int_operand);
                break;
            case VM_HALT:
                printf("HALT");
                break;
            case VM_RET:
                printf("RET");
                break;
            case VM_CALL:
                printf("CALL %s", instr->str_operand);
                break;
            case VM_POP:
                printf("POP");
                break;
            case VM_DUP:
                printf("DUP");
                break;
            default:
                printf("UNKNOWN 0x%02x", instr->opcode);
                break;
        }
        printf("\n");
    }
    printf("总共 %d 条指令\n", vm->code_size);
    printf("==============\n");
}

/* 打印值的辅助函数 */
void vm_print_value(VMValue value) {
    switch (value.type) {
        case TYPE_INT:
            printf("%d", value.value.int_val);
            break;
        case TYPE_REAL:
            printf("%f", value.value.real_val);
            break;
        case TYPE_BOOL:
            printf("%s", value.value.bool_val ? "TRUE" : "FALSE");
            break;
        case TYPE_STRING:
            printf("\"%s\"", value.value.str_val ? value.value.str_val : "");
            break;
        default:
            printf("unknown");
            break;
    }
}