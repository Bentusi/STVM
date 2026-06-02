/**
 * @file hotreload.h
 * @brief 热更新管理器 - 支持运行时代码更新
 * 
 * 功能特性：
 * 1. 函数级热更新
 * 2. 安全点检测
 * 3. 模块暂存和延迟应用
 * 4. 兼容性验证
 * 
 * 使用流程：
 * 1. 创建热更新管理器: hotreload_create()
 * 2. 暂存新模块: hotreload_stage_module()
 * 3. 检查安全性: hotreload_is_safe()
 * 4. 应用更新: hotreload_apply_staged()
 * 5. 清理: hotreload_free()
 */

#ifndef STVM_HOTRELOAD_H
#define STVM_HOTRELOAD_H

#include "vm.h"
#include "bytecode.h"
#include "error.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 函数映射信息 - 记录新旧函数的对应关系
 */
typedef struct FunctionMapping {
    char* name;                     // 函数名
    FunctionEntry* old_func;        // 旧函数入口
    FunctionEntry* new_func;        // 新函数入口
    bool is_modified;               // 是否被修改
    bool is_new;                    // 是否是新增函数
    bool is_deleted;                // 是否被删除
    struct FunctionMapping* next;   // 链表指针
} FunctionMapping;

/**
 * @brief 热更新操作类型
 */
typedef enum {
    HOTRELOAD_OP_NONE,              // 无操作
    HOTRELOAD_OP_UPDATE_FUNCTION,   // 更新函数
    HOTRELOAD_OP_ADD_FUNCTION,      // 添加函数
    HOTRELOAD_OP_DELETE_FUNCTION,   // 删除函数
    HOTRELOAD_OP_REPLACE_MODULE     // 替换整个模块
} HotReloadOperation;

/**
 * @brief 热更新统计信息
 */
typedef struct HotReloadStats {
    uint32_t functions_updated;     // 已更新的函数数
    uint32_t functions_added;       // 新增的函数数
    uint32_t functions_deleted;     // 删除的函数数
    uint32_t constants_merged;      // 合并的常量数
    uint32_t instructions_added;    // 新增的指令数
    uint64_t last_reload_time;      // 上次热更新时间戳（毫秒）
    uint32_t reload_count;          // 热更新次数
} HotReloadStats;

/**
 * @brief 热更新管理器
 */
typedef struct HotReloadManager {
    VM* vm;                         // 虚拟机实例
    BytecodeModule* active_module;  // 当前活动模块（原始）
    BytecodeModule* staged_module;  // 暂存的新模块
    FunctionMapping* mappings;      // 函数映射表
    bool reload_pending;            // 是否有待处理的更新
    HotReloadOperation pending_op;  // 待处理的操作类型
    HotReloadStats stats;           // 统计信息
    
    // 配置选项
    bool allow_unsafe_reload;       // 是否允许非安全点热更新（慎用！）
    bool preserve_global_state;     // 是否保留全局变量状态
    bool verbose;                   // 是否输出详细日志
} HotReloadManager;

/**
 * @brief 创建热更新管理器
 * @param vm 虚拟机实例
 * @return 热更新管理器实例，失败返回 NULL
 */
HotReloadManager* hotreload_create(VM* vm);

/**
 * @brief 释放热更新管理器
 * @param mgr 热更新管理器实例
 */
void hotreload_free(HotReloadManager* mgr);

/**
 * @brief 暂存新模块（不立即生效）
 * @param mgr 热更新管理器实例
 * @param bytecode_file 新的字节码文件路径
 * @return 错误码
 * 
 * 此函数加载新模块并分析差异，但不立即应用更新。
 * 调用 hotreload_apply_staged() 来应用更新。
 */
ErrorCode hotreload_stage_module(HotReloadManager* mgr, const char* bytecode_file);

/**
 * @brief 暂存新模块（从内存加载）
 * @param mgr 热更新管理器实例
 * @param new_module 新的字节码模块（管理器不拥有所有权）
 * @return 错误码
 */
ErrorCode hotreload_stage_module_from_memory(HotReloadManager* mgr, BytecodeModule* new_module);

/**
 * @brief 检查是否可以安全地应用热更新
 * @param mgr 热更新管理器实例
 * @return true 如果可以安全热更新
 * 
 * 安全条件：
 * - 调用栈为空或只在主循环中
 * - 没有正在执行待更新的函数
 * - 模块兼容性检查通过
 */
bool hotreload_is_safe(HotReloadManager* mgr);

/**
 * @brief 应用暂存的更新
 * @param mgr 热更新管理器实例
 * @return 错误码
 * 
 * 此函数将暂存的模块应用到运行中的 VM。
 * 建议在调用前先调用 hotreload_is_safe() 检查。
 */
ErrorCode hotreload_apply_staged(HotReloadManager* mgr);

/**
 * @brief 取消暂存的更新
 * @param mgr 热更新管理器实例
 */
void hotreload_cancel_staged(HotReloadManager* mgr);

/**
 * @brief 检查模块兼容性
 * @param mgr 热更新管理器实例
 * @param new_module 新模块
 * @return true 如果兼容
 * 
 * 检查项：
 * - 全局变量数量和类型
 * - 函数签名兼容性
 * - 依赖库版本
 */
bool hotreload_check_compatibility(HotReloadManager* mgr, BytecodeModule* new_module);

/**
 * @brief 获取热更新统计信息
 * @param mgr 热更新管理器实例
 * @return 统计信息指针
 */
const HotReloadStats* hotreload_get_stats(HotReloadManager* mgr);

/**
 * @brief 重置统计信息
 * @param mgr 热更新管理器实例
 */
void hotreload_reset_stats(HotReloadManager* mgr);

/**
 * @brief 打印热更新差异（调试用）
 * @param mgr 热更新管理器实例
 */
void hotreload_dump_diff(HotReloadManager* mgr);

/**
 * @brief 设置详细日志模式
 * @param mgr 热更新管理器实例
 * @param verbose 是否详细输出
 */
void hotreload_set_verbose(HotReloadManager* mgr, bool verbose);

/**
 * @brief 一步式热更新（简化 API）
 * @param vm 虚拟机实例
 * @param bytecode_file 新的字节码文件路径
 * @param force 是否强制更新（跳过安全检查，慎用！）
 * @return 错误码
 * 
 * 此函数组合了 stage + check + apply 操作。
 * 推荐用于简单场景。复杂场景请使用分步 API。
 */
ErrorCode hotreload_update(VM* vm, const char* bytecode_file, bool force);

#endif // STVM_HOTRELOAD_H
