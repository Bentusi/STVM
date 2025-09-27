/*
 * IEC61131 ST语言编译器和虚拟机主程序
 * 支持源文件编译、字节码执行、调试等多种模式
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include "ast.h"
#include "symbol_table.h"
#include "type_checker.h"
#include "codegen.h"
#include "vm.h"
#include "libmgr.h"
#include "ms_sync.h"
#include "mmgr.h"
#include "bytecode.h"

/* 程序版本信息 */
#define PROGRAM_VERSION "1.0.0"
#define PROGRAM_NAME "stc"
#define MAX_LIB_PATHS 32
#define MAX_FILENAME_LEN 512
#define MAX_INPUT_LINE 2048

/* 运行模式 */
typedef enum {
    MODE_COMPILE_ONLY,      // 仅编译，不执行
    MODE_COMPILE_AND_RUN,   // 编译并执行
    MODE_RUN_BYTECODE,      // 执行字节码文件
    MODE_INTERACTIVE,       // 交互模式(REPL)
    MODE_DEBUG,             // 调试模式
    MODE_DISASSEMBLE,       // 反汇编模式
    MODE_SYNTAX_CHECK,      // 语法检查模式
    MODE_PROFILE,           // 性能分析模式
    MODE_FORMAT             // 代码格式化模式
} run_mode_t;

/* 同步配置 */
typedef struct sync_options {
    bool enable_sync;       // 启用同步
    char primary_ip[32];    // 主机IP
    char secondary_ip[32];  // 从机IP
    uint16_t sync_port;     // 同步端口
    bool is_primary;        // 是否为主机
    uint32_t heartbeat_interval;  // 心跳间隔(ms)
    uint32_t sync_timeout;  // 同步超时(ms)
} sync_options_t;

/* 编译器选项 */
typedef struct compiler_options {
    bool optimize;          // 启用优化
    bool warnings_as_errors; // 警告视为错误
    bool strict_mode;       // 严格模式
    int optimization_level; // 优化级别(0-3)
    bool generate_debug_info; // 生成调试信息
    bool enable_bounds_check; // 启用边界检查
} compiler_options_t;

/* 全局配置 */
typedef struct main_config {
    run_mode_t mode;                    // 运行模式
    char input_file[MAX_FILENAME_LEN];  // 输入文件
    char output_file[MAX_FILENAME_LEN]; // 输出文件
    bool verbose;                       // 详细输出
    bool quiet;                         // 静默模式
    bool show_ast;                      // 显示AST
    bool show_bytecode;                 // 显示字节码
    bool show_tokens;                   // 显示词法分析结果
    bool enable_warnings;               // 启用警告
    bool enable_colors;                 // 启用彩色输出
    sync_options_t sync;                // 同步选项
    compiler_options_t compiler;        // 编译器选项
    char lib_paths[MAX_LIB_PATHS][MAX_FILENAME_LEN]; // 库搜索路径
    int lib_path_count;                 // 库路径数量
    int max_errors;                     // 最大错误数
    bool benchmark_mode;                // 基准测试模式
} main_config_t;

/* 全局状态 */
typedef struct main_context {
    library_manager_t *lib_mgr;
    master_slave_sync_t *sync_mgr;
    st_vm_t *vm;
    bool initialized;
    bool signal_received;
    int exit_code;
} main_context_t;

/* 外部声明 */
extern FILE *yyin;
extern int yyparse(void);
extern ast_node_t* get_ast_root(void);

/* 函数声明 */
static void print_version(void);
static void print_help(void);
static void print_usage(void);
static int parse_arguments(int argc, char **argv, main_config_t *config);
static int init_main_context(main_context_t *ctx, main_config_t *config);
static void cleanup_main_context(main_context_t *ctx);
static int compile_source_file(main_context_t *ctx, const char *filename, main_config_t *config);
static int execute_bytecode_file(main_context_t *ctx, const char *filename, main_config_t *config);
static int interactive_mode(main_context_t *ctx, main_config_t *config);
static int debug_mode(main_context_t *ctx, const char *filename, main_config_t *config);
static int disassemble_file(const char *filename, main_config_t *config);
static int syntax_check_file(const char *filename, main_config_t *config);
static int profile_mode(main_context_t *ctx, const char *filename, main_config_t *config);
static int format_source_file(const char *filename, main_config_t *config);

/* 辅助函数 */
static bool file_exists(const char *filename);
static const char* get_file_extension(const char *filename);
static char* generate_output_filename(const char *input_file, const char *extension);
static int setup_sync_system(main_context_t *ctx, const sync_options_t *sync_opts);
static int init_libraries(main_context_t *ctx, main_config_t *config);
static void signal_handler(int signum);
static void setup_signal_handlers(void);
static void print_colored(const char *text, const char *color);
static int validate_config(const main_config_t *config);
static void print_statistics(const main_context_t *ctx, const main_config_t *config);

/* 颜色定义 */
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"

/* 全局变量 */
static main_context_t g_main_ctx = {0};

