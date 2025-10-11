/**
 * @file typecheck.h
 * @brief ST语言类型检查器
 * 
 * 语义分析和类型检查，验证程序的类型正确性
 */

#ifndef TYPECHECK_H
#define TYPECHECK_H

#include "ast.h"
#include "symtbl.h"
#include "error.h"

// 类型检查器上下文
typedef struct {
    SymbolTable* symtbl;        // 符号表
    Symbol* current_function;   // 当前函数（用于检查return）
    int error_count;            // 错误计数
    bool had_error;             // 是否有错误
} TypeChecker;

/**
 * @brief 初始化类型检查器
 * @param checker 类型检查器指针
 * @param symtbl 符号表指针
 * @return 错误码
 */
ErrorCode typecheck_init(TypeChecker* checker, SymbolTable* symtbl);

/**
 * @brief 清理类型检查器资源
 * @param checker 类型检查器指针
 */
void typecheck_cleanup(TypeChecker* checker);

/**
 * @brief 检查程序
 * @param checker 类型检查器指针
 * @param program 程序AST节点
 * @return 错误码
 */
ErrorCode typecheck_program(TypeChecker* checker, ASTNode* program);

/**
 * @brief 检查函数声明
 * @param checker 类型检查器指针
 * @param function 函数AST节点
 * @return 错误码
 */
ErrorCode typecheck_function(TypeChecker* checker, ASTNode* function);

#endif // TYPECHECK_H
