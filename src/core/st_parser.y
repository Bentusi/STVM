%{
/**
 * @file st_parser.y
 * @brief ST语言语法分析器 (Bison)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ast.h"
#include "types.h"
#include "mmgr.h"

/* 需要 strdup 声明 */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

/* 外部声明 */
extern int yylex(void);
extern int yylineno;
extern int yycolumn;
extern char* yytext;
extern char* strdup(const char*);

void yyerror(const char* s);

/* 全局变量：解析结果 */
ASTNode* parse_result = NULL;

%}

/* 在生成的头文件中包含必要的类型定义 */
%code requires {
    #include "ast.h"
    #include "types.h"
}

/* 启用位置跟踪 */
%locations

/* 联合体定义：token和非终结符的值类型 */
%union {
    int64_t int_value;
    double real_value;
    int bool_value;
    char* string_value;
    ASTNode* ast_node;
    TypeInfo* type_info;
    BinaryOp binary_op;
    UnaryOp unary_op;
}

/* Token声明 */
%token TOKEN_PROGRAM TOKEN_END_PROGRAM TOKEN_BEGIN
%token TOKEN_FUNCTION TOKEN_END_FUNCTION
%token TOKEN_PRINT
%token TOKEN_VAR TOKEN_VAR_INPUT TOKEN_VAR_OUTPUT TOKEN_VAR_LOCAL TOKEN_VAR_EXTERNAL TOKEN_END_VAR TOKEN_AT
%token TOKEN_IF TOKEN_THEN TOKEN_ELSIF TOKEN_ELSE TOKEN_END_IF
%token TOKEN_CASE TOKEN_OF TOKEN_END_CASE
%token TOKEN_FOR TOKEN_TO TOKEN_BY TOKEN_DO TOKEN_END_FOR
%token TOKEN_WHILE TOKEN_END_WHILE
%token TOKEN_REPEAT TOKEN_UNTIL TOKEN_END_REPEAT
%token TOKEN_RETURN TOKEN_EXIT
%token TOKEN_IMPORT TOKEN_FROM TOKEN_AS

%token TOKEN_BOOL TOKEN_INT TOKEN_REAL TOKEN_STRING TOKEN_ARRAY

%token <bool_value> TOKEN_TRUE TOKEN_FALSE
%token <int_value> TOKEN_INTEGER_LITERAL
%token <real_value> TOKEN_REAL_LITERAL
%token <string_value> TOKEN_STRING_LITERAL TOKEN_IDENTIFIER

%token TOKEN_ASSIGN TOKEN_EQ TOKEN_NE TOKEN_LT TOKEN_LE TOKEN_GT TOKEN_GE
%token TOKEN_PLUS TOKEN_MINUS TOKEN_MULTIPLY TOKEN_DIVIDE TOKEN_MOD
%token TOKEN_AND TOKEN_OR TOKEN_XOR TOKEN_NOT
%token TOKEN_SHL TOKEN_SHR

%token TOKEN_SEMICOLON TOKEN_COLON TOKEN_COMMA TOKEN_DOT TOKEN_RANGE
%token TOKEN_LPAREN TOKEN_RPAREN TOKEN_LBRACKET TOKEN_RBRACKET

%token TOKEN_ERROR

/* 非终结符类型声明 */
%type <ast_node> program import_list import_decl
%type <ast_node> var_decl_list var_decl var_decl_item
%type <ast_node> function_list function_decl function_params function_local_vars function_var_decl_list external_var_decl_list
%type <ast_node> statement_list statement
%type <ast_node> assignment_stmt if_stmt elsif_list while_stmt for_stmt case_stmt return_stmt print_stmt
%type <ast_node> expression or_expr xor_expr and_expr comparison_expr
%type <ast_node> add_expr mult_expr unary_expr primary_expr
%type <ast_node> argument_list
%type <type_info> type_spec

/* 运算符优先级和结合性（符合 IEC 61131-3）*/
%left TOKEN_OR
%left TOKEN_XOR
%left TOKEN_AND
%left TOKEN_EQ TOKEN_NE
%left TOKEN_LT TOKEN_LE TOKEN_GT TOKEN_GE
%left TOKEN_PLUS TOKEN_MINUS
%left TOKEN_SHL TOKEN_SHR  /* 移位操作与加减同级 */
%left TOKEN_MULTIPLY TOKEN_DIVIDE TOKEN_MOD
%right TOKEN_NOT UNARY_MINUS

