/**
 * @file test_symtbl.c
 * @brief 符号表测试程序
 */

#include "symtbl.h"
#include "mmgr.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_symtbl_init(void) {
    printf("\n--- Test: Symbol Table Initialization ---\n");
    
    SymbolTable* symtbl = symtbl_init();
    assert(symtbl != NULL);
    assert(symtbl->global_scope != NULL);
    assert(symtbl->current_scope == symtbl->global_scope);
    assert(symtbl->current_level == 0);
    assert(symtbl->global_var_count == 0);
    printf("✓ Symbol table initialized\n");
    
    symtbl_free(symtbl);
    printf("✓ Symbol table freed\n");
}

void test_define_variables(void) {
    printf("\n--- Test: Define Variables ---\n");
    
    SymbolTable* symtbl = symtbl_init();
    
    // 定义全局变量
    TypeInfo* int_type = type_info_create(TYPE_INT);
    Symbol* var1 = symtbl_define_variable(symtbl, "count", int_type, false);
    assert(var1 != NULL);
    assert(strcmp(var1->name, "count") == 0);
    assert(var1->kind == SYM_VARIABLE);
    assert(var1->is_global == true);
    assert(var1->index == 0);
    printf("✓ Defined global variable: count [global #%d]\n", var1->index);
    
    // 定义另一个全局变量
    TypeInfo* real_type = type_info_create(TYPE_REAL);
    Symbol* var2 = symtbl_define_variable(symtbl, "temperature", real_type, false);
    assert(var2 != NULL);
    assert(var2->is_global == true);
    assert(var2->index == 1);
    printf("✓ Defined global variable: temperature [global #%d]\n", var2->index);
    
    // 定义常量
    Value const_val = value_create(TYPE_INT);
    const_val.int_val = 100;
    Symbol* const_sym = symtbl_define_constant(symtbl, "MAX_SIZE", const_val);
    assert(const_sym != NULL);
    assert(const_sym->kind == SYM_CONSTANT);
    assert(const_sym->const_value.int_val == 100);
    printf("✓ Defined constant: MAX_SIZE = 100\n");
    
    // 尝试重复定义（应该失败）
    TypeInfo* dup_type = type_info_create(TYPE_INT);
    Symbol* dup = symtbl_define_variable(symtbl, "count", dup_type, false);
    assert(dup == NULL);
    printf("✓ Duplicate variable definition rejected\n");
    type_info_free(dup_type);
    
    symtbl_free(symtbl);
}

void test_scope_management(void) {
    printf("\n--- Test: Scope Management ---\n");
    
    SymbolTable* symtbl = symtbl_init();
    
    // 全局作用域
    assert(symtbl_current_level(symtbl) == 0);
    printf("✓ Current level: %d (global)\n", symtbl_current_level(symtbl));
    
    // 进入新作用域
    symtbl_enter_scope(symtbl);
    assert(symtbl_current_level(symtbl) == 1);
    printf("✓ Entered scope, level: %d\n", symtbl_current_level(symtbl));
    
    // 再进入一个作用域
    symtbl_enter_scope(symtbl);
    assert(symtbl_current_level(symtbl) == 2);
    printf("✓ Entered scope, level: %d\n", symtbl_current_level(symtbl));
    
    // 离开作用域
    symtbl_leave_scope(symtbl);
    assert(symtbl_current_level(symtbl) == 1);
    printf("✓ Left scope, level: %d\n", symtbl_current_level(symtbl));
    
    symtbl_leave_scope(symtbl);
    assert(symtbl_current_level(symtbl) == 0);
    printf("✓ Left scope, level: %d\n", symtbl_current_level(symtbl));
    
    symtbl_free(symtbl);
}

