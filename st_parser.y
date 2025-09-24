/*
 * IEC61131 结构化文本语言语法分析器
 * 支持完整的ST语言特性：程序、库、导入、函数、变量、控制结构等
 */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "mmgr.h"
#include "symbol_table.h"

extern int yylex(void);
extern int yylineno;
extern int yycolumn;

void yyerror(const char *s);

/* 全局AST根节点 */
static ast_node_t *g_ast_root = NULL;
%}

/* YYSTYPE 联合体定义 */
%union {
    int integer;
    double real;
    int boolean;
    char *string;
    ast_node_t *node;
    struct type_info *type;
    operator_type_t op;
    var_category_t var_cat;
    param_category_t param_cat;
    function_type_t func_type;
}

/* 终结符定义 */
%token <integer> INTEGER_LITERAL
%token <real> REAL_LITERAL
%token <boolean> BOOL_LITERAL
%token <string> STRING_LITERAL WSTRING_LITERAL
%token <string> TIME_LITERAL DATE_LITERAL TOD_LITERAL DT_LITERAL
%token <string> IDENTIFIER
%token <string> PRAGMA

/* 程序结构关键字 */
%token PROGRAM END_PROGRAM FUNCTION END_FUNCTION FUNCTION_BLOCK END_FUNCTION_BLOCK

/* 库和导入关键字 */
%token LIBRARY END_LIBRARY IMPORT FROM AS EXPORT VERSION

/* 变量声明关键字 */
%token VAR END_VAR VAR_INPUT VAR_OUTPUT VAR_IN_OUT VAR_EXTERNAL VAR_GLOBAL
%token VAR_ACCESS VAR_TEMP VAR_CONFIG VAR_CONSTANT

/* 控制结构关键字 */
%token IF THEN ELSE ELSIF END_IF
%token CASE OF END_CASE
%token FOR TO BY DO END_FOR
%token WHILE END_WHILE
%token REPEAT UNTIL END_REPEAT
%token EXIT CONTINUE RETURN

/* 数据类型关键字 */
%token TYPE_BOOL TYPE_BYTE TYPE_WORD TYPE_DWORD TYPE_LWORD
%token TYPE_SINT TYPE_INT TYPE_DINT TYPE_LINT
%token TYPE_USINT TYPE_UINT TYPE_UDINT TYPE_ULINT
%token TYPE_REAL TYPE_LREAL
%token TYPE_STRING TYPE_WSTRING TYPE_TIME TYPE_DATE
%token TYPE_TIME_OF_DAY TYPE_DATE_AND_TIME
%token TYPE_ARRAY TYPE_STRUCT END_STRUCT TYPE_UNION END_UNION
%token TYPE_ENUM END_ENUM TYPE END_TYPE

/* 运算符 */
%token ASSIGN EQ NE LT LE GT GE PLUS MINUS MULTIPLY DIVIDE MOD POWER
%token AND OR XOR NOT

/* 分隔符和标点符号 */
%token LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE
%token SEMICOLON COLON COMMA DOT RANGE

/* 错误标记 */
%token ERROR_TOKEN

/* 非终结符类型定义 */
%type <node> compilation_unit program_declaration library_declaration
%type <node> import_list import_declaration
%type <node> declaration_list declaration
%type <node> var_declaration var_list var_item
%type <node> function_declaration parameter_list parameter_declaration
%type <node> statement_list statement
%type <node> assignment_statement if_statement elsif_list elsif_statement
%type <node> case_statement case_list case_item case_values
%type <node> for_statement while_statement repeat_statement
%type <node> return_statement exit_statement continue_statement
%type <node> expression_statement
%type <node> expression logical_expr comparison_expr additive_expr
%type <node> multiplicative_expr unary_expr primary_expr
%type <node> function_call argument_list
%type <node> array_access member_access
%type <node> type_declaration array_type struct_type enum_type
%type <node> array_bounds bound_specification
%type <node> struct_members enum_values
%type <type> type_specifier basic_type

