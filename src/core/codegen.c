/**
 * @file codegen.c
 * @brief 代码生成器的实现
 * 
 * 代码生成策略：
 * 1. 表达式采用栈式求值（后序遍历）
 * 2. 跳转使用回填技术
 * 3. 变量访问通过符号表获取偏移量
 * 4. 函数调用需要参数入栈
 */

#include "codegen.h"
#include "mmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 前向声明
static ErrorCode generate_program(CodeGenContext* ctx, ASTNode* program);
static ErrorCode generate_var_decls(CodeGenContext* ctx, ASTNode* var_decls);
static ErrorCode generate_binary_op(CodeGenContext* ctx, ASTNode* node);
static ErrorCode generate_unary_op(CodeGenContext* ctx, ASTNode* node);
static ErrorCode generate_identifier(CodeGenContext* ctx, ASTNode* node);
static ErrorCode generate_literal(CodeGenContext* ctx, ASTNode* node);
static ErrorCode generate_function_call(CodeGenContext* ctx, ASTNode* node);
static ErrorCode generate_array_access(CodeGenContext* ctx, ASTNode* node);
static ErrorCode generate_assign(CodeGenContext* ctx, ASTNode* node);
static ErrorCode generate_if(CodeGenContext* ctx, ASTNode* node, LoopContext* loop_ctx);
static ErrorCode generate_while(CodeGenContext* ctx, ASTNode* node, LoopContext* loop_ctx);
static ErrorCode generate_for(CodeGenContext* ctx, ASTNode* node, LoopContext* loop_ctx);
static ErrorCode generate_return(CodeGenContext* ctx, ASTNode* node);
static ErrorCode generate_block(CodeGenContext* ctx, ASTNode* stmts, LoopContext* loop_ctx);

/**
 * @brief 创建代码生成器上下文
 */
CodeGenContext* codegen_create(BytecodeModule* module, SymbolTable* symtbl) {
    CodeGenContext* ctx = (CodeGenContext*)mmgr_alloc(sizeof(CodeGenContext));
    if (!ctx) return NULL;
    
    memset(ctx, 0, sizeof(CodeGenContext));
    ctx->module = module;
    ctx->symtbl = symtbl;
    ctx->current_function = NULL;
    ctx->local_var_count = 0;
    ctx->error_code = OK;
    
    return ctx;
}

/**
 * @brief 释放代码生成器上下文
 */
void codegen_free(CodeGenContext* ctx) {
    if (!ctx) return;
    mmgr_free(ctx);
}

/**
 * @brief 发射一条指令
 */
int32_t codegen_emit(CodeGenContext* ctx, Opcode opcode, uint16_t operand) {
    return bytecode_add_instruction(ctx->module, opcode, 0, operand);
}

/**
 * @brief 发射一条带flags的指令
 */
int32_t codegen_emit_with_flags(CodeGenContext* ctx, Opcode opcode, uint8_t flags, uint16_t operand) {
    Instruction instr;
    instr.opcode = opcode;
    instr.flags = flags;
    instr.operand = operand;
    
    // 直接添加到指令数组
    if (ctx->module->instruction_count >= ctx->module->instruction_capacity) {
        // 扩容
        size_t new_capacity = ctx->module->instruction_capacity * 2;
        if (new_capacity == 0) new_capacity = 256;
        
        Instruction* new_instructions = (Instruction*)mmgr_realloc(
            ctx->module->instructions,
            new_capacity * sizeof(Instruction)
        );
        if (!new_instructions) return -1;
        
        ctx->module->instructions = new_instructions;
        ctx->module->instruction_capacity = new_capacity;
    }
    
    int32_t index = ctx->module->instruction_count;
    ctx->module->instructions[index] = instr;
    ctx->module->instruction_count++;
    
    return index;
}

/**
 * @brief 回填跳转指令
 */
void codegen_patch_jump(CodeGenContext* ctx, int32_t instruction_index, uint16_t target_address) {
    if (instruction_index >= 0 && (uint32_t)instruction_index < ctx->module->instruction_count) {
        ctx->module->instructions[instruction_index].operand = target_address;
    }
}

/**
 * @brief 添加常量
 */
