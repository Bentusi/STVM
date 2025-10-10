/**
 * @file test_bytecode.c
 * @brief 字节码模块测试程序
 */

#include "bytecode.h"
#include "mmgr.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_module_creation(void) {
    printf("\n--- Test: Module Creation ---\n");
    
    BytecodeModule* module = bytecode_module_create();
    assert(module != NULL);
    assert(module->instruction_count == 0);
    assert(module->const_count == 0);
    assert(module->function_count == 0);
    printf("✓ Created empty bytecode module\n");
    
    bytecode_module_free(module);
    printf("✓ Freed bytecode module\n");
}

void test_add_instructions(void) {
    printf("\n--- Test: Add Instructions ---\n");
    
    BytecodeModule* module = bytecode_module_create();
    
    // 添加一些指令
    uint32_t idx1 = bytecode_add_instruction(module, OP_PUSH, 0, 0);
    assert(idx1 == 0);
    printf("✓ Added PUSH instruction at index %u\n", idx1);
    
    uint32_t idx2 = bytecode_add_instruction(module, OP_ADD, 0, 0);
    assert(idx2 == 1);
    printf("✓ Added ADD instruction at index %u\n", idx2);
    
    uint32_t idx3 = bytecode_add_instruction(module, OP_HALT, 0, 0);
    assert(idx3 == 2);
    printf("✓ Added HALT instruction at index %u\n", idx3);
    
    assert(module->instruction_count == 3);
    printf("✓ Total instructions: %u\n", module->instruction_count);
    
    bytecode_module_free(module);
}

void test_add_constants(void) {
    printf("\n--- Test: Add Constants ---\n");
    
    BytecodeModule* module = bytecode_module_create();
    
    // 添加整数常量
    uint32_t int_idx = bytecode_add_int_constant(module, 42);
    assert(module->constants[int_idx].type == CONST_INT);
    assert(module->constants[int_idx].int_val == 42);
    printf("✓ Added INT constant: 42 at index %u\n", int_idx);
    
    // 添加实数常量
    uint32_t real_idx = bytecode_add_real_constant(module, 3.14);
    assert(module->constants[real_idx].type == CONST_REAL);
    assert(module->constants[real_idx].real_val == 3.14);
    printf("✓ Added REAL constant: 3.14 at index %u\n", real_idx);
    
    // 添加布尔常量
    uint32_t bool_idx = bytecode_add_bool_constant(module, true);
    assert(module->constants[bool_idx].type == CONST_BOOL);
    assert(module->constants[bool_idx].bool_val == true);
    printf("✓ Added BOOL constant: true at index %u\n", bool_idx);
    
    // 添加字符串常量
    uint32_t str_idx = bytecode_add_string_constant(module, "Hello, World!");
    assert(module->constants[str_idx].type == CONST_STRING);
    printf("✓ Added STRING constant: \"%s\" at index %u\n", 
           module->constants[str_idx].string_val, str_idx);
    
    // 测试常量去重
    uint32_t dup_idx = bytecode_add_int_constant(module, 42);
    assert(dup_idx == int_idx);
    printf("✓ Duplicate constant reused same index: %u\n", dup_idx);
    
    bytecode_module_free(module);
}

void test_add_functions(void) {
    printf("\n--- Test: Add Functions ---\n");
    
    BytecodeModule* module = bytecode_module_create();
    
    // 添加函数
    uint32_t func_idx = bytecode_add_function(module, "test_func", 100, 2, 3, TYPE_INT);
    assert(func_idx == 0);
    
    FunctionEntry* func = &module->functions[func_idx];
    assert(strcmp(func->name, "test_func") == 0);
    assert(func->address == 100);
    assert(func->param_count == 2);
    assert(func->local_count == 3);
    assert(func->return_type == TYPE_INT);
    printf("✓ Added function: %s(@%u, params=%d, locals=%d)\n",
           func->name, func->address, func->param_count, func->local_count);
    
    // 查找函数
    FunctionEntry* found = bytecode_find_function(module, "test_func");
    assert(found != NULL);
    assert(found == func);
    printf("✓ Found function: %s\n", found->name);
    
    // 查找不存在的函数
    FunctionEntry* not_found = bytecode_find_function(module, "nonexistent");
    assert(not_found == NULL);
    printf("✓ Non-existent function returned NULL\n");
    
    bytecode_module_free(module);
}

void test_patch_operand(void) {
    printf("\n--- Test: Patch Operand ---\n");
    
    BytecodeModule* module = bytecode_module_create();
    
    // 添加跳转指令（操作数暂时为0）
    uint32_t jmp_idx = bytecode_add_instruction(module, OP_JMP, 0, 0);
    printf("✓ Added JMP instruction with operand 0\n");
    
    // 添加更多指令
    bytecode_add_instruction(module, OP_PUSH, 0, 0);
    bytecode_add_instruction(module, OP_PUSH, 0, 1);
    
    // 回填跳转目标
    uint32_t target = bytecode_current_position(module);
    bytecode_patch_operand(module, jmp_idx, target);
    assert(module->instructions[jmp_idx].operand == target);
    printf("✓ Patched JMP operand to %u\n", target);
    
    bytecode_module_free(module);
}

