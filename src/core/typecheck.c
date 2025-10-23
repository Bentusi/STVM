/**
 * @file typecheck.c
 * @brief ST语言类型检查器实现 (简化版)
 */

#include "typecheck.h"
#include "mmgr.h"
#include "error.h"
#include <stdio.h>
#include <string.h>

// 前向声明
static TypeInfo* check_statement(TypeChecker* checker, ASTNode* stmt);
static TypeInfo* check_expression(TypeChecker* checker, ASTNode* expr);
static TypeInfo* check_statement_list(TypeChecker* checker, ASTNode* stmt);

ErrorCode typecheck_init(TypeChecker* checker, SymbolTable* symtbl, LibraryManager* libmgr) {
    if (!checker || !symtbl) {
        return ERR_TYPE;
    }
    
    checker->symtbl = symtbl;
    checker->libmgr = libmgr;
    checker->current_function = NULL;
    checker->error_count = 0;
    
    // 注册内置函数 PRINT (可变参数函数)
    // 创建一个 VOID 返回类型的函数签名
    TypeInfo* void_type = type_info_create(TYPE_VOID);
    Symbol* print_sym = symtbl_define_function(symtbl, "PRINT", void_type, NULL, -1);
    if (print_sym) {
        print_sym->param_count = -1;  // -1 表示可变参数
    }
    
    // 注册内置函数 SYSTEM (1个参数：STRING，返回 INT)
    TypeInfo* int_type = type_info_create(TYPE_INT);
    Symbol* system_sym = symtbl_define_function(symtbl, "SYSTEM", int_type, NULL, 1);
    if (system_sym) {
        system_sym->param_count = 1;  // 1个参数
    }
    
    // 注册质量位访问函数
    Symbol* get_quality_sym = symtbl_define_function(symtbl, "GetQuality", int_type, NULL, 1);
    if (get_quality_sym) {
        get_quality_sym->param_count = 1;
    }
    
    TypeInfo* qreal_type = type_info_create(TYPE_QREAL);
    Symbol* make_qreal_sym = symtbl_define_function(symtbl, "MakeQReal", qreal_type, NULL, 2);
    if (make_qreal_sym) {
        make_qreal_sym->param_count = 2;
    }
    
    TypeInfo* real_type = type_info_create(TYPE_REAL);
    Symbol* to_real_sym = symtbl_define_function(symtbl, "ToReal", real_type, NULL, 1);
    if (to_real_sym) {
        to_real_sym->param_count = 1;
    }
    
    // SetQuality函数比较特殊，返回修改后的值，类型与输入相同
    TypeInfo* void_type2 = type_info_create(TYPE_VOID);
    Symbol* set_quality_sym = symtbl_define_function(symtbl, "SetQuality", void_type2, NULL, 2);
    if (set_quality_sym) {
        set_quality_sym->param_count = 2;
    }
    
    return OK;
}

void typecheck_cleanup(TypeChecker* checker) {
    if (!checker) return;
    checker->symtbl = NULL;
    checker->current_function = NULL;
}

