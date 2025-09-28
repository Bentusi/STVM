#include "bytecode_generator.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <assert.h>

/* 内部辅助函数声明 */
static int bc_compile_node(bytecode_generator_t* gen, ast_node_t* node);
static int bc_compile_program(bytecode_generator_t* gen, ast_node_t* node);
static int bc_compile_function(bytecode_generator_t* gen, ast_node_t* node);
static int bc_compile_statement(bytecode_generator_t* gen, ast_node_t* node);
static int bc_compile_expression(bytecode_generator_t* gen, ast_node_t* node);
static int bc_compile_assignment(bytecode_generator_t* gen, ast_node_t* node);
static int bc_compile_if_statement(bytecode_generator_t* gen, ast_node_t* node);
static int bc_compile_while_statement(bytecode_generator_t* gen, ast_node_t* node);
static int bc_compile_function_call(bytecode_generator_t* gen, ast_node_t* node);
static int bc_compile_binary_op(bytecode_generator_t* gen, ast_node_t* node);
static int bc_compile_unary_op(bytecode_generator_t* gen, ast_node_t* node);
static int bc_compile_literal(bytecode_generator_t* gen, ast_node_t* node);
static int bc_compile_identifier(bytecode_generator_t* gen, ast_node_t* node);
static int bc_compile_return_statement(bytecode_generator_t* gen, ast_node_t* node);

static int bc_ensure_instruction_capacity(bytecode_generator_t* gen, uint32_t additional);
static uint32_t bc_add_label(bytecode_generator_t* gen, const char* name);
static uint32_t bc_find_label(bytecode_generator_t* gen, const char* name);
static int bc_add_unresolved_jump(bytecode_generator_t* gen, uint32_t instr_index, const char* label);
static int bc_resolve_jumps(bytecode_generator_t* gen);

static constant_pool_t* bc_create_constant_pool(void);
static void bc_destroy_constant_pool(constant_pool_t* pool);
static string_pool_t* bc_create_string_pool(void);
static void bc_destroy_string_pool(string_pool_t* pool);

static int bc_add_error(bytecode_generator_t* gen, uint32_t line, uint32_t column, 
                       const char* message, const char* suggestion);
static int bc_add_warning(bytecode_generator_t* gen, uint32_t line, uint32_t column,
                         const char* message);

static opcode_t bc_get_binary_op_opcode(ast_node_type_t op_type);
static opcode_t bc_get_unary_op_opcode(ast_node_type_t op_type);
static opcode_t bc_get_comparison_op_opcode(ast_node_type_t op_type);

/* 全局操作码到字符串映射表 */
static const char* opcode_names[] = {
    [OP_LOAD_CONST_INT] = "LOAD_CONST_INT",
    [OP_LOAD_CONST_REAL] = "LOAD_CONST_REAL",
    [OP_LOAD_CONST_BOOL] = "LOAD_CONST_BOOL",
    [OP_LOAD_CONST_STRING] = "LOAD_CONST_STRING",
    [OP_LOAD_VAR] = "LOAD_VAR",
    [OP_STORE_VAR] = "STORE_VAR",
    [OP_LOAD_PARAM] = "LOAD_PARAM",
    [OP_STORE_PARAM] = "STORE_PARAM",
    [OP_LOAD_LOCAL] = "LOAD_LOCAL",
    [OP_STORE_LOCAL] = "STORE_LOCAL",
    [OP_ADD] = "ADD",
    [OP_SUB] = "SUB",
    [OP_MUL] = "MUL",
    [OP_DIV] = "DIV",
    [OP_MOD] = "MOD",
    [OP_NEG] = "NEG",
    [OP_ABS] = "ABS",
    [OP_EQ] = "EQ",
    [OP_NE] = "NE",
    [OP_LT] = "LT",
    [OP_LE] = "LE",
    [OP_GT] = "GT",
    [OP_GE] = "GE",
    [OP_AND] = "AND",
    [OP_OR] = "OR",
    [OP_NOT] = "NOT",
    [OP_JMP] = "JMP",
    [OP_JMP_IF] = "JMP_IF",
    [OP_JMP_UNLESS] = "JMP_UNLESS",
    [OP_CALL] = "CALL",
    [OP_RET] = "RET",
    [OP_RET_VALUE] = "RET_VALUE",
    [OP_PUSH] = "PUSH",
    [OP_POP] = "POP",
    [OP_DUP] = "DUP",
    [OP_SWAP] = "SWAP",
    [OP_HALT] = "HALT",
    [OP_NOP] = "NOP",
    [OP_DEBUG_BREAK] = "DEBUG_BREAK",
    [OP_SYNC_CHECKPOINT] = "SYNC_CHECKPOINT",
    [OP_LIB_CALL] = "LIB_CALL"
};

/* 创建字节码生成器 */
bytecode_generator_t* bc_generator_create(void) {
    bytecode_generator_t* gen = calloc(1, sizeof(bytecode_generator_t));
    if (!gen) {
        return NULL;
    }

    gen->instruction_capacity = BC_DEFAULT_INSTRUCTION_CAPACITY;
    gen->instructions = malloc(gen->instruction_capacity * sizeof(instruction_t));
    if (!gen->instructions) {
        free(gen);
        return NULL;
    }

    gen->constant_pool = bc_create_constant_pool();
    gen->string_pool = bc_create_string_pool();
    
    if (!gen->constant_pool || !gen->string_pool) {
        bc_generator_destroy(gen);
        return NULL;
    }

    gen->label_capacity = BC_DEFAULT_LABEL_CAPACITY;
    gen->labels = malloc(gen->label_capacity * sizeof(*gen->labels));
    gen->unresolved_jumps = malloc(gen->label_capacity * sizeof(*gen->unresolved_jumps));
    
    if (!gen->labels || !gen->unresolved_jumps) {
        bc_generator_destroy(gen);
        return NULL;
    }

    gen->errors = calloc(1, sizeof(error_list_t));
    if (!gen->errors) {
        bc_generator_destroy(gen);
        return NULL;
    }

    gen->output_format = BC_OUTPUT_BINARY;
    gen->opt_level = BC_OPT_BASIC;
    gen->generate_debug_info = false;
    gen->enable_sync_support = false;

    return gen;
}