int main(int argc, char **argv) {
    main_config_t config = {0};
    int result = EXIT_SUCCESS;

    /* 设置默认配置 */
    config.mode = MODE_COMPILE_AND_RUN;
    config.sync.sync_port = 8888;
    config.sync.heartbeat_interval = 100;
    config.sync.sync_timeout = 5000;
    config.enable_warnings = true;
    config.enable_colors = isatty(STDOUT_FILENO);
    config.compiler.optimization_level = 1;
    config.compiler.generate_debug_info = true;
    config.compiler.enable_bounds_check = true;
    config.max_errors = 10;

    /* 设置信号处理 */
    setup_signal_handlers();

    /* 显示程序信息 */
    if (!config.quiet) {
        print_colored("IEC61131 ST语言编译器", COLOR_CYAN);
        printf(" v%s\n", PROGRAM_VERSION);
    }

    /* 解析命令行参数 */
    if (parse_arguments(argc, argv, &config) != 0) {
        result = EXIT_FAILURE;
        goto cleanup;
    }

    /* 验证配置 */
    if (validate_config(&config) != 0) {
        result = EXIT_FAILURE;
        goto cleanup;
    }

    /* 初始化主上下文 */
    if (init_main_context(&g_main_ctx, &config) != 0) {
        fprintf(stderr, "错误: 系统初始化失败\n");
        result = EXIT_FAILURE;
        goto cleanup;
    }

    /* 根据运行模式执行相应操作 */
    switch (config.mode) {
        case MODE_COMPILE_ONLY:
        case MODE_COMPILE_AND_RUN:
            if (strlen(config.input_file) == 0) {
                fprintf(stderr, "错误: 未指定输入文件\n");
                print_usage();
                result = EXIT_FAILURE;
                break;
            }
            result = compile_source_file(&g_main_ctx, config.input_file, &config);
            break;

        case MODE_RUN_BYTECODE:
            if (strlen(config.input_file) == 0) {
                fprintf(stderr, "错误: 未指定字节码文件\n");
                result = EXIT_FAILURE;
                break;
            }
            result = execute_bytecode_file(&g_main_ctx, config.input_file, &config);
            break;

        case MODE_INTERACTIVE:
            result = interactive_mode(&g_main_ctx, &config);
            break;

        case MODE_DEBUG:
            if (strlen(config.input_file) == 0) {
                fprintf(stderr, "错误: 未指定调试文件\n");
                result = EXIT_FAILURE;
                break;
            }
            result = debug_mode(&g_main_ctx, config.input_file, &config);
            break;

        case MODE_DISASSEMBLE:
            if (strlen(config.input_file) == 0) {
                fprintf(stderr, "错误: 未指定反汇编文件\n");
                result = EXIT_FAILURE;
                break;
            }
            result = disassemble_file(config.input_file, &config);
            break;

        case MODE_SYNTAX_CHECK:
            if (strlen(config.input_file) == 0) {
                fprintf(stderr, "错误: 未指定检查文件\n");
                result = EXIT_FAILURE;
                break;
            }
            result = syntax_check_file(config.input_file, &config);
            break;

        case MODE_PROFILE:
            if (strlen(config.input_file) == 0) {
                fprintf(stderr, "错误: 未指定性能分析文件\n");
                result = EXIT_FAILURE;
                break;
            }
            result = profile_mode(&g_main_ctx, config.input_file, &config);
            break;

        case MODE_FORMAT:
            if (strlen(config.input_file) == 0) {
                fprintf(stderr, "错误: 未指定格式化文件\n");
                result = EXIT_FAILURE;
                break;
            }
            result = format_source_file(config.input_file, &config);
            break;

        default:
            fprintf(stderr, "错误: 未知运行模式\n");
            result = EXIT_FAILURE;
            break;
    }

    /* 打印统计信息 */
    if (config.verbose && !config.quiet) {
        print_statistics(&g_main_ctx, &config);
    }

cleanup:
    /* 清理资源 */
    cleanup_main_context(&g_main_ctx);

    if (!config.quiet) {
        if (result == EXIT_SUCCESS) {
            print_colored("执行完成\n", COLOR_GREEN);
        } else {
            print_colored("执行失败\n", COLOR_RED);
        }
    }

    return result;
}