%type <var_cat> var_category
%type <param_cat> parameter_category
%type <func_type> function_type

/* 运算符优先级和结合性 */
%right ASSIGN
%left OR
%left XOR
%left AND
%left EQ NE
%left LT LE GT GE
%left PLUS MINUS
%left MULTIPLY DIVIDE MOD
%right POWER
%right NOT UMINUS UPLUS
%left LPAREN RPAREN LBRACKET RBRACKET DOT

%start compilation_unit

%%

/* ========== 编译单元 ========== */
compilation_unit: 
    import_list program_declaration {
        $$ = ast_create_program($2->data.program.name, $1, 
                               $2->data.program.declarations, 
                               $2->data.program.statements);
        g_ast_root = $$;
        ast_set_root($$);
    }
    | program_declaration {
        $$ = $1;
        g_ast_root = $$;
        ast_set_root($$);
    }
    | import_list library_declaration {
        $$ = $2;
        if ($2->data.library.declarations) {
            /* 将导入添加到库声明前 */
        }
        g_ast_root = $$;
        ast_set_root($$);
    }
    | library_declaration {
        $$ = $1;
        g_ast_root = $$;
        ast_set_root($$);
    }
    ;

/* ========== 导入声明 ========== */
import_list:
    import_declaration {
        $$ = ast_create_list(AST_IMPORT_LIST);
        $$ = ast_list_append($$, $1);
    }
    | import_list import_declaration {
        $$ = ast_list_append($1, $2);
    }
    ;

import_declaration:
    IMPORT IDENTIFIER SEMICOLON {
        $$ = ast_create_import($2, NULL, NULL, NULL, false);
    }
    | IMPORT IDENTIFIER AS IDENTIFIER SEMICOLON {
        $$ = ast_create_import($2, $4, NULL, NULL, false);
    }
    | IMPORT IDENTIFIER FROM STRING_LITERAL SEMICOLON {
        $$ = ast_create_import($2, NULL, $4, NULL, false);
    }
    | IMPORT IDENTIFIER AS IDENTIFIER FROM STRING_LITERAL SEMICOLON {
        $$ = ast_create_import($2, $4, $6, NULL, false);
    }
    ;

/* ========== 程序声明 ========== */
program_declaration:
    PROGRAM IDENTIFIER declaration_list statement_list END_PROGRAM {
        $$ = ast_create_program($2, NULL, $3, $4);
    }
    | PROGRAM IDENTIFIER statement_list END_PROGRAM {
        $$ = ast_create_program($2, NULL, NULL, $3);
    }
    | PROGRAM IDENTIFIER declaration_list END_PROGRAM {
        $$ = ast_create_program($2, NULL, $3, NULL);
    }
    | PROGRAM IDENTIFIER END_PROGRAM {
        $$ = ast_create_program($2, NULL, NULL, NULL);
    }
    ;

/* ========== 库声明 ========== */
library_declaration:
    LIBRARY IDENTIFIER VERSION STRING_LITERAL declaration_list END_LIBRARY {
        $$ = ast_create_library($2, $4, $5, NULL);
    }
    | LIBRARY IDENTIFIER declaration_list END_LIBRARY {
        $$ = ast_create_library($2, "1.0", $3, NULL);
    }
    ;

/* ========== 声明列表 ========== */
declaration_list:
    /* empty */ { 
        $$ = NULL; 
    }
    | declaration {
        $$ = ast_create_list(AST_DECLARATION_LIST);
        $$ = ast_list_append($$, $1);
    }
    | declaration_list declaration {
        $$ = ast_list_append($1, $2);
    }
    ;

declaration:
    var_declaration { $$ = $1; }
    | function_declaration { $$ = $1; }
    | type_declaration { $$ = $1; }
    ;

/* ========== 变量声明 ========== */
var_declaration:
    var_category var_list END_VAR {
        $$ = ast_create_var_declaration($1, $2);
    }
    ;