/* 销毁字节码生成器 */
void bc_generator_destroy(bytecode_generator_t* gen) {
    if (!gen) return;

    free(gen->instructions);
    
    if (gen->constant_pool) {
        bc_destroy_constant_pool(gen->constant_pool);
    }
    
    if (gen->string_pool) {
        bc_destroy_string_pool(gen->string_pool);
    }

    // 清理变量表
    variable_entry_t* var = gen->global_vars;
    while (var) {
        variable_entry_t* next = var->next;
        free(var->name);
        if (var->type == TYPE_STRING_ID && var->initial_value.string_value) {
            free(var->initial_value.string_value);
        }
        free(var);
        var = next;
    }

    // 清理函数表
    function_entry_t* func = gen->functions;
    while (func) {
        function_entry_t* next = func->next;
        free(func->name);
        if (func->param_names) {
            for (uint32_t i = 0; i < func->param_count; i++) {
                free(func->param_names[i]);
            }
            free(func->param_names);
        }
        free(func->func_type);
        free(func);
        func = next;
    }

    // 清理标签
    if (gen->labels) {
        for (uint32_t i = 0; i < gen->label_count; i++) {
            free(gen->labels[i].name);
        }
        free(gen->labels);
    }

    // 清理未解析跳转
    if (gen->unresolved_jumps) {
        for (uint32_t i = 0; i < gen->unresolved_jump_count; i++) {
            free(gen->unresolved_jumps[i].label_name);
        }
        free(gen->unresolved_jumps);
    }

    // 清理错误列表
    if (gen->errors) {
        error_entry_t* error = gen->errors->errors;
        while (error) {
            error_entry_t* next = error->next;
            free(error->message);
            free(error->suggestion);
            free(error);
            error = next;
        }
        free(gen->errors);
    }

    // 清理调试信息
    if (gen->debug_info) {
        free(gen->debug_info->pc_to_line);
        if (gen->debug_info->line_to_source) {
            for (uint32_t i = 0; i < gen->debug_info->source_line_count; i++) {
                free(gen->debug_info->line_to_source[i]);
            }
            free(gen->debug_info->line_to_source);
        }
        free(gen->debug_info->source_filename);
        free(gen->debug_info);
    }

    // 清理行号映射
    line_mapping_t* mapping = gen->line_mapping;
    while (mapping) {
        line_mapping_t* next = mapping->next;
        free(mapping);
        mapping = next;
    }

    free(gen);
}

/* 初始化字节码生成器 */
int bc_generator_init(bytecode_generator_t* gen, symbol_table_t* sym_table) {
    if (!gen || !sym_table) {
        return BC_ERROR_INVALID_PARAM;
    }

    gen->symbol_table = sym_table;
    gen->instruction_count = 0;
    gen->label_count = 0;
    gen->unresolved_jump_count = 0;
    gen->label_counter = 0;

    // 初始化统计信息
    memset(&gen->stats, 0, sizeof(gen->stats));

    return BC_SUCCESS;
}

/* 设置输出格式 */
int bc_generator_set_output_format(bytecode_generator_t* gen, output_format_t format) {
    if (!gen) return BC_ERROR_INVALID_PARAM;
    gen->output_format = format;
    return BC_SUCCESS;
}

/* 设置优化级别 */
int bc_generator_set_optimization_level(bytecode_generator_t* gen, optimization_level_t level) {
    if (!gen) return BC_ERROR_INVALID_PARAM;
    gen->opt_level = level;
    return BC_SUCCESS;
}

/* 启用调试信息 */
void bc_generator_enable_debug_info(bytecode_generator_t* gen, bool enable) {
    if (gen) {
        gen->generate_debug_info = enable;
        if (enable && !gen->debug_info) {
            gen->debug_info = calloc(1, sizeof(debug_info_t));
        }
    }
}

/* 启用同步支持 */
void bc_generator_enable_sync_support(bytecode_generator_t* gen, bool enable) {
    if (gen) {
        gen->enable_sync_support = enable;
    }
}

/* 编译程序 */
int bc_generator_compile_program(bytecode_generator_t* gen, ast_node_t* ast) {
    if (!gen || !ast) {
        return BC_ERROR_INVALID_PARAM;
    }

    gen->ast_root = ast;
    
    // 编译AST
    int result = bc_compile_node(gen, ast);
    if (result != BC_SUCCESS) {
        return result;
    }

    // 解析跳转标签
    result = bc_resolve_jumps(gen);
    if (result != BC_SUCCESS) {
        return result;
    }

    // 应用优化
    if (gen->opt_level > BC_OPT_NONE) {
        result = bc_generator_optimize(gen);
        if (result != BC_SUCCESS) {
            return result;
        }
    }

    return BC_SUCCESS;
}