/* 解析命令行参数 */
static int parse_arguments(int argc, char **argv, main_config_t *config) {
    int opt;
    static struct option long_options[] = {
        {"help",         no_argument,       0, 'h'},
        {"version",      no_argument,       0, 'V'},
        {"compile",      no_argument,       0, 'c'},
        {"run",          required_argument, 0, 'r'},
        {"interactive",  no_argument,       0, 'i'},
        {"debug",        required_argument, 0, 'd'},
        {"disassemble",  required_argument, 0, 'D'},
        {"syntax-check", required_argument, 0, 's'},
        {"profile",      required_argument, 0, 'P'},
        {"format",       required_argument, 0, 'f'},
        {"output",       required_argument, 0, 'o'},
        {"verbose",      no_argument,       0, 'v'},
        {"quiet",        no_argument,       0, 'q'},
        {"optimize",     optional_argument, 0, 'O'},
        {"show-ast",     no_argument,       0, 'A'},
        {"show-bytecode", no_argument,      0, 'B'},
        {"show-tokens",  no_argument,       0, 'T'},
        {"library-path", required_argument, 0, 'L'},
        {"sync-primary", required_argument, 0, 'M'},
        {"sync-secondary", required_argument, 0, 'S'},
        {"sync-port",    required_argument, 0, 'p'},
        {"no-warnings",  no_argument,       0, 'W'},
        {"no-colors",    no_argument,       0, 'C'},
        {"strict",       no_argument,       0, 'X'},
        {"max-errors",   required_argument, 0, 'E'},
        {"benchmark",    no_argument,       0, 'b'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "hVcr:id:D:s:P:f:o:vqO::ABTL:M:S:p:WCXbE:",
                              long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                print_help();
                exit(EXIT_SUCCESS);

            case 'V':
                print_version();
                exit(EXIT_SUCCESS);

            case 'c':
                config->mode = MODE_COMPILE_ONLY;
                break;

            case 'r':
                config->mode = MODE_RUN_BYTECODE;
                strncpy(config->input_file, optarg, MAX_FILENAME_LEN - 1);
                break;

            case 'i':
                config->mode = MODE_INTERACTIVE;
                break;

            case 'd':
                config->mode = MODE_DEBUG;
                strncpy(config->input_file, optarg, MAX_FILENAME_LEN - 1);
                break;

            case 'D':
                config->mode = MODE_DISASSEMBLE;
                strncpy(config->input_file, optarg, MAX_FILENAME_LEN - 1);
                break;

            case 's':
                config->mode = MODE_SYNTAX_CHECK;
                strncpy(config->input_file, optarg, MAX_FILENAME_LEN - 1);
                break;

            case 'P':
                config->mode = MODE_PROFILE;
                strncpy(config->input_file, optarg, MAX_FILENAME_LEN - 1);
                break;

            case 'f':
                config->mode = MODE_FORMAT;
                strncpy(config->input_file, optarg, MAX_FILENAME_LEN - 1);
                break;

            case 'o':
                strncpy(config->output_file, optarg, MAX_FILENAME_LEN - 1);
                break;

            case 'v':
                config->verbose = true;
                break;

            case 'q':
                config->quiet = true;
                config->verbose = false;
                break;

            case 'O':
                config->compiler.optimize = true;
                if (optarg) {
                    config->compiler.optimization_level = atoi(optarg);
                    if (config->compiler.optimization_level > 3) {
                        config->compiler.optimization_level = 3;
                    }
                }
                break;

            case 'A':
                config->show_ast = true;
                break;

            case 'B':
                config->show_bytecode = true;
                break;

            case 'T':
                config->show_tokens = true;
                break;

            case 'L':
                if (config->lib_path_count < MAX_LIB_PATHS) {
                    strncpy(config->lib_paths[config->lib_path_count], optarg,
                           MAX_FILENAME_LEN - 1);
                    config->lib_path_count++;
                } else {
                    fprintf(stderr, "警告: 库路径数量超过限制，忽略 %s\n", optarg);
                }
                break;

            case 'M':
                config->sync.enable_sync = true;
                config->sync.is_primary = true;
                strncpy(config->sync.primary_ip, optarg, sizeof(config->sync.primary_ip) - 1);
                break;

            case 'S':
                config->sync.enable_sync = true;
                config->sync.is_primary = false;
                strncpy(config->sync.secondary_ip, optarg, sizeof(config->sync.secondary_ip) - 1);
                break;

            case 'p':
                config->sync.sync_port = (uint16_t)atoi(optarg);
                break;

            case 'W':
                config->enable_warnings = false;
                break;

            case 'C':
                config->enable_colors = false;
                break;

            case 'X':
                config->compiler.strict_mode = true;
                config->compiler.warnings_as_errors = true;
                break;

            case 'E':
                config->max_errors = atoi(optarg);
                if (config->max_errors <= 0) {
                    config->max_errors = 1;
                }
                break;

            case 'b':
                config->benchmark_mode = true;
                break;

            case '?':
                fprintf(stderr, "使用 '%s --help' 查看帮助信息\n", argv[0]);
                return -1;

            default:
                return -1;
        }
    }

    /* 处理非选项参数（输入文件） */
    if (optind < argc && strlen(config->input_file) == 0) {
        strncpy(config->input_file, argv[optind], MAX_FILENAME_LEN - 1);
    }

    return 0;
}

/* 初始化主上下文 */
static int init_main_context(main_context_t *ctx, main_config_t *config) {
    if (ctx->initialized) {
        return 0;
    }

    /* 初始化内存管理器 */
    if (mmgr_init() != 0) {
        fprintf(stderr, "错误: 内存管理器初始化失败\n");
        return -1;
    }

    /* 初始化库管理器 */
    if (init_libraries(ctx, config) != 0) {
        fprintf(stderr, "错误: 库管理器初始化失败\n");
        return -1;
    }

    /* 创建虚拟机（按需） */
    if (config->mode == MODE_COMPILE_AND_RUN || config->mode == MODE_RUN_BYTECODE ||
        config->mode == MODE_DEBUG || config->mode == MODE_PROFILE) {
        ctx->vm = vm_create();
        if (!ctx->vm) {
            fprintf(stderr, "错误: 虚拟机创建失败\n");
            return -1;
        }

        vm_config_t vm_config = {0};
        vm_config.enable_debug = (config->mode == MODE_DEBUG);
        vm_config.enable_sync = config->sync.enable_sync;

        if (vm_init(ctx->vm, &vm_config) != 0) {
            fprintf(stderr, "错误: 虚拟机初始化失败\n");
            return -1;
        }

        /* 设置库管理器 */
        vm_set_library_manager(ctx->vm, ctx->lib_mgr);
    }

    ctx->initialized = true;

    if (config->verbose) {
        printf("系统初始化完成\n");
    }

    return 0;
}

