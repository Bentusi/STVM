/*
 * IEC61131 ST语言类型检查器实现
 * 支持完整的语义分析、类型推断和错误报告
 */

#include "type_checker.h"
#include "mmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* 内部辅助函数声明 */
static int type_checker_check_node_recursive(type_checker_context_t *ctx, ast_node_t *node);
static bool type_checker_is_assignable(type_checker_context_t *ctx, type_info_t *from, type_info_t *to);
static void type_checker_add_message(type_checker_context_t *ctx, type_severity_t severity,
                                    type_check_error_t error_code, source_location_t *location,
                                    const char *format, va_list args);
static type_inference_result_t* type_checker_create_inference_result(type_info_t *type);
static void type_checker_destroy_inference_result(type_inference_result_t *result);

/* ========== 类型检查器管理函数 ========== */

/* 创建类型检查器上下文 */
type_checker_context_t* type_checker_create(void) {
    type_checker_context_t *ctx = (type_checker_context_t*)mmgr_alloc_general(sizeof(type_checker_context_t));
    if (!ctx) {
        return NULL;
    }
    
    memset(ctx, 0, sizeof(type_checker_context_t));
    
    /* 设置默认配置 */
    ctx->options.strict_mode = true;
    ctx->options.warnings_as_errors = false;
    ctx->options.check_unused_vars = true;
    ctx->options.check_uninitialized = true;
    ctx->options.allow_implicit_conv = true;
    
    /* 分配内存 */
    //ctx->builtin_types = (type_info_t**)mmgr_alloc_general(sizeof(type_info_t*) * 16);
    ctx->compatibility_table = (type_compatibility_t*)mmgr_alloc_general(
        sizeof(type_compatibility_t) * 64);
    ctx->function_signatures = (function_signature_t*)mmgr_alloc_general(
        sizeof(function_signature_t) * 128);
    
    if (!ctx->compatibility_table || !ctx->function_signatures) {
        return NULL;
    }
    
    return ctx;
}

/* 销毁类型检查器上下文 */
void type_checker_destroy(type_checker_context_t *ctx) {
    if (!ctx) return;
    
    /* 清理消息链表 */
    type_checker_clear_messages(ctx);
    
    /* 清理函数签名 */
    for (uint32_t i = 0; i < ctx->function_count; i++) {
        type_checker_destroy_function_signature(&ctx->function_signatures[i]);
    }
    
    /* 内存由MMGR管理，不需要显式释放 */
}

/* 初始化类型检查器 */
int type_checker_init(type_checker_context_t *ctx, symbol_table_manager_t *sym_mgr) {
    if (!ctx) {
        return -1;
    }
    
    ctx->sym_mgr = sym_mgr;
    
    /* 初始化内置类型 */
    if (type_checker_init_builtin_types(ctx) != 0) {
        return -1;
    }
    
    /* 注册内置函数 */
    if (type_checker_register_builtin_functions(ctx) != 0) {
        return -1;
    }
    
    return 0;
}

/* ========== 配置管理 ========== */

int type_checker_set_strict_mode(type_checker_context_t *ctx, bool enabled) {
    if (!ctx) return -1;
    ctx->options.strict_mode = enabled;
    return 0;
}

int type_checker_set_warnings_as_errors(type_checker_context_t *ctx, bool enabled) {
    if (!ctx) return -1;
    ctx->options.warnings_as_errors = enabled;
    return 0;
}

int type_checker_enable_unused_check(type_checker_context_t *ctx, bool enabled) {
    if (!ctx) return -1;
    ctx->options.check_unused_vars = enabled;
    return 0;
}

int type_checker_allow_implicit_conversion(type_checker_context_t *ctx, bool enabled) {
    if (!ctx) return -1;
    ctx->options.allow_implicit_conv = enabled;
    return 0;
}

/* ========== 顶层检查函数 ========== */

/* 检查程序 */
int type_checker_check_program(type_checker_context_t *ctx, ast_node_t *program) {
    if (!ctx || !program) {
        return -1;
    }
    
    if (program->type != AST_PROGRAM) {
        type_checker_add_error(ctx, TYPE_CHECK_MEMORY_ERROR, program->loc,
                              "Expected program node");
        return -1;
    }
    
    /* 进入程序作用域 */
    if (type_checker_enter_scope(ctx, program->data.program.name) != 0) {
        return -1;
    }
    
    ctx->current_context.scope_depth = 0;
    ctx->current_context.in_function = false;
    
    /* 检查声明列表 */
    if (program->data.program.declarations) {
        if (type_checker_check_declarations(ctx, program->data.program.declarations) != 0) {
            type_checker_exit_scope(ctx);
            return -1;
        }
    }
    
    /* 检查语句列表 */
    if (program->data.program.statements) {
        if (type_checker_check_statements(ctx, program->data.program.statements) != 0) {
            type_checker_exit_scope(ctx);
            return -1;
        }
    }
    
    /* 检查未使用变量 */
    if (ctx->options.check_unused_vars) {
        type_checker_check_unused_variables(ctx);
    }
    
    /* 退出程序作用域 */
    type_checker_exit_scope(ctx);
    
    return ctx->error_count > 0 ? -1 : 0;
}

/* 检查声明列表 */
int type_checker_check_declarations(type_checker_context_t *ctx, ast_node_t *decl_list) {
    if (!ctx || !decl_list) {
        return 0;
    }
    
    ast_node_t *current = decl_list;
    while (current) {
        if (type_checker_check_node_recursive(ctx, current) != 0) {
            return -1;
        }
        current = current->next;
    }
    
    return 0;
}

/* 检查语句列表 */
int type_checker_check_statements(type_checker_context_t *ctx, ast_node_t *stmt_list) {
    if (!ctx || !stmt_list) {
        return 0;
    }
    
    ast_node_t *current = stmt_list;
    while (current) {
        if (type_checker_check_node_recursive(ctx, current) != 0) {
            return -1;
        }
        current = current->next;
    }
    
    return 0;
}

/* 检查AST节点 */
int type_checker_check_node(type_checker_context_t *ctx, ast_node_t *node) {
    return type_checker_check_node_recursive(ctx, node);
}

/* 递归检查AST节点 */
static int type_checker_check_node_recursive(type_checker_context_t *ctx, ast_node_t *node) {
    if (!ctx || !node) {
        return 0;
    }
    
    ctx->statistics.nodes_checked++;
    
    switch (node->type) {
        case AST_VAR_DECL:
            return type_checker_check_var_declaration(ctx, node);
            
        case AST_FUNCTION_DECL:
            return type_checker_check_function_declaration(ctx, node);
            
        case AST_ASSIGNMENT:
            return type_checker_check_assignment(ctx, node);
            
        case AST_IF_STMT:
            return type_checker_check_if_statement(ctx, node);
            
        case AST_FOR_STMT:
            return type_checker_check_for_statement(ctx, node);
            
        case AST_WHILE_STMT:
            return type_checker_check_while_statement(ctx, node);
            
        case AST_RETURN_STMT:
            return type_checker_check_return_statement(ctx, node);
            
        case AST_BINARY_OP:
        case AST_UNARY_OP:
        case AST_LITERAL:
        case AST_IDENTIFIER:
            /* 表达式节点通过推断来检查 */
            {
                type_inference_result_t *result = type_checker_infer_expression(ctx, node);
                if (!result) {
                    return -1;
                }
                
                /* 设置节点类型信息 */
                node->data_type = result->inferred_type;
                type_checker_destroy_inference_result(result);
                return 0;
            }
            
        case AST_DECLARATION_LIST:
        case AST_STATEMENT_LIST:
            /* 列表节点：递归检查所有子节点 */
            {
                ast_node_t *current = node;
                while (current) {
                    if (type_checker_check_node_recursive(ctx, current) != 0) {
                        return -1;
                    }
                    current = current->next;
                }
                return 0;
            }
            
        default:
            type_checker_add_error(ctx, TYPE_CHECK_MEMORY_ERROR, node->loc,
                                  "Unknown AST node type: %d", node->type);
            return -1;
    }
}

/* ========== 声明检查函数 ========== */

/* 检查变量声明 */
int type_checker_check_var_declaration(type_checker_context_t *ctx, ast_node_t *var_decl) {
    if (!ctx || !var_decl || var_decl->type != AST_VAR_DECL) {
        return -1;
    }
    
    /* 检查变量列表 */
    ast_node_t *var_item = var_decl->data.var_decl.var_list;
    while (var_item) {
        if (type_checker_check_var_item(ctx, var_item) != 0) {
            return -1;
        }
        var_item = var_item->next;
    }
    
    return 0;
}

/* 检查变量项 */
int type_checker_check_var_item(type_checker_context_t *ctx, ast_node_t *var_item) {
    if (!ctx || !var_item || var_item->type != AST_VAR_ITEM) {
        return -1;
    }
    
    const char *var_name = var_item->data.var_item.name;
    type_info_t *var_type = var_item->data.var_item.type;
    
    /* 检查是否重复定义 */
    symbol_t *existing = type_checker_resolve_symbol(ctx, var_name);
    if (existing && existing->scope_level == ctx->current_context.scope_depth) {
        type_checker_add_error(ctx, TYPE_CHECK_REDEFINED_SYMBOL, var_item->loc,
                              "Variable '%s' already defined", var_name);
        return -1;
    }
    
    /* 注册符号 */
    if (type_checker_register_symbol(ctx, var_name, var_type, SYMBOL_VAR) != 0) {
        return -1;
    }
    
    /* 检查初始值（如果有） */
    if (var_item->data.var_item.init_value) {
        type_inference_result_t *init_result = type_checker_infer_expression(
            ctx, var_item->data.var_item.init_value);
        
        if (!init_result) {
            return -1;
        }
        
        /* 检查类型兼容性 */
        if (!type_checker_is_assignable(ctx, init_result->inferred_type, var_type)) {
            type_checker_add_error(ctx, TYPE_CHECK_TYPE_MISMATCH, var_item->loc,
                                  "Cannot initialize variable '%s' of type '%s' with value of type '%s'",
                                  var_name, type_checker_type_to_string(var_type),
                                  type_checker_type_to_string(init_result->inferred_type));
            type_checker_destroy_inference_result(init_result);
            return -1;
        }
        
        type_checker_destroy_inference_result(init_result);
    }
    
    return 0;
}