int32_t codegen_add_constant(CodeGenContext* ctx, Value value) {
    // 根据Value类型调用相应的bytecode_add_*_constant函数
    switch (value.type) {
        case TYPE_INT:
            return bytecode_add_int_constant(ctx->module, value.int_val);
        case TYPE_REAL:
            return bytecode_add_real_constant(ctx->module, value.real_val);
        case TYPE_BOOL:
            return bytecode_add_bool_constant(ctx->module, value.bool_val);
        case TYPE_STRING:
            return bytecode_add_string_constant(ctx->module, value.string_val);
        default:
            return -1;
    }
}

/**
 * @brief 获取当前指令位置
 */
int32_t codegen_current_position(CodeGenContext* ctx) {
    return ctx->module->instruction_count;
}

/**
 * @brief 创建跳转标签
 */
JumpLabel* jump_label_create(int32_t instruction_index) {
    JumpLabel* label = (JumpLabel*)mmgr_alloc(sizeof(JumpLabel));
    if (!label) return NULL;
    
    label->instruction_index = instruction_index;
    label->next = NULL;
    
    return label;
}

/**
 * @brief 释放跳转标签列表
 */
void jump_label_free_list(JumpLabel* labels) {
    while (labels) {
        JumpLabel* next = labels->next;
        mmgr_free(labels);
        labels = next;
    }
}

/**
 * @brief 回填跳转标签列表
 */
void codegen_patch_jump_list(CodeGenContext* ctx, JumpLabel* labels, uint16_t target_address) {
    while (labels) {
        codegen_patch_jump(ctx, labels->instruction_index, target_address);
        labels = labels->next;
    }
}

/**
 * @brief 从AST生成字节码
 */
ErrorCode codegen_generate(CodeGenContext* ctx, ASTNode* program) {
    if (!ctx || !program) {
        return ERR_RUNTIME;
    }
    
    if (program->type != AST_PROGRAM) {
        snprintf(ctx->error_msg, sizeof(ctx->error_msg), 
                "Expected program node, got %d", program->type);
        ctx->error_code = ERR_RUNTIME;
        return ERR_RUNTIME;
    }
    
    return generate_program(ctx, program);
}

/**
 * @brief 生成程序节点
 */
static ErrorCode generate_program(CodeGenContext* ctx, ASTNode* program) {
    ErrorCode err;
    
    // 设置模块的全局变量计数（typecheck已定义全局变量，symtbl保存了计数）
    ctx->module->global_count = ctx->symtbl->global_var_count;
    
    // 设置入口点为0（从头开始执行）
    ctx->module->entry_point = 0;
    
    // 1. 生成全局变量声明（初始化代码）
    if (program->data.program.var_decls) {
        err = generate_var_decls(ctx, program->data.program.var_decls);
        if (err != OK) return err;
    }
    
    // 2. 发射跳转指令跳过函数定义到主程序体
    int32_t jump_to_main = codegen_emit(ctx, OP_JMP, 0);
    
    // 3. 生成函数定义
    ASTNode* func = program->data.program.functions;
    while (func) {
        err = codegen_function(ctx, func);
        if (err != OK) return err;
        func = func->next;
    }
    
    // 4. 回填跳转目标到主程序体起始位置
    uint32_t main_start = codegen_current_position(ctx);
    codegen_patch_jump(ctx, jump_to_main, main_start);
    
    // 5. 生成主程序体
    if (program->data.program.body) {
        err = generate_block(ctx, program->data.program.body, NULL);
        if (err != OK) return err;
    }
    
    // 主程序结束，发射HALT
    codegen_emit(ctx, OP_HALT, 0);
    
    return OK;
}

/**
 * @brief 生成变量声明
 */
static ErrorCode generate_var_decls(CodeGenContext* ctx, ASTNode* var_decls) {
    ASTNode* decl = var_decls;
    
    while (decl) {
        if (decl->type == AST_VAR_DECL) {
            // 如果有初始化表达式，生成代码
            if (decl->data.var_decl.initializer) {
                ErrorCode err = codegen_expr(ctx, decl->data.var_decl.initializer);
                if (err != OK) return err;
                
                // 查找符号
                Symbol* sym = symtbl_lookup(ctx->symtbl, decl->data.var_decl.name);
                if (!sym) {
                    snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                            "Undefined variable: %s", decl->data.var_decl.name);
                    ctx->error_code = ERR_NAME;
                    return ERR_NAME;
                }
                
                // 生成STORE指令：使用正确的索引/偏移
                uint8_t flags = sym->is_global ? 0x01 : 0x00;
                uint16_t addr = sym->is_global ? sym->index : sym->offset;
                codegen_emit_with_flags(ctx, OP_STORE, flags, addr);
            }
            
            /* 变量计数由调用者（codegen_function 或程序级别）负责
             * 这里仅负责生成初始化代码（如果有）。
             */
        }
        
        decl = decl->next;
    }
    
    return OK;
}

