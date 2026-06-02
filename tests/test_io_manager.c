/**
 * @file test_io_manager.c
 * @brief I/O 管理器测试程序
 * 
 * 测试内容：
 * 1. I/O 地址解析
 * 2. I/O 点添加与查找
 * 3. 模拟适配器读写
 * 4. 自动刷新功能
 * 5. 统计信息输出
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "iomgr.h"

// 测试宏
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "❌ FAILED: %s\n   %s:%d\n", message, __FILE__, __LINE__); \
            return -1; \
        } else { \
            printf("✓ PASSED: %s\n", message); \
        } \
    } while(0)

/**
 * @brief 测试1：I/O 地址解析
 */
int test_address_parsing() {
    printf("\n=== Test 1: I/O Address Parsing ===\n");
    
    IOAddress addr;
    ErrorCode err;
    char buffer[32];
    
    // 测试 %IX0.0
    err = io_address_parse("%IX0.0", &addr);
    TEST_ASSERT(err == OK, "Parse %IX0.0");
    TEST_ASSERT(addr.location == IO_LOC_INPUT, "Location is INPUT");
    TEST_ASSERT(addr.size == IO_SIZE_BIT, "Size is BIT");
    TEST_ASSERT(addr.byte_offset == 0, "Byte offset is 0");
    TEST_ASSERT(addr.bit_offset == 0, "Bit offset is 0");
    
    io_address_format(&addr, buffer, sizeof(buffer));
    TEST_ASSERT(strcmp(buffer, "%IX0.0") == 0, "Format back to %IX0.0");
    
    // 测试 %QW5
    err = io_address_parse("%QW5", &addr);
    TEST_ASSERT(err == OK, "Parse %QW5");
    TEST_ASSERT(addr.location == IO_LOC_OUTPUT, "Location is OUTPUT");
    TEST_ASSERT(addr.size == IO_SIZE_WORD, "Size is WORD");
    TEST_ASSERT(addr.byte_offset == 5, "Byte offset is 5");
    
    // 测试 %MD100
    err = io_address_parse("%MD100", &addr);
    TEST_ASSERT(err == OK, "Parse %MD100");
    TEST_ASSERT(addr.location == IO_LOC_MEMORY, "Location is MEMORY");
    TEST_ASSERT(addr.size == IO_SIZE_DWORD, "Size is DWORD");
    TEST_ASSERT(addr.byte_offset == 100, "Byte offset is 100");
    
    // 测试无效地址
    err = io_address_parse("INVALID", &addr);
    TEST_ASSERT(err != OK, "Reject invalid address");
    
    printf("✓ All address parsing tests passed\n");
    return 0;
}

/**
 * @brief 测试2：I/O 管理器创建与销毁
 */
int test_manager_lifecycle() {
    printf("\n=== Test 2: Manager Lifecycle ===\n");
    
    // 使用模拟适配器
    IOHardwareAdapter* adapter = io_create_sim_adapter();
    TEST_ASSERT(adapter != NULL, "Create simulator adapter");
    
    // 创建管理器
    IOManager* mgr = io_manager_create(adapter);
    TEST_ASSERT(mgr != NULL, "Create I/O manager");
    TEST_ASSERT(mgr->hal_adapter == adapter, "Adapter set correctly");
    TEST_ASSERT(mgr->point_count == 0, "Initial point count is 0");
    
    // 释放管理器
    io_manager_free(mgr);
    printf("✓ Manager freed successfully\n");
    
    return 0;
}

/**
 * @brief 测试3：添加和查找 I/O 点
 */
