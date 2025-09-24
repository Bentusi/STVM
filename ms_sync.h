#ifndef MS_SYNC_H
#define MS_SYNC_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* 网络配置 */
#define MAX_SYNC_MESSAGE_SIZE   1024
#define DEFAULT_SYNC_PORT       8888
#define HEARTBEAT_INTERVAL_MS   100     // 心跳间隔100ms
#define HEARTBEAT_TIMEOUT_MS    500     // 心跳超时500ms
#define MAX_SYNC_VARIABLES      256     // 最大同步变量数
#define SYNC_MAGIC              0x53544243  // "STBC"

/* 节点角色 */
typedef enum {
    NODE_ROLE_PRIMARY,      // 主节点
    NODE_ROLE_SECONDARY,    // 从节点
    NODE_ROLE_STANDALONE    // 独立节点（无同步）
} node_role_t;

/* 节点状态 */
typedef enum {
    NODE_STATE_INIT,        // 初始化中
    NODE_STATE_ACTIVE,      // 活跃状态
    NODE_STATE_STANDBY,     // 待机状态
    NODE_STATE_TAKEOVER,    // 接管中
    NODE_STATE_FAILED,      // 故障状态
    NODE_STATE_SHUTDOWN     // 关闭状态
} node_state_t;

/* 同步消息类型 */
typedef enum {
    MSG_TYPE_HEARTBEAT,     // 心跳消息
    MSG_TYPE_VAR_SYNC,      // 变量同步
    MSG_TYPE_STATE_SYNC,    // 状态同步
    MSG_TYPE_CHECKPOINT,    // 检查点同步
    MSG_TYPE_TAKEOVER,      // 接管请求
    MSG_TYPE_ACK,           // 确认消息
    MSG_TYPE_ERROR          // 错误消息
} sync_msg_type_t;

/* 同步消息头 */
typedef struct sync_msg_header {
    uint32_t magic;             // 魔数
    uint32_t sequence;          // 序列号
    sync_msg_type_t type;       // 消息类型
    uint32_t payload_size;      // 负载大小
    uint32_t checksum;          // 校验和
    uint64_t timestamp;         // 时间戳
} sync_msg_header_t;

/* 心跳消息 */
typedef struct heartbeat_msg {
    node_role_t role;           // 节点角色
    node_state_t state;         // 节点状态
    uint32_t vm_pc;             // 虚拟机程序计数器
    uint32_t sync_var_count;    // 同步变量数量
    uint64_t uptime;            // 运行时间
} heartbeat_msg_t;

/* 变量同步消息 */
typedef struct var_sync_msg {
    uint32_t var_index;         // 变量索引
    uint32_t var_type;          // 变量类型
    union {
        bool bool_val;          // 布尔值
        int32_t int_val;        // 整数值
        double real_val;        // 实数值
        struct {
            uint32_t length;    // 字符串长度
            char data[128];     // 字符串数据
        } string_val;
    } value;
} var_sync_msg_t;

/* 状态同步消息 */
typedef struct state_sync_msg {
    uint32_t vm_pc;             // 程序计数器
    uint32_t stack_depth;       // 栈深度
    uint32_t call_stack_depth;  // 调用栈深度
    node_state_t node_state;    // 节点状态
} state_sync_msg_t;

/* 检查点消息 */
typedef struct checkpoint_msg {
    uint32_t checkpoint_id;     // 检查点ID
    uint32_t var_count;         // 变量数量
    uint8_t data[512];          // 变量数据快照
} checkpoint_msg_t;

/* 完整同步消息 */
typedef struct sync_message {
    sync_msg_header_t header;   // 消息头
    union {
        heartbeat_msg_t heartbeat;      // 心跳数据
        var_sync_msg_t var_sync;        // 变量同步数据
        state_sync_msg_t state_sync;    // 状态同步数据
        checkpoint_msg_t checkpoint;    // 检查点数据
        char error_msg[256];            // 错误消息
    } payload;
} sync_message_t;

/* 网络连接信息 */
typedef struct network_connection {
    int socket_fd;              // 套接字描述符
    struct sockaddr_in addr;    // 网络地址
    bool is_connected;          // 是否已连接
    uint64_t last_recv_time;    // 最后接收时间
    uint64_t last_send_time;    // 最后发送时间
    uint32_t send_sequence;     // 发送序列号
    uint32_t recv_sequence;     // 接收序列号
} network_connection_t;