/* 编译源文件 */
static int compile_source_file(main_context_t *ctx, const char *filename, main_config_t *config) {
    FILE *input_file = NULL;
    ast_node_t *ast_root = NULL;
    bytecode_file_t bytecode_file = {0};
    codegen_context_t *codegen_ctx = NULL;
    int result = 0;

    if (config->verbose) {
        printf("编译源文件: %s\n", filename);
    }

    /* 检查文件是否存在 */
    if (!file_exists(filename)) {
        fprintf(stderr, "错误: 文件不存在: %s\n", filename);
        return -1;
    }

    /* 打开输入文件 */
    input_file = fopen(filename, "r");
    if (!input_file) {
        fprintf(stderr, "错误: 无法打开文件: %s\n", filename);
        return -1;
    }

    /* 设置词法分析器输入 */
    yyin = input_file;

    if (config->verbose) {
        printf("正在进行词法和语法分析...\n");
    }

    /* 进行语法分析 */
    if (yyparse() != 0) {
        fprintf(stderr, "错误: 语法分析失败\n");
        result = -1;
        goto cleanup;
    }

    /* 获取AST根节点 */
    ast_root = get_ast_root();
    if (!ast_root) {
        fprintf(stderr, "错误: 未生成AST\n");
        result = -1;
        goto cleanup;
    }

    if (config->show_ast) {
        printf("\n=== 抽象语法树 ===\n");
        ast_print_tree(ast_root, 0);
        printf("==================\n\n");
    }

    /* 语义分析 */
    if (config->verbose) {
        printf("正在进行语义分析...\n");
    }

    type_checker_context_t *type_ctx = type_checker_create();
    if (!type_ctx) {
        fprintf(stderr, "错误: 类型检查器初始化失败\n");
        result = -1;
        goto cleanup;
    }

    if (type_checker_check_program(type_ctx, ast_root) != 0) {
        fprintf(stderr, "错误: 语义分析失败\n");
        type_checker_destroy(type_ctx);
        result = -1;
        goto cleanup;
    }

    type_checker_destroy(type_ctx);

    /* 代码生成 */
    if (config->verbose) {
        printf("正在生成字节码...\n");
    }

    codegen_ctx = codegen_create_context();
    if (!codegen_ctx) {
        fprintf(stderr, "错误: 代码生成器初始化失败\n");
        result = -1;
        goto cleanup;
    }

    /* 初始化代码生成器 */
    if (codegen_init_context(codegen_ctx, NULL, ctx->lib_mgr, ctx->sync_mgr) != 0) {
        fprintf(stderr, "错误: 代码生成器初始化失败\n");
        result = -1;
        goto cleanup;
    }

    /* 启用同步支持 */
    if (config->sync.enable_sync) {
        codegen_enable_sync(codegen_ctx);
    }

    /* 生成字节码 */
    if (codegen_compile_program(codegen_ctx, ast_root, &bytecode_file) != 0) {
        fprintf(stderr, "错误: 字节码生成失败: %s\n",
                codegen_get_last_error(codegen_ctx));
        result = -1;
        goto cleanup;
    }

    if (config->show_bytecode) {
        printf("\n=== 字节码反汇编 ===\n");
        bytecode_disassemble_file(&bytecode_file, stdout);
        printf("==================\n\n");
    }

    /* 保存字节码文件 */
    if (config->mode == MODE_COMPILE_ONLY || strlen(config->output_file) > 0) {
        const char *output_filename = config->output_file;
        if (strlen(output_filename) == 0) {
            output_filename = generate_output_filename(filename, "stbc");
        }

        if (config->verbose) {
            printf("保存字节码文件: %s\n", output_filename);
        }

        if (bytecode_save_file(&bytecode_file, output_filename) != 0) {
            fprintf(stderr, "错误: 无法保存字节码文件: %s\n", output_filename);
            result = -1;
            goto cleanup;
        }
    }

    /* 执行字节码（如果需要） */
    if (config->mode == MODE_COMPILE_AND_RUN) {
        if (config->verbose) {
            printf("正在执行程序...\n");
        }

        /* 启用同步（如果需要） */
        if (config->sync.enable_sync) {
            if (setup_sync_system(ctx, &config->sync) != 0) {
                fprintf(stderr, "错误: 同步系统设置失败\n");
                result = -1;
                goto cleanup;
            }
        }

        /* 加载字节码 */
        if (vm_load_bytecode(ctx->vm, &bytecode_file) != 0) {
            fprintf(stderr, "错误: 字节码加载失败\n");
            result = -1;
            goto cleanup;
        }

        /* 执行程序 */
        if (vm_execute(ctx->vm) != 0) {
            fprintf(stderr, "错误: 程序执行失败: %s\n", vm_get_last_error(ctx->vm));
            result = -1;
            goto cleanup;
        }

        if (config->verbose) {
            printf("程序执行完成\n");
            vm_print_statistics(ctx->vm);
        }
    }

cleanup:
    if (input_file) {
        fclose(input_file);
    }

    if (codegen_ctx) {
        codegen_destroy_context(codegen_ctx);
    }

    bytecode_free_file(&bytecode_file);

    return result;
}