ErrorCode typecheck_program(TypeChecker* checker, ASTNode* program) {
    if (!checker || !program) {
        return ERR_TYPE;
    }
    
    if (program->type != AST_PROGRAM) {
        fprintf(stderr, "Type error: Expected program node\n");
        return ERR_TYPE;
    }
    
    // 处理导入
    if (program->data.program.imports && checker->libmgr) {
        ASTNode* import = program->data.program.imports;
        while (import) {
            if (import->type == AST_IMPORT) {
                const char* module = import->data.import.module_name;
                char** symbols = import->data.import.symbols;
                int count = import->data.import.symbol_count;
                char** aliases = import->data.import.aliases;
                
                // 加载库
                ErrorCode err = libmgr_load_library(checker->libmgr, module);
                if (err != OK) {
                    fprintf(stderr, "Type error: Failed to load library '%s'\n", module);
                    checker->error_count++;
                    import = import->next;
                    continue;
                }
                
                // 导入符号
                if (symbols == NULL || count == 0) {
                    // 导入所有符号
                    err = libmgr_import_all(checker->libmgr, module);
                    if (err != OK) {
                        fprintf(stderr, "Type error: Failed to import from library '%s'\n", module);
                        checker->error_count++;
                    }
                } else {
                    // 导入指定符号
                    for (int i = 0; i < count; i++) {
                        const char* symbol = symbols[i];
                        const char* alias = (aliases && aliases[i]) ? aliases[i] : NULL;
                        
                        // 检查是否是模块别名（"*"）
                        if (strcmp(symbol, "*") == 0 && alias) {
                            // 这是模块别名，暂时跳过（需要在libmgr中实现）
                            printf("[typecheck] Module alias not yet implemented: %s\n", alias);
                        } else {
                            err = libmgr_import_symbol(checker->libmgr, module, symbol, alias);
                            if (err != OK) {
                                fprintf(stderr, "Type error: Failed to import symbol '%s' from '%s'\n", 
                                        symbol, module);
                                checker->error_count++;
                            }
                        }
                    }
                }
            }
            import = import->next;
        }
    }
    
    // 检查声明
    if (program->data.program.var_decls) {
        ASTNode* decl = program->data.program.var_decls;
        while (decl) {
            if (decl->type == AST_VAR_DECL) {
                const char* name = decl->data.var_decl.name;
                TypeInfo* type = decl->data.var_decl.type;
                bool is_const = decl->data.var_decl.is_const;
                
                // 注册变量到符号表
                Symbol* sym = symtbl_define_variable(checker->symtbl, name, type, is_const);
                if (!sym) {
                    fprintf(stderr, "Type error: Cannot define variable '%s'\n", name);
                    checker->error_count++;
                } else {
                    // 如果是外部I/O变量，设置相应标志和地址
                    if (decl->data.var_decl.is_external) {
                        sym->is_external = true;
                        sym->io_address = decl->data.var_decl.io_address ? 
                                         mmgr_strdup(decl->data.var_decl.io_address) : NULL;
                    }
                }
                
                // 如果有初始化表达式，检查类型
                if (decl->data.var_decl.initializer) {
                    TypeInfo* init_type = check_expression(checker, decl->data.var_decl.initializer);
                    if (init_type && !type_info_can_convert(type, init_type)) {
                        fprintf(stderr, "Type error: Cannot initialize variable '%s' with incompatible type\n", name);
                        checker->error_count++;
                    }
                }
            }
            decl = decl->next;
        }
    }
    
    // 检查函数
    if (program->data.program.functions) {
        ASTNode* func = program->data.program.functions;
        while (func) {
            if (func->type == AST_FUNCTION_DECL) {
                typecheck_function(checker, func);
            }
            func = func->next;
        }
    }
    
    // 检查主程序语句
    if (program->data.program.body) {
        check_statement_list(checker, program->data.program.body);
    }
    
    return checker->error_count == 0 ? OK : ERR_TYPE;
}

