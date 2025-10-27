/**
 * @file force.c
 * @brief 变量强制功能实现
 */

#include "force.h"
#include "mmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ========== 内部辅助函数 ========== */

/**
 * @brief 创建新的强制变量节点
 */
static ForceVariable* create_force_variable(const char* name, int32_t index, Value value, bool persistent) {
    ForceVariable* fv = (ForceVariable*)mmgr_alloc(sizeof(ForceVariable));
    if (!fv) return NULL;
    
    fv->name = name ? mmgr_strdup(name) : NULL;
    fv->var_index = index;
    fv->forced_value = value;
    fv->original_value = (Value){.type = TYPE_VOID};
    fv->status = FORCE_VALUE;
    fv->is_persistent = persistent;
    fv->force_timestamp = (uint64_t)time(NULL);
    fv->next = NULL;
    
    return fv;
}

/**
 * @brief 释放强制变量节点
 */
static void free_force_variable(ForceVariable* fv) {
    if (!fv) return;
    
    if (fv->name) {
        mmgr_free(fv->name);
    }
    
    // 如果值包含动态分配的数据（如字符串），需要释放
    if (fv->forced_value.type == TYPE_STRING && fv->forced_value.string_val) {
        mmgr_free(fv->forced_value.string_val);
    }
    if (fv->original_value.type == TYPE_STRING && fv->original_value.string_val) {
        mmgr_free(fv->original_value.string_val);
    }
    
    mmgr_free(fv);
}

/* ========== 强制管理器生命周期 ========== */

ForceManager* force_manager_create(void) {
    ForceManager* mgr = (ForceManager*)mmgr_alloc(sizeof(ForceManager));
    if (!mgr) return NULL;
    
    mgr->force_list = NULL;
    mgr->force_count = 0;
    mgr->max_forces = 0;  // 无限制
    mgr->force_enabled = true;
    mgr->force_locked = false;
    mgr->warn_on_force = false;
    
    return mgr;
}

void force_manager_destroy(ForceManager* mgr) {
    if (!mgr) return;
    
    force_manager_reset(mgr);
    mmgr_free(mgr);
}

void force_manager_reset(ForceManager* mgr) {
    if (!mgr) return;
    
    ForceVariable* current = mgr->force_list;
    while (current) {
        ForceVariable* next = current->next;
        free_force_variable(current);
        current = next;
    }
    
    mgr->force_list = NULL;
    mgr->force_count = 0;
}

/* ========== 强制操作 ========== */

bool force_variable(ForceManager* mgr, const char* var_name, Value value, bool persistent) {
    if (!mgr || !var_name) return false;
    
    // 检查是否锁定
    if (mgr->force_locked) {
        fprintf(stderr, "Force: Manager is locked, cannot add force\n");
        return false;
    }
    
    // 检查是否达到最大数量
    if (mgr->max_forces > 0 && mgr->force_count >= mgr->max_forces) {
        fprintf(stderr, "Force: Maximum force count (%d) reached\n", mgr->max_forces);
        return false;
    }
    
    // 检查是否已存在
    ForceVariable* existing = force_find_variable(mgr, var_name);
    if (existing) {
        // 更新现有强制
        if (existing->forced_value.type == TYPE_STRING && existing->forced_value.string_val) {
            mmgr_free(existing->forced_value.string_val);
        }
        existing->forced_value = value;
        existing->is_persistent = persistent;
        existing->status = FORCE_VALUE;
        existing->force_timestamp = (uint64_t)time(NULL);
        
        if (mgr->warn_on_force) {
            printf("Force: Updated force on variable '%s'\n", var_name);
        }
        return true;
    }
    
    // 创建新的强制
    ForceVariable* fv = create_force_variable(var_name, -1, value, persistent);
    if (!fv) return false;
    
    // 添加到链表头
    fv->next = mgr->force_list;
    mgr->force_list = fv;
    mgr->force_count++;
    
    if (mgr->warn_on_force) {
        printf("Force: Forced variable '%s'\n", var_name);
    }
    
    return true;
}

bool force_variable_by_index(ForceManager* mgr, int32_t var_index, Value value, bool persistent) {
    if (!mgr || var_index < 0) return false;
    
    if (mgr->force_locked) {
        fprintf(stderr, "Force: Manager is locked\n");
        return false;
    }
    
    // 检查是否已存在
    ForceVariable* existing = force_find_by_index(mgr, var_index);
    if (existing) {
        // 更新
        if (existing->forced_value.type == TYPE_STRING && existing->forced_value.string_val) {
            mmgr_free(existing->forced_value.string_val);
        }
        existing->forced_value = value;
        existing->is_persistent = persistent;
        existing->status = FORCE_VALUE;
        existing->force_timestamp = (uint64_t)time(NULL);
        return true;
    }
    
    // 创建新的强制
    ForceVariable* fv = create_force_variable(NULL, var_index, value, persistent);
    if (!fv) return false;
    
    fv->next = mgr->force_list;
    mgr->force_list = fv;
    mgr->force_count++;
    
    return true;
}

