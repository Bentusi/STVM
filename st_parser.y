/*
 * IEC61131 结构化文本语言语法分析器
 * 支持：程序结构、变量声明、控制流、表达式
 */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "vm.h"

extern int yylex();
extern void yyerror(const char *msg);
extern int line_num;

/* 抽象语法树根节点 */
ASTNode *ast_root = NULL;
%}

/* YYSTYPE 联合体定义 */
%union {
    int int_val;
    double real_val;
    char *str_val;
    int bool_val;
    ASTNode *node;
    VarDecl *var_decl;
    DataType data_type;
}

/* 终结符定义 */
%token <str_val> IDENTIFIER STRING_LITERAL
%token <int_val> INT_LITERAL
%token <real_val> REAL_LITERAL
%token <bool_val> BOOL_LITERAL

/* 关键字 */
%token PROGRAM END_PROGRAM VAR END_VAR VAR_INPUT VAR_OUTPUT VAR_IN_OUT
%token BOOL_TYPE INT_TYPE REAL_TYPE STRING_TYPE
%token IF THEN ELSE ELSIF END_IF FOR TO BY DO END_FOR WHILE END_WHILE
%token CASE OF END_CASE

/* 运算符 */
%token ASSIGN EQ NE LT LE GT GE PLUS MINUS MUL DIV MOD AND OR NOT

/* 分隔符 */
%token SEMICOLON COLON COMMA DOT LPAREN RPAREN LBRACKET RBRACKET

/* 错误标识 */
%token ERROR

/* 非终结符类型 */
%type <node> program statement_list statement assignment_stmt
%type <node> if_stmt for_stmt while_stmt case_stmt case_item expression term factor
%type <node> comparison logical_expr case_list
%type <var_decl> var_declaration var_decl_list
%type <data_type> data_type

/* 运算符优先级 */
%left OR
%left AND
%left EQ NE
%left LT LE GT GE
%left PLUS MINUS
%left MUL DIV MOD
%right NOT
%right UMINUS

%start program

%%

/* 程序结构 */
program: PROGRAM IDENTIFIER var_section statement_list END_PROGRAM
        {
            $$ = create_program_node($2, $4);
            ast_root = $$;
        }
        ;

/* 变量声明段 */
var_section: /* empty */
           | VAR var_decl_list END_VAR
           ;

var_decl_list: var_declaration
             {
                 add_variable($1);
             }
             | var_decl_list var_declaration
             {
                 add_variable($2);
             }
             ;

var_declaration: IDENTIFIER COLON data_type SEMICOLON
               {
                   $$ = create_var_decl($1, $3);
               }
               ;

data_type: BOOL_TYPE   { $$ = TYPE_BOOL; }
         | INT_TYPE    { $$ = TYPE_INT; }
         | REAL_TYPE   { $$ = TYPE_REAL; }
         | STRING_TYPE { $$ = TYPE_STRING; }
         ;

/* 语句列表 */
statement_list: statement
              {
                  $$ = create_statement_list($1);
              }
              | statement_list statement
              {
                  $$ = add_statement($1, $2);
              }
              ;

/* 语句 */
statement: assignment_stmt SEMICOLON { $$ = $1; }
         | if_stmt                   { $$ = $1; }
         | for_stmt                  { $$ = $1; }
         | while_stmt                { $$ = $1; }
         | case_stmt                 { $$ = $1; }
         ;

/* 赋值语句 */
assignment_stmt: IDENTIFIER ASSIGN expression
               {
                   $$ = create_assign_node($1, $3);
               }
               ;

/* IF语句 */
if_stmt: IF expression THEN statement_list END_IF
       {
           $$ = create_if_node($2, $4, NULL);
       }
       | IF expression THEN statement_list ELSE statement_list END_IF
       {
           $$ = create_if_node($2, $4, $6);
       }
       ;

/* FOR循环 */
for_stmt: FOR IDENTIFIER ASSIGN expression TO expression DO statement_list END_FOR
        {
            $$ = create_for_node($2, $4, $6, $8);
        }
        ;

/* WHILE循环 */
while_stmt: WHILE expression DO statement_list END_WHILE
          {
              $$ = create_while_node($2, $4);
          }
          ;

/* CASE语句 */
case_stmt: CASE expression OF case_list END_CASE
         {
             $$ = create_case_node($2, $4);
         }
         ;

case_list: case_item
         {
             $$ = $1;  // 单个case项
         }
         | case_list case_item
         {
             // 将新的case_item添加到列表末尾
             ASTNode *current = $1;
             while (current->next != NULL) {
                 current = current->next;
             }
             current->next = $2;
             $$ = $1;
         }
         ;

case_item: expression COLON statement_list
         {
             $$ = create_case_item($1, $3);
         }
         ;

/* 表达式 */
expression: logical_expr { $$ = $1; }
          ;

logical_expr: comparison
            | logical_expr AND comparison
            {
                $$ = create_binary_op_node(OP_AND, $1, $3);
            }
            | logical_expr OR comparison
            {
                $$ = create_binary_op_node(OP_OR, $1, $3);
            }
            | NOT comparison
            {
                $$ = create_unary_op_node(OP_NOT, $2);
            }
            ;

comparison: term
          | comparison EQ term
          {
              $$ = create_binary_op_node(OP_EQ, $1, $3);
          }
          | comparison NE term
          {
              $$ = create_binary_op_node(OP_NE, $1, $3);
          }
          | comparison LT term
          {
              $$ = create_binary_op_node(OP_LT, $1, $3);
          }
          | comparison LE term
          {
              $$ = create_binary_op_node(OP_LE, $1, $3);
          }
          | comparison GT term
          {
              $$ = create_binary_op_node(OP_GT, $1, $3);
          }
          | comparison GE term
          {
              $$ = create_binary_op_node(OP_GE, $1, $3);
          }
          ;

term: factor
    | term PLUS factor
    {
        $$ = create_binary_op_node(OP_ADD, $1, $3);
    }
    | term MINUS factor
    {
        $$ = create_binary_op_node(OP_SUB, $1, $3);
    }
    ;

factor: IDENTIFIER
      {
          $$ = create_identifier_node($1);
      }
      | INT_LITERAL
      {
          $$ = create_int_literal_node($1);
      }
      | REAL_LITERAL
      {
          $$ = create_real_literal_node($1);
      }
      | BOOL_LITERAL
      {
          $$ = create_bool_literal_node($1);
      }
      | STRING_LITERAL
      {
          $$ = create_string_literal_node($1);
      }
      | LPAREN expression RPAREN
      {
          $$ = $2;
      }
      | factor MUL factor
      {
          $$ = create_binary_op_node(OP_MUL, $1, $3);
      }
      | factor DIV factor
      {
          $$ = create_binary_op_node(OP_DIV, $1, $3);
      }
      | MINUS factor %prec UMINUS
      {
          $$ = create_unary_op_node(OP_NEG, $2);
      }
      ;

%%

/* 错误处理函数 */
/* void yyerror(const char *msg) {
    printf("语法错误：第%d行 %s\n", line_num, msg);
} */