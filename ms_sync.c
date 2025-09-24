/*
 * IEC61131 ST语言主从同步系统实现
 * 支持双机热备、状态同步和故障切换
 */

#include "ms_sync.h"
#include "mmgr.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <fcntl.h>

/* 内部状态变量 */
static uint64_t g_start_time = 0;
static uint32_t g_message_sequence = 1;

/* 内部辅助函数声明 */
static int ms_sync_setup_socket_options(int sockfd);
static int ms_sync_update_peer_connection_state(master_slave_sync_t *sync_mgr);
static int ms_sync_process_single_message(master_slave_sync_t *sync_mgr, const sync_message_t *msg);
static sync_var_info_t* ms_sync_find_var_by_index(master_slave_sync_t *sync_mgr, uint32_t vm_var_index);
static sync_var_info_t* ms_sync_find_var_by_name(master_slave_sync_t *sync_mgr, const char *name);

/* ========== 主从同步管理函数 ========== */

/* 创建主从同步管理器 */
master_slave_sync_t* ms_sync_create(void) {
    master_slave_sync_t *sync_mgr = (master_slave_sync_t*)mmgr_alloc_general(sizeof(master_slave_sync_t));
    if (!sync_mgr) {
        return NULL;
    }
    
    memset(sync_mgr, 0, sizeof(master_slave_sync_t));
    
    /* 初始化默认值 */
    sync_mgr->role = NODE_ROLE_STANDALONE;
    sync_mgr->state = NODE_STATE_INIT;
    sync_mgr->sync_port = DEFAULT_SYNC_PORT;
    sync_mgr->local_conn.socket_fd = -1;
    sync_mgr->peer_conn.socket_fd = -1;
    sync_mgr->checkpoint_interval_ms = 1000; /* 默认1秒检查点间隔 */
    
    /* 记录启动时间 */
    if (g_start_time == 0) {
        g_start_time = ms_sync_get_timestamp();
    }
    
    return sync_mgr;
}

/* 销毁主从同步管理器 */
void ms_sync_destroy(master_slave_sync_t *sync_mgr) {
    if (!sync_mgr) return;
    
    /* 停止网络连接 */
    ms_sync_stop_network(sync_mgr);
    
    /* 内存由MMGR管理 */
}

/* 初始化主从同步管理器 */
int ms_sync_init(master_slave_sync_t *sync_mgr, const sync_config_t *config, 
                 node_role_t role, struct st_vm *vm) {
    if (!sync_mgr || !config || !vm) {
        return -1;
    }
    
    /* 保存配置 */
    strncpy(sync_mgr->local_ip, config->local_ip, sizeof(sync_mgr->local_ip) - 1);
    strncpy(sync_mgr->peer_ip, config->peer_ip, sizeof(sync_mgr->peer_ip) - 1);
    sync_mgr->sync_port = config->sync_port;
    sync_mgr->checkpoint_interval_ms = config->checkpoint_interval_ms;
    
    /* 设置角色和状态 */
    sync_mgr->role = role;
    sync_mgr->state = (role == NODE_ROLE_PRIMARY) ? NODE_STATE_ACTIVE : NODE_STATE_STANDBY;
    sync_mgr->vm = vm;
    
    /* 初始化连接状态 */
    sync_mgr->peer_alive = false;
    sync_mgr->last_heartbeat_time = ms_sync_get_timestamp();
    sync_mgr->peer_last_heartbeat_time = 0;
    
    ms_sync_log(sync_mgr, "INFO", "Sync manager initialized: role=%s, local_ip=%s, peer_ip=%s, port=%d",
               ms_sync_role_to_string(role), config->local_ip, config->peer_ip, config->sync_port);
    
    return 0;
}

/* ========== 网络连接管理 ========== */

/* 启动网络连接 */
int ms_sync_start_network(master_slave_sync_t *sync_mgr) {
    if (!sync_mgr) {
        return -1;
    }
    
    /* 创建本地套接字 */
    if (ms_sync_create_socket(sync_mgr) != 0) {
        ms_sync_set_error(sync_mgr, "Failed to create socket");
        return -1;
    }
    
    /* 绑定本地地址 */
    if (ms_sync_bind_socket(sync_mgr) != 0) {
        ms_sync_set_error(sync_mgr, "Failed to bind socket");
        return -1;
    }
    
    /* 尝试连接到对端 */
    if (ms_sync_connect_to_peer(sync_mgr) != 0) {
        ms_sync_log(sync_mgr, "WARN", "Failed to connect to peer, will retry later");
    }
    
    sync_mgr->sync_enabled = true;
    
    ms_sync_log(sync_mgr, "INFO", "Network started successfully");
    
    return 0;
}

/* 停止网络连接 */
int ms_sync_stop_network(master_slave_sync_t *sync_mgr) {
    if (!sync_mgr) {
        return -1;
    }
    
    sync_mgr->sync_enabled = false;
    ms_sync_close_connection(sync_mgr);
    
    ms_sync_log(sync_mgr, "INFO", "Network stopped");
    
    return 0;
}

/* 检查连接状态 */
bool ms_sync_is_connected(const master_slave_sync_t *sync_mgr) {
    if (!sync_mgr) {
        return false;
    }
    
    return sync_mgr->peer_conn.is_connected && sync_mgr->peer_alive;
}

/* ========== 同步变量管理 ========== */

