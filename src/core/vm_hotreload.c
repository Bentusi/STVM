/**
 * @file vm_hotreload.c
 * @brief VM 热加载集成实现
 */

#include "vm.h"
#include "hotreload.h"
#include "mmgr.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief 启用虚拟机的热加载功能
 */
ErrorCode vm_enable_hotreload(VM* vm, bool auto_apply, uint32_t check_interval) {
    if (!vm) return ERR_RUNTIME;
    
    // 如果已启用，先清理
    if (vm->hotreload) {
        vm_disable_hotreload(vm);
    }
    
    // 创建热加载管理器
    vm->hotreload = hotreload_create(vm);
    if (!vm->hotreload) {
        fprintf(stderr, "[VM] Failed to create hotreload manager\n");
        return ERR_OUT_OF_MEMORY;
    }
    
    // 配置
    vm->hotreload_enabled = true;
    vm->hotreload_auto_apply = auto_apply;
    vm->hotreload_check_interval = check_interval;
    vm->instructions_since_check = 0;
    
    // 设置详细输出
    hotreload_set_verbose(vm->hotreload, true);
    
    printf("[VM] Hotreload enabled (auto_apply=%d, check_interval=%u)\n", 
           auto_apply, check_interval);
    
    return OK;
}

/**
 * @brief 禁用虚拟机的热加载功能
 */
void vm_disable_hotreload(VM* vm) {
    if (!vm) return;
    
    if (vm->hotreload) {
        hotreload_free(vm->hotreload);
        vm->hotreload = NULL;
    }
    
    vm->hotreload_enabled = false;
    vm->hotreload_auto_apply = false;
    vm->hotreload_check_interval = 0;
    vm->instructions_since_check = 0;
    
    printf("[VM] Hotreload disabled\n");
}

/**
 * @brief 手动触发热加载更新
 */
ErrorCode vm_trigger_hotreload(VM* vm, const char* bytecode_file, bool force) {
    if (!vm || !bytecode_file) return ERR_RUNTIME;
    
    // 如果未启用，临时创建管理器
    bool temp_manager = false;
    if (!vm->hotreload) {
        vm->hotreload = hotreload_create(vm);
        if (!vm->hotreload) {
            fprintf(stderr, "[VM] Failed to create temporary hotreload manager\n");
            return ERR_OUT_OF_MEMORY;
        }
        temp_manager = true;
        hotreload_set_verbose(vm->hotreload, true);
    }
    
    ErrorCode err;
    
    printf("[VM] Triggering hotreload: %s (force=%d)\n", bytecode_file, force);
    
    // 暂存新模块
    err = hotreload_stage_module(vm->hotreload, bytecode_file);
    if (err != OK) {
        fprintf(stderr, "[VM] Failed to stage module: error code %d\n", err);
        goto cleanup;
    }
    
    // 检查安全性
    if (!force) {
        if (!hotreload_is_safe(vm->hotreload)) {
            fprintf(stderr, "[VM] Hotreload not safe at this point\n");
            fprintf(stderr, "[VM] Hint: Use force=true to override (risky!)\n");
            err = ERR_RUNTIME;
            goto cleanup;
        }
    } else {
        printf("[VM] Warning: Forcing hotreload (safety checks bypassed)\n");
    }
    
    // 应用更新
    err = hotreload_apply_staged(vm->hotreload);
    if (err != OK) {
        fprintf(stderr, "[VM] Failed to apply hotreload: error code %d\n", err);
        goto cleanup;
    }
    
    printf("[VM] ✓ Hotreload applied successfully\n");
    
    // 打印统计信息
    const HotReloadStats* stats = hotreload_get_stats(vm->hotreload);
    if (stats) {
        printf("[VM] Functions updated: %u, added: %u, deleted: %u\n",
               stats->functions_updated, stats->functions_added, stats->functions_deleted);
        printf("[VM] Reload time: %llu ms\n", (unsigned long long)stats->last_reload_time);
    }
    
cleanup:
    if (temp_manager) {
        hotreload_free(vm->hotreload);
        vm->hotreload = NULL;
    }
    
    return err;
}

