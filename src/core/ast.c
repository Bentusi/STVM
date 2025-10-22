/**
 * @file ast.c
 * @brief 抽象语法树的实现
 * 
 * AST节点内存管理和所有权规则：
 * 
 * 1. 节点创建：
 *    - 所有ast_create_*函数返回的节点由调用者拥有
 *    - 调用者负责在不再需要时调用ast_free_node释放
 * 
 * 2. 子节点所有权：
 *    - 当节点A包含子节点B的指针时，A拥有B
 *    - 释放A时会自动递归释放B
 *    - 例如：program拥有declarations/functions/body
 * 
 * 3. TypeInfo所有权：
 *    - TypeInfo使用引用计数管理（在types.h中定义）
 *    - AST节点创建时会retain TypeInfo（增加ref_count）
 *    - AST节点释放时会release TypeInfo（减少ref_count）
 *    - 多个节点可以共享同一个TypeInfo
 * 
 * 4. 字符串所有权：
 *    - AST节点中的字符串（如name）由节点拥有
 *    - 创建时使用mmgr_strdup复制
 *    - 释放节点时会自动释放字符串
 * 
 * 5. 链表节点：
 *    - next指针连接的节点属于同一个列表
 *    - 释放头节点时会递归释放整个链表
 *    - 注意：不要手动释放链表中的中间节点
 * 
 * 6. resolved_type字段：
 *    - 由类型检查器设置
 *    - 使用TypeInfo引用计数管理
 *    - 释放节点时会自动release
 */

#include "ast.h"
#include "mmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief 创建AST节点的通用函数
 */
static ASTNode* ast_create_node(ASTNodeType type) {
    ASTNode* node = (ASTNode*)mmgr_alloc_from_pool(POOL_AST, sizeof(ASTNode));
    if (!node) return NULL;
    
    memset(node, 0, sizeof(ASTNode));
    node->type = type;
    node->next = NULL;
    node->resolved_type = NULL;
    
    return node;
}

/**
 * @brief 创建程序节点
 */
ASTNode* ast_create_program(const char* name, ASTNode* imports, ASTNode* declarations, ASTNode* functions, ASTNode* statements) {
    ASTNode* node = ast_create_node(AST_PROGRAM);
    if (!node) return NULL;
    
    node->data.program.name = name ? mmgr_strdup(name) : NULL;
    node->data.program.imports = imports;
    node->data.program.var_decls = declarations;
    node->data.program.functions = functions;
    node->data.program.body = statements;
    
    return node;
}

/**
 * @brief 创建变量声明节点
 */
ASTNode* ast_create_var_decl(const char* name, TypeInfo* type, ASTNode* initializer, bool is_const, bool is_global) {
    ASTNode* node = ast_create_node(AST_VAR_DECL);
    if (!node) return NULL;
    
    node->data.var_decl.name = name ? mmgr_strdup(name) : NULL;
    node->data.var_decl.type = type ? type_info_retain(type) : NULL;
    node->data.var_decl.initializer = initializer;
    node->data.var_decl.is_const = is_const;
    node->data.var_decl.is_global = is_global;
    node->data.var_decl.is_external = false;
    node->data.var_decl.io_address = NULL;
    
    return node;
}

/**
 * @brief 创建外部 I/O 变量声明节点
 */
ASTNode* ast_create_external_var_decl(const char* name, TypeInfo* type, const char* io_address) {
    ASTNode* node = ast_create_node(AST_VAR_DECL);
    if (!node) return NULL;
    
    node->data.var_decl.name = name ? mmgr_strdup(name) : NULL;
    node->data.var_decl.type = type ? type_info_retain(type) : NULL;
    node->data.var_decl.initializer = NULL;
    node->data.var_decl.is_const = false;
    node->data.var_decl.is_global = true;  // External variables are always global
    node->data.var_decl.is_external = true;
    node->data.var_decl.io_address = io_address ? mmgr_strdup(io_address) : NULL;
    
    return node;
}

/**
 * @brief 创建函数声明节点
 */
