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
        
        char value_buf[64];  // 增大缓冲区以容纳质量位信息
        switch (current->forced_value.type) {
            case TYPE_INT:
                snprintf(value_buf, sizeof(value_buf), "%d", current->forced_value.int_val);
                break;
            case TYPE_QINT:
                snprintf(value_buf, sizeof(value_buf), "%d [%s]", 
                         current->forced_value.int_val, 
                         quality_to_string(current->forced_value.quality));
                break;
            case TYPE_REAL:
                snprintf(value_buf, sizeof(value_buf), "%.6f", current->forced_value.real_val);
                break;
            case TYPE_QREAL:
                snprintf(value_buf, sizeof(value_buf), "%.6f [%s]", 
                         current->forced_value.real_val, 
                         quality_to_string(current->forced_value.quality));
                break;
            case TYPE_BOOL:
                snprintf(value_buf, sizeof(value_buf), "%s", current->forced_value.bool_val ? "TRUE" : "FALSE");
                break;
            case TYPE_QBOOL:
                snprintf(value_buf, sizeof(value_buf), "%s [%s]", 
                         current->forced_value.bool_val ? "TRUE" : "FALSE", 
                         quality_to_string(current->forced_value.quality));
                break;
            case TYPE_STRING:
                snprintf(value_buf, sizeof(value_buf), "\"%s\"", current->forced_value.string_val ? current->forced_value.string_val : "");
                break;
            case TYPE_QSTRING:
                snprintf(value_buf, sizeof(value_buf), "\"%s\" [%s]", 
                         current->forced_value.string_val ? current->forced_value.string_val : "", 
                         quality_to_string(current->forced_value.quality));
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
        
        // 对于质量化类型，保存质量位
        switch (current->forced_value.type) {
            case TYPE_QINT:
            case TYPE_QREAL:
            case TYPE_QBOOL:
            case TYPE_QSTRING:
                fprintf(f, "quality=%s\n", quality_to_string(current->forced_value.quality));
                break;
            default:
                break;
        }
        
        switch (current->forced_value.type) {
            case TYPE_INT:
            case TYPE_QINT:
                fprintf(f, "value=%d\n", current->forced_value.int_val);
                break;
            case TYPE_REAL:
            case TYPE_QREAL:
                fprintf(f, "value=%f\n", current->forced_value.real_val);
                break;
            case TYPE_BOOL:
            case TYPE_QBOOL:
                fprintf(f, "value=%d\n", current->forced_value.bool_val);
                break;
            case TYPE_STRING:
            case TYPE_QSTRING:
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
    if (!mgr || !filename) return false;
    
    FILE* f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Force: Cannot open file '%s' for reading\n", filename);
        return false;
    }
    
    char line[512];
    char var_name[256] = {0};
    int var_index = -1;
    DataType value_type = TYPE_VOID;
    Value value = {0};
    bool persistent = false;
    bool has_var = false;
    int loaded_count = 0;
    
    while (fgets(line, sizeof(line), f)) {
        // 移除换行符
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        // 跳过注释和空行
        if (line[0] == '#' || line[0] == '\0') {
            continue;
        }
        
        // 解析配置项
        char key[256], val[256];
        if (sscanf(line, "%255[^=]=%255s", key, val) == 2) {
            // 全局配置
            if (strcmp(key, "force_enabled") == 0) {
                mgr->force_enabled = atoi(val) != 0;
            }
            else if (strcmp(key, "force_count") == 0) {
                // 仅用于验证，实际计数由添加操作更新
                continue;
            }
            // 变量配置
            else if (strcmp(key, "force_var") == 0) {
                strncpy(var_name, val, sizeof(var_name) - 1);
                var_name[sizeof(var_name) - 1] = '\0';
                var_index = -1;
                has_var = true;
            }
            else if (strcmp(key, "force_index") == 0) {
                var_index = atoi(val);
                var_name[0] = '\0';
                has_var = true;
            }
            else if (strcmp(key, "type") == 0) {
                // 解析类型字符串
                if (strcmp(val, "int") == 0) {
                    value_type = TYPE_INT;
                } else if (strcmp(val, "real") == 0) {
                    value_type = TYPE_REAL;
                } else if (strcmp(val, "bool") == 0) {
                    value_type = TYPE_BOOL;
                } else if (strcmp(val, "string") == 0) {
                    value_type = TYPE_STRING;
                } else if (strcmp(val, "qint") == 0) {
                    value_type = TYPE_QINT;
                } else if (strcmp(val, "qreal") == 0) {
                    value_type = TYPE_QREAL;
                } else if (strcmp(val, "qbool") == 0) {
                    value_type = TYPE_QBOOL;
                } else if (strcmp(val, "qstring") == 0) {
                    value_type = TYPE_QSTRING;
                } else {
                    value_type = TYPE_VOID;
                }
                // 初始化value
                value.type = value_type;
                value.quality = QUALITY_GOOD;  // 默认质量为GOOD
            }
            else if (strcmp(key, "quality") == 0) {
                // 解析质量位（仅对质量化类型有效）
                value.quality = string_to_quality(val);
            }
            else if (strcmp(key, "value") == 0) {
                // 根据类型解析值
                switch (value_type) {
                    case TYPE_INT:
                    case TYPE_QINT:
                        value.int_val = atoi(val);
                        break;
                    case TYPE_REAL:
                    case TYPE_QREAL:
                        value.real_val = atof(val);
                        break;
                    case TYPE_BOOL:
                    case TYPE_QBOOL:
                        value.bool_val = atoi(val) != 0;
                        break;
                    case TYPE_STRING:
                    case TYPE_QSTRING:
                        strncpy(value.string_val, val, sizeof(value.string_val) - 1);
                        value.string_val[sizeof(value.string_val) - 1] = '\0';
                        break;
                    default:
                        break;
                }
            }
            else if (strcmp(key, "persistent") == 0) {
                persistent = atoi(val) != 0;
                
                // 此时已有完整的变量信息，添加强制
                if (has_var && value_type != TYPE_VOID) {
                    bool success = false;
                    
                    if (var_name[0] != '\0') {
                        // 按名称强制
                        success = force_variable(mgr, var_name, value, persistent);
                    } else if (var_index >= 0) {
                        // 按索引强制
                        success = force_variable_by_index(mgr, var_index, value, persistent);
                    }
                    
                    if (success) {
                        loaded_count++;
                    }
                    
                    // 重置状态
                    var_name[0] = '\0';
                    var_index = -1;
                    value_type = TYPE_VOID;
                    persistent = false;
                    has_var = false;
                }
            }
        }
    }
    
    fclose(f);
    
    if (loaded_count > 0) {
        printf("Force: Loaded %d force variables from '%s'\n", loaded_count, filename);
    } else {
        printf("Force: No valid force variables found in '%s'\n", filename);
    }
    
    return loaded_count > 0;
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
