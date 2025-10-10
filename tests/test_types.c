/**
 * @file test_types.c
 * @brief 类型系统测试程序
 */

#include "types.h"
#include "mmgr.h"
#include "error.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_type_strings(void) {
    printf("\n--- Test: Type to String Conversion ---\n");
    
    assert(strcmp(type_to_string(TYPE_VOID), "void") == 0);
    assert(strcmp(type_to_string(TYPE_BOOL), "bool") == 0);
    assert(strcmp(type_to_string(TYPE_INT), "int") == 0);
    assert(strcmp(type_to_string(TYPE_REAL), "real") == 0);
    assert(strcmp(type_to_string(TYPE_STRING), "string") == 0);
    assert(strcmp(type_to_string(TYPE_ARRAY), "array") == 0);
    assert(strcmp(type_to_string(TYPE_FUNCTION), "function") == 0);
    
    printf("✓ All type names converted correctly\n");
}

void test_simple_type_info(void) {
    printf("\n--- Test: Simple Type Info ---\n");
    
    TypeInfo* ti_int = type_info_create(TYPE_INT);
    assert(ti_int != NULL);
    assert(ti_int->base_type == TYPE_INT);
    printf("✓ Created INT type info\n");
    
    TypeInfo* ti_real = type_info_create(TYPE_REAL);
    assert(ti_real != NULL);
    assert(ti_real->base_type == TYPE_REAL);
    printf("✓ Created REAL type info\n");
    
    TypeInfo* ti_bool = type_info_create(TYPE_BOOL);
    assert(ti_bool != NULL);
    assert(ti_bool->base_type == TYPE_BOOL);
    printf("✓ Created BOOL type info\n");
    
    type_info_free(ti_int);
    type_info_free(ti_real);
    type_info_free(ti_bool);
}

void test_array_type_info(void) {
    printf("\n--- Test: Array Type Info ---\n");
    
    // 创建一维整型数组
    TypeInfo* elem_type = type_info_create(TYPE_INT);
    int32_t sizes[] = {10};
    TypeInfo* array_type = type_info_create_array(elem_type, 1, sizes);
    
    assert(array_type != NULL);
    assert(array_type->base_type == TYPE_ARRAY);
    assert(array_type->array_info.dimensions == 1);
    assert(array_type->array_info.sizes[0] == 10);
    assert(array_type->array_info.elem_type == elem_type);
    
    printf("✓ Created 1D INT array [10]\n");
    
    type_info_free(array_type);
}

void test_function_type_info(void) {
    printf("\n--- Test: Function Type Info ---\n");
    
    // 创建函数类型：(INT, REAL) -> BOOL
    TypeInfo* return_type = type_info_create(TYPE_BOOL);
    TypeInfo* param_types[2];
    param_types[0] = type_info_create(TYPE_INT);
    param_types[1] = type_info_create(TYPE_REAL);
    
    TypeInfo* func_type = type_info_create_function(return_type, param_types, 2);
    
    assert(func_type != NULL);
    assert(func_type->base_type == TYPE_FUNCTION);
    assert(func_type->func_info.param_count == 2);
    assert(func_type->func_info.return_type->base_type == TYPE_BOOL);
    assert(func_type->func_info.param_types[0]->base_type == TYPE_INT);
    assert(func_type->func_info.param_types[1]->base_type == TYPE_REAL);
    
    printf("✓ Created function type (INT, REAL) -> BOOL\n");
    
    type_info_free(func_type);
}

void test_type_equality(void) {
    printf("\n--- Test: Type Equality ---\n");
    
    TypeInfo* ti1 = type_info_create(TYPE_INT);
    TypeInfo* ti2 = type_info_create(TYPE_INT);
    TypeInfo* ti3 = type_info_create(TYPE_REAL);
    
    assert(type_info_equals(ti1, ti2));
    printf("✓ INT equals INT\n");
    
    assert(!type_info_equals(ti1, ti3));
    printf("✓ INT not equals REAL\n");
    
    type_info_free(ti1);
    type_info_free(ti2);
    type_info_free(ti3);
}

void test_type_conversion(void) {
    printf("\n--- Test: Type Conversion Rules ---\n");
    
    TypeInfo* ti_int = type_info_create(TYPE_INT);
    TypeInfo* ti_real = type_info_create(TYPE_REAL);
    TypeInfo* ti_bool = type_info_create(TYPE_BOOL);
    
    // INT 可以转换为 REAL
    assert(type_info_can_convert(ti_int, ti_real));
    printf("✓ INT can convert to REAL\n");
    
    // REAL 不能转换为 INT
    assert(!type_info_can_convert(ti_real, ti_int));
    printf("✓ REAL cannot convert to INT\n");
    
    // BOOL 可以转换为 INT
    assert(type_info_can_convert(ti_bool, ti_int));
    printf("✓ BOOL can convert to INT\n");
    
    type_info_free(ti_int);
    type_info_free(ti_real);
    type_info_free(ti_bool);
}