ASTNode* ast_create_function_decl(const char* name, ASTNode* parameters, TypeInfo* return_type,
                                   ASTNode* declarations, ASTNode* body) {
    ASTNode* node = ast_create_node(AST_FUNCTION_DECL);
    if (!node) return NULL;
    
    node->data.function_decl.name = name ? mmgr_strdup(name) : NULL;
    node->data.function_decl.params = parameters;
    node->data.function_decl.return_type = return_type ? type_info_retain(return_type) : NULL;
    node->data.function_decl.declarations = declarations;
    node->data.function_decl.body = body;
    
    return node;
}

/**
 * @brief 创建导入语句节点
 */
ASTNode* ast_create_import(const char* module_name, char** symbols, int symbol_count, char** aliases) {
    ASTNode* node = ast_create_node(AST_IMPORT);
    if (!node) return NULL;
    
    node->data.import.module_name = module_name ? mmgr_strdup(module_name) : NULL;
    node->data.import.symbols = symbols;
    node->data.import.symbol_count = symbol_count;
    node->data.import.aliases = aliases;
    
    return node;
}

/**
 * @brief 创建赋值语句节点
 */
ASTNode* ast_create_assign(ASTNode* target, ASTNode* value) {
    ASTNode* node = ast_create_node(AST_ASSIGN);
    if (!node) return NULL;
    
    node->data.assign.target = target;
    node->data.assign.value = value;
    
    return node;
}

/**
 * @brief 创建if语句节点
 */
ASTNode* ast_create_if(ASTNode* condition, ASTNode* then_branch, ASTNode* else_branch) {
    ASTNode* node = ast_create_node(AST_IF);
    if (!node) return NULL;
    
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.then_branch = then_branch;
    node->data.if_stmt.else_branch = else_branch;
    
    return node;
}

/**
 * @brief 创建while循环节点
 */
ASTNode* ast_create_while(ASTNode* condition, ASTNode* body) {
    ASTNode* node = ast_create_node(AST_WHILE);
    if (!node) return NULL;
    
    node->data.while_stmt.condition = condition;
    node->data.while_stmt.body = body;
    
    return node;
}

/**
 * @brief 创建for循环节点
 */
ASTNode* ast_create_for(const char* variable, ASTNode* start, ASTNode* end, ASTNode* step, ASTNode* body) {
    ASTNode* node = ast_create_node(AST_FOR);
    if (!node) return NULL;
    
    node->data.for_stmt.variable = variable ? mmgr_strdup(variable) : NULL;
    node->data.for_stmt.start = start;
    node->data.for_stmt.end = end;
    node->data.for_stmt.step = step;
    node->data.for_stmt.body = body;
    
    return node;
}

/**
 * @brief 创建return语句节点
 */
ASTNode* ast_create_return(ASTNode* value) {
    ASTNode* node = ast_create_node(AST_RETURN);
    if (!node) return NULL;
    
    node->data.return_stmt.value = value;
    
    return node;
}

/**
 * @brief 创建case语句节点
 */
ASTNode* ast_create_case(ASTNode* expression, ASTNode** cases, int case_count, ASTNode* default_case) {
    ASTNode* node = ast_create_node(AST_CASE);
    if (!node) return NULL;
    
    node->data.case_stmt.expression = expression;
    node->data.case_stmt.cases = cases;
    node->data.case_stmt.case_count = case_count;
    node->data.case_stmt.default_case = default_case;
    
    return node;
}

/**
 * @brief 创建case分支元素节点
 */
ASTNode* ast_create_case_element(ASTNode* labels, ASTNode* statements) {
    ASTNode* node = ast_create_node(AST_CASE_ELEMENT);
    if (!node) return NULL;
    
    node->data.case_element.labels = labels;
    node->data.case_element.statements = statements;
    
    return node;
}

/**
 * @brief 创建语句块节点
 */
ASTNode* ast_create_block(ASTNode* statements) {
    ASTNode* node = ast_create_node(AST_BLOCK);
    if (!node) return NULL;
    
    // 语句块直接使用第一个语句作为链表头
    node->next = statements;
    
    return node;
}

/**
 * @brief 创建二元运算节点
 */
