#ifndef BYTECODE_GENERATOR_H
#define BYTECODE_GENERATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "ast.h"
#include "symbol_table.h"

/* 前向声明 */
typedef struct bytecode_generator bytecode_generator_t;
typedef struct constant_pool constant_pool_t;
typedef struct string_pool string_pool_t;
typedef struct instruction instruction_t;
typedef struct debug_info debug_info_t;
typedef struct line_mapping line_mapping_t;
typedef struct error_list error_list_t;
typedef struct warning_list warning_list_t;

/* 输出格式类型 */
typedef enum {
    BC_OUTPUT_BINARY,        // 二进制格式
    BC_OUTPUT_TEXT,          // 文本格式
    BC_OUTPUT_MEMORY         // 内存格式(用于直接VM加载)
} output_format_t;

/* 优化级别 */
typedef enum {
    BC_OPT_NONE = 0,        // 无优化
    BC_OPT_BASIC = 1,       // 基础优化
    BC_OPT_STANDARD = 2,    // 标准优化
    BC_OPT_AGGRESSIVE = 3   // 激进优化
} optimization_level_t;

/* 字节码指令操作码 */
typedef enum {
    // 数据操作指令
    OP_LOAD_CONST_INT,      // 加载整数常量
    OP_LOAD_CONST_REAL,     // 加载实数常量
    OP_LOAD_CONST_BOOL,     // 加载布尔常量
    OP_LOAD_CONST_STRING,   // 加载字符串常量
    OP_LOAD_VAR,            // 加载变量
    OP_STORE_VAR,           // 存储变量
    OP_LOAD_PARAM,          // 加载函数参数
    OP_STORE_PARAM,         // 存储函数参数
    OP_LOAD_LOCAL,          // 加载局部变量
    OP_STORE_LOCAL,         // 存储局部变量
    
    // 算术运算指令
    OP_ADD,                 // 加法
    OP_SUB,                 // 减法
    OP_MUL,                 // 乘法
    OP_DIV,                 // 除法
    OP_MOD,                 // 取模
    OP_NEG,                 // 取负
    OP_ABS,                 // 绝对值
    
    // 比较运算指令
    OP_EQ,                  // 等于
    OP_NE,                  // 不等于
    OP_LT,                  // 小于
    OP_LE,                  // 小于等于
    OP_GT,                  // 大于
    OP_GE,                  // 大于等于
    
    // 逻辑运算指令
    OP_AND,                 // 逻辑与
    OP_OR,                  // 逻辑或
    OP_NOT,                 // 逻辑非
    
    // 控制流指令
    OP_JMP,                 // 无条件跳转
    OP_JMP_IF,              // 条件跳转(真时跳转)
    OP_JMP_UNLESS,          // 条件跳转(假时跳转)
    OP_CALL,                // 函数调用
    OP_RET,                 // 函数返回
    OP_RET_VALUE,           // 带返回值的函数返回
    
    // 栈操作指令
    OP_PUSH,                // 压栈
    OP_POP,                 // 出栈
    OP_DUP,                 // 复制栈顶
    OP_SWAP,                // 交换栈顶两元素
    
    // 系统指令
    OP_HALT,                // 停机
    OP_NOP,                 // 空操作
    OP_DEBUG_BREAK,         // 调试断点
    
    // 扩展指令
    OP_SYNC_CHECKPOINT,     // 同步检查点
    OP_LIB_CALL,            // 库函数调用
    
    OP_COUNT                // 指令总数
} opcode_t;

/* 字节码指令结构 */
typedef struct instruction {
    opcode_t opcode;            // 操作码
    union {
        struct {
            uint32_t operand1;  // 操作数1
            uint32_t operand2;  // 操作数2
        };
        uint64_t long_operand;  // 长操作数
        double real_operand;    // 实数操作数
        char *string_operand;   // 字符串操作数
    };
    uint32_t line_number;       // 源码行号(调试用)
    uint32_t column_number;     // 源码列号(调试用)
} instruction_t;

