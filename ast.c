/*
 * IEC61131 抽象语法树 (AST) 实现
 * 实现语法树节点创建、管理和操作函数
 */

#include "ast.h"
#include "mmgr.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* 全局AST根节点和统计信息 */
static ast_node_t *g_ast_root = NULL;
static ast_static_t g_ast_stats = {0};

/* 内部辅助函数声明 */
static ast_node_t* ast_alloc_node(ast_node_type_t type);
static void ast_init_node(ast_node_t *node, ast_node_type_t type);
static void ast_update_static(ast_node_type_t type, bool is_creation);
static void ast_free_node_recursive(ast_node_t *node);

/* 分配AST节点 */
static ast_node_t* ast_alloc_node(ast_node_type_t type) {
    ast_node_t *node = (ast_node_t*)mmgr_alloc_ast_node(sizeof(ast_node_t));
    if (!node) {
        return NULL;
    }
    
    ast_init_node(node, type);
    ast_update_static(type, true);
    
    return node;
}

/* 初始化AST节点 */
static void ast_init_node(ast_node_t *node, ast_node_type_t type) {
    memset(node, 0, sizeof(ast_node_t));
    node->type = type;
    node->loc = NULL;
    node->data_type = NULL;
    node->next = NULL;
}

/* 更新统计信息 */
static void ast_update_static(ast_node_type_t type, bool is_creation) {
    if (is_creation) {
        g_ast_stats.total_nodes++;
        if (type < 64) {
            g_ast_stats.node_counts[type]++;
        }
        g_ast_stats.total_memory += sizeof(ast_node_t);
    } else {
        if (g_ast_stats.total_nodes > 0) {
            g_ast_stats.total_nodes--;
        }
        if (type < 64 && g_ast_stats.node_counts[type] > 0) {
            g_ast_stats.node_counts[type]--;
        }
        if (g_ast_stats.total_memory >= sizeof(ast_node_t)) {
            g_ast_stats.total_memory -= sizeof(ast_node_t);
        }
    }
}

/* ========== 顶层节点创建函数 ========== */

/* 创建程序节点 */
ast_node_t* ast_create_program(const char *name, ast_node_t *imports, 
                              ast_node_t *declarations, ast_node_t *statements) {
    ast_node_t *node = ast_alloc_node(AST_PROGRAM);
    if (!node) return NULL;
    
    node->data.program.name = mmgr_alloc_string(name);
    node->data.program.imports = imports;
    node->data.program.declarations = declarations;
    node->data.program.statements = statements;
    
    return node;
}

/* 创建库节点 */
ast_node_t* ast_create_library(const char *name, const char *version, 
                              ast_node_t *declarations, ast_node_t *exports) {
    ast_node_t *node = ast_alloc_node(AST_LIBRARY);
    if (!node) return NULL;
    
    node->data.library.name = mmgr_alloc_string(name);
    node->data.library.version = mmgr_alloc_string(version);
    node->data.library.declarations = declarations;
    node->data.library.exports = exports;
    
    return node;
}

/* 创建导入节点 */
ast_node_t* ast_create_import(const char *library_name, const char *alias, 
                             const char *path, ast_node_t *symbols, bool is_selective) {
    ast_node_t *node = ast_alloc_node(AST_IMPORT);
    if (!node) return NULL;
    
    node->data.import.library_name = mmgr_alloc_string(library_name);
    node->data.import.alias = alias ? mmgr_alloc_string(alias) : NULL;
    node->data.import.path = path ? mmgr_alloc_string(path) : NULL;
    node->data.import.symbols = symbols;
    node->data.import.is_selective = is_selective;
    
    return node;
}

/* ========== 声明节点创建函数 ========== */

/* 创建变量声明节点 */
ast_node_t* ast_create_var_declaration(var_category_t category, ast_node_t *var_list) {
    ast_node_t *node = ast_alloc_node(AST_VAR_DECL);
    if (!node) return NULL;
    
    node->data.var_decl.category = category;
    node->data.var_decl.var_list = var_list;
    
    return node;
}