/* 编译AST节点 */
static int bc_compile_node(bytecode_generator_t* gen, ast_node_t* node) {
    if (!node) return BC_SUCCESS;

    switch (node->type) {
        case AST_PROGRAM:
            return bc_compile_program(gen, node);

        case AST_FUNCTION_DECL:
            return bc_compile_function(gen, node);

        case AST_ASSIGNMENT:
            return bc_compile_assignment(gen, node);

        case AST_IF_STMT:
            return bc_compile_if_statement(gen, node);

        case AST_WHILE_STMT:
            return bc_compile_while_statement(gen, node);

        case AST_FUNCTION_CALL:
            return bc_compile_function_call(gen, node);

        case AST_BINARY_OP:
            return bc_compile_binary_op(gen, node);

        case AST_UNARY_OP:
            return bc_compile_unary_op(gen, node);

        case AST_LITERAL:
            return bc_compile_literal(gen, node);

        case AST_IDENTIFIER:
            return bc_compile_identifier(gen, node);

        case AST_RETURN_STMT:
            return bc_compile_return_statement(gen, node);

        case AST_STATEMENT_LIST:
            // 编译语句列表
            return bc_compile_statement(gen, node);
        default:
            // 尝试作为语句编译
            return bc_compile_statement(gen, node);
    }

    return BC_SUCCESS;
}

/* 编译程序节点 */
static int bc_compile_program(bytecode_generator_t* gen, ast_node_t* node) {
    if (!node || node->type != AST_PROGRAM) {
        return BC_ERROR_INVALID_PARAM;
    }

    // 编译程序体
    if (node->data.program.statements) {
        int result = bc_compile_node(gen, node->data.program.statements);
        if (result != BC_SUCCESS) return result;
    }

    // 添加程序结束指令
    return bc_generator_emit_instruction(gen, OP_HALT);
}

/* 编译函数节点 */
static int bc_compile_function(bytecode_generator_t* gen, ast_node_t* node) {
    if (!node || node->type != AST_FUNCTION_DECL) {
        return BC_ERROR_INVALID_PARAM;
    }

    // 创建函数入口标签
    char func_label[256];
    snprintf(func_label, sizeof(func_label), "__func_%s__", node->data.function_decl.name);
    
    // 添加函数到符号表
    function_entry_t* func_entry = malloc(sizeof(function_entry_t));
    if (!func_entry) {
        return BC_ERROR_OUT_OF_MEMORY;
    }

    func_entry->name = strdup(node->data.function_decl.name);
    func_entry->address = gen->instruction_count;
    func_entry->return_type = node->data.function_decl.return_type;
    func_entry->param_count = 0;
    func_entry->param_names = NULL;
    func_entry->local_var_count = 0;
    func_entry->is_builtin = false;
    func_entry->next = gen->functions;
    gen->functions = func_entry;

    // 统计参数个数
    ast_node_t* param = node->data.function_decl.parameters;
    while (param) {
        func_entry->param_count++;
        param = param->next;
    }

    // 分配参数名数组
    if (func_entry->param_count > 0) {
        func_entry->param_names = malloc(func_entry->param_count * sizeof(char*));
        param = node->data.function_decl.parameters;
        for (uint32_t i = 0; i < func_entry->param_count; i++) {
            func_entry->param_names[i] = strdup(param->data.identifier.name);
            param = param->next;
        }
    }

    gen->current_function = func_entry;
    gen->current_local_offset = 0;

    // 编译函数体
    if (node->data.function_decl.statements) {
        int result = bc_compile_node(gen, node->data.function_decl.statements);
        if (result != BC_SUCCESS) return result;
    }

    // 如果函数没有显式返回，添加默认返回
    if (node->data.function_decl.return_type != NULL) {
        // 加载默认返回值
        bc_generator_emit_instruction(gen, OP_LOAD_CONST_INT, 0);
    }
    bc_generator_emit_instruction(gen, OP_RET);

    gen->current_function = NULL;
    gen->stats.functions_compiled++;
    
    return BC_SUCCESS;
}

/* 编译赋值语句 */
static int bc_compile_assignment(bytecode_generator_t* gen, ast_node_t* node) {
    if (!node || node->type != AST_ASSIGNMENT) {
        return BC_ERROR_INVALID_PARAM;
    }

    // 编译右侧表达式
    int result = bc_compile_node(gen, node->data.assignment.value);
    if (result != BC_SUCCESS) return result;

    // 编译左侧标识符（变量存储）
    if (node->data.assignment.target && node->data.assignment.target->type == AST_IDENTIFIER) {
        return bc_generator_emit_instruction(gen, OP_STORE_VAR, 
                                           bc_generator_add_string_constant(gen, node->data.assignment.target->data.identifier.name));
    }

    return BC_ERROR_COMPILE_FAILED;
}

/* 编译IF语句 */
static int bc_compile_if_statement(bytecode_generator_t* gen, ast_node_t* node) {
    if (!node || node->type != AST_IF_STMT) {
        return BC_ERROR_INVALID_PARAM;
    }

    // 编译条件表达式
    int result = bc_compile_node(gen, node->data.if_stmt.condition);
    if (result != BC_SUCCESS) return result;

    // 生成标签
    char else_label[64], end_label[64];
    snprintf(else_label, sizeof(else_label), "_else_%u", gen->label_counter++);
    snprintf(end_label, sizeof(end_label), "_end_if_%u", gen->label_counter++);

    // 条件跳转到else分支
    uint32_t jmp_else_addr = gen->instruction_count;
    bc_generator_emit_instruction(gen, OP_JMP_UNLESS, 0); // 占位符
    bc_add_unresolved_jump(gen, jmp_else_addr, else_label);

    // 编译then分支
    if (node->data.if_stmt.then_stmt) {
        result = bc_compile_node(gen, node->data.if_stmt.then_stmt);
        if (result != BC_SUCCESS) return result;
    }

    // 跳转到结束
    uint32_t jmp_end_addr = gen->instruction_count;
    bc_generator_emit_instruction(gen, OP_JMP, 0); // 占位符
    bc_add_unresolved_jump(gen, jmp_end_addr, end_label);

    // else标签
    bc_add_label(gen, else_label);

    // 编译else分支
    if (node->data.if_stmt.else_stmt) {
        result = bc_compile_node(gen, node->data.if_stmt.else_stmt);
        if (result != BC_SUCCESS) return result;
    }

    // 结束标签
    bc_add_label(gen, end_label);

    return BC_SUCCESS;
}

