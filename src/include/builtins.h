/**
 * @file builtins.h
 * @brief 内置函数声明 - ST 语言标准内置函数
 */

#ifndef STVM_BUILTINS_H
#define STVM_BUILTINS_H

#include "vm.h"

/**
 * @brief 注册所有内置函数到虚拟机
 * @param vm 虚拟机实例
 * @return 成功返回true
 */
bool builtins_register_all(VM* vm);

/**
 * @brief PRINT 函数 - 格式化输出
 * 
 * 格式说明符：
 * - %d : INT 类型
 * - %f : REAL 类型
 * - %s : STRING 类型
 * - %b : BOOL 类型
 * - %% : 字面 %
 * 
 * 转义序列：
 * - \n : 换行
 * - \t : 制表符
 * - \r : 回车
 * - \\ : 反斜杠
 * 
 * 用法示例：
 *   PRINT('Hello, world!\n');
 *   PRINT('x = %d, y = %f\n', x, y);
 *   PRINT('Status: %b\n', flag);
 * 
 * @param vm 虚拟机实例
 * @param argc 参数个数（至少1个：格式字符串）
 * @return TYPE_VOID
 */
Value builtin_print(VM* vm, int32_t argc);

/**
 * @brief SYSTEM 函数 - 调用操作系统命令
 * 
 * 执行系统命令并返回退出状态码
 * 
 * 用法示例：
 *   result := SYSTEM('ls -l');
 *   result := SYSTEM('echo Hello World');
 *   result := SYSTEM('mkdir test_dir');
 * 
 * @param vm 虚拟机实例
 * @param argc 参数个数（必须为1个：命令字符串）
 * @return TYPE_INT - 命令的退出状态码（0表示成功）
 */
Value builtin_system(VM* vm, int32_t argc);

#endif // STVM_BUILTINS_H