/**
 * @brief 暂存新模块（不立即应用）
 */
ErrorCode vm_stage_hotreload(VM* vm, const char* bytecode_file) {
    if (!vm || !bytecode_file) return ERR_RUNTIME;
    
    if (!vm->hotreload) {
        fprintf(stderr, "[VM] Hotreload not enabled, call vm_enable_hotreload first\n");
        return ERR_RUNTIME;
    }
    
    printf("[VM] Staging hotreload module: %s\n", bytecode_file);
    
    ErrorCode err = hotreload_stage_module(vm->hotreload, bytecode_file);
    if (err != OK) {
        fprintf(stderr, "[VM] Failed to stage module: error code %d\n", err);
        return err;
    }
    
    printf("[VM] Module staged successfully\n");
    printf("[VM] Call vm_apply_hotreload() when safe to apply\n");
    
    return OK;
}

/**
 * @brief 应用已暂存的热更新
 */
ErrorCode vm_apply_hotreload(VM* vm) {
    if (!vm || !vm->hotreload) {
        fprintf(stderr, "[VM] Hotreload not enabled\n");
        return ERR_RUNTIME;
    }
    
    if (!vm->hotreload->reload_pending) {
        fprintf(stderr, "[VM] No pending hotreload to apply\n");
        return ERR_RUNTIME;
    }
    
    printf("[VM] Applying staged hotreload...\n");
    
    // 检查安全性
    if (!hotreload_is_safe(vm->hotreload)) {
        fprintf(stderr, "[VM] Hotreload not safe at this point\n");
        return ERR_RUNTIME;
    }
    
    ErrorCode err = hotreload_apply_staged(vm->hotreload);
    if (err != OK) {
        fprintf(stderr, "[VM] Failed to apply hotreload: error code %d\n", err);
        return err;
    }
    
    printf("[VM] ✓ Hotreload applied successfully\n");
    
    // 打印统计信息
    const HotReloadStats* stats = hotreload_get_stats(vm->hotreload);
    if (stats) {
        printf("[VM] Functions updated: %u, added: %u, deleted: %u\n",
               stats->functions_updated, stats->functions_added, stats->functions_deleted);
    }
    
    return OK;
}

/**
 * @brief 检查热加载是否安全
 */
bool vm_is_hotreload_safe(VM* vm) {
    if (!vm || !vm->hotreload) return false;
    return hotreload_is_safe(vm->hotreload);
}

/**
 * @brief 取消已暂存的热更新
 */
void vm_cancel_hotreload(VM* vm) {
    if (!vm || !vm->hotreload) return;
    
    printf("[VM] Cancelling staged hotreload\n");
    hotreload_cancel_staged(vm->hotreload);
}

/**
 * @brief 获取热加载统计信息
 */
const struct HotReloadStats* vm_get_hotreload_stats(VM* vm) {
    if (!vm || !vm->hotreload) return NULL;
    return hotreload_get_stats(vm->hotreload);
}

/**
 * @brief 内部函数：检查并应用待处理的热更新
 */
ErrorCode vm_check_hotreload(VM* vm) {
    if (!vm || !vm->hotreload) return OK;
    
    // 检查是否有待处理的更新
    if (!vm->hotreload->reload_pending) {
        return OK; // 没有更新
    }
    
    // 如果启用自动应用
    if (vm->hotreload_auto_apply) {
        // 检查安全性
        if (hotreload_is_safe(vm->hotreload)) {
            ErrorCode err = hotreload_apply_staged(vm->hotreload);
            if (err == OK) {
                printf("[VM] Auto-applied hotreload update\n");
                
                // 打印简要统计
                const HotReloadStats* stats = hotreload_get_stats(vm->hotreload);
                if (stats) {
                    printf("[VM] Updated: %u functions, Total reloads: %u\n",
                           stats->functions_updated, stats->reload_count);
                }
            } else {
                fprintf(stderr, "[VM] Auto-apply failed: error code %d\n", err);
            }
            return err;
        }
        // 不安全，稍后重试
    }
    
    return OK;
}
