/**
 * @file bytecode.h
 * @brief 精简高效的字节码指令集定义
 * 
 * 设计原则：
 * 1. 最小化指令数量（约28个核心指令）
 * 2. 指令功能正交（不重叠）
 * 3. 操作数编码简单
 * 4. 易于解释执行
 * 5. 为JIT优化留空间
 */

#ifndef STVM_BYTECODE_H
#define STVM_BYTECODE_H

#include <stdint.h>
#include <stdbool.h>
#include "types.h"
#include "error.h"

/**
 * @brief 操作码枚举（精简到28个核心指令）
 */
typedef enum {
    // === 栈操作（5个）===
    OP_PUSH = 0,        // 压入常量 operand: 常量池索引
    OP_POP,             // 弹出栈顶
    OP_DUP,             // 复制栈顶
    OP_LOAD,            // 加载变量 operand: 变量索引（局部/全局由flags决定）
    OP_STORE,           // 存储变量 operand: 变量索引
    
    // === 算术运算（6个）===
    OP_ADD,             // 加法（int/real自动识别）
    OP_SUB,             // 减法
    OP_MUL,             // 乘法
    OP_DIV,             // 除法
    OP_MOD,             // 取模（仅int）
    OP_NEG,             // 取负
    
    // === 比较运算（6个）===
    OP_EQ,              // 等于
    OP_NE,              // 不等于
    OP_LT,              // 小于
    OP_LE,              // 小于等于
    OP_GT,              // 大于
    OP_GE,              // 大于等于
    
    // === 逻辑运算（3个）===
    OP_AND,             // 逻辑与
    OP_OR,              // 逻辑或
    OP_NOT,             // 逻辑非
    
    // === 控制流（5个）===
    OP_JMP,             // 无条件跳转 operand: 目标地址
    OP_JZ,              // 为零跳转 operand: 目标地址
    OP_JNZ,             // 非零跳转 operand: 目标地址
    OP_CALL,            // 调用函数 operand: 函数地址
    OP_RET,             // 返回
    
    // === 其他（5个）===
    OP_HALT,            // 停机
    OP_CALL_EXT,        // 调用外部函数 operand: 函数索引
    OP_NOP,             // 空操作（调试/对齐用）
    OP_LOAD_INDEXED,    // 加载数组元素 operand: 数组基址，栈顶为索引
    OP_STORE_INDEXED,   // 存储数组元素 operand: 数组基址，栈顶为值，栈次为索引
    
    OP_COUNT            // 指令总数
} Opcode;

/**
 * @brief 指令标志位
 */
#define FLAG_GLOBAL  0x01   // LOAD/STORE: 全局变量
#define FLAG_LOCAL   0x00   // LOAD/STORE: 局部变量

/**
 * @brief 指令结构（固定4字节格式）
 * 
 * 布局：[opcode:8][flags:8][operand:16]
 * - opcode: 操作码
 * - flags: 标志位（如LOAD/STORE的全局/局部标志）
 * - operand: 16位操作数（足够大多数情况）
 */
typedef struct {
    uint8_t opcode;     // 操作码
    uint8_t flags;      // 标志位
    uint16_t operand;   // 操作数
} Instruction;

/**
 * @brief 常量类型
 */
typedef enum {
    CONST_INT,
    CONST_REAL,
    CONST_BOOL,
    CONST_STRING
} ConstantType;

/**
 * @brief 常量池条目
 */
typedef struct {
    ConstantType type;
    union {
        int32_t int_val;
        double real_val;
        bool bool_val;
        char* string_val;   // 字符串指针
    };
} Constant;

/**
 * @brief 函数表条目
 */
typedef struct {
    char* name;             // 函数名
    uint32_t address;       // 函数入口地址
    int32_t param_count;    // 参数个数
    int32_t local_count;    // 局部变量个数
    DataType return_type;   // 返回类型
    DataType* param_types;  // 参数类型数组（新增）
} FunctionEntry;

/**
 * @brief 字节码模块
 */
