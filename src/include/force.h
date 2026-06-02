/**
 * @file force.h
 * @brief 变量强制功能 - 用于调试和测试
 * 
 * 提供强制变量值的功能，使变量在程序运行时保持特定值
 * 类似于PLC中的Force功能
 */

#ifndef STVM_FORCE_H
#define STVM_FORCE_H

#include "types.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief 强制状态枚举
 */
typedef enum {
    FORCE_NONE = 0,      // 未强制
    FORCE_VALUE,         // 强制为特定值
    FORCE_UNFORCE        // 标记为取消强制
} ForceStatus;

/**
 * @brief 强制变量信息
 */
typedef struct ForceVariable {
    char* name;                    // 变量名（可以是限定名）
    int32_t var_index;             // 变量在VM中的索引（-1表示未解析）
    Value forced_value;            // 强制的值
    Value original_value;          // 原始值（用于恢复）
    ForceStatus status;            // 强制状态
    bool is_persistent;            // 是否持久化强制
    uint64_t force_timestamp;      // 强制时间戳
    struct ForceVariable* next;    // 链表下一个节点
} ForceVariable;

/**
 * @brief 强制管理器
 */
typedef struct ForceManager {
    ForceVariable* force_list;     // 强制变量链表
    int32_t force_count;           // 当前强制变量数量
    int32_t max_forces;            // 最大允许强制数（0=无限制）
    bool force_enabled;            // 全局强制使能
    bool force_locked;             // 强制锁定（防止运行时修改）
    bool warn_on_force;            // 强制时是否警告
} ForceManager;

/* ========== 强制管理器生命周期 ========== */

/**
 * @brief 创建强制管理器
 * @return 新的强制管理器实例
 */
ForceManager* force_manager_create(void);

/**
 * @brief 销毁强制管理器并释放所有资源
 * @param mgr 强制管理器指针
 */
void force_manager_destroy(ForceManager* mgr);

/**
 * @brief 重置强制管理器（移除所有强制）
 * @param mgr 强制管理器指针
 */
void force_manager_reset(ForceManager* mgr);

/* ========== 强制操作 ========== */

/**
 * @brief 强制变量为特定值
 * @param mgr 强制管理器
 * @param var_name 变量名
 * @param value 强制的值
 * @param persistent 是否持久化（跨周期保持）
 * @return 成功返回true，失败返回false
 */
bool force_variable(ForceManager* mgr, const char* var_name, Value value, bool persistent);

/**
 * @brief 通过索引强制变量
 * @param mgr 强制管理器
 * @param var_index 变量索引
 * @param value 强制的值
 * @param persistent 是否持久化
 * @return 成功返回true，失败返回false
 */
bool force_variable_by_index(ForceManager* mgr, int32_t var_index, Value value, bool persistent);

/**
 * @brief 取消变量强制
 * @param mgr 强制管理器
 * @param var_name 变量名
 * @return 成功返回true，失败返回false
 */
bool unforce_variable(ForceManager* mgr, const char* var_name);

/**
 * @brief 通过索引取消变量强制
 * @param mgr 强制管理器
 * @param var_index 变量索引
 * @return 成功返回true，失败返回false
 */
bool unforce_variable_by_index(ForceManager* mgr, int32_t var_index);

/**
 * @brief 取消所有变量强制
 * @param mgr 强制管理器
 * @return 取消的强制数量
 */
int32_t unforce_all(ForceManager* mgr);

/**
 * @brief 更新强制值（变量已被强制时）
 * @param mgr 强制管理器
 * @param var_name 变量名
 * @param new_value 新的强制值
 * @return 成功返回true，失败返回false
 */
bool force_update_value(ForceManager* mgr, const char* var_name, Value new_value);

/* ========== 查询和状态 ========== */

/**
 * @brief 查找强制变量
 * @param mgr 强制管理器
 * @param var_name 变量名
 * @return 找到返回ForceVariable指针，否则返回NULL
 */
ForceVariable* force_find_variable(ForceManager* mgr, const char* var_name);

/**
 * @brief 通过索引查找强制变量
 * @param mgr 强制管理器
 * @param var_index 变量索引
 * @return 找到返回ForceVariable指针，否则返回NULL
 */
ForceVariable* force_find_by_index(ForceManager* mgr, int32_t var_index);

/**
 * @brief 检查变量是否被强制
 * @param mgr 强制管理器
 * @param var_name 变量名
 * @return 被强制返回true，否则返回false
 */
bool force_is_forced(ForceManager* mgr, const char* var_name);

/**
 * @brief 通过索引检查变量是否被强制
 * @param mgr 强制管理器
 * @param var_index 变量索引
 * @return 被强制返回true，否则返回false
 */
bool force_is_forced_by_index(ForceManager* mgr, int32_t var_index);

/**
 * @brief 获取强制值
 * @param mgr 强制管理器
 * @param var_name 变量名
 * @return 被强制返回强制值指针，否则返回NULL
 */
Value* force_get_forced_value(ForceManager* mgr, const char* var_name);

/**
 * @brief 通过索引获取强制值
 * @param mgr 强制管理器
 * @param var_index 变量索引
 * @return 被强制返回强制值指针，否则返回NULL
 */
Value* force_get_forced_value_by_index(ForceManager* mgr, int32_t var_index);

/**
 * @brief 获取强制数量
 * @param mgr 强制管理器
 * @return 当前强制变量数量
 */
int32_t force_get_count(ForceManager* mgr);

/* ========== 强制使能控制 ========== */

/**
 * @brief 启用/禁用全局强制
 * @param mgr 强制管理器
 * @param enable true启用，false禁用
 */
void force_enable(ForceManager* mgr, bool enable);

/**
 * @brief 检查全局强制是否启用
 * @param mgr 强制管理器
 * @return 启用返回true，否则返回false
 */
bool force_is_enabled(ForceManager* mgr);

/**
 * @brief 锁定/解锁强制管理器
 * @param mgr 强制管理器
 * @param lock true锁定，false解锁
 */
void force_lock(ForceManager* mgr, bool lock);

/**
 * @brief 检查强制管理器是否锁定
 * @param mgr 强制管理器
 * @return 锁定返回true，否则返回false
 */
bool force_is_locked(ForceManager* mgr);

/**
 * @brief 设置最大强制数量限制
 * @param mgr 强制管理器
 * @param max_forces 最大数量（0=无限制）
 */
void force_set_max_forces(ForceManager* mgr, int32_t max_forces);

/* ========== 调试和导出 ========== */

/**
 * @brief 打印所有强制变量状态
 * @param mgr 强制管理器
 */
void force_print_status(ForceManager* mgr);

/**
 * @brief 保存强制列表到文件
 * @param mgr 强制管理器
 * @param filename 文件名
 * @return 成功返回true，失败返回false
 */
bool force_save_to_file(ForceManager* mgr, const char* filename);

/**
 * @brief 从文件加载强制列表
 * @param mgr 强制管理器
 * @param filename 文件名
 * @return 成功返回true，失败返回false
 */
bool force_load_from_file(ForceManager* mgr, const char* filename);

/**
 * @brief 导出强制列表为JSON格式字符串
 * @param mgr 强制管理器
 * @return JSON字符串（需要调用者释放）
 */
char* force_export_json(ForceManager* mgr);

/**
 * @brief 从JSON字符串导入强制列表
 * @param mgr 强制管理器
 * @param json JSON字符串
 * @return 成功返回true，失败返回false
 */
bool force_import_json(ForceManager* mgr, const char* json);

#endif // STVM_FORCE_H