ErrorCode typecheck_function(TypeChecker* checker, ASTNode* func_decl) {
    if (!checker || !func_decl) {
        return ERR_TYPE;
    }
    
    if (func_decl->type != AST_FUNCTION_DECL) {
        fprintf(stderr, "Type error: Expected function declaration\n");
        return ERR_TYPE;
    }
    
    const char* func_name = func_decl->data.function_decl.name;
    TypeInfo* return_type = func_decl->data.function_decl.return_type;
    ASTNode* params = func_decl->data.function_decl.params;
    
    // 计数参数
    int param_count = 0;
    ASTNode* p = params;
    while (p) {
        param_count++;
        p = p->next;
    }
    
    // 收集参数类型
    TypeInfo** param_types = NULL;
    if (param_count > 0) {
        param_types = (TypeInfo**)mmgr_alloc(sizeof(TypeInfo*) * param_count);
        p = params;
        for (int i = 0; i < param_count; i++) {
            if (p->type == AST_VAR_DECL) {
                param_types[i] = p->data.var_decl.type;
            }
            p = p->next;
        }
    }
    
    // 注册函数
    Symbol* func_sym = symtbl_define_function(checker->symtbl, func_name, return_type, param_types, param_count);
    if (!func_sym) {
        fprintf(stderr, "Type error: Cannot define function '%s'\n", func_name);
        checker->error_count++;
        if (param_types) mmgr_free(param_types);
        return ERR_NAME;
    }
    
    if (param_types) mmgr_free(param_types);
    
    // 保存当前函数上下文
    Symbol* prev_func = checker->current_function;
    checker->current_function = func_sym;
    
    // 先处理函数内的静态变量声明（VAR块）—— 在进入作用域之前
    // 这些变量使用函数名作为命名空间，存储在全局区但有函数作用域
    if (func_decl->data.function_decl.declarations) {
        ASTNode* decl = func_decl->data.function_decl.declarations;
        while (decl) {
            if (decl->type == AST_VAR_DECL && decl->data.var_decl.is_global) {
                const char* name = decl->data.var_decl.name;
                TypeInfo* type = decl->data.var_decl.type;
                bool is_const = decl->data.var_decl.is_const;
                
                // 定义为函数作用域的静态变量
                Symbol* sym = symtbl_define_static_variable(checker->symtbl, name, type, is_const, func_name);
                if (!sym) {
                    fprintf(stderr, "Type error: Cannot define static variable '%s' in function '%s'\n", 
                            name, func_name);
                    checker->error_count++;
                } else {
                    // 如果是外部I/O变量，设置相应标志和地址
                    if (decl->data.var_decl.is_external) {
                        sym->is_external = true;
                        sym->io_address = decl->data.var_decl.io_address ? 
                                         mmgr_strdup(decl->data.var_decl.io_address) : NULL;
                    }
                }
                
                if (decl->data.var_decl.initializer) {
                    TypeInfo* init_type = check_expression(checker, decl->data.var_decl.initializer);
                    if (init_type && !type_info_can_convert(type, init_type)) {
                        fprintf(stderr, "Type error: Cannot initialize variable '%s' with incompatible type\n", name);
                        checker->error_count++;
                    }
                }
            }
            decl = decl->next;
        }
    }
    
    // 进入函数作用域
    symtbl_enter_scope(checker->symtbl);
    
    // 注册参数
    p = params;
    while (p) {
        if (p->type == AST_VAR_DECL) {
            const char* param_name = p->data.var_decl.name;
            TypeInfo* param_type = p->data.var_decl.type;
            if (!symtbl_define_parameter(checker->symtbl, param_name, param_type)) {
                fprintf(stderr, "Type error: Cannot define parameter '%s'\n", param_name);
                checker->error_count++;
            }
        }
        p = p->next;
    }
    
    // 注册函数名作为返回值变量（IEC 61131-3 特性）
    // 仅当函数有返回类型时
    if (return_type != NULL) {
        if (!symtbl_define_variable(checker->symtbl, func_name, return_type, false)) {
            fprintf(stderr, "Type error: Cannot define return value variable '%s'\n", func_name);
            checker->error_count++;
        }
    }
    
    // 处理局部变量声明（VAR_LOCAL块）
    if (func_decl->data.function_decl.declarations) {
        ASTNode* decl = func_decl->data.function_decl.declarations;
        while (decl) {
            if (decl->type == AST_VAR_DECL && !decl->data.var_decl.is_global) {
                const char* name = decl->data.var_decl.name;
                TypeInfo* type = decl->data.var_decl.type;
                bool is_const = decl->data.var_decl.is_const;
                
                // 在当前（函数）作用域定义变量
                Symbol* sym = symtbl_define_variable(checker->symtbl, name, type, is_const);
                if (!sym) {
                    fprintf(stderr, "Type error: Cannot define local variable '%s'\n", name);
                    checker->error_count++;
                } else {
                    // 如果是外部I/O变量，设置相应标志和地址
                    if (decl->data.var_decl.is_external) {
                        sym->is_external = true;
                        sym->io_address = decl->data.var_decl.io_address ? 
                                         mmgr_strdup(decl->data.var_decl.io_address) : NULL;
                    }
                }
                
                if (decl->data.var_decl.initializer) {
                    TypeInfo* init_type = check_expression(checker, decl->data.var_decl.initializer);
                    if (init_type && !type_info_can_convert(type, init_type)) {
                        fprintf(stderr, "Type error: Cannot initialize variable '%s' with incompatible type\n", name);
                        checker->error_count++;
                    }
                }
            }
            decl = decl->next;
        }
    }
    
    // 检查函数体
    if (func_decl->data.function_decl.body) {
        check_statement_list(checker, func_decl->data.function_decl.body);
    }
    
    // 离开函数作用域
    symtbl_leave_scope(checker->symtbl);
    
    // 恢复上下文
    checker->current_function = prev_func;
    
    return checker->error_count == 0 ? OK : ERR_TYPE;
}

