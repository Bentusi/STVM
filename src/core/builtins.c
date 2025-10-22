/**
 * @file builtins.c
 * @brief 内置函数实现 - ST 语言标准内置函数
 */

#include "builtins.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef _WIN32
#include <sys/wait.h>
#endif
#include "vm.h"

/**
 * @brief PRINT 函数实现
 * 
 * 格式化输出函数，类似 C 语言的 printf
 * 第一个参数是格式字符串，后续是可变参数
 */
Value builtin_print(VM* vm, int32_t argc) {
    Value result = {.type = TYPE_VOID};
    
    if (argc < 1) {
        fprintf(stderr, "PRINT: 需要至少1个参数（格式字符串）\n");
        return result;
    }
    
    // 获取格式字符串（第一个参数，从栈底往上数）
    Value fmt_val = vm_get_arg(vm, argc - 1);
    if (fmt_val.type != TYPE_STRING || !fmt_val.string_val) {
        fprintf(stderr, "PRINT: 第一个参数必须是字符串\n");
        return result;
    }
    
    const char* fmt = fmt_val.string_val;
    int arg_index = argc - 2;  // 从倒数第二个参数开始（从栈底往上）
    
    // 解析格式字符串并输出
    for (const char* p = fmt; *p; p++) {
        if (*p == '%' && *(p + 1)) {
            p++;  // 跳过 %
            
            switch (*p) {
                case 'd': {
                    // 整数
                    if (arg_index < 0) {
                        fprintf(stderr, "PRINT: 参数不足\n");
                        return result;
                    }
                    Value arg = vm_get_arg(vm, arg_index--);
                    if (arg.type == TYPE_INT) {
                        printf("%d", arg.int_val);
                    } else {
                        fprintf(stderr, "PRINT: %%d 需要 INT 类型，得到 %d\n", arg.type);
                    }
                    break;
                }
                
                case 'f': {
                    // 浮点数
                    if (arg_index < 0) {
                        fprintf(stderr, "PRINT: 参数不足\n");
                        return result;
                    }
                    Value arg = vm_get_arg(vm, arg_index--);
                    if (arg.type == TYPE_REAL) {
                        printf("%f", arg.real_val);
                    } else if (arg.type == TYPE_INT) {
                        // 自动转换 INT 到 REAL
                        printf("%f", (double)arg.int_val);
                    } else {
                        fprintf(stderr, "PRINT: %%f 需要 REAL 类型，得到 %d\n", arg.type);
                    }
                    break;
                }
                
                case 's': {
                    // 字符串
                    if (arg_index < 0) {
                        fprintf(stderr, "PRINT: 参数不足\n");
                        return result;
                    }
                    Value arg = vm_get_arg(vm, arg_index--);
                    if (arg.type == TYPE_STRING && arg.string_val) {
                        printf("%s", arg.string_val);
                    } else {
                        fprintf(stderr, "PRINT: %%s 需要 STRING 类型，得到 %d\n", arg.type);
                    }
                    break;
                }
                
                case 'b': {
                    // 布尔值
                    if (arg_index < 0) {
                        fprintf(stderr, "PRINT: 参数不足\n");
                        return result;
                    }
                    Value arg = vm_get_arg(vm, arg_index--);
                    if (arg.type == TYPE_BOOL) {
                        printf("%s", arg.bool_val ? "TRUE" : "FALSE");
                    } else {
                        fprintf(stderr, "PRINT: %%b 需要 BOOL 类型，得到 %d\n", arg.type);
                    }
                    break;
                }
                
                case '%': {
                    // 字面 %
                    putchar('%');
                    break;
                }
                
                default:
                    // 未知格式说明符，原样输出
                    putchar('%');
                    putchar(*p);
                    break;
            }
        } else if (*p == '\\' && *(p + 1)) {
            // 处理转义序列
            p++;
            switch (*p) {
                case 'n': putchar('\n'); break;
                case 't': putchar('\t'); break;
                case 'r': putchar('\r'); break;
                case '\\': putchar('\\'); break;
                case '\'': putchar('\''); break;
                case '"': putchar('"'); break;
                default: 
                    putchar('\\');
                    putchar(*p); 
                    break;
            }
        } else {
            // 普通字符
            putchar(*p);
        }
    }
    
    fflush(stdout);
    return result;
}

/**
 * @brief SYSTEM 函数实现
 * 
 * 调用操作系统命令并返回退出状态码
 * 使用 system() C 标准库函数执行命令
 */
Value builtin_system(VM* vm, int32_t argc) {
    Value result = {.type = TYPE_INT, .int_val = -1};
    
    // 检查参数个数
    if (argc != 1) {
        fprintf(stderr, "SYSTEM: 需要恰好1个参数（命令字符串）\n");
        return result;
    }
    
    // 获取命令字符串参数
    Value cmd_val = vm_get_arg(vm, 0);
    if (cmd_val.type != TYPE_STRING || !cmd_val.string_val) {
        fprintf(stderr, "SYSTEM: 参数必须是字符串类型\n");
        return result;
    }
    
    const char* command = cmd_val.string_val;
    
    // 执行系统命令
    int exit_code = system(command);
    
    // 在 POSIX 系统上，system() 返回值需要通过 WEXITSTATUS 宏提取
    // 成功通常是 0，失败是非 0
#ifdef _WIN32
    // Windows: 直接返回退出码
    result.int_val = exit_code;
#else
    // POSIX: 需要检查返回值
    if (exit_code == -1) {
        // system() 调用失败
        fprintf(stderr, "SYSTEM: 无法执行命令 '%s'\n", command);
        result.int_val = -1;
    } else {
        // 提取实际的退出状态
        if (WIFEXITED(exit_code)) {
            result.int_val = WEXITSTATUS(exit_code);
        } else {
            // 进程异常终止
            result.int_val = -1;
        }
    }
#endif
    
    return result;
}