void test_local_variables(void) {
    printf("\n--- Test: Local Variables ---\n");
    
    SymbolTable* symtbl = symtbl_init();
    
    // 定义全局变量
    TypeInfo* global_type = type_info_create(TYPE_INT);
    Symbol* global_var = symtbl_define_variable(symtbl, "global_x", global_type, false);
    assert(global_var->is_global == true);
    printf("✓ Defined global variable: global_x\n");
    
    // 进入函数作用域
    symtbl_enter_scope(symtbl);
    symtbl_reset_local_offset(symtbl);
    
    // 定义局部变量
    TypeInfo* local_type = type_info_create(TYPE_INT);
    Symbol* local_var = symtbl_define_variable(symtbl, "local_y", local_type, false);
    assert(local_var != NULL);
    assert(local_var->is_global == false);
    assert(local_var->offset == 0);
    printf("✓ Defined local variable: local_y [local @%d]\n", local_var->offset);
    
    // 定义另一个局部变量
    TypeInfo* local_type2 = type_info_create(TYPE_REAL);
    Symbol* local_var2 = symtbl_define_variable(symtbl, "local_z", local_type2, false);
    assert(local_var2->offset == 1);
    printf("✓ Defined local variable: local_z [local @%d]\n", local_var2->offset);
    
    symtbl_leave_scope(symtbl);
    symtbl_free(symtbl);
}

void test_symbol_lookup(void) {
    printf("\n--- Test: Symbol Lookup ---\n");
    
    SymbolTable* symtbl = symtbl_init();
    
    // 定义全局变量
    TypeInfo* type1 = type_info_create(TYPE_INT);
    symtbl_define_variable(symtbl, "global_var", type1, false);
    
    // 在全局作用域查找
    Symbol* found = symtbl_lookup(symtbl, "global_var");
    assert(found != NULL);
    assert(strcmp(found->name, "global_var") == 0);
    printf("✓ Found global variable: %s\n", found->name);
    
    // 进入新作用域
    symtbl_enter_scope(symtbl);
    
    // 定义局部变量
    TypeInfo* type2 = type_info_create(TYPE_INT);
    symtbl_define_variable(symtbl, "local_var", type2, false);
    
    // 查找局部变量
    Symbol* local_found = symtbl_lookup(symtbl, "local_var");
    assert(local_found != NULL);
    assert(strcmp(local_found->name, "local_var") == 0);
    printf("✓ Found local variable: %s\n", local_found->name);
    
    // 从局部作用域查找全局变量（应该能找到）
    Symbol* global_from_local = symtbl_lookup(symtbl, "global_var");
    assert(global_from_local != NULL);
    printf("✓ Found global variable from local scope: %s\n", global_from_local->name);
    
    // 查找不存在的变量
    Symbol* not_found = symtbl_lookup(symtbl, "nonexistent");
    assert(not_found == NULL);
    printf("✓ Non-existent variable returned NULL\n");
    
    symtbl_leave_scope(symtbl);
    symtbl_free(symtbl);
}

void test_define_functions(void) {
    printf("\n--- Test: Define Functions ---\n");
    
    SymbolTable* symtbl = symtbl_init();
    
    // 定义函数：INT add(INT a, INT b)
    TypeInfo* return_type = type_info_create(TYPE_INT);
    TypeInfo* param_types[2];
    param_types[0] = type_info_create(TYPE_INT);
    param_types[1] = type_info_create(TYPE_INT);
    
    Symbol* func = symtbl_define_function(symtbl, "add", return_type, param_types, 2);
    assert(func != NULL);
    assert(func->kind == SYM_FUNCTION);
    assert(strcmp(func->name, "add") == 0);
    assert(func->param_count == 2);
    printf("✓ Defined function: add(INT, INT) -> INT\n");
    
    // 查找函数
    Symbol* found_func = symtbl_lookup(symtbl, "add");
    assert(found_func != NULL);
    assert(found_func == func);
    printf("✓ Found function: %s\n", found_func->name);
    
    symtbl_free(symtbl);
}

void test_function_parameters(void) {
    printf("\n--- Test: Function Parameters ---\n");
    
    SymbolTable* symtbl = symtbl_init();
    
    // 进入函数作用域
    symtbl_enter_scope(symtbl);
    symtbl_reset_local_offset(symtbl);
    
    // 定义参数
    TypeInfo* param1_type = type_info_create(TYPE_INT);
    Symbol* param1 = symtbl_define_parameter(symtbl, "a", param1_type);
    assert(param1 != NULL);
    assert(param1->kind == SYM_PARAMETER);
    assert(param1->offset == 0);
    printf("✓ Defined parameter: a [param @%d]\n", param1->offset);
    
    TypeInfo* param2_type = type_info_create(TYPE_INT);
    Symbol* param2 = symtbl_define_parameter(symtbl, "b", param2_type);
    assert(param2->offset == 1);
    printf("✓ Defined parameter: b [param @%d]\n", param2->offset);
    
    // 定义局部变量（应该接着参数的偏移）
    TypeInfo* local_type = type_info_create(TYPE_INT);
    Symbol* local = symtbl_define_variable(symtbl, "temp", local_type, false);
    assert(local->offset == 2);
    printf("✓ Defined local variable: temp [local @%d]\n", local->offset);
    
    symtbl_leave_scope(symtbl);
    symtbl_free(symtbl);
}