/**
 * @brief 生成函数定义
 */
ErrorCode codegen_function(CodeGenContext* ctx, ASTNode* func_decl) {
    if (!func_decl || func_decl->type != AST_FUNCTION_DECL) {
        return ERR_RUNTIME;
    }
    
    // 查找函数符号
    Symbol* func_sym = symtbl_lookup(ctx->symtbl, func_decl->data.function_decl.name);
    if (!func_sym) {
        snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                "Undefined function: %s", func_decl->data.function_decl.name);
        ctx->error_code = ERR_NAME;
        return ERR_NAME;
    }
    
    // 设置当前函数上下文
    ctx->current_function = func_sym;
    ctx->local_var_count = 0;
    
    // 记录函数入口地址
    uint32_t entry_address = codegen_current_position(ctx);
    
    // 进入函数作用域
    symtbl_enter_scope(ctx->symtbl);
    
    // 重置局部变量偏移（参数和局部变量一起计数）
    symtbl_reset_local_offset(ctx->symtbl);
    
    // 注册参数到符号表并计数
    int32_t param_count = 0;
    ASTNode* param = func_decl->data.function_decl.params;
    while (param) {
        if (param->type == AST_VAR_DECL) {
            const char* param_name = param->data.var_decl.name;
            TypeInfo* param_type = param->data.var_decl.type;
            // 参数作为局部变量，偏移量从0开始
            if (!symtbl_define_parameter(ctx->symtbl, param_name, param_type)) {
                snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                        "Cannot define parameter: %s", param_name);
                ctx->error_code = ERR_NAME;
                symtbl_leave_scope(ctx->symtbl);
                return ERR_NAME;
            }
            param_count++;
            ctx->local_var_count++;
        }
        param = param->next;
    }
    
    // 注册局部变量到符号表（在生成初始化代码之前）
    ASTNode* decl = func_decl->data.function_decl.declarations;
    while (decl) {
        if (decl->type == AST_VAR_DECL) {
            const char* var_name = decl->data.var_decl.name;
            TypeInfo* var_type = decl->data.var_decl.type;
            bool is_const = decl->data.var_decl.is_const;
            
            if (!symtbl_define_variable(ctx->symtbl, var_name, var_type, is_const)) {
                snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                        "Cannot define local variable: %s", var_name);
                ctx->error_code = ERR_NAME;
                symtbl_leave_scope(ctx->symtbl);
                return ERR_NAME;
            }
            ctx->local_var_count++;
        }
        decl = decl->next;
    }
    
    // 生成局部变量初始化代码
    if (func_decl->data.function_decl.declarations) {
        ErrorCode err = generate_var_decls(ctx, func_decl->data.function_decl.declarations);
        if (err != OK) {
            symtbl_leave_scope(ctx->symtbl);
            return err;
        }
    }
    
    // 生成函数体
    if (func_decl->data.function_decl.body) {
        ErrorCode err = generate_block(ctx, func_decl->data.function_decl.body, NULL);
        if (err != OK) {
            symtbl_leave_scope(ctx->symtbl);
            return err;
        }
    }
    
    // 函数结束，如果没有显式return，添加隐式return
    codegen_emit(ctx, OP_RET, 0);
    
    // 离开函数作用域
    symtbl_leave_scope(ctx->symtbl);
    
    // 添加函数到函数表
    // 提取返回类型
    DataType return_type = TYPE_VOID;
    if (func_decl->data.function_decl.return_type) {
        return_type = func_decl->data.function_decl.return_type->base_type;
    }
    
    // 提取参数类型
    DataType* param_types = NULL;
    if (param_count > 0 && func_decl->data.function_decl.return_type && 
        func_decl->data.function_decl.return_type->base_type == TYPE_FUNCTION) {
        TypeInfo* func_type = func_decl->data.function_decl.return_type;
        if (func_type->func_info.param_types) {
            param_types = (DataType*)mmgr_alloc(sizeof(DataType) * param_count);
            for (int32_t i = 0; i < param_count; i++) {
                param_types[i] = func_type->func_info.param_types[i]->base_type;
            }
        }
    }
    
    bytecode_add_function(ctx->module, func_decl->data.function_decl.name, 
                         entry_address, param_count, ctx->local_var_count, return_type, param_types);
    
    // 释放参数类型数组
    if (param_types) {
        mmgr_free(param_types);
    }
    
    // 清除当前函数上下文
    ctx->current_function = NULL;
    ctx->local_var_count = 0;
    
    return OK;
}

