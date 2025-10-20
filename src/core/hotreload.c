/**
 * @file hotreload.c
 * @brief 热更新管理器实现
 */

#include "hotreload.h"
#include "mmgr.h"
#include "bytecode_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 前向声明
static ErrorCode analyze_module_diff(HotReloadManager* mgr);
static void free_function_mappings(FunctionMapping* mappings);
static FunctionMapping* create_function_mapping(const char* name, 
                                                 FunctionEntry* old_func, 
                                                 FunctionEntry* new_func);
static ErrorCode merge_modules(HotReloadManager* mgr);
static bool is_function_in_call_stack(VM* vm, const char* func_name);

/**
 * @brief 创建热更新管理器
 */
HotReloadManager* hotreload_create(VM* vm) {
    if (!vm) return NULL;
    
    HotReloadManager* mgr = (HotReloadManager*)mmgr_alloc(sizeof(HotReloadManager));
    if (!mgr) return NULL;
    
    memset(mgr, 0, sizeof(HotReloadManager));
    mgr->vm = vm;
    mgr->active_module = vm->module;
    mgr->preserve_global_state = true;  // 默认保留全局状态
    mgr->verbose = false;
    
    if (mgr->verbose) {
        printf("[HotReload] Manager created for VM %p\n", (void*)vm);
    }
    
    return mgr;
}

/**
 * @brief 释放热更新管理器
 */
void hotreload_free(HotReloadManager* mgr) {
    if (!mgr) return;
    
    if (mgr->verbose) {
        printf("[HotReload] Freeing manager, total reloads: %u\n", mgr->stats.reload_count);
    }
    
    // 清理暂存的模块
    if (mgr->staged_module && mgr->staged_module != mgr->active_module) {
        bytecode_module_free(mgr->staged_module);
    }
    
    // 清理函数映射
    free_function_mappings(mgr->mappings);
    
    mmgr_free(mgr);
}

/**
 * @brief 暂存新模块（从文件加载）
 */
ErrorCode hotreload_stage_module(HotReloadManager* mgr, const char* bytecode_file) {
    if (!mgr || !bytecode_file) return ERR_RUNTIME;
    
    if (mgr->verbose) {
        printf("[HotReload] Staging module from: %s\n", bytecode_file);
    }
    
    // 加载新模块
    BytecodeModule* new_module = bytecode_load(bytecode_file);
    if (!new_module) {
        fprintf(stderr, "[HotReload] Failed to load bytecode from: %s\n", bytecode_file);
        return ERR_RUNTIME;
    }
    
    return hotreload_stage_module_from_memory(mgr, new_module);
}

/**
 * @brief 暂存新模块（从内存加载）
 */
ErrorCode hotreload_stage_module_from_memory(HotReloadManager* mgr, BytecodeModule* new_module) {
    if (!mgr || !new_module) return ERR_RUNTIME;
    
    // 清理之前暂存的模块
    if (mgr->staged_module && mgr->staged_module != mgr->active_module) {
        bytecode_module_free(mgr->staged_module);
    }
    free_function_mappings(mgr->mappings);
    mgr->mappings = NULL;
    
    mgr->staged_module = new_module;
    mgr->reload_pending = true;
    mgr->pending_op = HOTRELOAD_OP_REPLACE_MODULE;
    
    // 分析模块差异
    ErrorCode err = analyze_module_diff(mgr);
    if (err != OK) {
        fprintf(stderr, "[HotReload] Failed to analyze module diff\n");
        bytecode_module_free(mgr->staged_module);
        mgr->staged_module = NULL;
        mgr->reload_pending = false;
        return err;
    }
    
    if (mgr->verbose) {
        printf("[HotReload] Module staged successfully\n");
        hotreload_dump_diff(mgr);
    }
    
    return OK;
}

/**
 * @brief 检查是否可以安全热更新
 */
bool hotreload_is_safe(HotReloadManager* mgr) {
    if (!mgr || !mgr->reload_pending) return false;
    
    // 如果允许非安全更新，直接返回 true（危险！）
    if (mgr->allow_unsafe_reload) {
        if (mgr->verbose) {
            printf("[HotReload] WARNING: Unsafe reload allowed!\n");
        }
        return true;
    }
    
    VM* vm = mgr->vm;
    
    // 检查 1: VM 必须不在运行状态或在主循环中
    if (vm->running && vm->call_sp > 0) {
        if (mgr->verbose) {
            printf("[HotReload] Unsafe: VM has active call stack (depth=%d)\n", vm->call_sp + 1);
        }
        return false;
    }
    
    // 检查 2: 确保没有正在执行待更新的函数
    FunctionMapping* mapping = mgr->mappings;
    while (mapping) {
        if (mapping->is_modified && is_function_in_call_stack(vm, mapping->name)) {
            if (mgr->verbose) {
                printf("[HotReload] Unsafe: Function '%s' is in call stack\n", mapping->name);
            }
            return false;
        }
        mapping = mapping->next;
    }
    
    // 检查 3: 兼容性验证
    if (!hotreload_check_compatibility(mgr, mgr->staged_module)) {
        if (mgr->verbose) {
            printf("[HotReload] Unsafe: Module compatibility check failed\n");
        }
        return false;
    }
    
    if (mgr->verbose) {
        printf("[HotReload] Safety check passed\n");
    }
    
    return true;
}

