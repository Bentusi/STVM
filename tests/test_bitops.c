/**
 * @file test_bitops.c
 * @brief 位操作功能测试（符合 IEC 61131-3 标准）
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "bytecode.h"
#include "vm.h"
#include "mmgr.h"

// 颜色定义
#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define YELLOW "\033[0;33m"
#define RESET "\033[0m"

/**
 * @brief 测试辅助函数：创建测试模块
 */
static BytecodeModule* create_test_module(void) {
    BytecodeModule* module = bytecode_module_create();
    assert(module != NULL);
    return module;
}

/**
 * @brief 测试辅助函数：释放模块和VM
 */
static void cleanup(BytecodeModule* module, VM* vm) {
    if (vm) vm_free(vm);
    if (module) bytecode_module_free(module);
}

/**
 * @brief 测试用例1：位与操作（BIT_AND）
 */
static void test_bit_and(void) {
    printf("  测试 BIT_AND (位与)... ");
    
    BytecodeModule* module = create_test_module();
    
    // 测试: 0x0F & 0x33 = 0x03
    // PUSH 15 (0x0F)
    // PUSH 51 (0x33)
    // BIT_AND
    // HALT
    
    int32_t idx1 = bytecode_add_int_constant(module, 15);
    int32_t idx2 = bytecode_add_int_constant(module, 51);
    
    bytecode_add_instruction(module, OP_PUSH, 0, idx1);
    bytecode_add_instruction(module, OP_PUSH, 0, idx2);
    bytecode_add_instruction(module, OP_BIT_AND, 0, 0);
    bytecode_add_instruction(module, OP_HALT, 0, 0);
    
    VM* vm = vm_create(module);
    assert(vm != NULL);
    
    ErrorCode err = vm_run(vm);
    assert(err == OK);
    assert(vm->sp == 0);
    assert(vm->stack[0].type == TYPE_INT);
    assert(vm->stack[0].int_val == 3);  // 0x0F & 0x33 = 0x03
    
    cleanup(module, vm);
    printf(GREEN "通过✓" RESET "\n");
}

/**
 * @brief 测试用例2：位或操作（BIT_OR）
 */
static void test_bit_or(void) {
    printf("  测试 BIT_OR (位或)... ");
    
    BytecodeModule* module = create_test_module();
    
    // 测试: 0x0F | 0x30 = 0x3F
    int32_t idx1 = bytecode_add_int_constant(module, 15);  // 0x0F
    int32_t idx2 = bytecode_add_int_constant(module, 48);  // 0x30
    
    bytecode_add_instruction(module, OP_PUSH, 0, idx1);
    bytecode_add_instruction(module, OP_PUSH, 0, idx2);
    bytecode_add_instruction(module, OP_BIT_OR, 0, 0);
    bytecode_add_instruction(module, OP_HALT, 0, 0);
    
    VM* vm = vm_create(module);
    assert(vm != NULL);
    
    ErrorCode err = vm_run(vm);
    assert(err == OK);
    assert(vm->stack[0].int_val == 63);  // 0x0F | 0x30 = 0x3F
    
    cleanup(module, vm);
    printf(GREEN "通过✓" RESET "\n");
}

/**
 * @brief 测试用例3：位异或操作（BIT_XOR）
 */
static void test_bit_xor(void) {
    printf("  测试 BIT_XOR (位异或)... ");
    
    BytecodeModule* module = create_test_module();
    
    // 测试: 0xFF ^ 0xAA = 0x55
    int32_t idx1 = bytecode_add_int_constant(module, 255);  // 0xFF
    int32_t idx2 = bytecode_add_int_constant(module, 170);  // 0xAA
    
    bytecode_add_instruction(module, OP_PUSH, 0, idx1);
    bytecode_add_instruction(module, OP_PUSH, 0, idx2);
    bytecode_add_instruction(module, OP_BIT_XOR, 0, 0);
    bytecode_add_instruction(module, OP_HALT, 0, 0);
    
    VM* vm = vm_create(module);
    assert(vm != NULL);
    
    ErrorCode err = vm_run(vm);
    assert(err == OK);
    assert(vm->stack[0].int_val == 85);  // 0xFF ^ 0xAA = 0x55
    
    cleanup(module, vm);
    printf(GREEN "通过✓" RESET "\n");
}

/**
 * @brief 测试用例4：位取反操作（BIT_NOT）
 */
