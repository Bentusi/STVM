/*
 * IEC61131 ST语言代码生成器实现
 * 将AST转换为字节码，支持库函数调用和主从同步
 */

#include "codegen.h"
#include "mmgr.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* 内部辅助函数声明 */
static int codegen_compile_node_recursive(codegen_context_t *ctx, ast_node_t *node);
static int codegen_process_import_list(codegen_context_t *ctx, ast_node_t *import_list);
static uint32_t codegen_calculate_alignment(type_info_t *type);
static bool codegen_validate_function_signature(const function_call_info_t *call_info,
                                               uint32_t arg_count);

/* ========== 代码生成器管理函数 ========== */

/* 创建代码生成器上下文 */
codegen_context_t* codegen_create_context(void) {
    codegen_context_t *ctx = (codegen_context_t*)mmgr_alloc_general(sizeof(codegen_context_t));
    if (!ctx) {
        return NULL;
    }
    
    memset(ctx, 0, sizeof(codegen_context_t));
    
    /* 初始化控制流管理 */
    ctx->control_flow.break_labels = (uint32_t*)mmgr_alloc_general(sizeof(uint32_t) * 32);
    ctx->control_flow.continue_labels = (uint32_t*)mmgr_alloc_general(sizeof(uint32_t) * 32);
    
    if (!ctx->control_flow.break_labels || !ctx->control_flow.continue_labels) {
        return NULL;
    }
    
    ctx->control_flow.label_stack_depth = 0;
    ctx->control_flow.next_temp_label = 1;
    
    return ctx;
}

/* 销毁代码生成器上下文 */
void codegen_destroy_context(codegen_context_t *ctx) {    
    /* 内存由MMGR管理，不需要显式释放 */
}

/* 初始化代码生成器上下文 */
int codegen_init_context(codegen_context_t *ctx, symbol_table_manager_t *sym_mgr,
                         library_manager_t *lib_mgr, master_slave_sync_t *sync_mgr) {
    if (!ctx || !sym_mgr || !lib_mgr) {
        return -1;
    }
    
    /* 创建字节码生成器 */
    ctx->bc_gen = bytecode_generator_create();
    if (!ctx->bc_gen) {
        codegen_set_error(ctx, "无法创建字节码生成器");
        return -1;
    }
    
    ctx->sym_mgr = sym_mgr;
    ctx->lib_mgr = lib_mgr;
    ctx->sync_enabled = false;
    
    /* 初始化变量分配偏移 */
    ctx->var_allocation.global_offset = 0;
    ctx->var_allocation.local_offset = 0;
    ctx->var_allocation.param_offset = 0;
    
    return 0;
}

/* ========== AST编译函数 ========== */

/* 编译程序 */
int codegen_compile_program(codegen_context_t *ctx, ast_node_t *program_node,
                           bytecode_file_t *output_file) {
    if (!ctx || !program_node || !output_file) {
        codegen_set_error(ctx, "编译程序：无效参数");
        return -1;
    }
    
    if (program_node->type != AST_PROGRAM) {
        codegen_set_error(ctx, "编译程序：节点类型不是程序");
        return -1;
    }
    
    /* 处理导入列表 */
    if (program_node->data.program.imports) {
        if (codegen_process_import_list(ctx, program_node->data.program.imports) != 0) {
            return -1;
        }
    }
    
    /* 编译声明列表 */
    if (program_node->data.program.declarations) {
        if (codegen_compile_ast(ctx, program_node->data.program.declarations) != 0) {
            return -1;
        }
    }
    
    /* 生成主程序入口标签 */
    bytecode_mark_label(ctx->bc_gen, "main");
    
    /* 编译语句列表 */
    if (program_node->data.program.statements) {
        if (codegen_compile_statement_list(ctx, program_node->data.program.statements) != 0) {
            return -1;
        }
    }
    
    /* 生成程序结束指令 */
    bytecode_emit_instr(ctx->bc_gen, OP_HALT);
    
    /* 生成字节码文件 */
    if (bytecode_generate_file(ctx->bc_gen, output_file) != 0) {
        codegen_set_error(ctx, "生成字节码文件失败");
        return -1;
    }
    
    ctx->statistics.instructions_generated = ctx->bc_gen->instr_count;
    
    return 0;
}

/* 编译AST节点 */
int codegen_compile_ast(codegen_context_t *ctx, ast_node_t *node) {
    if (!ctx || !node) {
        return -1;
    }
    
    return codegen_compile_node_recursive(ctx, node);
}