/* 常量池条目 */
typedef struct constant_entry {
    enum {
        CONST_INT,
        CONST_REAL,
        CONST_BOOL,
        CONST_STRING
    } type;
    union {
        int64_t int_value;
        double real_value;
        bool bool_value;
        char *string_value;
    } value;
    uint32_t index;             // 在常量池中的索引
    struct constant_entry *next;
} constant_entry_t;

/* 常量池 */
typedef struct constant_pool {
    constant_entry_t *entries;
    uint32_t count;
    uint32_t capacity;
} constant_pool_t;

/* 字符串池 */
typedef struct string_pool {
    char **strings;
    uint32_t count;
    uint32_t capacity;
} string_pool_t;

/* 变量表条目 */
typedef struct variable_entry {
    char *name;                 // 变量名
    base_type_t type;           // 变量类型
    uint32_t offset;            // 内存偏移
    bool is_global;             // 是否为全局变量
    bool is_parameter;          // 是否为函数参数
    union {
        int64_t int_value;      // 初始值
        double real_value;
        bool bool_value;
        char *string_value;
    } initial_value;
    struct variable_entry *next;
} variable_entry_t;

/* 函数表条目 */
typedef struct function_entry {
    char *name;                 // 函数名
    uint32_t address;           // 函数地址
    base_type_t return_type;    // 返回类型
    uint32_t param_count;       // 参数个数
    func_category_t *func_type; // 函数类型
    char **param_names;         // 参数名列表
    uint32_t local_var_count;   // 局部变量个数
    bool is_builtin;            // 是否为内置函数
    struct function_entry *next;
} function_entry_t;

/* 调试信息 */
typedef struct debug_info {
    uint32_t *pc_to_line;       // PC到行号的映射
    char **line_to_source;      // 行号到源码的映射
    uint32_t instruction_count;
    uint32_t source_line_count;
    char *source_filename;
} debug_info_t;

/* 行号映射 */
typedef struct line_mapping {
    uint32_t pc;                // 程序计数器
    uint32_t line;              // 源码行号
    uint32_t column;            // 源码列号
    struct line_mapping *next;
} line_mapping_t;

/* 错误信息 */
typedef struct error_entry {
    enum {
        ERROR_LEVEL_ERROR,
        ERROR_LEVEL_WARNING,
        ERROR_LEVEL_INFO
    } level;
    uint32_t line;
    uint32_t column;
    char *message;
    char *suggestion;
    struct error_entry *next;
} error_entry_t;

/* 错误列表 */
typedef struct error_list {
    error_entry_t *errors;
    uint32_t error_count;
    uint32_t warning_count;
} error_list_t;

/* 字节码文件头 */
typedef struct bytecode_file_header {
    uint32_t magic;             // 魔数 'STBC'
    uint16_t version_major;     // 主版本号
    uint16_t version_minor;     // 次版本号
    uint32_t flags;             // 标志位
    uint32_t timestamp;         // 编译时间戳
    uint32_t checksum;          // 文件校验和
    uint32_t header_size;       // 文件头大小
    uint32_t section_count;     // 段数量
} bytecode_file_header_t;

/* 字节码文件段头 */
typedef struct bytecode_section_header {
    uint32_t type;              // 段类型
    uint32_t offset;            // 段偏移
    uint32_t size;              // 段大小
    uint32_t flags;             // 段标志
} bytecode_section_header_t;

/* 字节码文件段类型 */
typedef enum {
    BC_SECTION_METADATA = 0,    // 元数据段
    BC_SECTION_CONSTANTS,       // 常量池段
    BC_SECTION_STRINGS,         // 字符串池段
    BC_SECTION_VARIABLES,       // 全局变量段
    BC_SECTION_FUNCTIONS,       // 函数表段
    BC_SECTION_INSTRUCTIONS,    // 指令段
    BC_SECTION_DEBUG_INFO,      // 调试信息段
    BC_SECTION_CHECKSUM         // 校验段
} bytecode_section_type_t;

