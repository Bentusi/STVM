/**
 * @file test_hotreload.c
 * @brief 热更新功能测试程序
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "hotreload.h"
#include "bytecode_io.h"
#include "mmgr.h"

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s <v1.stbc> <v2.stbc>\n", argv[0]);
        printf("Example: %s examples/hotreload_v1.stbc examples/hotreload_v2.stbc\n", argv[0]);
        return 1;
    }
    
    const char* v1_file = argv[1];
    const char* v2_file = argv[2];
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║          STVM 热更新功能测试程序                       ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n\n");
    
    // 初始化内存管理器
    mmgr_init();
    
    // ========== 阶段 1: 加载并运行 V1 ==========
    printf("▶ Phase 1: Loading and running V1...\n");
    printf("   File: %s\n\n", v1_file);
    
    BytecodeModule* module_v1 = bytecode_load(v1_file);
    if (!module_v1) {
        fprintf(stderr, "Error: Failed to load %s\n", v1_file);
        return 1;
    }
    
    VM* vm = vm_create(module_v1);
    if (!vm) {
        fprintf(stderr, "Error: Failed to create VM\n");
        bytecode_module_free(module_v1);
        return 1;
    }
    
    ErrorCode err = vm_run(vm);
    if (err != OK) {
        fprintf(stderr, "Error: VM execution failed (code=%d)\n", err);
        vm_free(vm);
        bytecode_module_free(module_v1);
        return 1;
    }
    
    printf("\n");
    
    // ========== 阶段 2: 创建热更新管理器 ==========
    printf("▶ Phase 2: Creating HotReload Manager...\n\n");
    
    HotReloadManager* hotreload = hotreload_create(vm);
    if (!hotreload) {
        fprintf(stderr, "Error: Failed to create hotreload manager\n");
        vm_free(vm);
        bytecode_module_free(module_v1);
        return 1;
    }
    
    hotreload_set_verbose(hotreload, true);
    
    // ========== 阶段 3: 暂存 V2 模块 ==========
    printf("▶ Phase 3: Staging V2 module...\n");
    printf("   File: %s\n\n", v2_file);
    
    err = hotreload_stage_module(hotreload, v2_file);
    if (err != OK) {
        fprintf(stderr, "Error: Failed to stage module (code=%d)\n", err);
        hotreload_free(hotreload);
        vm_free(vm);
        bytecode_module_free(module_v1);
        return 1;
    }
    
    // ========== 阶段 4: 安全性检查 ==========
    printf("\n▶ Phase 4: Safety check...\n");
    
    if (!hotreload_is_safe(hotreload)) {
        fprintf(stderr, "Warning: Hot reload is not safe at this point\n");
        fprintf(stderr, "         Forcing reload anyway for demo purposes...\n");
        hotreload->allow_unsafe_reload = true;
    } else {
        printf("   ✓ Safe to reload\n");
    }
    
    // ========== 阶段 5: 应用热更新 ==========
    printf("\n▶ Phase 5: Applying hot reload...\n\n");
    
    err = hotreload_apply_staged(hotreload);
    if (err != OK) {
        fprintf(stderr, "Error: Failed to apply reload (code=%d)\n", err);
        hotreload_free(hotreload);
        vm_free(vm);
        bytecode_module_free(module_v1);
        return 1;
    }
    
    printf("\n");
    
    // ========== 阶段 6: 运行更新后的代码 ==========
    printf("▶ Phase 6: Running reloaded code...\n\n");
    
    // 重置 VM 状态 (保留全局变量)
    vm_reset_execution_state(vm);
    
    err = vm_run(vm);
    if (err != OK) {
        fprintf(stderr, "Error: VM execution failed after reload (code=%d)\n", err);
        hotreload_free(hotreload);
        vm_free(vm);
        return 1;
    }
    
    printf("\n");
    
    // ========== 总结 ==========
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║                    测试结果                             ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    
    const HotReloadStats* stats = hotreload_get_stats(hotreload);
    printf("║ 热更新次数:     %5u                                 ║\n", stats->reload_count);
    printf("║ 更新函数数:     %5u                                 ║\n", stats->functions_updated);
    printf("║ 新增函数数:     %5u                                 ║\n", stats->functions_added);
    printf("║ 删除函数数:     %5u                                 ║\n", stats->functions_deleted);
    printf("║ 耗时 (ms):      %5llu                                ║\n", 
           (unsigned long long)stats->last_reload_time);
    printf("╚════════════════════════════════════════════════════════╝\n");
    
    printf("\n✓ Hot reload test completed successfully!\n\n");
    
    // 清理
    hotreload_free(hotreload);
    vm_free(vm);
    // 注意：module_v1 已经被热更新管理器管理，不需要手动释放
    mmgr_cleanup();
    
    return 0;
}