var_category:
    VAR { $$ = VAR_LOCAL; }
    | VAR_INPUT { $$ = VAR_INPUT; }
    | VAR_OUTPUT { $$ = VAR_OUTPUT; }
    | VAR_IN_OUT { $$ = VAR_IN_OUT; }
    | VAR_EXTERNAL { $$ = VAR_EXTERNAL; }
    | VAR_GLOBAL { $$ = VAR_GLOBAL; }
    | VAR_CONSTANT { $$ = VAR_CONSTANT; }
    | VAR_TEMP { $$ = VAR_TEMP; }
    | VAR_CONFIG { $$ = VAR_CONFIG; }
    ;

var_list:
    var_item {
        $$ = $1;
    }
    | var_list var_item {
        $1->next = $2;
        $$ = $1;
    }
    ;

var_item:
    IDENTIFIER COLON type_specifier SEMICOLON {
        $$ = ast_create_var_item($1, $3, NULL, NULL);
        /* 添加到符号表 */
        add_symbol($1, $3, get_current_scope());
    }
    | IDENTIFIER COLON type_specifier ASSIGN expression SEMICOLON {
        $$ = ast_create_var_item($1, $3, $5, NULL);
        /* 添加到符号表 */
        add_symbol($1, $3, get_current_scope());
    }
    | IDENTIFIER COLON array_type SEMICOLON {
        $$ = ast_create_var_item($1, NULL, NULL, $3);
    }
    ;

/* ========== 类型说明符 ========== */
type_specifier:
    basic_type { $$ = $1; }
    | array_type { $$ = NULL; /* 处理数组类型 */ }
    | struct_type { $$ = NULL; /* 处理结构体类型 */ }
    | IDENTIFIER { $$ = NULL; /* 用户定义类型 */ }
    ;

basic_type:
    TYPE_BOOL { $$ = create_basic_type(TYPE_BOOL_ID); }
    | TYPE_BYTE { $$ = create_basic_type(TYPE_BOOL_ID); /* 简化映射 */ }
    | TYPE_INT { $$ = create_basic_type(TYPE_INT_ID); }
    | TYPE_DINT { $$ = create_basic_type(TYPE_DINT_ID); }
    | TYPE_REAL { $$ = create_basic_type(TYPE_REAL_ID); }
    | TYPE_LREAL { $$ = create_basic_type(TYPE_REAL_ID); }
    | TYPE_STRING { $$ = create_basic_type(TYPE_STRING_ID); }
    | TYPE_TIME { $$ = create_basic_type(TYPE_TIME_ID); }
    ;

/* ========== 复合类型 ========== */
array_type:
    TYPE_ARRAY LBRACKET array_bounds RBRACKET OF type_specifier {
        $$ = ast_create_array_type($6, $3);
    }
    ;

array_bounds:
    bound_specification {
        $$ = $1;
    }
    | array_bounds COMMA bound_specification {
        $1->next = $3;
        $$ = $1;
    }
    ;

bound_specification:
    INTEGER_LITERAL RANGE INTEGER_LITERAL {
        ast_node_t *start = ast_create_literal(LITERAL_INT, &$1);
        ast_node_t *end = ast_create_literal(LITERAL_INT, &$3);
        $$ = ast_create_binary_operation(OP_ASSIGN, start, end);
    }
    ;

struct_type:
    TYPE_STRUCT struct_members END_STRUCT {
        $$ = ast_create_struct_type($2);
    }
    ;

struct_members:
    var_item {
        $$ = ast_create_list(AST_STRUCT_MEMBER_LIST);
        $$ = ast_list_append($$, $1);
    }
    | struct_members var_item {
        $$ = ast_list_append($1, $2);
    }
    ;

enum_type:
    TYPE_ENUM enum_values END_ENUM {
        $$ = ast_create_enum_type($2);
    }
    ;