/* 起始符号 */
%start program

%%

/* 程序结构 */
program:
    TOKEN_PROGRAM TOKEN_IDENTIFIER
    import_list
    var_decl_list
    function_list
    statement_list
    TOKEN_END_PROGRAM
    {
        // ast_create_program(name, imports, declarations, functions, statements)
        $$ = ast_create_program($2, $3, $4, $5, $6);
        parse_result = $$;
    }
    | TOKEN_PROGRAM TOKEN_IDENTIFIER
    import_list
    var_decl_list
    function_list
    TOKEN_BEGIN
    statement_list
    TOKEN_END_PROGRAM
    {
        // 带 BEGIN 的程序结构
        $$ = ast_create_program($2, $3, $4, $5, $7);
        parse_result = $$;
    }
    ;

/* 导入声明列表 */
import_list:
    /* 空 */ { $$ = NULL; }
    | import_list import_decl
    {
        if ($1 == NULL) {
            $$ = $2;
        } else {
            ASTNode* last = $1;
            while (last->next) last = last->next;
            last->next = $2;
            $$ = $1;
        }
    }
    ;

import_decl:
    /* IMPORT 'module.stbc'; - 导入整个库 */
    TOKEN_IMPORT TOKEN_STRING_LITERAL TOKEN_SEMICOLON
    {
        $$ = ast_create_import($2, NULL, 0, NULL);
    }
    /* IMPORT 'module.stbc' AS alias; - 导入库并使用前缀 */
    | TOKEN_IMPORT TOKEN_STRING_LITERAL TOKEN_AS TOKEN_IDENTIFIER TOKEN_SEMICOLON
    {
        // 使用别名作为模块别名（存在symbols[0]中）
        char** symbols = (char**)malloc(sizeof(char*) * 1);
        char** aliases = (char**)malloc(sizeof(char*) * 1);
        symbols[0] = strdup("*");  // 特殊标记表示所有符号
        aliases[0] = $4;
        $$ = ast_create_import($2, symbols, 1, aliases);
    }
    /* IMPORT func FROM 'module.stbc'; - 导入单个函数 */
    | TOKEN_IMPORT TOKEN_IDENTIFIER TOKEN_FROM TOKEN_STRING_LITERAL TOKEN_SEMICOLON
    {
        char** symbols = (char**)malloc(sizeof(char*) * 1);
        symbols[0] = $2;
        $$ = ast_create_import($4, symbols, 1, NULL);
    }
    /* IMPORT func AS alias FROM 'module.stbc'; - 导入带别名 */
    | TOKEN_IMPORT TOKEN_IDENTIFIER TOKEN_AS TOKEN_IDENTIFIER TOKEN_FROM TOKEN_STRING_LITERAL TOKEN_SEMICOLON
    {
        char** symbols = (char**)malloc(sizeof(char*) * 1);
        char** aliases = (char**)malloc(sizeof(char*) * 1);
        symbols[0] = $2;
        aliases[0] = $4;
        $$ = ast_create_import($6, symbols, 1, aliases);
    }
    /* TODO: 支持多个符号导入 IMPORT func1, func2 FROM 'module.stbc'; */
    ;

/* 变量声明列表 */
var_decl_list:
    /* 空 */ { $$ = NULL; }
    | var_decl_list var_decl
    {
        if ($1 == NULL) {
            $$ = $2;
        } else {
            ASTNode* last = $1;
            while (last->next) last = last->next;
            last->next = $2;
            $$ = $1;
        }
    }
    ;

var_decl:
    TOKEN_VAR var_decl_item TOKEN_END_VAR
    {
        $$ = $2;
        // 注意：AST var_decl结构中没有is_input/is_output字段
        // 这些语义可以在后续的符号表中处理
    }
    | TOKEN_VAR_INPUT var_decl_item TOKEN_END_VAR
    {
        $$ = $2;
        // 输入变量：可以在后续类型检查时处理
    }
    | TOKEN_VAR_OUTPUT var_decl_item TOKEN_END_VAR
    {
        $$ = $2;
        // 输出变量：可以在后续类型检查时处理
    }
    | TOKEN_VAR_EXTERNAL external_var_decl_list TOKEN_END_VAR
    {
        $$ = $2;
        // 外部I/O变量：映射到硬件地址
    }
    ;