bool unforce_variable(ForceManager* mgr, const char* var_name) {
    if (!mgr || !var_name) return false;
    
    if (mgr->force_locked) {
        fprintf(stderr, "Force: Manager is locked\n");
        return false;
    }
    
    ForceVariable* prev = NULL;
    ForceVariable* current = mgr->force_list;
    
    while (current) {
        if (current->name && strcmp(current->name, var_name) == 0) {
            // 找到，从链表移除
            if (prev) {
                prev->next = current->next;
            } else {
                mgr->force_list = current->next;
            }
            
            free_force_variable(current);
            mgr->force_count--;
            
            if (mgr->warn_on_force) {
                printf("Force: Unforced variable '%s'\n", var_name);
            }
            return true;
        }
        prev = current;
        current = current->next;
    }
    
    return false;
}

bool unforce_variable_by_index(ForceManager* mgr, int32_t var_index) {
    if (!mgr || var_index < 0) return false;
    
    if (mgr->force_locked) return false;
    
    ForceVariable* prev = NULL;
    ForceVariable* current = mgr->force_list;
    
    while (current) {
        if (current->var_index == var_index) {
            if (prev) {
                prev->next = current->next;
            } else {
                mgr->force_list = current->next;
            }
            
            free_force_variable(current);
            mgr->force_count--;
            return true;
        }
        prev = current;
        current = current->next;
    }
    
    return false;
}

int32_t unforce_all(ForceManager* mgr) {
    if (!mgr) return 0;
    
    if (mgr->force_locked) {
        fprintf(stderr, "Force: Manager is locked\n");
        return 0;
    }
    
    int32_t count = mgr->force_count;
    force_manager_reset(mgr);
    
    if (mgr->warn_on_force && count > 0) {
        printf("Force: Removed all %d forces\n", count);
    }
    
    return count;
}

bool force_update_value(ForceManager* mgr, const char* var_name, Value new_value) {
    if (!mgr || !var_name) return false;
    
    ForceVariable* fv = force_find_variable(mgr, var_name);
    if (!fv || fv->status != FORCE_VALUE) {
        return false;
    }
    
    // 释放旧值（如果是字符串）
    if (fv->forced_value.type == TYPE_STRING && fv->forced_value.string_val) {
        mmgr_free(fv->forced_value.string_val);
    }
    
    fv->forced_value = new_value;
    fv->force_timestamp = (uint64_t)time(NULL);
    
    return true;
}

/* ========== 查询和状态 ========== */

