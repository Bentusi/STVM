/**
 * @file error.h
 * @brief 错误码定义 - 简化的错误处理系统
 * 
 * 错误码作为函数返回值使用，错误信息存储在各模块上下文中
 * 无需独立的错误管理子系统
 */

#ifndef STVM_ERROR_H
#define STVM_ERROR_H

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/**
 * @brief 错误码枚举
 */
typedef enum {
    OK = 0,                 // 成功
    ERR_SYNTAX,             // 语法错误
    ERR_TYPE,               // 类型错误
    ERR_NAME,               // 名称错误（未定义符号）
    ERR_STACK_OVERFLOW,     // 栈溢出
    ERR_STACK_UNDERFLOW,    // 栈下溢
    ERR_DIV_ZERO,           // 除零错误
    ERR_OUT_OF_BOUNDS,      // 数组越界
    ERR_OUT_OF_MEMORY,      // 内存不足
    ERR_FILE_IO,            // 文件IO错误
    ERR_RUNTIME,            // 通用运行时错误
    ERR_INVALID_BYTECODE,   // 无效的字节码
    ERR_INVALID_INSTRUCTION,// 无效的指令
    ERR_CALL_STACK_OVERFLOW,// 调用栈溢出
    ERR_INVALID_ARGUMENT,   // 无效参数
    ERR_NOT_FOUND,          // 未找到
    ERR_ALREADY_EXISTS,     // 已存在
    ERR_PERMISSION_DENIED,  // 权限拒绝
    ERR_DEVICE_NOT_OPEN,    // 设备未打开
    ERR_SYSTEM_ERROR,       // 系统错误
    ERR_UNKNOWN             // 未知错误
} ErrorCode;

/**
 * @brief 将错误码转换为字符串（用于调试和错误报告）
 * @param code 错误码
 * @return 错误码对应的字符串描述
 */
static inline const char* error_code_to_string(ErrorCode code) {
    switch (code) {
        case OK:                     return "OK";
        case ERR_SYNTAX:             return "Syntax Error";
        case ERR_TYPE:               return "Type Error";
        case ERR_NAME:               return "Name Error";
        case ERR_STACK_OVERFLOW:     return "Stack Overflow";
        case ERR_STACK_UNDERFLOW:    return "Stack Underflow";
        case ERR_DIV_ZERO:           return "Division by Zero";
        case ERR_OUT_OF_BOUNDS:      return "Array Index Out of Bounds";
        case ERR_OUT_OF_MEMORY:      return "Out of Memory";
        case ERR_FILE_IO:            return "File I/O Error";
        case ERR_RUNTIME:            return "Runtime Error";
        case ERR_INVALID_BYTECODE:   return "Invalid Bytecode";
        case ERR_INVALID_INSTRUCTION:return "Invalid Instruction";
        case ERR_CALL_STACK_OVERFLOW:return "Call Stack Overflow";
        case ERR_INVALID_ARGUMENT:   return "Invalid Argument";
        case ERR_NOT_FOUND:          return "Not Found";
        case ERR_ALREADY_EXISTS:     return "Already Exists";
        case ERR_PERMISSION_DENIED:  return "Permission Denied";
        case ERR_DEVICE_NOT_OPEN:    return "Device Not Open";
        case ERR_SYSTEM_ERROR:       return "System Error";
        case ERR_UNKNOWN:            return "Unknown Error";
        default:                     return "Undefined Error";
    }
}

/**
 * @brief 格式化错误消息到缓冲区
 * @param buffer 目标缓冲区
 * @param size 缓冲区大小
 * @param format 格式字符串
 * @param ... 可变参数
 */
static inline void format_error_message(char* buffer, size_t size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, size, format, args);
    va_end(args);
}

/**
 * @brief 辅助宏：如果表达式返回错误则直接返回
 */
#define RETURN_IF_ERROR(expr) do { \
    ErrorCode __err = (expr); \
    if (__err != OK) return __err; \
} while(0)

/**
 * @brief 辅助宏：报告错误并设置错误信息到上下文
 * 使用示例：REPORT_ERROR(vm, ERR_STACK_OVERFLOW, "Stack overflow at line %d", line);
 */
#define REPORT_ERROR(ctx, code, ...) do { \
    (ctx)->error_code = (code); \
    format_error_message((ctx)->error_msg, sizeof((ctx)->error_msg), __VA_ARGS__); \
} while(0)

#endif // STVM_ERROR_H