var_decl_item:
    TOKEN_IDENTIFIER TOKEN_COLON type_spec TOKEN_SEMICOLON
    {
        // ast_create_var_decl(name, type, initializer, is_const, is_global)
        // 程序级声明都是全局变量
        $$ = ast_create_var_decl($1, $3, NULL, false, true);
    }
    | TOKEN_IDENTIFIER TOKEN_COLON type_spec TOKEN_ASSIGN expression TOKEN_SEMICOLON
    {
        $$ = ast_create_var_decl($1, $3, $5, false, true);
    }
    | var_decl_item TOKEN_IDENTIFIER TOKEN_COLON type_spec TOKEN_SEMICOLON
    {
        ASTNode* new_decl = ast_create_var_decl($2, $4, NULL, false, true);
        ASTNode* last = $1;
        while (last->next) last = last->next;
        last->next = new_decl;
        $$ = $1;
    }
    | var_decl_item TOKEN_IDENTIFIER TOKEN_COLON type_spec TOKEN_ASSIGN expression TOKEN_SEMICOLON
    {
        ASTNode* new_decl = ast_create_var_decl($2, $4, $6, false, true);
        ASTNode* last = $1;
        while (last->next) last = last->next;
        last->next = new_decl;
        $$ = $1;
    }
    ;

/* 类型规范 */
type_spec:
    TOKEN_BOOL      { $$ = type_info_create(TYPE_BOOL); }
    | TOKEN_INT     { $$ = type_info_create(TYPE_INT); }
    | TOKEN_REAL    { $$ = type_info_create(TYPE_REAL); }
    | TOKEN_STRING  { $$ = type_info_create(TYPE_STRING); }
    | TOKEN_ARRAY TOKEN_LBRACKET TOKEN_INTEGER_LITERAL TOKEN_RANGE TOKEN_INTEGER_LITERAL TOKEN_RBRACKET TOKEN_OF type_spec
    {
        // type_info_create_array(elem_type, dimensions, sizes)
        int32_t size = (int32_t)($5 - $3 + 1);
        int32_t* sizes = (int32_t*)mmgr_alloc(sizeof(int32_t));
        sizes[0] = size;
        $$ = type_info_create_array($8, 1, sizes);
    }
    | TOKEN_ARRAY TOKEN_LBRACKET TOKEN_INTEGER_LITERAL TOKEN_RBRACKET TOKEN_OF type_spec
    {
        // 单索引数组语法: ARRAY[N] OF type
        // 范围为 0..N-1（但我们直接使用 N 作为大小）
        int32_t size = (int32_t)$3;
        int32_t* sizes = (int32_t*)mmgr_alloc(sizeof(int32_t));
        sizes[0] = size;
        $$ = type_info_create_array($6, 1, sizes);
    }
    ;

/* 函数声明列表 */
function_list:
    /* 空 */ { $$ = NULL; }
    | function_list function_decl
    {
        if ($1 == NULL) {
            $$ = $2;
        } else {
            ASTNode* last = $1;
            while (last->next) last = last->next;
            last->next = $2;
            $$ = $1;
        }
    }
    ;

function_decl:
    TOKEN_FUNCTION TOKEN_IDENTIFIER TOKEN_COLON type_spec
    function_params
    function_local_vars
    TOKEN_BEGIN
    statement_list
    TOKEN_END_FUNCTION
    {
        // 标准语法: FUNCTION name : return_type
        // function_params (参数 VAR_INPUT - 单独的块)
        // function_local_vars (局部变量 VAR - 单独的块)
        // BEGIN statement_list END_FUNCTION
        // ast_create_function_decl(name, parameters, return_type, declarations, body)
        $$ = ast_create_function_decl($2, $5, $4, $6, $8);
    }
    | TOKEN_FUNCTION TOKEN_IDENTIFIER TOKEN_COLON type_spec
    function_params
    function_local_vars
    statement_list
    TOKEN_END_FUNCTION
    {
        // 不带 BEGIN 的版本
        $$ = ast_create_function_decl($2, $5, $4, $6, $7);
    }
    | TOKEN_FUNCTION TOKEN_IDENTIFIER
    function_params
    function_local_vars
    TOKEN_BEGIN
    statement_list
    TOKEN_END_FUNCTION
    {
        // 无返回类型的函数（PROCEDURE风格）
        $$ = ast_create_function_decl($2, $3, NULL, $4, $6);
    }
    | TOKEN_FUNCTION TOKEN_IDENTIFIER
    function_params
    function_local_vars
    statement_list
    TOKEN_END_FUNCTION
    {
        // 无返回类型，不带 BEGIN
        $$ = ast_create_function_decl($2, $3, NULL, $4, $5);
    }
    ;