static TypeInfo* check_statement_list(TypeChecker* checker, ASTNode* stmt) {
    TypeInfo* last_type = NULL;
    
    while (stmt) {
        last_type = check_statement(checker, stmt);
        stmt = stmt->next;
    }
    
    return last_type;
}

static TypeInfo* check_statement(TypeChecker* checker, ASTNode* stmt) {
    if (!stmt) return NULL;
    
    switch (stmt->type) {
        case AST_ASSIGN: {
            // 检查赋值语句
            TypeInfo* target_type = check_expression(checker, stmt->data.assign.target);
            TypeInfo* value_type = check_expression(checker, stmt->data.assign.value);
            
            // 特殊处理：函数体内对函数名的赋值是设置返回值
            if (target_type && target_type->base_type == TYPE_FUNCTION && 
                checker->current_function && 
                stmt->data.assign.target->type == AST_IDENTIFIER) {
                
                const char* target_name = stmt->data.assign.target->data.identifier.name;
                if (strcmp(target_name, checker->current_function->name) == 0) {
                    // 检查返回值类型是否匹配
                    TypeInfo* return_type = target_type->func_info.return_type;
                    if (return_type && value_type && !type_info_can_convert(return_type, value_type)) {
                        fprintf(stderr, "Type error: Return value type mismatch\n");
                        checker->error_count++;
                    }
                    return NULL;
                }
            }
            
            if (target_type && value_type && !type_info_can_convert(target_type, value_type)) {
                fprintf(stderr, "Type error: Assignment type mismatch\n");
                checker->error_count++;
            }
            return NULL;
        }
        
        case AST_IF: {
            // 检查条件表达式
            TypeInfo* cond_type = check_expression(checker, stmt->data.if_stmt.condition);
            if (cond_type && cond_type->base_type != TYPE_BOOL) {
                fprintf(stderr, "Type error: IF condition must be boolean\n");
                checker->error_count++;
            }
            
            // 检查then分支
            if (stmt->data.if_stmt.then_branch) {
                check_statement_list(checker, stmt->data.if_stmt.then_branch);
            }
            
            // 检查else分支
            if (stmt->data.if_stmt.else_branch) {
                check_statement_list(checker, stmt->data.if_stmt.else_branch);
            }
            return NULL;
        }
        
        case AST_WHILE: {
            // 检查条件
            TypeInfo* cond_type = check_expression(checker, stmt->data.while_stmt.condition);
            if (cond_type && cond_type->base_type != TYPE_BOOL) {
                fprintf(stderr, "Type error: WHILE condition must be boolean\n");
                checker->error_count++;
            }
            
            // 检查循环体
            if (stmt->data.while_stmt.body) {
                check_statement_list(checker, stmt->data.while_stmt.body);
            }
            return NULL;
        }
        
        case AST_FOR: {
            // 检查循环变量、起始值、结束值
            TypeInfo* start_type = check_expression(checker, stmt->data.for_stmt.start);
            TypeInfo* end_type = check_expression(checker, stmt->data.for_stmt.end);
            
            if (start_type && start_type->base_type != TYPE_INT) {
                fprintf(stderr, "Type error: FOR start value must be integer\n");
                checker->error_count++;
            }
            if (end_type && end_type->base_type != TYPE_INT) {
                fprintf(stderr, "Type error: FOR end value must be integer\n");
                checker->error_count++;
            }
            
            if (stmt->data.for_stmt.step) {
                TypeInfo* step_type = check_expression(checker, stmt->data.for_stmt.step);
                if (step_type && step_type->base_type != TYPE_INT) {
                    fprintf(stderr, "Type error: FOR step value must be integer\n");
                    checker->error_count++;
                }
            }
            
            // 检查循环体
            if (stmt->data.for_stmt.body) {
                check_statement_list(checker, stmt->data.for_stmt.body);
            }
            return NULL;
        }
        
        case AST_RETURN: {
            TypeInfo* ret_type = NULL;
            if (stmt->data.return_stmt.value) {
                ret_type = check_expression(checker, stmt->data.return_stmt.value);
            }
            
            // 检查返回类型是否匹配
            if (checker->current_function) {
                TypeInfo* expected_type = checker->current_function->type;
                if (ret_type && expected_type && !type_info_can_convert(expected_type, ret_type)) {
                    fprintf(stderr, "Type error: Return type mismatch\n");
                    checker->error_count++;
                }
            }
            return ret_type;
        }
        
        case AST_CASE: {
            // 检查 CASE 表达式
            TypeInfo* case_expr_type = check_expression(checker, stmt->data.case_stmt.expression);
            if (!case_expr_type) {
                fprintf(stderr, "Type error: CASE expression has no type\n");
                checker->error_count++;
                return NULL;
            }
            
            // 检查每个 CASE 分支
            if (stmt->data.case_stmt.cases) {
                for (int i = 0; i < stmt->data.case_stmt.case_count; i++) {
                    ASTNode* case_elem = stmt->data.case_stmt.cases[i];
                    if (!case_elem || case_elem->type != AST_CASE_ELEMENT) {
                        continue;
                    }
                    
                    // 检查标签列表中的每个标签
                    ASTNode* label = case_elem->data.case_element.labels;
                    while (label) {
                        TypeInfo* label_type = check_expression(checker, label);
                        if (label_type && !type_info_can_convert(case_expr_type, label_type)) {
                            fprintf(stderr, "Type error: CASE label type does not match expression type\n");
                            checker->error_count++;
                        }
                        label = label->next;
                    }
                    
                    // 检查分支语句
                    if (case_elem->data.case_element.statements) {
                        check_statement_list(checker, case_elem->data.case_element.statements);
                    }
                }
            }
            
            // 检查 ELSE 分支（如果有）
            if (stmt->data.case_stmt.default_case) {
                check_statement_list(checker, stmt->data.case_stmt.default_case);
            }
            
            return NULL;
        }
        
        case AST_CASE_ELEMENT: {
            // CASE_ELEMENT 由 AST_CASE 处理，不应单独出现
            fprintf(stderr, "Type error: CASE_ELEMENT cannot appear outside CASE statement\n");
            checker->error_count++;
            return NULL;
        }
        
        case AST_FUNCTION_CALL: {
            // 函数调用作为语句
            check_expression(checker, stmt);
            return NULL;
        }
        
        default:
            // 其他语句类型
            return NULL;
    }
}