/* 同步变量信息 */
typedef struct sync_var_info {
    uint32_t vm_var_index;      // VM变量索引
    char name[64];              // 变量名
    uint32_t type;              // 变量类型
    uint32_t size;              // 变量大小
    bool is_dirty;              // 是否已修改
    uint64_t last_sync_time;    // 最后同步时间
} sync_var_info_t;

/* 同步统计信息 */
typedef struct sync_statistics {
    uint64_t messages_sent;     // 发送消息数
    uint64_t messages_received; // 接收消息数
    uint64_t bytes_sent;        // 发送字节数
    uint64_t bytes_received;    // 接收字节数
    uint64_t heartbeats_sent;   // 发送心跳数
    uint64_t heartbeats_received; // 接收心跳数
    uint64_t sync_errors;       // 同步错误数
    uint64_t checksum_errors;   // 校验和错误数
    uint64_t timeouts;          // 超时次数
    uint64_t failovers;         // 故障切换次数
} sync_statistics_t;

/* 主从同步管理器 */
typedef struct master_slave_sync {
    /* 基本配置 */
    node_role_t role;           // 当前角色
    node_state_t state;         // 当前状态
    char local_ip[16];          // 本地IP地址
    char peer_ip[16];           // 对端IP地址
    uint16_t sync_port;         // 同步端口
    
    /* 网络连接 */
    network_connection_t local_conn;    // 本地连接
    network_connection_t peer_conn;     // 对端连接
    
    /* 虚拟机引用 */
    struct st_vm *vm;           // 关联的虚拟机
    
    /* 同步变量管理 */
    sync_var_info_t sync_vars[MAX_SYNC_VARIABLES];  // 同步变量信息
    uint32_t sync_var_count;    // 同步变量数量
    
    /* 心跳和健康检查 */
    uint64_t last_heartbeat_time;       // 最后心跳时间
    uint64_t peer_last_heartbeat_time;  // 对端最后心跳时间
    uint32_t heartbeat_timeout_count;   // 心跳超时计数
    bool peer_alive;            // 对端是否存活
    
    /* 检查点管理 */
    uint32_t last_checkpoint_id;        // 最后检查点ID
    uint64_t last_checkpoint_time;      // 最后检查点时间
    uint32_t checkpoint_interval_ms;    // 检查点间隔
    
    /* 错误处理 */
    char last_error[256];       // 最后错误信息
    bool has_error;             // 是否有错误
    
    /* 统计信息 */
    sync_statistics_t stats;    // 同步统计
    
    /* 线程和同步 */
    bool sync_enabled;          // 是否启用同步
    bool sync_thread_running;   // 同步线程是否运行
    
} master_slave_sync_t;

/* 同步配置 */
typedef struct sync_config {
    char local_ip[16];          // 本地IP地址
    char peer_ip[16];           // 对端IP地址
    uint16_t sync_port;         // 同步端口
    uint32_t heartbeat_interval_ms;     // 心跳间隔
    uint32_t heartbeat_timeout_ms;      // 心跳超时
    uint32_t checkpoint_interval_ms;    // 检查点间隔
    bool enable_auto_failover;  // 是否启用自动故障切换
    bool enable_data_checksum;  // 是否启用数据校验
} sync_config_t;

/* ========== 主从同步管理函数 ========== */

/* 初始化和清理 */
master_slave_sync_t* ms_sync_create(void);
void ms_sync_destroy(master_slave_sync_t *sync_mgr);
int ms_sync_init(master_slave_sync_t *sync_mgr, const sync_config_t *config, 
                 node_role_t role, struct st_vm *vm);

/* 网络连接管理 */
int ms_sync_start_network(master_slave_sync_t *sync_mgr);
int ms_sync_stop_network(master_slave_sync_t *sync_mgr);
bool ms_sync_is_connected(const master_slave_sync_t *sync_mgr);

/* 同步变量管理 */
int ms_sync_register_variable(master_slave_sync_t *sync_mgr, const char *var_name,
                              uint32_t vm_var_index, uint32_t var_type, uint32_t var_size);
int ms_sync_unregister_variable(master_slave_sync_t *sync_mgr, const char *var_name);
int ms_sync_mark_variable_dirty(master_slave_sync_t *sync_mgr, uint32_t vm_var_index);

/* 同步操作 */
int ms_sync_send_variable_update(master_slave_sync_t *sync_mgr, uint32_t vm_var_index);
int ms_sync_send_heartbeat(master_slave_sync_t *sync_mgr);
int ms_sync_send_checkpoint(master_slave_sync_t *sync_mgr);
int ms_sync_process_messages(master_slave_sync_t *sync_mgr);