/**
 * @brief 注册所有内置函数到虚拟机
 */
bool builtins_register_all(VM* vm) {
    if (!vm) return false;
    
    // 注册 PRINT 函数（可变参数，使用 -1 表示）
    if (!vm_register_external_function(vm, "PRINT", builtin_print, -1)) {
        return false;
    }
    
    // 注册 SYSTEM 函数（1个参数）
    if (!vm_register_external_function(vm, "SYSTEM", builtin_system, 1)) {
        return false;
    }
    
    // 注册质量位访问函数
    if (!vm_register_external_function(vm, "GetQuality", builtin_get_quality, 1)) {
        return false;
    }
    
    if (!vm_register_external_function(vm, "SetQuality", builtin_set_quality, 2)) {
        return false;
    }
    
    if (!vm_register_external_function(vm, "MakeQReal", builtin_make_qreal, 2)) {
        return false;
    }
    
    if (!vm_register_external_function(vm, "ToReal", builtin_to_real, 1)) {
        return false;
    }
    
    return true;
}

/**
 * @brief GetQuality 函数实现
 * 获取质量化变量的质量位
 */
Value builtin_get_quality(VM* vm, int32_t argc) {
    Value result = {.type = TYPE_INT, .quality = QUALITY_GOOD, .int_val = 0};
    
    if (argc != 1) {
        fprintf(stderr, "GetQuality: 需要1个参数\n");
        return result;
    }
    
    Value val = vm_get_arg(vm, 0);
    
    // 如果是质量化类型，返回质量位；否则返回GOOD
    if (is_qualified_type(val.type)) {
        result.int_val = (int)val.quality;
    } else {
        result.int_val = (int)QUALITY_GOOD;
    }
    
    return result;
}

/**
 * @brief SetQuality 函数实现
 * 设置质量化变量的质量位
 */
Value builtin_set_quality(VM* vm, int32_t argc) {
    Value result = {.type = TYPE_VOID, .quality = QUALITY_GOOD};
    
    if (argc != 2) {
        fprintf(stderr, "SetQuality: 需要2个参数 (变量, 质量位)\n");
        return result;
    }
    
    Value val = vm_get_arg(vm, 0);
    Value quality_val = vm_get_arg(vm, 1);
    
    if (quality_val.type != TYPE_INT) {
        fprintf(stderr, "SetQuality: 质量位必须是整数\n");
        return result;
    }
    
    // 检查质量位值是否有效
    int quality = quality_val.int_val;
    if (quality < 0 || quality > 3) {
        fprintf(stderr, "SetQuality: 质量位值必须在0-3之间\n");
        return result;
    }
    
    // 如果是质量化类型，创建新值；否则转换为质量化类型
    if (is_qualified_type(val.type)) {
        result = val;
        result.quality = (QualityFlag)quality;
        result.type = val.type;
    } else {
        result = val;
        result.type = get_qualified_type(val.type);
        result.quality = (QualityFlag)quality;
    }
    
    return result;
}

/**
 * @brief MakeQReal 函数实现
 * 创建带指定质量位的实数值
 */
Value builtin_make_qreal(VM* vm, int32_t argc) {
    Value result = {.type = TYPE_QREAL, .quality = QUALITY_GOOD, .real_val = 0.0};
    
    if (argc != 2) {
        fprintf(stderr, "MakeQReal: 需要2个参数 (实数值, 质量位)\n");
        return result;
    }
    
    // 参数顺序：第一个参数是质量位，第二个参数是实数值（栈的特性）
    Value quality_val = vm_get_arg(vm, 0);  // 最后压入的参数
    Value real_val = vm_get_arg(vm, 1);     // 第一个参数
    
    if (real_val.type != TYPE_REAL && real_val.type != TYPE_INT) {
        fprintf(stderr, "MakeQReal: 第一个参数必须是数值，得到类型: %d\n", real_val.type);
        return result;
    }
    
    if (quality_val.type != TYPE_INT) {
        fprintf(stderr, "MakeQReal: 第二个参数必须是整数，得到类型: %d\n", quality_val.type);
        return result;
    }
    
    // 检查质量位值是否有效
    int quality = quality_val.int_val;
    if (quality < 0 || quality > 3) {
        fprintf(stderr, "MakeQReal: 质量位值必须在0-3之间，得到: %d\n", quality);
        return result;
    }
    
    result.real_val = (real_val.type == TYPE_REAL) ? real_val.real_val : (double)real_val.int_val;
    result.quality = (QualityFlag)quality;
    
    return result;
}

/**
 * @brief ToReal 函数实现
 * 从质量化类型提取实数值
 */
Value builtin_to_real(VM* vm, int32_t argc) {
    Value result = {.type = TYPE_REAL, .quality = QUALITY_GOOD, .real_val = 0.0};
    
    if (argc != 1) {
        fprintf(stderr, "ToReal: 需要1个参数\n");
        return result;
    }
    
    Value val = vm_get_arg(vm, 0);
    
    // 根据类型提取实数值
    switch (val.type) {
        case TYPE_REAL:
        case TYPE_QREAL:
            result.real_val = val.real_val;
            break;
        case TYPE_INT:
        case TYPE_QINT:
            result.real_val = (double)val.int_val;
            break;
        default:
            fprintf(stderr, "ToReal: 参数必须是数值类型\n");
            break;
    }
    
    return result;
}