ForceVariable* force_find_variable(ForceManager* mgr, const char* var_name) {
    if (!mgr || !var_name) return NULL;
    
    ForceVariable* current = mgr->force_list;
    while (current) {
        if (current->name && strcmp(current->name, var_name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

ForceVariable* force_find_by_index(ForceManager* mgr, int32_t var_index) {
    if (!mgr || var_index < 0) return NULL;
    
    ForceVariable* current = mgr->force_list;
    while (current) {
        if (current->var_index == var_index) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

bool force_is_forced(ForceManager* mgr, const char* var_name) {
    if (!mgr || !mgr->force_enabled) return false;
    
    ForceVariable* fv = force_find_variable(mgr, var_name);
    return (fv && fv->status == FORCE_VALUE);
}

bool force_is_forced_by_index(ForceManager* mgr, int32_t var_index) {
    if (!mgr || !mgr->force_enabled) return false;
    
    ForceVariable* fv = force_find_by_index(mgr, var_index);
    return (fv && fv->status == FORCE_VALUE);
}

Value* force_get_forced_value(ForceManager* mgr, const char* var_name) {
    if (!mgr || !mgr->force_enabled) return NULL;
    
    ForceVariable* fv = force_find_variable(mgr, var_name);
    if (fv && fv->status == FORCE_VALUE) {
        return &fv->forced_value;
    }
    
    return NULL;
}

Value* force_get_forced_value_by_index(ForceManager* mgr, int32_t var_index) {
    if (!mgr || !mgr->force_enabled) return NULL;
    
    ForceVariable* fv = force_find_by_index(mgr, var_index);
    if (fv && fv->status == FORCE_VALUE) {
        return &fv->forced_value;
    }
    
    return NULL;
}

int32_t force_get_count(ForceManager* mgr) {
    return mgr ? mgr->force_count : 0;
}

/* ========== 强制使能控制 ========== */

void force_enable(ForceManager* mgr, bool enable) {
    if (mgr) {
        mgr->force_enabled = enable;
        if (mgr->warn_on_force) {
            printf("Force: Global force %s\n", enable ? "enabled" : "disabled");
        }
    }
}

bool force_is_enabled(ForceManager* mgr) {
    return mgr ? mgr->force_enabled : false;
}

void force_lock(ForceManager* mgr, bool lock) {
    if (mgr) {
        mgr->force_locked = lock;
        if (mgr->warn_on_force) {
            printf("Force: Manager %s\n", lock ? "locked" : "unlocked");
        }
    }
}

bool force_is_locked(ForceManager* mgr) {
    return mgr ? mgr->force_locked : false;
}

void force_set_max_forces(ForceManager* mgr, int32_t max_forces) {
    if (mgr) {
        mgr->max_forces = max_forces;
    }
}

/* ========== 调试和导出 ========== */

void force_print_status(ForceManager* mgr) {
    if (!mgr) return;
    
    printf("\n=== Force Manager Status ===\n");
    printf("Force Enabled: %s\n", mgr->force_enabled ? "YES" : "NO");
    printf("Force Locked: %s\n", mgr->force_locked ? "YES" : "NO");
    printf("Force Count: %d\n", mgr->force_count);
    printf("Max Forces: %d\n", mgr->max_forces);
    
    if (mgr->force_count == 0) {
        printf("No forced variables\n");
        return;
    }
    
    printf("\nForced Variables:\n");
    printf("%-30s %-10s %-20s %s\n", "Name/Index", "Type", "Forced Value", "Persistent");
    printf("%-30s %-10s %-20s %s\n", "----------", "----", "------------", "----------");
    
    ForceVariable* current = mgr->force_list;
    while (current) {
        char name_buf[32];
        if (current->name) {
            snprintf(name_buf, sizeof(name_buf), "%s", current->name);
        } else {
            snprintf(name_buf, sizeof(name_buf), "[%d]", current->var_index);
        }
        
        char value_buf[32];
        switch (current->forced_value.type) {
            case TYPE_INT:
                snprintf(value_buf, sizeof(value_buf), "%d", current->forced_value.int_val);
                break;
            case TYPE_REAL:
                snprintf(value_buf, sizeof(value_buf), "%.6f", current->forced_value.real_val);
                break;
            case TYPE_BOOL:
                snprintf(value_buf, sizeof(value_buf), "%s", current->forced_value.bool_val ? "TRUE" : "FALSE");
                break;
            case TYPE_STRING:
                snprintf(value_buf, sizeof(value_buf), "\"%s\"", current->forced_value.string_val ? current->forced_value.string_val : "");
                break;
            default:
                snprintf(value_buf, sizeof(value_buf), "(unknown)");
                break;
        }
        
        printf("%-30s %-10s %-20s %s\n", 
               name_buf,
               type_to_string(current->forced_value.type),
               value_buf,
               current->is_persistent ? "YES" : "NO");
        
        current = current->next;
    }
    printf("\n");
}

bool force_save_to_file(ForceManager* mgr, const char* filename) {
    if (!mgr || !filename) return false;
    
    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Force: Cannot open file '%s' for writing\n", filename);
        return false;
    }
    
    fprintf(f, "# STVM Force File\n");
    fprintf(f, "# Generated: %ld\n", (long)time(NULL));
    fprintf(f, "force_enabled=%d\n", mgr->force_enabled);
    fprintf(f, "force_count=%d\n\n", mgr->force_count);
    
    ForceVariable* current = mgr->force_list;
    while (current) {
        if (current->name) {
            fprintf(f, "force_var=%s\n", current->name);
        } else {
            fprintf(f, "force_index=%d\n", current->var_index);
        }
        
        fprintf(f, "type=%s\n", type_to_string(current->forced_value.type));
        
        switch (current->forced_value.type) {
            case TYPE_INT:
                fprintf(f, "value=%d\n", current->forced_value.int_val);
                break;
            case TYPE_REAL:
                fprintf(f, "value=%f\n", current->forced_value.real_val);
                break;
            case TYPE_BOOL:
                fprintf(f, "value=%d\n", current->forced_value.bool_val);
                break;
            case TYPE_STRING:
                fprintf(f, "value=%s\n", current->forced_value.string_val ? current->forced_value.string_val : "");
                break;
            default:
                break;
        }
        
        fprintf(f, "persistent=%d\n\n", current->is_persistent);
        current = current->next;
    }
    
    fclose(f);
    return true;
}

bool force_load_from_file(ForceManager* mgr, const char* filename) {
    // TODO: 实现从文件加载
    // 需要解析文件格式并重建强制列表
    fprintf(stderr, "Force: load_from_file not yet implemented\n");
    return false;
}

char* force_export_json(ForceManager* mgr) {
    // TODO: 实现JSON导出
    fprintf(stderr, "Force: export_json not yet implemented\n");
    return NULL;
}

bool force_import_json(ForceManager* mgr, const char* json) {
    // TODO: 实现JSON导入
    fprintf(stderr, "Force: import_json not yet implemented\n");
    return false;
}
