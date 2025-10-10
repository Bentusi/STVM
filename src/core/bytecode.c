/**
 * @file bytecode.c
 * @brief 字节码模块的实现
 */

#include "bytecode.h"
#include "mmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 初始容量
#define INITIAL_INSTRUCTION_CAPACITY 256
#define INITIAL_CONSTANT_CAPACITY 64
#define INITIAL_FUNCTION_CAPACITY 32

/**
 * @brief 获取操作码名称字符串
 */
const char* opcode_to_string(Opcode opcode) {
    static const char* opcode_names[] = {
        "PUSH", "POP", "DUP", "LOAD", "STORE",
        "ADD", "SUB", "MUL", "DIV", "MOD", "NEG",
        "EQ", "NE", "LT", "LE", "GT", "GE",
        "AND", "OR", "NOT",
        "JMP", "JZ", "JNZ", "CALL", "RET",
        "HALT", "CALL_EXT", "NOP"
    };
    
    if (opcode >= 0 && opcode < OP_COUNT) {
        return opcode_names[opcode];
    }
    return "UNKNOWN";
}

/**
 * @brief 创建字节码模块
 */
BytecodeModule* bytecode_module_create(void) {
    BytecodeModule* module = (BytecodeModule*)mmgr_calloc(sizeof(BytecodeModule));
    if (!module) return NULL;
    
    // 分配指令数组
    module->instruction_capacity = INITIAL_INSTRUCTION_CAPACITY;
    module->instructions = (Instruction*)mmgr_alloc(sizeof(Instruction) * module->instruction_capacity);
    if (!module->instructions) {
        mmgr_free(module);
        return NULL;
    }
    
    // 分配常量池
    module->const_capacity = INITIAL_CONSTANT_CAPACITY;
    module->constants = (Constant*)mmgr_alloc(sizeof(Constant) * module->const_capacity);
    if (!module->constants) {
        mmgr_free(module->instructions);
        mmgr_free(module);
        return NULL;
    }
    
    // 分配函数表
    module->function_capacity = INITIAL_FUNCTION_CAPACITY;
    module->functions = (FunctionEntry*)mmgr_alloc(sizeof(FunctionEntry) * module->function_capacity);
    if (!module->functions) {
        mmgr_free(module->constants);
        mmgr_free(module->instructions);
        mmgr_free(module);
        return NULL;
    }
    
    module->instruction_count = 0;
    module->const_count = 0;
    module->function_count = 0;
    module->global_count = 0;
    module->entry_point = 0;
    module->line_numbers = NULL;
    module->source_file = NULL;
    
    return module;
}

/**
 * @brief 释放字节码模块
 */
void bytecode_module_free(BytecodeModule* module) {
    if (!module) return;
    
    // 释放指令数组
    if (module->instructions) {
        mmgr_free(module->instructions);
    }
    
    // 释放常量池中的字符串
    for (uint32_t i = 0; i < module->const_count; i++) {
        if (module->constants[i].type == CONST_STRING && module->constants[i].string_val) {
            mmgr_free(module->constants[i].string_val);
        }
    }
    if (module->constants) {
        mmgr_free(module->constants);
    }
    
    // 释放函数表中的字符串
    for (uint32_t i = 0; i < module->function_count; i++) {
        if (module->functions[i].name) {
            mmgr_free(module->functions[i].name);
        }
    }
    if (module->functions) {
        mmgr_free(module->functions);
    }
    
    // 释放行号数组
    if (module->line_numbers) {
        mmgr_free(module->line_numbers);
    }
    
    // 释放源文件名
    if (module->source_file) {
        mmgr_free(module->source_file);
    }
    
    mmgr_free(module);
}

/**
 * @brief 扩展指令数组容量
 */
static bool expand_instructions(BytecodeModule* module) {
    uint32_t new_capacity = module->instruction_capacity * 2;
    Instruction* new_instructions = (Instruction*)mmgr_realloc(
        module->instructions, 
        sizeof(Instruction) * new_capacity
    );
    
    if (!new_instructions) return false;
    
    module->instructions = new_instructions;
    module->instruction_capacity = new_capacity;
    
    // 同时扩展行号数组（如果存在）
    if (module->line_numbers) {
        int* new_line_numbers = (int*)mmgr_realloc(
            module->line_numbers,
            sizeof(int) * new_capacity
        );
        if (new_line_numbers) {
            module->line_numbers = new_line_numbers;
        }
    }
    
    return true;
}

/**
 * @brief 添加指令
 */