/* 递归编译AST节点 */
static int codegen_compile_node_recursive(codegen_context_t *ctx, ast_node_t *node) {
    if (!node) return 0;
    
    switch (node->type) {
        case AST_VAR_DECL:
            return codegen_compile_var_declaration(ctx, node);
            
        case AST_FUNCTION_DECL:
            return codegen_compile_function_declaration(ctx, node);
            
        case AST_ASSIGNMENT:
            return codegen_compile_assignment(ctx, node);
            
        case AST_IF_STMT:
            return codegen_compile_if_statement(ctx, node);
            
        case AST_FOR_STMT:
            return codegen_compile_for_statement(ctx, node);
            
        case AST_WHILE_STMT:
            return codegen_compile_while_statement(ctx, node);
            
        case AST_CASE_STMT:
            return codegen_compile_case_statement(ctx, node);
            
        case AST_RETURN_STMT:
            return codegen_compile_return_statement(ctx, node);
            
        case AST_BINARY_OP:
            return codegen_compile_binary_operation(ctx, node);
            
        case AST_UNARY_OP:
            return codegen_compile_unary_operation(ctx, node);
            
        case AST_FUNCTION_CALL:
            return codegen_compile_function_call(ctx, node);
            
        case AST_IDENTIFIER:
            return codegen_compile_identifier(ctx, node);
            
        case AST_LITERAL:
            return codegen_compile_literal(ctx, node);
            
        case AST_DECLARATION_LIST:
        case AST_STATEMENT_LIST:
            /* 编译列表中的所有项 */
            if (codegen_compile_node_recursive(ctx, node) != 0) return -1;
            if (node->next) {
                return codegen_compile_node_recursive(ctx, node->next);
            }
            return 0;
            
        default:
            codegen_set_error(ctx, "不支持的AST节点类型");
            return -1;
    }
}

/* ========== 声明编译 ========== */

/* 编译变量声明 */
int codegen_compile_var_declaration(codegen_context_t *ctx, ast_node_t *var_decl) {
    if (!ctx || !var_decl || var_decl->type != AST_VAR_DECL) {
        return -1;
    }
    
    bool is_global = (var_decl->data.var_decl.category == SYMBOL_VAR_GLOBAL);
    
    /* 遍历变量列表 */
    ast_node_t *var_item = var_decl->data.var_decl.var_list;
    while (var_item) {
        if (var_item->type == AST_VAR_ITEM) {
            const char *var_name = var_item->data.var_item.name;
            type_info_t *var_type = var_item->data.var_item.type;
            
            /* 分配变量 */
            if (codegen_allocate_variable(ctx, var_name, var_type, is_global, false) != 0) {
                return -1;
            }
            
            /* 如果有初始值，生成初始化代码 */
            if (var_item->data.var_item.init_value) {
                /* 编译初始值表达式 */
                if (codegen_compile_expression(ctx, var_item->data.var_item.init_value) != 0) {
                    return -1;
                }
                
                /* 存储到变量 */
                var_access_info_t *access = codegen_get_variable_access(ctx, var_name);
                if (!access) {
                    codegen_set_error(ctx, "获取变量访问信息失败");
                    return -1;
                }
                
                if (codegen_generate_variable_store(ctx, access) != 0) {
                    return -1;
                }
            }
            
            ctx->statistics.variables_allocated++;
        }
        
        var_item = var_item->next;
    }
    
    return 0;
}