/**
 * @brief 应用暂存的更新
 */
ErrorCode hotreload_apply_staged(HotReloadManager* mgr) {
    if (!mgr || !mgr->reload_pending) {
        return ERR_RUNTIME;
    }
    
    if (mgr->verbose) {
        printf("[HotReload] Applying staged update...\n");
    }
    
    // 安全检查
    if (!mgr->allow_unsafe_reload && !hotreload_is_safe(mgr)) {
        fprintf(stderr, "[HotReload] Cannot apply update: safety check failed\n");
        return ERR_RUNTIME;
    }
    
    // 记录开始时间
    clock_t start = clock();
    
    // 合并模块
    ErrorCode err = merge_modules(mgr);
    if (err != OK) {
        fprintf(stderr, "[HotReload] Failed to merge modules\n");
        return err;
    }
    
    // 更新统计信息
    mgr->stats.reload_count++;
    mgr->stats.last_reload_time = (uint64_t)((clock() - start) * 1000 / CLOCKS_PER_SEC);
    
    // 清理暂存状态
    mgr->reload_pending = false;
    mgr->pending_op = HOTRELOAD_OP_NONE;
    
    if (mgr->verbose) {
        printf("[HotReload] Update applied successfully in %llu ms\n", 
               (unsigned long long)mgr->stats.last_reload_time);
        printf("[HotReload] Total reloads: %u\n", mgr->stats.reload_count);
    }
    
    return OK;
}

/**
 * @brief 取消暂存的更新
 */
void hotreload_cancel_staged(HotReloadManager* mgr) {
    if (!mgr) return;
    
    if (mgr->verbose) {
        printf("[HotReload] Cancelling staged update\n");
    }
    
    if (mgr->staged_module && mgr->staged_module != mgr->active_module) {
        bytecode_module_free(mgr->staged_module);
    }
    mgr->staged_module = NULL;
    
    free_function_mappings(mgr->mappings);
    mgr->mappings = NULL;
    
    mgr->reload_pending = false;
    mgr->pending_op = HOTRELOAD_OP_NONE;
}

/**
 * @brief 检查模块兼容性
 */
bool hotreload_check_compatibility(HotReloadManager* mgr, BytecodeModule* new_module) {
    if (!mgr || !new_module) return false;
    
    BytecodeModule* old_module = mgr->active_module;
    
    // 检查 1: 全局变量数量（当前实现要求相同）
    if (old_module->global_count != new_module->global_count) {
        if (mgr->verbose) {
            printf("[HotReload] Incompatible: Global count changed (%u -> %u)\n",
                   old_module->global_count, new_module->global_count);
        }
        // 注意：这是简化的检查，未来可以支持全局变量迁移
        return false;
    }
    
    // 检查 2: 确保关键函数签名不变（如果有入口函数）
    // TODO: 实现函数签名比较
    
    // 检查 3: 依赖库版本（如果有）
    // TODO: 实现库依赖检查
    
    return true;
}

/**
 * @brief 获取统计信息
 */
const HotReloadStats* hotreload_get_stats(HotReloadManager* mgr) {
    return mgr ? &mgr->stats : NULL;
}

/**
 * @brief 重置统计信息
 */
void hotreload_reset_stats(HotReloadManager* mgr) {
    if (mgr) {
        memset(&mgr->stats, 0, sizeof(HotReloadStats));
    }
}

/**
 * @brief 打印热更新差异
 */
void hotreload_dump_diff(HotReloadManager* mgr) {
    if (!mgr) return;
    
    printf("\n========== Hot Reload Diff ==========\n");
    printf("Functions Updated: %u\n", mgr->stats.functions_updated);
    printf("Functions Added:   %u\n", mgr->stats.functions_added);
    printf("Functions Deleted: %u\n", mgr->stats.functions_deleted);
    
    printf("\nFunction Mappings:\n");
    FunctionMapping* mapping = mgr->mappings;
    while (mapping) {
        printf("  %s: ", mapping->name);
        if (mapping->is_new) {
            printf("[NEW]\n");
        } else if (mapping->is_deleted) {
            printf("[DELETED]\n");
        } else if (mapping->is_modified) {
            printf("[MODIFIED]\n");
        } else {
            printf("[UNCHANGED]\n");
        }
        mapping = mapping->next;
    }
    printf("=====================================\n\n");
}

/**
 * @brief 设置详细日志
 */
void hotreload_set_verbose(HotReloadManager* mgr, bool verbose) {
    if (mgr) {
        mgr->verbose = verbose;
    }
}

/**
 * @brief 一步式热更新
 */
