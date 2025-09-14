/*
 * IEC61131 结构化文本编译器和虚拟机主程序
 * 整合词法分析、语法分析和虚拟机执行
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "vm.h"

/* 外部声明 */
extern FILE *yyin;
extern int yyparse();
extern ASTNode *ast_root;
extern int line_num;

/* 函数声明 */

void print_usage(const char *program_name);
int compile_and_run_file(const char *filename);
int compile_and_run_string(const char *source);
void collect_functions(VMState *vm, ASTNode *ast);
int validate_function_calls(VMState *vm, ASTNode *ast);

/* 主函数 */
int main(int argc, char *argv[]) {
    printf("IEC61131 结构化文本编译器和虚拟机 v1.0\n");
    printf("支持：变量声明、赋值、IF/FOR/WHILE/CASE控制结构、表达式运算、函数定义和调用\n\n");
    
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    /* 处理命令行参数 */
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage(argv[0]);
        return 0;
    }
    
    if (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--interactive") == 0) {
        /* 交互模式 */
        printf("进入交互模式（输入 'exit' 退出）:\n");
        char buffer[1024];
        
        while (1) {
            printf("ST> ");
            if (!fgets(buffer, sizeof(buffer), stdin)) break;
            
            if (strncmp(buffer, "exit", 4) == 0) break;
            
            compile_and_run_string(buffer);
        }
        
        printf("再见！\n");
        return 0;
    }
    
    /* 文件模式 */
    return compile_and_run_file(argv[1]);
}

/* 打印使用说明 */
void print_usage(const char *program_name) {
    printf("用法: %s [选项] [文件名]\n\n", program_name);
    printf("选项:\n");
    printf("  -h, --help        显示此帮助信息\n");
    printf("  -i, --interactive 进入交互模式\n");
    printf("  文件名            编译并执行指定的ST文件\n\n");
    printf("示例:\n");
    printf("  %s program.st     # 编译并执行program.st文件\n", program_name);
    printf("  %s -i             # 进入交互模式\n", program_name);
}

/* 编译并执行文件 */
int compile_and_run_file(const char *filename) {
    /* 打开输入文件 */
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("错误：无法打开文件 '%s'\n", filename);
        return 1;
    }
    
    printf("编译文件: %s\n", filename);
    
    /* 设置词法分析器输入 */
    yyin = file;
    line_num = 1;
    ast_root = NULL;
    
    /* 执行语法分析 */
    int parse_result = yyparse();
    fclose(file);
    
    if (parse_result != 0) {
        printf("编译失败：语法分析错误\n");
        return 1;
    }
    
    if (!ast_root) {
        printf("编译失败：没有生成抽象语法树\n");
        return 1;
    }
    
    printf("编译成功！\n");
    
    /* 打印AST结构（调试用） */
    printf("\n=== 抽象语法树 ===\n");
    print_ast(ast_root, 0);
    printf("==================\n\n");
    
    /* 创建虚拟机并编译 */
    VMState *vm = vm_create();
    if (!vm) {
        printf("错误：无法创建虚拟机\n");
        free_ast(ast_root);
        return 1;
    }
    
    printf("生成字节码...\n");
    
    /* 预处理：收集函数定义 */
    collect_functions(vm, ast_root);
    
    /* 编译AST到字节码 */
    vm_compile_ast(vm, ast_root);
    
    if (vm_get_error(vm)) {
        printf("编译错误：%s\n", vm_get_error(vm));
        vm_destroy(vm);
        free_ast(ast_root);
        return 1;
    }
    
    printf("字节码生成完成！\n\n");
    
    /* 打印函数表（调试用） */
    printf("=== 已注册的函数 ===\n");
    vm_print_functions(vm);
    printf("===================\n\n");
    
    /* 打印字节码（调试用） */
    printf("=== 生成的字节码 ===\n");
    vm_print_code(vm);
    printf("===================\n\n");
    
    /* 执行虚拟机 */
    printf("=== 开始执行 ===\n");
    int exec_result = vm_run(vm);
    
    if (exec_result != 0) {
        printf("执行错误：%s\n", vm_get_error(vm));
    } else {
        printf("=== 执行完成 ===\n\n");
        vm_print_variables(vm);
    }
    
    /* 清理资源 */
    vm_destroy(vm);
    free_ast(ast_root);
    
    return exec_result;
}

