/*
 * IEC61131 结构化文本语言语法分析器
 * 支持：程序结构、变量声明、控制流、表达式、函数定义和调用
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
    ParamDecl *param_decl;
    DataType data_type;
}

/* 终结符定义 */
%token <str_val> IDENTIFIER STRING_LITERAL
%token <int_val> INT_LITERAL
%token <real_val> REAL_LITERAL
%token <bool_val> BOOL_LITERAL

/* 关键字 */
%token PROGRAM END_PROGRAM VAR END_VAR VAR_INPUT VAR_OUTPUT VAR_IN_OUT FUNCTION END_FUNCTION FUNCTION_BLOCK END_FUNCTION_BLOCK RETURN
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
%type <node> compilation_unit program statement_list statement assignment_stmt function_decl function_block_decl
%type <node> if_stmt for_stmt while_stmt case_stmt case_item expression term factor
%type <node> comparison logical_expr case_list function_call argument_list return_stmt
%type <node> function_body var_section_list var_section declaration_list
%type <param_decl> input_decls output_decls inout_decls function_param_sections
%type <param_decl> input_param_section output_param_section inout_param_section
%type <param_decl> input_param_decl output_param_decl inout_param_decl
%type <var_decl> var_declaration var_decl_list local_var_decls
%type <data_type> data_type

/* 运算符优先级 */
%right UMINUS
%left OR
%left AND
%left EQ NE
%left LT LE GT GE
%left PLUS MINUS
%left MUL DIV MOD
%right NOT

%start compilation_unit

%%

/* 编译单元 - 支持多个函数定义和一个程序 */
compilation_unit: declaration_list program
                {
                    /* 创建包含函数声明和程序的编译单元 */
                    $$ = create_compilation_unit_node($1, $2);
                    ast_root = $$;
                }
                | program
                {
                    $$ = $1;
                    ast_root = $$;
                }
                | declaration_list
                {
                    /* 只有函数声明，没有程序 */
                    $$ = $1;
                    ast_root = $$;
                }
                ;

/* 声明列表 - 函数和函数块的列表 */
declaration_list: function_decl
                {
                    $$ = $1;
                }
                | function_block_decl
                {
                    $$ = $1;
                }
                | declaration_list function_decl
                {
                    /* 将新函数链接到声明列表的末尾 */
                    ASTNode *current = $1;
                    while (current->next != NULL) {
                        current = current->next;
                    }
                    current->next = $2;
                    $$ = $1;  /* 返回链表头 */
                }
                | declaration_list function_block_decl
                {
                    /* 将新函数块链接到声明列表的末尾 */
                    ASTNode *current = $1;
                    while (current->next != NULL) {
                        current = current->next;
                    }
                    current->next = $2;
                    $$ = $1;  /* 返回链表头 */
                }
                ;

/* 程序结构 */
program: PROGRAM IDENTIFIER var_section_list statement_list END_PROGRAM
        {
            $$ = create_program_node($2, $4);
        }
        ;

/* 变量声明段列表 */
var_section_list: /* empty */
                {
                    $$ = NULL;
                }
                | var_section_list var_section
                {
                    if ($1 == NULL) {
                        $$ = $2;
                    } else {
                        /* 链接多个变量声明段 */
                        ASTNode *current = $1;
                        while (current->next != NULL) {
                            current = current->next;
                        }
                        current->next = $2;
                        $$ = $1;
                    }
                }
                ;

/* 变量声明段 */
var_section: VAR var_decl_list END_VAR
           {
               $$ = create_var_section_node($2);
           }
           ;

