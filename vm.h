/*
 * IEC61131 虚拟机 (VM) 定义
 * 平台无关的字节码执行引擎
 */

#ifndef VM_H
#define VM_H

#include "ast.h"

/* 虚拟机指令集 */
typedef enum {
    /* 数据操作指令 */
    VM_LOAD_INT,    // 加载整数常量
    VM_LOAD_REAL,   // 加载实数常量
    VM_LOAD_BOOL,   // 加载布尔常量
    VM_LOAD_STRING, // 加载字符串常量
    VM_LOAD_VAR,    // 从变量加载值
    VM_STORE_VAR,   // 存储值到变量
    VM_POP,         // 弹出栈顶元素
    VM_DUP,         // 复制栈顶元素
    
    /* 算术运算指令 */
    VM_ADD,         // 加法
    VM_SUB,         // 减法
    VM_MUL,         // 乘法
    VM_DIV,         // 除法
    VM_MOD,         // 取模
    VM_NEG,         // 取负
    
    /* 比较运算指令 */
    VM_EQ,          // 等于
    VM_NE,          // 不等于
    VM_LT,          // 小于
    VM_LE,          // 小于等于
    VM_GT,          // 大于
    VM_GE,          // 大于等于
    
    /* 逻辑运算指令 */
    VM_AND,         // 逻辑与
    VM_OR,          // 逻辑或
    VM_NOT,         // 逻辑非
    
    /* 控制流指令 */
    VM_JMP,         // 无条件跳转
    VM_JZ,          // 条件跳转（为零时跳转）
    VM_JNZ,         // 条件跳转（非零时跳转）
    VM_CALL,        // 函数调用
    VM_RET,         // 函数返回
    VM_LOAD_PARAM,  //
    VM_STORE_PARAM,  //
    VM_PUSH_FRAME,
    VM_POP_FRAME,
    
    /* 系统指令 */
    VM_HALT,        // 停机
    VM_NOP          // 空操作
} VMOpcode;

/* 虚拟机值类型 */
typedef struct {
    DataType type;
    Value value;
} VMValue;

/* 虚拟机指令结构 */
typedef struct {
    VMOpcode opcode;
    union {
        int int_operand;
        double real_operand;
        char *str_operand;
        int addr_operand;
    };
} VMInstruction;

/* 函数信息结构 */
typedef struct VMFunction {
    char *name;
    int address;
    int param_count;
    DataType return_type;
    ParamDecl *params;
    struct VMFunction *next;
} VMFunction;

/* 调用帧结构 */
typedef struct VMFrame {
    int return_address;
    int base_pointer;
    VMFunction *function;
    struct VMFrame *prev;
} VMFrame;

/* 虚拟机变量存储 */
typedef struct VMVariable {
    char *name;
    VMValue value;
    struct VMVariable *next;
} VMVariable;

/* 虚拟机状态 */
typedef struct {
    VMInstruction *code;        // 指令代码段
    int code_size;              // 代码大小
    int pc;                     // 程序计数器
    
    VMValue *stack;             // 操作数栈
    int stack_size;             // 栈大小
    int sp;                     // 栈指针
    
    VMFrame *current_frame;     // 当前调用帧
    int frame_count;            // 调用帧数量
    
    VMVariable *variables;      // 变量存储
    VMFunction *functions;      // 函数表
    
    int running;                // 运行状态
    char *error_msg;           // 错误信息
} VMState;

/* 虚拟机函数声明 */

/* 虚拟机初始化和清理 */
VMState *vm_create(void);
void vm_destroy(VMState *vm);

/* 代码生成 */
void vm_emit(VMState *vm, VMOpcode opcode, ...);
void vm_compile_ast(VMState *vm, ASTNode *ast);

/* 虚拟机执行 */
int vm_run(VMState *vm);
void vm_reset(VMState *vm);

/* 变量操作 */
void vm_set_variable(VMState *vm, const char *name, VMValue value);
VMValue vm_get_variable(VMState *vm, const char *name);

/* 栈操作 */
void vm_push(VMState *vm, VMValue value);
VMValue vm_pop(VMState *vm);

/* 帧管理 */
void vm_push_frame(VMState *vm, VMFunction *func, int return_addr);
void vm_pop_frame(VMState *vm);

/* 函数管理 */
void vm_register_function(VMState *vm, const char *name, int address, DataType return_type, ParamDecl *params);
VMFunction *vm_find_function(VMState *vm, const char *name);
void vm_call_function(VMState *vm, const char *name, int arg_count);
void vm_return_function(VMState *vm);

/* 调试和工具函数 */
void vm_print_code(VMState *vm);
void vm_print_stack(VMState *vm);
void vm_print_variables(VMState *vm);
void vm_print_functions(VMState *vm);

/* 错误处理 */
void vm_set_error(VMState *vm, const char *error);
const char *vm_get_error(VMState *vm);

#endif // VM_H