/* 检查函数声明 */
int type_checker_check_function_declaration(type_checker_context_t *ctx, ast_node_t *func_decl) {
    if (!ctx || !func_decl || func_decl->type != AST_FUNCTION_DECL) {
        return -1;
    }
    
    const char *func_name = func_decl->data.function_decl.name;
    type_info_t *return_type = func_decl->data.function_decl.return_type;
    
    /* 检查是否重复定义 */
    function_signature_t *existing = type_checker_resolve_function(ctx, func_name);
    if (existing) {
        type_checker_add_error(ctx, TYPE_CHECK_REDEFINED_SYMBOL, func_decl->loc,
                              "Function '%s' already defined", func_name);
        return -1;
    }
    
    /* 进入函数作用域 */
    if (type_checker_enter_scope(ctx, func_name) != 0) {
        return -1;
    }
    
    /* 设置当前函数上下文 */
    ctx->current_context.in_function = true;
    ctx->current_context.expected_return_type = return_type;
    
    /* 检查函数声明 */
    if (func_decl->data.function_decl.declarations) {
        if (type_checker_check_declarations(ctx, func_decl->data.function_decl.declarations) != 0) {
            type_checker_exit_scope(ctx);
            return -1;
        }
    }
    
    /* 检查函数体 */
    if (func_decl->data.function_decl.statements) {
        if (type_checker_check_statements(ctx, func_decl->data.function_decl.statements) != 0) {
            type_checker_exit_scope(ctx);
            return -1;
        }
    }
    
    /* 创建函数签名 */
    function_signature_t *signature = type_checker_create_function_signature(
        func_name, return_type, NULL, NULL, 0);
    
    if (!signature) {
        type_checker_exit_scope(ctx);
        return -1;
    }
    
    /* 注册函数 */
    if (type_checker_register_function(ctx, signature) != 0) {
        type_checker_destroy_function_signature(signature);
        type_checker_exit_scope(ctx);
        return -1;
    }
    
    /* 恢复上下文 */
    ctx->current_context.in_function = false;
    ctx->current_context.expected_return_type = NULL;
    
    /* 退出函数作用域 */
    type_checker_exit_scope(ctx);
    
    return 0;
}

/* 检查函数参数 */
int type_checker_check_function_parameters(type_checker_context_t *ctx, ast_node_t *params) {
    if (!ctx || !params) {
        return 0;
    }
    
    /* 简化实现：检查参数列表 */
    ast_node_t *param = params;
    while (param) {
        if (type_checker_check_var_item(ctx, param) != 0) {
            return -1;
        }
        param = param->next;
    }
    
    return 0;
}

/* 检查函数返回 */
int type_checker_check_function_return(type_checker_context_t *ctx, ast_node_t *return_stmt) {
    return type_checker_check_return_statement(ctx, return_stmt);
}

/* ========== 语句检查函数 ========== */

/* 检查赋值语句 */
int type_checker_check_assignment(type_checker_context_t *ctx, ast_node_t *assignment) {
    if (!ctx || !assignment || assignment->type != AST_ASSIGNMENT) {
        return -1;
    }
    
    ast_node_t *target = assignment->data.assignment.target;
    ast_node_t *value = assignment->data.assignment.value;
    
    /* 解析目标变量 */
    symbol_t *target_symbol = type_checker_resolve_symbol(ctx, target->data.identifier.name);
    if (!target_symbol) {
        type_checker_add_error(ctx, TYPE_CHECK_UNDEFINED_SYMBOL, assignment->loc,
                              "Undefined variable '%s'", target->data.identifier.name);
        return -1;
    }
    
    /* 推断右侧表达式类型 */
    type_inference_result_t *value_result = type_checker_infer_expression(ctx, value);
    if (!value_result) {
        return -1;
    }
    
    /* 检查类型兼容性 */
    if (!type_checker_is_assignable(ctx, value_result->inferred_type, target_symbol->data_type)) {
        type_checker_add_error(ctx, TYPE_CHECK_TYPE_MISMATCH, assignment->loc,
                              "Cannot assign value of type '%s' to variable '%s' of type '%s'",
                              type_checker_type_to_string(value_result->inferred_type),
                              target->data.identifier.name,
                              type_checker_type_to_string(target_symbol->data_type));
        type_checker_destroy_inference_result(value_result);
        return -1;
    }
    
    type_checker_destroy_inference_result(value_result);
    return 0;
}

/* 检查左值 */
int type_checker_check_lvalue(type_checker_context_t *ctx, ast_node_t *lvalue) {
    if (!ctx || !lvalue) {
        return -1;
    }
    
    /* 简化实现：只检查标识符 */
    if (lvalue->type != AST_IDENTIFIER) {
        type_checker_add_error(ctx, TYPE_CHECK_INVALID_ASSIGNMENT, lvalue->loc,
                              "Invalid left-hand side in assignment");
        return -1;
    }
    
    return 0;
}

/* 检查IF语句 */
int type_checker_check_if_statement(type_checker_context_t *ctx, ast_node_t *if_stmt) {
    if (!ctx || !if_stmt || if_stmt->type != AST_IF_STMT) {
        return -1;
    }
    
    /* 检查条件表达式 */
    if (type_checker_check_condition(ctx, if_stmt->data.if_stmt.condition) != 0) {
        return -1;
    }
    
    /* 检查THEN语句 */
    if (if_stmt->data.if_stmt.then_stmt) {
        if (type_checker_check_statements(ctx, if_stmt->data.if_stmt.then_stmt) != 0) {
            return -1;
        }
    }
    
    /* 检查ELSE语句 */
    if (if_stmt->data.if_stmt.else_stmt) {
        if (type_checker_check_statements(ctx, if_stmt->data.if_stmt.else_stmt) != 0) {
            return -1;
        }
    }
    
    return 0;
}

/* 检查FOR语句 */
int type_checker_check_for_statement(type_checker_context_t *ctx, ast_node_t *for_stmt) {
    if (!ctx || !for_stmt || for_stmt->type != AST_FOR_STMT) {
        return -1;
    }
    
    const char *loop_var = for_stmt->data.for_stmt.var_name;
    
    /* 检查循环变量 */
    symbol_t *var_symbol = type_checker_resolve_symbol(ctx, loop_var);
    if (!var_symbol) {
        type_checker_add_error(ctx, TYPE_CHECK_UNDEFINED_SYMBOL, for_stmt->loc,
                              "Undefined loop variable '%s'", loop_var);
        return -1;
    }
    
    /* 检查循环变量是否为整数类型 */
    if (var_symbol->data_type->base_type != TYPE_INT_ID &&
        var_symbol->data_type->base_type != TYPE_DINT_ID) {
        type_checker_add_error(ctx, TYPE_CHECK_TYPE_MISMATCH, for_stmt->loc,
                              "Loop variable '%s' must be integer type", loop_var);
        return -1;
    }
    
    /* 检查起始值 */
    type_inference_result_t *start_result = type_checker_infer_expression(
        ctx, for_stmt->data.for_stmt.start_value);
    if (!start_result) {
        return -1;
    }
    
    if (!type_checker_is_assignable(ctx, start_result->inferred_type, var_symbol->data_type)) {
        type_checker_add_error(ctx, TYPE_CHECK_TYPE_MISMATCH, for_stmt->loc,
                              "Start value type incompatible with loop variable");
        type_checker_destroy_inference_result(start_result);
        return -1;
    }
    type_checker_destroy_inference_result(start_result);
    
    /* 检查结束值 */
    type_inference_result_t *end_result = type_checker_infer_expression(
        ctx, for_stmt->data.for_stmt.end_value);
    if (!end_result) {
        return -1;
    }
    
    if (!type_checker_is_assignable(ctx, end_result->inferred_type, var_symbol->data_type)) {
        type_checker_add_error(ctx, TYPE_CHECK_TYPE_MISMATCH, for_stmt->loc,
                              "End value type incompatible with loop variable");
        type_checker_destroy_inference_result(end_result);
        return -1;
    }
    type_checker_destroy_inference_result(end_result);
    
    /* 设置循环上下文 */
    bool old_in_loop = ctx->current_context.in_loop;
    ctx->current_context.in_loop = true;
    
    /* 检查循环体 */
    int result = 0;
    if (for_stmt->data.for_stmt.statements) {
        result = type_checker_check_statements(ctx, for_stmt->data.for_stmt.statements);
    }
    
    /* 恢复上下文 */
    ctx->current_context.in_loop = old_in_loop;
    
    return result;
}

/* 检查WHILE语句 */
int type_checker_check_while_statement(type_checker_context_t *ctx, ast_node_t *while_stmt) {
    if (!ctx || !while_stmt || while_stmt->type != AST_WHILE_STMT) {
        return -1;
    }
    
    /* 检查条件表达式 */
    if (type_checker_check_condition(ctx, while_stmt->data.while_stmt.condition) != 0) {
        return -1;
    }
    
    /* 设置循环上下文 */
    bool old_in_loop = ctx->current_context.in_loop;
    ctx->current_context.in_loop = true;
    
    /* 检查循环体 */
    int result = 0;
    if (while_stmt->data.while_stmt.statements) {
        result = type_checker_check_statements(ctx, while_stmt->data.while_stmt.statements);
    }
    
    /* 恢复上下文 */
    ctx->current_context.in_loop = old_in_loop;
    
    return result;
}

/* 检查CASE语句 */
int type_checker_check_case_statement(type_checker_context_t *ctx, ast_node_t *case_stmt) {
    if (!ctx || !case_stmt) {
        return -1;
    }
    
    /* 简化实现：暂不支持CASE语句 */
    type_checker_add_error(ctx, TYPE_CHECK_MEMORY_ERROR, case_stmt->loc,
                          "CASE statement not yet implemented");
    return -1;
}