var_decl_list: var_declaration
             {
                 add_variable($1);
                 $$ = $1;
             }
             | var_decl_list var_declaration
             {
                 add_variable($2);
                 /* 链接变量声明 */
                 VarDecl *current = $1;
                 while (current->next != NULL) {
                     current = current->next;
                 }
                 current->next = $2;
                 $$ = $1;
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
statement_list: /* empty */
              {
                  $$ = NULL;
              }
              | statement_list statement
              {
                  if ($1 == NULL) {
                      $$ = create_statement_list($2);
                  } else {
                      $$ = add_statement($1, $2);
                  }
              }
              ;

/* 函数声明 - 完整实现 */
function_decl: FUNCTION IDENTIFIER COLON data_type function_param_sections local_var_decls function_body END_FUNCTION
             {
                 $$ = create_function_node_complete($2, $5, $4, $6, $7);
                 /* 将函数注册到函数表中 */
                 FunctionDecl *func_decl = create_function_decl($2, $4, $5, $7);
                 add_function(func_decl);
             }
             | FUNCTION IDENTIFIER COLON data_type function_param_sections function_body END_FUNCTION
             {
                 $$ = create_function_node_complete($2, $5, $4, NULL, $6);
                 /* 将函数注册到函数表中 */
                 FunctionDecl *func_decl = create_function_decl($2, $4, $5, $6);
                 add_function(func_decl);
             }
             | FUNCTION IDENTIFIER COLON data_type local_var_decls function_body END_FUNCTION
             {
                 $$ = create_function_node_complete($2, NULL, $4, $5, $6);
                 /* 将函数注册到函数表中 */
                 FunctionDecl *func_decl = create_function_decl($2, $4, NULL, $6);
                 add_function(func_decl);
             }
             | FUNCTION IDENTIFIER COLON data_type function_body END_FUNCTION
             {
                 $$ = create_function_node_complete($2, NULL, $4, NULL, $5);
                 /* 将函数注册到函数表中 */
                 FunctionDecl *func_decl = create_function_decl($2, $4, NULL, $5);
                 add_function(func_decl);
             }
             ;

/* 函数参数声明段 - 可选 */
function_param_sections: /* empty */
                       {
                           $$ = NULL;
                       }
                       | function_param_sections input_param_section
                       {
                           $$ = append_param_list($1, $2);
                       }
                       | function_param_sections output_param_section
                       {
                           $$ = append_param_list($1, $2);
                       }
                       | function_param_sections inout_param_section
                       {
                           $$ = append_param_list($1, $2);
                       }
                       ;

/* 输入参数段 */
input_param_section: VAR_INPUT input_decls END_VAR
                   {
                       $$ = $2;
                   }
                   ;

/* 输出参数段 */
output_param_section: VAR_OUTPUT output_decls END_VAR
                    {
                        $$ = $2;
                    }
                    ;

/* 输入输出参数段 */
inout_param_section: VAR_IN_OUT inout_decls END_VAR
                   {
                       $$ = $2;
                   }
                   ;

/* 输入参数声明 */
input_decls: input_param_decl
           {
               $$ = $1;
           }
           | input_decls input_param_decl
           {
               $$ = append_param_list($1, $2);
           }
           ;

/* 输出参数声明 */
output_decls: output_param_decl
            {
                $$ = $1;
            }
            | output_decls output_param_decl
            {
                $$ = append_param_list($1, $2);
            }
            ;

/* 输入输出参数声明 */
inout_decls: inout_param_decl
           {
               $$ = $1;
           }
           | inout_decls inout_param_decl
           {
               $$ = append_param_list($1, $2);
           }
           ;

/* 具体参数声明 */
input_param_decl: IDENTIFIER COLON data_type SEMICOLON
                {
                    $$ = create_input_param_node($1, $3);
                }
                ;

output_param_decl: IDENTIFIER COLON data_type SEMICOLON
                 {
                     $$ = create_output_param_node($1, $3);
                 }
                 ;

inout_param_decl: IDENTIFIER COLON data_type SEMICOLON
                {
                    $$ = create_inout_param_node($1, $3);
                }
                ;

/* 局部变量声明 - 可选 */
local_var_decls: /* empty */
               {
                   $$ = NULL;
               }
               | VAR var_decl_list END_VAR
               {
                   $$ = $2;
               }
               ;

/* 函数体 */
function_body: statement_list
             {
                 $$ = $1;
             }
             ;

/* 函数块声明 */
function_block_decl: FUNCTION_BLOCK IDENTIFIER function_param_sections local_var_decls function_body END_FUNCTION_BLOCK
                   {
                       $$ = create_function_block_node_complete($2, $3, $4, $5);
                   }
                   | FUNCTION_BLOCK IDENTIFIER function_param_sections function_body END_FUNCTION_BLOCK
                   {
                       $$ = create_function_block_node_complete($2, $3, NULL, $4);
                   }
                   | FUNCTION_BLOCK IDENTIFIER local_var_decls function_body END_FUNCTION_BLOCK
                   {
                       $$ = create_function_block_node_complete($2, NULL, $3, $4);
                   }
                   | FUNCTION_BLOCK IDENTIFIER function_body END_FUNCTION_BLOCK
                   {
                       $$ = create_function_block_node_complete($2, NULL, NULL, $3);
                   }
                   ;

/* 语句 */
statement: assignment_stmt SEMICOLON { $$ = $1; }
         | if_stmt                   { $$ = $1; }
         | for_stmt                  { $$ = $1; }
         | while_stmt                { $$ = $1; }
         | case_stmt                 { $$ = $1; }
         | function_call SEMICOLON   { $$ = $1; }
         | return_stmt SEMICOLON     { $$ = $1; }
         ;

/* 赋值语句 */
assignment_stmt: IDENTIFIER ASSIGN expression
               {
                   $$ = create_assign_node($1, $3);
               }
               ;

/* 函数调用 */
function_call: IDENTIFIER LPAREN argument_list RPAREN
             {
                 $$ = create_function_call_node($1, $3);
             }
             | IDENTIFIER LPAREN RPAREN
             {
                 $$ = create_function_call_node($1, NULL);
             }
             ;

/* 参数列表 */
argument_list: expression
             {
                 $$ = $1;
             }
             | argument_list COMMA expression
             {
                 ASTNode *current = $1;
                 while (current->next != NULL) {
                     current = current->next;
                 }
                 current->next = $3;
                 $$ = $1;
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
             $$ = $1;
         }
         | case_list case_item
         {
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
    | term MUL factor
    {
        $$ = create_binary_op_node(OP_MUL, $1, $3);
    }
    | term DIV factor
    {
        $$ = create_binary_op_node(OP_DIV, $1, $3);
    }
    | term MOD factor
    {
        $$ = create_binary_op_node(OP_MOD, $1, $3);
    }
    ;

factor: IDENTIFIER
      {
          $$ = create_identifier_node($1);
      }
      | MINUS factor %prec UMINUS
      {
          $$ = create_unary_op_node(OP_NEG, $2);
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
      | function_call
      {
          $$ = $1;
      }
      ;

return_stmt: RETURN expression
           {
               $$ = create_return_node($2);
           }
           ;

%%

/* 错误处理函数 */
/* void yyerror(const char *msg) {
    printf("语法错误：第%d行 %s\n", line_num, msg);
} */