typedef struct {
    // 指令序列
    Instruction* instructions;
    uint32_t instruction_count;
    uint32_t instruction_capacity;
    
    // 常量池
    Constant* constants;
    uint32_t const_count;
    uint32_t const_capacity;
    
    // 函数表
    FunctionEntry* functions;
    uint32_t function_count;
    uint32_t function_capacity;
    
    // 全局变量
    uint32_t global_count;
    
    // 入口点
    uint32_t entry_point;
    
    // 调试信息
    int* line_numbers;      // 指令行号映射（可选）
    char* source_file;      // 源文件名（可选）
} BytecodeModule;

/**
 * @brief 获取操作码名称字符串
 * @param opcode 操作码
 * @return 操作码名称
 */
const char* opcode_to_string(Opcode opcode);

/**
 * @brief 创建字节码模块
 * @return 新的字节码模块指针
 */
BytecodeModule* bytecode_module_create(void);

/**
 * @brief 释放字节码模块
 * @param module 字节码模块指针
 */
void bytecode_module_free(BytecodeModule* module);

/**
 * @brief 添加指令
 * @param module 字节码模块
 * @param opcode 操作码
 * @param flags 标志位
 * @param operand 操作数
 * @return 指令索引
 */
uint32_t bytecode_add_instruction(BytecodeModule* module, Opcode opcode, uint8_t flags, uint16_t operand);

/**
 * @brief 添加指令（带行号）
 * @param module 字节码模块
 * @param opcode 操作码
 * @param flags 标志位
 * @param operand 操作数
 * @param line 源码行号
 * @return 指令索引
 */
uint32_t bytecode_add_instruction_with_line(BytecodeModule* module, Opcode opcode, uint8_t flags, uint16_t operand, int line);

/**
 * @brief 修改指令的操作数（用于回填跳转地址）
 * @param module 字节码模块
 * @param index 指令索引
 * @param operand 新操作数
 */
void bytecode_patch_operand(BytecodeModule* module, uint32_t index, uint16_t operand);

/**
 * @brief 添加整数常量
 * @param module 字节码模块
 * @param value 整数值
 * @return 常量池索引
 */
uint32_t bytecode_add_int_constant(BytecodeModule* module, int32_t value);

/**
 * @brief 添加实数常量
 * @param module 字节码模块
 * @param value 实数值
 * @return 常量池索引
 */
uint32_t bytecode_add_real_constant(BytecodeModule* module, double value);

/**
 * @brief 添加布尔常量
 * @param module 字节码模块
 * @param value 布尔值
 * @return 常量池索引
 */
uint32_t bytecode_add_bool_constant(BytecodeModule* module, bool value);

/**
 * @brief 添加字符串常量
 * @param module 字节码模块
 * @param value 字符串值
 * @return 常量池索引
 */
uint32_t bytecode_add_string_constant(BytecodeModule* module, const char* value);

/**
 * @brief 添加函数到函数表
 * @param module 字节码模块
 * @param name 函数名
 * @param address 函数入口地址
 * @param param_count 参数个数
 * @param local_count 局部变量个数
 * @param return_type 返回类型
 * @param param_types 参数类型数组（可选，传NULL表示不指定）
 * @return 函数索引
 */
uint32_t bytecode_add_function(BytecodeModule* module, const char* name, uint32_t address, 
                                int32_t param_count, int32_t local_count, DataType return_type,
                                const DataType* param_types);

/**
 * @brief 查找函数
 * @param module 字节码模块
 * @param name 函数名
 * @return 函数条目指针，未找到返回NULL
 */
FunctionEntry* bytecode_find_function(BytecodeModule* module, const char* name);

/**
 * @brief 获取当前指令位置（用于标签）
 * @param module 字节码模块
 * @return 当前指令索引
 */
uint32_t bytecode_current_position(BytecodeModule* module);

/**
 * @brief 反汇编指令（调试用）
 * @param instr 指令
 * @param buffer 输出缓冲区
 * @param size 缓冲区大小
 */
void bytecode_disassemble_instruction(Instruction instr, char* buffer, size_t size);

/**
 * @brief 打印字节码模块(调试用)
 * @param module 字节码模块
 */
void bytecode_print_module(BytecodeModule* module);

/**
 * @brief 将库模块合并到主模块(静态链接)
 * @param main 主模块
 * @param library 库模块
 * @param library_name 库名(用于限定符)
 * @return 错误码
 */
ErrorCode bytecode_merge_library(BytecodeModule* main, BytecodeModule* library, const char* library_name);

#endif // STVM_BYTECODE_H