void test_qualified_names(void) {
    printf("\n--- Test: Qualified Names ---\n");
    
    // 创建限定名
    char* qualified = symtbl_make_qualified_name("math", "sqrt");
    assert(qualified != NULL);
    assert(strcmp(qualified, "math.sqrt") == 0);
    printf("✓ Created qualified name: %s\n", qualified);
    
    // 解析限定名
    char module[64], symbol[64];
    bool parsed = symtbl_parse_qualified_name(qualified, module, symbol, 64);
    assert(parsed == true);
    assert(strcmp(module, "math") == 0);
    assert(strcmp(symbol, "sqrt") == 0);
    printf("✓ Parsed qualified name: module='%s', symbol='%s'\n", module, symbol);
    
    mmgr_free(qualified);
    
    // 尝试解析非限定名
    bool not_qualified = symtbl_parse_qualified_name("simple", module, symbol, 64);
    assert(not_qualified == false);
    printf("✓ Non-qualified name correctly rejected\n");
}

void test_library_symbols(void) {
    printf("\n--- Test: Library Symbols ---\n");
    
    SymbolTable* symtbl = symtbl_init();
    
    // 创建库符号
    TypeInfo* func_type = type_info_create_function(
        type_info_create(TYPE_REAL),
        NULL, 0
    );
    Symbol* lib_sym = (Symbol*)mmgr_alloc_from_pool(POOL_SYMBOL, sizeof(Symbol));
    memset(lib_sym, 0, sizeof(Symbol));
    lib_sym->name = mmgr_strdup("sqrt");
    lib_sym->kind = SYM_FUNCTION;
    lib_sym->type = func_type;
    
    // 注册库符号
    bool registered = symtbl_register_library_symbol(symtbl, "math", lib_sym);
    assert(registered == true);
    assert(lib_sym->is_library == true);
    assert(strcmp(lib_sym->library_name, "math") == 0);
    assert(lib_sym->qualified_name != NULL);
    assert(strcmp(lib_sym->qualified_name, "math.sqrt") == 0);
    printf("✓ Registered library symbol: %s\n", lib_sym->qualified_name);
    
    // 查找库符号（使用限定名）
    Symbol* found = symtbl_lookup(symtbl, "math.sqrt");
    assert(found != NULL);
    assert(found == lib_sym);
    printf("✓ Found library symbol: %s\n", found->qualified_name);
    
    symtbl_free(symtbl);
}

void test_symbol_print(void) {
    printf("\n--- Test: Symbol Table Print ---\n");
    
    SymbolTable* symtbl = symtbl_init();
    
    // 添加一些符号
    TypeInfo* type1 = type_info_create(TYPE_INT);
    symtbl_define_variable(symtbl, "x", type1, false);
    
    TypeInfo* type2 = type_info_create(TYPE_REAL);
    symtbl_define_variable(symtbl, "y", type2, false);
    
    TypeInfo* ret_type = type_info_create(TYPE_INT);
    TypeInfo* param_types[1];
    param_types[0] = type_info_create(TYPE_INT);
    symtbl_define_function(symtbl, "square", ret_type, param_types, 1);
    
    // 打印符号表
    symtbl_print(symtbl);
    
    symtbl_free(symtbl);
}

int main(void) {
    printf("========================================\n");
    printf("  STVM Symbol Table Test Suite\n");
    printf("========================================\n");
    
    // 初始化内存管理器
    if (!mmgr_init()) {
        fprintf(stderr, "Failed to initialize memory manager\n");
        return 1;
    }
    printf("✓ Memory manager initialized\n");
    
    // 运行测试
    test_symtbl_init();
    test_define_variables();
    test_scope_management();
    test_local_variables();
    test_symbol_lookup();
    test_define_functions();
    test_function_parameters();
    test_qualified_names();
    test_library_symbols();
    test_symbol_print();
    
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