/* 编译函数声明 */
int codegen_compile_function_declaration(codegen_context_t *ctx, ast_node_t *func_decl) {
    if (!ctx || !func_decl || func_decl->type != AST_FUNCTION_DECL) {
        return -1;
    }
    
    const char *func_name = func_decl->data.function_decl.name;
    type_info_t *return_type = func_decl->data.function_decl.return_type;
    
    /* 设置当前函数上下文 */
    ctx->current_function.name = mmgr_alloc_string(func_name);
    ctx->current_function.return_type = return_type;
    ctx->current_function.has_return_value = (return_type != NULL);
    ctx->current_function.local_var_offset = 0;
    
    /* 创建函数标签 */
    char func_label[256];
    snprintf(func_label, sizeof(func_label), "func_%s", func_name);
    
    /* 标记函数开始 */
    bytecode_mark_label(ctx->bc_gen, func_label);
    
    /* 进入函数作用域 */
    scope_t *func_scope = enter_scope(func_name, SCOPE_FUNCTION);
    if (!func_scope) {
        codegen_set_error(ctx, "无法创建函数作用域");
        return -1;
    }
    
    /* 编译函数参数（如果有） */
    if (func_decl->data.function_decl.parameters) {
        /* 参数编译逻辑 - 简化版 */
        ctx->current_function.param_count = 1; /* 占位 */
    }
    
    /* 编译局部声明 */
    if (func_decl->data.function_decl.declarations) {
        if (codegen_compile_ast(ctx, func_decl->data.function_decl.declarations) != 0) {
            exit_scope();
            return -1;
        }
    }
    
    /* 编译函数体 */
    if (func_decl->data.function_decl.statements) {
        if (codegen_compile_statement_list(ctx, func_decl->data.function_decl.statements) != 0) {
            exit_scope();
            return -1;
        }
    }
    
    /* 如果函数没有显式返回，添加默认返回 */
    if (ctx->current_function.has_return_value) {
        /* 有返回值的函数需要默认返回值 */
        switch (return_type->base_type) {
            case TYPE_INT_ID:
                bytecode_emit_instr_int(ctx->bc_gen, OP_LOAD_CONST_INT, 0);
                break;
            case TYPE_REAL_ID:
                bytecode_emit_instr_real(ctx->bc_gen, OP_LOAD_CONST_REAL, 0.0);
                break;
            case TYPE_BOOL_ID:
                bytecode_emit_instr_int(ctx->bc_gen, OP_LOAD_CONST_BOOL, 0);
                break;
            default:
                break;
        }
        bytecode_emit_instr(ctx->bc_gen, OP_RET_VALUE);
    } else {
        bytecode_emit_instr(ctx->bc_gen, OP_RET);
    }
    
    /* 退出函数作用域 */
    exit_scope();
    
    /* 注册函数 */
    uint32_t func_addr = bytecode_get_label_address(ctx->bc_gen, func_label);
    codegen_register_function(ctx, func_name, return_type, ctx->current_function.param_count);
    
    /* 清除当前函数上下文 */
    memset(&ctx->current_function, 0, sizeof(ctx->current_function));
    
    ctx->statistics.functions_compiled++;
    
    return 0;
}

/* ========== 语句编译 ========== */

/* 编译语句列表 */
int codegen_compile_statement_list(codegen_context_t *ctx, ast_node_t *stmt_list) {
    if (!ctx) return -1;
    
    ast_node_t *current = stmt_list;
    while (current) {
        if (codegen_compile_node_recursive(ctx, current) != 0) {
            return -1;
        }
        current = current->next;
    }
    
    return 0;
}

/* 编译赋值语句 */
int codegen_compile_assignment(codegen_context_t *ctx, ast_node_t *assignment) {
    if (!ctx || !assignment || assignment->type != AST_ASSIGNMENT) {
        return -1;
    }
    
    const char *target_name = assignment->data.assignment.target;
    ast_node_t *value_expr = assignment->data.assignment.value;
    
    /* 编译右侧表达式 */
    if (codegen_compile_expression(ctx, value_expr) != 0) {
        return -1;
    }
    
    /* 获取目标变量访问信息 */
    var_access_info_t *access = codegen_get_variable_access(ctx, target_name);
    if (!access) {
        codegen_set_error(ctx, "未找到目标变量");
        return -1;
    }
    
    /* 生成存储指令 */
    if (codegen_generate_variable_store(ctx, access) != 0) {
        return -1;
    }
    
    /* 如果是同步变量，生成同步指令 */
    if (ctx->sync_enabled && access->is_sync) {
        if (codegen_generate_sync_instruction(ctx, target_name) != 0) {
            return -1;
        }
    }
    
    return 0;
}

/* 编译IF语句 */
int codegen_compile_if_statement(codegen_context_t *ctx, ast_node_t *if_stmt) {
    if (!ctx || !if_stmt || if_stmt->type != AST_IF_STMT) {
        return -1;
    }
    
    uint32_t else_label = codegen_create_temp_label(ctx);
    uint32_t end_label = codegen_create_temp_label(ctx);
    
    char *else_label_name = codegen_get_temp_label_name(ctx, else_label);
    char *end_label_name = codegen_get_temp_label_name(ctx, end_label);
    
    /* 编译条件表达式 */
    if (codegen_compile_expression(ctx, if_stmt->data.if_stmt.condition) != 0) {
        return -1;
    }
    
    /* 条件为假时跳转到else分支 */
    codegen_emit_jump_if_false(ctx, else_label_name);
    
    /* 编译then语句 */
    if (codegen_compile_statement_list(ctx, if_stmt->data.if_stmt.then_stmt) != 0) {
        return -1;
    }
    
    /* 跳转到语句结束 */
    codegen_emit_jump(ctx, end_label_name);
    
    /* else分支标签 */
    bytecode_mark_label(ctx->bc_gen, else_label_name);
    
    /* 编译else语句（如果有） */
    if (if_stmt->data.if_stmt.else_stmt) {
        if (codegen_compile_statement_list(ctx, if_stmt->data.if_stmt.else_stmt) != 0) {
            return -1;
        }
    }
    
    /* 语句结束标签 */
    bytecode_mark_label(ctx->bc_gen, end_label_name);
    
    return 0;
}

