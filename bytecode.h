#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* 字节码文件魔数 */
#define BYTECODE_MAGIC          "STBC"
#define BYTECODE_VERSION        0x00010000  /* 版本 1.0.0 */
#define BYTECODE_HEADER_SIZE    32

/* 虚拟机指令操作码 - 完整指令集 */
typedef enum {
    /* 空操作和控制 */
    OP_NOP = 0x00,              // 空操作
    OP_HALT,                    // 停机指令
    
    /* 数据移动指令 */
    OP_LOAD_CONST_INT,          // 加载整数常量
    OP_LOAD_CONST_REAL,         // 加载实数常量
    OP_LOAD_CONST_BOOL,         // 加载布尔常量
    OP_LOAD_CONST_STRING,       // 加载字符串常量
    
    OP_LOAD_LOCAL,              // 加载局部变量
    OP_LOAD_GLOBAL,             // 加载全局变量
    OP_LOAD_PARAM,              // 加载参数
    
    OP_STORE_LOCAL,             // 存储到局部变量
    OP_STORE_GLOBAL,            // 存储到全局变量
    OP_STORE_PARAM,             // 存储到参数
    
    /* 栈操作指令 */
    OP_PUSH,                    // 通用压栈
    OP_POP,                     // 弹栈
    OP_DUP,                     // 复制栈顶
    OP_SWAP,                    // 交换栈顶两元素
    
    /* 算术运算指令 - 整数 */
    OP_ADD_INT,                 // 整数加法
    OP_SUB_INT,                 // 整数减法
    OP_MUL_INT,                 // 整数乘法
    OP_DIV_INT,                 // 整数除法
    OP_MOD_INT,                 // 整数取模
    OP_NEG_INT,                 // 整数取负
    
    /* 算术运算指令 - 实数 */
    OP_ADD_REAL,                // 实数加法
    OP_SUB_REAL,                // 实数减法
    OP_MUL_REAL,                // 实数乘法
    OP_DIV_REAL,                // 实数除法
    OP_NEG_REAL,                // 实数取负
    
    /* 逻辑运算指令 */
    OP_AND_BOOL,                // 逻辑与
    OP_OR_BOOL,                 // 逻辑或
    OP_XOR_BOOL,                // 逻辑异或
    OP_NOT_BOOL,                // 逻辑非
    
    /* 比较指令 - 整数 */
    OP_EQ_INT,                  // 整数相等
    OP_NE_INT,                  // 整数不等
    OP_LT_INT,                  // 整数小于
    OP_LE_INT,                  // 整数小于等于
    OP_GT_INT,                  // 整数大于
    OP_GE_INT,                  // 整数大于等于
    
    /* 比较指令 - 实数 */
    OP_EQ_REAL,                 // 实数相等
    OP_NE_REAL,                 // 实数不等
    OP_LT_REAL,                 // 实数小于
    OP_LE_REAL,                 // 实数小于等于
    OP_GT_REAL,                 // 实数大于
    OP_GE_REAL,                 // 实数大于等于
    
    /* 比较指令 - 字符串 */
    OP_EQ_STRING,               // 字符串相等
    OP_NE_STRING,               // 字符串不等
    OP_LT_STRING,               // 字符串小于
    OP_LE_STRING,               // 字符串小于等于
    OP_GT_STRING,               // 字符串大于
    OP_GE_STRING,               // 字符串大于等于
    
    /* 类型转换指令 */
    OP_INT_TO_REAL,             // 整数转实数
    OP_REAL_TO_INT,             // 实数转整数
    OP_INT_TO_STRING,           // 整数转字符串
    OP_REAL_TO_STRING,          // 实数转字符串
    OP_BOOL_TO_STRING,          // 布尔转字符串
    OP_STRING_TO_INT,           // 字符串转整数
    OP_STRING_TO_REAL,          // 字符串转实数
    
    /* 控制流指令 */
    OP_JMP,                     // 无条件跳转
    OP_JMP_TRUE,                // 真值跳转
    OP_JMP_FALSE,               // 假值跳转
    OP_JMP_EQ,                  // 相等跳转
    OP_JMP_NE,                  // 不等跳转
    
    /* 函数调用指令 */
    OP_CALL,                    // 函数调用
    OP_CALL_BUILTIN,            // 内置函数调用
    OP_CALL_LIBRARY,            // 库函数调用
    OP_RET,                     // 函数返回
    OP_RET_VALUE,               // 带返回值的函数返回
    
    /* 数组操作指令 */
    OP_ARRAY_LOAD,              // 数组元素加载
    OP_ARRAY_STORE,             // 数组元素存储
    OP_ARRAY_LEN,               // 数组长度
    
    /* 结构体操作指令 */
    OP_STRUCT_LOAD,             // 结构体成员加载
    OP_STRUCT_STORE,            // 结构体成员存储
    
    /* 调试指令 */
    OP_DEBUG_PRINT,             // 调试打印
    OP_BREAKPOINT,              // 断点
    OP_LINE_INFO,               // 行号信息
    
    /* 同步指令（主从模式） */
    OP_SYNC_VAR,                // 变量同步
    OP_SYNC_CHECKPOINT,         // 同步检查点
    
    /* 指令数量标记 */
    OP_INSTRUCTION_COUNT
} opcode_t;