/* 创建变量项节点 */
ast_node_t* ast_create_var_item(const char *name, struct type_info *type, 
                                ast_node_t *init_value, ast_node_t *array_bounds) {
    ast_node_t *node = ast_alloc_node(AST_VAR_ITEM);
    if (!node) return NULL;
    
    node->data.var_item.name = mmgr_alloc_string(name);
    node->data.var_item.type = type;
    node->data.var_item.init_value = init_value;
    node->data.var_item.array_bounds = array_bounds;
    node->data.var_item.is_constant = false;
    node->data.var_item.is_retain = false;
    
    return node;
}

/* 创建函数声明节点 */
ast_node_t* ast_create_function_declaration(const char *name, function_type_t func_type,
                                           ast_node_t *parameters, struct type_info *return_type,
                                           ast_node_t *declarations, ast_node_t *statements) {
    ast_node_t *node = ast_alloc_node(AST_FUNCTION_DECL);
    if (!node) return NULL;
    
    node->data.function_decl.name = mmgr_alloc_string(name);
    node->data.function_decl.func_type = func_type;
    node->data.function_decl.parameters = parameters;
    node->data.function_decl.return_type = return_type;
    node->data.function_decl.declarations = declarations;
    node->data.function_decl.statements = statements;
    node->data.function_decl.is_exported = false;
    node->data.function_decl.builtin_impl = NULL;
    
    return node;
}

/* 创建参数节点 */
ast_node_t* ast_create_parameter(const char *name, struct type_info *type, 
                                param_category_t category, ast_node_t *default_value) {
    ast_node_t *node = ast_alloc_node(AST_PARAM_DECL);
    if (!node) return NULL;
    
    node->data.param_decl.name = mmgr_alloc_string(name);
    node->data.param_decl.type = type;
    node->data.param_decl.category = category;
    node->data.param_decl.default_value = default_value;
    
    return node;
}

/* ========== 语句节点创建函数 ========== */

/* 创建赋值语句节点 */
ast_node_t* ast_create_assignment(ast_node_t *target, ast_node_t *value) {
    ast_node_t *node = ast_alloc_node(AST_ASSIGNMENT);
    if (!node) return NULL;
    
    node->data.assignment.target = target;
    node->data.assignment.value = value;
    
    return node;
}

/* 创建IF语句节点 */
ast_node_t* ast_create_if_statement(ast_node_t *condition, ast_node_t *then_stmt,
                                   ast_node_t *elsif_list, ast_node_t *else_stmt) {
    ast_node_t *node = ast_alloc_node(AST_IF_STMT);
    if (!node) return NULL;
    
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.then_stmt = then_stmt;
    node->data.if_stmt.elsif_list = elsif_list;
    node->data.if_stmt.else_stmt = else_stmt;
    
    return node;
}

/* 创建CASE语句节点 */
ast_node_t* ast_create_case_statement(ast_node_t *selector, ast_node_t *case_list,
                                     ast_node_t *else_stmt) {
    ast_node_t *node = ast_alloc_node(AST_CASE_STMT);
    if (!node) return NULL;
    
    node->data.case_stmt.selector = selector;
    node->data.case_stmt.case_list = case_list;
    node->data.case_stmt.else_stmt = else_stmt;
    
    return node;
}

/* 创建FOR循环节点 */
ast_node_t* ast_create_for_statement(const char *var_name, ast_node_t *start_value,
                                    ast_node_t *end_value, ast_node_t *by_value,
                                    ast_node_t *statements) {
    ast_node_t *node = ast_alloc_node(AST_FOR_STMT);
    if (!node) return NULL;
    
    node->data.for_stmt.var_name = mmgr_alloc_string(var_name);
    node->data.for_stmt.start_value = start_value;
    node->data.for_stmt.end_value = end_value;
    node->data.for_stmt.by_value = by_value;
    node->data.for_stmt.statements = statements;
    
    return node;
}

/* 创建WHILE循环节点 */
ast_node_t* ast_create_while_statement(ast_node_t *condition, ast_node_t *statements) {
    ast_node_t *node = ast_alloc_node(AST_WHILE_STMT);
    if (!node) return NULL;
    
    node->data.while_stmt.condition = condition;
    node->data.while_stmt.statements = statements;
    
    return node;
}

/* 创建RETURN语句节点 */
ast_node_t* ast_create_return_statement(ast_node_t *return_value) {
    ast_node_t *node = ast_alloc_node(AST_RETURN_STMT);
    if (!node) return NULL;
    
    node->data.return_stmt.return_value = return_value;
    
    return node;
}

