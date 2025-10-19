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
        "AND", "OR", "NOT", "XOR",
        "BIT_AND", "BIT_OR", "BIT_XOR", "BIT_NOT", "SHL", "SHR",
        "JMP", "JZ", "JNZ", "CALL", "RET",
        "HALT", "CALL_EXT", "NOP", "LOAD_INDEXED", "STORE_INDEXED"
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
    module->library_deps = NULL;
    module->library_dep_count = 0;
    
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
    
    // 释放函数表中的字符串和参数类型数组
    for (uint32_t i = 0; i < module->function_count; i++) {
        if (module->functions[i].name) {
            mmgr_free(module->functions[i].name);
        }
        if (module->functions[i].param_types) {
            mmgr_free(module->functions[i].param_types);
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
    
    // 释放库依赖信息
    for (uint32_t i = 0; i < module->library_dep_count; i++) {
        if (module->library_deps[i]) {
            mmgr_free(module->library_deps[i]);
        }
    }
    if (module->library_deps) {
        mmgr_free(module->library_deps);
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
                                int32_t param_count, int32_t local_count, DataType return_type,
                                const DataType* param_types) {
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
    
    // 复制参数类型数组
    if (param_types && param_count > 0) {
        module->functions[index].param_types = (DataType*)mmgr_alloc(sizeof(DataType) * param_count);
        if (module->functions[index].param_types) {
            memcpy(module->functions[index].param_types, param_types, sizeof(DataType) * param_count);
        }
    } else {
        module->functions[index].param_types = NULL;
    }
    
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
        case OP_XOR:
        case OP_BIT_AND:
        case OP_BIT_OR:
        case OP_BIT_XOR:
        case OP_BIT_NOT:
        case OP_SHL:
        case OP_SHR:
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

/**
 * @brief 将库模块合并到主模块(静态链接)
 * 
 * 合并过程:
 * 1. 合并常量池 - 库常量添加到主模块,更新库指令中的常量索引
 * 2. 合并函数表 - 库函数添加到主模块,更新地址偏移
 * 3. 合并指令流 - 库指令追加到主模块,更新跳转地址和调用地址
 * 4. 更新外部调用 - 将主模块中的 CALL_EXT 改为 CALL
 */
ErrorCode bytecode_merge_library(BytecodeModule* main, BytecodeModule* library, const char* library_name) {
    if (!main || !library || !library_name) {
        return ERR_RUNTIME;
    }
    
    // 保存当前主模块的偏移量,用于地址重定位
    uint32_t func_offset = main->function_count;
    uint32_t instr_offset = main->instruction_count;
    
    // === 1. 合并常量池 - 建立索引映射 ===
    // 分配索引映射表 (库常量索引 -> 主模块常量索引)
    uint32_t* const_map = (uint32_t*)mmgr_alloc(sizeof(uint32_t) * library->const_count);
    if (!const_map) {
        return ERR_OUT_OF_MEMORY;
    }
    
    for (uint32_t i = 0; i < library->const_count; i++) {
        Constant* lib_const = &library->constants[i];
        uint32_t new_idx;
        
        switch (lib_const->type) {
            case CONST_INT:
                new_idx = bytecode_add_int_constant(main, lib_const->int_val);
                break;
            case CONST_REAL:
                new_idx = bytecode_add_real_constant(main, lib_const->real_val);
                break;
            case CONST_BOOL:
                new_idx = bytecode_add_bool_constant(main, lib_const->bool_val);
                break;
            case CONST_STRING:
                new_idx = bytecode_add_string_constant(main, lib_const->string_val);
                break;
            default:
                mmgr_free(const_map);
                return ERR_INVALID_BYTECODE;
        }
        
        // 记录索引映射
        const_map[i] = new_idx;
    }
    
    // === 2. 合并函数表 ===
    for (uint32_t i = 0; i < library->function_count; i++) {
        FunctionEntry* lib_func = &library->functions[i];
        
        // 构建完全限定名 (例如: mathlib::Add)
        char qualified_name[256];
        snprintf(qualified_name, sizeof(qualified_name), "%s::%s", library_name, lib_func->name);
        
        // 添加函数,地址需要重定位
        uint32_t new_address = lib_func->address + instr_offset;
        uint32_t new_idx = bytecode_add_function(
            main,
            qualified_name,
            new_address,
            lib_func->param_count,
            lib_func->local_count,
            lib_func->return_type,
            lib_func->param_types
        );
        
        // 验证索引连续性
        if (new_idx != func_offset + i) {
            fprintf(stderr, "Error: Function index mismatch during merge\n");
            return ERR_INVALID_BYTECODE;
        }
    }
    
    // === 3. 合并指令流 ===
    // 先扩展指令数组容量
    uint32_t new_instr_count = main->instruction_count + library->instruction_count;
    if (new_instr_count > main->instruction_capacity) {
        uint32_t new_capacity = main->instruction_capacity;
        while (new_capacity < new_instr_count) {
            new_capacity *= 2;
        }
        Instruction* new_instructions = (Instruction*)mmgr_realloc(
            main->instructions,
            sizeof(Instruction) * new_capacity
        );
        if (!new_instructions) {
            return ERR_OUT_OF_MEMORY;
        }
        main->instructions = new_instructions;
        main->instruction_capacity = new_capacity;
    }
    
    // 复制并重定位库指令
    for (uint32_t i = 0; i < library->instruction_count; i++) {
        Instruction instr = library->instructions[i];
        Opcode op = instr.opcode;
        uint16_t operand = instr.operand;
        uint8_t flags = instr.flags;
        
        // 根据操作码类型重定位操作数
        switch (op) {
            case OP_PUSH:
                // 常量索引需要使用映射表重定位
                if (operand < library->const_count) {
                    operand = const_map[operand];
                }
                break;
                
            case OP_CALL:
                // 函数索引需要重定位
                operand += func_offset;
                break;
                
            case OP_JMP:
            case OP_JZ:
            case OP_JNZ:
                // 跳转地址需要重定位
                operand += instr_offset;
                break;
                
            case OP_CALL_EXT:
                // 外部调用需要转换为普通调用
                // 函数索引已经在函数表合并时处理
                operand += func_offset;
                op = OP_CALL;  // 改为普通CALL
                break;
                
            // 其他指令不需要重定位
            default:
                break;
        }
        
        // 创建重定位后的指令
        main->instructions[instr_offset + i].opcode = op;
        main->instructions[instr_offset + i].flags = flags;
        main->instructions[instr_offset + i].operand = operand;
    }
    
    main->instruction_count = new_instr_count;
    
    // 释放映射表
    mmgr_free(const_map);
    
    // === 4. 更新主模块中的外部调用 ===
    // 遍历主模块原有指令,将对库函数的 CALL_EXT 改为 CALL,并修正函数索引
    size_t lib_name_len = strlen(library_name);
    for (uint32_t i = 0; i < instr_offset; i++) {
        Instruction* instr = &main->instructions[i];
        Opcode op = instr->opcode;
        
        if (op == OP_CALL_EXT) {
            uint16_t old_func_idx = instr->operand;
            
            // 检查是否是对库函数的调用
            if (old_func_idx < func_offset) {  // 只检查主模块原有的函数
                FunctionEntry* func = &main->functions[old_func_idx];
                
                // 检查函数名是否以库路径为前缀 (例如 "examples/mathlib.stbc.")
                // 限定名格式: library_path.symbol_name
                if (strncmp(func->name, library_name, lib_name_len) == 0 &&
                    func->name[lib_name_len] == '.') {
                    // 这是对库函数的调用,需要找到合并后的实际函数索引
                    // 主模块中的名称: "examples/mathlib.stbc.Power"
                    // 合并后的名称: "examples/mathlib.stbc::Power"
                    const char* symbol_name = func->name + lib_name_len + 1;  // 跳过 "."
                    
                    // 查找合并后的函数
                    bool found = false;
                    for (uint32_t j = func_offset; j < main->function_count; j++) {
                        FunctionEntry* lib_func = &main->functions[j];
                        
                        // 构建期望的名称: library_path::symbol_name
                        char expected_name[512];
                        snprintf(expected_name, sizeof(expected_name), "%s::%s", library_name, symbol_name);
                        
                        if (strcmp(lib_func->name, expected_name) == 0) {
                            // 找到匹配的函数,更新指令
                            instr->opcode = OP_CALL;
                            instr->operand = j;
                            found = true;
                            break;
                        }
                    }
                    
                    if (!found) {
                        fprintf(stderr, "Warning: Cannot find library function for '%s'\n", func->name);
                    }
                }
            }
        }
    }
    
    return OK;
}

/**
 * @brief 添加库依赖到字节码模块
 */
ErrorCode bytecode_add_library_dependency(BytecodeModule* module, const char* library_path) {
    if (!module || !library_path) {
        return ERR_RUNTIME;
    }
    
    // 检查是否已经存在此库依赖
    for (uint32_t i = 0; i < module->library_dep_count; i++) {
        if (strcmp(module->library_deps[i], library_path) == 0) {
            return OK;  // 已存在,不重复添加
        }
    }
    
    // 扩展库依赖数组
    char** new_deps = (char**)mmgr_realloc(
        module->library_deps,
        sizeof(char*) * (module->library_dep_count + 1)
    );
    if (!new_deps) {
        return ERR_OUT_OF_MEMORY;
    }
    
    // 复制库路径
    size_t len = strlen(library_path);
    char* lib_path_copy = (char*)mmgr_alloc(len + 1);
    if (!lib_path_copy) {
        return ERR_OUT_OF_MEMORY;
    }
    strcpy(lib_path_copy, library_path);
    
    new_deps[module->library_dep_count] = lib_path_copy;
    module->library_deps = new_deps;
    module->library_dep_count++;
    
    return OK;
}