/* 检查RETURN语句 */
int type_checker_check_return_statement(type_checker_context_t *ctx, ast_node_t *return_stmt) {
    if (!ctx || !return_stmt || return_stmt->type != AST_RETURN_STMT) {
        return -1;
    }
    
    if (!ctx->current_context.in_function) {
        type_checker_add_error(ctx, TYPE_CHECK_INVALID_OPERATION, return_stmt->loc,
                              "RETURN statement outside function");
        return -1;
    }
    
    ast_node_t *return_value = return_stmt->data.return_stmt.return_value;
    type_info_t *expected_type = ctx->current_context.expected_return_type;
    
    /* 检查返回值 */
    if (return_value && expected_type) {
        /* 有返回值且函数有返回类型 */
        type_inference_result_t *value_result = type_checker_infer_expression(ctx, return_value);
        if (!value_result) {
            return -1;
        }
        
        if (!type_checker_is_assignable(ctx, value_result->inferred_type, expected_type)) {
            type_checker_add_error(ctx, TYPE_CHECK_RETURN_TYPE_MISMATCH, return_stmt->loc,
                                  "Return value type '%s' doesn't match function return type '%s'",
                                  type_checker_type_to_string(value_result->inferred_type),
                                  type_checker_type_to_string(expected_type));
            type_checker_destroy_inference_result(value_result);
            return -1;
        }
        
        type_checker_destroy_inference_result(value_result);
    } else if (return_value && !expected_type) {
        /* 有返回值但函数无返回类型 */
        type_checker_add_error(ctx, TYPE_CHECK_RETURN_TYPE_MISMATCH, return_stmt->loc,
                              "Function has no return type but RETURN statement has value");
        return -1;
    } else if (!return_value && expected_type) {
        /* 无返回值但函数有返回类型 */
        type_checker_add_error(ctx, TYPE_CHECK_RETURN_TYPE_MISMATCH, return_stmt->loc,
                              "Function expects return value but RETURN statement has none");
        return -1;
    }
    
    return 0;
}

/* ========== 表达式检查函数 ========== */

/* 推断表达式类型 */
type_inference_result_t* type_checker_infer_expression(type_checker_context_t *ctx, ast_node_t *expr) {
    if (!ctx || !expr) {
        return NULL;
    }
    
    ctx->statistics.types_inferred++;
    
    switch (expr->type) {
        case AST_BINARY_OP:
            return type_checker_infer_binary_op(ctx, expr);
            
        case AST_UNARY_OP:
            return type_checker_infer_unary_op(ctx, expr);
            
        case AST_IDENTIFIER:
            return type_checker_infer_identifier(ctx, expr);
            
        case AST_LITERAL:
            return type_checker_infer_literal(ctx, expr);
            
        default:
            type_checker_add_error(ctx, TYPE_CHECK_MEMORY_ERROR, expr->loc,
                                  "Cannot infer type for AST node type %d", expr->type);
            return NULL;
    }
}

/* 推断二元操作类型 */
type_inference_result_t* type_checker_infer_binary_op(type_checker_context_t *ctx, ast_node_t *binary_op) {
    if (!ctx || !binary_op || binary_op->type != AST_BINARY_OP) {
        return NULL;
    }
    
    operator_type_t op = binary_op->data.binary_op.op;
    
    /* 推断左右操作数类型 */
    type_inference_result_t *left_result = type_checker_infer_expression(
        ctx, binary_op->data.binary_op.left);
    if (!left_result) {
        return NULL;
    }
    
    type_inference_result_t *right_result = type_checker_infer_expression(
        ctx, binary_op->data.binary_op.right);
    if (!right_result) {
        type_checker_destroy_inference_result(left_result);
        return NULL;
    }
    
    /* 验证操作符合法性 */
    if (!type_checker_validate_binary_operation(ctx, op, left_result->inferred_type,
                                               right_result->inferred_type)) {
        type_checker_add_error(ctx, TYPE_CHECK_INVALID_OPERATION, binary_op->loc,
                              "Invalid binary operation '%s' between types '%s' and '%s'",
                              type_checker_operator_to_string(op),
                              type_checker_type_to_string(left_result->inferred_type),
                              type_checker_type_to_string(right_result->inferred_type));
        type_checker_destroy_inference_result(left_result);
        type_checker_destroy_inference_result(right_result);
        return NULL;
    }
    
    /* 推导结果类型 */
    type_info_t *result_type = type_checker_deduce_binary_result_type(
        ctx, op, left_result->inferred_type, right_result->inferred_type);
    
    if (!result_type) {
        type_checker_destroy_inference_result(left_result);
        type_checker_destroy_inference_result(right_result);
        return NULL;
    }
    
    /* 创建结果 */
    type_inference_result_t *result = type_checker_create_inference_result(result_type);
    if (result) {
        /* 检查是否可以进行常量折叠 */
        if (left_result->is_constant && right_result->is_constant) {
            result->is_constant = true;
            /* 简化：只处理整数运算 */
            if (result_type->base_type == TYPE_INT_ID && op == OP_ADD) {
                result->const_value.int_val = left_result->const_value.int_val + 
                                            right_result->const_value.int_val;
            }
        }
    }
    
    type_checker_destroy_inference_result(left_result);
    type_checker_destroy_inference_result(right_result);
    
    return result;
}

/* 推断一元操作类型 */
type_inference_result_t* type_checker_infer_unary_op(type_checker_context_t *ctx, ast_node_t *unary_op) {
    if (!ctx || !unary_op || unary_op->type != AST_UNARY_OP) {
        return NULL;
    }
    
    operator_type_t op = unary_op->data.unary_op.op;
    
    /* 推断操作数类型 */
    type_inference_result_t *operand_result = type_checker_infer_expression(
        ctx, unary_op->data.unary_op.operand);
    if (!operand_result) {
        return NULL;
    }
    
    /* 验证操作符合法性 */ 
    if (!type_checker_validate_unary_operation(ctx, op, operand_result->inferred_type)) {
        type_checker_add_error(ctx, TYPE_CHECK_INVALID_OPERATION, unary_op->loc,
                              "Invalid unary operation '%s' on type '%s'",
                              type_checker_operator_to_string(op),
                              type_checker_type_to_string(operand_result->inferred_type));
        type_checker_destroy_inference_result(operand_result);
        return NULL;
    }
    
    /* 推导结果类型 */
    type_info_t *result_type = type_checker_deduce_unary_result_type(
        ctx, op, operand_result->inferred_type);
    
    if (!result_type) {
        type_checker_destroy_inference_result(operand_result);
        return NULL;
    }
    
    /* 创建结果 */
    type_inference_result_t *result = type_checker_create_inference_result(result_type);
    if (result && operand_result->is_constant) {
        result->is_constant = true;
        /* 简化：只处理整数取负 */
        if (result_type->base_type == TYPE_INT_ID && op == OP_NEG) {
            result->const_value.int_val = -operand_result->const_value.int_val;
        }
    }
    
    type_checker_destroy_inference_result(operand_result);
    
    return result;
}

/* 推断函数调用类型 */
type_inference_result_t* type_checker_infer_function_call(type_checker_context_t *ctx, ast_node_t *func_call) {
    if (!ctx || !func_call) {
        return NULL;
    }
    
    /* 简化实现：暂不支持函数调用推断 */
    type_checker_add_error(ctx, TYPE_CHECK_MEMORY_ERROR, func_call->loc,
                          "Function call type inference not yet implemented");
    return NULL;
}

/* 推断标识符类型 */
type_inference_result_t* type_checker_infer_identifier(type_checker_context_t *ctx, ast_node_t *identifier) {
    if (!ctx || !identifier || identifier->type != AST_IDENTIFIER) {
        return NULL;
    }
    
    const char *name = identifier->data.identifier.name;
    
    /* 解析符号 */
    symbol_t *symbol = type_checker_resolve_symbol(ctx, name);
    if (!symbol) {
        type_checker_add_error(ctx, TYPE_CHECK_UNDEFINED_SYMBOL, identifier->loc,
                              "Undefined identifier '%s'", name);
        return NULL;
    }
    
    ctx->statistics.symbols_resolved++;
    
    /* 创建推断结果 */
    type_inference_result_t *result = type_checker_create_inference_result(symbol->data_type);
    if (result) {
        result->is_lvalue = true;
        result->is_constant = false;
    }
    
    return result;
}

/* 推断字面量类型 */
type_inference_result_t* type_checker_infer_literal(type_checker_context_t *ctx, ast_node_t *literal) {
    if (!ctx || !literal || literal->type != AST_LITERAL) {
        return NULL;
    }
    
    type_info_t *type = NULL;
    type_inference_result_t *result = NULL;
    
    switch (literal->data.literal.literal_type) {
        case LITERAL_INT:
            type = type_checker_get_builtin_type(ctx, TYPE_INT_ID);
            result = type_checker_create_inference_result(type);
            if (result) {
                result->is_constant = true;
                result->const_value.int_val = literal->data.literal.value.int_val;
            }
            break;
            
        case LITERAL_REAL:
            type = type_checker_get_builtin_type(ctx, TYPE_REAL_ID);
            result = type_checker_create_inference_result(type);
            if (result) {
                result->is_constant = true;
                result->const_value.real_val = literal->data.literal.value.real_val;
            }
            break;
            
        case LITERAL_BOOL:
            type = type_checker_get_builtin_type(ctx, TYPE_BOOL_ID);
            result = type_checker_create_inference_result(type);
            if (result) {
                result->is_constant = true;
                result->const_value.bool_val = literal->data.literal.value.bool_val;
            }
            break;
            
        case LITERAL_STRING:
            type = type_checker_get_builtin_type(ctx, TYPE_STRING_ID);
            result = type_checker_create_inference_result(type);
            if (result) {
                result->is_constant = true;
                result->const_value.string_val = literal->data.literal.value.string_val;
            }
            break;
            
        default:
            type_checker_add_error(ctx, TYPE_CHECK_MEMORY_ERROR, literal->loc,
                                  "Unknown literal type");
            return NULL;
    }
    
    return result;
}

/* ========== 表达式验证 ========== */

/* 检查表达式与期望类型的兼容性 */
int type_checker_check_expression(type_checker_context_t *ctx, ast_node_t *expr, 
                                 type_info_t *expected_type) {
    if (!ctx || !expr) {
        return -1;
    }
    
    /* 推断表达式类型 */
    type_inference_result_t *result = type_checker_infer_expression(ctx, expr);
    if (!result) {
        return -1;
    }
    
    /* 如果没有期望类型，只要能推断出类型就成功 */
    if (!expected_type) {
        type_checker_destroy_inference_result(result);
        return 0;
    }
    
    /* 检查类型兼容性 */
    if (!type_checker_is_assignable(ctx, result->inferred_type, expected_type)) {
        type_checker_add_error(ctx, TYPE_CHECK_TYPE_MISMATCH, expr->loc,
                              "Expression type '%s' is not compatible with expected type '%s'",
                              type_checker_type_to_string(result->inferred_type),
                              type_checker_type_to_string(expected_type));
        type_checker_destroy_inference_result(result);
        return -1;
    }
    
    type_checker_destroy_inference_result(result);
    return 0;
}