/* 编译WHILE语句 */
static int bc_compile_while_statement(bytecode_generator_t* gen, ast_node_t* node) {
    if (!node || node->type != AST_WHILE_STMT) {
        return BC_ERROR_INVALID_PARAM;
    }

    // 生成标签
    char loop_label[64], end_label[64];
    snprintf(loop_label, sizeof(loop_label), "_loop_%u", gen->label_counter++);
    snprintf(end_label, sizeof(end_label), "_end_loop_%u", gen->label_counter++);

    // 循环开始标签
    bc_add_label(gen, loop_label);

    // 编译条件表达式
    int result = bc_compile_node(gen, node->data.while_stmt.condition);
    if (result != BC_SUCCESS) return result;

    // 条件跳转到结束
    uint32_t jmp_end_addr = gen->instruction_count;
    bc_generator_emit_instruction(gen, OP_JMP_UNLESS, 0); // 占位符
    bc_add_unresolved_jump(gen, jmp_end_addr, end_label);

    // 编译循环体
    if (node->data.while_stmt.statements) {
        result = bc_compile_node(gen, node->data.while_stmt.statements);
        if (result != BC_SUCCESS) return result;
    }

    // 跳回循环开始
    uint32_t jmp_loop_addr = gen->instruction_count;
    bc_generator_emit_instruction(gen, OP_JMP, 0); // 占位符
    bc_add_unresolved_jump(gen, jmp_loop_addr, loop_label);

    // 结束标签
    bc_add_label(gen, end_label);

    return BC_SUCCESS;
}

/* 编译函数调用 */
static int bc_compile_function_call(bytecode_generator_t* gen, ast_node_t* node) {
    if (!node || node->type != AST_FUNCTION_CALL) {
        return BC_ERROR_INVALID_PARAM;
    }

    // 编译参数（从左到右）
    uint32_t param_count = 0;
    ast_node_t* arg = node->data.function_call.arguments;
    while (arg) {
        if (arg->type == AST_PARAMETER_LIST) {
            int result = bc_compile_node(gen, arg);
            if (result != BC_SUCCESS) return result;
            param_count++;
            arg = arg->next;
        }
    }

    // 生成函数调用指令
    char func_label[256];
    snprintf(func_label, sizeof(func_label), "__func_%s__", node->data.function_call.function_name);
    
    return bc_generator_emit_instruction(gen, OP_CALL, 
                                       bc_generator_add_string_constant(gen, func_label),
                                       param_count);
}

/* 编译二元操作 */
static int bc_compile_binary_op(bytecode_generator_t* gen, ast_node_t* node) {
    if (!node || node->type != AST_BINARY_OP) {
        return BC_ERROR_INVALID_PARAM;
    }

    // 编译左操作数
    int result = bc_compile_node(gen, node->data.binary_op.left);
    if (result != BC_SUCCESS) return result;

    // 编译右操作数
    result = bc_compile_node(gen, node->data.binary_op.right);
    if (result != BC_SUCCESS) return result;

    // 生成操作指令
    opcode_t opcode = bc_get_binary_op_opcode(node->type);
    if (opcode == OP_COUNT) {
        return BC_ERROR_COMPILE_FAILED;
    }

    return bc_generator_emit_instruction(gen, opcode);
}

/* 编译一元操作 */
static int bc_compile_unary_op(bytecode_generator_t* gen, ast_node_t* node) {
    if (!node || node->type != AST_UNARY_OP) {
        return BC_ERROR_INVALID_PARAM;
    }

    // 编译操作数
    int result = bc_compile_node(gen, node->data.unary_op.operand);
    if (result != BC_SUCCESS) return result;

    // 生成操作指令
    opcode_t opcode = bc_get_unary_op_opcode(node->type);
    if (opcode >= OP_COUNT) {
        return BC_ERROR_COMPILE_FAILED;
    }

    return bc_generator_emit_instruction(gen, opcode);
}

/* 编译字面量 */
static int bc_compile_literal(bytecode_generator_t* gen, ast_node_t* node) {
    if (!node || node->type != AST_LITERAL) {
        return BC_ERROR_INVALID_PARAM;
    }

    switch (node->data_type->base_type) {
        case TYPE_INT_ID:
            return bc_generator_emit_instruction(gen, OP_LOAD_CONST_INT, 
                                               bc_generator_add_int_constant(gen, node->data.var_item.init_value->data.literal.value.int_val));

        case TYPE_REAL_ID:
            return bc_generator_emit_instruction(gen, OP_LOAD_CONST_REAL,
                                               bc_generator_add_real_constant(gen, node->data.var_item.init_value->data.literal.value.real_val));

        case TYPE_BOOL_ID:
            return bc_generator_emit_instruction(gen, OP_LOAD_CONST_BOOL,
                                               bc_generator_add_bool_constant(gen, node->data.var_item.init_value->data.literal.value.bool_val));

        case TYPE_STRING_ID:
            return bc_generator_emit_instruction(gen, OP_LOAD_CONST_STRING,
                                               bc_generator_add_string_constant(gen, node->data.var_item.init_value->data.literal.value.string_val));

        default:
            return BC_ERROR_TYPE_MISMATCH;
    }
}

/* 编译标识符 */
static int bc_compile_identifier(bytecode_generator_t* gen, ast_node_t* node) {
    if (!node || node->type != AST_IDENTIFIER) {
        return BC_ERROR_INVALID_PARAM;
    }

    // 生成加载变量指令
    return bc_generator_emit_instruction(gen, OP_LOAD_VAR,
                                       bc_generator_add_string_constant(gen, node->data.var_item.init_value->data.literal.value.string_val));
}

