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
static ErrorCode generate_member_access(CodeGenContext* ctx, ASTNode* node);
static ErrorCode generate_assign(CodeGenContext* ctx, ASTNode* node);
static ErrorCode generate_if(CodeGenContext* ctx, ASTNode* node, LoopContext* loop_ctx);
static ErrorCode generate_while(CodeGenContext* ctx, ASTNode* node, LoopContext* loop_ctx);
static ErrorCode generate_for(CodeGenContext* ctx, ASTNode* node, LoopContext* loop_ctx);
static ErrorCode generate_repeat(CodeGenContext* ctx, ASTNode* node, LoopContext* loop_ctx);
static ErrorCode generate_case(CodeGenContext* ctx, ASTNode* node, LoopContext* loop_ctx);
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
 * @brief 添加字符串常量到常量池
 */
static int32_t codegen_add_string_constant(CodeGenContext* ctx, const char* str) {
    if (!ctx || !str) return -1;
    return bytecode_add_string_constant(ctx->module, str);
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
    
    // 填充全局变量元数据（用于热加载）
    if (ctx->module->global_count > 0) {
        ctx->module->globals_info = (GlobalEntry*)mmgr_calloc(sizeof(GlobalEntry) * ctx->module->global_count);
        if (!ctx->module->globals_info) {
            ctx->error_code = ERR_OUT_OF_MEMORY;
            return ERR_OUT_OF_MEMORY;
        }
        
        // 遍历符号表填充元数据
        Scope* scope = ctx->symtbl->global_scope;
        if (scope && scope->symbols) {
            for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
                Symbol* sym = scope->symbols[i];
                while (sym) {
                    if (sym->kind == SYM_VARIABLE && sym->is_global) {
                        int32_t idx = sym->index;
                        if (idx >= 0 && idx < (int32_t)ctx->module->global_count) {
                            ctx->module->globals_info[idx].name = mmgr_strdup(sym->name);
                            ctx->module->globals_info[idx].type = sym->type->base_type;
                            ctx->module->globals_info[idx].index = idx;
                        }
                    }
                    sym = sym->next;
                }
            }
        }
    }
    
    // 设置入口点为0（从头开始执行）
    ctx->module->entry_point = 0;
    
    // 1. 生成全局变量声明（初始化代码）
    if (program->data.program.var_decls) {
        err = generate_var_decls(ctx, program->data.program.var_decls);
        if (err != OK) return err;
    }
    
    // 1.5. 生成函数内定义的静态变量的初始化代码
    // 注意：由于库模块的初始化代码可能不被执行，
    // 我们将静态变量初始化移到函数内部处理（见 generate_var_decls）
    // 这里保留代码框架但不生成任何指令
    /*
    ASTNode* func = program->data.program.functions;
    while (func) {
        if (func->type == AST_FUNCTION_DECL && func->data.function_decl.declarations) {
            const char* func_name = func->data.function_decl.name;
            ASTNode* decl = func->data.function_decl.declarations;
            while (decl) {
                if (decl->type == AST_VAR_DECL && decl->data.var_decl.is_global && 
                    decl->data.var_decl.initializer) {
                    // 静态变量初始化已移到函数内部
                }
                decl = decl->next;
            }
        }
        func = func->next;
    }
    */
    
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
            // 处理所有变量声明，包括函数内的静态变量
            // 注意：函数内的静态变量（is_global=true）也在这里初始化
            // 以确保库模块的静态变量能被正确初始化
            
            // 如果是函数内的静态变量，需要使用完全限定名
            const char* var_name = decl->data.var_decl.name;
            Symbol* sym = NULL;
            
            if (ctx->current_function && decl->data.var_decl.is_global) {
                // 静态变量：使用完全限定名查找
                const char* func_name = ctx->current_function->name;
                size_t qualified_len = strlen(func_name) + strlen(var_name) + 2;
                char* qualified_name = (char*)mmgr_alloc(qualified_len);
                if (!qualified_name) {
                    ctx->error_code = ERR_RUNTIME;
                    return ERR_RUNTIME;
                }
                snprintf(qualified_name, qualified_len, "%s.%s", func_name, var_name);
                sym = symtbl_lookup(ctx->symtbl, qualified_name);
                mmgr_free(qualified_name);
            } else {
                // 普通变量：直接查找
                sym = symtbl_lookup(ctx->symtbl, var_name);
            }
            if (!sym) {
                snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                        "Undefined variable: %s", decl->data.var_decl.name);
                ctx->error_code = ERR_NAME;
                return ERR_NAME;
            }
            
            // 如果有初始化表达式，生成代码
            if (decl->data.var_decl.initializer) {
                ErrorCode err = codegen_expr(ctx, decl->data.var_decl.initializer);
                if (err != OK) return err;
                
                // 生成STORE指令：使用正确的索引/偏移
                uint8_t flags = sym->is_global ? 0x01 : 0x00;
                uint16_t addr = sym->is_global ? sym->index : sym->offset;
                codegen_emit_with_flags(ctx, OP_STORE, flags, addr);
            } else if (sym->type && sym->type->base_type != TYPE_ARRAY && sym->type->base_type != TYPE_FUNCTION) {
                // 没有初始化表达式，且不是数组或函数类型，生成默认值
                // 数组和函数不需要显式初始化
                
                // 仅对局部变量生成默认初始化代码
                // 全局变量由VM初始化为0，且在热更新时需要保留值，不应重置
                if (!sym->is_global) {
                    Value default_val = {.type = TYPE_VOID};
                    default_val.type = sym->type->base_type;
                    switch (default_val.type) {
                        case TYPE_INT:
                            default_val.int_val = 0;
                            break;
                        case TYPE_REAL:
                            default_val.real_val = 0.0;
                            break;
                        case TYPE_BOOL:
                            default_val.bool_val = false;
                            break;
                        case TYPE_STRING:
                            default_val.string_val = "";
                            break;
                        default:
                            // 跳过其他类型（TYPE_VOID等）
                            decl = decl->next;
                            continue;
                    }
                    
                    int32_t const_index = codegen_add_constant(ctx, default_val);
                    if (const_index < 0) {
                        ctx->error_code = ERR_OUT_OF_MEMORY;
                        return ERR_OUT_OF_MEMORY;
                    }
                    codegen_emit(ctx, OP_PUSH, (uint16_t)const_index);
                    
                    // 生成STORE指令：使用正确的索引/偏移
                    uint8_t flags = sym->is_global ? 0x01 : 0x00;
                    uint16_t addr = sym->is_global ? sym->index : sym->offset;
                    codegen_emit_with_flags(ctx, OP_STORE, flags, addr);
                }
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
    
    // 注册函数名作为返回值变量（IEC 61131-3 特性）
    // 仅当函数有返回类型时
    const char* func_name = func_decl->data.function_decl.name;
    TypeInfo* return_type = func_sym->type->func_info.return_type;
    if (return_type != NULL) {
        if (!symtbl_define_variable(ctx->symtbl, func_name, return_type, false)) {
            snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                    "Cannot define return value variable: %s", func_name);
            ctx->error_code = ERR_NAME;
            symtbl_leave_scope(ctx->symtbl);
            return ERR_NAME;
        }
        // 记录返回值变量的 offset
        (void)ctx->symtbl->local_var_offset;  // 返回值变量的 offset，当前未使用
        ctx->local_var_count++;
    }
    
    // 注册局部变量到符号表（在生成初始化代码之前）
    // 跳过静态变量（is_global=true），它们已在类型检查阶段添加到全局符号表
    // 静态变量使用完全限定名存储，但在函数作用域内可通过短名访问
    ASTNode* decl = func_decl->data.function_decl.declarations;
    while (decl) {
        if (decl->type == AST_VAR_DECL && !decl->data.var_decl.is_global) {
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
    DataType ret_type = TYPE_VOID;
    if (func_decl->data.function_decl.return_type) {
        ret_type = func_decl->data.function_decl.return_type->base_type;
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
    
    // 使用符号表的 local_var_offset 作为局部变量总数（已考虑数组大小）
    int32_t total_local_vars = ctx->symtbl->local_var_offset;
    
    bytecode_add_function(ctx->module, func_decl->data.function_decl.name, 
                         entry_address, param_count, total_local_vars, ret_type, param_types);
    
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
            
        case AST_MEMBER_ACCESS:
            return generate_member_access(ctx, expr);
            
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
    const char* var_name = node->data.identifier.name;
    Symbol* sym = NULL;
    
    // 如果当前在函数内，先尝试查找完全限定名（用于静态变量）
    if (ctx->current_function) {
        const char* func_name = ctx->current_function->name;
        size_t qualified_len = strlen(func_name) + strlen(var_name) + 2;
        char* qualified_name = (char*)mmgr_alloc(qualified_len);
        if (qualified_name) {
            snprintf(qualified_name, qualified_len, "%s.%s", func_name, var_name);
            sym = symtbl_lookup(ctx->symtbl, qualified_name);
            
            if (sym && sym->is_static) {
                // 找到静态变量
                mmgr_free(qualified_name);
                
                // 检查是否为外部I/O变量
                if (sym->is_external && sym->io_address) {
                    // 生成 OP_IO_READ 指令
                    int32_t addr_index = codegen_add_string_constant(ctx, sym->io_address);
                    if (addr_index < 0) {
                        ctx->error_code = ERR_OUT_OF_MEMORY;
                        return ERR_OUT_OF_MEMORY;
                    }
                    codegen_emit(ctx, OP_IO_READ, (uint16_t)addr_index);
                    return OK;
                }
                
                uint8_t flags = 0x01;  // 静态变量在全局区
                uint16_t addr = sym->index;
                codegen_emit_with_flags(ctx, OP_LOAD, flags, addr);
                return OK;
            }
            mmgr_free(qualified_name);
        }
    }
    
    // 否则使用常规查找
    sym = symtbl_lookup(ctx->symtbl, var_name);
    if (!sym) {
        snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                "Undefined variable: %s", var_name);
        ctx->error_code = ERR_NAME;
        return ERR_NAME;
    }
    
    // 检查是否为外部I/O变量
    if (sym->is_external && sym->io_address) {
        // 生成 OP_IO_READ 指令
        int32_t addr_index = codegen_add_string_constant(ctx, sym->io_address);
        if (addr_index < 0) {
            ctx->error_code = ERR_OUT_OF_MEMORY;
            return ERR_OUT_OF_MEMORY;
        }
        codegen_emit(ctx, OP_IO_READ, (uint16_t)addr_index);
        return OK;
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
    BinaryOp op = node->data.binary_op.op;
    
    // 对于AND/OR/XOR，根据结果类型选择逻辑或位运算指令
    if (op == BINOP_AND || op == BINOP_OR || op == BINOP_XOR) {
        // 检查resolved_type来决定使用逻辑还是位运算
        if (node->resolved_type && node->resolved_type->base_type == TYPE_INT) {
            // 整数类型：使用位运算
            switch (op) {
                case BINOP_AND: opcode = OP_BIT_AND; break;
                case BINOP_OR:  opcode = OP_BIT_OR;  break;
                case BINOP_XOR: opcode = OP_BIT_XOR; break;
                default: opcode = OP_AND; break;
            }
        } else {
            // 布尔类型：使用逻辑运算
            switch (op) {
                case BINOP_AND: opcode = OP_AND; break;
                case BINOP_OR:  opcode = OP_OR;  break;
                case BINOP_XOR: opcode = OP_XOR; break;
                default: opcode = OP_AND; break;
            }
        }
    } else {
        // 其他运算符
        switch (op) {
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
            case BINOP_BIT_AND: opcode = OP_BIT_AND; break;
            case BINOP_BIT_OR:  opcode = OP_BIT_OR;  break;
            case BINOP_BIT_XOR: opcode = OP_BIT_XOR; break;
            case BINOP_SHL: opcode = OP_SHL; break;
            case BINOP_SHR: opcode = OP_SHR; break;
            default:
                snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                        "Unsupported binary operator: %d", op);
                ctx->error_code = ERR_RUNTIME;
                return ERR_RUNTIME;
        }
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
    UnaryOp op = node->data.unary_op.op;
    
    // 对于NOT，根据结果类型选择逻辑或位运算
    if (op == UNOP_NOT) {
        if (node->resolved_type && node->resolved_type->base_type == TYPE_INT) {
            opcode = OP_BIT_NOT;  // 整数类型：位取反
        } else {
            opcode = OP_NOT;      // 布尔类型：逻辑非
        }
    } else {
        switch (op) {
            case UNOP_NEG: opcode = OP_NEG; break;
            case UNOP_BIT_NOT: opcode = OP_BIT_NOT; break;
            default:
                snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                        "Unsupported unary operator: %d", op);
                ctx->error_code = ERR_RUNTIME;
                return ERR_RUNTIME;
        }
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
    
    // 获取函数名（对于库函数使用完全限定名）
    const char* func_name = node->data.function_call.name;
    if (sym && sym->is_library && sym->qualified_name) {
        // 这是导入的库函数，使用完全限定名
        func_name = sym->qualified_name;
    }
    
    // 检查是否是内部定义的函数（有地址）
    FunctionEntry* func = bytecode_find_function(ctx->module, func_name);
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
            // 从符号表获取返回类型
            DataType return_type = TYPE_VOID;
            if (sym->type) {
                if (sym->type->base_type == TYPE_FUNCTION && sym->type->func_info.return_type) {
                    // 函数类型，从func_info中获取返回类型
                    return_type = sym->type->func_info.return_type->base_type;
                } else if (sym->type->base_type != TYPE_FUNCTION) {
                    // 简单类型（不应该出现，但作为fallback）
                    return_type = sym->type->base_type;
                }
            }
            
            // 添加到函数表（使用完全限定名）
            uint32_t func_idx = bytecode_add_function(ctx->module,
                                                      func_name,  // 使用完全限定名
                                                      0,  // 外部函数地址为0
                                                      sym->param_count,
                                                      0,  // 无局部变量
                                                      return_type,  // 从符号表获取返回类型
                                                      NULL);  // 无参数类型
            if (func_idx == (uint32_t)-1) {
                ctx->error_code = ERR_RUNTIME;
                snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                        "Failed to add external function: %s", func_name);
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
    
    return OK;
}

/**
 * @brief 生成成员访问代码（质量化类型的.VAL/.QUALITY）
 */
static ErrorCode generate_member_access(CodeGenContext* ctx, ASTNode* node) {
    // 成员访问：variable.VAL 或 variable.QUALITY
    
    // 检查对象是否是标识符
    if (node->data.member_access.object->type != AST_IDENTIFIER) {
        snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                "Member access object must be an identifier");
        ctx->error_code = ERR_RUNTIME;
        return ERR_RUNTIME;
    }
    
    // 获取变量符号
    const char* var_name = node->data.member_access.object->data.identifier.name;
    Symbol* var_sym = symtbl_lookup(ctx->symtbl, var_name);
    if (!var_sym) {
        snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                "Undefined variable: %s", var_name);
        ctx->error_code = ERR_NAME;
        return ERR_NAME;
    }
    
    // 检查是否是质量化类型
    if (!is_qualified_type(var_sym->type->base_type)) {
        snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                "Variable '%s' is not a qualified type", var_name);
        ctx->error_code = ERR_TYPE;
        return ERR_TYPE;
    }
    
    // 根据成员类型生成相应的指令
    uint8_t flags = var_sym->is_global ? FLAG_GLOBAL : 0x00;
    uint16_t var_offset = var_sym->is_global ? var_sym->index : var_sym->offset;
    
    switch (node->data.member_access.member) {
        case MEMBER_VAL:
            // 生成 OP_LOAD_VAL 指令
            codegen_emit_with_flags(ctx, OP_LOAD_VAL, flags, var_offset);
            break;
            
        case MEMBER_QUALITY:
            // 生成 OP_LOAD_QUALITY 指令
            codegen_emit_with_flags(ctx, OP_LOAD_QUALITY, flags, var_offset);
            break;
            
        default:
            snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                    "Unknown member type: %d", node->data.member_access.member);
            ctx->error_code = ERR_RUNTIME;
            return ERR_RUNTIME;
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
            
        case AST_REPEAT:
            return generate_repeat(ctx, stmt, loop_ctx);
            
        case AST_RETURN:
            return generate_return(ctx, stmt);
            
        case AST_CASE:
            return generate_case(ctx, stmt, loop_ctx);
            
        case AST_BLOCK:
            // AST_BLOCK节点的语句列表存储在 next 指针中（见 ast_create_block）
            return generate_block(ctx, stmt->next, loop_ctx);
            
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
        const char* var_name = target->data.identifier.name;
        Symbol* sym = NULL;
        
        // 如果当前在函数内，先尝试查找完全限定名（用于静态变量）
        if (ctx->current_function) {
            const char* func_name = ctx->current_function->name;
            size_t qualified_len = strlen(func_name) + strlen(var_name) + 2;
            char* qualified_name = (char*)mmgr_alloc(qualified_len);
            if (qualified_name) {
                snprintf(qualified_name, qualified_len, "%s.%s", func_name, var_name);
                sym = symtbl_lookup(ctx->symtbl, qualified_name);
                
                if (sym && sym->is_static) {
                    // 找到静态变量
                    mmgr_free(qualified_name);
                    
                    // 检查是否为外部I/O变量
                    if (sym->is_external && sym->io_address) {
                        // 生成 OP_IO_WRITE 指令
                        int32_t addr_index = codegen_add_string_constant(ctx, sym->io_address);
                        if (addr_index < 0) {
                            ctx->error_code = ERR_OUT_OF_MEMORY;
                            return ERR_OUT_OF_MEMORY;
                        }
                        codegen_emit(ctx, OP_IO_WRITE, (uint16_t)addr_index);
                        return OK;
                    }
                    
                    uint8_t flags = 0x01;  // 静态变量在全局区
                    uint16_t addr = sym->index;
                    codegen_emit_with_flags(ctx, OP_STORE, flags, addr);
                    return OK;
                }
                mmgr_free(qualified_name);
            }
        }
        
        // 否则使用常规查找
        sym = symtbl_lookup(ctx->symtbl, var_name);
        if (!sym) {
            snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                    "Undefined variable: %s", var_name);
            ctx->error_code = ERR_NAME;
            return ERR_NAME;
        }
        
        // 检查是否为外部I/O变量
        if (sym->is_external && sym->io_address) {
            // 生成 OP_IO_WRITE 指令
            int32_t addr_index = codegen_add_string_constant(ctx, sym->io_address);
            if (addr_index < 0) {
                ctx->error_code = ERR_OUT_OF_MEMORY;
                return ERR_OUT_OF_MEMORY;
            }
            codegen_emit(ctx, OP_IO_WRITE, (uint16_t)addr_index);
            return OK;
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
        
    } else if (target->type == AST_MEMBER_ACCESS) {
        // 成员访问赋值：variable.VAL := value 或 variable.QUALITY := value
        
        // 检查对象是否是标识符
        if (target->data.member_access.object->type != AST_IDENTIFIER) {
            snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                    "Member access object must be an identifier");
            ctx->error_code = ERR_RUNTIME;
            return ERR_RUNTIME;
        }
        
        // 获取变量符号
        const char* var_name = target->data.member_access.object->data.identifier.name;
        Symbol* var_sym = symtbl_lookup(ctx->symtbl, var_name);
        if (!var_sym) {
            snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                    "Undefined variable: %s", var_name);
            ctx->error_code = ERR_NAME;
            return ERR_NAME;
        }
        
        // 检查是否是质量化类型
        if (!is_qualified_type(var_sym->type->base_type)) {
            snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                    "Variable '%s' is not a qualified type", var_name);
            ctx->error_code = ERR_TYPE;
            return ERR_TYPE;
        }
        
        // 根据成员类型生成相应的存储指令
        uint8_t flags = var_sym->is_global ? FLAG_GLOBAL : 0x00;
        uint16_t var_offset = var_sym->is_global ? var_sym->index : var_sym->offset;
        
        switch (target->data.member_access.member) {
            case MEMBER_VAL:
                // 生成 OP_STORE_VAL 指令
                codegen_emit_with_flags(ctx, OP_STORE_VAL, flags, var_offset);
                break;
                
            case MEMBER_QUALITY:
                // 生成 OP_STORE_QUALITY 指令
                codegen_emit_with_flags(ctx, OP_STORE_QUALITY, flags, var_offset);
                break;
                
            default:
                snprintf(ctx->error_msg, sizeof(ctx->error_msg),
                        "Unknown member type: %d", target->data.member_access.member);
                ctx->error_code = ERR_RUNTIME;
                return ERR_RUNTIME;
        }
        
        return OK;
        
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
 * @brief 生成repeat循环
 * REPEAT ... UNTIL condition END_REPEAT
 * 循环体至少执行一次，然后检查条件，条件为真时退出
 */