/* 检查条件表达式 */
int type_checker_check_condition(type_checker_context_t *ctx, ast_node_t *condition) {
    if (!ctx || !condition) {
        return -1;
    }
    
    type_inference_result_t *result = type_checker_infer_expression(ctx, condition);
    if (!result) {
        return -1;
    }
    
    /* 检查条件是否为布尔类型 */
    if (result->inferred_type->base_type != TYPE_BOOL_ID) {
        type_checker_add_error(ctx, TYPE_CHECK_TYPE_MISMATCH, condition->loc,
                              "Condition must be boolean type, got '%s'",
                              type_checker_type_to_string(result->inferred_type));
        type_checker_destroy_inference_result(result);
        return -1;
    }
    
    type_checker_destroy_inference_result(result);
    return 0;
}

/* ========== 类型工具函数 ========== */

/* 类型转字符串 */
const char* type_checker_type_to_string(type_info_t *type) {
    if (!type) {
        return "NULL";
    }
    
    switch (type->base_type) {
        case TYPE_BOOL_ID:
            return "BOOL";
        case TYPE_BYTE_ID:
            return "BYTE";
        case TYPE_SINT_ID:
            return "SINT";
        case TYPE_USINT_ID:
            return "USINT";
        case TYPE_INT_ID:
            return "INT";
        case TYPE_UINT_ID:
            return "UINT";
        case TYPE_DINT_ID:
            return "DINT";
        case TYPE_UDINT_ID:
            return "UDINT";
        case TYPE_LINT_ID:
            return "LINT";
        case TYPE_ULINT_ID:
            return "ULINT";
        case TYPE_REAL_ID:
            return "REAL";
        case TYPE_LREAL_ID:
            return "LREAL";
        case TYPE_STRING_ID:
            return "STRING";
        case TYPE_WSTRING_ID:
            return "WSTRING";
        case TYPE_TIME_ID:
            return "TIME";
        case TYPE_DATE_ID:
            return "DATE";
        case TYPE_TIME_OF_DAY_ID:
            return "TOD";
        case TYPE_DATE_AND_TIME_ID:
            return "DT";
        case TYPE_ARRAY_ID:
            if (type->is_array) {
                return "ARRAY";
            }
            return "ARRAY_TYPE";
        case TYPE_STRUCT_ID:
            return "STRUCT";
        case TYPE_ENUM_ID:
            return "ENUM";
        case TYPE_FUNCTION_ID:
            return "FUNCTION";
        case TYPE_POINTER_ID:
            return "POINTER";
        default:
            return "UNKNOWN";
    }
}

/* 判断是否为数值类型 */
bool type_checker_is_numeric_type(type_info_t *type) {
    if (!type) {
        return false;
    }
    
    switch (type->base_type) {
        case TYPE_BYTE_ID:
        case TYPE_SINT_ID:
        case TYPE_USINT_ID:
        case TYPE_INT_ID:
        case TYPE_UINT_ID:
        case TYPE_DINT_ID:
        case TYPE_UDINT_ID:
        case TYPE_LINT_ID:
        case TYPE_ULINT_ID:
        case TYPE_REAL_ID:
        case TYPE_LREAL_ID:
            return true;
        default:
            return false;
    }
}

/* 判断是否为整数类型 */
bool type_checker_is_integer_type(type_info_t *type) {
    if (!type) {
        return false;
    }
    
    switch (type->base_type) {
        case TYPE_BYTE_ID:
        case TYPE_SINT_ID:
        case TYPE_USINT_ID:
        case TYPE_INT_ID:
        case TYPE_UINT_ID:
        case TYPE_DINT_ID:
        case TYPE_UDINT_ID:
        case TYPE_LINT_ID:
        case TYPE_ULINT_ID:
            return true;
        default:
            return false;
    }
}

/* 判断是否为实数类型 */
bool type_checker_is_real_type(type_info_t *type) {
    if (!type) {
        return false;
    }
    
    switch (type->base_type) {
        case TYPE_REAL_ID:
        case TYPE_LREAL_ID:
            return true;
        default:
            return false;
    }
}

/* 判断是否为布尔类型 */
bool type_checker_is_boolean_type(type_info_t *type) {
    if (!type) {
        return false;
    }
    
    return type->base_type == TYPE_BOOL_ID;
}

/* 判断是否为字符串类型 */
bool type_checker_is_string_type(type_info_t *type) {
    if (!type) {
        return false;
    }
    
    switch (type->base_type) {
        case TYPE_STRING_ID:
        case TYPE_WSTRING_ID:
            return true;
        default:
            return false;
    }
}

/* 判断是否为时间类型 */
bool type_checker_is_time_type(type_info_t *type) {
    if (!type) {
        return false;
    }
    
    switch (type->base_type) {
        case TYPE_TIME_ID:
        case TYPE_DATE_ID:
        case TYPE_TIME_OF_DAY_ID:
        case TYPE_DATE_AND_TIME_ID:
            return true;
        default:
            return false;
    }
}

/* 判断是否为有符号整数类型 */
bool type_checker_is_signed_integer_type(type_info_t *type) {
    if (!type) {
        return false;
    }
    
    switch (type->base_type) {
        case TYPE_SINT_ID:
        case TYPE_INT_ID:
        case TYPE_DINT_ID:
        case TYPE_LINT_ID:
            return true;
        default:
            return false;
    }
}

/* 判断是否为无符号整数类型 */
bool type_checker_is_unsigned_integer_type(type_info_t *type) {
    if (!type) {
        return false;
    }
    
    switch (type->base_type) {
        case TYPE_BYTE_ID:
        case TYPE_USINT_ID:
        case TYPE_UINT_ID:
        case TYPE_UDINT_ID:
        case TYPE_ULINT_ID:
            return true;
        default:
            return false;
    }
}

/* 判断是否为数组类型 */
bool type_checker_is_array_type(type_info_t *type) {
    if (!type) {
        return false;
    }
    
    return type->is_array || type->base_type == TYPE_ARRAY_ID;
}

/* 判断是否为结构体类型 */
bool type_checker_is_struct_type(type_info_t *type) {
    if (!type) {
        return false;
    }
    
    return type->base_type == TYPE_STRUCT_ID;
}

/* 判断是否为枚举类型 */
bool type_checker_is_enum_type(type_info_t *type) {
    if (!type) {
        return false;
    }
    
    return type->base_type == TYPE_ENUM_ID;
}

/* 判断是否为函数类型 */
bool type_checker_is_function_type(type_info_t *type) {
    if (!type) {
        return false;
    }
    
    return type->base_type == TYPE_FUNCTION_ID;
}

/* 判断是否为指针类型 */
bool type_checker_is_pointer_type(type_info_t *type) {
    if (!type) {
        return false;
    }
    
    return type->is_pointer || type->base_type == TYPE_POINTER_ID;
}

/* 判断是否为常量类型 */
bool type_checker_is_constant_type(type_info_t *type) {
    if (!type) {
        return false;
    }
    
    return type->is_constant;
}

/* 获取类型大小 */
uint32_t type_checker_get_type_size(type_info_t *type) {
    if (!type) {
        return 0;
    }
    
    if (type->is_array && type->compound.array_info.element_type) {
        /* 数组类型：元素大小 * 元素数量 */
        uint32_t element_size = type_checker_get_type_size(type->compound.array_info.element_type);
        uint32_t total_elements = 1;
        
        for (uint32_t i = 0; i < type->compound.array_info.dimensions; i++) {
            uint32_t lower = type->compound.array_info.bounds[i * 2];
            uint32_t upper = type->compound.array_info.bounds[i * 2 + 1];
            total_elements *= (upper - lower + 1);
        }
        
        return element_size * total_elements;
    }
    
    return type->size;
}

/* 获取类型对齐 */
uint32_t type_checker_get_type_alignment(type_info_t *type) {
    if (!type) {
        return 1;
    }
    
    return type->alignment;
}

/* 获取数组元素类型 */
type_info_t* type_checker_get_array_element_type(type_info_t *type) {
    if (!type || !type_checker_is_array_type(type)) {
        return NULL;
    }
    
    return type->compound.array_info.element_type;
}

/* 获取数组维数 */
uint32_t type_checker_get_array_dimensions(type_info_t *type) {
    if (!type || !type_checker_is_array_type(type)) {
        return 0;
    }
    
    return type->compound.array_info.dimensions;
}

/* 比较类型相等性 */
bool type_checker_types_equal(type_info_t *type1, type_info_t *type2) {
    if (type1 == type2) {
        return true;
    }
    
    if (!type1 || !type2) {
        return false;
    }
    
    /* 基础类型必须相同 */
    if (type1->base_type != type2->base_type) {
        return false;
    }
    
    /* 常量性和数组性必须相同 */
    if (type1->is_constant != type2->is_constant ||
        type1->is_array != type2->is_array ||
        type1->is_pointer != type2->is_pointer) {
        return false;
    }
    
    /* 数组类型的特殊比较 */
    if (type1->is_array) {
        if (type1->compound.array_info.dimensions != type2->compound.array_info.dimensions) {
            return false;
        }
        
        /* 比较元素类型 */
        if (!type_checker_types_equal(type1->compound.array_info.element_type,
                                     type2->compound.array_info.element_type)) {
            return false;
        }
        
        /* 比较数组边界 */
        for (uint32_t i = 0; i < type1->compound.array_info.dimensions * 2; i++) {
            if (type1->compound.array_info.bounds[i] != 
                type2->compound.array_info.bounds[i]) {
                return false;
            }
        }
    }
    
    return true;
}

/* 获取数值类型的精度 */
int type_checker_get_numeric_precision(type_info_t *type) {
    if (!type || !type_checker_is_numeric_type(type)) {
        return -1;
    }
    
    switch (type->base_type) {
        case TYPE_BYTE_ID:
        case TYPE_SINT_ID:
        case TYPE_USINT_ID:
            return 8;
        case TYPE_INT_ID:
        case TYPE_UINT_ID:
            return 16;
        case TYPE_DINT_ID:
        case TYPE_UDINT_ID:
        case TYPE_REAL_ID:
            return 32;
        case TYPE_LINT_ID:
        case TYPE_ULINT_ID:
        case TYPE_LREAL_ID:
            return 64;
        default:
            return -1;
    }
}