/* 编译FOR语句 */
int codegen_compile_for_statement(codegen_context_t *ctx, ast_node_t *for_stmt) {
    if (!ctx || !for_stmt || for_stmt->type != AST_FOR_STMT) {
        return -1;
    }
    
    const char *loop_var = for_stmt->data.for_stmt.var_name;
    uint32_t loop_start = codegen_create_temp_label(ctx);
    uint32_t loop_end = codegen_create_temp_label(ctx);
    
    char *loop_start_name = codegen_get_temp_label_name(ctx, loop_start);
    char *loop_end_name = codegen_get_temp_label_name(ctx, loop_end);
    
    /* 编译初始值并赋给循环变量 */
    if (codegen_compile_expression(ctx, for_stmt->data.for_stmt.start_value) != 0) {
        return -1;
    }
    
    var_access_info_t *loop_var_access = codegen_get_variable_access(ctx, loop_var);
    if (!loop_var_access) {
        codegen_set_error(ctx, "循环变量未声明");
        return -1;
    }
    
    if (codegen_generate_variable_store(ctx, loop_var_access) != 0) {
        return -1;
    }
    
    /* 循环开始标签 */
    bytecode_mark_label(ctx->bc_gen, loop_start_name);
    
    /* 检查循环条件：循环变量 <= 结束值 */
    if (codegen_generate_variable_load(ctx, loop_var_access) != 0) {
        return -1;
    }
    
    if (codegen_compile_expression(ctx, for_stmt->data.for_stmt.end_value) != 0) {
        return -1;
    }
    
    /* 比较并判断是否继续循环 */
    bytecode_emit_instr(ctx->bc_gen, OP_LE_INT); /* 假设是整数循环 */
    codegen_emit_jump_if_false(ctx, loop_end_name);
    
    /* 设置break和continue标签 */
    codegen_push_break_label(ctx, loop_end);
    codegen_push_continue_label(ctx, loop_start);
    
    /* 编译循环体 */
    if (codegen_compile_statement_list(ctx, for_stmt->data.for_stmt.statements) != 0) {
        return -1;
    }
    
    /* 增加循环变量 */
    if (codegen_generate_variable_load(ctx, loop_var_access) != 0) {
        return -1;
    }
    
    if (for_stmt->data.for_stmt.by_value) {
        /* 有BY子句，使用指定步长 */
        if (codegen_compile_expression(ctx, for_stmt->data.for_stmt.by_value) != 0) {
            return -1;
        }
    } else {
        /* 默认步长为1 */
        bytecode_emit_instr_int(ctx->bc_gen, OP_LOAD_CONST_INT, 1);
    }
    
    bytecode_emit_instr(ctx->bc_gen, OP_ADD_INT);
    if (codegen_generate_variable_store(ctx, loop_var_access) != 0) {
        return -1;
    }
    
    /* 跳回循环开始 */
    codegen_emit_jump(ctx, loop_start_name);
    
    /* 循环结束标签 */
    bytecode_mark_label(ctx->bc_gen, loop_end_name);
    
    /* 弹出标签栈 */
    codegen_pop_break_label(ctx);
    codegen_pop_continue_label(ctx);
    
    return 0;
}

