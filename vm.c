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
    vm->variables = NULL;
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
            instr->int_operand = va_arg(args, int);
            break;
        case VM_LOAD_REAL:
            instr->real_operand = va_arg(args, double);
            break;
        case VM_LOAD_STRING:
        case VM_LOAD_VAR:
        case VM_STORE_VAR:
            instr->str_operand = strdup(va_arg(args, char *));
            break;
        case VM_POP:
        case VM_DUP:
            /* 无操作数指令 */
            break;
        default:
            /* 其他指令无操作数 */
            break;
    }
    
    va_end(args);
}

/* 编译AST到虚拟机字节码 */
void vm_compile_ast(VMState *vm, ASTNode *ast) {
    if (!ast) return;
    
    switch (ast->type) {
        case NODE_PROGRAM: {
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
            
        default:
            vm_set_error(vm, "未知AST节点类型");
            break;
    }
}

/* 执行虚拟机 */
int vm_run(VMState *vm) {
    vm->running = 1;
    vm->pc = 0;
    vm->sp = 0;
    
    while (vm->running && vm->pc < vm->code_size) {
        VMInstruction *instr = &vm->code[vm->pc];
        
        switch (instr->opcode) {
            case VM_LOAD_INT: {
                VMValue val = {TYPE_INT, {.int_val = instr->int_operand}};
                vm_push(vm, val);
                break;
            }
            
            case VM_LOAD_REAL: {
                VMValue val = {TYPE_REAL, {.real_val = instr->real_operand}};
                vm_push(vm, val);
                break;
            }
            
            case VM_LOAD_BOOL: {
                VMValue val = {TYPE_BOOL, {.bool_val = instr->int_operand}};
                vm_push(vm, val);
                break;
            }
            
            case VM_LOAD_STRING: {
                VMValue val = {TYPE_STRING, {.str_val = strdup(instr->str_operand)}};
                vm_push(vm, val);
                break;
            }
            
            case VM_LOAD_VAR: {
                VMValue val = vm_get_variable(vm, instr->str_operand);
                vm_push(vm, val);
                break;
            }
            
            case VM_STORE_VAR: {
                VMValue val = vm_pop(vm);
                vm_set_variable(vm, instr->str_operand, val);
                break;
            }
            
            case VM_ADD: {
                VMValue b = vm_pop(vm);
                VMValue a = vm_pop(vm);
                VMValue result = {TYPE_INT, {.int_val = a.value.int_val + b.value.int_val}};
                vm_push(vm, result);
                break;
            }
            
            case VM_SUB: {
                VMValue b = vm_pop(vm);
                VMValue a = vm_pop(vm);
                VMValue result = {TYPE_INT, {.int_val = a.value.int_val - b.value.int_val}};
                vm_push(vm, result);
                break;
            }
            
            case VM_MUL: {
                VMValue b = vm_pop(vm);
                VMValue a = vm_pop(vm);
                VMValue result = {TYPE_INT, {.int_val = a.value.int_val * b.value.int_val}};
                vm_push(vm, result);
                break;
            }

            case VM_DIV: {
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
                } else {
                    VMValue result = {TYPE_INT, {.int_val = a.value.int_val / b.value.int_val}};
                    vm_push(vm, result);
                }
                break;
            }
            
            case VM_GE: {
                VMValue b = vm_pop(vm);
                VMValue a = vm_pop(vm);
                VMValue result = {TYPE_BOOL, {.bool_val = a.value.int_val >= b.value.int_val}};
                vm_push(vm, result);
                break;
            }

            case VM_LE: {
                VMValue b = vm_pop(vm);
                VMValue a = vm_pop(vm);
                VMValue result = {TYPE_BOOL, {.bool_val = a.value.int_val <= b.value.int_val}};
                vm_push(vm, result);
                break;
            }

            case VM_GT: {
                VMValue b = vm_pop(vm);
                VMValue a = vm_pop(vm);
                VMValue result = {TYPE_BOOL, {.bool_val = a.value.int_val > b.value.int_val}};
                vm_push(vm, result);
                break;
            }

            case VM_LT: {
                VMValue b = vm_pop(vm);
                VMValue a = vm_pop(vm);
                VMValue result = {TYPE_BOOL, {.bool_val = a.value.int_val < b.value.int_val}};
                vm_push(vm, result);
                break;
            }

            case VM_EQ: {
                VMValue b = vm_pop(vm);
                VMValue a = vm_pop(vm);
                VMValue result = {TYPE_BOOL, {.bool_val = a.value.int_val == b.value.int_val}};
                vm_push(vm, result);
                break;
            }
            
            case VM_NE: {
                VMValue b = vm_pop(vm);
                VMValue a = vm_pop(vm);
                VMValue result = {TYPE_BOOL, {.bool_val = a.value.int_val != b.value.int_val}};
                vm_push(vm, result);
                break;
            }

            case VM_JZ: {
                VMValue val = vm_pop(vm);
                if (!val.value.bool_val) {
                    vm->pc = instr->int_operand - 1;  // -1因为循环末尾会+1
                }
                break;
            }
            
            case VM_JMP: {
                vm->pc = instr->int_operand - 1;  // -1因为循环末尾会+1
                break;
            }

            case VM_POP: {
                // VM_POP 只弹出栈顶，无需处理 val
                VMValue val = vm_pop(vm);
                val = val;  // 避免未使用变量警告
                break;
            }

            case VM_DUP: {
                if (vm->sp <= 0) {
                    vm_set_error(vm, "栈下溢");
                    return -1;
                }
                VMValue val = vm->stack[vm->sp - 1];
                vm_push(vm, val);
                break;
            }

            case VM_HALT:
                vm->running = 0;
                break;

            default:
                vm_set_error(vm, "未知指令");
                return -1;
        }
        
        vm->pc++;
    }
    
    return 0;
}