/**
 * @brief 生成表达式代码
 */
ErrorCode codegen_expr(CodeGenContext* ctx, ASTNode* expr) {
    if (!expr) return ERR_RUNTIME;
    
    switch (expr->type) {
        case AST_LITERAL:
            return generate_literal(ctx, expr);
            
        case AST_IDENTIFIER:
            return generate_identifier(ctx, expr);
            
        case AST_BINARY_OP:
            return generate_binary_op(ctx, expr);
            
        case AST_UNARY_OP:
            return generate_unary_op(ctx, expr);
            
        case AST_FUNCTION_CALL:
            return generate_function_call(ctx, expr);
            
        case AST_ARRAY_ACCESS:
            return generate_array_access(ctx, expr);
            
        default:
            snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                    "Unsupported expression type: %d", expr->type);
            ctx->error_code = ERR_RUNTIME;
            return ERR_RUNTIME;
    }
}

/**
 * @brief 生成字面量
 */
static ErrorCode generate_literal(CodeGenContext* ctx, ASTNode* node) {
    int32_t const_index = codegen_add_constant(ctx, node->data.literal.value);
    if (const_index < 0) {
        ctx->error_code = ERR_OUT_OF_MEMORY;
        return ERR_OUT_OF_MEMORY;
    }
    
    codegen_emit(ctx, OP_PUSH, (uint16_t)const_index);
    return OK;
}

/**
 * @brief 生成标识符（变量读取）
 */
static ErrorCode generate_identifier(CodeGenContext* ctx, ASTNode* node) {
    Symbol* sym = symtbl_lookup(ctx->symtbl, node->data.identifier.name);
    if (!sym) {
        snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                "Undefined variable: %s", node->data.identifier.name);
        ctx->error_code = ERR_NAME;
        return ERR_NAME;
    }
    
    // LOAD指令：flags指示全局/局部，使用正确的索引/偏移
    uint8_t flags = sym->is_global ? 0x01 : 0x00;
    uint16_t addr = sym->is_global ? sym->index : sym->offset;
    codegen_emit_with_flags(ctx, OP_LOAD, flags, addr);
    
    return OK;
}

/**
 * @brief 生成二元运算
 */
static ErrorCode generate_binary_op(CodeGenContext* ctx, ASTNode* node) {
    ErrorCode err;
    
    // 左操作数
    err = codegen_expr(ctx, node->data.binary_op.left);
    if (err != OK) return err;
    
    // 右操作数
    err = codegen_expr(ctx, node->data.binary_op.right);
    if (err != OK) return err;
    
    // 运算符
    Opcode opcode;
    switch (node->data.binary_op.op) {
        case BINOP_ADD: opcode = OP_ADD; break;
        case BINOP_SUB: opcode = OP_SUB; break;
        case BINOP_MUL: opcode = OP_MUL; break;
        case BINOP_DIV: opcode = OP_DIV; break;
        case BINOP_MOD: opcode = OP_MOD; break;
        case BINOP_EQ:  opcode = OP_EQ;  break;
        case BINOP_NE:  opcode = OP_NE;  break;
        case BINOP_LT:  opcode = OP_LT;  break;
        case BINOP_LE:  opcode = OP_LE;  break;
        case BINOP_GT:  opcode = OP_GT;  break;
        case BINOP_GE:  opcode = OP_GE;  break;
        case BINOP_AND: opcode = OP_AND; break;
        case BINOP_OR:  opcode = OP_OR;  break;
        default:
            snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                    "Unsupported binary operator: %d", node->data.binary_op.op);
            ctx->error_code = ERR_RUNTIME;
            return ERR_RUNTIME;
    }
    
    codegen_emit(ctx, opcode, 0);
    return OK;
}