static ErrorCode generate_repeat(CodeGenContext* ctx, ASTNode* node, LoopContext* parent_loop) {
    ErrorCode err;
    
    // 创建循环上下文
    LoopContext loop_ctx;
    loop_ctx.break_labels = NULL;
    loop_ctx.continue_labels = NULL;
    loop_ctx.parent = parent_loop;
    
    // 循环开始位置
    int32_t loop_start = codegen_current_position(ctx);
    
    // 循环体（至少执行一次）
    err = generate_block(ctx, node->data.repeat_stmt.body, &loop_ctx);
    if (err != OK) return err;
    
    // continue跳转到这里（检查条件前）
    int32_t continue_pos = codegen_current_position(ctx);
    
    // 计算终止条件
    err = codegen_expr(ctx, node->data.repeat_stmt.condition);
    if (err != OK) return err;
    
    // JZ（条件为假时）跳回循环开始
    codegen_emit(ctx, OP_JZ, loop_start);
    
    // 循环结束位置
    int32_t loop_end = codegen_current_position(ctx);
    
    // 回填所有break标签到循环结束
    codegen_patch_jump_list(ctx, loop_ctx.break_labels, loop_end);
    
    // 回填所有continue标签到条件检查位置
    codegen_patch_jump_list(ctx, loop_ctx.continue_labels, continue_pos);
    
    // 释放标签
    jump_label_free_list(loop_ctx.break_labels);
    jump_label_free_list(loop_ctx.continue_labels);
    
    return OK;
}