uint32_t bytecode_add_instruction(BytecodeModule* module, Opcode opcode, uint8_t flags, uint16_t operand) {
    if (module->instruction_count >= module->instruction_capacity) {
        if (!expand_instructions(module)) {
            return (uint32_t)-1;
        }
    }
    
    uint32_t index = module->instruction_count++;
    module->instructions[index].opcode = opcode;
    module->instructions[index].flags = flags;
    module->instructions[index].operand = operand;
    
    return index;
}

/**
 * @brief 添加指令（带行号）
 */
uint32_t bytecode_add_instruction_with_line(BytecodeModule* module, Opcode opcode, 
                                             uint8_t flags, uint16_t operand, int line) {
    // 如果还没有行号数组，创建一个
    if (!module->line_numbers) {
        module->line_numbers = (int*)mmgr_calloc(sizeof(int) * module->instruction_capacity);
        if (!module->line_numbers) {
            return (uint32_t)-1;
        }
    }
    
    uint32_t index = bytecode_add_instruction(module, opcode, flags, operand);
    if (index != (uint32_t)-1) {
        module->line_numbers[index] = line;
    }
    
    return index;
}

/**
 * @brief 修改指令的操作数（用于回填跳转地址）
 */
void bytecode_patch_operand(BytecodeModule* module, uint32_t index, uint16_t operand) {
    if (index < module->instruction_count) {
        module->instructions[index].operand = operand;
    }
}

/**
 * @brief 扩展常量池容量
 */
static bool expand_constants(BytecodeModule* module) {
    uint32_t new_capacity = module->const_capacity * 2;
    Constant* new_constants = (Constant*)mmgr_realloc(
        module->constants,
        sizeof(Constant) * new_capacity
    );
    
    if (!new_constants) return false;
    
    module->constants = new_constants;
    module->const_capacity = new_capacity;
    return true;
}

/**
 * @brief 添加整数常量
 */
uint32_t bytecode_add_int_constant(BytecodeModule* module, int32_t value) {
    // 检查是否已存在相同的常量
    for (uint32_t i = 0; i < module->const_count; i++) {
        if (module->constants[i].type == CONST_INT && 
            module->constants[i].int_val == value) {
            return i;
        }
    }
    
    if (module->const_count >= module->const_capacity) {
        if (!expand_constants(module)) {
            return (uint32_t)-1;
        }
    }
    
    uint32_t index = module->const_count++;
    module->constants[index].type = CONST_INT;
    module->constants[index].int_val = value;
    
    return index;
}

/**
 * @brief 添加实数常量
 */
uint32_t bytecode_add_real_constant(BytecodeModule* module, double value) {
    // 检查是否已存在相同的常量
    for (uint32_t i = 0; i < module->const_count; i++) {
        if (module->constants[i].type == CONST_REAL && 
            module->constants[i].real_val == value) {
            return i;
        }
    }
    
    if (module->const_count >= module->const_capacity) {
        if (!expand_constants(module)) {
            return (uint32_t)-1;
        }
    }
    
    uint32_t index = module->const_count++;
    module->constants[index].type = CONST_REAL;
    module->constants[index].real_val = value;
    
    return index;
}

/**
 * @brief 添加布尔常量
 */
uint32_t bytecode_add_bool_constant(BytecodeModule* module, bool value) {
    // 检查是否已存在相同的常量
    for (uint32_t i = 0; i < module->const_count; i++) {
        if (module->constants[i].type == CONST_BOOL && 
            module->constants[i].bool_val == value) {
            return i;
        }
    }
    
    if (module->const_count >= module->const_capacity) {
        if (!expand_constants(module)) {
            return (uint32_t)-1;
        }
    }
    
    uint32_t index = module->const_count++;
    module->constants[index].type = CONST_BOOL;
    module->constants[index].bool_val = value;
    
    return index;
}

/**
 * @brief 添加字符串常量
 */
uint32_t bytecode_add_string_constant(BytecodeModule* module, const char* value) {
    if (!value) return (uint32_t)-1;
    
    // 检查是否已存在相同的常量
    for (uint32_t i = 0; i < module->const_count; i++) {
        if (module->constants[i].type == CONST_STRING && 
            strcmp(module->constants[i].string_val, value) == 0) {
            return i;
        }
    }
    
    if (module->const_count >= module->const_capacity) {
        if (!expand_constants(module)) {
            return (uint32_t)-1;
        }
    }
    
    uint32_t index = module->const_count++;
    module->constants[index].type = CONST_STRING;
    module->constants[index].string_val = mmgr_strdup(value);
    
    return index;
}

/**
 * @brief 扩展函数表容量
 */
static bool expand_functions(BytecodeModule* module) {
    uint32_t new_capacity = module->function_capacity * 2;
    FunctionEntry* new_functions = (FunctionEntry*)mmgr_realloc(
        module->functions,
        sizeof(FunctionEntry) * new_capacity
    );
    
    if (!new_functions) return false;
    
    module->functions = new_functions;
    module->function_capacity = new_capacity;
    return true;
}