/* 角色管理和故障切换 */
int ms_sync_set_role(master_slave_sync_t *sync_mgr, node_role_t new_role);
node_role_t ms_sync_get_role(const master_slave_sync_t *sync_mgr);
int ms_sync_initiate_failover(master_slave_sync_t *sync_mgr);
bool ms_sync_should_takeover(const master_slave_sync_t *sync_mgr);

/* 健康检查 */
bool ms_sync_is_peer_alive(const master_slave_sync_t *sync_mgr);
uint64_t ms_sync_get_peer_uptime(const master_slave_sync_t *sync_mgr);
int ms_sync_check_peer_health(master_slave_sync_t *sync_mgr);

/* ========== 消息处理函数 ========== */

/* 消息创建和解析 */
int ms_sync_create_heartbeat_msg(const master_slave_sync_t *sync_mgr, sync_message_t *msg);
int ms_sync_create_var_sync_msg(const master_slave_sync_t *sync_mgr, uint32_t var_index, 
                                sync_message_t *msg);
int ms_sync_create_state_sync_msg(const master_slave_sync_t *sync_mgr, sync_message_t *msg);
int ms_sync_create_checkpoint_msg(const master_slave_sync_t *sync_mgr, sync_message_t *msg);

/* 消息处理 */
int ms_sync_handle_heartbeat(master_slave_sync_t *sync_mgr, const heartbeat_msg_t *msg);
int ms_sync_handle_var_sync(master_slave_sync_t *sync_mgr, const var_sync_msg_t *msg);
int ms_sync_handle_state_sync(master_slave_sync_t *sync_mgr, const state_sync_msg_t *msg);
int ms_sync_handle_checkpoint(master_slave_sync_t *sync_mgr, const checkpoint_msg_t *msg);

/* 消息发送和接收 */
int ms_sync_send_message(master_slave_sync_t *sync_mgr, const sync_message_t *msg);
int ms_sync_receive_message(master_slave_sync_t *sync_mgr, sync_message_t *msg);

/* ========== 网络通信函数 ========== */

/* 套接字操作 */
int ms_sync_create_socket(master_slave_sync_t *sync_mgr);
int ms_sync_bind_socket(master_slave_sync_t *sync_mgr);
int ms_sync_connect_to_peer(master_slave_sync_t *sync_mgr);
int ms_sync_accept_connection(master_slave_sync_t *sync_mgr);
void ms_sync_close_connection(master_slave_sync_t *sync_mgr);

/* 数据传输 */
int ms_sync_send_data(const network_connection_t *conn, const void *data, size_t size);
int ms_sync_receive_data(const network_connection_t *conn, void *buffer, size_t size);

/* ========== 辅助函数 ========== */

/* 时间函数 */
uint64_t ms_sync_get_timestamp(void);
uint64_t ms_sync_get_uptime(void);
bool ms_sync_is_timeout(uint64_t start_time, uint32_t timeout_ms);

/* 校验和计算 */
uint32_t ms_sync_calculate_checksum(const void *data, size_t size);
bool ms_sync_verify_checksum(const sync_message_t *msg);

/* 错误处理 */
void ms_sync_set_error(master_slave_sync_t *sync_mgr, const char *error_msg);
const char* ms_sync_get_last_error(const master_slave_sync_t *sync_mgr);
void ms_sync_clear_error(master_slave_sync_t *sync_mgr);

/* ========== 调试和监控函数 ========== */

/* 统计信息 */
const sync_statistics_t* ms_sync_get_statistics(const master_slave_sync_t *sync_mgr);
void ms_sync_reset_statistics(master_slave_sync_t *sync_mgr);
void ms_sync_print_statistics(const master_slave_sync_t *sync_mgr);

/* 调试输出 */
void ms_sync_print_status(const master_slave_sync_t *sync_mgr);
void ms_sync_print_sync_variables(const master_slave_sync_t *sync_mgr);
void ms_sync_print_network_info(const master_slave_sync_t *sync_mgr);

/* 日志记录 */
void ms_sync_log(const master_slave_sync_t *sync_mgr, const char *level, 
                 const char *format, ...);

/* 节点状态字符串转换 */
const char* ms_sync_role_to_string(node_role_t role);
const char* ms_sync_state_to_string(node_state_t state);
const char* ms_sync_msg_type_to_string(sync_msg_type_t type);

#endif /* MS_SYNC_H */