/* 函数参数（VAR_INPUT块，可选） */
function_params:
    /* 空 */ { $$ = NULL; }
    | TOKEN_VAR_INPUT var_decl_item TOKEN_END_VAR
    {
        $$ = $2;  // 返回参数链表
    }
    ;

/* 函数局部变量（VAR_LOCAL块，可以有多个）和全局变量（VAR块） */
function_local_vars:
    /* 空 */ { $$ = NULL; }
    | function_local_vars TOKEN_VAR_LOCAL TOKEN_END_VAR
    {
        // 空的 VAR_LOCAL 块
        $$ = $1;
    }
    | function_local_vars TOKEN_VAR_LOCAL function_var_decl_list TOKEN_END_VAR
    {
        // VAR_LOCAL 块中的变量标记为局部变量
        ASTNode* decl = $3;
        while (decl) {
            if (decl->type == AST_VAR_DECL) {
                decl->data.var_decl.is_global = false;
            }
            decl = decl->next;
        }
        
        if ($1 == NULL) {
            $$ = $3;
        } else {
            // 连接到现有的局部变量链表
            ASTNode* last = $1;
            while (last->next) last = last->next;
            last->next = $3;
            $$ = $1;
        }
    }
    | function_local_vars TOKEN_VAR_EXTERNAL external_var_decl_list TOKEN_END_VAR
    {
        // VAR_EXTERNAL 块中的变量标记为外部 I/O 变量
        if ($1 == NULL) {
            $$ = $3;
        } else {
            // 连接到现有的变量链表
            ASTNode* last = $1;
            while (last->next) last = last->next;
            last->next = $3;
            $$ = $1;
        }
    }
    | function_local_vars TOKEN_VAR TOKEN_END_VAR
    {
        // 空的 VAR 块（函数内定义全局变量）
        $$ = $1;
    }
    | function_local_vars TOKEN_VAR function_var_decl_list TOKEN_END_VAR
    {
        // VAR 块中的变量标记为全局变量（函数内定义的全局变量）
        ASTNode* decl = $3;
        while (decl) {
            if (decl->type == AST_VAR_DECL) {
                decl->data.var_decl.is_global = true;
            }
            decl = decl->next;
        }
        
        if ($1 == NULL) {
            $$ = $3;
        } else {
            // 连接到现有的变量链表
            ASTNode* last = $1;
            while (last->next) last = last->next;
            last->next = $3;
            $$ = $1;
        }
    }
    ;

/* 函数内变量声明列表（用于 VAR 和 VAR_LOCAL 块） */
function_var_decl_list:
    TOKEN_IDENTIFIER TOKEN_COLON type_spec TOKEN_SEMICOLON
    {
        // is_global 将在上层规则中设置
        $$ = ast_create_var_decl($1, $3, NULL, false, false);
    }
    | TOKEN_IDENTIFIER TOKEN_COLON type_spec TOKEN_ASSIGN expression TOKEN_SEMICOLON
    {
        $$ = ast_create_var_decl($1, $3, $5, false, false);
    }
    | function_var_decl_list TOKEN_IDENTIFIER TOKEN_COLON type_spec TOKEN_SEMICOLON
    {
        ASTNode* new_decl = ast_create_var_decl($2, $4, NULL, false, false);
        ASTNode* last = $1;
        while (last->next) last = last->next;
        last->next = new_decl;
        $$ = $1;
    }
    | function_var_decl_list TOKEN_IDENTIFIER TOKEN_COLON type_spec TOKEN_ASSIGN expression TOKEN_SEMICOLON
    {
        ASTNode* new_decl = ast_create_var_decl($2, $4, $6, false, false);
        ASTNode* last = $1;
        while (last->next) last = last->next;
        last->next = new_decl;
        $$ = $1;
    }
    ;

/* 语句列表 */
statement_list:
    /* 空 */ { $$ = NULL; }
    | statement_list statement
    {
        if ($1 == NULL) {
            $$ = $2;
        } else {
            ASTNode* last = $1;
            while (last->next) last = last->next;
            last->next = $2;
            $$ = $1;
        }
    }
    ;