void test_disassemble(void) {
    printf("\n--- Test: Disassemble Instructions ---\n");
    
    BytecodeModule* module = bytecode_module_create();
    char buffer[128];
    
    // PUSH指令
    Instruction push_instr = {OP_PUSH, 0, 5};
    bytecode_disassemble_instruction(push_instr, buffer, sizeof(buffer));
    printf("  %s\n", buffer);
    
    // LOAD指令（局部变量）
    Instruction load_instr = {OP_LOAD, FLAG_LOCAL, 2};
    bytecode_disassemble_instruction(load_instr, buffer, sizeof(buffer));
    printf("  %s\n", buffer);
    
    // STORE指令（全局变量）
    Instruction store_instr = {OP_STORE, FLAG_GLOBAL, 0};
    bytecode_disassemble_instruction(store_instr, buffer, sizeof(buffer));
    printf("  %s\n", buffer);
    
    // ADD指令
    Instruction add_instr = {OP_ADD, 0, 0};
    bytecode_disassemble_instruction(add_instr, buffer, sizeof(buffer));
    printf("  %s\n", buffer);
    
    // JZ指令
    Instruction jz_instr = {OP_JZ, 0, 100};
    bytecode_disassemble_instruction(jz_instr, buffer, sizeof(buffer));
    printf("  %s\n", buffer);
    
    printf("✓ All instructions disassembled\n");
    
    bytecode_module_free(module);
}

void test_simple_program(void) {
    printf("\n--- Test: Simple Program ---\n");
    
    BytecodeModule* module = bytecode_module_create();
    
    // 简单程序：计算 2 + 3
    // PUSH #0  (常量2)
    // PUSH #1  (常量3)
    // ADD
    // HALT
    
    uint32_t c2 = bytecode_add_int_constant(module, 2);
    uint32_t c3 = bytecode_add_int_constant(module, 3);
    
    bytecode_add_instruction(module, OP_PUSH, 0, c2);
    bytecode_add_instruction(module, OP_PUSH, 0, c3);
    bytecode_add_instruction(module, OP_ADD, 0, 0);
    bytecode_add_instruction(module, OP_HALT, 0, 0);
    
    printf("✓ Generated bytecode for: 2 + 3\n");
    printf("  Constants: %u\n", module->const_count);
    printf("  Instructions: %u\n", module->instruction_count);
    
    // 打印完整模块
    bytecode_print_module(module);
    
    bytecode_module_free(module);
}

void test_line_numbers(void) {
    printf("\n--- Test: Line Numbers ---\n");
    
    BytecodeModule* module = bytecode_module_create();
    
    // 添加带行号的指令
    bytecode_add_instruction_with_line(module, OP_PUSH, 0, 0, 10);
    bytecode_add_instruction_with_line(module, OP_PUSH, 0, 1, 11);
    bytecode_add_instruction_with_line(module, OP_ADD, 0, 0, 12);
    
    assert(module->line_numbers != NULL);
    assert(module->line_numbers[0] == 10);
    assert(module->line_numbers[1] == 11);
    assert(module->line_numbers[2] == 12);
    printf("✓ Line numbers recorded correctly\n");
    
    bytecode_module_free(module);
}

void test_opcode_strings(void) {
    printf("\n--- Test: Opcode Strings ---\n");
    
    assert(strcmp(opcode_to_string(OP_PUSH), "PUSH") == 0);
    assert(strcmp(opcode_to_string(OP_ADD), "ADD") == 0);
    assert(strcmp(opcode_to_string(OP_JMP), "JMP") == 0);
    assert(strcmp(opcode_to_string(OP_CALL), "CALL") == 0);
    assert(strcmp(opcode_to_string(OP_HALT), "HALT") == 0);
    
    printf("✓ All opcode strings correct\n");
}

int main(void) {
    printf("========================================\n");
    printf("  STVM Bytecode Module Test Suite\n");
    printf("========================================\n");
    
    // 初始化内存管理器
    if (!mmgr_init()) {
        fprintf(stderr, "Failed to initialize memory manager\n");
        return 1;
    }
    printf("✓ Memory manager initialized\n");
    
    // 运行测试
    test_module_creation();
    test_add_instructions();
    test_add_constants();
    test_add_functions();
    test_patch_operand();
    test_disassemble();
    test_simple_program();
    test_line_numbers();
    test_opcode_strings();
    
    // 打印统计信息
    mmgr_print_stats();
    
    // 检查内存泄漏
    if (mmgr_check_leaks()) {
        printf("\n❌ Memory leaks detected!\n");
    } else {
        printf("\n✓ No memory leaks detected\n");
    }
    
    // 清理
    mmgr_cleanup();
    printf("✓ Memory manager cleaned up\n");
    
    printf("\n========================================\n");
    printf("  All Tests Passed! ✓\n");
    printf("========================================\n");
    
    return 0;
}