/* 编译WHILE语句 */
int codegen_compile_while_statement(codegen_context_t *ctx, ast_node_t *while_stmt) {
    if (!ctx || !while_stmt || while_stmt->type != AST_WHILE_STMT) {
        return -1;
    }
    
    uint32_t loop_start = codegen_create_temp_label(ctx);
    uint32_t loop_end = codegen_create_temp_label(ctx);
    
    char *loop_start_name = codegen_get_temp_label_name(ctx, loop_start);
    char *loop_end_name = codegen_get_temp_label_name(ctx, loop_end);
    
    /* 循环开始标签 */
    bytecode_mark_label(ctx->bc_gen, loop_start_name);
    
    /* 编译循环条件 */
    if (codegen_compile_expression(ctx, while_stmt->data.while_stmt.condition) != 0) {
        return -1;
    }
    
    /* 条件为假时跳出循环 */
    codegen_emit_jump_if_false(ctx, loop_end_name);
    
    /* 设置break和continue标签 */
    codegen_push_break_label(ctx, loop_end);
    codegen_push_continue_label(ctx, loop_start);
    
    /* 编译循环体 */
    if (codegen_compile_statement_list(ctx, while_stmt->data.while_stmt.statements) != 0) {
        return -1;
    }
    
    /* 跳回循环开始 */
    codegen_emit_jump(ctx, loop_start_name);
    
    /* 循环结束标签 */
    bytecode_mark_label(ctx->bc_gen, loop_end_name);
    
    /* 弹出标签栈 */
    codegen_pop_break_label(ctx);
    codegen_pop_continue_label(ctx);
    
    return 0;
}

/* 编译CASE语句 - 简化实现 */
int codegen_compile_case_statement(codegen_context_t *ctx, ast_node_t *case_stmt) {
    if (!ctx || !case_stmt || case_stmt->type != AST_CASE_STMT) {
        return -1;
    }
    
    /* 简化实现：转换为IF-ELSIF链 */
    codegen_set_error(ctx, "CASE语句暂未实现");
    return -1;
}

/* 编译RETURN语句 */
int codegen_compile_return_statement(codegen_context_t *ctx, ast_node_t *return_stmt) {
    if (!ctx || !return_stmt || return_stmt->type != AST_RETURN_STMT) {
        return -1;
    }
    
    /* 如果有返回值 */
    if (return_stmt->data.return_stmt.return_value) {
        /* 编译返回值表达式 */
        if (codegen_compile_expression(ctx, return_stmt->data.return_stmt.return_value) != 0) {
            return -1;
        }
        
        /* 生成带返回值的返回指令 */
        bytecode_emit_instr(ctx->bc_gen, OP_RET_VALUE);
    } else {
        /* 生成无返回值的返回指令 */
        bytecode_emit_instr(ctx->bc_gen, OP_RET);
    }
    
    return 0;
}

/* ========== 表达式编译 ========== */

/* 编译表达式 */
int codegen_compile_expression(codegen_context_t *ctx, ast_node_t *expr) {
    return codegen_compile_node_recursive(ctx, expr);
}

/* 编译二元操作 */
int codegen_compile_binary_operation(codegen_context_t *ctx, ast_node_t *binary_op) {
    if (!ctx || !binary_op || binary_op->type != AST_BINARY_OP) {
        return -1;
    }
    
    operator_type_t op = binary_op->data.binary_op.op;
    ast_node_t *left = binary_op->data.binary_op.left;
    ast_node_t *right = binary_op->data.binary_op.right;
    
    /* 编译左操作数 */
    if (codegen_compile_expression(ctx, left) != 0) {
        return -1;
    }
    
    /* 编译右操作数 */
    if (codegen_compile_expression(ctx, right) != 0) {
        return -1;
    }
    
    /* 选择操作码（简化：假设都是整数操作） */
    opcode_t opcode;
    switch (op) {
        case OP_ADD:
            opcode = OP_ADD_INT;
            break;
        case OP_SUB:
            opcode = OP_SUB_INT;
            break;
        case OP_MUL:
            opcode = OP_MUL_INT;
            break;
        case OP_DIV:
            opcode = OP_DIV_INT;
            break;
        case OP_MOD:
            opcode = OP_MOD_INT;
            break;
        case OP_EQ:
            opcode = OP_EQ_INT;
            break;
        case OP_NE:
            opcode = OP_NE_INT;
            break;
        case OP_LT:
            opcode = OP_LT_INT;
            break;
        case OP_LE:
            opcode = OP_LE_INT;
            break;
        case OP_GT:
            opcode = OP_GT_INT;
            break;
        case OP_GE:
            opcode = OP_GE_INT;
            break;
        case OP_AND:
            opcode = OP_AND_BOOL;
            break;
        case OP_OR:
            opcode = OP_OR_BOOL;
            break;
        default:
            codegen_set_error(ctx, "不支持的二元操作符");
            return -1;
    }
    
    /* 生成操作指令 */
    bytecode_emit_instr(ctx->bc_gen, opcode);
    
    return 0;
}