/**
 * @brief 生成一元运算
 */
static ErrorCode generate_unary_op(CodeGenContext* ctx, ASTNode* node) {
    ErrorCode err = codegen_expr(ctx, node->data.unary_op.operand);
    if (err != OK) return err;
    
    Opcode opcode;
    switch (node->data.unary_op.op) {
        case UNOP_NEG: opcode = OP_NEG; break;
        case UNOP_NOT: opcode = OP_NOT; break;
        default:
            snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                    "Unsupported unary operator: %d", node->data.unary_op.op);
            ctx->error_code = ERR_RUNTIME;
            return ERR_RUNTIME;
    }
    
    codegen_emit(ctx, opcode, 0);
    return OK;
}

/**
 * @brief 生成函数调用
 */
static ErrorCode generate_function_call(CodeGenContext* ctx, ASTNode* node) {
    ErrorCode err;
    
    // 压入参数（从左到右）
    for (int i = 0; i < node->data.function_call.arg_count; i++) {
        err = codegen_expr(ctx, node->data.function_call.arguments[i]);
        if (err != OK) return err;
    }
    
    // 首先检查符号表，判断是内部函数还是外部函数
    Symbol* sym = symtbl_lookup(ctx->symtbl, node->data.function_call.name);
    
    // 检查是否是内部定义的函数（有地址）
    FunctionEntry* func = bytecode_find_function(ctx->module, node->data.function_call.name);
    if (func && func->address != 0) {
        // 这是一个内部函数（有实际地址）
        uint32_t func_index = func - ctx->module->functions;
        codegen_emit(ctx, OP_CALL, (uint16_t)func_index);
        return OK;
    }
    
    // 这是外部函数或未定义函数
    if (sym && sym->kind == SYM_FUNCTION) {
        // 确保函数在函数表中（用于 CALL_EXT 查找）
        if (!func) {
            // 添加到函数表
            uint32_t func_idx = bytecode_add_function(ctx->module,
                                                      node->data.function_call.name,
                                                      0,  // 外部函数地址为0
                                                      sym->param_count,
                                                      0,  // 无局部变量
                                                      TYPE_VOID,  // 返回类型
                                                      NULL);  // 无参数类型
            if (func_idx == (uint32_t)-1) {
                ctx->error_code = ERR_RUNTIME;
                snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                        "Failed to add external function: %s", node->data.function_call.name);
                return ERR_RUNTIME;
            }
            func = &ctx->module->functions[func_idx];
        }
        
        // 发射CALL_EXT指令
        uint32_t func_index = func - ctx->module->functions;
        Instruction instr;
        instr.opcode = OP_CALL_EXT;
        instr.operand = func_index;  // 函数索引
        instr.flags = node->data.function_call.arg_count;
        
        if (ctx->module->instruction_count >= ctx->module->instruction_capacity) {
            ctx->error_code = ERR_RUNTIME;
            return ERR_RUNTIME;
        }
        ctx->module->instructions[ctx->module->instruction_count++] = instr;
        
        return OK;
    }
    
    // 函数未定义
    snprintf(ctx->error_msg, sizeof(ctx->error_msg),
            "Undefined function: %s", node->data.function_call.name);
    ctx->error_code = ERR_NAME;
    return ERR_NAME;
}

/**
 * @brief 生成数组访问
 */
