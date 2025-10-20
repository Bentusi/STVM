/**
 * @file vm_hotreload_integration.h
 * @brief VM 热加载集成接口
 * 
 * 此文件定义了将热加载功能集成到 VM 的 API
 */

#ifndef STVM_VM_HOTRELOAD_INTEGRATION_H
#define STVM_VM_HOTRELOAD_INTEGRATION_H

#include "vm.h"
#include "hotreload.h"
#include "error.h"

/**
 * @brief 启用虚拟机的热加载功能
 * 
 * @param vm 虚拟机实例
 * @param auto_apply 是否自动应用更新（true=自动，false=手动）
 * @param check_interval 检查间隔（执行多少条指令后检查一次）
 *                       0 = 每次主循环迭代都检查
 *                       1000 = 每执行1000条指令检查一次（推荐）
 * @return 错误码
 * 
 * @example
 * VM* vm = vm_create(module);
 * vm_enable_hotreload(vm, true, 1000);  // 每1000条指令检查，自动应用
 */
ErrorCode vm_enable_hotreload(VM* vm, bool auto_apply, uint32_t check_interval);

/**
 * @brief 禁用虚拟机的热加载功能
 * 
 * @param vm 虚拟机实例
 * 
 * 释放热加载管理器并禁用所有热加载功能。
 */
void vm_disable_hotreload(VM* vm);

/**
 * @brief 手动触发热加载更新
 * 
 * @param vm 虚拟机实例
 * @param bytecode_file 新的字节码文件路径
 * @param force 是否强制更新（跳过安全检查）
 * @return 错误码
 * 
 * 此函数会：
 * 1. 加载新字节码模块
 * 2. 检查兼容性
 * 3. 如果安全（或 force=true），立即应用更新
 * 
 * @warning force=true 会跳过安全检查，可能导致运行时错误！
 * 
 * @example
 * // 安全更新
 * ErrorCode err = vm_trigger_hotreload(vm, "app_v2.stbc", false);
 * if (err != OK) {
 *     fprintf(stderr, "Hotreload failed: %d\n", err);
 * }
 */
ErrorCode vm_trigger_hotreload(VM* vm, const char* bytecode_file, bool force);

/**
 * @brief 暂存新模块（不立即应用）
 * 
 * @param vm 虚拟机实例
 * @param bytecode_file 新的字节码文件路径
 * @return 错误码
 * 
 * 此函数只加载和分析新模块，不立即应用。
 * 需要后续调用 vm_apply_hotreload() 来应用更新。
 * 
 * @example
 * // 两步式更新
 * vm_stage_hotreload(vm, "app_v2.stbc");
 * // ... 等待合适时机 ...
 * if (vm_is_hotreload_safe(vm)) {
 *     vm_apply_hotreload(vm);
 * }
 */
ErrorCode vm_stage_hotreload(VM* vm, const char* bytecode_file);

/**
 * @brief 应用已暂存的热更新
 * 
 * @param vm 虚拟机实例
 * @return 错误码
 * 
 * 应用之前通过 vm_stage_hotreload() 暂存的更新。
 */
ErrorCode vm_apply_hotreload(VM* vm);

/**
 * @brief 检查热加载是否安全
 * 
 * @param vm 虚拟机实例
 * @return true 如果可以安全地应用热更新
 * 
 * 安全条件：
 * - 调用栈为空或只在主程序中
 * - 没有正在执行待更新的函数
 * - 模块兼容性检查通过
 */
bool vm_is_hotreload_safe(VM* vm);

/**
 * @brief 取消已暂存的热更新
 * 
 * @param vm 虚拟机实例
 */
void vm_cancel_hotreload(VM* vm);

/**
 * @brief 获取热加载统计信息
 * 
 * @param vm 虚拟机实例
 * @return 统计信息指针，如果未启用热加载则返回 NULL
 */
const HotReloadStats* vm_get_hotreload_stats(VM* vm);

/**
 * @brief 内部函数：检查并应用待处理的热更新
 * 
 * @param vm 虚拟机实例
 * @return 错误码
 * 
 * 此函数在 VM 执行循环中被调用，不应直接使用。
 */
ErrorCode vm_check_hotreload(VM* vm);

#endif // STVM_VM_HOTRELOAD_INTEGRATION_H
