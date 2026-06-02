/**
 * @file test_mmgr.c
 * @brief 内存管理器测试程序
 */

#include "mmgr.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

void test_basic_alloc_free(void) {
    printf("\n--- Test: Basic Allocation and Free ---\n");
    
    // 测试基本分配
    void* ptr1 = mmgr_alloc(32);
    assert(ptr1 != NULL);
    printf("✓ Allocated 32 bytes\n");
    
    void* ptr2 = mmgr_alloc(128);
    assert(ptr2 != NULL);
    printf("✓ Allocated 128 bytes\n");
    
    void* ptr3 = mmgr_alloc(1024);
    assert(ptr3 != NULL);
    printf("✓ Allocated 1024 bytes\n");
    
    // 测试释放
    mmgr_free(ptr1);
    printf("✓ Freed 32 bytes\n");
    
    mmgr_free(ptr2);
    printf("✓ Freed 128 bytes\n");
    
    mmgr_free(ptr3);
    printf("✓ Freed 1024 bytes\n");
}

void test_calloc(void) {
    printf("\n--- Test: Calloc (Zero-initialized Allocation) ---\n");
    
    size_t size = 64;
    char* ptr = (char*)mmgr_calloc(size);
    assert(ptr != NULL);
    
    // 检查是否全部清零
    bool all_zero = true;
    for (size_t i = 0; i < size; i++) {
        if (ptr[i] != 0) {
            all_zero = false;
            break;
        }
    }
    
    assert(all_zero);
    printf("✓ Allocated %zu bytes, all initialized to zero\n", size);
    
    mmgr_free(ptr);
}

void test_realloc(void) {
    printf("\n--- Test: Realloc ---\n");
    
    // 初始分配
    char* ptr = (char*)mmgr_alloc(32);
    assert(ptr != NULL);
    strcpy(ptr, "Hello, World!");
    printf("✓ Initial allocation: '%s'\n", ptr);
    
    // 扩展内存
    ptr = (char*)mmgr_realloc(ptr, 64);
    assert(ptr != NULL);
    assert(strcmp(ptr, "Hello, World!") == 0);
    printf("✓ Reallocated to 64 bytes, data preserved: '%s'\n", ptr);
    
    mmgr_free(ptr);
}

void test_strdup(void) {
    printf("\n--- Test: String Duplication ---\n");
    
    const char* original = "Test String 12345";
    char* duplicate = mmgr_strdup(original);
    
    assert(duplicate != NULL);
    assert(strcmp(original, duplicate) == 0);
    assert(duplicate != original); // 确保是新内存
    
    printf("✓ Original: '%s'\n", original);
    printf("✓ Duplicate: '%s'\n", duplicate);
    
    mmgr_free(duplicate);
}

void test_pool_allocation(void) {
    printf("\n--- Test: Pool-specific Allocation ---\n");
    
    // 从不同池分配
    void* ptr_ast = mmgr_alloc_from_pool(POOL_AST, 100);
    assert(ptr_ast != NULL);
    printf("✓ Allocated from AST pool\n");
    
    void* ptr_symbol = mmgr_alloc_from_pool(POOL_SYMBOL, 200);
    assert(ptr_symbol != NULL);
    printf("✓ Allocated from Symbol pool\n");
    
    void* ptr_string = mmgr_alloc_from_pool(POOL_STRING, 50);
    assert(ptr_string != NULL);
    printf("✓ Allocated from String pool\n");
    
    mmgr_free(ptr_ast);
    mmgr_free(ptr_symbol);
    mmgr_free(ptr_string);
}

void test_stress(void) {
    printf("\n--- Test: Stress Test (Multiple Allocations) ---\n");
    
    #define NUM_ALLOCS 100
    void* ptrs[NUM_ALLOCS];
    
    // 分配
    for (int i = 0; i < NUM_ALLOCS; i++) {
        size_t size = (i % 10 + 1) * 16; // 16, 32, 48, ..., 160
        ptrs[i] = mmgr_alloc(size);
        assert(ptrs[i] != NULL);
    }
    printf("✓ Allocated %d blocks\n", NUM_ALLOCS);
    
    // 释放
    for (int i = 0; i < NUM_ALLOCS; i++) {
        mmgr_free(ptrs[i]);
    }
    printf("✓ Freed %d blocks\n", NUM_ALLOCS);
}

void test_pool_reset(void) {
    printf("\n--- Test: Pool Reset ---\n");
    
    // 分配一些内存并保存指针
    void* ptrs[10];
    for (int i = 0; i < 10; i++) {
        ptrs[i] = mmgr_alloc_from_pool(POOL_SMALL, 32);
    }
    printf("✓ Allocated 10 blocks from SMALL pool\n");
    
    // 先正常释放所有内存
    for (int i = 0; i < 10; i++) {
        mmgr_free(ptrs[i]);
    }
    
    // 重置池（现在池已经是空的了）
    mmgr_reset_pool(POOL_SMALL);
    printf("✓ Reset SMALL pool\n");
    
    // 再次分配应该成功
    void* ptr = mmgr_alloc_from_pool(POOL_SMALL, 32);
    assert(ptr != NULL);
    printf("✓ Successfully allocated after reset\n");
    
    mmgr_free(ptr);
}

int main(void) {
    printf("========================================\n");
    printf("  STVM Memory Manager Test Suite\n");
    printf("========================================\n");
    
    // 初始化内存管理器
    if (!mmgr_init()) {
        fprintf(stderr, "Failed to initialize memory manager\n");
        return 1;
    }
    printf("✓ Memory manager initialized\n");
    
    // 运行测试
    test_basic_alloc_free();
    test_calloc();
    test_realloc();
    test_strdup();
    test_pool_allocation();
    test_stress();
    test_pool_reset();
    
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