/* 获取更高精度的类型 */
type_info_t* type_checker_get_higher_precision_type(type_checker_context_t *ctx,
                                                   type_info_t *type1, 
                                                   type_info_t *type2) {
    if (!ctx || !type1 || !type2) {
        return NULL;
    }
    
    /* 如果有一个是实数类型，返回实数类型 */
    if (type_checker_is_real_type(type1)) {
        if (type_checker_is_real_type(type2)) {
            /* 两个都是实数，返回精度更高的 */
            return (type_checker_get_numeric_precision(type1) >= 
                    type_checker_get_numeric_precision(type2)) ? type1 : type2;
        }
        return type1;
    }
    
    if (type_checker_is_real_type(type2)) {
        return type2;
    }
    
    /* 两个都是整数，返回精度更高的 */
    if (type_checker_is_integer_type(type1) && type_checker_is_integer_type(type2)) {
        int prec1 = type_checker_get_numeric_precision(type1);
        int prec2 = type_checker_get_numeric_precision(type2);
        
        if (prec1 > prec2) {
            return type1;
        } else if (prec2 > prec1) {
            return type2;
        } else {
            /* 精度相同，优先有符号类型 */
            if (type_checker_is_signed_integer_type(type1)) {
                return type1;
            }
            return type2;
        }
    }
    
    /* 默认返回第一个类型 */
    return type1;
}

/* 操作符转字符串 */
const char* type_checker_operator_to_string(operator_type_t op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_MOD: return "MOD";
        case OP_POWER: return "**";
        case OP_AND: return "AND";
        case OP_OR: return "OR";
        case OP_XOR: return "XOR";
        case OP_NOT: return "NOT";
        case OP_EQ: return "=";
        case OP_NE: return "<>";
        case OP_LT: return "<";
        case OP_LE: return "<=";
        case OP_GT: return ">";
        case OP_GE: return ">=";
        case OP_NEG: return "-";
        case OP_POS: return "+";
        case OP_ASSIGN: return ":=";
        default: return "UNKNOWN_OP";
    }
}

/* ========== 辅助函数实现 ========== */

/* 类型兼容性检查 */
static bool type_checker_is_assignable(type_checker_context_t *ctx, type_info_t *from, type_info_t *to) {
    if (!ctx || !from || !to) {
        return false;
    }
    
    /* 相同类型总是兼容的 */
    if (type_checker_types_equal(from, to)) {
        return true;
    }
    
    /* 检查是否允许隐式转换 */
    return type_checker_is_implicitly_convertible(ctx, from, to);
}

/* 创建类型推断结果 */
static type_inference_result_t* type_checker_create_inference_result(type_info_t *type) {
    if (!type) {
        return NULL;
    }
    
    type_inference_result_t *result = (type_inference_result_t*)mmgr_alloc_general(
        sizeof(type_inference_result_t));
    if (!result) {
        return NULL;
    }
    
    memset(result, 0, sizeof(type_inference_result_t));
    result->inferred_type = type;
    result->is_constant = false;
    result->is_lvalue = false;
    
    return result;
}

/* 销毁类型推断结果 */
static void type_checker_destroy_inference_result(type_inference_result_t *result) {
    if (!result) return;
    
    /* 内存由MMGR管理，不需要显式释放 */
    /* 但需要清空指针避免悬空引用 */
    result->inferred_type = NULL;
}

/* 添加消息到消息链表 */
static void type_checker_add_message(type_checker_context_t *ctx, type_severity_t severity,
                                    type_check_error_t error_code, source_location_t *location,
                                    const char *format, va_list args) {
    if (!ctx || !format) {
        return;
    }
    
    type_check_message_t *msg = (type_check_message_t*)mmgr_alloc_general(
        sizeof(type_check_message_t));
    if (!msg) {
        return;
    }
    
    msg->severity = severity;
    msg->error_code = error_code;
    msg->location = location;
    
    /* 格式化消息 */
    char *message_buf = (char*)mmgr_alloc_general(512);
    if (message_buf) {
        vsnprintf(message_buf, 512, format, args);
        msg->message = message_buf;
    } else {
        msg->message = mmgr_alloc_string("Memory allocation error");
    }
    
    /* 添加到消息链表头部 */
    msg->next = ctx->messages;
    ctx->messages = msg;
}

/* ========== 类型检查器API ========== */

/* 创建类型检查器 */
type_checker_context_t* create_type_checker() {
    return type_checker_create();
}

/* 销毁类型检查器 */
void destroy_type_checker(type_checker_context_t *ctx) {
    type_checker_destroy(ctx);
}

/* 初始化类型检查器 */
int init_type_checker(type_checker_context_t *ctx, symbol_table_manager_t *sym_mgr) {
    return type_checker_init(ctx, sym_mgr);
}

/* 设置严格模式 */
int set_strict_mode(type_checker_context_t *ctx, bool enabled) {
    return type_checker_set_strict_mode(ctx, enabled);
}

/* 设置警告为错误 */
int set_warnings_as_errors(type_checker_context_t *ctx, bool enabled) {
    return type_checker_set_warnings_as_errors(ctx, enabled);
}

/* 启用未使用变量检查 */
int enable_unused_check(type_checker_context_t *ctx, bool enabled) {
    return type_checker_enable_unused_check(ctx, enabled);
}

/* 允许隐式转换 */
int allow_implicit_conversion(type_checker_context_t *ctx, bool enabled) {
    return type_checker_allow_implicit_conversion(ctx, enabled);
}

/* 检查程序 */
int check_program(type_checker_context_t *ctx, ast_node_t *program) {
    return type_checker_check_program(ctx, program);
}

/* 检查声明列表 */
int check_declarations(type_checker_context_t *ctx, ast_node_t *decl_list) {
    return type_checker_check_declarations(ctx, decl_list);
}

/* 检查语句列表 */
int check_statements(type_checker_context_t *ctx, ast_node_t *stmt_list) {
    return type_checker_check_statements(ctx, stmt_list);
}

/* 检查AST节点 */
int check_node(type_checker_context_t *ctx, ast_node_t *node) {
    return type_checker_check_node(ctx, node);
}

/* 获取错误数量 */
uint32_t get_error_count(const type_checker_context_t *ctx) {
    return type_checker_get_error_count(ctx);
}

/* 获取警告数量 */
uint32_t get_warning_count(const type_checker_context_t *ctx) {
    return type_checker_get_warning_count(ctx);
}

/* 检查是否有错误 */
bool has_errors(const type_checker_context_t *ctx) {
    return type_checker_has_errors(ctx);
}

/* 打印消息 */
void print_messages(const type_checker_context_t *ctx) {
    type_checker_print_messages(ctx);
}

/* 打印总结 */
void print_summary(const type_checker_context_t *ctx) {
    type_checker_print_summary(ctx);
}

/* 获取消息字符串 */
char* get_message_string(const type_checker_context_t *ctx) {
    return type_checker_get_message_string(ctx);
}

/* 清理消息 */
void clear_messages(type_checker_context_t *ctx) {
    type_checker_clear_messages(ctx);
}

/* 打印统计信息 */
void print_statistics(const type_checker_context_t *ctx) {
    type_checker_print_statistics(ctx);
}

/* 转储符号表 */
void dump_symbol_table(const type_checker_context_t *ctx) {
    type_checker_dump_symbol_table(ctx);
}

/* 转储类型表 */
void dump_type_table(const type_checker_context_t *ctx) {
    type_checker_dump_type_table(ctx);
}

/* 转储函数签名 */
void dump_function_signatures(const type_checker_context_t *ctx) {
    type_checker_dump_function_signatures(ctx);
}

/* ========== 常量求值 ========== */

/* 检查是否为常量表达式 */
bool type_checker_is_constant_expression(type_checker_context_t *ctx, ast_node_t *expr) {
    if (!ctx || !expr) {
        return false;
    }
    
    switch (expr->type) {
        case AST_LITERAL:
            return true;
            
        case AST_IDENTIFIER:
            /* 简化：假设标识符不是常量 */
            return false;
            
        case AST_BINARY_OP: {
            /* 二元操作：两个操作数都是常量才是常量 */
            return type_checker_is_constant_expression(ctx, expr->data.binary_op.left) &&
                   type_checker_is_constant_expression(ctx, expr->data.binary_op.right);
        }
        
        case AST_UNARY_OP:
            /* 一元操作：操作数是常量才是常量 */
            return type_checker_is_constant_expression(ctx, expr->data.unary_op.operand);
            
        default:
            return false;
    }
}

/* 求值常量表达式 */
type_inference_result_t* type_checker_evaluate_constant(type_checker_context_t *ctx, 
                                                       ast_node_t *expr) {
    if (!ctx || !expr || !type_checker_is_constant_expression(ctx, expr)) {
        return NULL;
    }
    
    /* 复用现有的类型推断逻辑 */
    return type_checker_infer_expression(ctx, expr);
}

/* 常量折叠 */
ast_node_t* type_checker_fold_constant(type_checker_context_t *ctx, ast_node_t *expr) {
    if (!ctx || !expr) {
        return NULL;
    }
    
    type_inference_result_t *result = type_checker_evaluate_constant(ctx, expr);
    if (!result || !result->is_constant) {
        return expr; /* 无法折叠，返回原表达式 */
    }
    
    /* 创建新的字面量节点 */
    ast_node_t *literal = NULL;
    switch (result->inferred_type->base_type) {
        case TYPE_INT_ID:
            literal = ast_create_literal(LITERAL_INT, &result->const_value.int_val);
            break;
        case TYPE_REAL_ID:
            literal = ast_create_literal(LITERAL_REAL, &result->const_value.real_val);
            break;
        case TYPE_BOOL_ID:
            literal = ast_create_literal(LITERAL_BOOL, &result->const_value.bool_val);
            break;
        case TYPE_STRING_ID:
            literal = ast_create_literal(LITERAL_STRING, result->const_value.string_val);
            break;
        default:
            literal = expr;
            break;
    }
    
    type_checker_destroy_inference_result(result);
    return literal ? literal : expr;
}

/* ========== 语义分析辅助 ========== */

/* 死代码检测 */
int type_checker_check_dead_code(type_checker_context_t *ctx, ast_node_t *node) {
    if (!ctx || !node) {
        return 0;
    }
    
    /* 简化实现：检查RETURN语句后的代码 */
    if (node->type == AST_STATEMENT_LIST) {
        ast_node_t *stmt = node;
        bool found_return = false;
        
        while (stmt) {
            if (stmt->type == AST_RETURN_STMT) {
                found_return = true;
            } else if (found_return) {
                type_checker_add_warning(ctx, TYPE_CHECK_MEMORY_ERROR, stmt->loc,
                                        "Unreachable code after RETURN statement");
            }
            stmt = stmt->next;
        }
    }
    
    return 0;
}