/* 编译返回语句 */
static int bc_compile_return_statement(bytecode_generator_t* gen, ast_node_t* node) {
    if (!node || node->type != AST_RETURN_STMT) {
        return BC_ERROR_INVALID_PARAM;
    }

    // 编译返回值表达式
    if (node->data.return_stmt.return_value) {
        int result = bc_compile_node(gen, node->data.return_stmt.return_value);
        if (result != BC_SUCCESS) return result;
        return bc_generator_emit_instruction(gen, OP_RET_VALUE);
    } else {
        return bc_generator_emit_instruction(gen, OP_RET);
    }
}

/* 编译语句 */
static int bc_compile_statement(bytecode_generator_t* gen, ast_node_t* node) {
    if (!node) return BC_SUCCESS;

    // 根据节点类型分发编译
    switch (node->type) {
        case AST_ASSIGNMENT:
            return bc_compile_assignment(gen, node);
        case AST_IF_STMT:
            return bc_compile_if_statement(gen, node);
        case AST_WHILE_STMT:
            return bc_compile_while_statement(gen, node);
        case AST_FUNCTION_CALL:
            return bc_compile_function_call(gen, node);
        case AST_RETURN_STMT:
            return bc_compile_return_statement(gen, node);
        default:
            return bc_compile_expression(gen, node);
    }
}

/* 编译表达式 */
static int bc_compile_expression(bytecode_generator_t* gen, ast_node_t* node) {
    if (!node) return BC_SUCCESS;

    switch (node->type) {
        case AST_BINARY_OP:
            return bc_compile_binary_op(gen, node);
        case AST_UNARY_OP:
            return bc_compile_unary_op(gen, node);
        case AST_LITERAL:
            return bc_compile_literal(gen, node);
        case AST_IDENTIFIER:
            return bc_compile_identifier(gen, node);
        case AST_FUNCTION_CALL:
            return bc_compile_function_call(gen, node);
        default:
            return BC_ERROR_COMPILE_FAILED;
    }
}

/* 生成指令 */
int bc_generator_emit_instruction(bytecode_generator_t* gen, opcode_t opcode, ...) {
    if (!gen) return BC_ERROR_INVALID_PARAM;

    // 确保容量足够
    if (bc_ensure_instruction_capacity(gen, 1) != BC_SUCCESS) {
        return BC_ERROR_OUT_OF_MEMORY;
    }

    instruction_t* instr = &gen->instructions[gen->instruction_count];
    instr->opcode = opcode;
    instr->line_number = 0; // TODO: 从AST节点获取
    instr->column_number = 0;

    // 处理操作数
    va_list args;
    va_start(args, opcode);

    switch (opcode) {
        case OP_LOAD_CONST_INT:
        case OP_LOAD_CONST_REAL:
        case OP_LOAD_CONST_BOOL:
        case OP_LOAD_CONST_STRING:
        case OP_LOAD_VAR:
        case OP_STORE_VAR:
        case OP_JMP:
        case OP_JMP_IF:
        case OP_JMP_UNLESS:
            instr->operand1 = va_arg(args, uint32_t);
            break;

        case OP_CALL:
            instr->operand1 = va_arg(args, uint32_t); // 函数名索引
            instr->operand2 = va_arg(args, uint32_t); // 参数个数
            break;

        default:
            // 无操作数指令
            break;
    }

    va_end(args);

    gen->instruction_count++;
    return BC_SUCCESS;
}

/* 添加整数常量 */
uint32_t bc_generator_add_int_constant(bytecode_generator_t* gen, int64_t value) {
    if (!gen || !gen->constant_pool) return 0;

    // 查找是否已存在
    constant_entry_t* entry = gen->constant_pool->entries;
    while (entry) {
        if (entry->type == CONST_INT && entry->value.int_value == value) {
            return entry->index;
        }
        entry = entry->next;
    }

    // 创建新条目
    entry = malloc(sizeof(constant_entry_t));
    if (!entry) return 0;

    entry->type = CONST_INT;
    entry->value.int_value = value;
    entry->index = gen->constant_pool->count++;
    entry->next = gen->constant_pool->entries;
    gen->constant_pool->entries = entry;

    gen->stats.constants_generated++;
    return entry->index;
}

/* 添加实数常量 */
uint32_t bc_generator_add_real_constant(bytecode_generator_t* gen, double value) {
    if (!gen || !gen->constant_pool) return 0;

    // 查找是否已存在
    constant_entry_t* entry = gen->constant_pool->entries;
    while (entry) {
        if (entry->type == CONST_REAL && entry->value.real_value == value) {
            return entry->index;
        }
        entry = entry->next;
    }

    // 创建新条目
    entry = malloc(sizeof(constant_entry_t));
    if (!entry) return 0;

    entry->type = CONST_REAL;
    entry->value.real_value = value;
    entry->index = gen->constant_pool->count++;
    entry->next = gen->constant_pool->entries;
    gen->constant_pool->entries = entry;

    gen->stats.constants_generated++;
    return entry->index;
}

/* 添加布尔常量 */
uint32_t bc_generator_add_bool_constant(bytecode_generator_t* gen, bool value) {
    if (!gen || !gen->constant_pool) return 0;

    // 查找是否已存在
    constant_entry_t* entry = gen->constant_pool->entries;
    while (entry) {
        if (entry->type == CONST_BOOL && entry->value.bool_value == value) {
            return entry->index;
        }
        entry = entry->next;
    }

    // 创建新条目
    entry = malloc(sizeof(constant_entry_t));
    if (!entry) return 0;

    entry->type = CONST_BOOL;
    entry->value.bool_value = value;
    entry->index = gen->constant_pool->count++;
    entry->next = gen->constant_pool->entries;
    gen->constant_pool->entries = entry;

    gen->stats.constants_generated++;
    return entry->index;
}

