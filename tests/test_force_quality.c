/**
 * @file test_force_quality.c
 * @brief Force质量位功能测试
 */

#include "force.h"
#include "types.h"
#include "mmgr.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

// 测试计数器
static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) \
    printf("\n=== Test: %s ===\n", name); \
    tests_run++;

#define ASSERT(condition, message) \
    if (!(condition)) { \
        printf("FAILED: %s\n", message); \
        return; \
    }

#define PASS() \
    printf("PASSED\n"); \
    tests_passed++;

/**
 * @brief 测试质量位强制和恢复
 */
void test_quality_force_and_restore() {
    TEST("Quality Force and Restore");
    
    ForceManager* mgr = force_manager_create();
    ASSERT(mgr != NULL, "创建Force管理器失败");
    
    // 强制QINT变量，使用BAD质量位
    Value qint_val;
    qint_val.type = TYPE_QINT;
    qint_val.int_val = 999;
    qint_val.quality = QUALITY_BAD;
    
    bool result = force_variable(mgr, "sensor1", qint_val, false);
    ASSERT(result == true, "强制QINT变量失败");
    
    // 获取并验证
    ForceVariable* fv = force_find_variable(mgr, "sensor1");
    ASSERT(fv != NULL, "获取强制变量失败");
    ASSERT(fv->forced_value.type == TYPE_QINT, "类型不匹配");
    ASSERT(fv->forced_value.int_val == 999, "值不匹配");
    ASSERT(fv->forced_value.quality == QUALITY_BAD, "质量位不匹配");
    
    force_manager_destroy(mgr);
    PASS();
}

/**
 * @brief 测试所有质量位类型
 */
void test_all_quality_flags() {
    TEST("All Quality Flags");
    
    ForceManager* mgr = force_manager_create();
    ASSERT(mgr != NULL, "创建Force管理器失败");
    
    // QUALITY_GOOD
    Value val_good;
    val_good.type = TYPE_QREAL;
    val_good.real_val = 1.0;
    val_good.quality = QUALITY_GOOD;
    force_variable(mgr, "good_var", val_good, false);
    
    // QUALITY_UNCERTAIN
    Value val_uncertain;
    val_uncertain.type = TYPE_QREAL;
    val_uncertain.real_val = 2.0;
    val_uncertain.quality = QUALITY_UNCERTAIN;
    force_variable(mgr, "uncertain_var", val_uncertain, false);
    
    // QUALITY_BAD
    Value val_bad;
    val_bad.type = TYPE_QREAL;
    val_bad.real_val = 3.0;
    val_bad.quality = QUALITY_BAD;
    force_variable(mgr, "bad_var", val_bad, false);
    
    // QUALITY_ERROR
    Value val_error;
    val_error.type = TYPE_QREAL;
    val_error.real_val = 4.0;
    val_error.quality = QUALITY_ERROR;
    force_variable(mgr, "error_var", val_error, false);
    
    // 验证所有质量位
    ForceVariable* fv_good = force_find_variable(mgr, "good_var");
    ASSERT(fv_good && fv_good->forced_value.quality == QUALITY_GOOD, "GOOD质量位不匹配");
    
    ForceVariable* fv_uncertain = force_find_variable(mgr, "uncertain_var");
    ASSERT(fv_uncertain && fv_uncertain->forced_value.quality == QUALITY_UNCERTAIN, "UNCERTAIN质量位不匹配");
    
    ForceVariable* fv_bad = force_find_variable(mgr, "bad_var");
    ASSERT(fv_bad && fv_bad->forced_value.quality == QUALITY_BAD, "BAD质量位不匹配");
    
    ForceVariable* fv_error = force_find_variable(mgr, "error_var");
    ASSERT(fv_error && fv_error->forced_value.quality == QUALITY_ERROR, "ERROR质量位不匹配");
    
    force_manager_destroy(mgr);
    PASS();
}

/**
 * @brief 测试质量位文件保存和加载
 */
