/**
 * @file builtins.c
 * @brief 内置函数实现 - ST 语言标准内置函数
 */

#include "builtins.h"
#include <stdio.h>
#include <string.h>
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
 * @brief 注册所有内置函数到虚拟机
 */
bool builtins_register_all(VM* vm) {
    if (!vm) return false;
    
    // 注册 PRINT 函数（可变参数，使用 -1 表示）
    if (!vm_register_external_function(vm, "PRINT", builtin_print, -1)) {
        return false;
    }
    
    return true;
}