/* 编译一元操作 */
int codegen_compile_unary_operation(codegen_context_t *ctx, ast_node_t *unary_op) {
    if (!ctx || !unary_op || unary_op->type != AST_UNARY_OP) {
        return -1;
    }
    
    operator_type_t op = unary_op->data.unary_op.op;
    ast_node_t *operand = unary_op->data.unary_op.operand;
    
    /* 编译操作数 */
    if (codegen_compile_expression(ctx, operand) != 0) {
        return -1;
    }
    
    /* 生成一元操作指令 */
    opcode_t opcode;
    switch (op) {
        case OP_NEG:
            opcode = OP_NEG_INT; /* 简化：假设是整数 */
            break;
        case OP_NOT:
            opcode = OP_NOT_BOOL;
            break;
        default:
            codegen_set_error(ctx, "不支持的一元操作符");
            return -1;
    }
    
    bytecode_emit_instr(ctx->bc_gen, opcode);
    
    return 0;
}

/* 编译函数调用 */
int codegen_compile_function_call(codegen_context_t *ctx, ast_node_t *func_call) {
    if (!ctx || !func_call || func_call->type != AST_FUNCTION_CALL) {
        return -1;
    }
    
    const char *func_name = func_call->data.function_call.function_name;
    const char *lib_name = func_call->data.function_call.library_name;
    ast_node_t *args = func_call->data.function_call.arguments;
    
    /* 编译参数列表 */
    uint32_t arg_count = 0;
    ast_node_t *current_arg = args;
    while (current_arg) {
        if (codegen_compile_expression(ctx, current_arg) != 0) {
            return -1;
        }
        arg_count++;
        current_arg = current_arg->next;
    }
    
    /* 解析函数调用信息 */
    function_call_info_t *call_info = codegen_resolve_function_call(ctx, func_name, lib_name);
    if (!call_info) {
        codegen_set_error(ctx, "无法解析函数调用");
        return -1;
    }
    
    /* 生成函数调用指令 */
    if (codegen_generate_function_call(ctx, call_info, arg_count) != 0) {
        return -1;
    }
    
    ctx->statistics.library_calls++;
    
    return 0;
}

/* 编译标识符 */
int codegen_compile_identifier(codegen_context_t *ctx, ast_node_t *identifier) {
    if (!ctx || !identifier || identifier->type != AST_IDENTIFIER) {
        return -1;
    }
    
    const char *name = identifier->data.identifier.name;
    
    /* 获取变量访问信息 */
    var_access_info_t *access = codegen_get_variable_access(ctx, name);
    if (!access) {
        codegen_set_error(ctx, "未找到变量");
        return -1;
    }
    
    /* 生成加载指令 */
    return codegen_generate_variable_load(ctx, access);
}

/* 编译字面量 */
int codegen_compile_literal(codegen_context_t *ctx, ast_node_t *literal) {
    if (!ctx || !literal || literal->type != AST_LITERAL) {
        return -1;
    }
    
    literal_type_t lit_type = literal->data.literal.literal_type;
    
    switch (lit_type) {
        case LITERAL_INT:
            bytecode_emit_instr_int(ctx->bc_gen, OP_LOAD_CONST_INT,
                                   literal->data.literal.value.int_val);
            break;
            
        case LITERAL_REAL:
            bytecode_emit_instr_real(ctx->bc_gen, OP_LOAD_CONST_REAL,
                                    literal->data.literal.value.real_val);
            break;
            
        case LITERAL_BOOL:
            bytecode_emit_instr_int(ctx->bc_gen, OP_LOAD_CONST_BOOL,
                                   literal->data.literal.value.bool_val ? 1 : 0);
            break;
            
        case LITERAL_STRING:
            /* 添加到常量池并生成指令 */
            uint32_t str_index = bytecode_add_const_string(ctx->bc_gen,
                                                          literal->data.literal.value.string_val);
            bytecode_emit_instr_int(ctx->bc_gen, OP_LOAD_CONST_INT, str_index);
            break;
            
        default:
            codegen_set_error(ctx, "不支持的字面量类型");
            return -1;
    }
    
    return 0;
}

/* ========== 变量和函数管理 ========== */

