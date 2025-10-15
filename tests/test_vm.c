/**
 * @file test_vm.c
 * @brief 虚拟机测试
 */

#include "vm.h"
#include "bytecode.h"
#include "mmgr.h"
#include <stdio.h>
#include <assert.h>

void test_basic_arithmetic() {
    printf("\n--- Test: Basic Arithmetic ---\n");
    fflush(stdout);
    
    // 创建字节码模块: 2 + 3 * 4 = 14
    BytecodeModule* module = bytecode_module_create();
    
    // 添加常量
    uint32_t c2 = bytecode_add_int_constant(module, 2);
    uint32_t c3 = bytecode_add_int_constant(module, 3);
    uint32_t c4 = bytecode_add_int_constant(module, 4);
    
    // 生成字节码: PUSH 3, PUSH 4, MUL, PUSH 2, ADD, HALT
    bytecode_add_instruction(module, OP_PUSH, 0, c3);
    bytecode_add_instruction(module, OP_PUSH, 0, c4);
    bytecode_add_instruction(module, OP_MUL, 0, 0);
    bytecode_add_instruction(module, OP_PUSH, 0, c2);
    bytecode_add_instruction(module, OP_ADD, 0, 0);
    bytecode_add_instruction(module, OP_HALT, 0, 0);
    
    module->entry_point = 0;
    
    // 创建虚拟机并执行
    VM* vm = vm_create(module);
    assert(vm != NULL);
    
    ErrorCode err = vm_run(vm);
    assert(err == OK);
    
    // 检查结果
    Value result;
    vm_get_result(vm, &result);
    assert(result.type == TYPE_INT);
    assert(result.int_val == 14);
    
    printf("✓ Result: %d\n", result.int_val);
    
    vm_free(vm);
    bytecode_module_free(module);
}

void test_comparison_and_jump() {
    printf("\n--- Test: Comparison and Jump ---\n");
    
    BytecodeModule* module = bytecode_module_create();
    
    // 添加常量
    uint32_t c5 = bytecode_add_int_constant(module, 5);
    uint32_t c10 = bytecode_add_int_constant(module, 10);
    uint32_t c100 = bytecode_add_int_constant(module, 100);
    uint32_t c200 = bytecode_add_int_constant(module, 200);
    
    // 字节码: if (5 < 10) result = 100; else result = 200;
    // 0: PUSH 5
    // 1: PUSH 10
    // 2: LT
    // 3: JZ 6     // 如果false，跳到6
    // 4: PUSH 100
    // 5: JMP 7    // 跳到7
    // 6: PUSH 200
    // 7: HALT
    
    bytecode_add_instruction(module, OP_PUSH, 0, c5);      // 0
    bytecode_add_instruction(module, OP_PUSH, 0, c10);     // 1
    bytecode_add_instruction(module, OP_LT, 0, 0);         // 2
    bytecode_add_instruction(module, OP_JZ, 0, 6);         // 3
    bytecode_add_instruction(module, OP_PUSH, 0, c100);    // 4
    bytecode_add_instruction(module, OP_JMP, 0, 7);        // 5
    bytecode_add_instruction(module, OP_PUSH, 0, c200);    // 6
    bytecode_add_instruction(module, OP_HALT, 0, 0);       // 7
    
    module->entry_point = 0;
    
    VM* vm = vm_create(module);
    ErrorCode err = vm_run(vm);
    assert(err == OK);
    
    Value result;
    vm_get_result(vm, &result);
    assert(result.type == TYPE_INT);
    assert(result.int_val == 100);  // 5 < 10 is true
    
    printf("✓ Result: %d (expected 100)\n", result.int_val);
    
    vm_free(vm);
    bytecode_module_free(module);
}

void test_global_variables() {
    printf("\n--- Test: Global Variables ---\n");
    
    BytecodeModule* module = bytecode_module_create();
    module->global_count = 2;  // 两个全局变量
    
    uint32_t c42 = bytecode_add_int_constant(module, 42);
    
    // 字节码: g0 = 42; g1 = g0 + g0; result = g1
    // PUSH 42
    // STORE g0
    // LOAD g0
    // LOAD g0
    // ADD
    // STORE g1
    // LOAD g1
    // HALT
    
    bytecode_add_instruction(module, OP_PUSH, 0, c42);
    bytecode_add_instruction(module, OP_STORE, FLAG_GLOBAL, 0);
    bytecode_add_instruction(module, OP_LOAD, FLAG_GLOBAL, 0);
    bytecode_add_instruction(module, OP_LOAD, FLAG_GLOBAL, 0);
    bytecode_add_instruction(module, OP_ADD, 0, 0);
    bytecode_add_instruction(module, OP_STORE, FLAG_GLOBAL, 1);
    bytecode_add_instruction(module, OP_LOAD, FLAG_GLOBAL, 1);
    bytecode_add_instruction(module, OP_HALT, 0, 0);
    
    module->entry_point = 0;
    
    VM* vm = vm_create(module);
    ErrorCode err = vm_run(vm);
    assert(err == OK);
    
    Value result;
    vm_get_result(vm, &result);
    assert(result.int_val == 84);
    
    printf("✓ Global var result: %d\n", result.int_val);
    
    vm_free(vm);
    bytecode_module_free(module);
}

void test_real_numbers() {
    printf("\n--- Test: Real Numbers ---\n");
    
    BytecodeModule* module = bytecode_module_create();
    
    uint32_t c_pi = bytecode_add_real_constant(module, 3.14159);
    uint32_t c2 = bytecode_add_real_constant(module, 2.0);
    
    // 计算: pi * 2
    bytecode_add_instruction(module, OP_PUSH, 0, c_pi);
    bytecode_add_instruction(module, OP_PUSH, 0, c2);
    bytecode_add_instruction(module, OP_MUL, 0, 0);
    bytecode_add_instruction(module, OP_HALT, 0, 0);
    
    module->entry_point = 0;
    
    VM* vm = vm_create(module);
    ErrorCode err = vm_run(vm);
    assert(err == OK);
    
    Value result;
    vm_get_result(vm, &result);
    assert(result.type == TYPE_REAL);
    printf("✓ Real result: %.5f\n", result.real_val);
    
    vm_free(vm);
    bytecode_module_free(module);
}