static void test_bit_not(void) {
    printf("  测试 BIT_NOT (位取反)... ");
    
    BytecodeModule* module = create_test_module();
    
    // 测试: ~0x0F = 0xFFFFFFF0 (32位补码)
    int32_t idx = bytecode_add_int_constant(module, 15);  // 0x0F
    
    bytecode_add_instruction(module, OP_PUSH, 0, idx);
    bytecode_add_instruction(module, OP_BIT_NOT, 0, 0);
    bytecode_add_instruction(module, OP_HALT, 0, 0);
    
    VM* vm = vm_create(module);
    assert(vm != NULL);
    
    ErrorCode err = vm_run(vm);
    assert(err == OK);
    assert(vm->stack[0].int_val == -16);  // ~15 = -16 (补码表示)
    
    cleanup(module, vm);
    printf(GREEN "通过✓" RESET "\n");
}

/**
 * @brief 测试用例5：左移操作（SHL）
 */
static void test_shl(void) {
    printf("  测试 SHL (左移)... ");
    
    BytecodeModule* module = create_test_module();
    
    // 测试: 1 << 3 = 8
    int32_t idx_value = bytecode_add_int_constant(module, 1);
    int32_t idx_shift = bytecode_add_int_constant(module, 3);
    
    bytecode_add_instruction(module, OP_PUSH, 0, idx_value);
    bytecode_add_instruction(module, OP_PUSH, 0, idx_shift);
    bytecode_add_instruction(module, OP_SHL, 0, 0);
    bytecode_add_instruction(module, OP_HALT, 0, 0);
    
    VM* vm = vm_create(module);
    assert(vm != NULL);
    
    ErrorCode err = vm_run(vm);
    assert(err == OK);
    assert(vm->stack[0].int_val == 8);  // 1 << 3 = 8
    
    cleanup(module, vm);
    printf(GREEN "通过✓" RESET "\n");
}

/**
 * @brief 测试用例6：右移操作（SHR）
 */
static void test_shr(void) {
    printf("  测试 SHR (右移)... ");
    
    BytecodeModule* module = create_test_module();
    
    // 测试: 16 >> 2 = 4
    int32_t idx_value = bytecode_add_int_constant(module, 16);
    int32_t idx_shift = bytecode_add_int_constant(module, 2);
    
    bytecode_add_instruction(module, OP_PUSH, 0, idx_value);
    bytecode_add_instruction(module, OP_PUSH, 0, idx_shift);
    bytecode_add_instruction(module, OP_SHR, 0, 0);
    bytecode_add_instruction(module, OP_HALT, 0, 0);
    
    VM* vm = vm_create(module);
    assert(vm != NULL);
    
    ErrorCode err = vm_run(vm);
    assert(err == OK);
    assert(vm->stack[0].int_val == 4);  // 16 >> 2 = 4
    
    cleanup(module, vm);
    printf(GREEN "通过✓" RESET "\n");
}

/**
 * @brief 测试用例7：复杂位操作（掩码操作）
 */
static void test_bitmask_operations(void) {
    printf("  测试位掩码操作... ");
    
    BytecodeModule* module = create_test_module();
    
    // 测试：设置第3位和第5位
    // value = 0
    // value = value OR (1 << 3)  // 设置第3位
    // value = value OR (1 << 5)  // 设置第5位
    // 结果应该是 40 (0b101000)
    
    int32_t idx_0 = bytecode_add_int_constant(module, 0);
    int32_t idx_1 = bytecode_add_int_constant(module, 1);
    int32_t idx_3 = bytecode_add_int_constant(module, 3);
    int32_t idx_5 = bytecode_add_int_constant(module, 5);
    
    // value = 0
    bytecode_add_instruction(module, OP_PUSH, 0, idx_0);
    
    // 设置第3位: value | (1 << 3)
    bytecode_add_instruction(module, OP_PUSH, 0, idx_1);
    bytecode_add_instruction(module, OP_PUSH, 0, idx_3);
    bytecode_add_instruction(module, OP_SHL, 0, 0);
    bytecode_add_instruction(module, OP_BIT_OR, 0, 0);
    
    // 设置第5位: value | (1 << 5)
    bytecode_add_instruction(module, OP_PUSH, 0, idx_1);
    bytecode_add_instruction(module, OP_PUSH, 0, idx_5);
    bytecode_add_instruction(module, OP_SHL, 0, 0);
    bytecode_add_instruction(module, OP_BIT_OR, 0, 0);
    
    bytecode_add_instruction(module, OP_HALT, 0, 0);
    
    VM* vm = vm_create(module);
    assert(vm != NULL);
    
    ErrorCode err = vm_run(vm);
    assert(err == OK);
    assert(vm->stack[0].int_val == 40);  // 0b101000 = 40
    
    cleanup(module, vm);
    printf(GREEN "通过✓" RESET "\n");
}

/**
 * @brief 测试用例8：位操作类型检查（错误情况）
 */