/**
 * @brief 生成 CASE 语句
 * 
 * CASE 语句的代码生成策略：
 * 1. 计算 CASE 表达式并保存在栈上（需要多次比较）
 * 2. 对每个分支的每个标签：
 *    - 复制栈顶的表达式值
 *    - 计算标签值
 *    - 比较是否相等
 *    - 如果相等，跳转到该分支的代码
 * 3. 所有标签都不匹配，跳转到 ELSE 分支（如果有）或结束
 * 4. 每个分支执行完后跳转到 CASE 语句结束
 */
static ErrorCode generate_case(CodeGenContext* ctx, ASTNode* node, LoopContext* loop_ctx) {
    ErrorCode err;
    
    // 1. 计算 CASE 表达式
    err = codegen_expr(ctx, node->data.case_stmt.expression);
    if (err != OK) return err;
    
    // 用于存储每个分支开始位置的跳转索引
    typedef struct {
        int32_t* jump_indices;  // 该分支所有标签的跳转索引
        int label_count;        // 标签数量
    } BranchJumps;
    
    int case_count = node->data.case_stmt.case_count;
    BranchJumps* branch_jumps = (BranchJumps*)mmgr_alloc(sizeof(BranchJumps) * case_count);
    if (!branch_jumps) return ERR_OUT_OF_MEMORY;
    
    // 2. 为每个分支的每个标签生成比较和条件跳转
    for (int i = 0; i < case_count; i++) {
        ASTNode* case_elem = node->data.case_stmt.cases[i];
        if (!case_elem || case_elem->type != AST_CASE_ELEMENT) {
            continue;
        }
        
        // 计算该分支有多少个标签
        int label_count = 0;
        ASTNode* label = case_elem->data.case_element.labels;
        while (label) {
            label_count++;
            label = label->next;
        }
        
        branch_jumps[i].label_count = label_count;
        branch_jumps[i].jump_indices = (int32_t*)mmgr_alloc(sizeof(int32_t) * label_count);
        if (!branch_jumps[i].jump_indices) {
            for (int j = 0; j < i; j++) {
                mmgr_free(branch_jumps[j].jump_indices);
            }
            mmgr_free(branch_jumps);
            return ERR_OUT_OF_MEMORY;
        }
        
        // 为每个标签生成比较代码
        label = case_elem->data.case_element.labels;
        int label_idx = 0;
        while (label) {
            // 复制栈顶的 CASE 表达式值（用于比较）
            codegen_emit(ctx, OP_DUP, 0);
            
            // 计算标签值
            err = codegen_expr(ctx, label);
            if (err != OK) {
                for (int j = 0; j <= i; j++) {
                    mmgr_free(branch_jumps[j].jump_indices);
                }
                mmgr_free(branch_jumps);
                return err;
            }
            
            // 比较是否相等
            codegen_emit(ctx, OP_EQ, 0);
            
            // 如果相等，跳转到该分支（稍后回填）
            branch_jumps[i].jump_indices[label_idx] = codegen_emit(ctx, OP_JNZ, 0);
            
            label_idx++;
            label = label->next;
        }
    }
    
    // 3. 所有标签都不匹配，跳转到 ELSE 或结束
    int32_t default_jump = codegen_emit(ctx, OP_JMP, 0);
    
    // 用于存储每个分支结束时跳转到 CASE 结束的跳转索引
    int32_t* end_jumps = (int32_t*)mmgr_alloc(sizeof(int32_t) * case_count);
    if (!end_jumps) {
        for (int i = 0; i < case_count; i++) {
            mmgr_free(branch_jumps[i].jump_indices);
        }
        mmgr_free(branch_jumps);
        return ERR_OUT_OF_MEMORY;
    }
    
    // 4. 生成每个分支的代码
    for (int i = 0; i < case_count; i++) {
        ASTNode* case_elem = node->data.case_stmt.cases[i];
        if (!case_elem) continue;
        
        // 回填该分支所有标签的跳转地址到这里
        int32_t branch_start = codegen_current_position(ctx);
        for (int j = 0; j < branch_jumps[i].label_count; j++) {
            codegen_patch_jump(ctx, branch_jumps[i].jump_indices[j], branch_start);
        }
        
        // 弹出栈顶的 CASE 表达式值（已经匹配，不再需要）
        codegen_emit(ctx, OP_POP, 0);
        
        // 生成分支语句
        if (case_elem->data.case_element.statements) {
            err = generate_block(ctx, case_elem->data.case_element.statements, loop_ctx);
            if (err != OK) {
                for (int j = 0; j < case_count; j++) {
                    mmgr_free(branch_jumps[j].jump_indices);
                }
                mmgr_free(branch_jumps);
                mmgr_free(end_jumps);
                return err;
            }
        }
        
        // 跳转到 CASE 结束
        end_jumps[i] = codegen_emit(ctx, OP_JMP, 0);
    }
    
    // 5. 生成 ELSE 分支（如果有）
    int32_t else_start = codegen_current_position(ctx);
    codegen_patch_jump(ctx, default_jump, else_start);
    
    // 弹出栈顶的 CASE 表达式值
    codegen_emit(ctx, OP_POP, 0);
    
    if (node->data.case_stmt.default_case) {
        err = generate_block(ctx, node->data.case_stmt.default_case, loop_ctx);
        if (err != OK) {
            for (int i = 0; i < case_count; i++) {
                mmgr_free(branch_jumps[i].jump_indices);
            }
            mmgr_free(branch_jumps);
            mmgr_free(end_jumps);
            return err;
        }
    }
    
    // 6. 回填所有分支结束时的跳转到 CASE 结束
    int32_t case_end = codegen_current_position(ctx);
    for (int i = 0; i < case_count; i++) {
        codegen_patch_jump(ctx, end_jumps[i], case_end);
    }
    
    // 释放内存
    for (int i = 0; i < case_count; i++) {
        mmgr_free(branch_jumps[i].jump_indices);
    }
    mmgr_free(branch_jumps);
    mmgr_free(end_jumps);
    
    return OK;
}

/**
 * @brief 生成return语句
 */
static ErrorCode generate_return(CodeGenContext* ctx, ASTNode* node) {
    ErrorCode err;
    
    if (node->data.return_stmt.value) {
        // 计算返回值表达式
        err = codegen_expr(ctx, node->data.return_stmt.value);
        if (err != OK) return err;
        
        // IEC 61131-3: 返回值通过赋值给函数名变量实现
        // 将栈顶的返回值赋给返回值变量（函数名变量）
        if (ctx->current_function && ctx->current_function->name) {
            Symbol* return_var = symtbl_lookup(ctx->symtbl, ctx->current_function->name);
            if (return_var && return_var->scope_level > 0) {  // 局部变量
                uint8_t flags = 0x00;  // 局部变量标志
                uint16_t offset = return_var->index;
                codegen_emit_with_flags(ctx, OP_STORE, flags, offset);
            }
        }
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