/* 添加字符串常量 */
uint32_t bc_generator_add_string_constant(bytecode_generator_t* gen, const char* value) {
    if (!gen || !gen->string_pool || !value) return 0;

    // 查找是否已存在
    for (uint32_t i = 0; i < gen->string_pool->count; i++) {
        if (strcmp(gen->string_pool->strings[i], value) == 0) {
            return i;
        }
    }

    // 扩展容量
    if (gen->string_pool->count >= gen->string_pool->capacity) {
        uint32_t new_capacity = gen->string_pool->capacity * 2;
        char** new_strings = realloc(gen->string_pool->strings, 
                                   new_capacity * sizeof(char*));
        if (!new_strings) return 0;
        
        gen->string_pool->strings = new_strings;
        gen->string_pool->capacity = new_capacity;
    }

    // 添加新字符串
    gen->string_pool->strings[gen->string_pool->count] = strdup(value);
    if (!gen->string_pool->strings[gen->string_pool->count]) return 0;

    return gen->string_pool->count++;
}

/* 保存到文件 */
int bc_generator_save_to_file(bytecode_generator_t* gen, const char* filename) {
    if (!gen || !filename) return BC_ERROR_INVALID_PARAM;

    FILE* file = fopen(filename, "wb");
    if (!file) return BC_ERROR_FILE_IO;

    // 写文件头
    bytecode_file_header_t header;
    header.magic = BC_MAGIC_NUMBER;
    header.version_major = BC_VERSION_MAJOR;
    header.version_minor = BC_VERSION_MINOR;
    header.flags = 0;
    header.timestamp = (uint32_t)time(NULL);
    header.checksum = 0; // TODO: 计算校验和
    header.header_size = sizeof(header);
    header.section_count = 3; // 指令段、常量段、字符串段

    if (gen->enable_sync_support) {
        header.flags |= 0x01; // 同步支持标志
    }
    
    if (gen->generate_debug_info) {
        header.flags |= 0x02; // 调试信息标志
        header.section_count++;
    }

    fwrite(&header, sizeof(header), 1, file);

    // 写段头
    bytecode_section_header_t sections[8];
    uint32_t offset = sizeof(header) + header.section_count * sizeof(bytecode_section_header_t);
    
    // 指令段
    sections[0].type = BC_SECTION_INSTRUCTIONS;
    sections[0].offset = offset;
    sections[0].size = gen->instruction_count * sizeof(instruction_t);
    sections[0].flags = 0;
    offset += sections[0].size;

    // 常量段
    sections[1].type = BC_SECTION_CONSTANTS;
    sections[1].offset = offset;
    sections[1].size = gen->constant_pool->count * sizeof(constant_entry_t);
    sections[1].flags = 0;
    offset += sections[1].size;

    // 字符串段
    sections[2].type = BC_SECTION_STRINGS;
    sections[2].offset = offset;
    sections[2].size = 0;
    for (uint32_t i = 0; i < gen->string_pool->count; i++) {
        sections[2].size += strlen(gen->string_pool->strings[i]) + 1;
    }
    sections[2].flags = 0;

    fwrite(sections, sizeof(bytecode_section_header_t), header.section_count, file);

    // 写指令段
    fwrite(gen->instructions, sizeof(instruction_t), gen->instruction_count, file);

    // 写常量段
    constant_entry_t* entry = gen->constant_pool->entries;
    while (entry) {
        fwrite(entry, sizeof(constant_entry_t), 1, file);
        entry = entry->next;
    }

    // 写字符串段
    for (uint32_t i = 0; i < gen->string_pool->count; i++) {
        size_t len = strlen(gen->string_pool->strings[i]) + 1;
        fwrite(gen->string_pool->strings[i], 1, len, file);
    }

    fclose(file);
    return BC_SUCCESS;
}

/* 保存到内存 */
int bc_generator_save_to_memory(bytecode_generator_t* gen, bytecode_file_t* bc_file) {
    if (!gen || !bc_file) return BC_ERROR_INVALID_PARAM;

    memset(bc_file, 0, sizeof(bytecode_file_t));

    // 设置文件头
    bc_file->header.magic = BC_MAGIC_NUMBER;
    bc_file->header.version_major = BC_VERSION_MAJOR;
    bc_file->header.version_minor = BC_VERSION_MINOR;
    bc_file->header.timestamp = (uint32_t)time(NULL);

    // 复制指令
    bc_file->instruction_count = gen->instruction_count;
    bc_file->instructions = malloc(gen->instruction_count * sizeof(instruction_t));
    if (!bc_file->instructions) return BC_ERROR_OUT_OF_MEMORY;
    
    memcpy(bc_file->instructions, gen->instructions, 
           gen->instruction_count * sizeof(instruction_t));

    // 复制常量池
    bc_file->constants = malloc(sizeof(constant_pool_t));
    if (!bc_file->constants) {
        free(bc_file->instructions);
        return BC_ERROR_OUT_OF_MEMORY;
    }
    memcpy(bc_file->constants, gen->constant_pool, sizeof(constant_pool_t));

    // 复制字符串池
    bc_file->strings = malloc(sizeof(string_pool_t));
    if (!bc_file->strings) {
        free(bc_file->instructions);
        free(bc_file->constants);
        return BC_ERROR_OUT_OF_MEMORY;
    }
    memcpy(bc_file->strings, gen->string_pool, sizeof(string_pool_t));

    bc_file->is_loaded = true;
    return BC_SUCCESS;
}