/* ========== 表达式节点创建函数 ========== */

/* 创建二元操作节点 */
ast_node_t* ast_create_binary_operation(operator_type_t op, ast_node_t *left, ast_node_t *right) {
    ast_node_t *node = ast_alloc_node(AST_BINARY_OP);
    if (!node) return NULL;
    
    node->data.binary_op.op = op;
    node->data.binary_op.left = left;
    node->data.binary_op.right = right;
    
    return node;
}

/* 创建一元操作节点 */
ast_node_t* ast_create_unary_operation(operator_type_t op, ast_node_t *operand) {
    ast_node_t *node = ast_alloc_node(AST_UNARY_OP);
    if (!node) return NULL;
    
    node->data.unary_op.op = op;
    node->data.unary_op.operand = operand;
    
    return node;
}

/* 创建函数调用节点 */
ast_node_t* ast_create_function_call(const char *function_name, const char *library_name,
                                    ast_node_t *arguments) {
    ast_node_t *node = ast_alloc_node(AST_FUNCTION_CALL);
    if (!node) return NULL;
    
    node->data.function_call.function_name = mmgr_alloc_string(function_name);
    node->data.function_call.library_name = library_name ? mmgr_alloc_string(library_name) : NULL;
    node->data.function_call.arguments = arguments;
    node->data.function_call.symbol_ref = NULL;
    
    return node;
}

/* 创建数组访问节点 */
ast_node_t* ast_create_array_access(ast_node_t *array, ast_node_t *indices) {
    ast_node_t *node = ast_alloc_node(AST_ARRAY_ACCESS);
    if (!node) return NULL;
    
    node->data.array_access.array = array;
    node->data.array_access.indices = indices;
    
    return node;
}

/* 创建字面量节点 */
ast_node_t* ast_create_literal(literal_type_t type, const void *value) {
    ast_node_t *node = ast_alloc_node(AST_LITERAL);
    if (!node) return NULL;
    
    node->data.literal.literal_type = type;
    
    switch (type) {
        case LITERAL_INT:
            node->data.literal.value.int_val = *(const int32_t*)value;
            break;
        case LITERAL_REAL:
            node->data.literal.value.real_val = *(const double*)value;
            break;
        case LITERAL_BOOL:
            node->data.literal.value.bool_val = *(const bool*)value;
            break;
        case LITERAL_STRING:
        case LITERAL_WSTRING:
            node->data.literal.value.string_val = mmgr_alloc_string((const char*)value);
            break;
        case LITERAL_TIME:
        case LITERAL_DATE:
        case LITERAL_TOD:
        case LITERAL_DT:
            node->data.literal.value.string_val = mmgr_alloc_string((const char*)value);
            break;
    }
    
    return node;
}

/* 创建标识符节点 */
ast_node_t* ast_create_identifier(const char *name) {
    ast_node_t *node = ast_alloc_node(AST_IDENTIFIER);
    if (!node) return NULL;
    
    node->data.identifier.name = mmgr_alloc_string(name);
    node->data.identifier.symbol_ref = NULL;
    
    return node;
}

/* ========== 兼容性函数 - 与词法分析器匹配 ========== */

/* 创建列表节点 */
ast_node_t* ast_create_list(ast_node_type_t list_type) {
    ast_node_t *node = ast_alloc_node(list_type);
    if (!node) return NULL;
    
    node->data.list.items = NULL;
    node->data.list.count = 0;
    node->data.list.capacity = 0;
    
    return node;
}

/* 向列表追加项目 */
ast_node_t* ast_list_append(ast_node_t *list, ast_node_t *item) {
    if (!list || !item) return list;
    
    /* 简化实现：使用链表 */
    if (!list->data.list.items) {
        /* 第一个项目 */
        list->data.list.items = (ast_node_t**)mmgr_alloc_ast_node(sizeof(ast_node_t*));
        if (!list->data.list.items) return list;
        
        list->data.list.items[0] = item;
        list->data.list.count = 1;
        list->data.list.capacity = 1;
    } else {
        /* 使用next指针链接（简化实现） */
        ast_node_t *current = list->data.list.items[list->data.list.count - 1];
        while (current->next) {
            current = current->next;
        }
        current->next = item;
        list->data.list.count++;
    }
    
    return list;
}