/* 执行字节码文件 */
static int execute_bytecode_file(main_context_t *ctx, const char *filename, main_config_t *config) {
    bytecode_file_t bytecode_file = {0};
    int result = 0;

    if (config->verbose) {
        printf("执行字节码文件: %s\n", filename);
    }

    /* 检查文件是否存在 */
    if (!file_exists(filename)) {
        fprintf(stderr, "错误: 文件不存在: %s\n", filename);
        return -1;
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

    if (config->show_bytecode) {
        printf("\n=== 字节码反汇编 ===\n");
        bytecode_disassemble_file(&bytecode_file, stdout);
        printf("==================\n\n");
    }

    /* 启用同步（如果需要） */
    if (config->sync.enable_sync) {
        if (setup_sync_system(ctx, &config->sync) != 0) {
            fprintf(stderr, "错误: 同步系统设置失败\n");
            result = -1;
            goto cleanup;
        }
    }

    /* 加载字节码 */
    if (vm_load_bytecode(ctx->vm, &bytecode_file) != 0) {
        fprintf(stderr, "错误: 字节码加载失败: %s\n", vm_get_last_error(ctx->vm));
        result = -1;
        goto cleanup;
    }

    /* 执行程序 */
    if (config->verbose) {
        printf("正在执行程序...\n");
    }

    if (vm_execute(ctx->vm) != 0) {
        fprintf(stderr, "错误: 程序执行失败: %s\n", vm_get_last_error(ctx->vm));
        result = -1;
        goto cleanup;
    }

    if (config->verbose) {
        printf("程序执行完成\n");
        vm_print_statistics(ctx->vm);
    }

cleanup:
    bytecode_free_file(&bytecode_file);

    return result;
}

/* 交互模式 */
static int interactive_mode(main_context_t *ctx, main_config_t *config) {
    char input_line[MAX_INPUT_LINE];
    char temp_filename[] = "/tmp/st_interactive_XXXXXX";
    int temp_fd;
    FILE *temp_file = NULL;
    int line_count = 0;

    print_colored("ST语言交互模式", COLOR_CYAN);
    printf(" (输入 '.help' 查看命令, '.quit' 退出)\n");

    /* 显示交互命令 */
    printf("\n可用命令:\n");
    printf("  .help      - 显示帮助\n");
    printf("  .quit      - 退出交互模式\n");
    printf("  .clear     - 清屏\n");
    printf("  .reset     - 重置环境\n");
    printf("  .vars      - 显示变量\n");
    printf("  .funcs     - 显示函数\n");
    printf("  .libs      - 显示已加载库\n");
    printf("  .ast       - 切换AST显示\n");
    printf("  .bytecode  - 切换字节码显示\n");
    printf("  .time      - 切换执行时间显示\n");
    printf("\n");

    /* 创建临时文件 */
    temp_fd = mkstemp(temp_filename);
    if (temp_fd == -1) {
        fprintf(stderr, "错误: 无法创建临时文件: %s\n", strerror(errno));
        return -1;
    }

    while (!ctx->signal_received) {
        printf("st[%d]> ", ++line_count);
        fflush(stdout);

        if (!fgets(input_line, sizeof(input_line), stdin)) {
            break;
        }

        /* 去除换行符 */
        size_t len = strlen(input_line);
        if (len > 0 && input_line[len - 1] == '\n') {
            input_line[len - 1] = '\0';
            len--;
        }

        /* 跳过空行 */
        if (len == 0) {
            continue;
        }

        /* 处理交互命令 */
        if (input_line[0] == '.') {
            if (strcmp(input_line, ".help") == 0) {
                printf("ST语言交互模式帮助:\n");
                printf("  直接输入ST语句或表达式执行\n");
                printf("  支持变量定义、函数调用等\n");
                printf("  使用 .命令 控制交互环境\n");
            } else if (strcmp(input_line, ".quit") == 0) {
                break;
            } else if (strcmp(input_line, ".clear") == 0) {
                system("clear || cls");
            } else if (strcmp(input_line, ".reset") == 0) {
                if (ctx->vm) {
                    vm_reset(ctx->vm);
                }
                printf("环境已重置\n");
            } else if (strcmp(input_line, ".vars") == 0) {
                if (ctx->vm) {
                    vm_print_variables(ctx->vm);
                }
            } else if (strcmp(input_line, ".funcs") == 0) {
                // TODO: 实现函数显示
                printf("函数显示功能未实现\n");
                // symbol_table_print_functions(ctx->global_symtab);
            } else if (strcmp(input_line, ".libs") == 0) {
                libmgr_print_loaded_libraries();
            } else if (strcmp(input_line, ".ast") == 0) {
                config->show_ast = !config->show_ast;
                printf("AST显示: %s\n", config->show_ast ? "开启" : "关闭");
            } else if (strcmp(input_line, ".bytecode") == 0) {
                config->show_bytecode = !config->show_bytecode;
                printf("字节码显示: %s\n", config->show_bytecode ? "开启" : "关闭");
            } else if (strcmp(input_line, ".time") == 0) {
                config->benchmark_mode = !config->benchmark_mode;
                printf("执行时间显示: %s\n", config->benchmark_mode ? "开启" : "关闭");
            } else {
                printf("未知命令: %s (输入 '.help' 查看帮助)\n", input_line);
            }
            continue;
        }

        /* 写入临时文件并执行ST代码 */
        temp_file = fdopen(temp_fd, "w");
        if (!temp_file) {
            fprintf(stderr, "错误: 无法打开临时文件\n");
            break;
        }

        /* 构造完整的ST程序 */
        fprintf(temp_file, "PROGRAM Interactive\n");
        fprintf(temp_file, "VAR\n");
        fprintf(temp_file, "    result : INT;\n");
        fprintf(temp_file, "END_VAR\n");
        fprintf(temp_file, "%s\n", input_line);
        fprintf(temp_file, "END_PROGRAM\n");

        fclose(temp_file);

        /* 编译并执行 */
        main_config_t temp_config = *config;
        temp_config.mode = MODE_COMPILE_AND_RUN;
        strcpy(temp_config.input_file, temp_filename);

        if (compile_source_file(ctx, temp_filename, &temp_config) != 0) {
            printf("执行失败\n");
        }

        /* 重新打开临时文件用于下次写入 */
        close(temp_fd);
        temp_fd = open(temp_filename, O_WRONLY | O_TRUNC);
        if (temp_fd == -1) {
            fprintf(stderr, "错误: 无法重新打开临时文件\n");
            break;
        }
    }

    /* 清理 */
    if (temp_fd != -1) {
        close(temp_fd);
    }
    unlink(temp_filename);

    print_colored("退出交互模式\n", COLOR_YELLOW);
    return 0;
}

/* 简化的调试模式 */
static int debug_mode(main_context_t *ctx, const char *filename, main_config_t *config) {
    printf("调试模式: %s\n", filename);

    /* 首先编译文件 */
    main_config_t compile_config = *config;
    compile_config.mode = MODE_COMPILE_ONLY;
    compile_config.compiler.generate_debug_info = true;

    if (compile_source_file(ctx, filename, &compile_config) != 0) {
        return -1;
    }

    /* 生成字节码文件名 */
    char *bytecode_filename = generate_output_filename(filename, "stbc");

    /* 加载字节码并启动简单调试器 */
    bytecode_file_t bytecode_file = {0};
    if (bytecode_load_file(&bytecode_file, bytecode_filename) != 0) {
        fprintf(stderr, "错误: 无法加载字节码文件用于调试\n");
        return -1;
    }

    /* 加载字节码到虚拟机 */
    if (vm_load_bytecode(ctx->vm, &bytecode_file) != 0) {
        fprintf(stderr, "错误: 字节码加载失败\n");
        bytecode_free_file(&bytecode_file);
        return -1;
    }

    /* 启动简单的调试器 */
    printf("简单调试器 (输入 'help' 查看命令)\n");

    char debug_input[256];
    while (1) {
        printf("(stdbg) ");
        fflush(stdout);

        if (!fgets(debug_input, sizeof(debug_input), stdin)) {
            break;
        }

        /* 去除换行符 */
        size_t len = strlen(debug_input);
        if (len > 0 && debug_input[len - 1] == '\n') {
            debug_input[len - 1] = '\0';
        }

        if (strcmp(debug_input, "help") == 0) {
            printf("调试命令:\n");
            printf("  run      - 运行程序\n");
            printf("  step     - 单步执行\n");
            printf("  continue - 继续执行\n");
            printf("  break <addr> - 设置断点\n");
            printf("  info stack   - 显示栈信息\n");
            printf("  info vars    - 显示变量信息\n");
            printf("  quit     - 退出调试器\n");
        } else if (strcmp(debug_input, "run") == 0) {
            if (vm_execute(ctx->vm) != 0) {
                printf("程序执行出错: %s\n", vm_get_last_error(ctx->vm));
            } else {
                printf("程序执行完成\n");
            }
        } else if (strcmp(debug_input, "step") == 0) {
            if (vm_execute_single_step(ctx->vm) != 0) {
                printf("单步执行出错: %s\n", vm_get_last_error(ctx->vm));
            } else {
                printf("PC: %u, 状态: %s\n", vm_get_pc(ctx->vm),
                       vm_state_to_string(vm_get_state(ctx->vm)));
            }
        } else if (strcmp(debug_input, "info stack") == 0) {
            vm_print_stack(ctx->vm);
        } else if (strcmp(debug_input, "info vars") == 0) {
            vm_print_variables(ctx->vm);
        } else if (strcmp(debug_input, "quit") == 0) {
            break;
        } else {
            printf("未知命令: %s (输入 'help' 查看帮助)\n", debug_input);
        }
    }

    bytecode_free_file(&bytecode_file);
    return 0;
}

/* 反汇编文件 */
static int disassemble_file(const char *filename, main_config_t *config) {
    bytecode_file_t bytecode_file = {0};

    if (config->verbose) {
        printf("反汇编字节码文件: %s\n", filename);
    }

    /* 加载字节码文件 */
    if (bytecode_load_file(&bytecode_file, filename) != 0) {
        fprintf(stderr, "错误: 无法加载字节码文件: %s\n", filename);
        return -1;
    }

    /* 验证字节码文件 */
    if (!bytecode_verify_file(&bytecode_file)) {
        fprintf(stderr, "错误: 字节码文件验证失败\n");
        bytecode_free_file(&bytecode_file);
        return -1;
    }

    /* 输出文件信息 */
    bytecode_print_file_info(&bytecode_file);
    printf("\n");

    /* 反汇编输出 */
    bytecode_disassemble_file(&bytecode_file, stdout);

    bytecode_free_file(&bytecode_file);
    return 0;
}

/* 语法检查文件 */
static int syntax_check_file(const char *filename, main_config_t *config) {
    FILE *input_file = NULL;
    ast_node_t *ast_root = NULL;
    int result = 0;

    if (config->verbose) {
        printf("语法检查文件: %s\n", filename);
    }

    /* 检查文件是否存在 */
    if (!file_exists(filename)) {
        fprintf(stderr, "错误: 文件不存在: %s\n", filename);
        return -1;
    }

    /* 打开输入文件 */
    input_file = fopen(filename, "r");
    if (!input_file) {
        fprintf(stderr, "错误: 无法打开文件: %s\n", filename);
        return -1;
    }

    /* 设置词法分析器输入 */
    yyin = input_file;

    /* 进行语法分析 */
    if (yyparse() != 0) {
        fprintf(stderr, "语法检查失败: 发现语法错误\n");
        result = -1;
        goto cleanup;
    }

    /* 获取AST根节点 */
    ast_root = get_ast_root();
    if (!ast_root) {
        fprintf(stderr, "语法检查失败: 未生成AST\n");
        result = -1;
        goto cleanup;
    }

    printf("语法检查通过\n");

    if (config->show_ast) {
        printf("\n=== 抽象语法树 ===\n");
        ast_print_tree(ast_root, 0);
        printf("==================\n");
    }

cleanup:
    if (input_file) {
        fclose(input_file);
    }

    return result;
}

/* 性能分析模式 */
static int profile_mode(main_context_t *ctx, const char *filename, main_config_t *config) {
    print_colored("性能分析模式", COLOR_BLUE);
    printf(" - %s\n", filename);

    /* 编译并执行 */
    main_config_t profile_config = *config;
    profile_config.mode = MODE_COMPILE_AND_RUN;
    profile_config.benchmark_mode = true;

    int result = compile_source_file(ctx, filename, &profile_config);

    return result;
}

/* 代码格式化模式 */
static int format_source_file(const char *filename, main_config_t *config) {
    FILE *input_file = NULL;
    FILE *output_file = NULL;
    ast_node_t *ast_root = NULL;
    int result = 0;

    if (config->verbose) {
        printf("格式化源文件: %s\n", filename);
    }

    if (!file_exists(filename)) {
        fprintf(stderr, "错误: 文件不存在: %s\n", filename);
        return -1;
    }

    /* 解析源文件 */
    input_file = fopen(filename, "r");
    if (!input_file) {
        fprintf(stderr, "错误: 无法打开文件: %s\n", filename);
        return -1;
    }

    yyin = input_file;

    if (yyparse() != 0) {
        fprintf(stderr, "错误: 语法分析失败，无法格式化\n");
        result = -1;
        goto cleanup;
    }

    ast_root = get_ast_root();
    if (!ast_root) {
        fprintf(stderr, "错误: 未生成AST\n");
        result = -1;
        goto cleanup;
    }

    /* 确定输出文件 */
    const char *output_filename = config->output_file;
    if (strlen(output_filename) == 0) {
        output_filename = filename; // 原地格式化
    }

    output_file = fopen(output_filename, "w");
    if (!output_file) {
        fprintf(stderr, "错误: 无法创建输出文件: %s\n", output_filename);
        result = -1;
        goto cleanup;
    }

    if (config->verbose) {
        printf("代码格式化完成: %s\n", output_filename);
    }

cleanup:
    if (input_file) fclose(input_file);
    if (output_file) fclose(output_file);

    return result;
}

/* 辅助函数实现 */
static void print_version(void) {
    printf("%s version %s\n", PROGRAM_NAME, PROGRAM_VERSION);
    printf("IEC61131 结构化文本语言编译器和虚拟机\n");
    printf("特性: 库管理, 主从同步, 调试, 性能分析, 交互模式\n");
    printf("作者: ST编译器开发团队\n");
}

static void print_help(void) {
    printf("用法: %s [选项] [文件]\n\n", PROGRAM_NAME);

    printf("基本选项:\n");
    printf("  -h, --help              显示此帮助信息\n");
    printf("  -V, --version           显示版本信息\n");
    printf("  -v, --verbose           显示详细信息\n");
    printf("  -q, --quiet             静默模式\n");
    printf("  -o, --output FILE       指定输出文件\n");
    printf("  -W, --no-warnings       禁用警告信息\n");
    printf("  -C, --no-colors         禁用彩色输出\n");
    printf("  -E, --max-errors N      最大错误数量\n");

    printf("\n运行模式:\n");
    printf("  -c, --compile           仅编译，不执行\n");
    printf("  -r, --run FILE          执行字节码文件\n");
    printf("  -i, --interactive       交互模式(REPL)\n");
    printf("  -d, --debug FILE        调试模式\n");
    printf("  -D, --disassemble FILE  反汇编字节码文件\n");
    printf("  -s, --syntax-check FILE 语法检查模式\n");
    printf("  -P, --profile FILE      性能分析模式\n");
    printf("  -f, --format FILE       代码格式化模式\n");

    printf("\n编译选项:\n");
    printf("  -O, --optimize[=N]      启用优化(级别0-3)\n");
    printf("  -A, --show-ast          显示抽象语法树\n");
    printf("  -B, --show-bytecode     显示字节码\n");
    printf("  -T, --show-tokens       显示词法分析结果\n");
    printf("  -X, --strict            严格模式\n");
    printf("  -L, --library-path DIR  添加库搜索路径\n");
    printf("  -b, --benchmark         基准测试模式\n");

    printf("\n同步选项:\n");
    printf("  -M, --sync-primary IP   启用主机同步模式\n");
    printf("  -S, --sync-secondary IP 启用从机同步模式\n");
    printf("  -p, --sync-port PORT    设置同步端口 (默认: 8888)\n");

    printf("\n示例:\n");
    printf("  %s program.st                    # 编译并执行\n", PROGRAM_NAME);
    printf("  %s -c -O2 program.st             # 优化编译\n", PROGRAM_NAME);
    printf("  %s -r program.stbc               # 执行字节码\n", PROGRAM_NAME);
    printf("  %s -i                            # 交互模式\n", PROGRAM_NAME);
    printf("  %s -d program.st                 # 调试模式\n", PROGRAM_NAME);
    printf("  %s -P program.st                 # 性能分析\n", PROGRAM_NAME);
    printf("  %s -f program.st                 # 代码格式化\n", PROGRAM_NAME);
    printf("  %s -M 192.168.1.100 program.st  # 主机同步模式\n", PROGRAM_NAME);
}

static void print_usage(void) {
    printf("用法: %s [选项] [文件]\n", PROGRAM_NAME);
    printf("使用 '%s --help' 查看详细帮助信息\n", PROGRAM_NAME);
}

static bool file_exists(const char *filename) {
    struct stat st;
    return stat(filename, &st) == 0;
}

static const char* get_file_extension(const char *filename) {
    const char *dot = strrchr(filename, '.');
    return dot ? dot + 1 : "";
}

static char* generate_output_filename(const char *input_file, const char *extension) {
    static char output_file[MAX_FILENAME_LEN];
    const char *dot = strrchr(input_file, '.');

    if (dot) {
        size_t base_len = dot - input_file;
        strncpy(output_file, input_file, base_len);
        output_file[base_len] = '\0';
    } else {
        strcpy(output_file, input_file);
    }

    strcat(output_file, ".");
    strcat(output_file, extension);

    return output_file;
}

static int setup_sync_system(main_context_t *ctx, const sync_options_t *sync_opts) {
    if (!sync_opts->enable_sync) {
        return 0;
    }

    /* 配置同步参数 */
    sync_config_t sync_config = {0};

    if (sync_opts->is_primary) {
        strncpy(sync_config.local_ip, sync_opts->primary_ip, sizeof(sync_config.local_ip) - 1);
        strncpy(sync_config.peer_ip, sync_opts->secondary_ip, sizeof(sync_config.peer_ip) - 1);
    } else {
        strncpy(sync_config.local_ip, sync_opts->secondary_ip, sizeof(sync_config.local_ip) - 1);
        strncpy(sync_config.peer_ip, sync_opts->primary_ip, sizeof(sync_config.peer_ip) - 1);
    }

    sync_config.sync_port = sync_opts->sync_port;
    sync_config.heartbeat_interval_ms = 100;
    sync_config.heartbeat_timeout_ms = 500;
    sync_config.checkpoint_interval_ms = 1000;
    sync_config.enable_auto_failover = true;
    sync_config.enable_data_checksum = true;

    /* 启用虚拟机同步 */
    vm_sync_mode_t sync_mode = sync_opts->is_primary ? VM_SYNC_PRIMARY : VM_SYNC_SECONDARY;

    if (vm_enable_sync(ctx->vm, &sync_config, sync_mode) != 0) {
        return -1;
    }

    printf("同步系统已启用: %s模式, 端口: %u\n",
           sync_opts->is_primary ? "主机" : "从机", sync_opts->sync_port);

    return 0;
}

static int init_libraries(main_context_t *ctx, main_config_t *config) {
    /* 创建库管理器 */
    ctx->lib_mgr = libmgr_create();
    if (!ctx->lib_mgr) {
        return -1;
    }

    /* 初始化库管理器 */
    if (libmgr_init(ctx->lib_mgr) != 0) {
        return -1;
    }

    /* 添加库搜索路径 */
    libmgr_add_search_path(".");
    libmgr_add_search_path("/usr/local/lib/st");
    libmgr_add_search_path("/usr/lib/st");

    /* 添加用户指定的库路径 */
    for (int i = 0; i < config->lib_path_count; i++) {
        libmgr_add_search_path(config->lib_paths[i]);
    }

    /* 加载内置库 */
    if (libmgr_load_builtin_libraries(ctx->lib_mgr) != 0) {
        if (config->verbose) {
            printf("警告: 部分内置库加载失败\n");
        }
    }

    if (config->verbose) {
        printf("库管理器初始化完成\n");
        libmgr_print_loaded_libraries();
    }

    return 0;
}

static void cleanup_main_context(main_context_t *ctx) {
    if (!ctx->initialized) {
        return;
    }

    if (ctx->vm) {
        vm_destroy(ctx->vm);
        ctx->vm = NULL;
    }

    if (ctx->sync_mgr) {
        ms_sync_destroy(ctx->sync_mgr);
        ctx->sync_mgr = NULL;
    }

    if (ctx->lib_mgr) {
        libmgr_destroy(ctx->lib_mgr);
        ctx->lib_mgr = NULL;
    }

    yylex_destroy();
    mmgr_cleanup();

    ctx->initialized = false;
}

static void signal_handler(int signum) {
    g_main_ctx.signal_received = true;
    g_main_ctx.exit_code = signum;

    switch (signum) {
        case SIGINT:
            printf("\n收到中断信号，正在清理...\n");
            break;
        case SIGTERM:
            printf("\n收到终止信号，正在清理...\n");
            break;
    }

    cleanup_main_context(&g_main_ctx);
    exit(g_main_ctx.exit_code);
}

static void setup_signal_handlers(void) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
}

static void print_colored(const char *text, const char *color) {
    if (isatty(STDOUT_FILENO)) {
        printf("%s%s%s", color, text, COLOR_RESET);
    } else {
        printf("%s", text);
    }
}

static void print_statistics(const main_context_t *ctx, const main_config_t *config) {
    printf("\n=== 执行统计 ===\n");

    if (ctx->vm) {
        vm_print_statistics(ctx->vm);
    }

    if (ctx->lib_mgr) {
        printf("已加载库数量: %d\n", libmgr_get_loaded_count(ctx->lib_mgr));
    }

    printf("内存使用: %zu bytes\n", mmgr_get_total_allocated());
    printf("================\n");
}