/* 反汇编 */
int bc_file_disassemble(const bytecode_file_t* bc_file, FILE* output) {
    if (!bc_file || !output) return BC_ERROR_INVALID_PARAM;

    fprintf(output, "=== 字节码反汇编 ===\n");
    fprintf(output, "版本: %d.%d\n", bc_file->header.version_major, bc_file->header.version_minor);
    fprintf(output, "指令数: %u\n\n", bc_file->instruction_count);

    for (uint32_t i = 0; i < bc_file->instruction_count; i++) {
        const instruction_t* instr = &bc_file->instructions[i];
        fprintf(output, "%04u: %-16s", i, bc_opcode_to_string(instr->opcode));

        switch (instr->opcode) {
            case OP_LOAD_CONST_INT:
            case OP_LOAD_CONST_REAL:
            case OP_LOAD_CONST_BOOL:
            case OP_LOAD_CONST_STRING:
            case OP_LOAD_VAR:
            case OP_STORE_VAR:
                fprintf(output, " %u", instr->operand1);
                break;
            case OP_CALL:
                fprintf(output, " %u, %u", instr->operand1, instr->operand2);
                break;
            case OP_JMP:
            case OP_JMP_IF:
            case OP_JMP_UNLESS:
                fprintf(output, " -> %u", instr->operand1);
                break;
            default:
                break;
        }
        fprintf(output, "\n");
    }

    return BC_SUCCESS;
}

/* 释放字节码文件 */
void bc_file_free(bytecode_file_t* bc_file) {
    if (!bc_file) return;

    free(bc_file->instructions);
    
    if (bc_file->constants) {
        bc_destroy_constant_pool(bc_file->constants);
    }
    
    if (bc_file->strings) {
        bc_destroy_string_pool(bc_file->strings);
    }
    
    free(bc_file->filename);
    memset(bc_file, 0, sizeof(bytecode_file_t));
}

/* 验证字节码文件 */
bool bc_file_verify(const bytecode_file_t* bc_file) {
    if (!bc_file) return false;
    
    // 检查魔数
    if (bc_file->header.magic != BC_MAGIC_NUMBER) {
        return false;
    }
    
    // 检查版本
    if (bc_file->header.version_major != BC_VERSION_MAJOR) {
        return false;
    }
    
    return true;
}

/* 操作码转字符串 */
const char* bc_opcode_to_string(opcode_t opcode) {
    if (opcode >= 0 && opcode < OP_COUNT) {
        return opcode_names[opcode];
    }
    return "UNKNOWN";
}

/* 获取最后的错误 */
const char* bc_generator_get_last_error(bytecode_generator_t* gen) {
    if (!gen || !gen->errors || !gen->errors->errors) {
        return "无错误信息";
    }
    return gen->errors->errors->message;
}

/* 打印统计信息 */
void bc_generator_print_statistics(bytecode_generator_t* gen) {
    if (!gen) return;

    printf("=== 字节码生成统计 ===\n");
    printf("指令数: %u\n", gen->instruction_count);
    printf("函数数: %u\n", gen->stats.functions_compiled);
    printf("常量数: %u\n", gen->stats.constants_generated);
    printf("变量数: %u\n", gen->stats.variables_processed);
    printf("优化次数: %u\n", gen->stats.optimizations_applied);
    printf("======================\n");
}

/* 优化器 */
int bc_generator_optimize(bytecode_generator_t* gen) {
    if (!gen) return BC_ERROR_INVALID_PARAM;

    int optimizations = 0;

    if (gen->opt_level >= BC_OPT_BASIC) {
        // 常量折叠
        if (bc_generator_apply_constant_folding(gen) == BC_SUCCESS) {
            optimizations++;
        }
    }

    if (gen->opt_level >= BC_OPT_STANDARD) {
        // 死代码消除
        if (bc_generator_apply_dead_code_elimination(gen) == BC_SUCCESS) {
            optimizations++;
        }
        
        // 跳转优化
        if (bc_generator_apply_jump_optimization(gen) == BC_SUCCESS) {
            optimizations++;
        }
    }

    gen->stats.optimizations_applied += optimizations;
    return BC_SUCCESS;
}

/* 简单的常量折叠实现 */
int bc_generator_apply_constant_folding(bytecode_generator_t* gen) {
    // TODO: 实现常量折叠优化
    return BC_SUCCESS;
}

/* 简单的死代码消除实现 */
int bc_generator_apply_dead_code_elimination(bytecode_generator_t* gen) {
    // TODO: 实现死代码消除优化  
    return BC_SUCCESS;
}

/* 简单的跳转优化实现 */
int bc_generator_apply_jump_optimization(bytecode_generator_t* gen) {
    // TODO: 实现跳转优化
    return BC_SUCCESS;
}

/* === 内部辅助函数实现 === */

/* 确保指令容量 */
static int bc_ensure_instruction_capacity(bytecode_generator_t* gen, uint32_t additional) {
    if (gen->instruction_count + additional <= gen->instruction_capacity) {
        return BC_SUCCESS;
    }

    uint32_t new_capacity = gen->instruction_capacity * 2;
    while (new_capacity < gen->instruction_count + additional) {
        new_capacity *= 2;
    }

    instruction_t* new_instructions = realloc(gen->instructions, 
                                            new_capacity * sizeof(instruction_t));
    if (!new_instructions) {
        return BC_ERROR_OUT_OF_MEMORY;
    }

    gen->instructions = new_instructions;
    gen->instruction_capacity = new_capacity;
    return BC_SUCCESS;
}

/* 添加标签 */
static uint32_t bc_add_label(bytecode_generator_t* gen, const char* name) {
    if (!gen || !name) return 0;

    // 扩展容量
    if (gen->label_count >= gen->label_capacity) {
        gen->label_capacity *= 2;
        gen->labels = realloc(gen->labels, gen->label_capacity * sizeof(*gen->labels));
        if (!gen->labels) return 0;
    }

    gen->labels[gen->label_count].name = strdup(name);
    gen->labels[gen->label_count].address = gen->instruction_count;
    gen->labels[gen->label_count].resolved = true;
    
    return gen->label_count++;
}

