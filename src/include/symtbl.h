/**
 * @file symtbl.h
 * @brief 符号表 - 管理变量、函数和类型符号
 */

#ifndef STVM_SYMTBL_H
#define STVM_SYMTBL_H

#include <stdint.h>
#include <stdbool.h>
#include "types.h"

/**
 * @brief 符号类型
 */
typedef enum {
    SYM_VARIABLE,       // 变量
    SYM_FUNCTION,       // 函数
    SYM_PARAMETER,      // 参数
    SYM_CONSTANT        // 常量
} SymbolKind;

/**
 * @brief 符号定义
 */
typedef struct Symbol {
    char* name;                     // 符号名称
    char* qualified_name;           // 完全限定名（如：module.function）
    SymbolKind kind;                // 符号类型
    TypeInfo* type;                 // 类型信息
    bool owns_type;                 // 是否拥有类型信息（决定是否在释放时free type）
    
    // 作用域信息
    int scope_level;                // 作用域层级（0=全局）
    
    // 变量/参数信息
    int32_t offset;                 // 内存偏移（局部变量/参数）
    int32_t index;                  // 全局变量索引
    bool is_global;                 // 是否为全局变量
    bool is_static;                 // 是否为静态变量（函数内VAR块）
    char* function_scope;           // 所属函数名（静态变量用）
    bool is_external;               // 是否为外部I/O变量
    char* io_address;               // I/O地址（外部变量用）
    
    // 函数信息
    uint32_t address;               // 函数入口地址（字节码）
    int32_t param_count;            // 参数个数
    int32_t local_count;            // 局部变量个数
    
    // 库信息
    bool is_library;                // 是否来自库
    char* library_name;             // 所属库名
    
    // 常量信息
    Value const_value;              // 常量值（如果是常量）
    
    // 哈希表链接
    struct Symbol* next;            // 哈希冲突链表
} Symbol;

/**
 * @brief 作用域结构
 */
typedef struct Scope {
    int level;                      // 作用域层级
    Symbol** symbols;               // 符号哈希表
    int symbol_count;               // 符号数量
    struct Scope* parent;           // 父作用域
    struct Scope* next;             // 兄弟作用域（用于管理）
} Scope;

/**
 * @brief 符号表结构
 */
typedef struct {
    Scope* global_scope;            // 全局作用域
    Scope* current_scope;           // 当前作用域
    int current_level;              // 当前层级
    
    // 变量计数器
    int32_t global_var_count;       // 全局变量计数
    int32_t local_var_offset;       // 当前函数局部变量偏移
    
    // 错误信息
    int error_code;
    char error_msg[256];
} SymbolTable;

/**
 * @brief 初始化符号表
 * @return 新的符号表指针
 */
SymbolTable* symtbl_init(void);

/**
 * @brief 释放符号表
 * @param symtbl 符号表指针
 */
void symtbl_free(SymbolTable* symtbl);

/**
 * @brief 进入新作用域
 * @param symtbl 符号表
 */
void symtbl_enter_scope(SymbolTable* symtbl);

/**
 * @brief 离开当前作用域
 * @param symtbl 符号表
 */
void symtbl_leave_scope(SymbolTable* symtbl);

/**
 * @brief 定义变量符号
 * @param symtbl 符号表
 * @param name 变量名
 * @param type 类型信息
 * @param is_const 是否为常量
 * @return 符号指针，失败返回NULL
 */
Symbol* symtbl_define_variable(SymbolTable* symtbl, const char* name, TypeInfo* type, bool is_const);

/**
 * @brief 定义函数作用域的静态变量（函数内VAR块）
 * @param symtbl 符号表
 * @param name 变量名（短名）
 * @param type 类型信息
 * @param is_const 是否为常量
 * @param function_name 所属函数名
 * @return 符号指针，失败返回NULL
 */
Symbol* symtbl_define_static_variable(SymbolTable* symtbl, const char* name, TypeInfo* type, 
                                      bool is_const, const char* function_name);

/**
 * @brief 定义函数符号
 * @param symtbl 符号表
 * @param name 函数名
 * @param return_type 返回类型
 * @param param_types 参数类型数组
 * @param param_count 参数个数
 * @return 符号指针，失败返回NULL
 */