/* 注册同步变量 */
int ms_sync_register_variable(master_slave_sync_t *sync_mgr, const char *var_name,
                              uint32_t vm_var_index, uint32_t var_type, uint32_t var_size) {
    if (!sync_mgr || !var_name || sync_mgr->sync_var_count >= MAX_SYNC_VARIABLES) {
        return -1;
    }
    
    /* 检查是否已存在 */
    if (ms_sync_find_var_by_name(sync_mgr, var_name) != NULL) {
        ms_sync_set_error(sync_mgr, "Variable already registered");
        return -1;
    }
    
    /* 添加新的同步变量 */
    sync_var_info_t *var_info = &sync_mgr->sync_vars[sync_mgr->sync_var_count];
    
    strncpy(var_info->name, var_name, sizeof(var_info->name) - 1);
    var_info->name[sizeof(var_info->name) - 1] = '\0';
    var_info->vm_var_index = vm_var_index;
    var_info->type = var_type;
    var_info->size = var_size;
    var_info->is_dirty = false;
    var_info->last_sync_time = 0;
    
    sync_mgr->sync_var_count++;
    
    ms_sync_log(sync_mgr, "INFO", "Registered sync variable: %s (index=%u, type=%u, size=%u)",
               var_name, vm_var_index, var_type, var_size);
    
    return 0;
}

/* 注销同步变量 */
int ms_sync_unregister_variable(master_slave_sync_t *sync_mgr, const char *var_name) {
    if (!sync_mgr || !var_name) {
        return -1;
    }
    
    /* 查找变量 */
    for (uint32_t i = 0; i < sync_mgr->sync_var_count; i++) {
        if (strcmp(sync_mgr->sync_vars[i].name, var_name) == 0) {
            /* 移动后续元素 */
            memmove(&sync_mgr->sync_vars[i], &sync_mgr->sync_vars[i + 1],
                   (sync_mgr->sync_var_count - i - 1) * sizeof(sync_var_info_t));
            sync_mgr->sync_var_count--;
            
            ms_sync_log(sync_mgr, "INFO", "Unregistered sync variable: %s", var_name);
            return 0;
        }
    }
    
    ms_sync_set_error(sync_mgr, "Variable not found");
    return -1;
}

/* 标记变量为脏 */
int ms_sync_mark_variable_dirty(master_slave_sync_t *sync_mgr, uint32_t vm_var_index) {
    if (!sync_mgr) {
        return -1;
    }
    
    sync_var_info_t *var_info = ms_sync_find_var_by_index(sync_mgr, vm_var_index);
    if (!var_info) {
        return -1; /* 变量未注册，忽略 */
    }
    
    var_info->is_dirty = true;
    
    return 0;
}

/* ========== 同步操作 ========== */

/* 发送变量更新 */
int ms_sync_send_variable_update(master_slave_sync_t *sync_mgr, uint32_t vm_var_index) {
    if (!sync_mgr || !ms_sync_is_connected(sync_mgr)) {
        return -1;
    }
    
    sync_var_info_t *var_info = ms_sync_find_var_by_index(sync_mgr, vm_var_index);
    if (!var_info) {
        return -1;
    }
    
    sync_message_t msg;
    if (ms_sync_create_var_sync_msg(sync_mgr, vm_var_index, &msg) != 0) {
        return -1;
    }
    
    if (ms_sync_send_message(sync_mgr, &msg) != 0) {
        return -1;
    }
    
    var_info->is_dirty = false;
    var_info->last_sync_time = ms_sync_get_timestamp();
    
    return 0;
}

/* 发送心跳 */
int ms_sync_send_heartbeat(master_slave_sync_t *sync_mgr) {
    if (!sync_mgr) {
        return -1;
    }
    
    sync_message_t msg;
    if (ms_sync_create_heartbeat_msg(sync_mgr, &msg) != 0) {
        return -1;
    }
    
    if (ms_sync_send_message(sync_mgr, &msg) != 0) {
        return -1;
    }
    
    sync_mgr->last_heartbeat_time = ms_sync_get_timestamp();
    sync_mgr->stats.heartbeats_sent++;
    
    return 0;
}

/* 发送检查点 */
int ms_sync_send_checkpoint(master_slave_sync_t *sync_mgr) {
    if (!sync_mgr || !ms_sync_is_connected(sync_mgr)) {
        return -1;
    }
    
    sync_message_t msg;
    if (ms_sync_create_checkpoint_msg(sync_mgr, &msg) != 0) {
        return -1;
    }
    
    if (ms_sync_send_message(sync_mgr, &msg) != 0) {
        return -1;
    }
    
    sync_mgr->last_checkpoint_time = ms_sync_get_timestamp();
    sync_mgr->last_checkpoint_id++;
    
    return 0;
}