/**
 * @brief 添加函数
 */
uint32_t bytecode_add_function(BytecodeModule* module, const char* name, uint32_t address,
                                int32_t param_count, int32_t local_count, DataType return_type) {
    if (!name) return (uint32_t)-1;
    
    if (module->function_count >= module->function_capacity) {
        if (!expand_functions(module)) {
            return (uint32_t)-1;
        }
    }
    
    uint32_t index = module->function_count++;
    module->functions[index].name = mmgr_strdup(name);
    module->functions[index].address = address;
    module->functions[index].param_count = param_count;
    module->functions[index].local_count = local_count;
    module->functions[index].return_type = return_type;
    
    return index;
}

/**
 * @brief 查找函数
 */
FunctionEntry* bytecode_find_function(BytecodeModule* module, const char* name) {
    if (!module || !name) return NULL;
    
    for (uint32_t i = 0; i < module->function_count; i++) {
        if (strcmp(module->functions[i].name, name) == 0) {
            return &module->functions[i];
        }
    }
    
    return NULL;
}

/**
 * @brief 获取当前指令位置
 */
uint32_t bytecode_current_position(BytecodeModule* module) {
    return module->instruction_count;
}

/**
 * @brief 反汇编指令
 */
void bytecode_disassemble_instruction(Instruction instr, char* buffer, size_t size) {
    if (!buffer || size == 0) return;
    
    const char* opname = opcode_to_string(instr.opcode);
    
    // 根据操作码类型格式化输出
    switch (instr.opcode) {
        case OP_PUSH:
            snprintf(buffer, size, "%-8s #%u", opname, instr.operand);
            break;
        case OP_LOAD:
        case OP_STORE:
            snprintf(buffer, size, "%-8s %s %u", opname, 
                    (instr.flags & FLAG_GLOBAL) ? "global" : "local",
                    instr.operand);
            break;
        case OP_JMP:
        case OP_JZ:
        case OP_JNZ:
            snprintf(buffer, size, "%-8s @%u", opname, instr.operand);
            break;
        case OP_CALL:
        case OP_CALL_EXT:
            snprintf(buffer, size, "%-8s func[%u]", opname, instr.operand);
            break;
        case OP_POP:
        case OP_DUP:
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_MOD:
        case OP_NEG:
        case OP_EQ:
        case OP_NE:
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE:
        case OP_AND:
        case OP_OR:
        case OP_NOT:
        case OP_RET:
        case OP_HALT:
        case OP_NOP:
            snprintf(buffer, size, "%s", opname);
            break;
        default:
            snprintf(buffer, size, "%-8s %u", opname, instr.operand);
            break;
    }
}

/**
 * @brief 打印字节码模块
 */
void bytecode_print_module(BytecodeModule* module) {
    if (!module) return;
    
    printf("\n=== Bytecode Module ===\n");
    printf("Entry point: %u\n", module->entry_point);
    printf("Global variables: %u\n", module->global_count);
    
    // 打印常量池
    printf("\n--- Constants (%u) ---\n", module->const_count);
    for (uint32_t i = 0; i < module->const_count; i++) {
        printf("  [%u] ", i);
        switch (module->constants[i].type) {
            case CONST_INT:
                printf("INT: %d\n", module->constants[i].int_val);
                break;
            case CONST_REAL:
                printf("REAL: %g\n", module->constants[i].real_val);
                break;
            case CONST_BOOL:
                printf("BOOL: %s\n", module->constants[i].bool_val ? "true" : "false");
                break;
            case CONST_STRING:
                printf("STRING: \"%s\"\n", module->constants[i].string_val);
                break;
        }
    }
    
    // 打印函数表
    printf("\n--- Functions (%u) ---\n", module->function_count);
    for (uint32_t i = 0; i < module->function_count; i++) {
        FunctionEntry* func = &module->functions[i];
        printf("  [%u] %s(@%u): params=%d, locals=%d, return=%s\n",
               i, func->name, func->address,
               func->param_count, func->local_count,
               type_to_string(func->return_type));
    }
    
    // 打印指令
    printf("\n--- Instructions (%u) ---\n", module->instruction_count);
    char disasm[128];
    for (uint32_t i = 0; i < module->instruction_count; i++) {
        bytecode_disassemble_instruction(module->instructions[i], disasm, sizeof(disasm));
        
        if (module->line_numbers) {
            printf("  %04u (line %3d): %s\n", i, module->line_numbers[i], disasm);
        } else {
            printf("  %04u: %s\n", i, disasm);
        }
    }
    
    printf("======================\n\n");
}