static void test_type_checking(void) {
    printf("  测试类型检查（期望失败）... ");
    
    BytecodeModule* module = create_test_module();
    
    // 测试：对REAL类型进行位操作应该失败
    int32_t idx_real = bytecode_add_real_constant(module, 3.14);
    int32_t idx_int = bytecode_add_int_constant(module, 5);
    
    bytecode_add_instruction(module, OP_PUSH, 0, idx_real);
    bytecode_add_instruction(module, OP_PUSH, 0, idx_int);
    bytecode_add_instruction(module, OP_BIT_AND, 0, 0);
    bytecode_add_instruction(module, OP_HALT, 0, 0);
    
    VM* vm = vm_create(module);
    assert(vm != NULL);
    
    ErrorCode err = vm_run(vm);
    assert(err == ERR_TYPE);  // 应该返回类型错误
    
    cleanup(module, vm);
    printf(GREEN "通过✓" RESET "\n");
}

/**
 * @brief 测试用例9：逻辑异或（XOR）
 */
static void test_logical_xor(void) {
    printf("  测试逻辑异或 (XOR)... ");
    
    BytecodeModule* module = create_test_module();
    
    // 测试: TRUE XOR FALSE = TRUE
    int32_t idx_true = bytecode_add_bool_constant(module, true);
    int32_t idx_false = bytecode_add_bool_constant(module, false);
    
    bytecode_add_instruction(module, OP_PUSH, 0, idx_true);
    bytecode_add_instruction(module, OP_PUSH, 0, idx_false);
    bytecode_add_instruction(module, OP_XOR, 0, 0);
    bytecode_add_instruction(module, OP_HALT, 0, 0);
    
    VM* vm = vm_create(module);
    assert(vm != NULL);
    
    ErrorCode err = vm_run(vm);
    assert(err == OK);
    assert(vm->stack[0].type == TYPE_BOOL);
    assert(vm->stack[0].bool_val == true);
    
    cleanup(module, vm);
    printf(GREEN "通过✓" RESET "\n");
}

/**
 * @brief 测试用例10：实际应用 - GPIO寄存器操作
 */
static void test_gpio_register_ops(void) {
    printf("  测试GPIO寄存器操作... ");
    
    BytecodeModule* module = create_test_module();
    
    // 模拟GPIO寄存器操作：
    // 1. 读取寄存器值 = 0x00
    // 2. 设置位2和位5 (输出使能)
    // 3. 清除位3 (禁用中断)
    // 结果: (0x00 | 0x24) & ~0x08 = 0x24
    
    int32_t idx_reg = bytecode_add_int_constant(module, 0x00);
    int32_t idx_set_mask = bytecode_add_int_constant(module, 0x24);  // 位2和位5
    int32_t idx_clear_mask = bytecode_add_int_constant(module, 0x08); // 位3
    
    // 读取寄存器
    bytecode_add_instruction(module, OP_PUSH, 0, idx_reg);
    
    // 设置位：reg | set_mask
    bytecode_add_instruction(module, OP_PUSH, 0, idx_set_mask);
    bytecode_add_instruction(module, OP_BIT_OR, 0, 0);
    
    // 清除位：result & ~clear_mask
    bytecode_add_instruction(module, OP_PUSH, 0, idx_clear_mask);
    bytecode_add_instruction(module, OP_BIT_NOT, 0, 0);
    bytecode_add_instruction(module, OP_BIT_AND, 0, 0);
    
    bytecode_add_instruction(module, OP_HALT, 0, 0);
    
    VM* vm = vm_create(module);
    assert(vm != NULL);
    
    ErrorCode err = vm_run(vm);
    assert(err == OK);
    assert(vm->stack[0].int_val == 0x24);
    
    cleanup(module, vm);
    printf(GREEN "通过✓" RESET "\n");
}

/**
 * @brief 主测试函数
 */
int main(void) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║     位操作功能测试套件（IEC 61131-3 标准）                ║\n");
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    printf("运行位操作测试...\n\n");
    
    // 基本位操作
    test_bit_and();
    test_bit_or();
    test_bit_xor();
    test_bit_not();
    
    // 移位操作
    test_shl();
    test_shr();
    
    // 复杂操作
    test_bitmask_operations();
    test_gpio_register_ops();
    
    // 逻辑XOR
    test_logical_xor();
    
    // 类型检查
    test_type_checking();
    
    printf("\n");
    printf(GREEN "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" RESET);
    printf(GREEN "  ✓ 所有测试通过！(10/10)\n" RESET);
    printf(GREEN "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" RESET);
    printf("\n");
    
    return 0;
}