/* 处理消息 */
int ms_sync_process_messages(master_slave_sync_t *sync_mgr) {
    if (!sync_mgr || !sync_mgr->sync_enabled) {
        return -1;
    }
    
    sync_message_t msg;
    int result;
    
    /* 尝试接收消息 */
    while ((result = ms_sync_receive_message(sync_mgr, &msg)) == 0) {
        if (ms_sync_process_single_message(sync_mgr, &msg) != 0) {
            ms_sync_log(sync_mgr, "ERROR", "Failed to process message type %s",
                       ms_sync_msg_type_to_string(msg.header.type));
        }
    }
    
    /* 检查心跳超时 */
    if (ms_sync_check_peer_health(sync_mgr) != 0) {
        return -1;
    }
    
    /* 发送心跳（如果需要） */
    uint64_t now = ms_sync_get_timestamp();
    if (now - sync_mgr->last_heartbeat_time > HEARTBEAT_INTERVAL_MS) {
        ms_sync_send_heartbeat(sync_mgr);
    }
    
    /* 发送检查点（如果需要） */
    if (sync_mgr->role == NODE_ROLE_PRIMARY &&
        now - sync_mgr->last_checkpoint_time > sync_mgr->checkpoint_interval_ms) {
        ms_sync_send_checkpoint(sync_mgr);
    }
    
    /* 同步脏变量 */
    if (sync_mgr->role == NODE_ROLE_PRIMARY) {
        for (uint32_t i = 0; i < sync_mgr->sync_var_count; i++) {
            if (sync_mgr->sync_vars[i].is_dirty) {
                ms_sync_send_variable_update(sync_mgr, sync_mgr->sync_vars[i].vm_var_index);
            }
        }
    }
    
    return 0;
}

/* ========== 角色管理和故障切换 ========== */

/* 设置角色 */
int ms_sync_set_role(master_slave_sync_t *sync_mgr, node_role_t new_role) {
    if (!sync_mgr) {
        return -1;
    }
    
    node_role_t old_role = sync_mgr->role;
    sync_mgr->role = new_role;
    
    /* 更新状态 */
    switch (new_role) {
        case NODE_ROLE_PRIMARY:
            sync_mgr->state = NODE_STATE_ACTIVE;
            break;
        case NODE_ROLE_SECONDARY:
            sync_mgr->state = NODE_STATE_STANDBY;
            break;
        case NODE_ROLE_STANDALONE:
            sync_mgr->state = NODE_STATE_ACTIVE;
            break;
    }
    
    ms_sync_log(sync_mgr, "INFO", "Role changed from %s to %s",
               ms_sync_role_to_string(old_role), ms_sync_role_to_string(new_role));
    
    return 0;
}

/* 获取角色 */
node_role_t ms_sync_get_role(const master_slave_sync_t *sync_mgr) {
    return sync_mgr ? sync_mgr->role : NODE_ROLE_STANDALONE;
}

/* 启动故障切换 */
int ms_sync_initiate_failover(master_slave_sync_t *sync_mgr) {
    if (!sync_mgr || sync_mgr->role != NODE_ROLE_SECONDARY) {
        return -1;
    }
    
    ms_sync_log(sync_mgr, "WARN", "Initiating failover from secondary to primary");
    
    /* 更改角色 */
    if (ms_sync_set_role(sync_mgr, NODE_ROLE_PRIMARY) != 0) {
        return -1;
    }
    
    /* 更新统计 */
    sync_mgr->stats.failovers++;
    
    /* 通知VM切换到主节点模式 */
    if (sync_mgr->vm) {
        /* VM状态切换逻辑 */
        sync_mgr->vm->sync_mode = VM_SYNC_PRIMARY;
    }
    
    ms_sync_log(sync_mgr, "INFO", "Failover completed successfully");
    
    return 0;
}

/* 检查是否应该接管 */
bool ms_sync_should_takeover(const master_slave_sync_t *sync_mgr) {
    if (!sync_mgr || sync_mgr->role != NODE_ROLE_SECONDARY) {
        return false;
    }
    
    /* 检查对端是否失效 */
    if (!sync_mgr->peer_alive) {
        return true;
    }
    
    /* 检查心跳超时 */
    uint64_t now = ms_sync_get_timestamp();
    if (sync_mgr->peer_last_heartbeat_time > 0 &&
        now - sync_mgr->peer_last_heartbeat_time > HEARTBEAT_TIMEOUT_MS * 3) {
        return true;
    }
    
    return false;
}

/* ========== 健康检查 ========== */

/* 检查对端是否存活 */
bool ms_sync_is_peer_alive(const master_slave_sync_t *sync_mgr) {
    return sync_mgr ? sync_mgr->peer_alive : false;
}

/* 获取对端运行时间 */
uint64_t ms_sync_get_peer_uptime(const master_slave_sync_t *sync_mgr) {
    /* 简化实现：返回0 */
    return 0;
}

/* 检查对端健康状态 */
int ms_sync_check_peer_health(master_slave_sync_t *sync_mgr) {
    if (!sync_mgr) {
        return -1;
    }
    
    uint64_t now = ms_sync_get_timestamp();
    
    /* 检查心跳超时 */
    if (sync_mgr->peer_last_heartbeat_time > 0 &&
        now - sync_mgr->peer_last_heartbeat_time > HEARTBEAT_TIMEOUT_MS) {
        
        sync_mgr->heartbeat_timeout_count++;
        
        if (sync_mgr->heartbeat_timeout_count >= 3) {
            if (sync_mgr->peer_alive) {
                ms_sync_log(sync_mgr, "WARN", "Peer heartbeat timeout, marking as failed");
                sync_mgr->peer_alive = false;
                
                /* 如果是从节点且应该接管，执行故障切换 */
                if (ms_sync_should_takeover(sync_mgr)) {
                    ms_sync_initiate_failover(sync_mgr);
                }
            }
        }
    } else {
        sync_mgr->heartbeat_timeout_count = 0;
        if (!sync_mgr->peer_alive && sync_mgr->peer_last_heartbeat_time > 0) {
            ms_sync_log(sync_mgr, "INFO", "Peer recovered");
            sync_mgr->peer_alive = true;
        }
    }
    
    return 0;
}

