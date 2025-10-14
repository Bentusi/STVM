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
    
    // 1. 生成全局变量声明
    if (program->data.program.var_decls) {
        err = generate_var_decls(ctx, program->data.program.var_decls);
        if (err != OK) return err;
    }
    
    // 2. 生成函数定义
    ASTNode* func = program->data.program.functions;
    while (func) {
        err = codegen_function(ctx, func);
        if (err != OK) return err;
        func = func->next;
    }
    
    // 3. 生成主程序体（作为隐式的main函数）
    if (program->data.program.body) {
        // 设置入口点
        ctx->module->entry_point = codegen_current_position(ctx);
        
        err = generate_block(ctx, program->data.program.body, NULL);
        if (err != OK) return err;
        
        // 主程序结束，发射HALT
        codegen_emit(ctx, OP_HALT, 0);
    }
    
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
                
                // 生成STORE指令
                uint8_t flags = sym->is_global ? 0x01 : 0x00;
                codegen_emit_with_flags(ctx, OP_STORE, flags, sym->offset);
            }
            
            if (ctx->current_function) {
                ctx->local_var_count++;
            } else {
                ctx->module->global_count++;
            }
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
    
    // 计算参数数量
    int32_t param_count = 0;
    ASTNode* param = func_decl->data.function_decl.params;
    while (param) {
        param_count++;
        param = param->next;
    }
    
    // 生成局部变量声明
    if (func_decl->data.function_decl.declarations) {
        ErrorCode err = generate_var_decls(ctx, func_decl->data.function_decl.declarations);
        if (err != OK) return err;
    }
    
    // 生成函数体
    if (func_decl->data.function_decl.body) {
        ErrorCode err = generate_block(ctx, func_decl->data.function_decl.body, NULL);
        if (err != OK) return err;
    }
    
    // 函数结束，如果没有显式return，添加隐式return
    codegen_emit(ctx, OP_RET, 0);
    
    // 添加函数到函数表
    // bytecode_add_function需要return_type参数，从TypeInfo获取
    DataType return_type = TYPE_VOID;
    if (func_decl->data.function_decl.return_type) {
        return_type = func_decl->data.function_decl.return_type->base_type;
    }
    bytecode_add_function(ctx->module, func_decl->data.function_decl.name, 
                         entry_address, param_count, ctx->local_var_count, return_type);
    
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
    
    // LOAD指令：flags指示全局/局部
    uint8_t flags = sym->is_global ? 0x01 : 0x00;
    codegen_emit_with_flags(ctx, OP_LOAD, flags, sym->offset);
    
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
    
    // 查找函数
    FunctionEntry* func = bytecode_find_function(ctx->module, node->data.function_call.name);
    if (!func) {
        // 可能是外部函数，使用CALL_EXT
        // TODO: 实现外部函数表
        snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                "Undefined function: %s", node->data.function_call.name);
        ctx->error_code = ERR_NAME;
        return ERR_NAME;
    }
    
    // 发射CALL指令，operand是函数地址
    codegen_emit(ctx, OP_CALL, (uint16_t)func->address);
    
    return OK;
}

/**
 * @brief 生成数组访问
 */
static ErrorCode generate_array_access(CodeGenContext* ctx, ASTNode* node) {
    ErrorCode err;
    
    // 数组基址
    err = codegen_expr(ctx, node->data.array_access.array);
    if (err != OK) return err;
    
    // 索引
    err = codegen_expr(ctx, node->data.array_access.index);
    if (err != OK) return err;
    
    // TODO: 发射数组访问指令（当前字节码系统未定义）
    // 暂时返回错误
    snprintf(ctx->error_msg, sizeof(ctx->error_msg),
            "Array access not yet implemented");
    ctx->error_code = ERR_RUNTIME;
    return ERR_RUNTIME;
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
    
    // 获取左值（变量）
    ASTNode* target = node->data.assign.target;
    if (target->type != AST_IDENTIFIER) {
        snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                "Assignment target must be identifier");
        ctx->error_code = ERR_RUNTIME;
        return ERR_RUNTIME;
    }
    
    Symbol* sym = symtbl_lookup(ctx->symtbl, target->data.identifier.name);
    if (!sym) {
        snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                "Undefined variable: %s", target->data.identifier.name);
        ctx->error_code = ERR_NAME;
        return ERR_NAME;
    }
    
    // 生成STORE指令
    uint8_t flags = sym->is_global ? 0x01 : 0x00;
    codegen_emit_with_flags(ctx, OP_STORE, flags, sym->offset);
    
    return OK;
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
    codegen_emit_with_flags(ctx, OP_STORE, flags, loop_var->offset);
    
    // 循环开始
    int32_t loop_start = codegen_current_position(ctx);
    
    // 加载循环变量
    codegen_emit_with_flags(ctx, OP_LOAD, flags, loop_var->offset);
    
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
    codegen_emit_with_flags(ctx, OP_LOAD, flags, loop_var->offset);
    
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
    codegen_emit_with_flags(ctx, OP_STORE, flags, loop_var->offset);
    
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
