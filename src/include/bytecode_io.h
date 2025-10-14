/**
 * @file bytecode_io.h
 * @brief 字节码序列化/反序列化 - .stbc文件格式
 */

#ifndef STVM_BYTECODE_IO_H
#define STVM_BYTECODE_IO_H

#include "bytecode.h"
#include "error.h"
#include <stdint.h>
#include <stdio.h>

/**
 * @brief STBC文件格式魔数
 */
#define STBC_MAGIC 0x53544243  // "STBC" in ASCII

/**
 * @brief STBC文件版本
 */
#define STBC_VERSION_MAJOR 1
#define STBC_VERSION_MINOR 0

/**
 * @brief STBC文件头
 */
typedef struct STBCHeader {
    uint32_t magic;                 // 魔数 (STBC_MAGIC)
    uint16_t version_major;         // 主版本号
    uint16_t version_minor;         // 次版本号
    uint32_t entry_point;           // 入口点地址
    uint32_t global_var_count;      // 全局变量数量
    uint32_t constant_count;        // 常量池大小
    uint32_t function_count;        // 函数数量
    uint32_t instruction_count;     // 指令数量
    uint32_t checksum;              // CRC32校验和（可选）
} STBCHeader;

/**
 * @brief 保存字节码模块到文件
 * @param module 字节码模块
 * @param filename 文件名（.stbc）
 * @return 成功返回OK，失败返回错误码
 */
ErrorCode bytecode_save(const BytecodeModule* module, const char* filename);

/**
 * @brief 从文件加载字节码模块
 * @param filename 文件名（.stbc）
 * @return 成功返回字节码模块，失败返回NULL
 */
BytecodeModule* bytecode_load(const char* filename);

/**
 * @brief 保存字节码模块到文件流
 * @param module 字节码模块
 * @param fp 文件指针
 * @return 成功返回OK，失败返回错误码
 */
ErrorCode bytecode_save_to_stream(const BytecodeModule* module, FILE* fp);

/**
 * @brief 从文件流加载字节码模块
 * @param fp 文件指针
 * @return 成功返回字节码模块，失败返回NULL
 */
BytecodeModule* bytecode_load_from_stream(FILE* fp);

/**
 * @brief 验证STBC文件头
 * @param header 文件头
 * @return 有效返回true，否则返回false
 */
bool bytecode_validate_header(const STBCHeader* header);

/**
 * @brief 计算字节码模块的校验和
 * @param module 字节码模块
 * @return 校验和
 */
uint32_t bytecode_compute_checksum(const BytecodeModule* module);

/**
 * @brief 验证字节码模块的校验和
 * @param module 字节码模块
 * @param expected_checksum 期望的校验和
 * @return 匹配返回true，否则返回false
 */
bool bytecode_verify_checksum(const BytecodeModule* module, uint32_t expected_checksum);

#endif // STVM_BYTECODE_IO_H
