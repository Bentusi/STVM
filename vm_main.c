/*
 * ST虚拟机运行器 - 专门用于执行字节码文件
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "vm.h"
#include "bytecode.h"
#include "ms_sync.h"
#include "libmgr.h"
#include "mmgr.h"

#define VM_VERSION "1.0.0"

/* 虚拟机配置 */
typedef struct vm_runner_config {
    char *bytecode_file;        // 字节码文件
    bool verbose;               // 详细输出
    bool debug_mode;            // 调试模式
    bool show_stats;            // 显示统计信息
    bool sync_primary;          // 主机模式
    bool sync_secondary;        // 从机模式
    char sync_ip[16];           // 同步IP
    uint16_t sync_port;         // 同步端口
    uint32_t max_execution_time; // 最大执行时间（秒）
} vm_runner_config_t;

static void print_vm_help(void);
static void print_vm_version(void);
static int parse_vm_arguments(int argc, char **argv, vm_runner_config_t *config);
static int run_bytecode_with_sync(const char *filename, vm_runner_config_t *config);

int main(int argc, char **argv) {
    vm_runner_config_t config = {0};
    int result = 0;
    
    /* 设置默认配置 */
    config.sync_port = 8888;
    config.max_execution_time = 3600; // 1小时
    
    printf("ST虚拟机运行器 v%s\n", VM_VERSION);
    
    /* 解析命令行参数 */
    if (parse_vm_arguments(argc, argv, &config) != 0) {
        return EXIT_FAILURE;
    }
    
    if (!config.bytecode_file) {
        fprintf(stderr, "错误: 未指定字节码文件\n");
        fprintf(stderr, "使用 '%s --help' 查看帮助信息\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    /* 初始化内存管理器 */
    if (mmgr_init() != 0) {
        fprintf(stderr, "错误: 内存管理器初始化失败\n");
        return EXIT_FAILURE;
    }
    
    /* 运行字节码 */
    result = run_bytecode_with_sync(config.bytecode_file, &config);
    
    /* 清理资源 */
    mmgr_cleanup();
    
    return result;
}

static int parse_vm_arguments(int argc, char **argv, vm_runner_config_t *config) {
    int opt;
    static struct option long_options[] = {
        {"help",            no_argument,       0, 'h'},
        {"version",         no_argument,       0, 'V'},
        {"verbose",         no_argument,       0, 'v'},
        {"debug",           no_argument,       0, 'd'},
        {"stats",           no_argument,       0, 's'},
        {"sync-primary",    required_argument, 0, 'P'},
        {"sync-secondary",  required_argument, 0, 'S'},
        {"port",            required_argument, 0, 'p'},
        {"timeout",         required_argument, 0, 't'},
        {0, 0, 0, 0}
    };
    
    while ((opt = getopt_long(argc, argv, "hVvdsP:S:p:t:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                print_vm_help();
                exit(EXIT_SUCCESS);
                
            case 'V':
                print_vm_version();
                exit(EXIT_SUCCESS);
                
            case 'v':
                config->verbose = true;
                break;
                
            case 'd':
                config->debug_mode = true;
                break;
                
            case 's':
                config->show_stats = true;
                break;
                
            case 'P':
                config->sync_primary = true;
                strncpy(config->sync_ip, optarg, sizeof(config->sync_ip) - 1);
                break;
                
            case 'S':
                config->sync_secondary = true;
                strncpy(config->sync_ip, optarg, sizeof(config->sync_ip) - 1);
                break;
                
            case 'p':
                config->sync_port = (uint16_t)atoi(optarg);
                break;
                
            case 't':
                config->max_execution_time = (uint32_t)atoi(optarg);
                break;
                
            case '?':
                return -1;
                
            default:
                return -1;
        }
    }
    
    /* 获取字节码文件名 */
    if (optind < argc) {
        config->bytecode_file = argv[optind];
    }
    
    return 0;
}

static int run_bytecode_with_sync(const char *filename, vm_runner_config_t *config) {
    bytecode_file_t bytecode_file = {0};
    st_vm_t *vm = NULL;
    library_manager_t *lib_mgr = NULL;
    int result = 0;
    
    if (config->verbose) {
        printf("加载字节码文件: %s\n", filename);
    }
    
    /* 加载字节码文件 */
    if (bytecode_load_file(&bytecode_file, filename) != 0) {
        fprintf(stderr, "错误: 无法加载字节码文件: %s\n", filename);
        return -1;
    }
    
    /* 验证字节码文件 */
    if (!bytecode_verify_file(&bytecode_file)) {
        fprintf(stderr, "错误: 字节码文件验证失败\n");
        result = -1;
        goto cleanup;
    }
    
    /* 创建库管理器 */
    lib_mgr = libmgr_create();
    if (!lib_mgr) {
        fprintf(stderr, "错误: 库管理器创建失败\n");
        result = -1;
        goto cleanup;
    }
    
    if (libmgr_init(lib_mgr) != 0) {
        fprintf(stderr, "错误: 库管理器初始化失败\n");
        result = -1;
        goto cleanup;
    }
    
    /* 加载内置库 */
    libmgr_load_builtin_libraries(lib_mgr);
    
    /* 创建虚拟机 */
    vm = vm_create();
    if (!vm) {
        fprintf(stderr, "错误: 虚拟机创建失败\n");
        result = -1;
        goto cleanup;
    }
    
    /* 配置虚拟机 */
    vm_config_t vm_config = {0};
    vm_config.enable_debug = config->debug_mode;
    vm_config.enable_statistics = config->show_stats;
    vm_config.max_execution_time_ms = config->max_execution_time * 1000;
    
    if (config->sync_primary || config->sync_secondary) {
        vm_config.enable_sync = true;
        vm_config.sync_mode = config->sync_primary ? VM_SYNC_PRIMARY : VM_SYNC_SECONDARY;
    }
    
    if (vm_init(vm, &vm_config) != 0) {
        fprintf(stderr, "错误: 虚拟机初始化失败\n");
        result = -1;
        goto cleanup;
    }
    
    /* 设置库管理器 */
    vm_set_library_manager(vm, lib_mgr);
    
    /* 启用同步（如果需要） */
    if (config->sync_primary || config->sync_secondary) {
        sync_config_t sync_config = {0};
        strncpy(sync_config.local_ip, config->sync_ip, sizeof(sync_config.local_ip) - 1);
        sync_config.sync_port = config->sync_port;
        sync_config.heartbeat_interval_ms = 100;
        sync_config.heartbeat_timeout_ms = 500;
        sync_config.enable_auto_failover = true;
        
        vm_sync_mode_t sync_mode = config->sync_primary ? VM_SYNC_PRIMARY : VM_SYNC_SECONDARY;
        
        if (vm_enable_sync(vm, &sync_config, sync_mode) != 0) {
            fprintf(stderr, "错误: 同步系统启用失败\n");
            result = -1;
            goto cleanup;
        }
        
        if (config->verbose) {
            printf("同步系统已启用: %s模式\n", config->sync_primary ? "主机" : "从机");
        }
    }
    
    /* 加载字节码 */
    if (vm_load_bytecode(vm, &bytecode_file) != 0) {
        fprintf(stderr, "错误: 字节码加载失败: %s\n", vm_get_last_error(vm));
        result = -1;
        goto cleanup;
    }
    
    /* 执行程序 */
    if (config->verbose) {
        printf("正在执行程序...\n");
    }
    
    if (vm_execute(vm) != 0) {
        fprintf(stderr, "错误: 程序执行失败: %s\n", vm_get_last_error(vm));
        result = -1;
        goto cleanup;
    }
    
    if (config->verbose) {
        printf("程序执行完成\n");
    }
    
    /* 显示统计信息 */
    if (config->show_stats) {
        vm_print_statistics(vm);
        vm_print_memory_info(vm);
        
        if (config->sync_primary || config->sync_secondary) {
            vm_print_sync_status(vm);
        }
    }
    
cleanup:
    if (vm) {
        vm_destroy(vm);
    }
    
    if (lib_mgr) {
        libmgr_destroy(lib_mgr);
    }
    
    bytecode_free_file(&bytecode_file);
    
    return result;
}

static void print_vm_version(void) {
    printf("stvm version %s\n", VM_VERSION);
    printf("ST语言虚拟机运行器\n");
}

static void print_vm_help(void) {
    printf("用法: stvm [选项] <字节码文件>\n\n");
    
    printf("选项:\n");
    printf("  -h, --help              显示此帮助信息\n");
    printf("  -V, --version           显示版本信息\n");
    printf("  -v, --verbose           显示详细信息\n");
    printf("  -d, --debug             启用调试模式\n");
    printf("  -s, --stats             显示执行统计信息\n");
    printf("  -t, --timeout SECONDS   设置最大执行时间\n");
    
    printf("\n同步选项:\n");
    printf("  -P, --sync-primary IP   主机同步模式\n");
    printf("  -S, --sync-secondary IP 从机同步模式\n");
    printf("  -p, --port PORT         同步端口 (默认: 8888)\n");
    
    printf("\n示例:\n");
    printf("  stvm program.stbc                    # 执行字节码\n");
    printf("  stvm -v -s program.stbc              # 详细模式带统计\n");
    printf("  stvm -P 192.168.1.100 program.stbc  # 主机同步模式\n");
    printf("  stvm -S 192.168.1.101 program.stbc  # 从机同步模式\n");
}