void test_stack_operations() {
    printf("\n--- Test: Stack Operations ---\n");
    
    BytecodeModule* module = bytecode_module_create();
    
    uint32_t c5 = bytecode_add_int_constant(module, 5);
    
    // PUSH 5, DUP, ADD -> result = 10
    bytecode_add_instruction(module, OP_PUSH, 0, c5);
    bytecode_add_instruction(module, OP_DUP, 0, 0);
    bytecode_add_instruction(module, OP_ADD, 0, 0);
    bytecode_add_instruction(module, OP_HALT, 0, 0);
    
    module->entry_point = 0;
    
    VM* vm = vm_create(module);
    ErrorCode err = vm_run(vm);
    assert(err == OK);
    
    Value result;
    vm_get_result(vm, &result);
    assert(result.int_val == 10);
    
    printf("✓ Stack DUP result: %d\n", result.int_val);
    
    vm_free(vm);
    bytecode_module_free(module);
}

void test_logical_operations() {
    printf("\n--- Test: Logical Operations ---\n");
    
    BytecodeModule* module = bytecode_module_create();
    
    uint32_t c_true = bytecode_add_bool_constant(module, true);
    uint32_t c_false = bytecode_add_bool_constant(module, false);
    
    // true AND false -> false
    bytecode_add_instruction(module, OP_PUSH, 0, c_true);
    bytecode_add_instruction(module, OP_PUSH, 0, c_false);
    bytecode_add_instruction(module, OP_AND, 0, 0);
    bytecode_add_instruction(module, OP_HALT, 0, 0);
    
    module->entry_point = 0;
    
    VM* vm = vm_create(module);
    ErrorCode err = vm_run(vm);
    assert(err == OK);
    
    Value result;
    vm_get_result(vm, &result);
    assert(result.type == TYPE_BOOL);
    assert(result.bool_val == false);
    
    printf("✓ Logical AND result: %s\n", result.bool_val ? "true" : "false");
    
    vm_free(vm);
    bytecode_module_free(module);
}

// 外部函数示例：打印整数
Value ext_print_int(VM* vm, int32_t argc) {
    (void)argc; // 忽略未使用警告
    Value arg = vm_get_arg(vm, 0);
    printf("[External] print_int: %d\n", arg.int_val);
    Value v = {.type = TYPE_VOID};
    return v;
}

// 外部函数示例：两个整数相加
Value ext_add(VM* vm, int32_t argc) {
    (void)argc;
    Value a = vm_get_arg(vm, 1); // 注意：参数顺序是从栈底到栈顶
    Value b = vm_get_arg(vm, 0);
    Value result;
    result.type = TYPE_INT;
    result.int_val = a.int_val + b.int_val;
    return result;
}

void test_external_functions() {
    printf("\n--- Test: External Functions ---\n");
    
    BytecodeModule* module = bytecode_module_create();
    
    // 注册两个外部函数
    DataType params_one[] = {TYPE_INT};
    DataType params_two[] = {TYPE_INT, TYPE_INT};
    uint32_t print_idx = bytecode_add_function(module, "print_int", 0, 1, 0, TYPE_VOID, params_one);
    uint32_t add_idx = bytecode_add_function(module, "ext_add", 0, 2, 0, TYPE_INT, params_two);
    
    // 测试1: 调用print_int(42)
    uint32_t c42 = bytecode_add_int_constant(module, 42);
    bytecode_add_instruction(module, OP_PUSH, 0, c42);
    bytecode_add_instruction(module, OP_CALL_EXT, 1, print_idx); // flags=参数个数
    
    // 测试2: 调用ext_add(10, 20)
    uint32_t c10 = bytecode_add_int_constant(module, 10);
    uint32_t c20 = bytecode_add_int_constant(module, 20);
    bytecode_add_instruction(module, OP_PUSH, 0, c10);
    bytecode_add_instruction(module, OP_PUSH, 0, c20);
    bytecode_add_instruction(module, OP_CALL_EXT, 2, add_idx);
    
    bytecode_add_instruction(module, OP_HALT, 0, 0);
    module->entry_point = 0;
    
    VM* vm = vm_create(module);
    
    // 注册外部函数
    vm_register_external_function(vm, "print_int", ext_print_int, 1);
    vm_register_external_function(vm, "ext_add", ext_add, 2);
    
    ErrorCode err = vm_run(vm);
    assert(err == OK);
    
    // 检查ext_add的结果
    Value result;
    vm_get_result(vm, &result);
    assert(result.type == TYPE_INT);
    assert(result.int_val == 30);
    
    printf("✓ External function result: %d\n", result.int_val);
    
    vm_free(vm);
    bytecode_module_free(module);
}

int main() {
    printf("========================================\n");
    printf("  STVM Virtual Machine Test Suite\n");
    printf("========================================\n");
    
    mmgr_init();
    printf("✓ Memory manager initialized\n");
    
    test_basic_arithmetic();
    test_comparison_and_jump();
    test_global_variables();
    test_real_numbers();
    test_stack_operations();
    test_logical_operations();
    test_external_functions();
    
    mmgr_print_stats();
    mmgr_cleanup();
    
    printf("\n========================================\n");
    printf("  All VM Tests Passed! ✓\n");
    printf("========================================\n");
    
    return 0;
}