/* 设置变量值 */
void vm_set_variable(VMState *vm, const char *name, VMValue value) {
    VMVariable *var = vm->variables;
    
    /* 查找现有变量 */
    while (var) {
        if (strcmp(var->name, name) == 0) {
            var->value = value;
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
}

/* 获取变量值 */
VMValue vm_get_variable(VMState *vm, const char *name) {
    VMVariable *var = vm->variables;
    
    while (var) {
        if (strcmp(var->name, name) == 0) {
            return var->value;
        }
        var = var->next;
    }
    
    /* 变量不存在，返回默认值 */
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

/* 打印变量状态（调试用） */
void vm_print_variables(VMState *vm) {
    printf("=== 变量状态 ===\n");
    VMVariable *var = vm->variables;
    while (var) {
        printf("%s = ", var->name);
        switch (var->value.type) {
            case TYPE_INT:
                printf("%d", var->value.value.int_val);
                break;
            case TYPE_REAL:
                printf("%f", var->value.value.real_val);
                break;
            case TYPE_BOOL:
                printf("%s", var->value.value.bool_val ? "TRUE" : "FALSE");
                break;
            case TYPE_STRING:
                printf("\"%s\"", var->value.value.str_val ? var->value.value.str_val : "");
                break;
            default:
                printf("未知类型");
                break;
        }
        printf("\n");
        var = var->next;
    }
    printf("================\n");
}

/* 打印字节码（调试用） */
void vm_print_code(VMState *vm) {
    printf("=== 字节码 ===\n");
    for (int i = 0; i < vm->code_size; i++) {
        VMInstruction *instr = &vm->code[i];
        printf("%d: ", i);
        
        switch (instr->opcode) {
            case VM_LOAD_INT:
                printf("LOAD_INT %d", instr->int_operand);
                break;
            case VM_LOAD_REAL:
                printf("LOAD_REAL %f", instr->real_operand);
                break;
            case VM_LOAD_BOOL:
                printf("LOAD_BOOL %d", instr->int_operand);
                break;
            case VM_LOAD_VAR:
                printf("LOAD_VAR %s", instr->str_operand);
                break;
            case VM_STORE_VAR:
                printf("STORE_VAR %s", instr->str_operand);
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
            default:
                printf("UNKNOWN");
                break;
        }
        printf("\n");
    }
    printf("==============\n");
}