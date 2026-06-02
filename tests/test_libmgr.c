/**
 * @file test_libmgr.c
 * @brief 库管理器测试
 */

#include "libmgr.h"
#include "bytecode.h"
#include "bytecode_io.h"
#include "mmgr.h"
#include <stdio.h>
#include <assert.h>

/**
 * @brief 创建一个简单的测试库
 */
static BytecodeModule* create_test_library() {
    BytecodeModule* module = bytecode_module_create();
    
    // 添加一些常量
    bytecode_add_int_constant(module, 42);
    bytecode_add_real_constant(module, 3.14);
    
    // 添加一些函数
    TypeInfo* int_type = type_info_create(TYPE_INT);
    TypeInfo* real_type = type_info_create(TYPE_REAL);
    
    DataType int_params[] = {TYPE_INT, TYPE_INT};
    DataType real_param[] = {TYPE_REAL};
    
    bytecode_add_function(module, "add", 0, 2, 0, TYPE_INT, int_params);
    bytecode_add_function(module, "multiply", 10, 2, 0, TYPE_INT, int_params);
    bytecode_add_function(module, "sqrt", 20, 1, 0, TYPE_REAL, real_param);
    
    type_info_free(int_type);
    type_info_free(real_type);
    
    // 添加一些简单的指令（占位）
    bytecode_add_instruction(module, OP_HALT, 0, 0);
    
    return module;
}

void test_library_creation() {
    printf("\n--- Test: Library Creation ---\n");
    
    // 创建测试库并保存
    BytecodeModule* module = create_test_library();
    
    ErrorCode err = bytecode_save(module, "test_lib.stbc");
    assert(err == OK);
    
    printf("✓ Created test library: test_lib.stbc\n");
    printf("  Functions: %u\n", module->function_count);
    printf("  Constants: %u\n", module->const_count);
    
    bytecode_module_free(module);
}

void test_library_loading() {
    printf("\n--- Test: Library Loading ---\n");
    
    SymbolTable* global_symtbl = symtbl_init();
    LibraryManager* mgr = libmgr_create(global_symtbl);
    
    // 加载测试库
    ErrorCode err = libmgr_load_library(mgr, "test_lib.stbc");
    assert(err == OK);
    
    // 验证库已加载
    LoadedLibrary* lib = libmgr_find_library(mgr, "test_lib");
    assert(lib != NULL);
    assert(lib->module != NULL);
    assert(lib->module->function_count == 3);
    
    printf("✓ Library loaded successfully\n");
    printf("  Name: %s\n", lib->name);
    printf("  Functions: %u\n", lib->module->function_count);
    
    libmgr_dump_libraries(mgr);
    
    libmgr_free(mgr);
    symtbl_free(global_symtbl);
}

void test_symbol_import() {
    printf("\n--- Test: Symbol Import ---\n");
    
    SymbolTable* global_symtbl = symtbl_init();
    LibraryManager* mgr = libmgr_create(global_symtbl);
    
    // 加载库
    ErrorCode err = libmgr_load_library(mgr, "test_lib.stbc");
    assert(err == OK);
    
    // 导入符号
    err = libmgr_import_symbol(mgr, "test_lib", "add", NULL);
    assert(err == OK);
    
    err = libmgr_import_symbol(mgr, "test_lib", "multiply", "mul");
    assert(err == OK);
    
    // 验证导入
    ImportedSymbol* sym1 = libmgr_find_import(mgr, "add");
    assert(sym1 != NULL);
    assert(strcmp(sym1->original_name, "add") == 0);
    
    ImportedSymbol* sym2 = libmgr_find_import(mgr, "mul");
    assert(sym2 != NULL);
    assert(strcmp(sym2->original_name, "multiply") == 0);
    
    printf("✓ Symbols imported successfully\n");
    libmgr_dump_imports(mgr);
    
    libmgr_free(mgr);
    symtbl_free(global_symtbl);
}

void test_qualified_name_resolution() {
    printf("\n--- Test: Qualified Name Resolution ---\n");
    
    SymbolTable* global_symtbl = symtbl_init();
    LibraryManager* mgr = libmgr_create(global_symtbl);
    
    // 加载库
    ErrorCode err = libmgr_load_library(mgr, "test_lib.stbc");
    assert(err == OK);
    
    // 解析限定名
    Symbol* sym = libmgr_resolve_symbol(mgr, "test_lib.add");
    assert(sym != NULL);
    
    printf("✓ Resolved: test_lib.add\n");
    printf("  Symbol kind: %d\n", sym->kind);
    
    libmgr_free(mgr);
    symtbl_free(global_symtbl);
}

void test_library_unload() {
    printf("\n--- Test: Library Unload ---\n");
    
    SymbolTable* global_symtbl = symtbl_init();
    LibraryManager* mgr = libmgr_create(global_symtbl);
    
    // 加载库
    ErrorCode err = libmgr_load_library(mgr, "test_lib.stbc");
    assert(err == OK);
    
    // 导入符号
    err = libmgr_import_symbol(mgr, "test_lib", "add", NULL);
    assert(err == OK);
    
    // 卸载库
    err = libmgr_unload_library(mgr, "test_lib");
    assert(err == OK);
    
    // 验证库已卸载
    LoadedLibrary* lib = libmgr_find_library(mgr, "test_lib");
    assert(lib == NULL);
    
    // 验证导入也已移除
    ImportedSymbol* sym = libmgr_find_import(mgr, "add");
    assert(sym == NULL);
    
    printf("✓ Library unloaded successfully\n");
    
    libmgr_free(mgr);
    symtbl_free(global_symtbl);
}

void test_search_paths() {
    printf("\n--- Test: Search Paths ---\n");
    
    SymbolTable* global_symtbl = symtbl_init();
    LibraryManager* mgr = libmgr_create(global_symtbl);
    
    // 添加搜索路径
    bool ok = libmgr_add_search_path(mgr, "./testlibs");
    assert(ok);
    
    ok = libmgr_add_search_path(mgr, "/usr/lib/stvm");
    assert(ok);
    
    printf("✓ Search paths added\n");
    printf("  Total search paths: %d\n", mgr->search_path_count);
    
    libmgr_free(mgr);
    symtbl_free(global_symtbl);
}

void test_duplicate_import() {
    printf("\n--- Test: Duplicate Import Detection ---\n");
    
    SymbolTable* global_symtbl = symtbl_init();
    LibraryManager* mgr = libmgr_create(global_symtbl);
    
    // 加载库
    ErrorCode err = libmgr_load_library(mgr, "test_lib.stbc");
    assert(err == OK);
    
    // 第一次导入
    err = libmgr_import_symbol(mgr, "test_lib", "add", NULL);
    assert(err == OK);
    
    // 尝试重复导入（应该失败）
    err = libmgr_import_symbol(mgr, "test_lib", "add", NULL);
    assert(err != OK);
    
    printf("✓ Duplicate import correctly rejected\n");
    
    libmgr_free(mgr);
    symtbl_free(global_symtbl);
}

int main() {
    printf("========================================\n");
    printf("  STVM Library Manager Test Suite\n");
    printf("========================================\n");
    
    mmgr_init();
    printf("✓ Memory manager initialized\n");
    
    test_library_creation();
    test_library_loading();
    test_symbol_import();
    test_qualified_name_resolution();
    test_library_unload();
    test_search_paths();
    test_duplicate_import();
    
    // 清理测试文件
    remove("test_lib.stbc");
    
    mmgr_print_stats();
    mmgr_cleanup();
    
    printf("\n========================================\n");
    printf("  All Library Manager Tests Passed! ✓\n");
    printf("========================================\n");
    
    return 0;
}