/* ========== 消息处理函数 ========== */

/* 创建心跳消息 */
int ms_sync_create_heartbeat_msg(const master_slave_sync_t *sync_mgr, sync_message_t *msg) {
    if (!sync_mgr || !msg) {
        return -1;
    }
    
    memset(msg, 0, sizeof(sync_message_t));
    
    /* 填充消息头 */
    msg->header.magic = SYNC_MAGIC;
    msg->header.sequence = g_message_sequence++;
    msg->header.type = MSG_TYPE_HEARTBEAT;
    msg->header.payload_size = sizeof(heartbeat_msg_t);
    msg->header.timestamp = ms_sync_get_timestamp();
    
    /* 填充心跳数据 */
    msg->payload.heartbeat.role = sync_mgr->role;
    msg->payload.heartbeat.state = sync_mgr->state;
    msg->payload.heartbeat.vm_pc = sync_mgr->vm ? sync_mgr->vm->pc : 0;
    msg->payload.heartbeat.sync_var_count = sync_mgr->sync_var_count;
    msg->payload.heartbeat.uptime = ms_sync_get_uptime();
    
    /* 计算校验和 */
    msg->header.checksum = ms_sync_calculate_checksum(&msg->payload, msg->header.payload_size);
    
    return 0;
}

/* 创建变量同步消息 */
int ms_sync_create_var_sync_msg(const master_slave_sync_t *sync_mgr, uint32_t var_index, 
                                sync_message_t *msg) {
    if (!sync_mgr || !msg || !sync_mgr->vm) {
        return -1;
    }
    
    sync_var_info_t *var_info = ms_sync_find_var_by_index((master_slave_sync_t*)sync_mgr, var_index);
    if (!var_info) {
        return -1;
    }
    
    memset(msg, 0, sizeof(sync_message_t));
    
    /* 填充消息头 */
    msg->header.magic = SYNC_MAGIC;
    msg->header.sequence = g_message_sequence++;
    msg->header.type = MSG_TYPE_VAR_SYNC;
    msg->header.payload_size = sizeof(var_sync_msg_t);
    msg->header.timestamp = ms_sync_get_timestamp();
    
    /* 填充变量数据 */
    msg->payload.var_sync.var_index = var_index;
    msg->payload.var_sync.var_type = var_info->type;
    
    /* 从VM获取变量值 */
    if (var_index < MAX_GLOBAL_VARS && sync_mgr->vm->global_vars) {
        vm_value_t *vm_var = &sync_mgr->vm->global_vars[var_index];
        switch (vm_var->type) {
            case VAL_BOOL:
                msg->payload.var_sync.value.bool_val = vm_var->data.bool_val;
                break;
            case VAL_INT:
                msg->payload.var_sync.value.int_val = vm_var->data.int_val;
                break;
            case VAL_REAL:
                msg->payload.var_sync.value.real_val = vm_var->data.real_val;
                break;
            case VAL_STRING:
                if (vm_var->data.string_val) {
                    strncpy(msg->payload.var_sync.value.string_val.data,
                           vm_var->data.string_val, sizeof(msg->payload.var_sync.value.string_val.data) - 1);
                    msg->payload.var_sync.value.string_val.length = strlen(vm_var->data.string_val);
                }
                break;
        }
    }
    
    /* 计算校验和 */
    msg->header.checksum = ms_sync_calculate_checksum(&msg->payload, msg->header.payload_size);
    
    return 0;
}

/* 创建状态同步消息 */
int ms_sync_create_state_sync_msg(const master_slave_sync_t *sync_mgr, sync_message_t *msg) {
    if (!sync_mgr || !msg) {
        return -1;
    }
    
    memset(msg, 0, sizeof(sync_message_t));
    
    /* 填充消息头 */
    msg->header.magic = SYNC_MAGIC;
    msg->header.sequence = g_message_sequence++;
    msg->header.type = MSG_TYPE_STATE_SYNC;
    msg->header.payload_size = sizeof(state_sync_msg_t);
    msg->header.timestamp = ms_sync_get_timestamp();
    
    /* 填充状态数据 */
    if (sync_mgr->vm) {
        msg->payload.state_sync.vm_pc = sync_mgr->vm->pc;
        msg->payload.state_sync.stack_depth = sync_mgr->vm->operand_stack.top;
        msg->payload.state_sync.call_stack_depth = sync_mgr->vm->call_stack_top;
    }
    msg->payload.state_sync.node_state = sync_mgr->state;
    
    /* 计算校验和 */
    msg->header.checksum = ms_sync_calculate_checksum(&msg->payload, msg->header.payload_size);
    
    return 0;
}