/* 外部 I/O 变量声明列表（VAR_EXTERNAL 块） */
external_var_decl_list:
    TOKEN_IDENTIFIER TOKEN_COLON type_spec TOKEN_AT TOKEN_STRING_LITERAL TOKEN_SEMICOLON
    {
        // 创建外部变量声明: 变量名 : 类型 AT "I/O地址";
        $$ = ast_create_external_var_decl($1, $3, $5);
        if ($1) mmgr_free((void*)$1);
        if ($5) mmgr_free((void*)$5);
    }
    | external_var_decl_list TOKEN_IDENTIFIER TOKEN_COLON type_spec TOKEN_AT TOKEN_STRING_LITERAL TOKEN_SEMICOLON
    {
        ASTNode* new_decl = ast_create_external_var_decl($2, $4, $6);
        ASTNode* last = $1;
        while (last->next) last = last->next;
        last->next = new_decl;
        $$ = $1;
        if ($2) mmgr_free((void*)$2);
        if ($6) mmgr_free((void*)$6);
    }
    ;

/* 语句 */
statement:
    assignment_stmt { $$ = $1; }
    | if_stmt       { $$ = $1; }
    | while_stmt    { $$ = $1; }
    | for_stmt      { $$ = $1; }
    | case_stmt     { $$ = $1; }
    | return_stmt   { $$ = $1; }
    | print_stmt    { $$ = $1; }
    | TOKEN_SEMICOLON { $$ = NULL; } /* 空语句 */
    ;

/* 赋值语句 */
assignment_stmt:
    TOKEN_IDENTIFIER TOKEN_ASSIGN expression TOKEN_SEMICOLON
    {
        ASTNode* target = ast_create_identifier($1);
        $$ = ast_create_assign(target, $3);
    }
    | TOKEN_IDENTIFIER TOKEN_LBRACKET expression TOKEN_RBRACKET TOKEN_ASSIGN expression TOKEN_SEMICOLON
    {
        ASTNode* array = ast_create_identifier($1);
        ASTNode* target = ast_create_array_access(array, $3);
        $$ = ast_create_assign(target, $6);
    }
    ;

/* IF语句 */
if_stmt:
    TOKEN_IF expression TOKEN_THEN statement_list TOKEN_END_IF
    {
        $$ = ast_create_if($2, $4, NULL);
    }
    | TOKEN_IF expression TOKEN_THEN statement_list TOKEN_ELSE statement_list TOKEN_END_IF
    {
        $$ = ast_create_if($2, $4, $6);
    }
    | TOKEN_IF expression TOKEN_THEN statement_list elsif_list TOKEN_END_IF
    {
        $$ = ast_create_if($2, $4, $5);
    }
    | TOKEN_IF expression TOKEN_THEN statement_list elsif_list TOKEN_ELSE statement_list TOKEN_END_IF
    {
        /* 找到elsif链的最后一个，添加else分支 */
        ASTNode* last_elsif = $5;
        while (last_elsif->type == AST_IF && last_elsif->data.if_stmt.else_branch) {
            last_elsif = last_elsif->data.if_stmt.else_branch;
        }
        if (last_elsif->type == AST_IF) {
            last_elsif->data.if_stmt.else_branch = ast_create_block($7);
        }
        $$ = ast_create_if($2, $4, $5);
    }
    ;

elsif_list:
    TOKEN_ELSIF expression TOKEN_THEN statement_list
    {
        $$ = ast_create_if($2, $4, NULL);
    }
    | elsif_list TOKEN_ELSIF expression TOKEN_THEN statement_list
    {
        ASTNode* new_elsif = ast_create_if($3, $5, NULL);
        /* 找到elsif链的最后一个 */
        ASTNode* last = $1;
        while (last->type == AST_IF && last->data.if_stmt.else_branch) {
            last = last->data.if_stmt.else_branch;
        }
        if (last->type == AST_IF) {
            last->data.if_stmt.else_branch = new_elsif;
        }
        $$ = $1;
    }
    ;

/* WHILE语句 */
while_stmt:
    TOKEN_WHILE expression TOKEN_DO statement_list TOKEN_END_WHILE
    {
        $$ = ast_create_while($2, $4);
    }
    ;

