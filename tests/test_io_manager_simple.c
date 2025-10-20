/**
 * @file test_io_manager.c
 * @brief I/O 管理器简单测试
 */

#include <stdio.h>
#include <assert.h>
#include "iomgr.h"

int main() {
    printf("=== STVM I/O Manager Test ===\n\n");
    
    // 测试1：地址解析
    printf("Test 1: Address parsing...\n");
    IOAddress addr;
    assert(io_address_parse("%IX0.0", &addr) == OK);
    assert(addr.location == IO_LOC_INPUT);
    assert(addr.size == IO_SIZE_BIT);
    printf("✓ Address parsing works\n\n");
    
    // 测试2：创建管理器
    printf("Test 2: Manager creation...\n");
    IOHardwareAdapter* adapter = io_adapter_create_simulator();
    assert(adapter != NULL);
    printf("✓ Simulator adapter created\n");
    
    IOManager* mgr = io_manager_create(adapter);
    assert(mgr != NULL);
    printf("✓ I/O manager created\n\n");
    
    // 测试3：添加 I/O 点
    printf("Test 3: Adding I/O points...\n");
    IOPointConfig config = {
        .address = {IO_LOC_INPUT, IO_SIZE_BIT, 0, 0},
        .device_type = IO_DEV_GPIO_IN,
        .access_mode = IO_ACCESS_READ_ONLY,
        .hardware_path = "/dev/gpio0",
        .hardware_address = 17,
        .scale = 1.0,
        .offset = 0.0,
        .enable_filter = false
    };
    
    ErrorCode err = io_manager_add_point(mgr, &config);
    assert(err == OK);
    printf("✓ I/O point added\n");
    
    // 测试4：查找 I/O 点
    printf("✓ Point count: %u\n\n", io_manager_get_point_count(mgr));
    
    IOPoint* point = io_manager_find_point(mgr, &config.address);
    assert(point != NULL);
    printf("✓ I/O point found\n\n");
    
    // 测试5：读写操作
    printf("Test 4: Read/Write operations...\n");
    Value write_val = {.type = TYPE_BOOL, .bool_val = true};
    Value read_val;
    
    config.address.location = IO_LOC_OUTPUT;
    config.device_type = IO_DEV_GPIO_OUT;
    config.access_mode = IO_ACCESS_READ_WRITE;
    io_manager_add_point(mgr, &config);
    
    err = io_manager_write(mgr, &config.address, &write_val);
    assert(err == OK);
    printf("✓ Write operation successful\n");
    
    err = io_manager_read(mgr, &config.address, &read_val);
    assert(err == OK);
    printf("✓ Read operation successful\n\n");
    
    // 清理
    printf("Cleanup...\n");
    io_manager_free(mgr);
    io_adapter_free_simulator(adapter);
    printf("✓ Cleanup complete\n\n");
    
    printf("=== All tests passed! ===\n");
    return 0;
}