/* 创建检查点消息 */
int ms_sync_create_checkpoint_msg(const master_slave_sync_t *sync_mgr, sync_message_t *msg) {
    if (!sync_mgr || !msg) {
        return -1;
    }
    
    memset(msg, 0, sizeof(sync_message_t));
    
    /* 填充消息头 */
    msg->header.magic = SYNC_MAGIC;
    msg->header.sequence = g_message_sequence++;
    msg->header.type = MSG_TYPE_CHECKPOINT;
    msg->header.payload_size = sizeof(checkpoint_msg_t);
    msg->header.timestamp = ms_sync_get_timestamp();
    
    /* 填充检查点数据 */
    msg->payload.checkpoint.checkpoint_id = sync_mgr->last_checkpoint_id + 1;
    msg->payload.checkpoint.var_count = sync_mgr->sync_var_count;
    
    /* 创建变量快照 */
    if (sync_mgr->vm && sync_mgr->vm->global_vars) {
        uint32_t offset = 0;
        for (uint32_t i = 0; i < sync_mgr->sync_var_count && offset < sizeof(msg->payload.checkpoint.data); i++) {
            sync_var_info_t *var_info = &sync_mgr->sync_vars[i];
            if (var_info->vm_var_index < MAX_GLOBAL_VARS) {
                vm_value_t *vm_var = &sync_mgr->vm->global_vars[var_info->vm_var_index];
                
                /* 简化：只复制基础数据类型 */
                if (offset + sizeof(vm_value_t) <= sizeof(msg->payload.checkpoint.data)) {
                    memcpy(&msg->payload.checkpoint.data[offset], vm_var, sizeof(vm_value_t));
                    offset += sizeof(vm_value_t);
                }
            }
        }
    }
    
    /* 计算校验和 */
    msg->header.checksum = ms_sync_calculate_checksum(&msg->payload, msg->header.payload_size);
    
    return 0;
}

/* 处理心跳消息 */
int ms_sync_handle_heartbeat(master_slave_sync_t *sync_mgr, const heartbeat_msg_t *msg) {
    if (!sync_mgr || !msg) {
        return -1;
    }
    
    sync_mgr->peer_last_heartbeat_time = ms_sync_get_timestamp();
    sync_mgr->peer_alive = true;
    sync_mgr->heartbeat_timeout_count = 0;
    sync_mgr->stats.heartbeats_received++;
    
    /* 检查角色冲突 */
    if (msg->role == NODE_ROLE_PRIMARY && sync_mgr->role == NODE_ROLE_PRIMARY) {
        ms_sync_log(sync_mgr, "WARN", "Dual primary detected! Setting self to secondary");
        ms_sync_set_role(sync_mgr, NODE_ROLE_SECONDARY);
    }
    
    return 0;
}

/* 处理变量同步消息 */
int ms_sync_handle_var_sync(master_slave_sync_t *sync_mgr, const var_sync_msg_t *msg) {
    if (!sync_mgr || !msg || !sync_mgr->vm || !sync_mgr->vm->global_vars) {
        return -1;
    }
    
    /* 只有从节点处理变量同步 */
    if (sync_mgr->role != NODE_ROLE_SECONDARY) {
        return 0;
    }
    
    /* 更新VM中的变量值 */
    uint32_t var_index = msg->var_index;
    if (var_index < MAX_GLOBAL_VARS) {
        vm_value_t *vm_var = &sync_mgr->vm->global_vars[var_index];
        
        switch (msg->var_type) {
            case VAL_BOOL:
                vm_var->type = VAL_BOOL;
                vm_var->data.bool_val = msg->value.bool_val;
                break;
            case VAL_INT:
                vm_var->type = VAL_INT;
                vm_var->data.int_val = msg->value.int_val;
                break;
            case VAL_REAL:
                vm_var->type = VAL_REAL;
                vm_var->data.real_val = msg->value.real_val;
                break;
            case VAL_STRING:
                vm_var->type = VAL_STRING;
                if (msg->value.string_val.length > 0) {
                    /* 分配新字符串 */
                    vm_var->data.string_val = mmgr_alloc_string_with_length(
                        msg->value.string_val.data, msg->value.string_val.length);
                }
                break;
        }
        
        /* 标记变量已同步 */
        sync_var_info_t *var_info = ms_sync_find_var_by_index(sync_mgr, var_index);
        if (var_info) {
            var_info->last_sync_time = ms_sync_get_timestamp();
            var_info->is_dirty = false;
        }
    }
    
    return 0;
}

/* 处理状态同步消息 */
int ms_sync_handle_state_sync(master_slave_sync_t *sync_mgr, const state_sync_msg_t *msg) {
    if (!sync_mgr || !msg || !sync_mgr->vm) {
        return -1;
    }
    
    /* 只有从节点同步状态 */
    if (sync_mgr->role != NODE_ROLE_SECONDARY) {
        return 0;
    }
    
    /* 同步VM状态 */
    sync_mgr->vm->pc = msg->vm_pc;
    /* 注意：栈状态同步需要谨慎处理，这里简化处理 */
    
    return 0;
}

/* 处理检查点消息 */
int ms_sync_handle_checkpoint(master_slave_sync_t *sync_mgr, const checkpoint_msg_t *msg) {
    if (!sync_mgr || !msg) {
        return -1;
    }
    
    /* 只有从节点处理检查点 */
    if (sync_mgr->role != NODE_ROLE_SECONDARY) {
        return 0;
    }
    
    ms_sync_log(sync_mgr, "INFO", "Received checkpoint %u with %u variables", 
               msg->checkpoint_id, msg->var_count);
    
    /* 恢复变量状态（简化实现） */
    if (sync_mgr->vm && sync_mgr->vm->global_vars) {
        uint32_t offset = 0;
        for (uint32_t i = 0; i < sync_mgr->sync_var_count && offset < sizeof(msg->data); i++) {
            sync_var_info_t *var_info = &sync_mgr->sync_vars[i];
            if (var_info->vm_var_index < MAX_GLOBAL_VARS) {
                if (offset + sizeof(vm_value_t) <= sizeof(msg->data)) {
                    memcpy(&sync_mgr->vm->global_vars[var_info->vm_var_index], 
                          &msg->data[offset], sizeof(vm_value_t));
                    offset += sizeof(vm_value_t);
                }
            }
        }
    }
    
    return 0;
}