Symbol* symtbl_define_function(SymbolTable* symtbl, const char* name, TypeInfo* return_type,
                                TypeInfo** param_types, int param_count);

/**
 * @brief 定义参数符号
 * @param symtbl 符号表
 * @param name 参数名
 * @param type 类型信息
 * @return 符号指针，失败返回NULL
 */
Symbol* symtbl_define_parameter(SymbolTable* symtbl, const char* name, TypeInfo* type);

/**
 * @brief 定义常量符号
 * @param symtbl 符号表
 * @param name 常量名
 * @param value 常量值
 * @return 符号指针，失败返回NULL
 */
Symbol* symtbl_define_constant(SymbolTable* symtbl, const char* name, Value value);

/**
 * @brief 查找符号（在当前作用域及父作用域中查找）
 * @param symtbl 符号表
 * @param name 符号名（可以是限定名，如：module.symbol）
 * @return 符号指针，未找到返回NULL
 */
Symbol* symtbl_lookup(SymbolTable* symtbl, const char* name);

/**
 * @brief 仅在当前作用域中查找符号
 * @param symtbl 符号表
 * @param name 符号名
 * @return 符号指针，未找到返回NULL
 */
Symbol* symtbl_lookup_current_scope(SymbolTable* symtbl, const char* name);

/**
 * @brief 在全局作用域中查找符号
 * @param symtbl 符号表
 * @param name 符号名
 * @return 符号指针，未找到返回NULL
 */
Symbol* symtbl_lookup_global(SymbolTable* symtbl, const char* name);

/**
 * @brief 注册库符号
 * @param symtbl 符号表
 * @param library_name 库名
 * @param symbol 符号
 * @return 成功返回true，失败返回false
 */
bool symtbl_register_library_symbol(SymbolTable* symtbl, const char* library_name, Symbol* symbol);

/**
 * @brief 创建限定名
 * @param module_name 模块名
 * @param symbol_name 符号名
 * @return 限定名字符串（需要调用者释放）
 */
char* symtbl_make_qualified_name(const char* module_name, const char* symbol_name);

/**
 * @brief 解析限定名（分离模块名和符号名）
 * @param qualified_name 限定名
 * @param module_name 输出模块名（调用者提供缓冲区）
 * @param symbol_name 输出符号名（调用者提供缓冲区）
 * @param buffer_size 缓冲区大小
 * @return 成功返回true，失败返回false
 */
bool symtbl_parse_qualified_name(const char* qualified_name, char* module_name, 
                                  char* symbol_name, size_t buffer_size);

/**
 * @brief 获取当前作用域层级
 * @param symtbl 符号表
 * @return 作用域层级
 */
int symtbl_current_level(SymbolTable* symtbl);

/**
 * @brief 重置局部变量偏移计数器
 * @param symtbl 符号表
 */
void symtbl_reset_local_offset(SymbolTable* symtbl);

/**
 * @brief 获取下一个全局变量索引
 * @param symtbl 符号表
 * @return 全局变量索引
 */
int32_t symtbl_next_global_index(SymbolTable* symtbl);

/**
 * @brief 获取下一个局部变量偏移
 * @param symtbl 符号表
 * @return 局部变量偏移
 */
int32_t symtbl_next_local_offset(SymbolTable* symtbl);

/**
 * @brief 遍历当前作用域的所有符号
 * @param symtbl 符号表
 * @param callback 回调函数
 * @param user_data 用户数据
 */
typedef void (*SymbolIterator)(Symbol* symbol, void* user_data);
void symtbl_iterate_current_scope(SymbolTable* symtbl, SymbolIterator callback, void* user_data);

/**
 * @brief 遍历所有作用域的所有符号
 * @param symtbl 符号表
 * @param callback 回调函数
 * @param user_data 用户数据
 */
void symtbl_iterate_all(SymbolTable* symtbl, SymbolIterator callback, void* user_data);

/**
 * @brief 打印符号表（调试用）
 * @param symtbl 符号表
 */
void symtbl_print(SymbolTable* symtbl);

/**
 * @brief 打印符号信息（调试用）
 * @param symbol 符号
 */
void symbol_print(Symbol* symbol);

#endif // STVM_SYMTBL_H