int test_io_points() {
    printf("\n=== Test 3: I/O Points Management ===\n");
    
    IOManager* mgr = io_manager_create(io_create_sim_adapter());
    TEST_ASSERT(mgr != NULL, "Create manager");
    
    // 添加数字输入点
    IOPointConfig config = {
        .address = {IO_LOC_INPUT, IO_SIZE_BIT, 0, 0},  // %IX0.0
        .device_type = IO_TYPE_DIGITAL_IN,
        .access_mode = IO_ACCESS_READ_ONLY,
        .hardware_path = "sim://gpio/17",
        .hardware_address = 17,
        .scale = 1.0,
        .offset = 0.0,
        .invert = false,
        .enable_filter = false,
        .refresh_interval_us = 0,
        .force_write = false,
        .read_protected = false,
        .write_protected = false
    };
    
    ErrorCode err = io_manager_add_point(mgr, &config);
    TEST_ASSERT(err == OK, "Add input point %IX0.0");
    TEST_ASSERT(mgr->point_count == 1, "Point count is 1");
    
    // 查找点
    IOAddress search_addr = {IO_LOC_INPUT, IO_SIZE_BIT, 0, 0};
    IOPoint* point = io_manager_find_point(mgr, &search_addr);
    TEST_ASSERT(point != NULL, "Find point %IX0.0");
    TEST_ASSERT(point->config.device_type == IO_TYPE_DIGITAL_IN, "Device type correct");
    
    // 添加数字输出点
    config.address = (IOAddress){IO_LOC_OUTPUT, IO_SIZE_BIT, 0, 0}; // %QX0.0
    config.device_type = IO_TYPE_DIGITAL_OUT;
    config.access_mode = IO_ACCESS_WRITE_ONLY;
    
    err = io_manager_add_point(mgr, &config);
    TEST_ASSERT(err == OK, "Add output point %QX0.0");
    TEST_ASSERT(mgr->point_count == 2, "Point count is 2");
    
    // 查找输出点
    search_addr = (IOAddress){IO_LOC_OUTPUT, IO_SIZE_BIT, 0, 0};
    point = io_manager_find_point(mgr, &search_addr);
    TEST_ASSERT(point != NULL, "Find point %QX0.0");
    
    // 尝试添加重复点
    err = io_manager_add_point(mgr, &config);
    TEST_ASSERT(err != OK, "Reject duplicate point");
    
    io_manager_free(mgr);
    printf("✓ All I/O points tests passed\n");
    
    return 0;
}

/**
 * @brief 测试4：I/O 读写操作
 */
int test_io_operations() {
    printf("\n=== Test 4: I/O Read/Write Operations ===\n");
    
    IOManager* mgr = io_manager_create(io_create_sim_adapter());
    
    // 添加输入点
    IOPointConfig in_config = {
        .address = {IO_LOC_INPUT, IO_SIZE_WORD, 0, 0},  // %IW0
        .device_type = IO_TYPE_ANALOG_IN,
        .access_mode = IO_ACCESS_READ_ONLY,
        .hardware_path = "sim://adc/0",
        .hardware_address = 0
    };
    io_manager_add_point(mgr, &in_config);
    
    // 添加输出点
    IOPointConfig out_config = {
        .address = {IO_LOC_OUTPUT, IO_SIZE_BIT, 0, 0},  // %QX0.0
        .device_type = IO_TYPE_DIGITAL_OUT,
        .access_mode = IO_ACCESS_WRITE_ONLY,
        .hardware_path = "sim://gpio/27",
        .hardware_address = 27
    };
    io_manager_add_point(mgr, &out_config);
    
    // 读取输入
    IOAddress in_addr = {IO_LOC_INPUT, IO_SIZE_WORD, 0, 0};
    Value in_value;
    ErrorCode err = io_manager_read(mgr, &in_addr, &in_value);
    TEST_ASSERT(err == OK, "Read analog input %IW0");
    printf("  Read value: %d\n", in_value.int_val);
    
    // 写入输出
    IOAddress out_addr = {IO_LOC_OUTPUT, IO_SIZE_BIT, 0, 0};
    Value out_value = {.type = TYPE_BOOL, .bool_val = true};
    err = io_manager_write(mgr, &out_addr, &out_value);
    TEST_ASSERT(err == OK, "Write digital output %QX0.0 = TRUE");
    
    // 再次读取输出点（验证写入）
    Value readback;
    err = io_manager_read(mgr, &out_addr, &readback);
    TEST_ASSERT(err == OK, "Read back output");
    TEST_ASSERT(readback.bool_val == true, "Output value is TRUE");
    
    // 写入 FALSE
    out_value.bool_val = false;
    io_manager_write(mgr, &out_addr, &out_value);
    io_manager_read(mgr, &out_addr, &readback);
    TEST_ASSERT(readback.bool_val == false, "Output value is FALSE");
    
    io_manager_free(mgr);
    printf("✓ All I/O operations tests passed\n");
    
    return 0;
}