/* ========== 网络通信函数 ========== */

/* 创建套接字 */
int ms_sync_create_socket(master_slave_sync_t *sync_mgr) {
    if (!sync_mgr) {
        return -1;
    }
    
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        ms_sync_set_error(sync_mgr, "Failed to create socket");
        return -1;
    }
    
    if (ms_sync_setup_socket_options(sockfd) != 0) {
        close(sockfd);
        return -1;
    }
    
    sync_mgr->local_conn.socket_fd = sockfd;
    
    return 0;
}

/* 绑定套接字 */
int ms_sync_bind_socket(master_slave_sync_t *sync_mgr) {
    if (!sync_mgr || sync_mgr->local_conn.socket_fd < 0) {
        return -1;
    }
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(sync_mgr->sync_port);
    
    if (strlen(sync_mgr->local_ip) > 0) {
        inet_pton(AF_INET, sync_mgr->local_ip, &addr.sin_addr);
    } else {
        addr.sin_addr.s_addr = INADDR_ANY;
    }
    
    if (bind(sync_mgr->local_conn.socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        ms_sync_set_error(sync_mgr, "Failed to bind socket");
        return -1;
    }
    
    sync_mgr->local_conn.addr = addr;
    sync_mgr->local_conn.is_connected = true;
    
    return 0;
}

/* 连接到对端 */
int ms_sync_connect_to_peer(master_slave_sync_t *sync_mgr) {
    if (!sync_mgr || strlen(sync_mgr->peer_ip) == 0) {
        return -1;
    }
    
    struct sockaddr_in peer_addr;
    memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(sync_mgr->sync_port);
    inet_pton(AF_INET, sync_mgr->peer_ip, &peer_addr.sin_addr);
    
    sync_mgr->peer_conn.addr = peer_addr;
    sync_mgr->peer_conn.socket_fd = sync_mgr->local_conn.socket_fd; /* UDP共用套接字 */
    sync_mgr->peer_conn.is_connected = true;
    
    return 0;
}

/* 接受连接（UDP不需要） */
int ms_sync_accept_connection(master_slave_sync_t *sync_mgr) {
    /* UDP不需要accept */
    return 0;
}

/* 关闭连接 */
void ms_sync_close_connection(master_slave_sync_t *sync_mgr) {
    if (!sync_mgr) return;
    
    if (sync_mgr->local_conn.socket_fd >= 0) {
        close(sync_mgr->local_conn.socket_fd);
        sync_mgr->local_conn.socket_fd = -1;
    }
    
    sync_mgr->local_conn.is_connected = false;
    sync_mgr->peer_conn.is_connected = false;
}

/* 发送数据 */
int ms_sync_send_data(const network_connection_t *conn, const void *data, size_t size) {
    if (!conn || !data || conn->socket_fd < 0) {
        return -1;
    }
    
    ssize_t sent = sendto(conn->socket_fd, data, size, 0,
                         (struct sockaddr*)&conn->addr, sizeof(conn->addr));
    
    return (sent == (ssize_t)size) ? 0 : -1;
}

/* 接收数据 */
int ms_sync_receive_data(const network_connection_t *conn, void *buffer, size_t size) {
    if (!conn || !buffer || conn->socket_fd < 0) {
        return -1;
    }
    
    struct sockaddr_in from_addr;
    socklen_t addr_len = sizeof(from_addr);
    
    ssize_t received = recvfrom(conn->socket_fd, buffer, size, MSG_DONTWAIT,
                               (struct sockaddr*)&from_addr, &addr_len);
    
    if (received < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 1; /* 无数据可读 */
        }
        return -1; /* 真正的错误 */
    }
    
    return 0;
}

/* 发送消息 */
int ms_sync_send_message(master_slave_sync_t *sync_mgr, const sync_message_t *msg) {
    if (!sync_mgr || !msg || !sync_mgr->peer_conn.is_connected) {
        return -1;
    }
    
    ssize_t sent = sendto(sync_mgr->peer_conn.socket_fd, msg, sizeof(sync_message_t), 0,
                         (struct sockaddr*)&sync_mgr->peer_conn.addr, sizeof(sync_mgr->peer_conn.addr));
    
    if (sent < 0) {
        ms_sync_set_error(sync_mgr, "Failed to send message");
        return -1;
    }
    
    sync_mgr->stats.messages_sent++;
    sync_mgr->stats.bytes_sent += sent;
    
    return 0;
}