/* 检查未使用变量 */
int type_checker_check_unused_variables(type_checker_context_t *ctx) {
    if (!ctx || !ctx->options.check_unused_vars) {
        return 0;
    }
    
    /* 简化实现：暂不检查 */
    type_checker_add_info(ctx, "Unused variable checking not implemented");
    return 0;
}

/* 检查未初始化变量 */
int type_checker_check_uninitialized_variables(type_checker_context_t *ctx, ast_node_t *node) {
    if (!ctx || !node || !ctx->options.check_uninitialized) {
        return 0;
    }
    
    /* 简化实现：暂不检查 */
    return 0;
}

/* 控制流分析 */
int type_checker_analyze_control_flow(type_checker_context_t *ctx, ast_node_t *node) {
    if (!ctx || !node) {
        return 0;
    }
    
    /* 简化实现：基础的控制流检查 */
    switch (node->type) {
        case AST_IF_STMT:
            /* 检查IF语句的条件和分支 */
            if (type_checker_check_condition(ctx, node->data.if_stmt.condition) != 0) {
                return -1;
            }
            break;
            
        case AST_WHILE_STMT:
            /* 检查WHILE循环的条件 */
            if (type_checker_check_condition(ctx, node->data.while_stmt.condition) != 0) {
                return -1;
            }
            break;
            
        case AST_FOR_STMT:
            /* 检查FOR循环的变量和范围 */
            /* 已在for语句检查中实现 */
            break;
            
        default:
            break;
    }
    
    return 0;
}

/* ========== 统计和诊断 ========== */

/* 打印统计信息 */
void type_checker_print_statistics(const type_checker_context_t *ctx) {
    if (!ctx) return;
    
    printf("=== 类型检查器统计 ===\n");
    printf("节点检查数: %u\n", ctx->statistics.nodes_checked);
    printf("符号解析数: %u\n", ctx->statistics.symbols_resolved);
    printf("类型推断数: %u\n", ctx->statistics.types_inferred);
    printf("内置类型数: %u\n", ctx->builtin_type_count);
    printf("注册函数数: %u\n", ctx->function_count);
    printf("兼容性规则: %u\n", ctx->compatibility_count);
    printf("当前作用域深度: %u\n", ctx->current_context.scope_depth);
    printf("===================\n");
}

/* 获取检查节点数 */
uint32_t type_checker_get_nodes_checked(const type_checker_context_t *ctx) {
    return ctx ? ctx->statistics.nodes_checked : 0;
}

/* 获取符号解析数 */
uint32_t type_checker_get_symbols_resolved(const type_checker_context_t *ctx) {
    return ctx ? ctx->statistics.symbols_resolved : 0;
}

/* 转储符号表 */
void type_checker_dump_symbol_table(const type_checker_context_t *ctx) {
    if (!ctx) return;
    
    printf("=== 符号表转储 ===\n");
    printf("作用域深度: %u\n", ctx->current_context.scope_depth);
    printf("当前函数: %s\n", 
           ctx->current_context.current_function ? 
           ctx->current_context.current_function->name : "无");
    printf("================\n");
}

/* 转储类型表 */
void type_checker_dump_type_table(const type_checker_context_t *ctx) {
    if (!ctx) return;
    
    printf("=== 类型表转储 ===\n");
    printf("内置类型数量: %u\n", ctx->builtin_type_count);
    
    for (uint32_t i = 0; i < ctx->builtin_type_count; i++) {
        if (ctx->builtin_types[i]) {
            printf("类型 %u: %s (大小: %u)\n", 
                   i, type_checker_type_to_string(ctx->builtin_types[i]),
                   ctx->builtin_types[i]->size);
        }
    }
    
    printf("兼容性规则数量: %u\n", ctx->compatibility_count);
    for (uint32_t i = 0; i < ctx->compatibility_count; i++) {
        type_compatibility_t *compat = &ctx->compatibility_table[i];
        printf("规则 %u: %s -> %s (隐式: %s, 显式: %s)\n", i,
               type_checker_type_to_string(compat->from_type),
               type_checker_type_to_string(compat->to_type),
               compat->is_implicit_allowed ? "是" : "否",
               compat->is_explicit_allowed ? "是" : "否");
    }
    printf("================\n");
}

/* 转储函数签名 */
void type_checker_dump_function_signatures(const type_checker_context_t *ctx) {
    if (!ctx) return;
    
    printf("=== 函数签名转储 ===\n");
    printf("注册函数数量: %u\n", ctx->function_count);
    
    for (uint32_t i = 0; i < ctx->function_count; i++) {
        function_signature_t *sig = &ctx->function_signatures[i];
        printf("函数 %u: %s(", i, sig->name);
        
        for (uint32_t j = 0; j < sig->param_count; j++) {
            if (j > 0) printf(", ");
            printf("%s", type_checker_type_to_string(sig->param_types[j]));
            if (sig->param_names && sig->param_names[j]) {
                printf(" %s", sig->param_names[j]);
            }
        }
        
        printf(") : %s", type_checker_type_to_string(sig->return_type));
        if (sig->is_builtin) printf(" [内置]");
        if (sig->is_variadic) printf(" [可变参数]");
        printf("\n");
    }
    printf("=================\n");
}

/* ========== 类型系统函数 ========== */

/* 检查类型兼容性 */
bool type_checker_is_compatible(type_checker_context_t *ctx, type_info_t *from_type, type_info_t *to_type) {
    if (!ctx || !from_type || !to_type) {
        return false;
    }
    
    /* 相同类型 */
    if (from_type->base_type == to_type->base_type) {
        return true;
    }
    
    /* 查找兼容性表 */
    for (uint32_t i = 0; i < ctx->compatibility_count; i++) {
        type_compatibility_t *compat = &ctx->compatibility_table[i];
        if (compat->from_type->base_type == from_type->base_type &&
            compat->to_type->base_type == to_type->base_type) {
            return compat->is_implicit_allowed || compat->is_explicit_allowed;
        }
    }
    
    return false;
}

/* 检查是否可隐式转换 */
bool type_checker_is_implicitly_convertible(type_checker_context_t *ctx, 
                                           type_info_t *from_type, type_info_t *to_type) {
    if (!ctx || !from_type || !to_type) {
        return false;
    }
    
    if (!ctx->options.allow_implicit_conv) {
        return from_type->base_type == to_type->base_type;
    }
    
    /* 相同类型 */
    if (from_type->base_type == to_type->base_type) {
        return true;
    }
    
    /* 查找兼容性表 */
    for (uint32_t i = 0; i < ctx->compatibility_count; i++) {
        type_compatibility_t *compat = &ctx->compatibility_table[i];
        if (compat->from_type->base_type == from_type->base_type &&
            compat->to_type->base_type == to_type->base_type) {
            return compat->is_implicit_allowed;
        }
    }
    
    return false;
}

/* 检查是否可显式转换 */
bool type_checker_is_explicitly_convertible(type_checker_context_t *ctx, 
                                           type_info_t *from_type, type_info_t *to_type) {
    if (!ctx || !from_type || !to_type) {
        return false;
    }
    
    /* 相同类型 */
    if (from_type->base_type == to_type->base_type) {
        return true;
    }
    
    /* 查找兼容性表 */
    for (uint32_t i = 0; i < ctx->compatibility_count; i++) {
        type_compatibility_t *compat = &ctx->compatibility_table[i];
        if (compat->from_type->base_type == from_type->base_type &&
            compat->to_type->base_type == to_type->base_type) {
            return compat->is_explicit_allowed;
        }
    }
    
    return false;
}

/* 推导二元操作结果类型 */
type_info_t* type_checker_deduce_binary_result_type(type_checker_context_t *ctx,
                                                   operator_type_t op,
                                                   type_info_t *left_type,
                                                   type_info_t *right_type) {
    if (!ctx || !left_type || !right_type) {
        return NULL;
    }
    
    switch (op) {
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_MOD:
            /* 算术运算：提升为更高精度的类型 */
            if (left_type->base_type == TYPE_REAL_ID || right_type->base_type == TYPE_REAL_ID) {
                return type_checker_get_builtin_type(ctx, TYPE_REAL_ID);
            } else if (left_type->base_type == TYPE_DINT_ID || right_type->base_type == TYPE_DINT_ID) {
                return type_checker_get_builtin_type(ctx, TYPE_DINT_ID);
            } else {
                return type_checker_get_builtin_type(ctx, TYPE_INT_ID);
            }
            
        case OP_EQ:
        case OP_NE:
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE:
            /* 比较运算：结果为布尔类型 */
            return type_checker_get_builtin_type(ctx, TYPE_BOOL_ID);
            
        case OP_AND:
        case OP_OR:
            /* 逻辑运算：结果为布尔类型 */
            return type_checker_get_builtin_type(ctx, TYPE_BOOL_ID);
            
        default:
            return NULL;
    }
}

/* 推导一元操作结果类型 */
type_info_t* type_checker_deduce_unary_result_type(type_checker_context_t *ctx,
                                                  operator_type_t op,
                                                  type_info_t *operand_type) {
    if (!ctx || !operand_type) {
        return NULL;
    }
    
    switch (op) {
        case OP_NEG:
            /* 取负：保持原类型 */
            return operand_type;
            
        case OP_NOT:
            /* 逻辑非：结果为布尔类型 */
            return type_checker_get_builtin_type(ctx, TYPE_BOOL_ID);
            
        default:
            return NULL;
    }
}

/* 验证二元操作 */
bool type_checker_validate_binary_operation(type_checker_context_t *ctx,
                                           operator_type_t op,
                                           type_info_t *left_type,
                                           type_info_t *right_type) {
    if (!ctx || !left_type || !right_type) {
        return false;
    }
    
    switch (op) {
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_MOD:
            /* 算术运算：需要数值类型 */
            return type_checker_is_numeric_type(left_type) && 
                   type_checker_is_numeric_type(right_type);
            
        case OP_EQ:
        case OP_NE:
            /* 相等比较：任何相同类型 */
            return type_checker_is_compatible(ctx, left_type, right_type);
            
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE:
            /* 大小比较：需要可比较类型 */
            return (type_checker_is_numeric_type(left_type) && 
                    type_checker_is_numeric_type(right_type)) ||
                   (type_checker_is_string_type(left_type) && 
                    type_checker_is_string_type(right_type));
            
        case OP_AND:
        case OP_OR:
            /* 逻辑运算：需要布尔类型 */
            return type_checker_is_boolean_type(left_type) && 
                   type_checker_is_boolean_type(right_type);
            
        default:
            return false;
    }
}

