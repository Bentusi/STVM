/**
 * @file test_force.c
 * @brief 变量强制功能单元测试
 */

#include "force.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define TEST(name) printf("\n=== Test: %s ===\n", name)
#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAILED: %s at line %d\n", #cond, __LINE__); \
        return false; \
    } \
} while(0)

#define PASS() do { printf("PASSED\n"); return true; } while(0)

// 测试1：创建和销毁
bool test_create_destroy() {
    TEST("Create and Destroy");
    
    ForceManager* mgr = force_manager_create();
    ASSERT(mgr != NULL);
    ASSERT(mgr->force_count == 0);
    ASSERT(mgr->force_enabled == true);
    ASSERT(mgr->force_locked == false);
    
    force_manager_destroy(mgr);
    
    PASS();
}

// 测试2：强制变量
bool test_force_variable() {
    TEST("Force Variable");
    
    ForceManager* mgr = force_manager_create();
    
    Value temp_value = {.type = TYPE_REAL, .real_val = 100.0};
    bool result = force_variable(mgr, "temperature", temp_value, false);
    
    ASSERT(result == true);
    ASSERT(mgr->force_count == 1);
    ASSERT(force_is_forced(mgr, "temperature") == true);
    
    Value* forced = force_get_forced_value(mgr, "temperature");
    ASSERT(forced != NULL);
    ASSERT(forced->type == TYPE_REAL);
    ASSERT(forced->real_val == 100.0);
    
    force_manager_destroy(mgr);
    
    PASS();
}

// 测试3：取消强制
bool test_unforce_variable() {
    TEST("Unforce Variable");
    
    ForceManager* mgr = force_manager_create();
    
    Value value = {.type = TYPE_INT, .int_val = 42};
    force_variable(mgr, "counter", value, false);
    
    ASSERT(force_is_forced(mgr, "counter") == true);
    
    bool result = unforce_variable(mgr, "counter");
    ASSERT(result == true);
    ASSERT(force_is_forced(mgr, "counter") == false);
    ASSERT(mgr->force_count == 0);
    
    force_manager_destroy(mgr);
    
    PASS();
}

// 测试4：多个强制
bool test_multiple_forces() {
    TEST("Multiple Forces");
    
    ForceManager* mgr = force_manager_create();
    
    Value v1 = {.type = TYPE_INT, .int_val = 10};
    Value v2 = {.type = TYPE_REAL, .real_val = 25.5};
    Value v3 = {.type = TYPE_BOOL, .bool_val = true};
    
    force_variable(mgr, "var1", v1, false);
    force_variable(mgr, "var2", v2, false);
    force_variable(mgr, "var3", v3, false);
    
    ASSERT(mgr->force_count == 3);
    ASSERT(force_is_forced(mgr, "var1") == true);
    ASSERT(force_is_forced(mgr, "var2") == true);
    ASSERT(force_is_forced(mgr, "var3") == true);
    
    force_manager_destroy(mgr);
    
    PASS();
}

// 测试5：取消所有强制
bool test_unforce_all() {
    TEST("Unforce All");
    
    ForceManager* mgr = force_manager_create();
    
    Value v = {.type = TYPE_INT, .int_val = 1};
    force_variable(mgr, "var1", v, false);
    force_variable(mgr, "var2", v, false);
    force_variable(mgr, "var3", v, false);
    
    ASSERT(mgr->force_count == 3);
    
    int32_t count = unforce_all(mgr);
    ASSERT(count == 3);
    ASSERT(mgr->force_count == 0);
    
    force_manager_destroy(mgr);
    
    PASS();
}

// 测试6：通过索引强制
bool test_force_by_index() {
    TEST("Force by Index");
    
    ForceManager* mgr = force_manager_create();
    
    Value value = {.type = TYPE_INT, .int_val = 99};
    bool result = force_variable_by_index(mgr, 5, value, false);
    
    ASSERT(result == true);
    ASSERT(force_is_forced_by_index(mgr, 5) == true);
    
    ForceVariable* fv = force_find_by_index(mgr, 5);
    ASSERT(fv != NULL);
    ASSERT(fv->var_index == 5);
    ASSERT(fv->forced_value.int_val == 99);
    
    force_manager_destroy(mgr);
    
    PASS();
}