/* 追加导入 */
ast_node_t* append_import(ast_node_t *list, ast_node_t *node) {
    if (!list) {
        list = ast_create_list(AST_IMPORT_LIST);
    }
    return ast_list_append(list, node);
}

/* ========== AST遍历函数 ========== */

/* 前序遍历 */
void ast_traverse_preorder(ast_node_t *node, ast_visitor_fn visitor, void *context) {
    if (!node || !visitor) return;
    
    ast_visit_result_t result = visitor(node, context);
    if (result == AST_VISIT_STOP) return;
    if (result == AST_VISIT_SKIP) return;
    
    /* 遍历子节点（根据节点类型） */
    switch (node->type) {
        case AST_PROGRAM:
            if (node->data.program.imports)
                ast_traverse_preorder(node->data.program.imports, visitor, context);
            if (node->data.program.declarations)
                ast_traverse_preorder(node->data.program.declarations, visitor, context);
            if (node->data.program.statements)
                ast_traverse_preorder(node->data.program.statements, visitor, context);
            break;
            
        case AST_BINARY_OP:
            if (node->data.binary_op.left)
                ast_traverse_preorder(node->data.binary_op.left, visitor, context);
            if (node->data.binary_op.right)
                ast_traverse_preorder(node->data.binary_op.right, visitor, context);
            break;
            
        case AST_FUNCTION_CALL:
            if (node->data.function_call.arguments)
                ast_traverse_preorder(node->data.function_call.arguments, visitor, context);
            break;
            
        /* 其他节点类型... */
        default:
            break;
    }
    
    /* 遍历链表中的下一个节点 */
    if (node->next) {
        ast_traverse_preorder(node->next, visitor, context);
    }
}

/* ========== AST节点管理函数 ========== */

/* 释放AST节点 */
void ast_free_node(ast_node_t *node) {
    if (!node) return;
    
    ast_free_node_recursive(node);
    ast_update_static(node->type, false);
}

/* 递归释放节点 */
static void ast_free_node_recursive(ast_node_t *node) {
    if (!node) return;
    
    /* 释放子节点（根据节点类型） */
    switch (node->type) {
        case AST_PROGRAM:
            ast_free_node_recursive(node->data.program.imports);
            ast_free_node_recursive(node->data.program.declarations);
            ast_free_node_recursive(node->data.program.statements);
            break;
            
        case AST_BINARY_OP:
            ast_free_node_recursive(node->data.binary_op.left);
            ast_free_node_recursive(node->data.binary_op.right);
            break;
            
        case AST_FUNCTION_CALL:
            ast_free_node_recursive(node->data.function_call.arguments);
            break;
            
        /* 其他节点类型... */
        default:
            break;
    }
    
    /* 释放链表中的下一个节点 */
    if (node->next) {
        ast_free_node_recursive(node->next);
    }
    
    /* 节点本身由MMGR管理，这里不需要显式释放 */
}

/* ========== AST调试和打印函数 ========== */

/* 打印AST树 */
void ast_print_tree(ast_node_t *node, int indent) {
    if (!node) return;
    
    /* 打印缩进 */
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    
    /* 打印节点信息 */
    printf("%s", ast_node_type_name(node->type));
    
    switch (node->type) {
        case AST_IDENTIFIER:
            printf(": %s", node->data.identifier.name);
            break;
        case AST_LITERAL:
            switch (node->data.literal.literal_type) {
                case LITERAL_INT:
                    printf(": %d", node->data.literal.value.int_val);
                    break;
                case LITERAL_REAL:
                    printf(": %f", node->data.literal.value.real_val);
                    break;
                case LITERAL_STRING:
                    printf(": \"%s\"", node->data.literal.value.string_val);
                    break;
                case LITERAL_BOOL:
                    printf(": %s", node->data.literal.value.bool_val ? "TRUE" : "FALSE");
                    break;
                default:
                    break;
            }
            break;
        case AST_BINARY_OP:
            printf(": %s", ast_operator_name(node->data.binary_op.op));
            break;
        default:
            break;
    }
    
    printf("\n");
    
    /* 递归打印子节点 */
    switch (node->type) {
        case AST_PROGRAM:
            if (node->data.program.imports) {
                ast_print_tree(node->data.program.imports, indent + 1);
            }
            if (node->data.program.declarations) {
                ast_print_tree(node->data.program.declarations, indent + 1);
            }
            if (node->data.program.statements) {
                ast_print_tree(node->data.program.statements, indent + 1);
            }
            break;
            
        case AST_BINARY_OP:
            if (node->data.binary_op.left) {
                ast_print_tree(node->data.binary_op.left, indent + 1);
            }
            if (node->data.binary_op.right) {
                ast_print_tree(node->data.binary_op.right, indent + 1);
            }
            break;
            
        /* 其他节点类型... */
        default:
            break;
    }
    
    /* 打印链表中的下一个节点 */
    if (node->next) {
        ast_print_tree(node->next, indent);
    }
}