ASTNode* ast_create_binary_op(BinaryOp op, ASTNode* left, ASTNode* right) {
    ASTNode* node = ast_create_node(AST_BINARY_OP);
    if (!node) return NULL;
    
    node->data.binary_op.op = op;
    node->data.binary_op.left = left;
    node->data.binary_op.right = right;
    
    return node;
}

/**
 * @brief 创建一元运算节点
 */
ASTNode* ast_create_unary_op(UnaryOp op, ASTNode* operand) {
    ASTNode* node = ast_create_node(AST_UNARY_OP);
    if (!node) return NULL;
    
    node->data.unary_op.op = op;
    node->data.unary_op.operand = operand;
    
    return node;
}

/**
 * @brief 创建字面量节点
 */
ASTNode* ast_create_literal(Value value) {
    ASTNode* node = ast_create_node(AST_LITERAL);
    if (!node) return NULL;
    
    node->data.literal.value = value;
    
    return node;
}

/**
 * @brief 创建标识符节点
 */
ASTNode* ast_create_identifier(const char* name) {
    ASTNode* node = ast_create_node(AST_IDENTIFIER);
    if (!node) return NULL;
    
    node->data.identifier.name = name ? mmgr_strdup(name) : NULL;
    
    return node;
}

/**
 * @brief 创建函数调用节点
 */
ASTNode* ast_create_function_call(const char* name, ASTNode** arguments, int arg_count) {
    ASTNode* node = ast_create_node(AST_FUNCTION_CALL);
    if (!node) return NULL;
    
    node->data.function_call.name = name ? mmgr_strdup(name) : NULL;
    node->data.function_call.arguments = arguments;
    node->data.function_call.arg_count = arg_count;
    
    return node;
}

/**
 * @brief 创建数组访问节点
 */
ASTNode* ast_create_array_access(ASTNode* array, ASTNode* index) {
    ASTNode* node = ast_create_node(AST_ARRAY_ACCESS);
    if (!node) return NULL;
    
    node->data.array_access.array = array;
    node->data.array_access.index = index;

    return node;
}

/**
 * @brief 创建成员访问节点（质量化类型的.VAL/.QUALITY）
 */
ASTNode* ast_create_member_access(ASTNode* object, MemberType member) {
    ASTNode* node = ast_create_node(AST_MEMBER_ACCESS);
    if (!node) return NULL;
    
    node->data.member_access.object = object;
    node->data.member_access.member = member;
    
    return node;
}

/**
 * @brief 将节点添加到列表末尾
 */
ASTNode* ast_append_node(ASTNode* list, ASTNode* node) {
    if (!node) return list;
    if (!list) return node;
    
    ASTNode* current = list;
    while (current->next) {
        current = current->next;
    }
    current->next = node;
    
    return list;
}

/**
 * @brief 释放AST节点及其子节点
 */