/* 查找标签 */
static uint32_t bc_find_label(bytecode_generator_t* gen, const char* name) {
    if (!gen || !name) return 0;

    for (uint32_t i = 0; i < gen->label_count; i++) {
        if (gen->labels[i].name && strcmp(gen->labels[i].name, name) == 0) {
            return i;
        }
    }
    return 0;
}

/* 添加未解析的跳转 */
static int bc_add_unresolved_jump(bytecode_generator_t* gen, uint32_t instr_index, const char* label) {
    if (!gen || !label) return BC_ERROR_INVALID_PARAM;

    // 扩展容量
    if (gen->unresolved_jump_count >= gen->unresolved_jump_capacity) {
        gen->unresolved_jump_capacity *= 2;
        gen->unresolved_jumps = realloc(gen->unresolved_jumps, 
                                      gen->unresolved_jump_capacity * sizeof(*gen->unresolved_jumps));
        if (!gen->unresolved_jumps) return BC_ERROR_OUT_OF_MEMORY;
    }

    gen->unresolved_jumps[gen->unresolved_jump_count].instruction_index = instr_index;
    gen->unresolved_jumps[gen->unresolved_jump_count].label_name = strdup(label);
    gen->unresolved_jump_count++;

    return BC_SUCCESS;
}

/* 解析跳转 */
static int bc_resolve_jumps(bytecode_generator_t* gen) {
    if (!gen) return BC_ERROR_INVALID_PARAM;

    for (uint32_t i = 0; i < gen->unresolved_jump_count; i++) {
        uint32_t instr_index = gen->unresolved_jumps[i].instruction_index;
        const char* label_name = gen->unresolved_jumps[i].label_name;

        // 查找标签
        uint32_t label_index = bc_find_label(gen, label_name);
        if (label_index < gen->label_count) {
            // 更新跳转目标
            gen->instructions[instr_index].operand1 = gen->labels[label_index].address;
        } else {
            // 标签未找到
            return BC_ERROR_SYMBOL_NOT_FOUND;
        }
    }

    return BC_SUCCESS;
}

/* 创建常量池 */
static constant_pool_t* bc_create_constant_pool(void) {
    constant_pool_t* pool = calloc(1, sizeof(constant_pool_t));
    if (!pool) return NULL;
    
    pool->capacity = BC_DEFAULT_CONSTANT_CAPACITY;
    return pool;
}

/* 销毁常量池 */
static void bc_destroy_constant_pool(constant_pool_t* pool) {
    if (!pool) return;

    constant_entry_t* entry = pool->entries;
    while (entry) {
        constant_entry_t* next = entry->next;
        if (entry->type == CONST_STRING && entry->value.string_value) {
            free(entry->value.string_value);
        }
        free(entry);
        entry = next;
    }
    free(pool);
}

/* 创建字符串池 */
static string_pool_t* bc_create_string_pool(void) {
    string_pool_t* pool = calloc(1, sizeof(string_pool_t));
    if (!pool) return NULL;
    
    pool->capacity = 64;
    pool->strings = malloc(pool->capacity * sizeof(char*));
    if (!pool->strings) {
        free(pool);
        return NULL;
    }
    
    return pool;
}

/* 销毁字符串池 */
static void bc_destroy_string_pool(string_pool_t* pool) {
    if (!pool) return;

    for (uint32_t i = 0; i < pool->count; i++) {
        free(pool->strings[i]);
    }
    free(pool->strings);
    free(pool);
}

/* 获取二元操作的操作码 */
static opcode_t bc_get_binary_op_opcode(ast_node_type_t op_type) {
    switch (op_type) {
        case OP_ADD: return OP_ADD;
        case OP_SUB: return OP_SUB;
        case OP_MUL: return OP_MUL;
        case OP_DIV: return OP_DIV;
        case OP_MOD: return OP_MOD;
        case OP_EQ: return OP_EQ;
        case OP_NE: return OP_NE;
        case OP_LT: return OP_LT;
        case OP_LE: return OP_LE;
        case OP_GT: return OP_GT;
        case OP_GE: return OP_GE;
        case OP_AND: return OP_AND;
        case OP_OR: return OP_OR;
        default: return OP_COUNT;
    }
}

/* 获取一元操作的操作码 */
static opcode_t bc_get_unary_op_opcode(ast_node_type_t op_type) {
    switch (op_type) {
        case OP_NEG: return OP_NEG;
        case OP_NOT: return OP_NOT;
        default: return OP_COUNT;
    }
}

/* 添加错误 */
static int bc_add_error(bytecode_generator_t* gen, uint32_t line, uint32_t column, 
                       const char* message, const char* suggestion) {
    if (!gen || !gen->errors || !message) return BC_ERROR_INVALID_PARAM;

    error_entry_t* error = malloc(sizeof(error_entry_t));
    if (!error) return BC_ERROR_OUT_OF_MEMORY;

    error->level = ERROR_LEVEL_ERROR;
    error->line = line;
    error->column = column;
    error->message = strdup(message);
    error->suggestion = suggestion ? strdup(suggestion) : NULL;
    error->next = gen->errors->errors;
    gen->errors->errors = error;
    gen->errors->error_count++;

    return BC_SUCCESS;
}

/* 加载到虚拟机 */
int bc_generator_load_to_vm(bytecode_generator_t* gen, void* vm) {
    if (!gen || !vm) return BC_ERROR_INVALID_PARAM;
    
    // TODO: 实现VM加载接口
    return BC_SUCCESS;
}

/* 其他未实现的接口 */
int bc_file_load(bytecode_file_t* bc_file, const char* filename) {
    // TODO: 实现文件加载
    return BC_ERROR_INVALID_FORMAT;
}

int bc_file_save(const bytecode_file_t* bc_file, const char* filename) {
    // TODO: 实现文件保存
    return BC_ERROR_FILE_IO;
}
