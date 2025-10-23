/**
 * @file main.c
 * @brief STVM 主程序 - 命令行工具入口
 */

#include "cli.h"
#include "vm.h"
#include "bytecode.h"
#include "bytecode_io.h"
#include "codegen.h"
#include "typecheck.h"
#include "symtbl.h"
#include "ast.h"
#include "mmgr.h"
#include "libmgr.h"
#include "debugger.h"
#include "iomgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <strings.h>
#include <unistd.h>

#ifdef _POSIX_C_SOURCE
#if _POSIX_C_SOURCE < 200112L
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200112L
#endif
#endif

// 版本信息
#define STVM_VERSION_MAJOR 1
#define STVM_VERSION_MINOR 0
#define STVM_VERSION_PATCH 0

// 外部声明（来自 parser）
extern FILE* yyin;
extern int yyparse(void);
extern ASTNode* parse_result;
extern void yyrestart(FILE*);

/**
 * @brief 解析命令行参数
 */
bool cli_parse_args(int argc, char** argv, CliOptions* options) {
    // 初始化默认值
    memset(options, 0, sizeof(CliOptions));
    options->mode = MODE_HELP;
    options->entry_function = NULL;  // 默认为NULL，运行时设为"main"
    options->cycle_time_ms = 0;   // 默认执行一次
    
    if (argc < 2) {
        return true; // 显示帮助
    }
    
    // 定义长选项
    static struct option long_options[] = {
        {"help",          no_argument,       0, 'h'},
        {"version",       no_argument,       0, 'v'},
        {"compile",       required_argument, 0, 'c'},
        {"run",           required_argument, 0, 'r'},
        {"repl",          no_argument,       0, 'i'},
        {"output",        required_argument, 0, 'o'},
        {"debug",         no_argument,       0, 'd'},
        {"verbose",       no_argument,       0, 'V'},
        {"optimize",      no_argument,       0, 'O'},
        {"dump-ast",      no_argument,       0, 'A'},
        {"dump-bytecode", no_argument,       0, 'B'},
        {"stats",         no_argument,       0, 's'},
        {"static",        no_argument,       0, 'S'},
        {"entry",         required_argument, 0, 'e'},
        {"cycle",         required_argument, 0, 'C'},
        {"io-simulator",  no_argument,       0, 'I'},
        {"io-config",     required_argument, 0, 'g'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    
    // 解析选项
    while ((c = getopt_long(argc, argv, "hvc:r:io:dVOsL:e:C:Ig:", long_options, &option_index)) != -1) {
        switch (c) {
            case 'h':
                options->mode = MODE_HELP;
                return true;
                
            case 'v':
                options->mode = MODE_VERSION;
                return true;
                
            case 'c':
                options->mode = MODE_COMPILE;
                options->input_file = optarg;
                break;
                
            case 'r':
                options->mode = MODE_RUN;
                options->input_file = optarg;
                break;
                
            case 'i':
                options->mode = MODE_REPL;
                break;
                
            case 'o':
                options->output_file = optarg;
                break;
                
            case 'd':
                options->debug = true;
                break;
                
            case 'V':
                options->verbose = true;
                break;
                
            case 'O':
                options->optimize = true;
                break;
                
            case 'A':
                options->dump_ast = true;
                break;
                
            case 'B':
                options->dump_bytecode = true;
                break;
                
            case 's':
                options->statistics = true;
                break;
                
            case 'S':
                options->static_link = true;
                break;
                
            case 'L':
                if (options->library_path_count < 16) {
                    options->library_paths[options->library_path_count++] = optarg;
                }
                break;
                
            case 'e':
                options->entry_function = optarg;
                break;
                
            case 'C':
                {
                    char* endptr;
                    long cycle_ms = strtol(optarg, &endptr, 10);
                    if (*endptr != '\0' || cycle_ms < 0 || cycle_ms > 3600000) { // 允许0-3600000毫秒
                        fprintf(stderr, "错误：无效的执行周期 '%s'（应为0-3600000毫秒，0表示单次执行）\n", optarg);
                        return false;
                    }
                    options->cycle_time_ms = (int)cycle_ms;
                }
                break;
                
            case 'I':
                options->use_io_simulator = true;
                break;
                
            case 'g':
                options->io_config_file = optarg;
                break;
                
            case '?':
                // getopt_long 已经打印了错误消息
                return false;
                
            default:
                return false;
        }
    }
    
    // 处理非选项参数（输入文件）
    if (optind < argc) {
        options->input_file = argv[optind];
    }
    
    // 自动检测模式：如果没有显式指定模式且提供了输入文件
    if (options->mode == MODE_HELP && options->input_file != NULL) {
        const char* ext = strrchr(options->input_file, '.');
        if (ext) {
            if (strcmp(ext, ".st") == 0) {
                // .st 文件：编译并运行
                options->mode = MODE_COMPILE_AND_RUN;
            } else if (strcmp(ext, ".stbc") == 0) {
                // .stbc 文件：直接运行
                options->mode = MODE_RUN;
            } else {
                fprintf(stderr, "错误：不支持的文件类型 '%s'\n", ext);
                fprintf(stderr, "支持的文件类型: .st (源代码), .stbc (字节码)\n");
                return false;
            }
        } else {
            fprintf(stderr, "错误：无法识别文件类型（缺少扩展名）\n");
            return false;
        }
    }
    
    // 验证参数
    if (options->mode == MODE_COMPILE && options->input_file == NULL) {
        fprintf(stderr, "错误：编译模式需要指定输入文件\n");
        return false;
    }
    
    if (options->mode == MODE_RUN && options->input_file == NULL) {
        fprintf(stderr, "错误：运行模式需要指定输入文件\n");
        return false;
    }
    
    if (options->mode == MODE_COMPILE_AND_RUN && options->input_file == NULL) {
        fprintf(stderr, "错误：需要指定输入文件\n");
        return false;
    }
    
    return true;
}

/**
 * @brief 显示帮助信息
 */
void cli_print_help(void) {
    printf("STVM - ST Language Compiler and Virtual Machine\n\n");
    printf("用法: stvm [选项] [文件]\n\n");
    printf("模式:\n");
    printf("  <file.st>               编译并运行ST源文件 (默认)\n");
    printf("  <file.stbc>             运行字节码文件 (默认)\n");
    printf("  -c, --compile <file>    仅编译ST源文件为字节码\n");
    printf("  -r, --run <file>        仅运行字节码文件\n");
    printf("  -i, --repl              启动交互式REPL\n");
    printf("  -h, --help              显示帮助信息\n");
    printf("  -v, --version           显示版本信息\n\n");
    printf("选项:\n");
    printf("  -o, --output <file>     指定输出文件名\n");
    printf("  -d, --debug             启用调试模式\n");
    printf("  -V, --verbose           详细输出\n");
    printf("  -O, --optimize          启用优化\n");
    printf("  -s, --stats             显示统计信息\n");
    printf("  --static                静态链接库（将库代码合并到输出）\n");
    printf("  -L <path>               添加库搜索路径\n");
    printf("  --dump-ast              打印抽象语法树\n");
    printf("  --dump-bytecode         打印字节码\n\n");
    printf("运行模式专用选项:\n");
    printf("  -e, --entry <function>  指定入口函数名（默认：main，不区分大小写）\n");
    printf("  -C, --cycle <ms>        指定执行周期（毫秒，默认：0表示单次执行）\n\n");
    printf("I/O 选项:\n");
    printf("  -I, --io-simulator      启用IO模拟器（无需真实硬件）\n");
    printf("  --io-config <file>      指定IO配置文件（JSON格式）\n\n");
    printf("示例:\n");
    printf("  stvm program.st                    # 编译并运行program.st\n");
    printf("  stvm program.stbc                  # 运行字节码\n");
    printf("  stvm -c program.st                 # 仅编译program.st\n");
    printf("  stvm -c program.st -o prog.stbc    # 编译并指定输出文件\n");
    printf("  stvm -r prog.stbc                  # 运行字节码\n");
    printf("  stvm -r prog.stbc -e MAIN -C 500   # 运行，入口函数MAIN，周期500ms\n");
    printf("  stvm program.st -I                 # 使用IO模拟器运行\n");
    printf("  stvm io_blink.st -I -C 100         # IO模拟器，周期100ms\n");
    printf("  stvm program.st -d                 # 调试模式运行\n");
    printf("  stvm -i                            # 启动REPL\n");
}

/**
 * @brief 显示版本信息
 */
void cli_print_version(void) {
    printf("STVM v%d.%d.%d\n", STVM_VERSION_MAJOR, STVM_VERSION_MINOR, STVM_VERSION_PATCH);
    printf("Copyright (c) 2025 JIANG Wei\n");
    printf("Designed and Developed by JIANG Wei\n");
    printf("IEC 61131-3 Structured Text Compiler and Virtual Machine\n");
    printf("Built on %s %s\n", __DATE__, __TIME__);
}

/**
 * @brief 编译模式
 */
int cli_compile(const CliOptions* options) {
    int exit_code = 0;
    
    if (options->verbose) {
        printf("编译文件: %s\n", options->input_file);
    }
    
    // 初始化内存管理器
    if (!mmgr_init()) {
        fprintf(stderr, "错误：无法初始化内存管理器\n");
        return 1;
    }
    
    // 打开输入文件
    FILE* input = fopen(options->input_file, "r");
    if (!input) {
        fprintf(stderr, "错误：无法打开文件 '%s': %s\n", options->input_file, strerror(errno));
        mmgr_cleanup();
        return 1;
    }
    
    // 解析源代码
    yyin = input;
    yyrestart(input);
    
    if (options->verbose) {
        printf("开始语法分析...\n");
    }
    
    if (yyparse() != 0 || parse_result == NULL) {
        fprintf(stderr, "错误：语法分析失败\n");
        fclose(input);
        mmgr_cleanup();
        return 1;
    }
    
    fclose(input);
    
    if (options->verbose) {
        printf("语法分析完成\n");
    }
    
    // 打印AST（如果需要）
    if (options->dump_ast) {
        printf("\n=== 抽象语法树 ===\n");
        ast_print(parse_result, 0);
        printf("\n");
    }
    
    // 创建符号表
    SymbolTable* symtbl = symtbl_init();
    if (!symtbl) {
        fprintf(stderr, "错误：无法创建符号表\n");
        ast_free_node(parse_result);
        mmgr_cleanup();
        return 1;
    }
    
    // 创建库管理器
    LibraryManager* libmgr = libmgr_create(symtbl);
    if (!libmgr) {
        fprintf(stderr, "错误：无法创建库管理器\n");
        symtbl_free(symtbl);
        ast_free_node(parse_result);
        mmgr_cleanup();
        return 1;
    }
    
    // 添加库搜索路径
    libmgr_add_search_path(libmgr, ".");
    libmgr_add_search_path(libmgr, "examples");
    
    // 类型检查
    if (options->verbose) {
        printf("开始类型检查...\n");
    }
    
    TypeChecker typechecker;
    ErrorCode err = typecheck_init(&typechecker, symtbl, libmgr);
    if (err != OK) {
        fprintf(stderr, "错误：无法创建类型检查器\n");
        libmgr_free(libmgr);
        symtbl_free(symtbl);
        ast_free_node(parse_result);
        mmgr_cleanup();
        return 1;
    }
    
    err = typecheck_program(&typechecker, parse_result);
    if (err != OK) {
        fprintf(stderr, "错误：类型检查失败\n");
        typecheck_cleanup(&typechecker);
        libmgr_free(libmgr);
        symtbl_free(symtbl);
        ast_free_node(parse_result);
        mmgr_cleanup();
        return 1;
    }
    
    if (options->verbose) {
        printf("类型检查完成\n");
    }
    
    typecheck_cleanup(&typechecker);
    
    // 创建字节码模块
    BytecodeModule* module = bytecode_module_create();
    if (!module) {
        fprintf(stderr, "错误：无法创建字节码模块\n");
        libmgr_free(libmgr);
        symtbl_free(symtbl);
        ast_free_node(parse_result);
        mmgr_cleanup();
        return 1;
    }
    
    // 代码生成
    if (options->verbose) {
        printf("开始代码生成...\n");
    }
    
    CodeGenContext* codegen = codegen_create(module, symtbl);
    if (!codegen) {
        fprintf(stderr, "错误：无法创建代码生成器\n");
        bytecode_module_free(module);
        libmgr_free(libmgr);
        symtbl_free(symtbl);
        ast_free_node(parse_result);
        mmgr_cleanup();
        return 1;
    }
    
    err = codegen_generate(codegen, parse_result);
    if (err != OK) {
        fprintf(stderr, "错误：代码生成失败: %s\n", codegen->error_msg);
        codegen_free(codegen);
        bytecode_module_free(module);
        libmgr_free(libmgr);
        symtbl_free(symtbl);
        ast_free_node(parse_result);
        mmgr_cleanup();
        return 1;
    }
    
    if (options->verbose) {
        printf("代码生成完成\n");
    }
    
    codegen_free(codegen);
    
    // 静态链接库(如果启用)
    if (options->static_link) {
        if (options->verbose) {
            printf("开始静态链接库...\n");
        }
        
        // 获取所有已加载的库
        uint32_t lib_count = libmgr_get_library_count(libmgr);
        
        if (lib_count > 0) {
            for (uint32_t i = 0; i < lib_count; i++) {
                const char* lib_name = libmgr_get_library_name(libmgr, i);
                const char* lib_path = libmgr_get_library_path(libmgr, lib_name);
                BytecodeModule* lib_module = libmgr_get_library_module(libmgr, lib_name);
                
                if (lib_module && lib_path) {
                    if (options->verbose) {
                        printf("  合并库: %s (%u 函数, %u 指令)\n", 
                               lib_name, lib_module->function_count, lib_module->instruction_count);
                    }
                    
                    // 使用库路径而不是名称来匹配函数前缀
                    err = bytecode_merge_library(module, lib_module, lib_path);
                    if (err != OK) {
                        fprintf(stderr, "警告：合并库 '%s' 失败\n", lib_name);
                    }
                } else {
                    fprintf(stderr, "警告：无法获取库模块 '%s'\n", lib_name);
                }
            }
            
            if (options->verbose) {
                printf("静态链接完成，最终: %u 函数, %u 指令\n",
                       module->function_count, module->instruction_count);
            }
        } else {
            if (options->verbose) {
                printf("没有库需要链接\n");
            }
        }
    } else {
        // 非静态链接,记录库依赖信息(新增)
        uint32_t lib_count = libmgr_get_library_count(libmgr);
        if (lib_count > 0) {
            if (options->verbose) {
                printf("记录库依赖信息（共 %u 个库）...\n", lib_count);
            }
            
            for (uint32_t i = 0; i < lib_count; i++) {
                const char* lib_name = libmgr_get_library_name(libmgr, i);
                const char* lib_path = libmgr_get_library_path(libmgr, lib_name);
                
                if (lib_path) {
                    bytecode_add_library_dependency(module, lib_path);
                    if (options->verbose) {
                        printf("  依赖库: %s\n", lib_path);
                    }
                }
            }
        }
    }
    
    // 打印字节码（如果需要）
    if (options->dump_bytecode) {
        printf("\n=== 字节码 ===\n");
        bytecode_print_module(module);
        printf("\n");
    }
    
    // 确定输出文件名
    char output_file[512];
    if (options->output_file) {
        strncpy(output_file, options->output_file, sizeof(output_file) - 1);
    } else {
        // 自动生成：将 .st 扩展名替换为 .stbc
        strncpy(output_file, options->input_file, sizeof(output_file) - 1);
        char* ext = strrchr(output_file, '.');
        if (ext) {
            strcpy(ext, ".stbc");
        } else {
            strcat(output_file, ".stbc");
        }
    }
    
    // 保存字节码
    if (options->verbose) {
        printf("保存字节码到: %s\n", output_file);
    }
    
    err = bytecode_save(module, output_file);
    if (err != OK) {
        fprintf(stderr, "错误：无法保存字节码文件\n");
        exit_code = 1;
    } else {
        printf("编译成功: %s\n", output_file);
    }
    
    // 显示统计信息
    if (options->statistics) {
        printf("\n=== 统计信息 ===\n");
        printf("指令数量: %d\n", module->instruction_count);
        printf("常量数量: %d\n", module->const_count);
        printf("函数数量: %d\n", module->function_count);
        printf("全局变量: %d\n", module->global_count);
        
        const MemoryStats* stats = mmgr_get_stats();
        printf("内存使用: %zu 字节\n", stats->total_allocated);
        printf("内存峰值: %zu 字节\n", stats->peak_usage);
    }
    
    // 清理
    bytecode_module_free(module);
    libmgr_free(libmgr);
    symtbl_free(symtbl);
    ast_free_node(parse_result);
    mmgr_cleanup();
    
    return exit_code;
}

/**
 * @brief 运行模式
 */
int cli_run(const CliOptions* options) {
    int exit_code = 0;
    
    if (options->verbose) {
        printf("加载字节码文件: %s\n", options->input_file);
    }
    
    // 初始化内存管理器
    if (!mmgr_init()) {
        fprintf(stderr, "错误：无法初始化内存管理器\n");
        return 1;
    }
    
    // 加载字节码
    BytecodeModule* module = bytecode_load(options->input_file);
    if (!module) {
        fprintf(stderr, "错误：无法加载字节码文件 '%s'\n", options->input_file);
        mmgr_cleanup();
        return 1;
    }

    // 加载库依赖(新增)
    LibraryManager* libmgr = NULL;
    if (module->library_dep_count > 0) {
        if (options->verbose) {
            printf("加载库依赖...\n");
        }
        
        // 创建库管理器 (运行模式不需要符号表,传NULL)
        libmgr = libmgr_create(NULL);
        if (!libmgr) {
            fprintf(stderr, "错误：无法创建库管理器\n");
            bytecode_module_free(module);
            mmgr_cleanup();
            return 1;
        }
        
        // 加载所有依赖的库
        for (uint32_t i = 0; i < module->library_dep_count; i++) {
            const char* lib_path = module->library_deps[i];
            if (options->verbose) {
                printf("  加载库: %s\n", lib_path);
            }
            
            ErrorCode err = libmgr_load_library(libmgr, lib_path);
            if (err != OK) {
                fprintf(stderr, "错误：无法加载库 '%s'\n", lib_path);
                libmgr_free(libmgr);
                bytecode_module_free(module);
                mmgr_cleanup();
                return 1;
            }
        }
        
        if (options->verbose) {
            printf("库依赖加载完成\n");
        }
    }
    
    // 打印字节码（如果需要）
    if (options->dump_bytecode) {
        printf("\n=== 字节码 ===\n");
        bytecode_print_module(module);
        printf("\n");
    }
    
    // 创建虚拟机
    VM* vm = vm_create(module);
    if (!vm) {
        fprintf(stderr, "错误：无法创建虚拟机\n");
        if (libmgr) libmgr_free(libmgr);
        bytecode_module_free(module);
        mmgr_cleanup();
        return 1;
    }
    
    // 设置库管理器(新增)
    if (libmgr) {
        vm_set_library_manager(vm, libmgr);
    }
    
    // 初始化IO管理器(如果启用)
    IOManager* iomgr = NULL;
    IOHardwareAdapter* io_adapter = NULL;
    
    if (options->use_io_simulator) {
        if (options->verbose) {
            printf("启用IO模拟器...\n");
        }
        
        // 创建模拟器适配器
        io_adapter = io_adapter_create_simulator();
        if (!io_adapter) {
            fprintf(stderr, "错误：无法创建IO模拟器适配器\n");
            vm_free(vm);
            if (libmgr) libmgr_free(libmgr);
            bytecode_module_free(module);
            mmgr_cleanup();
            return 1;
        }
        
        // 创建IO管理器
        iomgr = io_manager_create(io_adapter);
        if (!iomgr) {
            fprintf(stderr, "错误：无法创建IO管理器\n");
            io_adapter_free_simulator(io_adapter);
            vm_free(vm);
            if (libmgr) libmgr_free(libmgr);
            bytecode_module_free(module);
            mmgr_cleanup();
            return 1;
        }
        
        // 设置IO管理器到VM
        vm_set_io_manager(vm, iomgr);
        
        // TODO: 加载IO配置文件 (如果提供)
        if (options->io_config_file) {
            if (options->verbose) {
                printf("加载IO配置文件: %s\n", options->io_config_file);
            }
            // 这里需要实现JSON配置文件加载
            // ErrorCode err = io_manager_load_config(iomgr, options->io_config_file);
            fprintf(stderr, "警告：IO配置文件加载功能尚未实现\n");
        }
        
        // 启动IO自动刷新(10ms周期)
        ErrorCode err = io_manager_start_refresh(iomgr, 10000);
        if (err != OK) {
            fprintf(stderr, "警告：无法启动IO自动刷新\n");
        }
        
        if (options->verbose) {
            printf("IO模拟器已启用\n");
        }
    }
    
    FunctionEntry* entry_function = NULL;  // 声明在高作用域
    
    // 如果没有指定入口函数，则从程序入口点执行整个脚本
    if (!options->entry_function) {
        if (options->verbose) {
            printf("未指定入口函数，从程序入口点执行完整脚本\n");
        }
        
        // 执行完整程序（包括全局变量初始化和所有代码）
        ErrorCode err = vm_run_from(vm, module->entry_point);
        if (err != OK) {
            fprintf(stderr, "运行时错误: %s\n", vm->error_msg);
            exit_code = 1;
        } else if (options->verbose) {
            printf("脚本执行完成\n");
        }
    } else {
        // 指定了入口函数，按原有逻辑执行
        const char* entry_name = options->entry_function;
        
        if (options->verbose) {
            printf("字节码加载完成\n");
            printf("入口点: %d\n", module->entry_point);
            printf("全局变量数: %d\n", module->global_count);
            printf("库依赖数: %d\n", module->library_dep_count);
            printf("指定入口函数: %s\n", entry_name);
            printf("执行周期: %d 毫秒\n", options->cycle_time_ms);
        }
        
        // 查找指定的入口函数
        entry_function = bytecode_find_function(module, entry_name);
        if (!entry_function) {
            // 尝试不区分大小写的查找
            bool found = false;
            for (uint32_t i = 0; i < module->function_count; i++) {
                FunctionEntry* func = &module->functions[i];
                if (strcasecmp(func->name, entry_name) == 0) {
                    entry_function = func;
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                fprintf(stderr, "错误：找不到入口函数 '%s'\n", entry_name);
                fprintf(stderr, "可用函数列表:\n");
                for (uint32_t i = 0; i < module->function_count; i++) {
                    fprintf(stderr, "  %s\n", module->functions[i].name);
                }
                vm_free(vm);
                if (libmgr) libmgr_free(libmgr);
                bytecode_module_free(module);
                mmgr_cleanup();
                return 1;
            }
        }
        
        if (entry_function && options->verbose) {
            printf("找到入口函数: %s (地址: %d)\n", entry_function->name, entry_function->address);
        }
    }
    
    // 只有在指定了入口函数时才执行调试或正常执行模式
    if (entry_function) {
        // 调试模式
        if (options->debug) {
            if (options->verbose) {
                printf("启动调试器...\n");
            }
            
            Debugger* debugger = debugger_create(vm);
            if (!debugger) {
                fprintf(stderr, "错误：无法创建调试器\n");
                vm_free(vm);
                if (libmgr) libmgr_free(libmgr);
                bytecode_module_free(module);
                mmgr_cleanup();
                return 1;
            }
            
            // 设置入口点为指定函数
            vm->pc = entry_function->address;
            
            exit_code = debugger_run(debugger);
            debugger_free(debugger);
        } else {
            // 正常执行模式 - 支持周期性执行
            if (options->verbose) {
                printf("开始执行...\n");
            }
            
            // 如果执行周期大于0，则进行周期性执行；等于0则单次执行
            if (options->cycle_time_ms > 0) {
                printf("周期性执行模式 - 每 %d 毫秒执行一次函数 '%s'\n", 
                       options->cycle_time_ms, entry_function->name);
                printf("按 Ctrl+C 停止执行\n\n");
                
                uint64_t cycle_count = 0;
                while (true) {
                    // 重置VM执行状态但保留全局变量
                    vm_reset_execution_state(vm);
                    
                    if (options->verbose) {
                        printf("执行周期 %lu 开始...\n", cycle_count + 1);
                    }
                    
                    // 执行一次函数
                    ErrorCode err = vm_run_from(vm, entry_function->address);
                    if (err != OK) {
                        fprintf(stderr, "运行时错误 (周期 %lu): %s\n", cycle_count + 1, vm->error_msg);
                        // 在周期性执行中，继续下一个周期而不是退出
                        if (options->verbose) {
                            printf("忽略错误，继续执行下一个周期...\n");
                        }
                    } else if (options->verbose) {
                        printf("周期 %lu 执行完成\n", cycle_count + 1);
                    }
                    
                    cycle_count++;
                    
                    // 等待指定的时间间隔
                    usleep(options->cycle_time_ms * 1000); // 转换为微秒
                }
            } else {
                // 单次执行模式
                // 直接调用 vm_run_from 使用指定的入口函数地址
                ErrorCode err = vm_run_from(vm, entry_function->address);
                if (err != OK) {
                    fprintf(stderr, "运行时错误: %s\n", vm->error_msg);
                    exit_code = 1;
                } else if (options->verbose) {
                    printf("执行完成\n");
                }
            }
            
            // 显示统计信息
            if (options->statistics) {
                printf("\n=== 统计信息 ===\n");
                printf("执行指令数: %lu\n", (unsigned long)vm->instruction_count);
                
                const MemoryStats* stats = mmgr_get_stats();
                printf("内存使用: %zu 字节\n", stats->total_allocated);
                printf("内存峰值: %zu 字节\n", stats->peak_usage);
            }
        }
    }
    
    // 清理
    if (iomgr) {
        io_manager_stop_refresh(iomgr);
        io_manager_free(iomgr);
    }
    if (io_adapter) {
        io_adapter_free_simulator(io_adapter);
    }
    vm_free(vm);
    if (libmgr) libmgr_free(libmgr);
    bytecode_module_free(module);
    mmgr_cleanup();
    
    return exit_code;
}

/**
 * @brief 编译并运行模式
 */
int cli_compile_and_run(const CliOptions* options) {
    int exit_code = 0;
    
    if (options->verbose) {
        printf("编译并运行文件: %s\n", options->input_file);
    }
    
    // 初始化内存管理器
    if (!mmgr_init()) {
        fprintf(stderr, "错误：无法初始化内存管理器\n");
        return 1;
    }
    
    // 1. 语法分析阶段
    if (options->verbose) {
        printf("开始语法分析...\n");
    }
    
    FILE* input = fopen(options->input_file, "r");
    if (!input) {
        fprintf(stderr, "错误：无法打开文件 '%s': %s\n", 
                options->input_file, strerror(errno));
        mmgr_cleanup();
        return 1;
    }
    
    yyin = input;
    yyrestart(yyin);
    parse_result = NULL;
    
    int parse_status = yyparse();
    fclose(input);
    
    if (parse_status != 0 || parse_result == NULL) {
        fprintf(stderr, "错误：语法分析失败\n");
        mmgr_cleanup();
        return 1;
    }
    
    if (options->verbose) {
        printf("语法分析完成\n");
    }
    
    // 打印AST（如果需要）
    if (options->dump_ast) {
        printf("\n=== 抽象语法树 ===\n");
        ast_print(parse_result, 0);
        printf("\n");
    }
    
    // 2. 类型检查阶段
    if (options->verbose) {
        printf("开始类型检查...\n");
    }
    
    SymbolTable* symtbl = symtbl_init();
    if (!symtbl) {
        fprintf(stderr, "错误：无法创建符号表\n");
        ast_free_node(parse_result);
        mmgr_cleanup();
        return 1;
    }
    
    // 创建库管理器（用于处理IMPORT语句）
    LibraryManager* libmgr = libmgr_create(symtbl);
    if (!libmgr) {
        fprintf(stderr, "错误：无法创建库管理器\n");
        symtbl_free(symtbl);
        ast_free_node(parse_result);
        mmgr_cleanup();
        return 1;
    }
    
    // 添加库搜索路径
    libmgr_add_search_path(libmgr, ".");      // 当前目录
    libmgr_add_search_path(libmgr, "examples"); // examples目录
    
    TypeChecker typechecker;
    ErrorCode err = typecheck_init(&typechecker, symtbl, libmgr);
    if (err != OK) {
        fprintf(stderr, "错误：无法创建类型检查器\n");
        libmgr_free(libmgr);
        symtbl_free(symtbl);
        ast_free_node(parse_result);
        mmgr_cleanup();
        return 1;
    }
    
    err = typecheck_program(&typechecker, parse_result);
    if (err != OK) {
        fprintf(stderr, "错误：类型检查失败\n");
        typecheck_cleanup(&typechecker);
        libmgr_free(libmgr);
        symtbl_free(symtbl);
        ast_free_node(parse_result);
        mmgr_cleanup();
        return 1;
    }
    
    if (options->verbose) {
        printf("类型检查完成\n");
    }
    
    typecheck_cleanup(&typechecker);
    
    // 3. 代码生成阶段
    if (options->verbose) {
        printf("开始代码生成...\n");
    }
    
    BytecodeModule* module = bytecode_module_create();
    if (!module) {
        fprintf(stderr, "错误：无法创建字节码模块\n");
        libmgr_free(libmgr);
        symtbl_free(symtbl);
        ast_free_node(parse_result);
        mmgr_cleanup();
        return 1;
    }
    
    CodeGenContext* codegen = codegen_create(module, symtbl);
    if (!codegen) {
        fprintf(stderr, "错误：无法创建代码生成器\n");
        bytecode_module_free(module);
        libmgr_free(libmgr);
        symtbl_free(symtbl);
        ast_free_node(parse_result);
        mmgr_cleanup();
        return 1;
    }
    
    err = codegen_generate(codegen, parse_result);
    if (err != OK) {
        fprintf(stderr, "错误：代码生成失败: %s\n", codegen->error_msg);
        codegen_free(codegen);
        bytecode_module_free(module);
        libmgr_free(libmgr);
        symtbl_free(symtbl);
        ast_free_node(parse_result);
        mmgr_cleanup();
        return 1;
    }
    
    if (options->verbose) {
        printf("代码生成完成\n");
    }
    
    codegen_free(codegen);
    symtbl_free(symtbl);
    ast_free_node(parse_result);
    
    // 打印字节码（如果需要）
    if (options->dump_bytecode) {
        printf("\n=== 字节码 ===\n");
        bytecode_print_module(module);
        printf("\n");
    }
    
    // 4. 执行阶段
    if (options->verbose) {
        printf("开始执行...\n");
    }
    
    // 创建虚拟机
    VM* vm = vm_create(module);
    if (!vm) {
        fprintf(stderr, "错误：无法创建虚拟机\n");
        bytecode_module_free(module);
        libmgr_free(libmgr);
        mmgr_cleanup();
        return 1;
    }
    
    // 设置库管理器（用于运行时查找库函数）
    vm_set_library_manager(vm, libmgr);

    // 初始化IO管理器(如果启用)
    IOManager* iomgr = NULL;
    IOHardwareAdapter* io_adapter = NULL;
    
    if (options->use_io_simulator) {
        if (options->verbose) {
            printf("启用IO模拟器...\n");
        }
        
        // 创建模拟器适配器
        io_adapter = io_adapter_create_simulator();
        if (!io_adapter) {
            fprintf(stderr, "错误：无法创建IO模拟器适配器\n");
            vm_free(vm);
            bytecode_module_free(module);
            libmgr_free(libmgr);
            mmgr_cleanup();
            return 1;
        }
        
        // 创建IO管理器
        iomgr = io_manager_create(io_adapter);
        if (!iomgr) {
            fprintf(stderr, "错误：无法创建IO管理器\n");
            io_adapter_free_simulator(io_adapter);
            vm_free(vm);
            bytecode_module_free(module);
            libmgr_free(libmgr);
            mmgr_cleanup();
            return 1;
        }
        
        // 设置IO管理器到VM
        vm_set_io_manager(vm, iomgr);
        
        // TODO: 加载IO配置文件 (如果提供)
        if (options->io_config_file) {
            if (options->verbose) {
                printf("加载IO配置文件: %s\n", options->io_config_file);
            }
            fprintf(stderr, "警告：IO配置文件加载功能尚未实现\n");
        }
        
        // 启动IO自动刷新(10ms周期)
        ErrorCode io_err = io_manager_start_refresh(iomgr, 10000);
        if (io_err != OK) {
            fprintf(stderr, "警告：无法启动IO自动刷新\n");
        }
        
        if (options->verbose) {
            printf("IO模拟器已启用\n");
        }
    }

    // 执行一次全局变量初始化（从程序入口点执行），然后重置执行状态但保留全局变量
    if (options->verbose) {
        printf("初始化全局变量...\n");
    }
    ErrorCode init_err = vm_run_from(vm, module->entry_point);
    if (init_err != OK) {
        if (options->verbose) {
            printf("全局变量初始化完成（程序结束或错误）：%s\n", vm->error_msg);
        }
    } else {
        if (options->verbose) {
            printf("全局变量初始化完成\n");
        }
    }
    vm_reset_execution_state(vm);
    
    // 如果没有指定入口函数，则从程序入口点执行整个脚本
    if (!options->entry_function) {
        if (options->verbose) {
            printf("未指定入口函数，从程序入口点执行完整脚本\n");
        }
        
        // 执行完整程序（包括全局变量初始化和所有代码）
        ErrorCode err = vm_run_from(vm, module->entry_point);
        if (err != OK) {
            fprintf(stderr, "运行时错误: %s\n", vm->error_msg);
            exit_code = 1;
        } else if (options->verbose) {
            printf("脚本执行完成\n");
        }
    } else {
        // 指定了入口函数，按原有逻辑执行
        const char* entry_name = options->entry_function;
        
        // 查找指定的入口函数
        FunctionEntry* entry_function = bytecode_find_function(module, entry_name);
        if (!entry_function) {
            // 尝试不区分大小写的查找
            bool found = false;
            for (uint32_t i = 0; i < module->function_count; i++) {
                FunctionEntry* func = &module->functions[i];
                if (strcasecmp(func->name, entry_name) == 0) {
                    entry_function = func;
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                fprintf(stderr, "错误：找不到入口函数 '%s'\n", entry_name);
                fprintf(stderr, "可用函数列表:\n");
                for (uint32_t i = 0; i < module->function_count; i++) {
                    fprintf(stderr, "  %s\n", module->functions[i].name);
                }
                vm_free(vm);
                bytecode_module_free(module);
                libmgr_free(libmgr);
                mmgr_cleanup();
                return 1;
            }
        }
        
        if (options->verbose) {
            printf("找到入口函数: %s (地址: %d)\n", entry_function->name, entry_function->address);
            printf("执行周期: %d 毫秒\n", options->cycle_time_ms);
        }
        
        // 调试模式
        if (options->debug) {
            if (options->verbose) {
                printf("启动调试器...\n");
            }
            
            Debugger* debugger = debugger_create(vm);
            if (!debugger) {
                fprintf(stderr, "错误：无法创建调试器\n");
                vm_free(vm);
                bytecode_module_free(module);
                mmgr_cleanup();
                return 1;
            }
            
            // 设置入口点为指定函数
            vm->pc = entry_function->address;
            
            exit_code = debugger_run(debugger);
            debugger_free(debugger);
        } else {
            // 正常执行模式 - 支持周期性执行
            if (options->cycle_time_ms > 0) {
                printf("周期性执行模式 - 每 %d 毫秒执行一次函数 '%s'\n", 
                       options->cycle_time_ms, entry_function->name);
                printf("按 Ctrl+C 停止执行\n\n");
                
                uint64_t cycle_count = 0;
                while (true) {
                    // 重置VM状态到入口函数（保留全局变量）
                    vm_reset_execution_state(vm);
                    
                    if (options->verbose) {
                        printf("执行周期 %lu 开始...\n", cycle_count + 1);
                    }
                    
                    // 执行一次函数
                    ErrorCode err = vm_run_from(vm, entry_function->address);
                    if (err != OK) {
                        fprintf(stderr, "运行时错误 (周期 %lu): %s\n", cycle_count + 1, vm->error_msg);
                        if (options->verbose) {
                            printf("忽略错误，继续执行下一个周期...\n");
                        }
                    } else if (options->verbose) {
                        printf("周期 %lu 执行完成\n", cycle_count + 1);
                    }
                    
                    cycle_count++;
                    
                    // 等待指定的时间间隔
                    usleep(options->cycle_time_ms * 1000); // 转换为微秒
                }
            } else {
                // 单次执行模式
                ErrorCode err = vm_run_from(vm, entry_function->address);
                if (err != OK) {
                    fprintf(stderr, "运行时错误: %s\n", vm->error_msg);
                    exit_code = 1;
                } else if (options->verbose) {
                    printf("执行完成\n");
                }
            }
            
            // 显示统计信息
            if (options->statistics) {
                printf("\n=== 统计信息 ===\n");
                printf("指令数量: %d\n", module->instruction_count);
                printf("执行指令数: %lu\n", (unsigned long)vm->instruction_count);
                
                const MemoryStats* stats = mmgr_get_stats();
                printf("内存使用: %zu 字节\n", stats->total_allocated);
                printf("内存峰值: %zu 字节\n", stats->peak_usage);
            }
        }
    }
    
    // 清理
    if (iomgr) {
        io_manager_stop_refresh(iomgr);
        io_manager_free(iomgr);
    }
    if (io_adapter) {
        io_adapter_free_simulator(io_adapter);
    }
    vm_free(vm);
    bytecode_module_free(module);
    libmgr_free(libmgr);
    mmgr_cleanup();
    
    return exit_code;
}

/**
 * @brief REPL模式
 */
int cli_repl(const CliOptions* options) {
    // TODO: 未来可能需要使用 options 配置 REPL 行为
    (void)options;
    
    printf("STVM Interactive REPL v%d.%d.%d\n", 
           STVM_VERSION_MAJOR, STVM_VERSION_MINOR, STVM_VERSION_PATCH);
    printf("输入 '.help' 查看帮助, '.quit' 退出\n\n");
    
    // 初始化内存管理器
    if (!mmgr_init()) {
        fprintf(stderr, "错误：无法初始化内存管理器\n");
        return 1;
    }
    
    // 创建全局符号表
    SymbolTable* symtbl = symtbl_init();
    if (!symtbl) {
        fprintf(stderr, "错误：无法创建符号表\n");
        mmgr_cleanup();
        return 1;
    }
    
    // 创建字节码模块
    BytecodeModule* module = bytecode_module_create();
    if (!module) {
        fprintf(stderr, "错误：无法创建字节码模块\n");
        symtbl_free(symtbl);
        mmgr_cleanup();
        return 1;
    }
    
    // 创建虚拟机
    VM* vm = vm_create(module);
    if (!vm) {
        fprintf(stderr, "错误：无法创建虚拟机\n");
        bytecode_module_free(module);
        symtbl_free(symtbl);
        mmgr_cleanup();
        return 1;
    }
    
    char line[1024];
    int line_num = 1;
    
    while (true) {
        printf("st[%d]> ", line_num);
        fflush(stdout);
        
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        
        // 移除换行符
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        
        // 空行
        if (line[0] == '\0') {
            continue;
        }
        
        // REPL命令
        if (line[0] == '.') {
            if (strcmp(line, ".quit") == 0 || strcmp(line, ".exit") == 0) {
                break;
            } else if (strcmp(line, ".help") == 0) {
                printf("REPL 命令:\n");
                printf("  .help       显示帮助\n");
                printf("  .quit       退出REPL\n");
                printf("  .vars       显示所有变量\n");
                printf("  .clear      清除所有定义\n");
                printf("  .dump       显示字节码\n");
                continue;
            } else if (strcmp(line, ".vars") == 0) {
                printf("全局变量:\n");
                symtbl_print(symtbl);
                continue;
            } else if (strcmp(line, ".clear") == 0) {
                // 重置模块
                bytecode_module_free(module);
                module = bytecode_module_create();
                vm_free(vm);
                vm = vm_create(module);
                printf("已清除所有定义\n");
                continue;
            } else if (strcmp(line, ".dump") == 0) {
                bytecode_print_module(module);
                continue;
            } else {
                printf("未知命令: %s\n", line);
                continue;
            }
        }
        
        // TODO: 解析和执行输入
        // 这里需要集成 parser 来解析单行/多行输入
        // 暂时只打印输入
        printf("暂未实现执行: %s\n", line);
        
        line_num++;
    }
    
    printf("\n再见!\n");
    
    // 清理
    vm_free(vm);
    bytecode_module_free(module);
    symtbl_free(symtbl);
    mmgr_cleanup();
    
    return 0;
}

/**
 * @brief 主函数
 */
int main(int argc, char** argv) {
    CliOptions options;
    
    // 解析命令行参数
    if (!cli_parse_args(argc, argv, &options)) {
        cli_print_help();
        return 1;
    }
    
    // 处理不同模式
    switch (options.mode) {
        case MODE_HELP:
            cli_print_help();
            return 0;
            
        case MODE_VERSION:
            cli_print_version();
            return 0;
            
        case MODE_COMPILE:
            return cli_compile(&options);
            
        case MODE_RUN:
            return cli_run(&options);
            
        case MODE_COMPILE_AND_RUN:
            return cli_compile_and_run(&options);
            
        case MODE_REPL:
            return cli_repl(&options);
            
        default:
            fprintf(stderr, "错误：未知模式\n");
            return 1;
    }
}