void ast_free_node(ASTNode* node) {
    if (!node) return;
    
    // 递归释放子节点
    switch (node->type) {
        case AST_PROGRAM:
            ast_free_node(node->data.program.var_decls);
            ast_free_node(node->data.program.functions);
            ast_free_node(node->data.program.body);
            if (node->data.program.name) mmgr_free(node->data.program.name);
            break;
            
        case AST_VAR_DECL:
            ast_free_node(node->data.var_decl.initializer);
            if (node->data.var_decl.name) mmgr_free(node->data.var_decl.name);
            if (node->data.var_decl.type) type_info_free(node->data.var_decl.type);
            if (node->data.var_decl.io_address) mmgr_free(node->data.var_decl.io_address);
            break;
            
        case AST_FUNCTION_DECL:
            ast_free_node(node->data.function_decl.params);
            ast_free_node(node->data.function_decl.declarations);
            ast_free_node(node->data.function_decl.body);
            if (node->data.function_decl.name) mmgr_free(node->data.function_decl.name);
            if (node->data.function_decl.return_type) type_info_free(node->data.function_decl.return_type);
            break;
            
        case AST_IMPORT:
            if (node->data.import.module_name) {
                mmgr_free(node->data.import.module_name);
            }
            // 释放符号数组和别名数组
            if (node->data.import.symbols) {
                for (int i = 0; i < node->data.import.symbol_count; i++) {
                    if (node->data.import.symbols[i]) {
                        mmgr_free(node->data.import.symbols[i]);
                    }
                }
                mmgr_free(node->data.import.symbols);
            }
            if (node->data.import.aliases) {
                for (int i = 0; i < node->data.import.symbol_count; i++) {
                    if (node->data.import.aliases[i]) {
                        mmgr_free(node->data.import.aliases[i]);
                    }
                }
                mmgr_free(node->data.import.aliases);
            }
            break;
            
        case AST_ASSIGN:
            ast_free_node(node->data.assign.target);
            ast_free_node(node->data.assign.value);
            break;
            
        case AST_IF:
            ast_free_node(node->data.if_stmt.condition);
            ast_free_node(node->data.if_stmt.then_branch);
            ast_free_node(node->data.if_stmt.else_branch);
            break;
            
        case AST_WHILE:
            ast_free_node(node->data.while_stmt.condition);
            ast_free_node(node->data.while_stmt.body);
            break;
            
        case AST_FOR:
            ast_free_node(node->data.for_stmt.start);
            ast_free_node(node->data.for_stmt.end);
            ast_free_node(node->data.for_stmt.step);
            ast_free_node(node->data.for_stmt.body);
            if (node->data.for_stmt.variable) mmgr_free(node->data.for_stmt.variable);
            break;
            
        case AST_RETURN:
            ast_free_node(node->data.return_stmt.value);
            break;
            
        case AST_CASE:
            ast_free_node(node->data.case_stmt.expression);
            if (node->data.case_stmt.cases) {
                for (int i = 0; i < node->data.case_stmt.case_count; i++) {
                    ast_free_node(node->data.case_stmt.cases[i]);
                }
                mmgr_free(node->data.case_stmt.cases);
            }
            ast_free_node(node->data.case_stmt.default_case);
            break;
            
        case AST_CASE_ELEMENT:
            ast_free_node(node->data.case_element.labels);
            ast_free_node(node->data.case_element.statements);
            break;
            
        case AST_BINARY_OP:
            ast_free_node(node->data.binary_op.left);
            ast_free_node(node->data.binary_op.right);
            break;
            
        case AST_UNARY_OP:
            ast_free_node(node->data.unary_op.operand);
            break;
            
        case AST_IDENTIFIER:
            if (node->data.identifier.name) mmgr_free(node->data.identifier.name);
            break;
            
        case AST_FUNCTION_CALL:
            if (node->data.function_call.name) mmgr_free(node->data.function_call.name);
            for (int i = 0; i < node->data.function_call.arg_count; i++) {
                ast_free_node(node->data.function_call.arguments[i]);
            }
            if (node->data.function_call.arguments) mmgr_free(node->data.function_call.arguments);
            break;
            
        case AST_ARRAY_ACCESS:
            ast_free_node(node->data.array_access.array);
            ast_free_node(node->data.array_access.index);
            break;
            
        case AST_MEMBER_ACCESS:
            ast_free_node(node->data.member_access.object);
            break;
            
        default:
            break;
    }
    
    // 释放链表中的下一个节点
    if (node->next) {
        ast_free_node(node->next);
    }
    
    // 释放解析后的类型信息
    if (node->resolved_type) {
        type_info_free(node->resolved_type);
    }
    
    mmgr_free(node);
}

/**
 * @brief 设置节点的源码位置
 */
void ast_set_location(ASTNode* node, const char* filename, int line, int column) {
    if (!node) return;
    
    node->location.filename = filename;
    node->location.line = line;
    node->location.column = column;
}

/**
 * @brief 获取二元运算符的字符串表示
 */
const char* binary_op_to_string(BinaryOp op) {
    static const char* op_strings[] = {
        "+", "-", "*", "/", "MOD",
        "=", "<>", "<", "<=", ">", ">=",
        "AND", "OR"
    };
    
    if (op >= 0 && op <= BINOP_OR) {
        return op_strings[op];
    }
    return "?";
}

/**
 * @brief 获取一元运算符的字符串表示
 */
const char* unary_op_to_string(UnaryOp op) {
    switch (op) {
        case UNOP_NEG: return "-";
        case UNOP_NOT: return "NOT";
        default: return "?";
    }
}

