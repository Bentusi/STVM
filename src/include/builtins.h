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

/**
 * @brief GetQuality 函数 - 获取质量化变量的质量位
 * 
 * 返回变量的质量位值：
 * - 0: QUALITY_GOOD
 * - 1: QUALITY_UNCERTAIN
 * - 2: QUALITY_BAD
 * - 3: QUALITY_ERROR
 * 
 * @param vm 虚拟机实例
 * @param argc 参数个数（必须为1个）
 * @return TYPE_INT - 质量位值
 */
Value builtin_get_quality(VM* vm, int32_t argc);

/**
 * @brief SetQuality 函数 - 设置质量化变量的质量位
 * 
 * @param vm 虚拟机实例
 * @param argc 参数个数（必须为2个：变量, 质量位）
 * @return 带新质量位的变量值
 */
Value builtin_set_quality(VM* vm, int32_t argc);

/**
 * @brief MakeQReal 函数 - 创建带指定质量位的实数值
 * 
 * @param vm 虚拟机实例
 * @param argc 参数个数（必须为2个：实数值, 质量位）
 * @return TYPE_QREAL - 质量化实数
 */
Value builtin_make_qreal(VM* vm, int32_t argc);

/**
 * @brief ToReal 函数 - 从质量化类型提取实数值
 * 
 * @param vm 虚拟机实例
 * @param argc 参数个数（必须为1个）
 * @return TYPE_REAL - 提取的实数值
 */
Value builtin_to_real(VM* vm, int32_t argc);

#endif // STVM_BUILTINS_H