/* FOR语句 */
for_stmt:
    TOKEN_FOR TOKEN_IDENTIFIER TOKEN_ASSIGN expression TOKEN_TO expression TOKEN_DO statement_list TOKEN_END_FOR
    {
        // ast_create_for(variable, start, end, step, body)
        // 第一个参数是字符串变量名，不是AST节点
        $$ = ast_create_for($2, $4, $6, NULL, $8);
    }
    | TOKEN_FOR TOKEN_IDENTIFIER TOKEN_ASSIGN expression TOKEN_TO expression TOKEN_BY expression TOKEN_DO statement_list TOKEN_END_FOR
    {
        $$ = ast_create_for($2, $4, $6, $8, $10);
    }
    ;

/* CASE语句 */
case_stmt:
    TOKEN_CASE expression TOKEN_OF statement_list TOKEN_END_CASE
    {
        // TODO: CASE语句暂未实现，使用空语句代替
        $$ = NULL;
    }
    ;

/* RETURN语句 */
return_stmt:
    TOKEN_RETURN TOKEN_SEMICOLON
    {
        $$ = ast_create_return(NULL);
    }
    | TOKEN_RETURN expression TOKEN_SEMICOLON
    {
        $$ = ast_create_return($2);
    }
    ;

/* PRINT语句 - 格式化输出 */
print_stmt:
    TOKEN_PRINT TOKEN_LPAREN argument_list TOKEN_RPAREN TOKEN_SEMICOLON
    {
        // 将参数链表转换为数组
        int arg_count = 0;
        ASTNode* arg = $3;
        while (arg) {
            arg_count++;
            arg = arg->next;
        }
        
        ASTNode** args = NULL;
        if (arg_count > 0) {
            args = (ASTNode**)mmgr_alloc(sizeof(ASTNode*) * arg_count);
            arg = $3;
            for (int i = 0; i < arg_count; i++) {
                args[i] = arg;
                arg = arg->next;
                args[i]->next = NULL;  // 断开链表连接
            }
        }
        
        // 创建函数调用节点来表示 PRINT
        $$ = ast_create_function_call("PRINT", args, arg_count);
    }
    ;

/* 表达式 */
expression:
    or_expr { $$ = $1; }
    ;

or_expr:
    xor_expr { $$ = $1; }
    | or_expr TOKEN_OR xor_expr
    {
        $$ = ast_create_binary_op(BINOP_OR, $1, $3);
    }
    ;

xor_expr:
    and_expr { $$ = $1; }
    | xor_expr TOKEN_XOR and_expr
    {
        $$ = ast_create_binary_op(BINOP_XOR, $1, $3);
    }
    ;

and_expr:
    comparison_expr { $$ = $1; }
    | and_expr TOKEN_AND comparison_expr
    {
        $$ = ast_create_binary_op(BINOP_AND, $1, $3);
    }
    ;

comparison_expr:
    add_expr { $$ = $1; }
    | comparison_expr TOKEN_EQ add_expr
    {
        $$ = ast_create_binary_op(BINOP_EQ, $1, $3);
    }
    | comparison_expr TOKEN_NE add_expr
    {
        $$ = ast_create_binary_op(BINOP_NE, $1, $3);
    }
    | comparison_expr TOKEN_LT add_expr
    {
        $$ = ast_create_binary_op(BINOP_LT, $1, $3);
    }
    | comparison_expr TOKEN_LE add_expr
    {
        $$ = ast_create_binary_op(BINOP_LE, $1, $3);
    }
    | comparison_expr TOKEN_GT add_expr
    {
        $$ = ast_create_binary_op(BINOP_GT, $1, $3);
    }
    | comparison_expr TOKEN_GE add_expr
    {
        $$ = ast_create_binary_op(BINOP_GE, $1, $3);
    }
    ;

add_expr:
    mult_expr { $$ = $1; }
    | add_expr TOKEN_PLUS mult_expr
    {
        $$ = ast_create_binary_op(BINOP_ADD, $1, $3);
    }
    | add_expr TOKEN_MINUS mult_expr
    {
        $$ = ast_create_binary_op(BINOP_SUB, $1, $3);
    }
    | add_expr TOKEN_SHL mult_expr
    {
        $$ = ast_create_binary_op(BINOP_SHL, $1, $3);
    }
    | add_expr TOKEN_SHR mult_expr
    {
        $$ = ast_create_binary_op(BINOP_SHR, $1, $3);
    }
    ;

