/**
 * @file test_force_file.c
 * @brief Force文件加载/保存功能测试
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

// 测试1：保存和加载
bool test_save_and_load() {
    TEST("Save and Load");
    
    ForceManager* mgr = force_manager_create();
    ASSERT(mgr != NULL);
    
    // 添加几个强制变量
    Value v1 = {.type = TYPE_INT, .int_val = 100};
    Value v2 = {.type = TYPE_REAL, .real_val = 25.5};
    Value v3 = {.type = TYPE_BOOL, .bool_val = true};
    
    force_variable(mgr, "counter", v1, false);
    force_variable(mgr, "temperature", v2, true);
    force_variable(mgr, "alarm", v3, false);
    
    ASSERT(mgr->force_count == 3);
    
    // 保存到文件
    const char* filename = "/tmp/test_force_save.force";
    bool save_result = force_save_to_file(mgr, filename);
    ASSERT(save_result == true);
    
    // 清空当前强制
    unforce_all(mgr);
    ASSERT(mgr->force_count == 0);
    
    // 从文件加载
    bool load_result = force_load_from_file(mgr, filename);
    ASSERT(load_result == true);
    ASSERT(mgr->force_count == 3);
    
    // 验证加载的数据
    ASSERT(force_is_forced(mgr, "counter") == true);
    ASSERT(force_is_forced(mgr, "temperature") == true);
    ASSERT(force_is_forced(mgr, "alarm") == true);
    
    Value* loaded_counter = force_get_forced_value(mgr, "counter");
    ASSERT(loaded_counter != NULL);
    ASSERT(loaded_counter->type == TYPE_INT);
    ASSERT(loaded_counter->int_val == 100);
    
    Value* loaded_temp = force_get_forced_value(mgr, "temperature");
    ASSERT(loaded_temp != NULL);
    ASSERT(loaded_temp->type == TYPE_REAL);
    // 浮点数比较使用近似
    ASSERT(loaded_temp->real_val > 25.4 && loaded_temp->real_val < 25.6);
    
    Value* loaded_alarm = force_get_forced_value(mgr, "alarm");
    ASSERT(loaded_alarm != NULL);
    ASSERT(loaded_alarm->type == TYPE_BOOL);
    ASSERT(loaded_alarm->bool_val == true);
    
    force_manager_destroy(mgr);
    
    PASS();
}

// 测试2：按索引保存和加载
bool test_save_load_by_index() {
    TEST("Save and Load by Index");
    
    ForceManager* mgr = force_manager_create();
    ASSERT(mgr != NULL);
    
    // 按索引添加强制
    Value v1 = {.type = TYPE_INT, .int_val = 42};
    Value v2 = {.type = TYPE_REAL, .real_val = 3.14};
    
    force_variable_by_index(mgr, 0, v1, false);
    force_variable_by_index(mgr, 5, v2, true);
    
    ASSERT(mgr->force_count == 2);
    
    // 保存
    const char* filename = "/tmp/test_force_index.force";
    force_save_to_file(mgr, filename);
    
    // 清空
    unforce_all(mgr);
    ASSERT(mgr->force_count == 0);
    
    // 加载
    force_load_from_file(mgr, filename);
    ASSERT(mgr->force_count == 2);
    
    // 验证
    ASSERT(force_is_forced_by_index(mgr, 0) == true);
    ASSERT(force_is_forced_by_index(mgr, 5) == true);
    
    Value* v1_loaded = force_get_forced_value_by_index(mgr, 0);
    ASSERT(v1_loaded != NULL);
    ASSERT(v1_loaded->int_val == 42);
    
    Value* v2_loaded = force_get_forced_value_by_index(mgr, 5);
    ASSERT(v2_loaded != NULL);
    ASSERT(v2_loaded->real_val > 3.13 && v2_loaded->real_val < 3.15);
    
    force_manager_destroy(mgr);
    
    PASS();
}

// 测试3：持久化标志
bool test_persistent_flag() {
    TEST("Persistent Flag");
    
    ForceManager* mgr = force_manager_create();
    ASSERT(mgr != NULL);
    
    Value v1 = {.type = TYPE_INT, .int_val = 10};
    Value v2 = {.type = TYPE_INT, .int_val = 20};
    
    force_variable(mgr, "temp_var", v1, false);      // 临时
    force_variable(mgr, "persist_var", v2, true);    // 持久化
    
    // 保存和加载
    const char* filename = "/tmp/test_persistent.force";
    force_save_to_file(mgr, filename);
    unforce_all(mgr);
    force_load_from_file(mgr, filename);
    
    // 验证持久化标志
    ForceVariable* temp_fv = force_find_variable(mgr, "temp_var");
    ASSERT(temp_fv != NULL);
    ASSERT(temp_fv->is_persistent == false);
    
    ForceVariable* persist_fv = force_find_variable(mgr, "persist_var");
    ASSERT(persist_fv != NULL);
    ASSERT(persist_fv->is_persistent == true);
    
    force_manager_destroy(mgr);
    
    PASS();
}

// 测试4：质量化类型
bool test_qualified_types() {
    TEST("Qualified Types");
    
    ForceManager* mgr = force_manager_create();
    ASSERT(mgr != NULL);
    
    Value qint = {.type = TYPE_QINT, .int_val = 123, .quality = QUALITY_GOOD};
    Value qreal = {.type = TYPE_QREAL, .real_val = 45.6, .quality = QUALITY_UNCERTAIN};
    Value qbool = {.type = TYPE_QBOOL, .bool_val = true, .quality = QUALITY_BAD};
    
    force_variable(mgr, "qint_var", qint, false);
    force_variable(mgr, "qreal_var", qreal, false);
    force_variable(mgr, "qbool_var", qbool, false);
    
    // 保存和加载
    const char* filename = "/tmp/test_qualified.force";
    force_save_to_file(mgr, filename);
    unforce_all(mgr);
    force_load_from_file(mgr, filename);
    
    // 验证类型
    Value* loaded_qint = force_get_forced_value(mgr, "qint_var");
    ASSERT(loaded_qint != NULL);
    ASSERT(loaded_qint->type == TYPE_QINT);
    ASSERT(loaded_qint->int_val == 123);
    
    Value* loaded_qreal = force_get_forced_value(mgr, "qreal_var");
    ASSERT(loaded_qreal != NULL);
    ASSERT(loaded_qreal->type == TYPE_QREAL);
    
    Value* loaded_qbool = force_get_forced_value(mgr, "qbool_var");
    ASSERT(loaded_qbool != NULL);
    ASSERT(loaded_qbool->type == TYPE_QBOOL);
    ASSERT(loaded_qbool->bool_val == true);
    
    force_manager_destroy(mgr);
    
    PASS();
}

// 测试5：空文件和错误处理
bool test_error_handling() {
    TEST("Error Handling");
    
    ForceManager* mgr = force_manager_create();
    ASSERT(mgr != NULL);
    
    // 加载不存在的文件
    bool result = force_load_from_file(mgr, "/tmp/nonexistent_file.force");
    ASSERT(result == false);
    ASSERT(mgr->force_count == 0);
    
    // 保存到无效路径
    result = force_save_to_file(mgr, "/invalid/path/file.force");
    ASSERT(result == false);
    
    force_manager_destroy(mgr);
    
    PASS();
}

// 测试6：Enable状态保存和加载
bool test_enable_state() {
    TEST("Enable State Save/Load");
    
    ForceManager* mgr = force_manager_create();
    ASSERT(mgr != NULL);
    
    Value v = {.type = TYPE_INT, .int_val = 99};
    force_variable(mgr, "test_var", v, false);
    
    // 禁用Force
    force_enable(mgr, false);
    ASSERT(force_is_enabled(mgr) == false);
    
    // 保存
    const char* filename = "/tmp/test_enable.force";
    force_save_to_file(mgr, filename);
    
    // 重新创建并加载
    force_manager_destroy(mgr);
    mgr = force_manager_create();
    
    force_load_from_file(mgr, filename);
    
    // 验证enable状态被恢复
    ASSERT(force_is_enabled(mgr) == false);
    
    force_manager_destroy(mgr);
    
    PASS();
}

// 主测试函数
int main() {
    printf("\n");
    printf("*******************************************\n");
    printf("*   Force File Load/Save Tests           *\n");
    printf("*******************************************\n");
    
    int passed = 0;
    int total = 0;
    
    #define RUN_TEST(test) do { \
        total++; \
        if (test()) passed++; \
    } while(0)
    
    RUN_TEST(test_save_and_load);
    RUN_TEST(test_save_load_by_index);
    RUN_TEST(test_persistent_flag);
    RUN_TEST(test_qualified_types);
    RUN_TEST(test_error_handling);
    RUN_TEST(test_enable_state);
    
    printf("\n");
    printf("*******************************************\n");
    printf("*   Test Results: %d/%d passed           *\n", passed, total);
    printf("*******************************************\n");
    printf("\n");
    
    return (passed == total) ? 0 : 1;
}