/**
 * @brief 打印缩进
 */
static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

/**
 * @brief 打印AST（调试用）
 */
void ast_print(ASTNode* node, int indent) {
    if (!node) return;
    
    print_indent(indent);
    
    switch (node->type) {
        case AST_PROGRAM:
            printf("PROGRAM: %s\n", node->data.program.name ? node->data.program.name : "<unnamed>");
            if (node->data.program.var_decls) {
                print_indent(indent); printf("  Declarations:\n");
                ast_print(node->data.program.var_decls, indent + 2);
            }
            if (node->data.program.functions) {
                print_indent(indent); printf("  Functions:\n");
                ast_print(node->data.program.functions, indent + 2);
            }
            if (node->data.program.body) {
                print_indent(indent); printf("  Statements:\n");
                ast_print(node->data.program.body, indent + 2);
            }
            break;
            
        case AST_VAR_DECL:
            printf("VAR_DECL: %s : %s\n", 
                   node->data.var_decl.name ? node->data.var_decl.name : "<unnamed>",
                   node->data.var_decl.type ? type_to_string(node->data.var_decl.type->base_type) : "<unknown>");
            if (node->data.var_decl.initializer) {
                print_indent(indent); printf("  Init:\n");
                ast_print(node->data.var_decl.initializer, indent + 2);
            }
            break;
            
        case AST_FUNCTION_DECL:
            printf("FUNCTION: %s\n", node->data.function_decl.name ? node->data.function_decl.name : "<unnamed>");
            break;
            
        case AST_ASSIGN:
            printf("ASSIGN\n");
            print_indent(indent); printf("  Target:\n");
            ast_print(node->data.assign.target, indent + 2);
            print_indent(indent); printf("  Value:\n");
            ast_print(node->data.assign.value, indent + 2);
            break;
            
        case AST_IF:
            printf("IF\n");
            print_indent(indent); printf("  Condition:\n");
            ast_print(node->data.if_stmt.condition, indent + 2);
            print_indent(indent); printf("  Then:\n");
            ast_print(node->data.if_stmt.then_branch, indent + 2);
            if (node->data.if_stmt.else_branch) {
                print_indent(indent); printf("  Else:\n");
                ast_print(node->data.if_stmt.else_branch, indent + 2);
            }
            break;
            
        case AST_WHILE:
            printf("WHILE\n");
            print_indent(indent); printf("  Condition:\n");
            ast_print(node->data.while_stmt.condition, indent + 2);
            print_indent(indent); printf("  Body:\n");
            ast_print(node->data.while_stmt.body, indent + 2);
            break;
            
        case AST_CASE:
            printf("CASE\n");
            print_indent(indent); printf("  Expression:\n");
            ast_print(node->data.case_stmt.expression, indent + 2);
            print_indent(indent); printf("  Cases (%d):\n", node->data.case_stmt.case_count);
            if (node->data.case_stmt.cases) {
                for (int i = 0; i < node->data.case_stmt.case_count; i++) {
                    print_indent(indent + 1); printf("Case[%d]:\n", i);
                    ast_print(node->data.case_stmt.cases[i], indent + 2);
                }
            }
            if (node->data.case_stmt.default_case) {
                print_indent(indent); printf("  Default:\n");
                ast_print(node->data.case_stmt.default_case, indent + 2);
            }
            break;
            
        case AST_CASE_ELEMENT:
            printf("CASE_ELEMENT\n");
            print_indent(indent); printf("  Labels:\n");
            ast_print(node->data.case_element.labels, indent + 2);
            print_indent(indent); printf("  Statements:\n");
            ast_print(node->data.case_element.statements, indent + 2);
            break;
            
        case AST_BINARY_OP:
            printf("BINARY_OP: %s\n", binary_op_to_string(node->data.binary_op.op));
            print_indent(indent); printf("  Left:\n");
            ast_print(node->data.binary_op.left, indent + 2);
            print_indent(indent); printf("  Right:\n");
            ast_print(node->data.binary_op.right, indent + 2);
            break;
            
        case AST_UNARY_OP:
            printf("UNARY_OP: %s\n", unary_op_to_string(node->data.unary_op.op));
            print_indent(indent); printf("  Operand:\n");
            ast_print(node->data.unary_op.operand, indent + 2);
            break;
            
        case AST_LITERAL: {
            char buffer[256];
            value_to_string(node->data.literal.value, buffer, sizeof(buffer));
            printf("LITERAL: %s\n", buffer);
            break;
        }
            
        case AST_IDENTIFIER:
            printf("IDENTIFIER: %s\n", node->data.identifier.name ? node->data.identifier.name : "<unnamed>");
            break;
            
        case AST_FUNCTION_CALL:
            printf("CALL: %s (%d args)\n", 
                   node->data.function_call.name ? node->data.function_call.name : "<unnamed>",
                   node->data.function_call.arg_count);
            for (int i = 0; i < node->data.function_call.arg_count; i++) {
                print_indent(indent); printf("  Arg[%d]:\n", i);
                ast_print(node->data.function_call.arguments[i], indent + 2);
            }
            break;
            
        default:
            printf("<unknown node type: %d>\n", node->type);
            break;
    }
    
    // 打印链表中的下一个节点
    if (node->next) {
        ast_print(node->next, indent);
    }
}

