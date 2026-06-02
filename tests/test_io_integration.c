/**
 * @file test_io_integration.c
 * @brief I/O系统集成测试
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/include/vm.h"
#include "../src/include/bytecode_io.h"
#include "../src/include/iomgr.h"
#include "../src/include/mmgr.h"

void test_io_basic(void) {
    printf("=== 测试基本I/O功能 ===\n");
    
    // 初始化内存管理
    mmgr_init();
    
    // 创建 IOManager
    IOManager* iomgr = io_manager_create();
    assert(iomgr != NULL);
    
    // 注册测试适配器（模拟）
    IOAdapter* adapter = io_adapter_create_sim();
    assert(adapter != NULL);
    io_manager_register_adapter(iomgr, adapter);
    
    // 测试写入和读取
    Value write_val = {.type = TYPE_BOOL, .bool_val = true};
    ErrorCode err = io_manager_write(iomgr, "%QX0.0", write_val);
    assert(err == OK);
    printf("写入 %%QX0.0 = true\n");
    
    Value read_val;
    err = io_manager_read(iomgr, "%IX0.0", &read_val);
    assert(err == OK);
    printf("读取 %%IX0.0 = %s\n", read_val.bool_val ? "true" : "false");
    
    // 清理
    io_manager_free(iomgr);
    mmgr_cleanup();
    
    printf("✓ 基本I/O测试通过\n\n");
}

void test_vm_io_execution(void) {
    printf("=== 测试VM I/O执行 ===\n");
    
    // 初始化内存管理
    mmgr_init();
    
    // 加载编译好的字节码
    BytecodeModule* module = bytecode_load_from_file("examples/io_test.stbc");
    if (!module) {
        fprintf(stderr, "无法加载字节码文件\n");
        mmgr_cleanup();
        return;
    }
    
    // 创建 IOManager
    IOManager* iomgr = io_manager_create();
    assert(iomgr != NULL);
    
    // 注册模拟适配器
    IOAdapter* adapter = io_adapter_create_sim();
    assert(adapter != NULL);
    io_manager_register_adapter(iomgr, adapter);
    
    // 模拟按钮按下
    Value button_val = {.type = TYPE_BOOL, .bool_val = true};
    io_manager_write(iomgr, "%IX0.0", button_val);
    printf("设置按钮 %%IX0.0 = true\n");
    
    // 创建VM并设置IOManager
    VM* vm = vm_create(module);
    assert(vm != NULL);
    vm_set_io_manager(vm, iomgr);
    
    // 执行程序
    ErrorCode err = vm_run(vm);
    if (err != OK) {
        fprintf(stderr, "VM执行失败: %d\n", err);
    } else {
        printf("VM执行成功\n");
    }
    
    // 检查LED状态
    Value led_val;
    err = io_manager_read(iomgr, "%QX0.0", &led_val);
    assert(err == OK);
    printf("LED %%QX0.0 = %s\n", led_val.bool_val ? "ON" : "OFF");
    
    // 验证LED应该是ON（因为button是true）
    assert(led_val.bool_val == true);
    
    // 清理
    vm_free(vm);
    bytecode_module_free(module);
    io_manager_free(iomgr);
    mmgr_cleanup();
    
    printf("✓ VM I/O执行测试通过\n\n");
}

int main(void) {
    printf("开始I/O集成测试...\n\n");
    
    test_io_basic();
    test_vm_io_execution();
    
    printf("所有测试通过！\n");
    return 0;
}
