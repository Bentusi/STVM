/**
 * @file libmgr.h
 * @brief 库管理器 - 负责加载和管理外部库
 * 
 * 功能：
 * 1. 加载.stbc库文件
 * 2. 符号解析和导入
 * 3. 库依赖管理
 * 4. 符号冲突检测
 */

#ifndef STVM_LIBMGR_H
#define STVM_LIBMGR_H

#include "bytecode.h"
#include "symtbl.h"
#include "error.h"
#include <stdbool.h>

/**
 * @brief 已加载库信息
 */
typedef struct LoadedLibrary {
    char* name;                     // 库名称
    char* path;                     // 库文件路径
    BytecodeModule* module;         // 字节码模块
    SymbolTable* symbols;           // 库导出的符号表
    struct LoadedLibrary* next;     // 链表指针
} LoadedLibrary;

/**
 * @brief 导入的符号信息
 */
typedef struct ImportedSymbol {
    char* name;                     // 符号名（可能包含别名）
    char* original_name;            // 原始名称
    LoadedLibrary* library;         // 所属库
    Symbol* symbol;                 // 符号定义
    struct ImportedSymbol* next;    // 链表指针
} ImportedSymbol;

/**
 * @brief 库管理器结构
 */
typedef struct LibraryManager {
    LoadedLibrary* libraries;       // 已加载库列表
    ImportedSymbol* imports;        // 已导入符号列表
    SymbolTable* global_symtbl;     // 全局符号表
    char search_paths[256][512];    // 库搜索路径
    int search_path_count;          // 搜索路径数量
} LibraryManager;

/**
 * @brief 创建库管理器
 * @param global_symtbl 全局符号表
 * @return 库管理器实例
 */
LibraryManager* libmgr_create(SymbolTable* global_symtbl);

/**
 * @brief 释放库管理器
 * @param mgr 库管理器实例
 */
void libmgr_free(LibraryManager* mgr);

/**
 * @brief 添加库搜索路径
 * @param mgr 库管理器实例
 * @param path 搜索路径
 * @return 成功返回true
 */
bool libmgr_add_search_path(LibraryManager* mgr, const char* path);

/**
 * @brief 加载库文件
 * @param mgr 库管理器实例
 * @param filename 库文件名（可以是相对路径或绝对路径）
 * @return 错误码
 */
ErrorCode libmgr_load_library(LibraryManager* mgr, const char* filename);

/**
 * @brief 导入库中的符号
 * @param mgr 库管理器实例
 * @param library_name 库名称
 * @param symbol_name 符号名称
 * @param alias 别名（可选，为NULL表示不使用别名）
 * @return 错误码
 */
ErrorCode libmgr_import_symbol(LibraryManager* mgr, const char* library_name, 
                                const char* symbol_name, const char* alias);

/**
 * @brief 导入库中的所有符号
 * @param mgr 库管理器实例
 * @param library_name 库名称
 * @return 错误码
 */
ErrorCode libmgr_import_all(LibraryManager* mgr, const char* library_name);

/**
 * @brief 查找已加载的库
 * @param mgr 库管理器实例
 * @param name 库名称
 * @return 库信息，未找到返回NULL
 */
LoadedLibrary* libmgr_find_library(LibraryManager* mgr, const char* name);

/**
 * @brief 查找已导入的符号
 * @param mgr 库管理器实例
 * @param name 符号名称
 * @return 符号信息，未找到返回NULL
 */
ImportedSymbol* libmgr_find_import(LibraryManager* mgr, const char* name);

/**
 * @brief 解析符号引用（支持module.symbol格式）
 * @param mgr 库管理器实例
 * @param qualified_name 限定名（如"math.sin"）
 * @return 符号指针，未找到返回NULL
 */
Symbol* libmgr_resolve_symbol(LibraryManager* mgr, const char* qualified_name);

/**
 * @brief 卸载库
 * @param mgr 库管理器实例
 * @param name 库名称
 * @return 错误码
 */
ErrorCode libmgr_unload_library(LibraryManager* mgr, const char* name);

/**
 * @brief 打印已加载的库列表（调试用）
 * @param mgr 库管理器实例
 */
void libmgr_dump_libraries(LibraryManager* mgr);

/**
 * @brief 打印已导入的符号列表（调试用）
 * @param mgr 库管理器实例
 */
void libmgr_dump_imports(LibraryManager* mgr);

/**
 * @brief 获取已加载库的数量
 * @param mgr 库管理器实例
 * @return 库数量
 */
uint32_t libmgr_get_library_count(LibraryManager* mgr);

/**
 * @brief 根据索引获取库名称
 * @param mgr 库管理器实例
 * @param index 索引（0到count-1）
 * @return 库名称，索引无效返回NULL
 */
const char* libmgr_get_library_name(LibraryManager* mgr, uint32_t index);

/**
 * @brief 获取库的字节码模块
 * @param mgr 库管理器实例
 * @param name 库名称
 * @return 字节码模块指针，未找到返回NULL
 */
BytecodeModule* libmgr_get_library_module(LibraryManager* mgr, const char* name);

/**
 * @brief 获取库的完整路径
 * @param mgr 库管理器实例
 * @param name 库名称
 * @return 库路径，未找到返回NULL
 */
const char* libmgr_get_library_path(LibraryManager* mgr, const char* name);

#endif // STVM_LIBMGR_H