void test_quality_save_and_load() {
    TEST("Quality Save and Load");
    
    ForceManager* mgr1 = force_manager_create();
    ASSERT(mgr1 != NULL, "创建Force管理器失败");
    
    // 强制几个带质量位的变量
    Value qint_bad;
    qint_bad.type = TYPE_QINT;
    qint_bad.int_val = 100;
    qint_bad.quality = QUALITY_BAD;
    force_variable(mgr1, "qint_var", qint_bad, true);
    
    Value qreal_uncertain;
    qreal_uncertain.type = TYPE_QREAL;
    qreal_uncertain.real_val = 25.5;
    qreal_uncertain.quality = QUALITY_UNCERTAIN;
    force_variable(mgr1, "qreal_var", qreal_uncertain, false);
    
    Value qbool_error;
    qbool_error.type = TYPE_QBOOL;
    qbool_error.bool_val = true;
    qbool_error.quality = QUALITY_ERROR;
    force_variable(mgr1, "qbool_var", qbool_error, true);
    
    // 保存到文件
    const char* filename = "/tmp/test_quality.force";
    bool saved = force_save_to_file(mgr1, filename);
    ASSERT(saved == true, "保存文件失败");
    
    // 创建新管理器并加载
    ForceManager* mgr2 = force_manager_create();
    bool loaded = force_load_from_file(mgr2, filename);
    ASSERT(loaded == true, "加载文件失败");
    
    // 验证QINT
    ForceVariable* loaded_qint_fv = force_find_variable(mgr2, "qint_var");
    ASSERT(loaded_qint_fv != NULL, "QINT变量未加载");
    Value* loaded_qint = &loaded_qint_fv->forced_value;
    ASSERT(loaded_qint->type == TYPE_QINT, "QINT类型不匹配");
    ASSERT(loaded_qint->int_val == 100, "QINT值不匹配");
    ASSERT(loaded_qint->quality == QUALITY_BAD, "QINT质量位不匹配");
    
    // 验证QREAL
    ForceVariable* loaded_qreal_fv = force_find_variable(mgr2, "qreal_var");
    ASSERT(loaded_qreal_fv != NULL, "QREAL变量未加载");
    Value* loaded_qreal = &loaded_qreal_fv->forced_value;
    ASSERT(loaded_qreal->type == TYPE_QREAL, "QREAL类型不匹配");
    ASSERT(loaded_qreal->real_val == 25.5, "QREAL值不匹配");
    ASSERT(loaded_qreal->quality == QUALITY_UNCERTAIN, "QREAL质量位不匹配");
    
    // 验证QBOOL
    ForceVariable* loaded_qbool_fv = force_find_variable(mgr2, "qbool_var");
    ASSERT(loaded_qbool_fv != NULL, "QBOOL变量未加载");
    Value* loaded_qbool = &loaded_qbool_fv->forced_value;
    ASSERT(loaded_qbool->type == TYPE_QBOOL, "QBOOL类型不匹配");
    ASSERT(loaded_qbool->bool_val == true, "QBOOL值不匹配");
    ASSERT(loaded_qbool->quality == QUALITY_ERROR, "QBOOL质量位不匹配");
    
    force_manager_destroy(mgr1);
    force_manager_destroy(mgr2);
    PASS();
}

/**
 * @brief 测试质量位字符串转换
 */
