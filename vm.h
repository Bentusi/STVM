/*
 * IEC61131 虚拟机 (VM) 定义
 * 平台无关的字节码执行引擎
 */

#ifndef VM_H
#define VM_H

#include "ast.h"

/* 虚拟机操作码 */
typedef enum {
    VM_LOAD_INT,        // 加载整数到栈
    VM_LOAD_REAL,       // 加载实数到栈
    VM_LOAD_BOOL,       // 加载布尔值到栈
    VM_LOAD_STRING,     // 加载字符串到栈
    VM_LOAD_VAR,        // 加载变量值到栈
    VM_STORE_VAR,       // 存储栈顶值到变量
    VM_PUSH_ARGS,       // 压入参数个数
    VM_CALL,            // 函数调用
    VM_RET,             // 函数返回
    VM_ADD, VM_SUB, VM_MUL, VM_DIV, VM_MOD,  // 算术运算
    VM_EQ, VM_NE, VM_LT, VM_LE, VM_GT, VM_GE,  // 比较运算
    VM_AND, VM_OR, VM_NOT, VM_NEG,  // 逻辑运算
    VM_JMP, VM_JZ, VM_JNZ,  // 跳转指令
    VM_POP, VM_DUP,     // 栈操作
    VM_HALT             // 停机
} VMOpcode;

/* 调用栈帧 */
typedef struct CallFrame {
    int return_pc;      // 返回地址
    int frame_pointer;  // 帧指针
    int local_count;    // 局部变量数量
    struct CallFrame *prev;  // 前一个栈帧
} CallFrame;

/* VM指令结构 */
typedef struct {
    VMOpcode opcode;
    union {
        int int_operand;
        double real_operand;
        int bool_operand;
        char *str_operand;
    };
} VMInstruction;

/* VM值类型 */
typedef struct {
    DataType type;
    union {
        int int_val;
        double real_val;
        int bool_val;
        char *str_val;
    } value;
} VMValue;

/* VM变量 */
typedef struct VMVariable {
    char *name;
    VMValue value;
    struct VMVariable *next;
} VMVariable;

/* VM函数 */
typedef struct VMFunction {
    char *name;
    int addr;
    int param_count;
    struct VMFunction *next;
} VMFunction;

/* VM状态 */
typedef struct {
    VMInstruction *code;
    int code_size;
    int pc;
    
    VMValue *stack;
    int stack_size;
    int sp;
    
    CallFrame *call_stack;  // 调用栈
    VMVariable *variables;
    VMFunction *functions;
    
    int running;
    char *error_msg;
} VMState;

/* VM函数声明 */
VMState *vm_create(void);
void vm_destroy(VMState *vm);
void vm_emit(VMState *vm, VMOpcode opcode, ...);
void vm_compile_ast(VMState *vm, ASTNode *ast);
void vm_compile_function_list(VMState *vm, ASTNode *func_list);
void vm_initialize_global_variables(VMState *vm);
void vm_setup_function_parameters(VMState *vm, char *func_name, int param_count);
void vm_cleanup_function_parameters(VMState *vm);
int vm_run(VMState *vm);
void vm_reset(VMState *vm);

/* 栈操作 */
void vm_push(VMState *vm, VMValue value);
VMValue vm_pop(VMState *vm);

/* 变量管理 */
void vm_set_variable(VMState *vm, const char *name, VMValue value);
VMValue vm_get_variable(VMState *vm, const char *name);

/* 函数管理 */
void vm_set_function(VMState *vm, const char *name, int addr);

/* 调试函数 */
void vm_print_variables(VMState *vm);
void vm_print_functions(VMState *vm);
void vm_print_code(VMState *vm);
void vm_print_value(VMValue value);

/* 错误处理 */
void vm_set_error(VMState *vm, const char *error);
const char *vm_get_error(VMState *vm);

#endif // VM_H