enum_values:
    IDENTIFIER {
        $$ = ast_create_list(AST_ENUM_VALUE_LIST);
        $$ = ast_list_append($$, ast_create_identifier($1));
    }
    | enum_values COMMA IDENTIFIER {
        $$ = ast_list_append($1, ast_create_identifier($3));
    }
    ;

type_declaration:
    TYPE IDENTIFIER COLON type_specifier SEMICOLON END_TYPE {
        $$ = ast_create_type_declaration($2, $4);
    }
    ;

/* ========== 函数声明 ========== */
function_declaration:
    function_type IDENTIFIER COLON type_specifier declaration_list statement_list END_FUNCTION {
        $$ = ast_create_function_declaration($2, $1, NULL, $4, $5, $6);
    }
    | function_type IDENTIFIER LPAREN parameter_list RPAREN COLON type_specifier 
      declaration_list statement_list END_FUNCTION {
        $$ = ast_create_function_declaration($2, $1, $4, $7, $8, $9);
    }
    | function_type IDENTIFIER declaration_list statement_list END_FUNCTION_BLOCK {
        $$ = ast_create_function_declaration($2, $1, NULL, NULL, $3, $4);
    }
    ;

function_type:
    FUNCTION { $$ = FUNC_FUNCTION; }
    | FUNCTION_BLOCK { $$ = FUNC_FUNCTION_BLOCK; }
    ;

parameter_list:
    /* empty */ { 
        $$ = NULL; 
    }
    | parameter_declaration {
        $$ = ast_create_list(AST_PARAMETER_LIST);
        $$ = ast_list_append($$, $1);
    }
    | parameter_list SEMICOLON parameter_declaration {
        $$ = ast_list_append($1, $3);
    }
    ;

parameter_declaration:
    parameter_category IDENTIFIER COLON type_specifier {
        $$ = ast_create_parameter($2, $4, $1, NULL);
    }
    | parameter_category IDENTIFIER COLON type_specifier ASSIGN expression {
        $$ = ast_create_parameter($2, $4, $1, $6);
    }
    ;

parameter_category:
    /* empty */ { $$ = PARAM_INPUT; }
    | VAR_INPUT { $$ = PARAM_INPUT; }
    | VAR_OUTPUT { $$ = PARAM_OUTPUT; }
    | VAR_IN_OUT { $$ = PARAM_IN_OUT; }
    ;

/* ========== 语句 ========== */
statement_list:
    /* empty */ { 
        $$ = NULL; 
    }
    | statement {
        $$ = ast_create_list(AST_STATEMENT_LIST);
        $$ = ast_list_append($$, $1);
    }
    | statement_list statement {
        $$ = ast_list_append($1, $2);
    }
    ;

statement:
    assignment_statement SEMICOLON { $$ = $1; }
    | expression_statement SEMICOLON { $$ = $1; }
    | if_statement { $$ = $1; }
    | case_statement { $$ = $1; }
    | for_statement { $$ = $1; }
    | while_statement { $$ = $1; }
    | repeat_statement { $$ = $1; }
    | return_statement SEMICOLON { $$ = $1; }
    | exit_statement SEMICOLON { $$ = $1; }
    | continue_statement SEMICOLON { $$ = $1; }
    ;

/* ========== 赋值语句 ========== */
assignment_statement:
    primary_expr ASSIGN expression {
        $$ = ast_create_assignment($1, $3);
    }
    ;

expression_statement:
    function_call {
        $$ = ast_create_expression_statement($1);
    }
    ;

/* ========== 条件语句 ========== */
if_statement:
    IF expression THEN statement_list END_IF {
        $$ = ast_create_if_statement($2, $4, NULL, NULL);
    }
    | IF expression THEN statement_list ELSE statement_list END_IF {
        $$ = ast_create_if_statement($2, $4, NULL, $6);
    }
    | IF expression THEN statement_list elsif_list END_IF {
        $$ = ast_create_if_statement($2, $4, $5, NULL);
    }
    | IF expression THEN statement_list elsif_list ELSE statement_list END_IF {
        $$ = ast_create_if_statement($2, $4, $5, $7);
    }
    ;