/* 分配变量 */
int codegen_allocate_variable(codegen_context_t *ctx, const char *name,
                             type_info_t *type, bool is_global, bool is_sync) {
    if (!ctx || !name || !type) {
        return -1;
    }
    
    uint32_t var_size = codegen_get_type_size(type);
    uint32_t alignment = codegen_calculate_alignment(type);
    
    /* 计算对齐后的偏移 */
    uint32_t offset;
    if (is_global) {
        ctx->var_allocation.global_offset = 
            (ctx->var_allocation.global_offset + alignment - 1) & ~(alignment - 1);
        offset = ctx->var_allocation.global_offset;
        ctx->var_allocation.global_offset += var_size;
    } else {
        ctx->var_allocation.local_offset = 
            (ctx->var_allocation.local_offset + alignment - 1) & ~(alignment - 1);
        offset = ctx->var_allocation.local_offset;
        ctx->var_allocation.local_offset += var_size;
    }
    
    /* 添加到字节码生成器的变量表 */
    uint32_t var_index = bytecode_add_variable(ctx->bc_gen, name, type->base_type, var_size, is_global);
    if (var_index == UINT32_MAX) {
        codegen_set_error(ctx, "添加变量到字节码失败");
        return -1;
    }
    
    return 0;
}

/* 获取变量访问信息 */
var_access_info_t* codegen_get_variable_access(codegen_context_t *ctx, const char *name) {
    if (!ctx || !name) {
        return NULL;
    }
    
    /* 从符号表查找变量 */
    symbol_t *symbol = lookup_symbol(name);
    if (!symbol) {
        return NULL;
    }
    
    var_access_info_t *access = (var_access_info_t*)mmgr_alloc_general(sizeof(var_access_info_t));
    if (!access) {
        return NULL;
    }
    
    access->type = symbol->data_type;
    access->offset = symbol->address;
    access->is_sync = false; /* 简化：默认不同步 */
    access->library_name = NULL;
    
    if (symbol->is_global) {
        access->access_type = VAR_ACCESS_GLOBAL;
    } else {
        access->access_type = VAR_ACCESS_LOCAL;
    }
    
    return access;
}

/* 生成变量加载指令 */
int codegen_generate_variable_load(codegen_context_t *ctx, const var_access_info_t *access) {
    if (!ctx || !access) {
        return -1;
    }
    
    opcode_t opcode;
    switch (access->access_type) {
        case VAR_ACCESS_GLOBAL:
            opcode = OP_LOAD_GLOBAL;
            break;
        case VAR_ACCESS_LOCAL:
            opcode = OP_LOAD_LOCAL;
            break;
        case VAR_ACCESS_PARAM:
            opcode = OP_LOAD_PARAM;
            break;
        default:
            codegen_set_error(ctx, "不支持的变量访问类型");
            return -1;
    }
    
    bytecode_emit_instr_int(ctx->bc_gen, opcode, access->offset);
    
    return 0;
}

/* 生成变量存储指令 */
int codegen_generate_variable_store(codegen_context_t *ctx, const var_access_info_t *access) {
    if (!ctx || !access) {
        return -1;
    }
    
    opcode_t opcode;
    switch (access->access_type) {
        case VAR_ACCESS_GLOBAL:
            opcode = OP_STORE_GLOBAL;
            break;
        case VAR_ACCESS_LOCAL:
            opcode = OP_STORE_LOCAL;
            break;
        case VAR_ACCESS_PARAM:
            opcode = OP_STORE_PARAM;
            break;
        default:
            codegen_set_error(ctx, "不支持的变量访问类型");
            return -1;
    }
    
    bytecode_emit_instr_int(ctx->bc_gen, opcode, access->offset);
    
    return 0;
}

/* ========== 辅助函数实现 ========== */

/* 处理导入列表 */
static int codegen_process_import_list(codegen_context_t *ctx, ast_node_t *import_list) {
    if (!ctx || !import_list) return 0;
    
    ast_node_t *current = import_list;
    while (current) {
        if (current->type == AST_IMPORT) {
            const char *lib_name = current->data.import.library_name;
            const char *alias = current->data.import.alias;
            const char *path = current->data.import.path;
            
            /* 导入库 */
            if (libmgr_import_library(lib_name, alias, path) == NULL) {
                codegen_set_error(ctx, "库导入失败");
                return -1;
            }
        }
        current = current->next;
    }
    
    return 0;
}

/* 计算类型对齐 */
static uint32_t codegen_calculate_alignment(type_info_t *type) {
    if (!type) return 1;
    
    switch (type->base_type) {
        case TYPE_BOOL_ID:
            return 1;
        case TYPE_INT_ID:
            return 2;
        case TYPE_DINT_ID:
        case TYPE_REAL_ID:
            return 4;
        case TYPE_STRING_ID:
            return sizeof(void*);
        default:
            return 1;
    }
}

