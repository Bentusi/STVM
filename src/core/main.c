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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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
    
    if (argc < 2) {
        return true; // 显示帮助
    }
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            options->mode = MODE_HELP;
            return true;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            options->mode = MODE_VERSION;
            return true;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--compile") == 0) {
            options->mode = MODE_COMPILE;
            if (i + 1 < argc) {
                options->input_file = argv[++i];
            }
        } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--run") == 0) {
            options->mode = MODE_RUN;
            if (i + 1 < argc) {
                options->input_file = argv[++i];
            }
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--repl") == 0) {
            options->mode = MODE_REPL;
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                options->output_file = argv[++i];
            }
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--debug") == 0) {
            options->debug = true;
        } else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--verbose") == 0) {
            options->verbose = true;
        } else if (strcmp(argv[i], "-O") == 0 || strcmp(argv[i], "--optimize") == 0) {
            options->optimize = true;
        } else if (strcmp(argv[i], "--dump-ast") == 0) {
            options->dump_ast = true;
        } else if (strcmp(argv[i], "--dump-bytecode") == 0) {
            options->dump_bytecode = true;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--stats") == 0) {
            options->statistics = true;
        } else if (strcmp(argv[i], "--static") == 0) {
            options->static_link = true;
        } else if (strcmp(argv[i], "-L") == 0) {
            if (i + 1 < argc && options->library_path_count < 16) {
                options->library_paths[options->library_path_count++] = argv[++i];
            }
        } else if (options->input_file == NULL) {
            options->input_file = argv[i];
        }
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
    printf("示例:\n");
    printf("  stvm program.st                  # 编译并运行program.st\n");
    printf("  stvm program.stbc                # 运行字节码\n");
    printf("  stvm -c program.st               # 仅编译program.st\n");
    printf("  stvm -c program.st -o prog.stbc  # 编译并指定输出文件\n");
    printf("  stvm -r prog.stbc                # 运行字节码\n");
    printf("  stvm program.st -d               # 调试模式运行\n");
    printf("  stvm -i                          # 启动REPL\n");
}

/**
 * @brief 显示版本信息
 */
void cli_print_version(void) {
    printf("STVM v%d.%d.%d\n", STVM_VERSION_MAJOR, STVM_VERSION_MINOR, STVM_VERSION_PATCH);
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
    
    if (options->verbose) {
        printf("字节码加载完成\n");
        printf("入口点: %d\n", module->entry_point);
        printf("全局变量数: %d\n", module->global_count);
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
        bytecode_module_free(module);
        mmgr_cleanup();
        return 1;
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
        
        exit_code = debugger_run(debugger);
        debugger_free(debugger);
    } else {
        // 正常执行
        if (options->verbose) {
            printf("开始执行...\n");
        }
        
        ErrorCode err = vm_run(vm);
        if (err != OK) {
            fprintf(stderr, "运行时错误: %s\n", vm->error_msg);
            exit_code = 1;
        } else if (options->verbose) {
            printf("执行完成\n");
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
    
    // 清理
    vm_free(vm);
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
        
        exit_code = debugger_run(debugger);
        debugger_free(debugger);
    } else {
        // 正常执行
        err = vm_run(vm);
        if (err != OK) {
            fprintf(stderr, "运行时错误: %s\n", vm->error_msg);
            exit_code = 1;
        } else if (options->verbose) {
            printf("执行完成\n");
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
    
    // 清理
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