// 测试7：更新强制值
bool test_update_force() {
    TEST("Update Force Value");
    
    ForceManager* mgr = force_manager_create();
    
    Value v1 = {.type = TYPE_REAL, .real_val = 10.0};
    force_variable(mgr, "temp", v1, false);
    
    Value* forced = force_get_forced_value(mgr, "temp");
    ASSERT(forced->real_val == 10.0);
    
    Value v2 = {.type = TYPE_REAL, .real_val = 20.0};
    bool result = force_update_value(mgr, "temp", v2);
    ASSERT(result == true);
    
    forced = force_get_forced_value(mgr, "temp");
    ASSERT(forced->real_val == 20.0);
    
    force_manager_destroy(mgr);
    
    PASS();
}

// 测试8：使能/禁用
bool test_enable_disable() {
    TEST("Enable/Disable Force");
    
    ForceManager* mgr = force_manager_create();
    
    Value value = {.type = TYPE_INT, .int_val = 42};
    force_variable(mgr, "var", value, false);
    
    ASSERT(force_is_enabled(mgr) == true);
    ASSERT(force_is_forced(mgr, "var") == true);
    
    // 禁用强制
    force_enable(mgr, false);
    ASSERT(force_is_enabled(mgr) == false);
    ASSERT(force_is_forced(mgr, "var") == false);  // 禁用后不再强制
    
    // 重新启用
    force_enable(mgr, true);
    ASSERT(force_is_forced(mgr, "var") == true);
    
    force_manager_destroy(mgr);
    
    PASS();
}

// 测试9：锁定/解锁
bool test_lock_unlock() {
    TEST("Lock/Unlock Force Manager");
    
    ForceManager* mgr = force_manager_create();
    
    Value value = {.type = TYPE_INT, .int_val = 1};
    
    // 正常添加强制
    ASSERT(force_variable(mgr, "var1", value, false) == true);
    
    // 锁定
    force_lock(mgr, true);
    ASSERT(force_is_locked(mgr) == true);
    
    // 锁定后无法添加
    ASSERT(force_variable(mgr, "var2", value, false) == false);
    
    // 锁定后无法取消
    ASSERT(unforce_variable(mgr, "var1") == false);
    
    // 解锁
    force_lock(mgr, false);
    ASSERT(force_is_locked(mgr) == false);
    
    // 解锁后可以操作
    ASSERT(force_variable(mgr, "var2", value, false) == true);
    ASSERT(unforce_variable(mgr, "var1") == true);
    
    force_manager_destroy(mgr);
    
    PASS();
}

// 测试10：最大强制数限制
bool test_max_forces() {
    TEST("Max Forces Limit");
    
    ForceManager* mgr = force_manager_create();
    force_set_max_forces(mgr, 3);
    
    Value value = {.type = TYPE_INT, .int_val = 1};
    
    ASSERT(force_variable(mgr, "var1", value, false) == true);
    ASSERT(force_variable(mgr, "var2", value, false) == true);
    ASSERT(force_variable(mgr, "var3", value, false) == true);
    
    // 超过限制
    ASSERT(force_variable(mgr, "var4", value, false) == false);
    ASSERT(mgr->force_count == 3);
    
    force_manager_destroy(mgr);
    
    PASS();
}

// 测试11：打印状态
bool test_print_status() {
    TEST("Print Status");
    
    ForceManager* mgr = force_manager_create();
    
    Value v1 = {.type = TYPE_INT, .int_val = 42};
    Value v2 = {.type = TYPE_REAL, .real_val = 3.14};
    Value v3 = {.type = TYPE_BOOL, .bool_val = true};
    
    force_variable(mgr, "counter", v1, false);
    force_variable(mgr, "temperature", v2, true);
    force_variable(mgr, "alarm", v3, false);
    
    force_print_status(mgr);
    
    force_manager_destroy(mgr);
    
    PASS();
}

// 主测试函数
int main() {
    printf("\n");
    printf("*******************************************\n");
    printf("*   Force Manager Unit Tests             *\n");
    printf("*******************************************\n");
    
    int passed = 0;
    int total = 0;
    
    #define RUN_TEST(test) do { \
        total++; \
        if (test()) passed++; \
    } while(0)
    
    RUN_TEST(test_create_destroy);
    RUN_TEST(test_force_variable);
    RUN_TEST(test_unforce_variable);
    RUN_TEST(test_multiple_forces);
    RUN_TEST(test_unforce_all);
    RUN_TEST(test_force_by_index);
    RUN_TEST(test_update_force);
    RUN_TEST(test_enable_disable);
    RUN_TEST(test_lock_unlock);
    RUN_TEST(test_max_forces);
    RUN_TEST(test_print_status);
    
    printf("\n");
    printf("*******************************************\n");
    printf("*   Test Results: %d/%d passed           *\n", passed, total);
    printf("*******************************************\n");
    printf("\n");
    
    return (passed == total) ? 0 : 1;
}