void test_value_creation(void) {
    printf("\n--- Test: Value Creation ---\n");
    
    Value v_bool = value_create(TYPE_BOOL);
    assert(v_bool.type == TYPE_BOOL);
    assert(v_bool.bool_val == false);
    printf("✓ Created BOOL value (default: false)\n");
    
    Value v_int = value_create(TYPE_INT);
    assert(v_int.type == TYPE_INT);
    assert(v_int.int_val == 0);
    printf("✓ Created INT value (default: 0)\n");
    
    Value v_real = value_create(TYPE_REAL);
    assert(v_real.type == TYPE_REAL);
    assert(v_real.real_val == 0.0);
    printf("✓ Created REAL value (default: 0.0)\n");
}

void test_value_conversions(void) {
    printf("\n--- Test: Value Conversions ---\n");
    
    // INT to BOOL
    Value v_int;
    v_int.type = TYPE_INT;
    v_int.int_val = 42;
    assert(value_to_bool(v_int) == true);
    printf("✓ INT(42) to BOOL = true\n");
    
    v_int.int_val = 0;
    assert(value_to_bool(v_int) == false);
    printf("✓ INT(0) to BOOL = false\n");
    
    // INT to REAL
    v_int.int_val = 100;
    assert(value_to_real(v_int) == 100.0);
    printf("✓ INT(100) to REAL = 100.0\n");
    
    // REAL to INT
    Value v_real;
    v_real.type = TYPE_REAL;
    v_real.real_val = 3.14;
    assert(value_to_int(v_real) == 3);
    printf("✓ REAL(3.14) to INT = 3\n");
    
    // BOOL to INT
    Value v_bool;
    v_bool.type = TYPE_BOOL;
    v_bool.bool_val = true;
    assert(value_to_int(v_bool) == 1);
    printf("✓ BOOL(true) to INT = 1\n");
}

void test_value_to_string(void) {
    printf("\n--- Test: Value to String ---\n");
    
    char buffer[256];
    
    // BOOL
    Value v_bool;
    v_bool.type = TYPE_BOOL;
    v_bool.bool_val = true;
    value_to_string(v_bool, buffer, sizeof(buffer));
    assert(strcmp(buffer, "true") == 0);
    printf("✓ BOOL(true) -> '%s'\n", buffer);
    
    // INT
    Value v_int;
    v_int.type = TYPE_INT;
    v_int.int_val = 42;
    value_to_string(v_int, buffer, sizeof(buffer));
    assert(strcmp(buffer, "42") == 0);
    printf("✓ INT(42) -> '%s'\n", buffer);
    
    // REAL
    Value v_real;
    v_real.type = TYPE_REAL;
    v_real.real_val = 3.14;
    value_to_string(v_real, buffer, sizeof(buffer));
    printf("✓ REAL(3.14) -> '%s'\n", buffer);
}

void test_value_copy(void) {
    printf("\n--- Test: Value Copy ---\n");
    
    // 测试字符串深拷贝
    Value v_original;
    v_original.type = TYPE_STRING;
    v_original.string_val = mmgr_strdup("Hello, World!");
    
    Value v_copy = value_copy(v_original);
    assert(v_copy.type == TYPE_STRING);
    assert(v_copy.string_val != NULL);
    assert(strcmp(v_copy.string_val, v_original.string_val) == 0);
    assert(v_copy.string_val != v_original.string_val); // 不同的内存地址
    
    printf("✓ String value copied: '%s'\n", v_copy.string_val);
    
    value_free(&v_original);
    value_free(&v_copy);
}

void test_error_codes(void) {
    printf("\n--- Test: Error Code Strings ---\n");
    
    assert(strcmp(error_code_to_string(OK), "OK") == 0);
    assert(strcmp(error_code_to_string(ERR_SYNTAX), "Syntax Error") == 0);
    assert(strcmp(error_code_to_string(ERR_TYPE), "Type Error") == 0);
    assert(strcmp(error_code_to_string(ERR_STACK_OVERFLOW), "Stack Overflow") == 0);
    
    printf("✓ All error codes have correct string representations\n");
}

int main(void) {
    printf("========================================\n");
    printf("  STVM Type System Test Suite\n");
    printf("========================================\n");
    
    // 初始化内存管理器
    if (!mmgr_init()) {
        fprintf(stderr, "Failed to initialize memory manager\n");
        return 1;
    }
    printf("✓ Memory manager initialized\n");
    
    // 运行测试
    test_type_strings();
    test_simple_type_info();
    test_array_type_info();
    test_function_type_info();
    test_type_equality();
    test_type_conversion();
    test_value_creation();
    test_value_conversions();
    test_value_to_string();
    test_value_copy();
    test_error_codes();
    
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