/* 获取类型大小 */
uint32_t codegen_get_type_size(type_info_t *type) {
    if (!type) return 0;
    
    return type->size;
}

/* 创建临时标签 */
uint32_t codegen_create_temp_label(codegen_context_t *ctx) {
    if (!ctx) return 0;
    
    return ctx->control_flow.next_temp_label++;
}

/* 获取临时标签名 */
char* codegen_get_temp_label_name(codegen_context_t *ctx, uint32_t label_id) {
    if (!ctx) return NULL;
    
    static char label_name[64];
    snprintf(label_name, sizeof(label_name), ".L%u", label_id);
    return label_name;
}

/* 生成跳转指令 */
uint32_t codegen_emit_jump(codegen_context_t *ctx, const char *label) {
    if (!ctx || !label) return UINT32_MAX;
    
    uint32_t addr = bytecode_get_label_address(ctx->bc_gen, label);
    return bytecode_emit_instr_addr(ctx->bc_gen, OP_JMP, addr);
}

/* 生成条件跳转指令 */
uint32_t codegen_emit_jump_if_false(codegen_context_t *ctx, const char *label) {
    if (!ctx || !label) return UINT32_MAX;
    
    uint32_t addr = bytecode_get_label_address(ctx->bc_gen, label);
    return bytecode_emit_instr_addr(ctx->bc_gen, OP_JMP_FALSE, addr);
}

/* 设置错误信息 */
void codegen_set_error(codegen_context_t *ctx, const char *error_msg) {
    if (!ctx || !error_msg) return;
    
    strncpy(ctx->last_error, error_msg, sizeof(ctx->last_error) - 1);
    ctx->last_error[sizeof(ctx->last_error) - 1] = '\0';
    ctx->has_error = true;
}

/* 获取错误信息 */
const char* codegen_get_last_error(codegen_context_t *ctx) {
    return ctx ? ctx->last_error : "无效上下文";
}

/* 其他函数的简化实现占位符 */
int codegen_register_function(codegen_context_t *ctx, const char *name,
                             type_info_t *return_type, uint32_t param_count) {
    /* 简化实现 */
    return 0;
}

function_call_info_t* codegen_resolve_function_call(codegen_context_t *ctx,
                                                   const char *function_name,
                                                   const char *library_name) {
    /* 简化实现：返回占位符 */
    static function_call_info_t placeholder = {FUNC_CALL_USER, NULL, NULL, 0, NULL, NULL};
    return &placeholder;
}

int codegen_generate_function_call(codegen_context_t *ctx, const function_call_info_t *call_info,
                                  uint32_t arg_count) {
    /* 简化实现：生成CALL指令 */
    return bytecode_emit_instr_int(ctx->bc_gen, OP_CALL, 0);
}

/* 标签栈操作 */
int codegen_push_break_label(codegen_context_t *ctx, uint32_t label_id) {
    if (!ctx || ctx->control_flow.label_stack_depth >= 32) return -1;
    
    ctx->control_flow.break_labels[ctx->control_flow.label_stack_depth] = label_id;
    return 0;
}

int codegen_push_continue_label(codegen_context_t *ctx, uint32_t label_id) {
    if (!ctx || ctx->control_flow.label_stack_depth >= 32) return -1;
    
    ctx->control_flow.continue_labels[ctx->control_flow.label_stack_depth] = label_id;
    ctx->control_flow.label_stack_depth++;
    return 0;
}

uint32_t codegen_pop_break_label(codegen_context_t *ctx) {
    if (!ctx || ctx->control_flow.label_stack_depth == 0) return 0;
    
    ctx->control_flow.label_stack_depth--;
    return ctx->control_flow.break_labels[ctx->control_flow.label_stack_depth];
}

uint32_t codegen_pop_continue_label(codegen_context_t *ctx) {
    if (!ctx || ctx->control_flow.label_stack_depth == 0) return 0;
    
    return ctx->control_flow.continue_labels[ctx->control_flow.label_stack_depth];
}

/* 同步支持函数 */
int codegen_generate_sync_instruction(codegen_context_t *ctx, const char *var_name) {
    if (!ctx || !var_name) return -1;
    
    /* 生成同步指令 */
    return bytecode_emit_instr_int(ctx->bc_gen, OP_SYNC_VAR, 0);
}