/* 接收消息 */
int ms_sync_receive_message(master_slave_sync_t *sync_mgr, sync_message_t *msg) {
    if (!sync_mgr || !msg || sync_mgr->local_conn.socket_fd < 0) {
        return -1;
    }
    
    struct sockaddr_in from_addr;
    socklen_t addr_len = sizeof(from_addr);
    
    ssize_t received = recvfrom(sync_mgr->local_conn.socket_fd, msg, sizeof(sync_message_t),
                               MSG_DONTWAIT, (struct sockaddr*)&from_addr, &addr_len);
    
    if (received < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 1; /* 无数据可读 */
        }
        return -1; /* 真正的错误 */
    }
    
    if (received < sizeof(sync_msg_header_t)) {
        return -1; /* 消息太短 */
    }
    
    /* 验证魔数和校验和 */
    if (msg->header.magic != SYNC_MAGIC || !ms_sync_verify_checksum(msg)) {
        sync_mgr->stats.checksum_errors++;
        return -1;
    }
    
    sync_mgr->stats.messages_received++;
    sync_mgr->stats.bytes_received += received;
    
    return 0;
}

/* ========== 辅助函数实现 ========== */

/* 处理单个消息 */
static int ms_sync_process_single_message(master_slave_sync_t *sync_mgr, const sync_message_t *msg) {
    if (!sync_mgr || !msg) {
        return -1;
    }
    
    switch (msg->header.type) {
        case MSG_TYPE_HEARTBEAT:
            return ms_sync_handle_heartbeat(sync_mgr, &msg->payload.heartbeat);
            
        case MSG_TYPE_VAR_SYNC:
            return ms_sync_handle_var_sync(sync_mgr, &msg->payload.var_sync);
            
        case MSG_TYPE_STATE_SYNC:
            return ms_sync_handle_state_sync(sync_mgr, &msg->payload.state_sync);
            
        case MSG_TYPE_CHECKPOINT:
            return ms_sync_handle_checkpoint(sync_mgr, &msg->payload.checkpoint);
            
        default:
            ms_sync_log(sync_mgr, "WARN", "Unknown message type: %d", msg->header.type);
            return -1;
    }
}

/* 按索引查找变量 */
static sync_var_info_t* ms_sync_find_var_by_index(master_slave_sync_t *sync_mgr, uint32_t vm_var_index) {
    if (!sync_mgr) return NULL;
    
    for (uint32_t i = 0; i < sync_mgr->sync_var_count; i++) {
        if (sync_mgr->sync_vars[i].vm_var_index == vm_var_index) {
            return &sync_mgr->sync_vars[i];
        }
    }
    
    return NULL;
}

/* 按名称查找变量 */
static sync_var_info_t* ms_sync_find_var_by_name(master_slave_sync_t *sync_mgr, const char *name) {
    if (!sync_mgr || !name) return NULL;
    
    for (uint32_t i = 0; i < sync_mgr->sync_var_count; i++) {
        if (strcmp(sync_mgr->sync_vars[i].name, name) == 0) {
            return &sync_mgr->sync_vars[i];
        }
    }
    
    return NULL;
}

/* 设置套接字选项 */
static int ms_sync_setup_socket_options(int sockfd) {
    int opt = 1;
    
    /* 允许地址重用 */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        return -1;
    }
    
    /* 设置非阻塞模式 */
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0) {
        return -1;
    }
    
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        return -1;
    }
    
    return 0;
}

/* ========== 时间和工具函数 ========== */