static ErrorCode generate_array_access(CodeGenContext* ctx, ASTNode* node) {
    ErrorCode err;
    
    // 数组访问：array[index]
    // 策略：将其转换为普通变量访问，索引必须是编译时常量
    
    // 检查数组基址是否是标识符
    if (node->data.array_access.array->type != AST_IDENTIFIER) {
        snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                "Array base must be an identifier");
        ctx->error_code = ERR_RUNTIME;
        return ERR_RUNTIME;
    }
    
    // 获取数组符号
    const char* array_name = node->data.array_access.array->data.identifier.name;
    Symbol* array_sym = symtbl_lookup(ctx->symtbl, array_name);
    if (!array_sym) {
        snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                "Undefined array: %s", array_name);
        ctx->error_code = ERR_NAME;
        return ERR_NAME;
    }
    
    // 检查索引是否是字面量（编译时常量）
    if (node->data.array_access.index->type != AST_LITERAL) {
        // 运行时数组索引 - 使用 OP_LOAD_INDEXED
        
        // 计算索引表达式并压入栈
        err = codegen_expr(ctx, node->data.array_access.index);
        if (err != OK) return err;
        
        // 发出 OP_LOAD_INDEXED 指令
        // operand = 数组基地址，flags 标识全局/局部
        uint8_t flags = array_sym->is_global ? FLAG_GLOBAL : 0x00;
        uint16_t base_offset = array_sym->is_global ? array_sym->index : array_sym->offset;
        codegen_emit_with_flags(ctx, OP_LOAD_INDEXED, flags, base_offset);
        
        return OK;
    }
    
    // 编译时常量索引
    // 从 literal value 中获取整数值
    Value idx_val = node->data.array_access.index->data.literal.value;
    if (idx_val.type != TYPE_INT) {
        snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                "Array index must be an integer");
        ctx->error_code = ERR_TYPE;
        return ERR_TYPE;
    }
    int32_t index = idx_val.int_val;
    
    // TODO: 检查数组边界（需要类型系统支持）
    
    // 计算实际变量偏移：base_offset + index
    uint16_t offset = array_sym->offset + index;
    
    // 生成LOAD指令
    // 使用 is_global 判断全局/局部
    codegen_emit(ctx, OP_LOAD, offset);
    if (array_sym->is_global) {
        // 设置全局标志
        ctx->module->instructions[ctx->module->instruction_count - 1].flags = FLAG_GLOBAL;
    }
    
    return OK;
}

/**
 * @brief 生成语句代码
 */
ErrorCode codegen_stmt(CodeGenContext* ctx, ASTNode* stmt, LoopContext* loop_ctx) {
    if (!stmt) return OK;
    
    switch (stmt->type) {
        case AST_ASSIGN:
            return generate_assign(ctx, stmt);
            
        case AST_IF:
            return generate_if(ctx, stmt, loop_ctx);
            
        case AST_WHILE:
            return generate_while(ctx, stmt, loop_ctx);
            
        case AST_FOR:
            return generate_for(ctx, stmt, loop_ctx);
            
        case AST_RETURN:
            return generate_return(ctx, stmt);
            
        case AST_BLOCK:
            // AST_BLOCK节点本身可能就是语句链表，或者有特定字段
            // 根据实际AST结构，这里假设body或statements字段
            return generate_block(ctx, stmt, loop_ctx);
            
        default:
            // 可能是表达式语句
            return codegen_expr(ctx, stmt);
    }
}

/**
 * @brief 生成赋值语句
 */