static TypeInfo* check_expression(TypeChecker* checker, ASTNode* expr) {
    if (!expr) return NULL;
    
    switch (expr->type) {
        case AST_LITERAL: {
            // 字面量有内置类型
            switch (expr->data.literal.value.type) {
                case TYPE_INT:
                    return type_info_create(TYPE_INT);
                case TYPE_REAL:
                    return type_info_create(TYPE_REAL);
                case TYPE_BOOL:
                    return type_info_create(TYPE_BOOL);
                case TYPE_STRING:
                    return type_info_create(TYPE_STRING);
                default:
                    return NULL;
            }
        }
        
        case AST_IDENTIFIER: {
            // 查找标识符
            Symbol* sym = symtbl_lookup(checker->symtbl, expr->data.identifier.name);
            if (!sym) {
                fprintf(stderr, "Type error: Undefined identifier '%s'\n", expr->data.identifier.name);
                checker->error_count++;
                return NULL;
            }
            return sym->type;
        }
        
        case AST_BINARY_OP: {
            // 二元运算
            TypeInfo* left_type = check_expression(checker, expr->data.binary_op.left);
            TypeInfo* right_type = check_expression(checker, expr->data.binary_op.right);
            
            if (!left_type || !right_type) {
                return NULL;
            }
            
            BinaryOp op = expr->data.binary_op.op;
            TypeInfo* result_type = NULL;
            
            // 获取基础类型（去除质量化）
            DataType left_base = is_qualified_type(left_type->base_type) ? 
                                 get_base_type(left_type->base_type) : left_type->base_type;
            DataType right_base = is_qualified_type(right_type->base_type) ? 
                                  get_base_type(right_type->base_type) : right_type->base_type;
            
            // 检查是否有质量化类型参与运算
            bool has_qualified = is_qualified_type(left_type->base_type) || 
                                is_qualified_type(right_type->base_type);
            
            // 算术运算: + - * / MOD
            if (op >= BINOP_ADD && op <= BINOP_MOD) {
                if (left_base == TYPE_INT && right_base == TYPE_INT) {
                    // 如果有质量化类型参与，结果也是质量化的
                    result_type = type_info_create(has_qualified ? TYPE_QINT : TYPE_INT);
                } else if ((left_base == TYPE_INT || left_base == TYPE_REAL) &&
                           (right_base == TYPE_INT || right_base == TYPE_REAL)) {
                    // 如果有质量化类型参与，结果也是质量化的
                    result_type = type_info_create(has_qualified ? TYPE_QREAL : TYPE_REAL);
                } else {
                    fprintf(stderr, "Type error: Arithmetic operation requires numeric types\n");
                    checker->error_count++;
                    return NULL;
                }
            }
            
            // 比较运算: = <> < <= > >=
            else if (op >= BINOP_EQ && op <= BINOP_GE) {
                if (!type_info_can_convert(left_type, right_type)) {
                    fprintf(stderr, "Type error: Comparison requires compatible types\n");
                    checker->error_count++;
                }
                result_type = type_info_create(TYPE_BOOL);
            }
            
            // 逻辑/位运算: AND OR XOR（符合 IEC 61131-3）
            // 可以用于BOOL（逻辑）或INT（位）类型
            else if (op >= BINOP_AND && op <= BINOP_XOR) {
                // BOOL AND BOOL -> BOOL（逻辑运算）
                if (left_type->base_type == TYPE_BOOL && right_type->base_type == TYPE_BOOL) {
                    result_type = type_info_create(TYPE_BOOL);
                }
                // INT AND INT -> INT（位运算）
                else if (left_type->base_type == TYPE_INT && right_type->base_type == TYPE_INT) {
                    result_type = type_info_create(TYPE_INT);
                }
                else {
                    fprintf(stderr, "Type error: AND/OR/XOR requires both operands to be BOOL or both INT\n");
                    checker->error_count++;
                    return NULL;
                }
            }
            
            // 位运算: BIT_AND BIT_OR BIT_XOR（仅INT）
            else if (op >= BINOP_BIT_AND && op <= BINOP_BIT_XOR) {
                if (left_type->base_type != TYPE_INT || right_type->base_type != TYPE_INT) {
                    fprintf(stderr, "Type error: Bitwise operation requires integer types\n");
                    checker->error_count++;
                    return NULL;
                }
                result_type = type_info_create(TYPE_INT);
            }
            
            // 移位运算: SHL SHR
            else if (op == BINOP_SHL || op == BINOP_SHR) {
                if (left_type->base_type != TYPE_INT || right_type->base_type != TYPE_INT) {
                    fprintf(stderr, "Type error: Shift operation requires integer types\n");
                    checker->error_count++;
                    return NULL;
                }
                result_type = type_info_create(TYPE_INT);
            }
            
            // 保存resolved_type到AST节点，供codegen使用
            if (result_type) {
                expr->resolved_type = result_type;
            }
            
            return result_type;
        }
        
        case AST_UNARY_OP: {
            // 一元运算
            TypeInfo* operand_type = check_expression(checker, expr->data.unary_op.operand);
            if (!operand_type) return NULL;
            
            UnaryOp op = expr->data.unary_op.op;
            TypeInfo* result_type = NULL;
            
            if (op == UNOP_NEG) {
                // 负号
                if (operand_type->base_type == TYPE_INT || operand_type->base_type == TYPE_REAL) {
                    result_type = operand_type;
                } else {
                    fprintf(stderr, "Type error: Negation requires numeric type\n");
                    checker->error_count++;
                    return NULL;
                }
            } else if (op == UNOP_NOT) {
                // NOT：可以是逻辑非（BOOL）或位取反（INT）
                if (operand_type->base_type == TYPE_BOOL) {
                    result_type = type_info_create(TYPE_BOOL);
                } else if (operand_type->base_type == TYPE_INT) {
                    result_type = type_info_create(TYPE_INT);
                } else {
                    fprintf(stderr, "Type error: NOT requires boolean or integer type\n");
                    checker->error_count++;
                    return NULL;
                }
            } else if (op == UNOP_BIT_NOT) {
                // 位取反：仅INT
                if (operand_type->base_type != TYPE_INT) {
                    fprintf(stderr, "Type error: Bitwise NOT requires integer type\n");
                    checker->error_count++;
                    return NULL;
                }
                result_type = type_info_create(TYPE_INT);
            }
            
            // 保存resolved_type到AST节点，供codegen使用
            if (result_type) {
                expr->resolved_type = result_type;
            }
            
            return result_type;
        }
        
        case AST_FUNCTION_CALL: {
            // 函数调用
            const char* func_name = expr->data.function_call.name;
            Symbol* func_sym = symtbl_lookup(checker->symtbl, func_name);
            
            // 如果在本地符号表中没找到，尝试从导入的符号中查找
            if (!func_sym && checker->libmgr) {
                ImportedSymbol* import = checker->libmgr->imports;
                while (import) {
                    // 检查原始名是否匹配
                    if (strcmp(import->original_name, func_name) == 0) {
                        func_sym = import->symbol;
                        // 重要：将AST中的函数名替换为完全限定名
                        // 这样代码生成器会使用正确的名称
                        mmgr_free((void*)expr->data.function_call.name);
                        expr->data.function_call.name = mmgr_strdup(import->name);
                        break;
                    }
                    import = import->next;
                }
            }
            
            if (!func_sym) {
                fprintf(stderr, "Type error: Undefined function '%s'\n", func_name);
                checker->error_count++;
                return NULL;
            }
            
            if (func_sym->kind != SYM_FUNCTION) {
                fprintf(stderr, "Type error: '%s' is not a function\n", func_name);
                checker->error_count++;
                return NULL;
            }
            
            // 检查参数数量
            int arg_count = expr->data.function_call.arg_count;
            // -1 表示可变参数函数，跳过参数数量检查
            if (func_sym->param_count != -1 && arg_count != func_sym->param_count) {
                fprintf(stderr, "Type error: Function '%s' expects %d arguments, got %d\n",
                        func_name, func_sym->param_count, arg_count);
                checker->error_count++;
            }
            
            // 简化版: 不检查每个参数类型
            // 返回函数返回类型（而不是函数类型本身）
            if (func_sym->type && func_sym->type->base_type == TYPE_FUNCTION) {
                return func_sym->type->func_info.return_type;
            }
            return func_sym->type;
        }
        
        case AST_ARRAY_ACCESS: {
            // 数组访问
            TypeInfo* array_type = check_expression(checker, expr->data.array_access.array);
            if (!array_type) return NULL;
            
            if (array_type->base_type != TYPE_ARRAY) {
                fprintf(stderr, "Type error: Array access requires array type\n");
                checker->error_count++;
                return NULL;
            }
            
            // 检查索引
            TypeInfo* index_type = check_expression(checker, expr->data.array_access.index);
            if (index_type && index_type->base_type != TYPE_INT) {
                fprintf(stderr, "Type error: Array index must be integer\n");
                checker->error_count++;
                return NULL;
            }
            
            // 返回元素类型
            return array_type->array_info.elem_type;
        }
        
        case AST_MEMBER_ACCESS: {
            // 成员访问
            ASTNode* object = expr->data.member_access.object;
            MemberType member = expr->data.member_access.member;
            
            // 检查 object 是否为标识符
            if (!object || object->type != AST_IDENTIFIER) {
                fprintf(stderr, "Type error: Member access requires variable identifier\n");
                checker->error_count++;
                return NULL;
            }
            
            const char* var_name = object->data.identifier.name;
            
            // 查找变量
            Symbol* sym = symtbl_lookup(checker->symtbl, var_name);
            if (!sym) {
                fprintf(stderr, "Type error: Undefined variable '%s'\n", var_name);
                checker->error_count++;
                return NULL;
            }
            
            // 检查变量是否为质量化类型
            if (!is_qualified_type(sym->type->base_type)) {
                fprintf(stderr, "Type error: Member access requires qualified type, '%s' is %s\n", 
                        var_name, type_to_string(sym->type->base_type));
                checker->error_count++;
                return NULL;
            }
            
            // 根据成员类型返回相应的类型
            if (member == MEMBER_VAL) {
                // VAL 成员返回对应的基础类型
                DataType base_type = get_base_type(sym->type->base_type);
                return type_info_create(base_type);
            } else if (member == MEMBER_QUALITY) {
                // QUALITY 成员总是返回 INT 类型
                return type_info_create(TYPE_INT);
            } else {
                fprintf(stderr, "Type error: Unknown member type\n");
                checker->error_count++;
                return NULL;
            }
        }
        
        default:
            return NULL;
    }
}