ErrorCode hotreload_update(VM* vm, const char* bytecode_file, bool force) {
    HotReloadManager* mgr = hotreload_create(vm);
    if (!mgr) return ERR_RUNTIME;
    
    mgr->allow_unsafe_reload = force;
    
    ErrorCode err = hotreload_stage_module(mgr, bytecode_file);
    if (err != OK) {
        hotreload_free(mgr);
        return err;
    }
    
    if (!force && !hotreload_is_safe(mgr)) {
        fprintf(stderr, "[HotReload] Update failed: not safe to reload\n");
        hotreload_free(mgr);
        return ERR_RUNTIME;
    }
    
    err = hotreload_apply_staged(mgr);
    hotreload_free(mgr);
    
    return err;
}

// ==================== 内部辅助函数 ====================

/**
 * @brief 分析模块差异
 */
static ErrorCode analyze_module_diff(HotReloadManager* mgr) {
    BytecodeModule* old_mod = mgr->active_module;
    BytecodeModule* new_mod = mgr->staged_module;
    
    if (!old_mod || !new_mod) return ERR_RUNTIME;
    
    // 重置统计
    mgr->stats.functions_updated = 0;
    mgr->stats.functions_added = 0;
    mgr->stats.functions_deleted = 0;
    
    // 构建函数映射表
    // 1. 遍历新模块的函数
    for (uint32_t i = 0; i < new_mod->function_count; i++) {
        FunctionEntry* new_func = &new_mod->functions[i];
        FunctionEntry* old_func = bytecode_find_function(old_mod, new_func->name);
        
        FunctionMapping* mapping = create_function_mapping(
            new_func->name, old_func, new_func);
        
        if (!old_func) {
            mapping->is_new = true;
            mgr->stats.functions_added++;
        } else {
            // 简单比较：检查地址或参数数量是否变化
            if (old_func->param_count != new_func->param_count ||
                old_func->local_count != new_func->local_count) {
                mapping->is_modified = true;
                mgr->stats.functions_updated++;
            }
        }
        
        // 添加到链表
        mapping->next = mgr->mappings;
        mgr->mappings = mapping;
    }
    
    // 2. 检查删除的函数
    for (uint32_t i = 0; i < old_mod->function_count; i++) {
        FunctionEntry* old_func = &old_mod->functions[i];
        FunctionEntry* new_func = bytecode_find_function(new_mod, old_func->name);
        
        if (!new_func) {
            FunctionMapping* mapping = create_function_mapping(
                old_func->name, old_func, NULL);
            mapping->is_deleted = true;
            mgr->stats.functions_deleted++;
            
            mapping->next = mgr->mappings;
            mgr->mappings = mapping;
        }
    }
    
    return OK;
}

/**
 * @brief 合并模块
 */
static ErrorCode merge_modules(HotReloadManager* mgr) {
    BytecodeModule* old_mod = mgr->active_module;
    BytecodeModule* new_mod = mgr->staged_module;
    
    if (!old_mod || !new_mod) return ERR_RUNTIME;
    
    // 策略：直接替换 VM 的模块指针
    // 注意：这是简化的实现，真实场景需要更复杂的合并逻辑
    
    if (mgr->verbose) {
        printf("[HotReload] Replacing module %p with %p\n", 
               (void*)old_mod, (void*)new_mod);
    }
    
    // 替换 VM 的模块
    mgr->vm->module = new_mod;
    mgr->active_module = new_mod;
    
    // 如果需要保留全局状态，复制全局变量
    if (mgr->preserve_global_state && old_mod->global_count == new_mod->global_count) {
        // 全局变量已经在 vm->globals 中，不需要额外操作
        if (mgr->verbose) {
            printf("[HotReload] Global state preserved (%u variables)\n", 
                   old_mod->global_count);
        }
    }
    
    // 重置 PC 到新模块的入口点
    mgr->vm->pc = new_mod->entry_point;
    
    // 注意：旧模块会在下次 hotreload_stage 或 hotreload_free 时释放
    
    return OK;
}

/**
 * @brief 创建函数映射
 */
static FunctionMapping* create_function_mapping(const char* name, 
                                                 FunctionEntry* old_func, 
                                                 FunctionEntry* new_func) {
    FunctionMapping* mapping = (FunctionMapping*)mmgr_alloc(sizeof(FunctionMapping));
    if (!mapping) return NULL;
    
    memset(mapping, 0, sizeof(FunctionMapping));
    mapping->name = mmgr_strdup(name);
    mapping->old_func = old_func;
    mapping->new_func = new_func;
    
    return mapping;
}

/**
 * @brief 释放函数映射链表
 */
static void free_function_mappings(FunctionMapping* mappings) {
    while (mappings) {
        FunctionMapping* next = mappings->next;
        mmgr_free(mappings->name);
        mmgr_free(mappings);
        mappings = next;
    }
}

/**
 * @brief 检查函数是否在调用栈中
 */
static bool is_function_in_call_stack(VM* vm, const char* func_name) {
    if (!vm || !func_name) return false;
    
    for (int32_t i = 0; i <= vm->call_sp; i++) {
        CallFrame* frame = &vm->call_stack[i];
        if (frame->function && frame->function->name &&
            strcmp(frame->function->name, func_name) == 0) {
            return true;
        }
    }
    
    return false;
}