/* 操作数类型 */
typedef enum {
    OPERAND_NONE,               // 无操作数
    OPERAND_INT,                // 整数操作数
    OPERAND_REAL,               // 实数操作数
    OPERAND_STRING,             // 字符串操作数
    OPERAND_ADDRESS             // 地址操作数
} operand_type_t;

/* 字节码指令结构 */
typedef struct bytecode_instr {
    opcode_t opcode;            // 操作码
    operand_type_t operand_type; // 操作数类型
    
    union {
        int32_t int_operand;    // 整数操作数
        double real_operand;    // 实数操作数
        uint32_t addr_operand;  // 地址操作数
        uint32_t str_index;     // 字符串索引（指向常量池）
    } operand;
    
    /* 调试信息 */
    uint32_t source_line;       // 源码行号
    uint32_t source_column;     // 源码列号
} bytecode_instr_t;

/* 常量池项类型 */
typedef enum {
    CONST_INT,                  // 整数常量
    CONST_REAL,                 // 实数常量
    CONST_BOOL,                 // 布尔常量
    CONST_STRING                // 字符串常量
} const_type_t;

/* 常量池项 */
typedef struct const_pool_item {
    const_type_t type;          // 常量类型
    union {
        int32_t int_val;        // 整数值
        double real_val;        // 实数值
        bool bool_val;          // 布尔值
        struct {
            uint32_t length;    // 字符串长度
            char *data;         // 字符串数据
        } string_val;
    } value;
} const_pool_item_t;

/* 变量描述符 */
typedef struct var_descriptor {
    char name[64];              // 变量名
    uint32_t type;              // 变量类型
    uint32_t offset;            // 内存偏移
    uint32_t size;              // 变量大小
    bool is_global;             // 是否全局变量
} var_descriptor_t;

/* 函数描述符 */
typedef struct func_descriptor {
    char name[64];              // 函数名
    uint32_t address;           // 函数地址
    uint32_t param_count;       // 参数数量
    uint32_t local_var_size;    // 局部变量大小
    uint32_t return_type;       // 返回类型
} func_descriptor_t;

/* 字节码文件头 */
typedef struct bytecode_header {
    char magic[4];              // 魔数 "STBC"
    uint32_t version;           // 版本号
    uint32_t flags;             // 标志位
    uint32_t instr_count;       // 指令数量
    uint32_t const_pool_size;   // 常量池大小
    uint32_t var_count;         // 变量数量
    uint32_t func_count;        // 函数数量
    uint32_t entry_point;       // 程序入口点
} bytecode_header_t;

/* 字节码文件结构 */
typedef struct bytecode_file {
    bytecode_header_t header;           // 文件头
    bytecode_instr_t *instructions;     // 指令序列
    const_pool_item_t *const_pool;      // 常量池
    var_descriptor_t *var_table;        // 变量表
    func_descriptor_t *func_table;      // 函数表
    
    /* 运行时信息 */
    bool is_loaded;                     // 是否已加载
    char *source_filename;              // 源文件名
} bytecode_file_t;

/* 指令信息结构 */
typedef struct instr_info {
    const char *name;                   // 指令名称
    operand_type_t operand_type;        // 操作数类型
    const char *description;            // 指令描述
} instr_info_t;

/* 字节码生成器 */
typedef struct bytecode_generator {
    bytecode_instr_t *instructions;     // 指令缓冲区
    uint32_t instr_count;               // 当前指令数量
    uint32_t instr_capacity;            // 指令缓冲区容量
    
    const_pool_item_t *const_pool;      // 常量池
    uint32_t const_count;               // 常量数量
    uint32_t const_capacity;            // 常量池容量
    
    var_descriptor_t *var_table;        // 变量表
    uint32_t var_count;                 // 变量数量
    uint32_t var_capacity;              // 变量表容量
    
    func_descriptor_t *func_table;      // 函数表
    uint32_t func_count;                // 函数数量
    uint32_t func_capacity;             // 函数表容量
    
    /* 标签和跳转管理 */
    struct {
        char name[32];
        uint32_t address;
        bool is_resolved;
    } labels[256];
    uint32_t label_count;
    
    /* 当前状态 */
    uint32_t current_line;              // 当前源码行号
    uint32_t current_column;            // 当前源码列号
    
} bytecode_generator_t;

/* ========== 字节码文件操作函数 ========== */