/* 编译并执行字符串 */
int compile_and_run_string(const char *source) {
    /* 创建临时文件 */
    FILE *temp_file = tmpfile();
    if (!temp_file) {
        printf("错误：无法创建临时文件\n");
        return 1;
    }
    
    /* 写入源代码 */
    fputs(source, temp_file);
    rewind(temp_file);
    
    /* 设置词法分析器输入 */
    yyin = temp_file;
    line_num = 1;
    ast_root = NULL;
    
    /* 执行语法分析 */
    int parse_result = yyparse();
    fclose(temp_file);
    
    if (parse_result != 0 || !ast_root) {
        printf("语法错误\n");
        return 1;
    }
    
    /* 创建虚拟机并执行 */
    VMState *vm = vm_create();
    if (!vm) {
        printf("错误：无法创建虚拟机\n");
        free_ast(ast_root);
        return 1;
    }
    
    /* 预处理：收集函数定义 */
    collect_functions(vm, ast_root);
    
    vm_compile_ast(vm, ast_root);
    
    if (vm_get_error(vm)) {
        printf("编译错误：%s\n", vm_get_error(vm));
        vm_destroy(vm);
        free_ast(ast_root);
        return 1;
    }
    
    int exec_result = vm_run(vm);
    
    if (exec_result != 0) {
        printf("执行错误：%s\n", vm_get_error(vm));
    } else {
        vm_print_variables(vm);
        
        /* 在交互模式下显示函数表 */
        if (vm->functions) {
            printf("\n已定义的函数:\n");
            vm_print_functions(vm);
        }
    }
    
    /* 清理资源 */
    vm_destroy(vm);
    free_ast(ast_root);
    
    return exec_result;
}

/* 收集函数定义的辅助函数 */
void collect_functions(VMState *vm, ASTNode *ast) {
    if (!ast) return;
    
    switch (ast->type) {
        case NODE_COMPILATION_UNIT: {
            /* 处理编译单元：先处理函数声明，再处理程序 */
            if (ast->left) {
                collect_functions(vm, ast->left);  /* 函数声明列表 */
            }
            if (ast->right) {
                collect_functions(vm, ast->right); /* 程序 */
            }
            break;
        }
        
        case NODE_FUNCTION: {
            /* 为函数预分配地址并注册 */
            int func_addr = vm->code_size;
            vm_register_function(vm, ast->identifier, func_addr, ast->return_type, ast->params);
            printf("注册函数: %s (地址: %d)\n", ast->identifier, func_addr);
            
            /* 处理函数链表中的下一个函数 */
            if (ast->next) {
                collect_functions(vm, ast->next);
            }
            break;
        }
        
        case NODE_FUNCTION_BLOCK: {
            /* 为函数块预分配地址并注册 */
            int fb_addr = vm->code_size;
            vm_register_function(vm, ast->identifier, fb_addr, ast->return_type, NULL);
            printf("注册函数块: %s (地址: %d)\n", ast->identifier, fb_addr);
            
            /* 处理函数链表中的下一个函数块 */
            if (ast->next) {
                collect_functions(vm, ast->next);
            }
            break;
        }
        
        case NODE_PROGRAM: {
            /* 递归处理程序中的语句 */
            collect_functions(vm, ast->statements);
            break;
        }
        
        case NODE_STATEMENT_LIST: {
            /* 递归处理语句列表 */
            collect_functions(vm, ast->left);
            if (ast->next) {
                collect_functions(vm, ast->next);
            }
            break;
        }
        
        default:
            /* 对于其他节点类型，递归检查子节点 */
            if (ast->left) collect_functions(vm, ast->left);
            if (ast->right) collect_functions(vm, ast->right);
            if (ast->condition) collect_functions(vm, ast->condition);
            if (ast->statements) collect_functions(vm, ast->statements);
            if (ast->else_statements) collect_functions(vm, ast->else_statements);
            if (ast->next) collect_functions(vm, ast->next);
            break;
    }
}

/* 验证函数调用的辅助函数 */
int validate_function_calls(VMState *vm, ASTNode *ast) {
    if (!ast) return 1;
    
    switch (ast->type) {
        case NODE_FUNCTION_CALL: {
            VMFunction *func = vm_find_function(vm, ast->identifier);
            if (!func) {
                printf("错误：未定义的函数 '%s'\n", ast->identifier);
                return 0;
            }
            
            /* 计算参数个数 */
            int arg_count = 0;
            ASTNode *arg = ast->arguments;
            while (arg) {
                arg_count++;
                arg = arg->next;
            }
            
            if (arg_count != func->param_count) {
                printf("错误：函数 '%s' 参数个数不匹配，期望 %d 个，实际 %d 个\n", 
                       ast->identifier, func->param_count, arg_count);
                return 0;
            }
            
            printf("验证函数调用: %s (%d 个参数)\n", ast->identifier, arg_count);
            break;
        }
        
        default:
            break;
    }
    
    /* 递归验证子节点 */
    int result = 1;
    if (ast->left) result &= validate_function_calls(vm, ast->left);
    if (ast->right) result &= validate_function_calls(vm, ast->right);
    if (ast->condition) result &= validate_function_calls(vm, ast->condition);
    if (ast->statements) result &= validate_function_calls(vm, ast->statements);
    if (ast->else_statements) result &= validate_function_calls(vm, ast->else_statements);
    if (ast->arguments) result &= validate_function_calls(vm, ast->arguments);
    if (ast->next) result &= validate_function_calls(vm, ast->next);
    
    return result;
}