/* 验证一元操作 */
bool type_checker_validate_unary_operation(type_checker_context_t *ctx,
                                          operator_type_t op,
                                          type_info_t *operand_type) {
    if (!ctx || !operand_type) {
        return false;
    }
    
    switch (op) {
        case OP_NEG:
            /* 取负：需要数值类型 */
            return type_checker_is_numeric_type(operand_type);
            
        case OP_NOT:
            /* 逻辑非：需要布尔类型 */
            return type_checker_is_boolean_type(operand_type);
            
        default:
            return false;
    }
}

/* ========== 符号管理函数 ========== */

/* 解析符号 */
symbol_t* type_checker_resolve_symbol(type_checker_context_t *ctx, const char *name) {
    if (!ctx || !name || !ctx->sym_mgr) {
        return NULL;
    }
    
    return lookup_symbol(name);
}

/* 解析函数 */
function_signature_t* type_checker_resolve_function(type_checker_context_t *ctx, 
                                                   const char *name) {
    if (!ctx || !name) {
        return NULL;
    }
    
    /* 在函数签名表中查找 */
    for (uint32_t i = 0; i < ctx->function_count; i++) {
        if (strcmp(ctx->function_signatures[i].name, name) == 0) {
            return &ctx->function_signatures[i];
        }
    }
    
    return NULL;
}

/* 注册符号 */
int type_checker_register_symbol(type_checker_context_t *ctx, const char *name, 
                                type_info_t *type, symbol_type_t sym_type) {
    if (!ctx || !name || !type || !ctx->sym_mgr) {
        return -1;
    }
    
    symbol_t *symbol = add_symbol(name, sym_type, type);
    if (!symbol) {
        return -1;
    }
    
    return 0;
}

/* 注册函数 */
int type_checker_register_function(type_checker_context_t *ctx, 
                                  const function_signature_t *signature) {
    if (!ctx || !signature || ctx->function_count >= 128) {
        return -1;
    }
    
    /* 复制函数签名 */
    function_signature_t *new_sig = &ctx->function_signatures[ctx->function_count];
    
    new_sig->name = mmgr_alloc_string(signature->name);
    new_sig->return_type = signature->return_type;
    new_sig->param_count = signature->param_count;
    new_sig->is_variadic = signature->is_variadic;
    new_sig->is_builtin = signature->is_builtin;
    
    if (signature->param_count > 0) {
        new_sig->param_types = (type_info_t**)mmgr_alloc_general(
            sizeof(type_info_t*) * signature->param_count);
        new_sig->param_names = (char**)mmgr_alloc_general(
            sizeof(char*) * signature->param_count);
        
        if (!new_sig->param_types || !new_sig->param_names) {
            return -1;
        }
        
        for (uint32_t i = 0; i < signature->param_count; i++) {
            new_sig->param_types[i] = signature->param_types[i];
            new_sig->param_names[i] = signature->param_names[i] ? 
                mmgr_alloc_string(signature->param_names[i]) : NULL;
        }
    } else {
        new_sig->param_types = NULL;
        new_sig->param_names = NULL;
    }
    
    ctx->function_count++;
    
    return 0;
}

/* 进入作用域 */
int type_checker_enter_scope(type_checker_context_t *ctx, const char *scope_name) {
    if (!ctx) {
        return -1;
    }
    
    ctx->current_context.scope_depth++;
    
    if (ctx->sym_mgr) {
        enter_scope(scope_name, ctx->current_context.scope_depth);
    }
    
    return 0;
}

/* 退出作用域 */
int type_checker_exit_scope(type_checker_context_t *ctx) {
    if (!ctx || ctx->current_context.scope_depth == 0) {
        return -1;
    }
    
    ctx->current_context.scope_depth--;
    
    if (ctx->sym_mgr) {
        exit_scope();
    }
    
    return 0;
}

/* ========== 函数签名管理 ========== */

/* 创建函数签名 */
function_signature_t* type_checker_create_function_signature(const char *name,
                                                           type_info_t *return_type,
                                                           type_info_t **param_types,
                                                           char **param_names,
                                                           uint32_t param_count) {
    if (!name) {
        return NULL;
    }
    
    function_signature_t *signature = (function_signature_t*)mmgr_alloc_general(
        sizeof(function_signature_t));
    if (!signature) {
        return NULL;
    }
    
    signature->name = mmgr_alloc_string(name);
    signature->return_type = return_type;
    signature->param_count = param_count;
    signature->is_variadic = false;
    signature->is_builtin = false;
    
    if (param_count > 0 && param_types) {
        signature->param_types = (type_info_t**)mmgr_alloc_general(
            sizeof(type_info_t*) * param_count);
        signature->param_names = (char**)mmgr_alloc_general(
            sizeof(char*) * param_count);
        
        if (!signature->param_types || !signature->param_names) {
            return NULL;
        }
        
        for (uint32_t i = 0; i < param_count; i++) {
            signature->param_types[i] = param_types[i];
            signature->param_names[i] = param_names && param_names[i] ? 
                mmgr_alloc_string(param_names[i]) : NULL;
        }
    } else {
        signature->param_types = NULL;
        signature->param_names = NULL;
    }
    
    return signature;
}

/* 销毁函数签名 */
void type_checker_destroy_function_signature(function_signature_t *signature) {
    if (!signature) return;
    
    /* 内存由MMGR管理，不需要显式释放 */
    /* 但需要清空指针避免悬空引用 */
    signature->name = NULL;
    signature->param_types = NULL;
    signature->param_names = NULL;
}

/* 检查函数调用兼容性 */
int type_checker_check_function_call_compatibility(type_checker_context_t *ctx,
                                                  const function_signature_t *signature,
                                                  ast_node_t *args) {
    if (!ctx || !signature) {
        return -1;
    }
    
    /* 计算实际参数数量 */
    uint32_t actual_count = 0;
    ast_node_t *arg = args;
    while (arg) {
        actual_count++;
        arg = arg->next;
    }
    
    /* 检查参数数量 */
    if (type_checker_check_parameter_count(ctx, signature, actual_count) != 0) {
        return -1;
    }
    
    /* 检查参数类型兼容性 */
    arg = args;
    for (uint32_t i = 0; i < signature->param_count && arg; i++) {
        type_inference_result_t *arg_result = type_checker_infer_expression(ctx, arg);
        if (!arg_result) {
            return -1;
        }
        
        if (!type_checker_is_assignable(ctx, arg_result->inferred_type, 
                                       signature->param_types[i])) {
            type_checker_add_error(ctx, TYPE_CHECK_FUNCTION_MISMATCH, arg->loc,
                                  "Argument %u: type '%s' is not compatible with parameter type '%s'",
                                  i + 1,
                                  type_checker_type_to_string(arg_result->inferred_type),
                                  type_checker_type_to_string(signature->param_types[i]));
            type_checker_destroy_inference_result(arg_result);
            return -1;
        }
        
        type_checker_destroy_inference_result(arg_result);
        arg = arg->next;
    }
    
    return 0;
}

/* 检查参数数量 */
int type_checker_check_parameter_count(type_checker_context_t *ctx,
                                      const function_signature_t *signature,
                                      uint32_t actual_count) {
    if (!ctx || !signature) {
        return -1;
    }
    
    if (signature->is_variadic) {
        /* 可变参数函数：实际参数数量必须>=形参数量 */
        if (actual_count < signature->param_count) {
            type_checker_add_error(ctx, TYPE_CHECK_FUNCTION_MISMATCH, NULL,
                                  "Function '%s' expects at least %u arguments, got %u",
                                  signature->name, signature->param_count, actual_count);
            return -1;
        }
    } else {
        /* 固定参数函数：数量必须完全匹配 */
        if (actual_count != signature->param_count) {
            type_checker_add_error(ctx, TYPE_CHECK_FUNCTION_MISMATCH, NULL,
                                  "Function '%s' expects %u arguments, got %u",
                                  signature->name, signature->param_count, actual_count);
            return -1;
        }
    }
    
    return 0;
}

/* ========== 内置类型和函数 ========== */

/* 初始化内置类型 */
int type_checker_init_builtin_types(type_checker_context_t *ctx) {
    if (!ctx) {
        return -1;
    }
    
    /* 创建基础类型 */
    ctx->builtin_types[TYPE_BOOL_ID] = create_basic_type(TYPE_BOOL_ID);
    ctx->builtin_types[TYPE_INT_ID] = create_basic_type(TYPE_INT_ID);
    ctx->builtin_types[TYPE_DINT_ID] = create_basic_type(TYPE_DINT_ID);
    ctx->builtin_types[TYPE_REAL_ID] = create_basic_type(TYPE_REAL_ID);
    ctx->builtin_types[TYPE_STRING_ID] = create_basic_type(TYPE_STRING_ID);
    ctx->builtin_types[TYPE_TIME_ID] = create_basic_type(TYPE_TIME_ID);
    
    ctx->builtin_type_count = 6;
    
    /* 设置类型兼容性规则 */
    ctx->compatibility_count = 0;
    
    /* INT -> REAL 隐式转换 */
    type_compatibility_t *compat = &ctx->compatibility_table[ctx->compatibility_count++];
    compat->from_type = ctx->builtin_types[TYPE_INT_ID];
    compat->to_type = ctx->builtin_types[TYPE_REAL_ID];
    compat->is_implicit_allowed = true;
    compat->is_explicit_allowed = true;
    
    /* REAL -> INT 显式转换 */
    compat = &ctx->compatibility_table[ctx->compatibility_count++];
    compat->from_type = ctx->builtin_types[TYPE_REAL_ID];
    compat->to_type = ctx->builtin_types[TYPE_INT_ID];
    compat->is_implicit_allowed = false;
    compat->is_explicit_allowed = true;
    
    /* DINT -> REAL 隐式转换 */
    compat = &ctx->compatibility_table[ctx->compatibility_count++];
    compat->from_type = ctx->builtin_types[TYPE_DINT_ID];
    compat->to_type = ctx->builtin_types[TYPE_REAL_ID];
    compat->is_implicit_allowed = true;
    compat->is_explicit_allowed = true;
    
    /* INT -> DINT 隐式转换 */
    compat = &ctx->compatibility_table[ctx->compatibility_count++];
    compat->from_type = ctx->builtin_types[TYPE_INT_ID];
    compat->to_type = ctx->builtin_types[TYPE_DINT_ID];
    compat->is_implicit_allowed = true;
    compat->is_explicit_allowed = true;
    
    return 0;
}