/* 获取时间戳 */
uint64_t ms_sync_get_timestamp(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

/* 获取运行时间 */
uint64_t ms_sync_get_uptime(void) {
    return ms_sync_get_timestamp() - g_start_time;
}

/* 检查超时 */
bool ms_sync_is_timeout(uint64_t start_time, uint32_t timeout_ms) {
    return (ms_sync_get_timestamp() - start_time) > timeout_ms;
}

/* 计算校验和 */
uint32_t ms_sync_calculate_checksum(const void *data, size_t size) {
    const uint8_t *bytes = (const uint8_t*)data;
    uint32_t checksum = 0;
    
    for (size_t i = 0; i < size; i++) {
        checksum += bytes[i];
        checksum = (checksum << 1) | (checksum >> 31); /* 循环左移 */
    }
    
    return checksum;
}

/* 验证校验和 */
bool ms_sync_verify_checksum(const sync_message_t *msg) {
    if (!msg) return false;
    
    uint32_t expected = ms_sync_calculate_checksum(&msg->payload, msg->header.payload_size);
    return msg->header.checksum == expected;
}

/* ========== 错误处理 ========== */

/* 设置错误信息 */
void ms_sync_set_error(master_slave_sync_t *sync_mgr, const char *error_msg) {
    if (!sync_mgr || !error_msg) return;
    
    strncpy(sync_mgr->last_error, error_msg, sizeof(sync_mgr->last_error) - 1);
    sync_mgr->last_error[sizeof(sync_mgr->last_error) - 1] = '\0';
    sync_mgr->has_error = true;
}

/* 获取错误信息 */
const char* ms_sync_get_last_error(const master_slave_sync_t *sync_mgr) {
    return sync_mgr ? sync_mgr->last_error : "Invalid sync manager";
}

/* 清除错误 */
void ms_sync_clear_error(master_slave_sync_t *sync_mgr) {
    if (!sync_mgr) return;
    
    sync_mgr->has_error = false;
    sync_mgr->last_error[0] = '\0';
}

/* ========== 调试和监控函数 ========== */

/* 获取统计信息 */
const sync_statistics_t* ms_sync_get_statistics(const master_slave_sync_t *sync_mgr) {
    return sync_mgr ? &sync_mgr->stats : NULL;
}

/* 重置统计信息 */
void ms_sync_reset_statistics(master_slave_sync_t *sync_mgr) {
    if (!sync_mgr) return;
    
    memset(&sync_mgr->stats, 0, sizeof(sync_statistics_t));
}

/* 打印统计信息 */
void ms_sync_print_statistics(const master_slave_sync_t *sync_mgr) {
    if (!sync_mgr) return;
    
    printf("\n=== Master-Slave Sync Statistics ===\n");
    printf("Messages sent: %lu\n", sync_mgr->stats.messages_sent);
    printf("Messages received: %lu\n", sync_mgr->stats.messages_received);
    printf("Bytes sent: %lu\n", sync_mgr->stats.bytes_sent);
    printf("Bytes received: %lu\n", sync_mgr->stats.bytes_received);
    printf("Heartbeats sent: %lu\n", sync_mgr->stats.heartbeats_sent);
    printf("Heartbeats received: %lu\n", sync_mgr->stats.heartbeats_received);
    printf("Sync errors: %lu\n", sync_mgr->stats.sync_errors);
    printf("Checksum errors: %lu\n", sync_mgr->stats.checksum_errors);
    printf("Timeouts: %lu\n", sync_mgr->stats.timeouts);
    printf("Failovers: %lu\n", sync_mgr->stats.failovers);
    printf("=====================================\n\n");
}

/* 打印状态信息 */
void ms_sync_print_status(const master_slave_sync_t *sync_mgr) {
    if (!sync_mgr) return;
    
    printf("\n=== Master-Slave Sync Status ===\n");
    printf("Role: %s\n", ms_sync_role_to_string(sync_mgr->role));
    printf("State: %s\n", ms_sync_state_to_string(sync_mgr->state));
    printf("Local IP: %s:%d\n", sync_mgr->local_ip, sync_mgr->sync_port);
    printf("Peer IP: %s:%d\n", sync_mgr->peer_ip, sync_mgr->sync_port);
    printf("Peer alive: %s\n", sync_mgr->peer_alive ? "YES" : "NO");
    printf("Connected: %s\n", ms_sync_is_connected(sync_mgr) ? "YES" : "NO");
    printf("Sync variables: %u\n", sync_mgr->sync_var_count);
    printf("Uptime: %lu ms\n", ms_sync_get_uptime());
    printf("================================\n\n");
}

/* 打印同步变量信息 */
void ms_sync_print_sync_variables(const master_slave_sync_t *sync_mgr) {
    if (!sync_mgr) return;
    
    printf("\n=== Sync Variables ===\n");
    for (uint32_t i = 0; i < sync_mgr->sync_var_count; i++) {
        const sync_var_info_t *var = &sync_mgr->sync_vars[i];
        printf("%u: %s (VM:%u, type:%u, size:%u, dirty:%s)\n",
               i, var->name, var->vm_var_index, var->type, var->size,
               var->is_dirty ? "YES" : "NO");
    }
    printf("======================\n\n");
}

/* 打印网络信息 */
void ms_sync_print_network_info(const master_slave_sync_t *sync_mgr) {
    if (!sync_mgr) return;
    
    printf("\n=== Network Information ===\n");
    printf("Local socket: %d\n", sync_mgr->local_conn.socket_fd);
    printf("Local connected: %s\n", sync_mgr->local_conn.is_connected ? "YES" : "NO");
    printf("Peer connected: %s\n", sync_mgr->peer_conn.is_connected ? "YES" : "NO");
    printf("Sync enabled: %s\n", sync_mgr->sync_enabled ? "YES" : "NO");
    printf("===========================\n\n");
}

/* 日志记录 */
void ms_sync_log(const master_slave_sync_t *sync_mgr, const char *level, 
                 const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    /* 获取时间戳 */
    uint64_t timestamp = ms_sync_get_timestamp();
    
    printf("[%lu] [%s] MS_SYNC: ", timestamp, level);
    vprintf(format, args);
    printf("\n");
    
    va_end(args);
}

/* ========== 字符串转换函数 ========== */

/* 角色转字符串 */
const char* ms_sync_role_to_string(node_role_t role) {
    switch (role) {
        case NODE_ROLE_PRIMARY: return "PRIMARY";
        case NODE_ROLE_SECONDARY: return "SECONDARY";
        case NODE_ROLE_STANDALONE: return "STANDALONE";
        default: return "UNKNOWN";
    }
}

/* 状态转字符串 */
const char* ms_sync_state_to_string(node_state_t state) {
    switch (state) {
        case NODE_STATE_INIT: return "INIT";
        case NODE_STATE_ACTIVE: return "ACTIVE";
        case NODE_STATE_STANDBY: return "STANDBY";
        case NODE_STATE_TAKEOVER: return "TAKEOVER";
        case NODE_STATE_FAILED: return "FAILED";
        case NODE_STATE_SHUTDOWN: return "SHUTDOWN";
        default: return "UNKNOWN";
    }
}

/* 消息类型转字符串 */
const char* ms_sync_msg_type_to_string(sync_msg_type_t type) {
    switch (type) {
        case MSG_TYPE_HEARTBEAT: return "HEARTBEAT";
        case MSG_TYPE_VAR_SYNC: return "VAR_SYNC";
        case MSG_TYPE_STATE_SYNC: return "STATE_SYNC";
        case MSG_TYPE_CHECKPOINT: return "CHECKPOINT";
        case MSG_TYPE_TAKEOVER: return "TAKEOVER";
        case MSG_TYPE_ACK: return "ACK";
        case MSG_TYPE_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}