/**
 * @brief 访问者模式遍历AST
 */
void ast_visit(ASTNode* node, ASTVisitor visitor, void* context) {
    if (!node || !visitor) return;
    
    visitor(node, context);
    
    // 递归访问子节点（根据节点类型）
    switch (node->type) {
        case AST_PROGRAM:
            ast_visit(node->data.program.var_decls, visitor, context);
            ast_visit(node->data.program.functions, visitor, context);
            ast_visit(node->data.program.body, visitor, context);
            break;
            
        case AST_VAR_DECL:
            ast_visit(node->data.var_decl.initializer, visitor, context);
            break;
            
        case AST_FUNCTION_DECL:
            ast_visit(node->data.function_decl.params, visitor, context);
            ast_visit(node->data.function_decl.declarations, visitor, context);
            ast_visit(node->data.function_decl.body, visitor, context);
            break;
            
        case AST_ASSIGN:
            ast_visit(node->data.assign.target, visitor, context);
            ast_visit(node->data.assign.value, visitor, context);
            break;
            
        case AST_IF:
            ast_visit(node->data.if_stmt.condition, visitor, context);
            ast_visit(node->data.if_stmt.then_branch, visitor, context);
            ast_visit(node->data.if_stmt.else_branch, visitor, context);
            break;
            
        case AST_WHILE:
            ast_visit(node->data.while_stmt.condition, visitor, context);
            ast_visit(node->data.while_stmt.body, visitor, context);
            break;
            
        case AST_CASE:
            ast_visit(node->data.case_stmt.expression, visitor, context);
            if (node->data.case_stmt.cases) {
                for (int i = 0; i < node->data.case_stmt.case_count; i++) {
                    ast_visit(node->data.case_stmt.cases[i], visitor, context);
                }
            }
            ast_visit(node->data.case_stmt.default_case, visitor, context);
            break;
            
        case AST_CASE_ELEMENT:
            ast_visit(node->data.case_element.labels, visitor, context);
            ast_visit(node->data.case_element.statements, visitor, context);
            break;
            
        case AST_BINARY_OP:
            ast_visit(node->data.binary_op.left, visitor, context);
            ast_visit(node->data.binary_op.right, visitor, context);
            break;
            
        case AST_UNARY_OP:
            ast_visit(node->data.unary_op.operand, visitor, context);
            break;
            
        case AST_FUNCTION_CALL:
            for (int i = 0; i < node->data.function_call.arg_count; i++) {
                ast_visit(node->data.function_call.arguments[i], visitor, context);
            }
            break;
            
        case AST_ARRAY_ACCESS:
            ast_visit(node->data.array_access.array, visitor, context);
            ast_visit(node->data.array_access.index, visitor, context);
            break;
            
        default:
            break;
    }
    
    // 访问链表中的下一个节点
    if (node->next) {
        ast_visit(node->next, visitor, context);
    }
}