void test_quality_string_conversion() {
    TEST("Quality String Conversion");
    
    // 测试quality_to_string
    ASSERT(strcmp(quality_to_string(QUALITY_GOOD), "good") == 0, "GOOD转字符串失败");
    ASSERT(strcmp(quality_to_string(QUALITY_UNCERTAIN), "uncertain") == 0, "UNCERTAIN转字符串失败");
    ASSERT(strcmp(quality_to_string(QUALITY_BAD), "bad") == 0, "BAD转字符串失败");
    ASSERT(strcmp(quality_to_string(QUALITY_ERROR), "error") == 0, "ERROR转字符串失败");
    
    // 测试string_to_quality (字符串)
    ASSERT(string_to_quality("good") == QUALITY_GOOD, "good解析失败");
    ASSERT(string_to_quality("uncertain") == QUALITY_UNCERTAIN, "uncertain解析失败");
    ASSERT(string_to_quality("bad") == QUALITY_BAD, "bad解析失败");
    ASSERT(string_to_quality("error") == QUALITY_ERROR, "error解析失败");
    
    // 测试string_to_quality (数字)
    ASSERT(string_to_quality("0") == QUALITY_GOOD, "0解析失败");
    ASSERT(string_to_quality("1") == QUALITY_UNCERTAIN, "1解析失败");
    ASSERT(string_to_quality("2") == QUALITY_BAD, "2解析失败");
    ASSERT(string_to_quality("3") == QUALITY_ERROR, "3解析失败");
    
    // 测试默认值
    ASSERT(string_to_quality("invalid") == QUALITY_GOOD, "无效字符串未返回默认值");
    ASSERT(string_to_quality(NULL) == QUALITY_GOOD, "NULL未返回默认值");
    
    PASS();
}

/**
 * @brief 测试混合类型（带质量位和不带质量位）
 */
void test_mixed_types() {
    TEST("Mixed Types (Qualified and Non-Qualified)");
    
    ForceManager* mgr = force_manager_create();
    ASSERT(mgr != NULL, "创建Force管理器失败");
    
    // 普通INT
    Value int_val;
    int_val.type = TYPE_INT;
    int_val.int_val = 10;
    int_val.quality = QUALITY_GOOD;  // 质量位应该被忽略
    force_variable(mgr, "normal_int", int_val, false);
    
    // QINT
    Value qint_val;
    qint_val.type = TYPE_QINT;
    qint_val.int_val = 20;
    qint_val.quality = QUALITY_BAD;
    force_variable(mgr, "quality_int", qint_val, false);
    
    // 保存并重新加载
    const char* filename = "/tmp/test_mixed.force";
    force_save_to_file(mgr, filename);
    
    ForceManager* mgr2 = force_manager_create();
    force_load_from_file(mgr2, filename);
    
    // 验证普通INT（质量位应该是默认GOOD）
    ForceVariable* loaded_int_fv = force_find_variable(mgr2, "normal_int");
    ASSERT(loaded_int_fv != NULL, "普通INT未加载");
    Value* loaded_int = &loaded_int_fv->forced_value;
    ASSERT(loaded_int->type == TYPE_INT, "INT类型不匹配");
    ASSERT(loaded_int->int_val == 10, "INT值不匹配");
    
    // 验证QINT（质量位应该是BAD）
    ForceVariable* loaded_qint_fv = force_find_variable(mgr2, "quality_int");
    ASSERT(loaded_qint_fv != NULL, "QINT未加载");
    Value* loaded_qint = &loaded_qint_fv->forced_value;
    ASSERT(loaded_qint->type == TYPE_QINT, "QINT类型不匹配");
    ASSERT(loaded_qint->int_val == 20, "QINT值不匹配");
    ASSERT(loaded_qint->quality == QUALITY_BAD, "QINT质量位不匹配");
    
    force_manager_destroy(mgr);
    force_manager_destroy(mgr2);
    PASS();
}

/**
 * @brief 主函数
 */
int main() {
    mmgr_init();
    
    printf("*******************************************\n");
    printf("*   Force Quality Tests                  *\n");
    printf("*******************************************\n");
    
    test_quality_force_and_restore();
    test_all_quality_flags();
    test_quality_save_and_load();
    test_quality_string_conversion();
    test_mixed_types();
    
    printf("\n*******************************************\n");
    printf("*   Test Results: %d/%d passed           *\n", tests_passed, tests_run);
    printf("*******************************************\n");
    
    mmgr_cleanup();
    return (tests_passed == tests_run) ? 0 : 1;
}