/* 字节码文件结构 */
typedef struct bytecode_file {
    bytecode_file_header_t header;
    bytecode_section_header_t *section_headers;
    
    // 各段数据
    void *metadata;
    constant_pool_t *constants;
    string_pool_t *strings;
    variable_entry_t *global_vars;
    function_entry_t *functions;
    instruction_t *instructions;
    uint32_t instruction_count;
    debug_info_t *debug_info;
    
    // 文件信息
    char *filename;
    uint32_t file_size;
    bool is_loaded;
} bytecode_file_t;

/* 字节码生成器主结构 */
typedef struct bytecode_generator {
    // 编译上下文
    ast_node_t *ast_root;
    symbol_table_t *symbol_table;
    
    // 字节码生成
    instruction_t *instructions;
    uint32_t instruction_count;
    uint32_t instruction_capacity;
    
    // 符号池管理
    constant_pool_t *constant_pool;
    string_pool_t *string_pool;
    
    // 符号表
    variable_entry_t *global_vars;
    function_entry_t *functions;
    
    // 当前编译状态
    function_entry_t *current_function;
    uint32_t current_local_offset;
    uint32_t label_counter;
    
    // 跳转标签管理
    struct {
        char *name;
        uint32_t address;
        bool resolved;
    } *labels;
    uint32_t label_count;
    uint32_t label_capacity;
    
    // 未解析的跳转
    struct {
        uint32_t instruction_index;
        char *label_name;
    } *unresolved_jumps;
    uint32_t unresolved_jump_count;
    uint32_t unresolved_jump_capacity;
    
    // 元数据和调试信息
    debug_info_t *debug_info;
    line_mapping_t *line_mapping;
    
    // 输出控制
    output_format_t output_format;
    optimization_level_t opt_level;
    bool generate_debug_info;
    bool enable_sync_support;
    
    // 错误处理
    error_list_t *errors;
    
    // 统计信息
    struct {
        uint32_t constants_generated;
        uint32_t variables_processed;
        uint32_t functions_compiled;
        uint32_t optimizations_applied;
    } stats;
} bytecode_generator_t;

/* 符号信息结构 */
typedef struct symbol_info {
    char *name;
    enum {
        SYMBOL_VARIABLE,
        SYMBOL_FUNCTION,
        SYMBOL_CONSTANT
    } type;
    base_type_t data_type;
    uint32_t address;
    bool is_global;
} symbol_info_t;

/* API函数声明 */

/* 创建和销毁 */
bytecode_generator_t* bc_generator_create(void);
void bc_generator_destroy(bytecode_generator_t* gen);

/* 初始化和配置 */
int bc_generator_init(bytecode_generator_t* gen, symbol_table_t* sym_table);
int bc_generator_set_output_format(bytecode_generator_t* gen, output_format_t format);
int bc_generator_set_optimization_level(bytecode_generator_t* gen, optimization_level_t level);
void bc_generator_enable_debug_info(bytecode_generator_t* gen, bool enable);
void bc_generator_enable_sync_support(bytecode_generator_t* gen, bool enable);

/* 编译流程 */
int bc_generator_compile_program(bytecode_generator_t* gen, ast_node_t* ast);
int bc_generator_generate_output(bytecode_generator_t* gen, const char* output_file);

/* 符号管理 */
int bc_generator_add_global_variable(bytecode_generator_t* gen, const char* name, 
                                      base_type_t type, void* initial_value);
int bc_generator_add_function(bytecode_generator_t* gen, const char* name, 
                              base_type_t return_type, uint32_t param_count,
                              func_category_t* func_types, char** param_names);
symbol_info_t* bc_generator_resolve_symbol(bytecode_generator_t* gen, const char* name);

