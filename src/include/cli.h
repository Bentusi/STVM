/**
 * @file cli.h
 * @brief 命令行接口 - STVM编译器和虚拟机主程序
 * 
 * 支持三种模式：
 * 1. 编译模式：将ST源代码编译成字节码
 * 2. 运行模式：执行字节码文件
 * 3. 交互模式：REPL（Read-Eval-Print Loop）
 */

#ifndef STVM_CLI_H
#define STVM_CLI_H

#include <stdbool.h>

/**
 * @brief 命令行模式
 */
typedef enum {
    MODE_COMPILE,           // 编译模式
    MODE_RUN,               // 运行模式
    MODE_COMPILE_AND_RUN,   // 编译并运行模式
    MODE_REPL,              // 交互模式
    MODE_WCET,              // WCET 分析模式
    MODE_HELP,              // 显示帮助
    MODE_VERSION            // 显示版本
} CliMode;

/**
 * @brief 命令行选项结构
 */
typedef struct {
    CliMode mode;                   // 运行模式
    char* input_file;               // 输入文件名
    char* output_file;              // 输出文件名
    bool debug;                     // 调试模式
    bool verbose;                   // 详细输出
    bool optimize;                  // 优化开关
    bool dump_ast;                  // 打印AST
    bool dump_bytecode;             // 打印字节码
    bool statistics;                // 显示统计信息
    bool compile_library;           // 编译为库文件
    bool static_link;               // 静态链接库（将库代码合并到输出）
    char* library_paths[16];        // 库搜索路径
    int library_path_count;         // 库路径数量
    char* entry_function;           // 入口函数名（运行模式专用）
    int cycle_time_ms;              // 执行周期（毫秒，运行模式专用）
    bool use_io_simulator;          // 启用IO模拟器
    char* io_config_file;           // IO配置文件路径
    
    // WCET 分析选项
    bool run_wcet;                  // 运行 WCET 分析
    char* wcet_entry;               // WCET 分析的入口函数
    double wcet_cpu_freq_mhz;       // WCET 目标 CPU 频率
    bool wcet_verbose;              // WCET 详细输出
    char* wcet_source_file;         // WCET 标注解析的 ST 源文件
} CliOptions;

/**
 * @brief 解析命令行参数
 * @param argc 参数个数
 * @param argv 参数数组
 * @param options 解析结果
 * @return 成功返回true，失败返回false
 */
bool cli_parse_args(int argc, char** argv, CliOptions* options);

/**
 * @brief 显示帮助信息
 */
void cli_print_help(void);

/**
 * @brief 显示版本信息
 */
void cli_print_version(void);

/**
 * @brief 编译模式入口
 * @param options 命令行选项
 * @return 成功返回0，失败返回错误码
 */
int cli_compile(const CliOptions* options);

/**
 * @brief 运行模式入口
 * @param options 命令行选项
 * @return 成功返回0，失败返回错误码
 */
int cli_run(const CliOptions* options);

/**
 * @brief 编译并运行模式入口
 * @param options 命令行选项
 * @return 成功返回0，失败返回错误码
 */
int cli_compile_and_run(const CliOptions* options);

/**
 * @brief WCET 分析模式入口
 * @param options 命令行选项
 * @return 成功返回0，失败返回错误码
 */
int cli_wcet(const CliOptions* options);

/**
 * @brief REPL模式入口
 * @param options 命令行选项
 * @return 成功返回0，失败返回错误码
 */
int cli_repl(const CliOptions* options);

#endif // STVM_CLI_H