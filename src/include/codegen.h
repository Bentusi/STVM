/**
 * @file codegen.h
 * @brief 代码生成器 - AST到字节码转换
 */

#ifndef STVM_CODEGEN_H
#define STVM_CODEGEN_H

#include "ast.h"
#include "bytecode.h"
#include "symtbl.h"
#include "error.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 跳转标签（用于回填）
 */
typedef struct JumpLabel {
    int32_t instruction_index;      // 需要回填的指令索引
    struct JumpLabel* next;         // 链表下一个
} JumpLabel;

/**
 * @brief 代码生成上下文
 */
typedef struct CodeGenContext {
    BytecodeModule* module;         // 目标字节码模块
    SymbolTable* symtbl;            // 当前符号表
    Symbol* current_function;       // 当前正在生成的函数
    int32_t local_var_count;        // 当前函数的局部变量数
    ErrorCode error_code;           // 错误码
    char error_msg[256];            // 错误消息
} CodeGenContext;

/**
 * @brief 循环上下文（用于break/continue）
 */
typedef struct LoopContext {
    JumpLabel* break_labels;        // break语句的跳转标签列表
    JumpLabel* continue_labels;     // continue语句的跳转标签列表
    struct LoopContext* parent;     // 父循环上下文
} LoopContext;

/**
 * @brief 创建代码生成器上下文
 * @param module 字节码模块
 * @param symtbl 符号表
 * @return 代码生成器上下文
 */
CodeGenContext* codegen_create(BytecodeModule* module, SymbolTable* symtbl);

/**
 * @brief 释放代码生成器上下文
 * @param ctx 代码生成器上下文
 */
void codegen_free(CodeGenContext* ctx);

/**
 * @brief 从AST生成字节码
 * @param ctx 代码生成器上下文
 * @param program AST根节点（程序节点）
 * @return 成功返回OK，失败返回错误码
 */
ErrorCode codegen_generate(CodeGenContext* ctx, ASTNode* program);

/**
 * @brief 生成表达式的字节码（结果压栈）
 * @param ctx 代码生成器上下文
 * @param expr 表达式节点
 * @return 成功返回OK，失败返回错误码
 */
ErrorCode codegen_expr(CodeGenContext* ctx, ASTNode* expr);

/**
 * @brief 生成语句的字节码
 * @param ctx 代码生成器上下文
 * @param stmt 语句节点
 * @param loop_ctx 循环上下文（用于break/continue，可为NULL）
 * @return 成功返回OK，失败返回错误码
 */
ErrorCode codegen_stmt(CodeGenContext* ctx, ASTNode* stmt, LoopContext* loop_ctx);

/**
 * @brief 生成函数声明的字节码
 * @param ctx 代码生成器上下文
 * @param func_decl 函数声明节点
 * @return 成功返回OK，失败返回错误码
 */
ErrorCode codegen_function(CodeGenContext* ctx, ASTNode* func_decl);

/**
 * @brief 发射（emit）一条指令
 * @param ctx 代码生成器上下文
 * @param opcode 操作码
 * @param operand 操作数
 * @return 指令的索引位置
 */
int32_t codegen_emit(CodeGenContext* ctx, Opcode opcode, uint16_t operand);

/**
 * @brief 发射一条带flags的指令
 * @param ctx 代码生成器上下文
 * @param opcode 操作码
 * @param flags 标志位
 * @param operand 操作数
 * @return 指令的索引位置
 */
int32_t codegen_emit_with_flags(CodeGenContext* ctx, Opcode opcode, uint8_t flags, uint16_t operand);

/**
 * @brief 回填跳转指令的目标地址
 * @param ctx 代码生成器上下文
 * @param instruction_index 指令索引
 * @param target_address 目标地址
 */
void codegen_patch_jump(CodeGenContext* ctx, int32_t instruction_index, uint16_t target_address);

/**
 * @brief 添加常量到常量池
 * @param ctx 代码生成器上下文
 * @param value 常量值
 * @return 常量在常量池中的索引
 */
int32_t codegen_add_constant(CodeGenContext* ctx, Value value);

/**
 * @brief 获取当前指令位置
 * @param ctx 代码生成器上下文
 * @return 当前指令数量（即下一条指令的索引）
 */
int32_t codegen_current_position(CodeGenContext* ctx);

/**
 * @brief 创建跳转标签
 * @param instruction_index 指令索引
 * @return 跳转标签
 */
JumpLabel* jump_label_create(int32_t instruction_index);

/**
 * @brief 释放跳转标签列表
 * @param labels 跳转标签链表
 */
void jump_label_free_list(JumpLabel* labels);

/**
 * @brief 回填跳转标签列表中的所有跳转
 * @param ctx 代码生成器上下文
 * @param labels 跳转标签链表
 * @param target_address 目标地址
 */
void codegen_patch_jump_list(CodeGenContext* ctx, JumpLabel* labels, uint16_t target_address);

#endif // STVM_CODEGEN_H