/* 获取节点类型名称 - 扩展支持所有类型 */
const char* ast_node_type_name(ast_node_type_t type) {
    switch (type) {
        case AST_PROGRAM: return "PROGRAM";
        case AST_LIBRARY: return "LIBRARY";
        case AST_IMPORT: return "IMPORT";
        case AST_IMPORT_LIST: return "IMPORT_LIST";
        case AST_VAR_DECL: return "VAR_DECL";
        case AST_VAR_ITEM: return "VAR_ITEM";
        case AST_FUNCTION_DECL: return "FUNCTION_DECL";
        case AST_FUNCTION_BLOCK_DECL: return "FUNCTION_BLOCK_DECL";
        case AST_PARAM_DECL: return "PARAM_DECL";
        case AST_TYPE_DECL: return "TYPE_DECL";
        case AST_ASSIGNMENT: return "ASSIGNMENT";
        case AST_IF_STMT: return "IF_STMT";
        case AST_ELSIF_STMT: return "ELSIF_STMT";
        case AST_CASE_STMT: return "CASE_STMT";
        case AST_CASE_ITEM: return "CASE_ITEM";
        case AST_FOR_STMT: return "FOR_STMT";
        case AST_WHILE_STMT: return "WHILE_STMT";
        case AST_REPEAT_STMT: return "REPEAT_STMT";
        case AST_RETURN_STMT: return "RETURN_STMT";
        case AST_EXIT_STMT: return "EXIT_STMT";
        case AST_CONTINUE_STMT: return "CONTINUE_STMT";
        case AST_EXPRESSION_STMT: return "EXPRESSION_STMT";
        case AST_BINARY_OP: return "BINARY_OP";
        case AST_UNARY_OP: return "UNARY_OP";
        case AST_FUNCTION_CALL: return "FUNCTION_CALL";
        case AST_ARRAY_ACCESS: return "ARRAY_ACCESS";
        case AST_MEMBER_ACCESS: return "MEMBER_ACCESS";
        case AST_LITERAL: return "LITERAL";
        case AST_IDENTIFIER: return "IDENTIFIER";
        case AST_BASIC_TYPE: return "BASIC_TYPE";
        case AST_ARRAY_TYPE: return "ARRAY_TYPE";
        case AST_STRUCT_TYPE: return "STRUCT_TYPE";
        case AST_ENUM_TYPE: return "ENUM_TYPE";
        case AST_USER_TYPE: return "USER_TYPE";
        case AST_DECLARATION_LIST: return "DECLARATION_LIST";
        case AST_STATEMENT_LIST: return "STATEMENT_LIST";
        case AST_EXPRESSION_LIST: return "EXPRESSION_LIST";
        case AST_PARAMETER_LIST: return "PARAMETER_LIST";
        case AST_CASE_LIST: return "CASE_LIST";
        case AST_ELSIF_LIST: return "ELSIF_LIST";
        case AST_STRUCT_MEMBER_LIST: return "STRUCT_MEMBER_LIST";
        case AST_ENUM_VALUE_LIST: return "ENUM_VALUE_LIST";
        default: return "UNKNOWN";
    }
}

/* 获取操作符名称 - 扩展支持所有操作符 */
const char* ast_operator_name(operator_type_t op) {
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
        case OP_NEG: return "-(unary)";
        case OP_POS: return "+(unary)";
        case OP_ASSIGN: return ":=";
        default: return "UNKNOWN_OP";
    }
}