static ErrorCode generate_assign(CodeGenContext* ctx, ASTNode* node) {
    ErrorCode err;
    
    // 计算右值
    err = codegen_expr(ctx, node->data.assign.value);
    if (err != OK) return err;
    
    // 获取左值（变量或数组元素）
    ASTNode* target = node->data.assign.target;
    
    if (target->type == AST_IDENTIFIER) {
        // 简单变量赋值
        Symbol* sym = symtbl_lookup(ctx->symtbl, target->data.identifier.name);
        if (!sym) {
            snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                    "Undefined variable: %s", target->data.identifier.name);
            ctx->error_code = ERR_NAME;
            return ERR_NAME;
        }
        
        // 生成STORE指令
        uint8_t flags = sym->is_global ? 0x01 : 0x00;
        uint16_t addr = sym->is_global ? sym->index : sym->offset;
        codegen_emit_with_flags(ctx, OP_STORE, flags, addr);
        
        return OK;
        
    } else if (target->type == AST_ARRAY_ACCESS) {
        // 数组元素赋值
        
        // 检查数组基址是否是标识符
        if (target->data.array_access.array->type != AST_IDENTIFIER) {
            snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                    "Array base must be an identifier");
            ctx->error_code = ERR_RUNTIME;
            return ERR_RUNTIME;
        }
        
        // 获取数组符号
        const char* array_name = target->data.array_access.array->data.identifier.name;
        Symbol* array_sym = symtbl_lookup(ctx->symtbl, array_name);
        if (!array_sym) {
            snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                    "Undefined array: %s", array_name);
            ctx->error_code = ERR_NAME;
            return ERR_NAME;
        }
        
        // 检查索引是否是字面量（编译时常量）
        if (target->data.array_access.index->type == AST_LITERAL) {
            // 编译时常量索引
            Value idx_val = target->data.array_access.index->data.literal.value;
            if (idx_val.type != TYPE_INT) {
                snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                        "Array index must be an integer");
                ctx->error_code = ERR_TYPE;
                return ERR_TYPE;
            }
            int32_t index = idx_val.int_val;
            
            // TODO: 检查数组边界（需要类型系统支持）
            
            // 计算实际变量偏移：base_offset + index
            uint16_t offset = (array_sym->is_global ? array_sym->index : array_sym->offset) + index;
            
            // 生成STORE指令
            uint8_t flags = array_sym->is_global ? 0x01 : 0x00;
            codegen_emit_with_flags(ctx, OP_STORE, flags, offset);
            
            return OK;
        } else {
            // 运行时索引 - 使用 OP_STORE_INDEXED
            // 栈布局：[... value, index]，其中 value 已经在栈上（之前计算的右值）
            // 现在需要计算索引并将其也压入栈
            err = codegen_expr(ctx, target->data.array_access.index);
            if (err != OK) return err;
            
            // 发出 OP_STORE_INDEXED 指令
            // operand = 数组基地址，flags 标识全局/局部
            uint8_t flags = array_sym->is_global ? FLAG_GLOBAL : 0x00;
            uint16_t base_offset = array_sym->is_global ? array_sym->index : array_sym->offset;
            codegen_emit_with_flags(ctx, OP_STORE_INDEXED, flags, base_offset);
            
            return OK;
        }
        
    } else {
        snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                "Invalid assignment target");
        ctx->error_code = ERR_RUNTIME;
        return ERR_RUNTIME;
    }
}

/**
 * @brief 生成if语句
 */
static ErrorCode generate_if(CodeGenContext* ctx, ASTNode* node, LoopContext* loop_ctx) {
    ErrorCode err;
    
    // 计算条件
    err = codegen_expr(ctx, node->data.if_stmt.condition);
    if (err != OK) return err;
    
    // JZ到else分支或结束
    int32_t jz_index = codegen_emit(ctx, OP_JZ, 0);
    
    // then分支
    err = generate_block(ctx, node->data.if_stmt.then_branch, loop_ctx);
    if (err != OK) return err;
    
    if (node->data.if_stmt.else_branch) {
        // 跳过else分支
        int32_t jmp_index = codegen_emit(ctx, OP_JMP, 0);
        
        // 回填JZ到else分支
        int32_t else_addr = codegen_current_position(ctx);
        codegen_patch_jump(ctx, jz_index, else_addr);
        
        // else分支
        err = generate_block(ctx, node->data.if_stmt.else_branch, loop_ctx);
        if (err != OK) return err;
        
        // 回填JMP到结束
        int32_t end_addr = codegen_current_position(ctx);
        codegen_patch_jump(ctx, jmp_index, end_addr);
    } else {
        // 回填JZ到结束
        int32_t end_addr = codegen_current_position(ctx);
        codegen_patch_jump(ctx, jz_index, end_addr);
    }
    
    return OK;
}

/**
 * @brief 生成while循环
 */
static ErrorCode generate_while(CodeGenContext* ctx, ASTNode* node, LoopContext* parent_loop) {
    ErrorCode err;
    
    // 创建循环上下文
    LoopContext loop_ctx;
    loop_ctx.break_labels = NULL;
    loop_ctx.continue_labels = NULL;
    loop_ctx.parent = parent_loop;
    
    // 循环开始位置
    int32_t loop_start = codegen_current_position(ctx);
    
    // 计算条件
    err = codegen_expr(ctx, node->data.while_stmt.condition);
    if (err != OK) return err;
    
    // JZ到循环结束
    int32_t jz_index = codegen_emit(ctx, OP_JZ, 0);
    
    // 循环体
    err = generate_block(ctx, node->data.while_stmt.body, &loop_ctx);
    if (err != OK) return err;
    
    // 跳回循环开始
    codegen_emit(ctx, OP_JMP, loop_start);
    
    // 回填JZ和所有break标签到循环结束
    int32_t loop_end = codegen_current_position(ctx);
    codegen_patch_jump(ctx, jz_index, loop_end);
    codegen_patch_jump_list(ctx, loop_ctx.break_labels, loop_end);
    
    // 回填所有continue标签到循环开始
    codegen_patch_jump_list(ctx, loop_ctx.continue_labels, loop_start);
    
    // 释放标签
    jump_label_free_list(loop_ctx.break_labels);
    jump_label_free_list(loop_ctx.continue_labels);
    
    return OK;
}

