#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdint.h>
#include <stdbool.h>

/* 常量定义 */
#define MAX_SCOPE_DEPTH 32
#define SYMBOL_HASH_SIZE 256
#define MAX_SYMBOL_NAME 64
#define SYMBOL_TABLE_SIZE 1024

/* 符号类型 */
typedef enum {
    SYMBOL_VARIABLE,        // 变量
    SYMBOL_FUNCTION,        // 函数
    SYMBOL_CONSTANT,        // 常量
    SYMBOL_TYPE             // 类型定义
} symbol_type_t;

/* 基础数据类型 */
typedef enum {
    TYPE_BOOL_ID,       // 布尔类型
    TYPE_BYTE_ID,       // 字节类型
    TYPE_INT_ID,        // 整数类型
    TYPE_REAL_ID,       // 实数类型
    TYPE_STRING_ID,     // 字符串类型
    TYPE_ARRAY_ID,      // 数组类型
    TYPE_STRUCT_ID,     // 结构体类型
    TYPE_FUNCTION_ID,   // 函数类型
    TYPE_USER_ID        // 用户定义类型
} base_type_t;

/* 变量类别 */
typedef enum {
    SYM_VAR_LOCAL,          // 局部变量
    SYM_VAR_GLOBAL,         // 全局变量
    SYM_VAR_INPUT,          // 输入变量
    SYM_VAR_OUTPUT,         // 输出变量
    SYM_VAR_IN_OUT,         // 输入输出变量
    SYM_VAR_TEMP            // 临时变量
} var_category_t;

/* 函数类别 */
typedef enum {
    SYM_FUNC_FUNCTION,     // 普通函数
    SYM_FUNC_FUNCTION_BLOCK // 功能块
} func_category_t;

/* 作用域类型 */
typedef enum {
    SCOPE_GLOBAL,       // 全局作用域
    SCOPE_PROGRAM,      // 程序作用域
    SCOPE_FUNCTION,     // 函数作用域
    SCOPE_BLOCK         // 块作用域
} scope_type_t;

/* 类型信息 */
typedef struct type_info {
    base_type_t base_type;      // 基础类型
    uint32_t size;              // 类型大小（字节）
    bool is_constant;           // 是否常量类型
    bool is_array;              // 是否数组类型
    
    /* 复合类型信息 */
    union {
        struct {
            struct type_info *element_type;     // 数组元素类型
            uint32_t dimension_count;           // 维数
            uint32_t *bounds;                   // 各维边界
        } array_info;
        
        struct {
            struct type_info *return_type;      // 返回类型
            uint32_t parameter_count;           // 参数数量
        } function_info;
    } compound;
    
    char *type_name;            // 类型名称
} type_info_t;

/* 符号表项 */
typedef struct symbol {
    char name[MAX_SYMBOL_NAME];         // 符号名称
    symbol_type_t type;                 // 符号类型
    type_info_t *data_type;             // 数据类型
    
    uint32_t address;                   // 内存地址或偏移
    scope_type_t scope_level;           // 作用域级别
    
    /* 符号分类信息 */
    union {
        struct {
            var_category_t category;    // 变量类别
            void *value_ptr;            // 变量值指针
        } var;
        
        struct {
            void *implementation;       // 函数实现指针
            uint32_t param_count;       // 参数数量
        } func;
        
        struct {
            void *const_value;          // 常量值
        } constant;
    } info;
    
    /* 库符号标识 */
    bool is_library_symbol;             // 是否来自库
    char *source_library;               // 源库名（如果来自库）
    
    /* 源码位置信息 */
    uint32_t line_number;               // 定义行号
    char *source_file;                  // 定义文件名
    
    /* 链表结构 */
    struct symbol *next;                // 同级链表下一项
    struct symbol *hash_next;           // 哈希表链表
} symbol_t;

/* 作用域 */
typedef struct scope {
    uint32_t level;                     // 作用域级别
    char name[MAX_SYMBOL_NAME];         // 作用域名称
    scope_type_t type;                  // 作用域类型
    
    symbol_t *symbols;                  // 符号链表头
    uint32_t symbol_count;              // 符号数量
    uint32_t next_address;              // 下一个可用地址
    
    /* 哈希表优化查找 */
    symbol_t *hash_table[SYMBOL_HASH_SIZE];
    
    struct scope *parent;               // 父作用域
} scope_t;

/* 符号表管理器 */
typedef struct symbol_table {
    scope_t *global_scope;              // 全局作用域
    scope_t *current_scope;             // 当前作用域
    scope_t *scope_stack[MAX_SCOPE_DEPTH]; // 作用域栈
    uint32_t scope_depth;               // 当前作用域深度
    
    /* 预定义类型 */
    type_info_t *builtin_types[16];     // 内置类型数组
    
    /* 统计信息 */
    uint32_t total_symbols;             // 总符号数
    uint32_t library_symbols;           // 库符号数
    
    bool is_initialized;                // 是否已初始化
} symbol_table_t;

/* ========== 核心接口 ========== */

/* 生命周期管理 */
symbol_table_t* symbol_table_create(void);
int symbol_table_init(symbol_table_t *table);
void symbol_table_destroy(symbol_table_t *table);
bool symbol_table_is_initialized(symbol_table_t *table);

/* 作用域管理 */
int symbol_table_enter_scope(symbol_table_t *table, const char *scope_name, scope_type_t type);
int symbol_table_exit_scope(symbol_table_t *table);
scope_t* symbol_table_get_current_scope(symbol_table_t *table);
scope_t* symbol_table_get_global_scope(symbol_table_t *table);

/* 符号定义（由编译器/库管理器调用） */
symbol_t* symbol_table_define_variable(symbol_table_t *table, const char *name, 
                                       type_info_t *type, var_category_t category);
symbol_t* symbol_table_define_function(symbol_table_t *table, const char *name,
                                       type_info_t *return_type, void *implementation);
symbol_t* symbol_table_define_constant(symbol_table_t *table, const char *name,
                                       type_info_t *type, void *value);

/* 库符号注册（关键接口） */
symbol_t* symbol_table_register_library_function(symbol_table_t *table, const char *name,
                                                 const char *qualified_name, type_info_t *type,
                                                 void *implementation, const char *library_name);
symbol_t* symbol_table_register_library_variable(symbol_table_t *table, const char *name,
                                                 const char *qualified_name, type_info_t *type,
                                                 void *value_ptr, const char *library_name);

/* 符号查找（由VM调用） */
symbol_t* symbol_table_lookup(symbol_table_t *table, const char *name);
symbol_t* symbol_table_lookup_function(symbol_table_t *table, const char *name);
symbol_t* symbol_table_lookup_variable(symbol_table_t *table, const char *name);
bool symbol_table_symbol_exists(symbol_table_t *table, const char *name);

/* 类型管理 */
type_info_t* symbol_table_get_builtin_type(base_type_t type);
type_info_t* symbol_table_create_array_type(type_info_t *element_type, uint32_t dimensions, uint32_t *bounds);
type_info_t* symbol_table_create_function_type(type_info_t *return_type, uint32_t param_count);
int symbol_table_register_user_type(symbol_table_t *table, const char *name, type_info_t *type);

/* 工具接口 */
void symbol_table_print_symbols(symbol_table_t *table);
void symbol_table_print_scope(symbol_table_t *table, scope_t *scope);
uint32_t symbol_table_get_symbol_count(symbol_table_t *table);
uint32_t symbol_table_get_library_symbol_count(symbol_table_t *table);

#endif /* SYMBOL_TABLE_H */