/* ========== 全局AST根节点管理 ========== */

/* 设置AST根节点 */
void ast_set_root(ast_node_t *root) {
    g_ast_root = root;
}

/* 获取AST根节点 */
ast_node_t* ast_get_root(void) {
    return g_ast_root;
}

/* ========== AST统计信息 ========== */

/* 获取统计信息 */
ast_static_t* ast_get_static(void) {
    return &g_ast_stats;
}

/* 重置统计信息 - 修正函数名 */
void ast_reset_statistics(void) {
    memset(&g_ast_stats, 0, sizeof(ast_static_t));
}

/* ========== 新增功能函数 ========== */

/* 创建ELSIF语句节点 */
ast_node_t* ast_create_elsif_statement(ast_node_t *condition, ast_node_t *statements) {
    ast_node_t *node = ast_alloc_node(AST_ELSIF_STMT);
    if (!node) return NULL;
    
    node->data.elsif_stmt.condition = condition;
    node->data.elsif_stmt.statements = statements;
    
    return node;
}

/* 创建CASE项节点 */
ast_node_t* ast_create_case_item(ast_node_t *values, ast_node_t *statements) {
    ast_node_t *node = ast_alloc_node(AST_CASE_ITEM);
    if (!node) return NULL;
    
    node->data.case_item.values = values;
    node->data.case_item.statements = statements;
    
    return node;
}

/* 创建REPEAT循环节点 */
ast_node_t* ast_create_repeat_statement(ast_node_t *statements, ast_node_t *condition) {
    ast_node_t *node = ast_alloc_node(AST_REPEAT_STMT);
    if (!node) return NULL;
    
    node->data.repeat_stmt.statements = statements;
    node->data.repeat_stmt.condition = condition;
    
    return node;
}

/* 创建EXIT语句节点 */
ast_node_t* ast_create_exit_statement(void) {
    ast_node_t *node = ast_alloc_node(AST_EXIT_STMT);
    if (!node) return NULL;
    
    return node;
}

/* 创建CONTINUE语句节点 */
ast_node_t* ast_create_continue_statement(void) {
    ast_node_t *node = ast_alloc_node(AST_CONTINUE_STMT);
    if (!node) return NULL;
    
    return node;
}

/* 创建表达式语句节点 */
ast_node_t* ast_create_expression_statement(ast_node_t *expression) {
    ast_node_t *node = ast_alloc_node(AST_EXPRESSION_STMT);
    if (!node) return NULL;
    
    /* 简化：直接返回表达式节点 */
    return expression;
}

/* 创建成员访问节点 */
ast_node_t* ast_create_member_access(ast_node_t *object, const char *member_name) {
    ast_node_t *node = ast_alloc_node(AST_MEMBER_ACCESS);
    if (!node) return NULL;
    
    node->data.member_access.object = object;
    node->data.member_access.member_name = mmgr_alloc_string(member_name);
    
    return node;
}

/* 创建类型声明节点 */
ast_node_t* ast_create_type_declaration(const char *name, struct type_info *type_def) {
    ast_node_t *node = ast_alloc_node(AST_TYPE_DECL);
    if (!node) return NULL;
    
    node->data.type_decl.name = mmgr_alloc_string(name);
    node->data.type_decl.type_def = type_def;
    
    return node;
}

/* 创建数组类型节点 */
ast_node_t* ast_create_array_type(struct type_info *element_type, ast_node_t *bounds) {
    ast_node_t *node = ast_alloc_node(AST_ARRAY_TYPE);
    if (!node) return NULL;
    
    node->data.array_type.element_type = element_type;
    node->data.array_type.bounds = bounds;
    
    return node;
}

/* 创建结构体类型节点 */
ast_node_t* ast_create_struct_type(ast_node_t *members) {
    ast_node_t *node = ast_alloc_node(AST_STRUCT_TYPE);
    if (!node) return NULL;
    
    node->data.struct_type.members = members;
    
    return node;
}

/* 创建枚举类型节点 */
ast_node_t* ast_create_enum_type(ast_node_t *values) {
    ast_node_t *node = ast_alloc_node(AST_ENUM_TYPE);
    if (!node) return NULL;
    
    node->data.enum_type.values = values;
    
    return node;
}