elsif_list:
    elsif_statement {
        $$ = ast_create_list(AST_ELSIF_LIST);
        $$ = ast_list_append($$, $1);
    }
    | elsif_list elsif_statement {
        $$ = ast_list_append($1, $2);
    }
    ;

elsif_statement:
    ELSIF expression THEN statement_list {
        $$ = ast_create_elsif_statement($2, $4);
    }
    ;

/* ========== CASE语句 ========== */
case_statement:
    CASE expression OF case_list END_CASE {
        $$ = ast_create_case_statement($2, $4, NULL);
    }
    | CASE expression OF case_list ELSE statement_list END_CASE {
        $$ = ast_create_case_statement($2, $4, $6);
    }
    ;

case_list:
    case_item {
        $$ = ast_create_list(AST_CASE_LIST);
        $$ = ast_list_append($$, $1);
    }
    | case_list case_item {
        $$ = ast_list_append($1, $2);
    }
    ;

case_item:
    case_values COLON statement_list {
        $$ = ast_create_case_item($1, $3);
    }
    ;

case_values:
    expression {
        $$ = ast_create_list(AST_EXPRESSION_LIST);
        $$ = ast_list_append($$, $1);
    }
    | case_values COMMA expression {
        $$ = ast_list_append($1, $3);
    }
    ;

/* ========== 循环语句 ========== */
for_statement:
    FOR IDENTIFIER ASSIGN expression TO expression DO statement_list END_FOR {
        $$ = ast_create_for_statement($2, $4, $6, NULL, $8);
    }
    | FOR IDENTIFIER ASSIGN expression TO expression BY expression DO statement_list END_FOR {
        $$ = ast_create_for_statement($2, $4, $6, $8, $10);
    }
    ;

while_statement:
    WHILE expression DO statement_list END_WHILE {
        $$ = ast_create_while_statement($2, $4);
    }
    ;

repeat_statement:
    REPEAT statement_list UNTIL expression END_REPEAT {
        $$ = ast_create_repeat_statement($2, $4);
    }
    ;

/* ========== 控制语句 ========== */
return_statement:
    RETURN {
        $$ = ast_create_return_statement(NULL);
    }
    | RETURN expression {
        $$ = ast_create_return_statement($2);
    }
    ;

exit_statement:
    EXIT {
        $$ = ast_create_exit_statement();
    }
    ;

continue_statement:
    CONTINUE {
        $$ = ast_create_continue_statement();
    }
    ;

/* ========== 表达式 ========== */
expression:
    logical_expr { $$ = $1; }
    ;

logical_expr:
    comparison_expr { $$ = $1; }
    | logical_expr AND comparison_expr {
        $$ = ast_create_binary_operation(OP_AND, $1, $3);
    }
    | logical_expr OR comparison_expr {
        $$ = ast_create_binary_operation(OP_OR, $1, $3);
    }
    | logical_expr XOR comparison_expr {
        $$ = ast_create_binary_operation(OP_XOR, $1, $3);
    }
    ;

comparison_expr:
    additive_expr { $$ = $1; }
    | comparison_expr EQ additive_expr {
        $$ = ast_create_binary_operation(OP_EQ, $1, $3);
    }
    | comparison_expr NE additive_expr {
        $$ = ast_create_binary_operation(OP_NE, $1, $3);
    }
    | comparison_expr LT additive_expr {
        $$ = ast_create_binary_operation(OP_LT, $1, $3);
    }
    | comparison_expr LE additive_expr {
        $$ = ast_create_binary_operation(OP_LE, $1, $3);
    }
    | comparison_expr GT additive_expr {
        $$ = ast_create_binary_operation(OP_GT, $1, $3);
    }
    | comparison_expr GE additive_expr {
        $$ = ast_create_binary_operation(OP_GE, $1, $3);
    }
    ;