/**
 * @brief 测试5：自动刷新功能
 */
int test_auto_refresh() {
    printf("\n=== Test 5: Auto Refresh ===\n");
    
    IOManager* mgr = io_manager_create(io_create_sim_adapter());
    
    // 添加一些I/O点
    IOPointConfig config = {
        .address = {IO_LOC_INPUT, IO_SIZE_BIT, 0, 0},
        .device_type = IO_TYPE_DIGITAL_IN,
        .access_mode = IO_ACCESS_READ_ONLY,
        .hardware_path = "sim://gpio/17",
        .hardware_address = 17
    };
    io_manager_add_point(mgr, &config);
    
    // 启动自动刷新 (100ms 周期)
    ErrorCode err = io_manager_start_refresh(mgr, 100000);
    TEST_ASSERT(err == OK, "Start auto refresh");
    TEST_ASSERT(mgr->auto_refresh == true, "Auto refresh enabled");
    
    printf("  Running auto refresh for 1 second...\n");
    sleep(1);
    
    // 检查统计信息
    uint64_t reads, writes, errors;
    io_manager_get_stats(mgr, &reads, &writes, &errors);
    printf("  Stats: reads=%lu, writes=%lu, errors=%lu\n", reads, writes, errors);
    TEST_ASSERT(reads > 0, "Auto refresh performed reads");
    
    // 停止刷新
    io_manager_stop_refresh(mgr);
    TEST_ASSERT(mgr->auto_refresh == false, "Auto refresh stopped");
    
    io_manager_free(mgr);
    printf("✓ Auto refresh test passed\n");
    
    return 0;
}

/**
 * @brief 测试6：统计信息
 */
int test_statistics() {
    printf("\n=== Test 6: Statistics ===\n");
    
    IOManager* mgr = io_manager_create(io_create_sim_adapter());
    
    // 添加点
    IOPointConfig config = {
        .address = {IO_LOC_MEMORY, IO_SIZE_WORD, 0, 0},  // %MW0
        .device_type = IO_TYPE_DIGITAL_OUT,
        .access_mode = IO_ACCESS_READ_WRITE,
        .hardware_path = "sim://mem/0",
        .hardware_address = 0
    };
    io_manager_add_point(mgr, &config);
    
    IOAddress addr = {IO_LOC_MEMORY, IO_SIZE_WORD, 0, 0};
    Value value = {.type = TYPE_INT, .int_val = 100};
    
    // 执行一些操作
    for (int i = 0; i < 10; i++) {
        io_manager_write(mgr, &addr, &value);
        io_manager_read(mgr, &addr, &value);
    }
    
    // 检查统计
    uint64_t reads, writes, errors;
    io_manager_get_stats(mgr, &reads, &writes, &errors);
    TEST_ASSERT(reads == 10, "Read count is 10");
    TEST_ASSERT(writes == 10, "Write count is 10");
    TEST_ASSERT(errors == 0, "No errors");
    
    // 打印诊断信息
    printf("\n");
    io_manager_print_diagnostics(mgr);
    
    // 重置统计
    io_manager_reset_stats(mgr);
    io_manager_get_stats(mgr, &reads, &writes, &errors);
    TEST_ASSERT(reads == 0, "Stats reset: reads = 0");
    TEST_ASSERT(writes == 0, "Stats reset: writes = 0");
    
    io_manager_free(mgr);
    printf("✓ Statistics test passed\n");
    
    return 0;
}

/**
 * @brief 主测试函数
 */
int main(int argc, char** argv) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║          STVM I/O Manager Test Suite                  ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    
    int result = 0;
    
    // 运行所有测试
    if (test_address_parsing() != 0) result = -1;
    if (test_manager_lifecycle() != 0) result = -1;
    if (test_io_points() != 0) result = -1;
    if (test_io_operations() != 0) result = -1;
    if (test_auto_refresh() != 0) result = -1;
    if (test_statistics() != 0) result = -1;
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    if (result == 0) {
        printf("║          ✓ ALL TESTS PASSED                           ║\n");
    } else {
        printf("║          ✗ SOME TESTS FAILED                          ║\n");
    }
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return result;
}
