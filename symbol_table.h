#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdint.h>
#include <stdbool.h>

/* 常量定义 */
#define MAX_SCOPE_DEPTH 32
#define SYMBOL_HASH_SIZE 256
#define MAX_SYMBOL_NAME 64

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
    VAR_LOCAL,          // 局部变量
    VAR_GLOBAL,         // 全局变量
    VAR_INPUT,          // 输入变量
    VAR_OUTPUT,         // 输出变量
    VAR_IN_OUT,         // 输入输出变量
    VAR_TEMP            // 临时变量
} var_category_t;

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
    uint32_t scope_level;               // 作用域级别
    
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
typedef struct scope {
    uint32_t level;                     // 作用域级别
    char *name;                         // 作用域名称
    symbol_t *symbols;                  // 符号链表
    struct scope *parent;               // 父作用域
    struct scope *children;             // 子作用域链表
    struct scope *next_sibling;         // 兄弟作用域
    
    uint32_t symbol_count;              // 符号数量
    uint32_t next_address;              // 下一个可用地址
    scope_type_t scope_type;            // 作用域类型
    
    /* 哈希表优化查找 */
    symbol_t *hash_table[SYMBOL_TABLE_SIZE];
} scope_t;

/* 符号表管理器 */
typedef struct symbol_table_manager {
    scope_t *global_scope;              // 全局作用域
    scope_t *current_scope;             // 当前作用域
    scope_t *scopes[MAX_SCOPE_DEPTH];   // 作用域栈
    uint32_t scope_depth;               // 当前作用域深度
    
    /* 预定义类型 */
    type_info_t *builtin_types[32];     // 内置类型数组
    
    /* 统计信息 */
    uint32_t total_symbols;             // 总符号数
    uint32_t total_types;               // 总类型数
    uint32_t memory_usage;              // 内存使用量
    
    /* 库管理 */
    struct {
        symbol_t **imported_libraries;  // 导入的库
        uint32_t library_count;         // 库数量
        char **library_paths;           // 库路径
        uint32_t path_count;            // 路径数量
    } library_info;
    
    bool is_initialized;                // 是否已初始化
} symbol_table_manager_t;

/* 符号查找结果 */
typedef struct symbol_lookup_result {
    symbol_t *symbol;                   // 找到的符号
    scope_t *scope;                     // 符号所在作用域
    uint32_t distance;                  // 作用域距离
    bool is_exact_match;                // 是否精确匹配
    char *resolved_name;                // 解析后的名称
} symbol_lookup_result_t;

/* 符号表管理接口 */
int symbol_table_init(void);
void symbol_table_cleanup(void);
bool symbol_table_is_initialized(void);

/* 作用域管理 */
scope_t* enter_scope(const char *name, int scope_type);
scope_t* exit_scope(void);
scope_t* get_current_scope(void);
scope_t* get_global_scope(void);
scope_t* find_scope_by_name(const char *name);

/* 符号操作 */
symbol_t* add_symbol(const char *name, symbol_type_t type, type_info_t *data_type);
symbol_t* add_variable_symbol(const char *name, type_info_t *data_type, 
                             var_category_t category, void *default_value);
symbol_t* add_function_symbol(const char *name, type_info_t *return_type,
                             symbol_t **parameters, uint32_t param_count,
                             function_category_t category);
symbol_t* add_library_symbol(const char *name, const char *version);
symbol_t* add_builtin_symbol(const char *name, symbol_type_t type, void *implementation);

/* 符号查找 */
symbol_t* lookup_symbol(const char *name);
symbol_t* lookup_symbol_in_scope(const char *name, scope_t *scope);
symbol_t* lookup_qualified_symbol(const char *qualified_name);
symbol_lookup_result_t* lookup_symbol_detailed(const char *name);

/* 符号管理 */
bool remove_symbol(const char *name);
bool update_symbol_value(const char *name, void *new_value);
bool set_symbol_attributes(const char *name, symbol_attributes_t attributes);

/* 类型管理 */
type_info_t* create_basic_type(int base_type);
type_info_t* create_array_type(type_info_t *element_type, uint32_t dimensions, uint32_t *bounds);
type_info_t* create_struct_type(const char *name, symbol_t **members, uint32_t member_count);
type_info_t* create_enum_type(const char *name, symbol_t **values, uint32_t value_count);
type_info_t* create_function_type(type_info_t *return_type, type_info_t **param_types, uint32_t param_count);
type_info_t* lookup_type(const char *type_name);
bool register_user_type(const char *name, type_info_t *type);

/* 库符号管理 */
bool import_library_symbols(const char *library_name, const char *alias);
bool export_symbol(const char *name);
symbol_t* resolve_library_symbol(const char *library_name, const char *symbol_name);
bool add_library_path(const char *path);

/* 符号表遍历 */
typedef bool (*symbol_visitor_fn)(symbol_t *symbol, void *context);
void traverse_symbols(symbol_visitor_fn visitor, void *context);
void traverse_scope_symbols(scope_t *scope, symbol_visitor_fn visitor, void *context);

/* 符号表验证 */
bool validate_symbol_table(void);
bool check_symbol_conflicts(void);
bool verify_symbol_types(void);

/* 调试和诊断 */
void print_symbol_table(void);
void print_scope_symbols(scope_t *scope, int indent);
void print_symbol_info(symbol_t *symbol);
void dump_symbol_statistics(void);

/* 内存管理 */
uint32_t get_symbol_table_memory_usage(void);
void compact_symbol_table(void);

/* 序列化支持 */
int save_symbol_table(const char *filename);
int load_symbol_table(const char *filename);

int init_symbol_table(void);
int cleanup_symbol_table(void);

#endif /* SYMBOL_TABLE_H */