/* 指令生成 */
int bc_generator_emit_instruction(bytecode_generator_t* gen, opcode_t opcode, ...);
int bc_generator_emit_label(bytecode_generator_t* gen, const char* label);
int bc_generator_patch_jump(bytecode_generator_t* gen, uint32_t jump_addr, uint32_t target_addr);

/* 常量池管理 */
uint32_t bc_generator_add_int_constant(bytecode_generator_t* gen, int64_t value);
uint32_t bc_generator_add_real_constant(bytecode_generator_t* gen, double value);
uint32_t bc_generator_add_bool_constant(bytecode_generator_t* gen, bool value);
uint32_t bc_generator_add_string_constant(bytecode_generator_t* gen, const char* value);

/* 字节码文件操作 */
int bc_generator_save_to_file(bytecode_generator_t* gen, const char* filename);
int bc_generator_save_to_memory(bytecode_generator_t* gen, bytecode_file_t* bc_file);
int bc_file_load(bytecode_file_t* bc_file, const char* filename);
int bc_file_save(const bytecode_file_t* bc_file, const char* filename);
void bc_file_free(bytecode_file_t* bc_file);
bool bc_file_verify(const bytecode_file_t* bc_file);

/* 反汇编和调试 */
int bc_generator_disassemble(bytecode_generator_t* gen, FILE* output);
int bc_file_disassemble(const bytecode_file_t* bc_file, FILE* output);
void bc_generator_print_symbols(bytecode_generator_t* gen);
void bc_generator_print_constants(bytecode_generator_t* gen);
void bc_generator_print_statistics(bytecode_generator_t* gen);

/* 错误处理 */
const char* bc_generator_get_last_error(bytecode_generator_t* gen);
uint32_t bc_generator_get_error_count(bytecode_generator_t* gen);
uint32_t bc_generator_get_warning_count(bytecode_generator_t* gen);
error_entry_t* bc_generator_get_errors(bytecode_generator_t* gen);

/* 优化器接口 */
int bc_generator_optimize(bytecode_generator_t* gen);
int bc_generator_apply_constant_folding(bytecode_generator_t* gen);
int bc_generator_apply_dead_code_elimination(bytecode_generator_t* gen);
int bc_generator_apply_jump_optimization(bytecode_generator_t* gen);

/* 虚拟机接口适配 */
int bc_generator_load_to_vm(bytecode_generator_t* gen, void* vm);
symbol_info_t* bc_generator_get_symbol_info(bytecode_generator_t* gen, const char* name);
debug_info_t* bc_generator_get_debug_info_at_pc(bytecode_generator_t* gen, uint32_t pc);

/* 工具函数 */
const char* bc_opcode_to_string(opcode_t opcode);
const char* bc_data_type_to_string(base_type_t type);
uint32_t bc_calculate_checksum(const void* data, uint32_t size);

/* 内联函数和宏定义 */
#define BC_MAGIC_NUMBER 0x53544243  // 'STBC'
#define BC_VERSION_MAJOR 1
#define BC_VERSION_MINOR 0
#define BC_DEFAULT_INSTRUCTION_CAPACITY 1024
#define BC_DEFAULT_CONSTANT_CAPACITY 256
#define BC_DEFAULT_LABEL_CAPACITY 128

/* 错误代码 */
typedef enum {
    BC_SUCCESS = 0,
    BC_ERROR_INVALID_PARAM = -1,
    BC_ERROR_OUT_OF_MEMORY = -2,
    BC_ERROR_COMPILE_FAILED = -3,
    BC_ERROR_FILE_IO = -4,
    BC_ERROR_INVALID_FORMAT = -5,
    BC_ERROR_SYMBOL_NOT_FOUND = -6,
    BC_ERROR_TYPE_MISMATCH = -7,
    BC_ERROR_OPTIMIZATION_FAILED = -8
} bc_error_code_t;

#endif /* BYTECODE_GENERATOR_H */