/**
 * @brief 生成for循环
 */
static ErrorCode generate_for(CodeGenContext* ctx, ASTNode* node, LoopContext* parent_loop) {
    ErrorCode err;
    
    // 创建循环上下文
    LoopContext loop_ctx;
    loop_ctx.break_labels = NULL;
    loop_ctx.continue_labels = NULL;
    loop_ctx.parent = parent_loop;
    
    // 查找循环变量
    Symbol* loop_var = symtbl_lookup(ctx->symtbl, node->data.for_stmt.variable);
    if (!loop_var) {
        snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                "Undefined loop variable: %s", node->data.for_stmt.variable);
        ctx->error_code = ERR_NAME;
        return ERR_NAME;
    }
    
    // 初始化：var := start
    err = codegen_expr(ctx, node->data.for_stmt.start);
    if (err != OK) return err;
    
    uint8_t flags = loop_var->is_global ? 0x01 : 0x00;
    uint16_t addr = loop_var->is_global ? loop_var->index : loop_var->offset;
    codegen_emit_with_flags(ctx, OP_STORE, flags, addr);
    
    // 循环开始
    int32_t loop_start = codegen_current_position(ctx);
    
    // 加载循环变量
    codegen_emit_with_flags(ctx, OP_LOAD, flags, addr);
    
    // 加载结束值
    err = codegen_expr(ctx, node->data.for_stmt.end);
    if (err != OK) return err;
    
    // 比较：var <= end
    codegen_emit(ctx, OP_LE, 0);
    
    // 如果false，跳出循环
    int32_t jz_index = codegen_emit(ctx, OP_JZ, 0);
    
    // 循环体
    err = generate_block(ctx, node->data.for_stmt.body, &loop_ctx);
    if (err != OK) return err;
    
    // 更新循环变量：var := var + step
    codegen_emit_with_flags(ctx, OP_LOAD, flags, addr);
    
    if (node->data.for_stmt.step) {
        err = codegen_expr(ctx, node->data.for_stmt.step);
        if (err != OK) return err;
    } else {
        // 默认步长为1
        Value one;
        one.type = TYPE_INT;
        one.int_val = 1;
        int32_t const_idx = codegen_add_constant(ctx, one);
        codegen_emit(ctx, OP_PUSH, const_idx);
    }
    
    codegen_emit(ctx, OP_ADD, 0);
    codegen_emit_with_flags(ctx, OP_STORE, flags, addr);
    
    // 跳回循环开始
    codegen_emit(ctx, OP_JMP, loop_start);
    
    // 回填跳转
    int32_t loop_end = codegen_current_position(ctx);
    codegen_patch_jump(ctx, jz_index, loop_end);
    codegen_patch_jump_list(ctx, loop_ctx.break_labels, loop_end);
    codegen_patch_jump_list(ctx, loop_ctx.continue_labels, loop_start);
    
    // 释放标签
    jump_label_free_list(loop_ctx.break_labels);
    jump_label_free_list(loop_ctx.continue_labels);
    
    return OK;
}

/**
 * @brief 生成return语句
 */
static ErrorCode generate_return(CodeGenContext* ctx, ASTNode* node) {
    ErrorCode err;
    
    if (node->data.return_stmt.value) {
        // 计算返回值
        err = codegen_expr(ctx, node->data.return_stmt.value);
        if (err != OK) return err;
    }
    
    codegen_emit(ctx, OP_RET, 0);
    return OK;
}

/**
 * @brief 生成语句块
 */
static ErrorCode generate_block(CodeGenContext* ctx, ASTNode* stmts, LoopContext* loop_ctx) {
    ASTNode* stmt = stmts;
    
    while (stmt) {
        ErrorCode err = codegen_stmt(ctx, stmt, loop_ctx);
        if (err != OK) return err;
        
        stmt = stmt->next;
    }
    
    return OK;
}