/* 获取内置类型 */
type_info_t* type_checker_get_builtin_type(type_checker_context_t *ctx, int type_id) {
    if (!ctx || type_id < 0 || type_id >= (int)ctx->builtin_type_count) {
        return NULL;
    }
    
    return ctx->builtin_types[type_id];
}

/* 注册内置函数 */
int type_checker_register_builtin_functions(type_checker_context_t *ctx) {
    if (!ctx) {
        return -1;
    }
    
    /* 获取基础类型 */
    type_info_t *int_type = type_checker_get_builtin_type(ctx, TYPE_INT_ID);
    type_info_t *real_type = type_checker_get_builtin_type(ctx, TYPE_REAL_ID);
    type_info_t *bool_type = type_checker_get_builtin_type(ctx, TYPE_BOOL_ID);
    type_info_t *string_type = type_checker_get_builtin_type(ctx, TYPE_STRING_ID);
    
    /* 数学函数 */
    type_checker_add_builtin_function(ctx, "ABS", int_type, 1, int_type);
    type_checker_add_builtin_function(ctx, "SIN", real_type, 1, real_type);
    type_checker_add_builtin_function(ctx, "COS", real_type, 1, real_type);
    type_checker_add_builtin_function(ctx, "TAN", real_type, 1, real_type);
    type_checker_add_builtin_function(ctx, "SQRT", real_type, 1, real_type);
    type_checker_add_builtin_function(ctx, "MIN", int_type, 2, int_type, int_type);
    type_checker_add_builtin_function(ctx, "MAX", int_type, 2, int_type, int_type);
    
    /* 类型转换函数 */
    type_checker_add_builtin_function(ctx, "REAL_TO_INT", int_type, 1, real_type);
    type_checker_add_builtin_function(ctx, "INT_TO_REAL", real_type, 1, int_type);
    type_checker_add_builtin_function(ctx, "BOOL_TO_INT", int_type, 1, bool_type);
    type_checker_add_builtin_function(ctx, "INT_TO_STRING", string_type, 1, int_type);
    
    /* 字符串函数 */
    type_checker_add_builtin_function(ctx, "LEN", int_type, 1, string_type);
    type_checker_add_builtin_function(ctx, "CONCAT", string_type, 2, string_type, string_type);
    type_checker_add_builtin_function(ctx, "LEFT", string_type, 2, string_type, int_type);
    type_checker_add_builtin_function(ctx, "RIGHT", string_type, 2, string_type, int_type);
    
    return 0;
}

/* 添加内置函数 */
int type_checker_add_builtin_function(type_checker_context_t *ctx, const char *name,
                                     type_info_t *return_type, uint32_t param_count, ...) {
    if (!ctx || !name || !return_type || ctx->function_count >= 128) {
        return -1;
    }
    
    function_signature_t *sig = &ctx->function_signatures[ctx->function_count];
    
    sig->name = mmgr_alloc_string(name);
    sig->return_type = return_type;
    sig->param_count = param_count;
    sig->is_builtin = true;
    sig->is_variadic = false;
    
    if (param_count > 0) {
        sig->param_types = (type_info_t**)mmgr_alloc_general(sizeof(type_info_t*) * param_count);
        if (!sig->param_types) {
            return -1;
        }
        
        va_list args;
        va_start(args, param_count);
        for (uint32_t i = 0; i < param_count; i++) {
            sig->param_types[i] = va_arg(args, type_info_t*);
        }
        va_end(args);
    } else {
        sig->param_types = NULL;
    }
    
    sig->param_names = NULL; /* 内置函数不需要参数名 */
    
    ctx->function_count++;
    
    return 0;
}

/* ========== 错误和警告管理 ========== */

/* 添加错误 */
void type_checker_add_error(type_checker_context_t *ctx, type_check_error_t error_code,
                           source_location_t *location, const char *format, ...) {
    if (!ctx || !format) return;
    
    va_list args;
    va_start(args, format);
    type_checker_add_message(ctx, TYPE_SEVERITY_ERROR, error_code, location, format, args);
    va_end(args);
    
    ctx->error_count++;
}

/* 添加警告 */
void type_checker_add_warning(type_checker_context_t *ctx, type_check_error_t error_code,
                             source_location_t *location, const char *format, ...) {
    if (!ctx || !format) return;
    
    va_list args;
    va_start(args, format);
    type_checker_add_message(ctx, TYPE_SEVERITY_WARNING, error_code, location, format, args);
    va_end(args);
    
    ctx->warning_count++;
    
    /* 如果警告视为错误 */
    if (ctx->options.warnings_as_errors) {
        ctx->error_count++;
    }
}

/* 添加信息 */
void type_checker_add_info(type_checker_context_t *ctx, const char *format, ...) {
    if (!ctx || !format) return;
    
    va_list args;
    va_start(args, format);
    type_checker_add_message(ctx, TYPE_SEVERITY_INFO, TYPE_CHECK_SUCCESS, NULL, format, args);
    va_end(args);
}

/* 获取错误数量 */
uint32_t type_checker_get_error_count(const type_checker_context_t *ctx) {
    return ctx ? ctx->error_count : 0;
}

/* 获取警告数量 */
uint32_t type_checker_get_warning_count(const type_checker_context_t *ctx) {
    return ctx ? ctx->warning_count : 0;
}

/* 检查是否有错误 */
bool type_checker_has_errors(const type_checker_context_t *ctx) {
    return ctx ? (ctx->error_count > 0) : false;
}

/* 打印消息 */
void type_checker_print_messages(const type_checker_context_t *ctx) {
    if (!ctx) return;
    
    type_check_message_t *msg = ctx->messages;
    while (msg) {
        printf("[%s] ", type_checker_severity_to_string(msg->severity));
        
        if (msg->location && msg->location->filename) {
            printf("%s:%d:%d: ", msg->location->filename, 
                   msg->location->line, msg->location->column);
        }
        
        printf("%s\n", msg->message);
        msg = msg->next;
    }
}

/* 打印总结 */
void type_checker_print_summary(const type_checker_context_t *ctx) {
    if (!ctx) return;
    
    printf("=== 类型检查总结 ===\n");
    printf("检查的节点数: %u\n", ctx->statistics.nodes_checked);
    printf("解析的符号数: %u\n", ctx->statistics.symbols_resolved);
    printf("推断的类型数: %u\n", ctx->statistics.types_inferred);
    printf("错误数: %u\n", ctx->error_count);
    printf("警告数: %u\n", ctx->warning_count);
    
    if (ctx->error_count == 0) {
        printf("类型检查通过！\n");
    } else {
        printf("发现 %u 个错误，需要修复\n", ctx->error_count);
    }
    printf("==================\n");
}

/* 获取消息字符串 */
char* type_checker_get_message_string(const type_checker_context_t *ctx) {
    if (!ctx || !ctx->messages) {
        return NULL;
    }
    
    /* 简化实现：返回第一个错误消息 */
    return mmgr_alloc_string(ctx->messages->message);
}

/* 清理消息 */
void type_checker_clear_messages(type_checker_context_t *ctx) {
    if (!ctx) return;
    
    /* 消息内存由MMGR管理，不需要显式释放 */
    ctx->messages = NULL;
    ctx->error_count = 0;
    ctx->warning_count = 0;
}

/* ========== 工具函数 ========== */

/* AST节点类型转字符串 */
const char* type_checker_ast_node_to_string(ast_node_type_t node_type) {
    switch (node_type) {
        case AST_PROGRAM: return "PROGRAM";
        case AST_VAR_DECL: return "VAR_DECL";
        case AST_VAR_ITEM: return "VAR_ITEM";
        case AST_FUNCTION_DECL: return "FUNCTION_DECL";
        case AST_ASSIGNMENT: return "ASSIGNMENT";
        case AST_IF_STMT: return "IF_STMT";
        case AST_FOR_STMT: return "FOR_STMT";
        case AST_WHILE_STMT: return "WHILE_STMT";
        case AST_RETURN_STMT: return "RETURN_STMT";
        case AST_BINARY_OP: return "BINARY_OP";
        case AST_UNARY_OP: return "UNARY_OP";
        case AST_LITERAL: return "LITERAL";
        case AST_IDENTIFIER: return "IDENTIFIER";
        case AST_DECLARATION_LIST: return "DECLARATION_LIST";
        case AST_STATEMENT_LIST: return "STATEMENT_LIST";
        default: return "UNKNOWN";
    }
}

/* 严重级别转字符串 */
const char* type_checker_severity_to_string(type_severity_t severity) {
    switch (severity) {
        case TYPE_SEVERITY_ERROR: return "ERROR";
        case TYPE_SEVERITY_WARNING: return "WARNING";
        case TYPE_SEVERITY_INFO: return "INFO";
        default: return "UNKNOWN";
    }
}

/* 错误码转换 */
const char* type_checker_error_to_string(type_check_error_t error_code) {
    switch (error_code) {
        case TYPE_CHECK_SUCCESS: return "成功";
        case TYPE_CHECK_TYPE_MISMATCH: return "类型不匹配";
        case TYPE_CHECK_UNDEFINED_SYMBOL: return "未定义符号";
        case TYPE_CHECK_REDEFINED_SYMBOL: return "重定义符号";
        case TYPE_CHECK_INVALID_OPERATION: return "无效操作";
        case TYPE_CHECK_INCOMPATIBLE_TYPES: return "不兼容类型";
        case TYPE_CHECK_INVALID_ASSIGNMENT: return "无效赋值";
        case TYPE_CHECK_FUNCTION_MISMATCH: return "函数签名不匹配";
        case TYPE_CHECK_RETURN_TYPE_MISMATCH: return "返回类型不匹配";
        case TYPE_CHECK_ARRAY_INDEX_ERROR: return "数组索引错误";
        case TYPE_CHECK_CONST_ASSIGNMENT: return "常量赋值错误";
        case TYPE_CHECK_MEMORY_ERROR: return "内存错误";
        default: return "未知错误";
    }
}