mult_expr:
    unary_expr { $$ = $1; }
    | mult_expr TOKEN_MULTIPLY unary_expr
    {
        $$ = ast_create_binary_op(BINOP_MUL, $1, $3);
    }
    | mult_expr TOKEN_DIVIDE unary_expr
    {
        $$ = ast_create_binary_op(BINOP_DIV, $1, $3);
    }
    | mult_expr TOKEN_MOD unary_expr
    {
        $$ = ast_create_binary_op(BINOP_MOD, $1, $3);
    }
    ;

unary_expr:
    primary_expr { $$ = $1; }
    | TOKEN_NOT unary_expr
    {
        $$ = ast_create_unary_op(UNOP_NOT, $2);
    }
    | TOKEN_MINUS unary_expr %prec UNARY_MINUS
    {
        $$ = ast_create_unary_op(UNOP_NEG, $2);
    }
    ;

primary_expr:
    TOKEN_INTEGER_LITERAL
    {
        Value val;
        val.type = TYPE_INT;
        val.int_val = $1;
        $$ = ast_create_literal(val);
    }
    | TOKEN_REAL_LITERAL
    {
        Value val;
        val.type = TYPE_REAL;
        val.real_val = $1;
        $$ = ast_create_literal(val);
    }
    | TOKEN_TRUE
    {
        Value val;
        val.type = TYPE_BOOL;
        val.bool_val = true;
        $$ = ast_create_literal(val);
    }
    | TOKEN_FALSE
    {
        Value val;
        val.type = TYPE_BOOL;
        val.bool_val = false;
        $$ = ast_create_literal(val);
    }
    | TOKEN_STRING_LITERAL
    {
        Value val;
        val.type = TYPE_STRING;
        val.string_val = $1;
        $$ = ast_create_literal(val);
    }
    | TOKEN_IDENTIFIER
    {
        $$ = ast_create_identifier($1);
    }
    | TOKEN_IDENTIFIER TOKEN_LPAREN argument_list TOKEN_RPAREN
    {
        // 将参数链表转换为数组
        int arg_count = 0;
        ASTNode* arg = $3;
        while (arg) {
            arg_count++;
            arg = arg->next;
        }
        
        ASTNode** args = NULL;
        if (arg_count > 0) {
            args = (ASTNode**)mmgr_alloc(sizeof(ASTNode*) * arg_count);
            arg = $3;
            for (int i = 0; i < arg_count; i++) {
                args[i] = arg;
                arg = arg->next;
                args[i]->next = NULL;  // 断开链表连接
            }
        }
        
        $$ = ast_create_function_call($1, args, arg_count);
    }
    | TOKEN_IDENTIFIER TOKEN_LPAREN TOKEN_RPAREN
    {
        $$ = ast_create_function_call($1, NULL, 0);
    }
    | TOKEN_IDENTIFIER TOKEN_LBRACKET expression TOKEN_RBRACKET
    {
        ASTNode* array = ast_create_identifier($1);
        $$ = ast_create_array_access(array, $3);
    }
    | TOKEN_LPAREN expression TOKEN_RPAREN
    {
        $$ = $2;
    }
    ;

argument_list:
    expression
    {
        $$ = $1;
    }
    | argument_list TOKEN_COMMA expression
    {
        ASTNode* last = $1;
        while (last->next) last = last->next;
        last->next = $3;
        $$ = $1;
    }
    ;

%%

void yyerror(const char* s) {
    fprintf(stderr, "Parse error at line %d, column %d: %s\n", yylineno, yycolumn, s);
    if (yytext[0] != '\0') {
        fprintf(stderr, "Near token: '%s'\n", yytext);
    }
}

/* 解析文件 */
ASTNode* parse_file(const char* filename) {
    extern FILE* yyin;
    yyin = fopen(filename, "r");
    if (!yyin) {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        return NULL;
    }
    
    parse_result = NULL;
    yyparse();
    
    fclose(yyin);
    return parse_result;
}

/* 解析字符串 */
ASTNode* parse_string(const char* source) {
    extern FILE* yyin;
    
    /* 创建临时文件 */
    FILE* temp = tmpfile();
    if (!temp) {
        fprintf(stderr, "Cannot create temporary file\n");
        return NULL;
    }
    
    fputs(source, temp);
    rewind(temp);
    
    yyin = temp;
    parse_result = NULL;
    yyparse();
    
    fclose(temp);
    return parse_result;
}