/* 文件I/O */
int bytecode_save_file(const bytecode_file_t *bc_file, const char *filename);
int bytecode_load_file(bytecode_file_t *bc_file, const char *filename);
void bytecode_free_file(bytecode_file_t *bc_file);

/* 文件验证 */
bool bytecode_verify_file(const bytecode_file_t *bc_file);
bool bytecode_check_version(uint32_t file_version);

/* ========== 字节码生成器函数 ========== */

/* 生成器管理 */
bytecode_generator_t* bytecode_generator_create(void);
void bytecode_generator_destroy(bytecode_generator_t *gen);
void bytecode_generator_reset(bytecode_generator_t *gen);

/* 指令生成 */
uint32_t bytecode_emit_instr(bytecode_generator_t *gen, opcode_t opcode);
uint32_t bytecode_emit_instr_int(bytecode_generator_t *gen, opcode_t opcode, int32_t operand);
uint32_t bytecode_emit_instr_real(bytecode_generator_t *gen, opcode_t opcode, double operand);
uint32_t bytecode_emit_instr_addr(bytecode_generator_t *gen, opcode_t opcode, uint32_t address);
uint32_t bytecode_emit_instr_string(bytecode_generator_t *gen, opcode_t opcode, const char *str);

/* 常量池管理 */
uint32_t bytecode_add_const_int(bytecode_generator_t *gen, int32_t value);
uint32_t bytecode_add_const_real(bytecode_generator_t *gen, double value);
uint32_t bytecode_add_const_bool(bytecode_generator_t *gen, bool value);
uint32_t bytecode_add_const_string(bytecode_generator_t *gen, const char *str);

/* 变量和函数管理 */
uint32_t bytecode_add_variable(bytecode_generator_t *gen, const char *name, 
                               uint32_t type, uint32_t size, bool is_global);
uint32_t bytecode_add_function(bytecode_generator_t *gen, const char *name,
                               uint32_t param_count, uint32_t return_type);

/* 标签和跳转管理 */
uint32_t bytecode_create_label(bytecode_generator_t *gen, const char *name);
void bytecode_mark_label(bytecode_generator_t *gen, const char *name);
uint32_t bytecode_get_label_address(bytecode_generator_t *gen, const char *name);
void bytecode_patch_jump(bytecode_generator_t *gen, uint32_t instr_index, uint32_t target_addr);

/* 生成最终字节码文件 */
int bytecode_generate_file(bytecode_generator_t *gen, bytecode_file_t *bc_file);

/* ========== 指令信息和调试函数 ========== */

/* 指令信息 */
const instr_info_t* bytecode_get_instr_info(opcode_t opcode);
const char* bytecode_get_opcode_name(opcode_t opcode);
operand_type_t bytecode_get_operand_type(opcode_t opcode);

/* 反汇编 */
void bytecode_disassemble_instr(const bytecode_instr_t *instr, char *buffer, size_t buffer_size);
void bytecode_disassemble_file(const bytecode_file_t *bc_file, FILE *output);

/* 调试输出 */
void bytecode_print_file_info(const bytecode_file_t *bc_file);
void bytecode_print_const_pool(const bytecode_file_t *bc_file);
void bytecode_print_var_table(const bytecode_file_t *bc_file);
void bytecode_print_func_table(const bytecode_file_t *bc_file);

/* ========== 字节码验证和优化函数 ========== */

/* 验证 */
bool bytecode_validate_instructions(const bytecode_file_t *bc_file);
bool bytecode_validate_jumps(const bytecode_file_t *bc_file);
bool bytecode_validate_const_pool(const bytecode_file_t *bc_file);

/* 统计信息 */
typedef struct bytecode_stats {
    uint32_t total_instructions;        // 总指令数
    uint32_t instr_counts[OP_INSTRUCTION_COUNT]; // 各指令使用次数
    uint32_t total_const_pool_size;     // 常量池总大小
    uint32_t total_code_size;           // 代码总大小
    uint32_t jump_instructions;         // 跳转指令数
    uint32_t function_calls;            // 函数调用数
} bytecode_stats_t;

void bytecode_get_statistics(const bytecode_file_t *bc_file, bytecode_stats_t *stats);
void bytecode_print_statistics(const bytecode_stats_t *stats);

/* ========== 错误处理 ========== */

typedef enum {
    BYTECODE_OK = 0,
    BYTECODE_ERROR_INVALID_FILE,
    BYTECODE_ERROR_VERSION_MISMATCH,
    BYTECODE_ERROR_CORRUPTED_DATA,
    BYTECODE_ERROR_INVALID_INSTRUCTION,
    BYTECODE_ERROR_INVALID_OPERAND,
    BYTECODE_ERROR_MEMORY_ERROR,
    BYTECODE_ERROR_IO_ERROR
} bytecode_error_t;

const char* bytecode_get_error_string(bytecode_error_t error);

#endif /* BYTECODE_H */