additive_expr:
    multiplicative_expr { $$ = $1; }
    | additive_expr PLUS multiplicative_expr {
        $$ = ast_create_binary_operation(OP_ADD, $1, $3);
    }
    | additive_expr MINUS multiplicative_expr {
        $$ = ast_create_binary_operation(OP_SUB, $1, $3);
    }
    ;

multiplicative_expr:
    unary_expr { $$ = $1; }
    | multiplicative_expr MULTIPLY unary_expr {
        $$ = ast_create_binary_operation(OP_MUL, $1, $3);
    }
    | multiplicative_expr DIVIDE unary_expr {
        $$ = ast_create_binary_operation(OP_DIV, $1, $3);
    }
    | multiplicative_expr MOD unary_expr {
        $$ = ast_create_binary_operation(OP_MOD, $1, $3);
    }
    | multiplicative_expr POWER unary_expr {
        $$ = ast_create_binary_operation(OP_POWER, $1, $3);
    }
    ;

unary_expr:
    primary_expr { $$ = $1; }
    | NOT unary_expr {
        $$ = ast_create_unary_operation(OP_NOT, $2);
    }
    | MINUS unary_expr %prec UMINUS {
        $$ = ast_create_unary_operation(OP_NEG, $2);
    }
    | PLUS unary_expr %prec UPLUS {
        $$ = ast_create_unary_operation(OP_POS, $2);
    }
    ;

primary_expr:
    INTEGER_LITERAL {
        $$ = ast_create_literal(LITERAL_INT, &$1);
    }
    | REAL_LITERAL {
        $$ = ast_create_literal(LITERAL_REAL, &$1);
    }
    | BOOL_LITERAL {
        $$ = ast_create_literal(LITERAL_BOOL, &$1);
    }
    | STRING_LITERAL {
        $$ = ast_create_literal(LITERAL_STRING, $1);
    }
    | WSTRING_LITERAL {
        $$ = ast_create_literal(LITERAL_WSTRING, $1);
    }
    | TIME_LITERAL {
        $$ = ast_create_literal(LITERAL_TIME, $1);
    }
    | DATE_LITERAL {
        $$ = ast_create_literal(LITERAL_DATE, $1);
    }
    | IDENTIFIER {
        $$ = ast_create_identifier($1);
    }
    | function_call {
        $$ = $1;
    }
    | array_access {
        $$ = $1;
    }
    | member_access {
        $$ = $1;
    }
    | LPAREN expression RPAREN {
        $$ = $2;
    }
    ;

/* ========== 函数调用 ========== */
function_call:
    IDENTIFIER LPAREN RPAREN {
        $$ = ast_create_function_call($1, NULL, NULL);
    }
    | IDENTIFIER LPAREN argument_list RPAREN {
        $$ = ast_create_function_call($1, NULL, $3);
    }
    | IDENTIFIER DOT IDENTIFIER LPAREN RPAREN {
        $$ = ast_create_function_call($3, $1, NULL);
    }
    | IDENTIFIER DOT IDENTIFIER LPAREN argument_list RPAREN {
        $$ = ast_create_function_call($3, $1, $5);
    }
    ;

argument_list:
    expression {
        $$ = ast_create_list(AST_EXPRESSION_LIST);
        $$ = ast_list_append($$, $1);
    }
    | argument_list COMMA expression {
        $$ = ast_list_append($1, $3);
    }
    ;

/* ========== 数组访问和成员访问 ========== */
array_access:
    primary_expr LBRACKET argument_list RBRACKET {
        $$ = ast_create_array_access($1, $3);
    }
    ;

member_access:
    primary_expr DOT IDENTIFIER {
        $$ = ast_create_member_access($1, $3);
    }
    ;

%%

/* ========== 错误处理函数 ========== */
void yyerror(const char *s) {
    fprintf(stderr, "语法错误 (行 %d, 列 %d): %s\n", yylineno, yycolumn, s);
}

/* ========== 全局AST访问函数 ========== */
ast_node_t* get_ast_root(void) {
    return g_ast_root;
}