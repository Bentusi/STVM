/*
 * IEC61131 虚拟机 (VM) 定义
 * 平台无关的字节码执行引擎
 */

#ifndef VM_H
#define VM_H

#include "ast.h"
#include "common.h"

#define MAX_STACK_SIZE 1024
#define MAX_CODE_SIZE 1024*1024*4

/* 虚拟机操作码
 * 无操作数指令直接使用opcode
 * 栈操作指令:
 * VM_RET   栈顶值作为返回值返回
 * VM_POP   弹出栈顶值
 * VM_DUP   复制栈顶值
 * 
 * 运算指令:
 * VM_ADD   栈顶两个值相加，结果压栈
 * VM_SUB   栈顶两个值相减，结果压栈
 * VM_MUL   栈顶两个值相乘，结果压栈
 * VM_DIV   栈顶两个值相除，结果压栈
 * VM_MOD   栈顶两个值取模，结果压栈
 * VM_NEG   栈顶值取负，结果压栈
 * 
 * 逻辑比较指令:
 * VM_EQ    栈顶两个值相等比较，结果压栈
 * VM_NE    栈顶两个值不等比较，结果压栈
 * VM_LT    栈顶两个值小于比较，结果压栈
 * VM_LE    栈顶两个值小于等于比较，结果压栈
 * VM_AND   栈顶两个值逻辑与，结果压栈
 * VM_OR    栈顶两个值逻辑或，结果压栈
 * VM_NOT   栈顶值逻辑非，结果压栈
 * 
 * 跳转指令:
 * VM_JZ    栈顶值为假时跳转到指定指令地址
 * VM_JNZ   栈顶值为真时跳转到指定指令地址
 * VM_HALT  停机

 * 有操作数指令使用联合体存储操作数
 * 变量操作指令:
 * VM_LOAD_INT    将操作压入栈顶 int_operand: 整数值
 * VM_LOAD_REAL   将操作压入栈顶 real_operand: 实数值
 * VM_LOAD_BOOL   将操作压入栈顶 int_operand: 布尔值 (0或1)
 * VM_LOAD_STRING 将操作压入栈顶 str_operand: 字符串值 (动态分配)
 * VM_LOAD_VAR    将操作压入栈顶 str_operand: 变量名
 * VM_STORE_VAR   弹出栈顶值到 str_operand: 变量名
 * VM_PUSH_ARGS   将栈顶操作数个数压入参数栈，在CALL调用使用 int_operand: 参数个数
 * VM_CALL        先LOAD变量，再压栈，再跳转 int_operand: 函数地址
 * VM_JMP         无条件跳转，设置PC指针到操作数 int_operand: 指令地址
 */
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
    char *function_name; // 当前函数名，用于参数查找
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
    
    int main_entry;
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