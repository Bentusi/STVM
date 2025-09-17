/*
 * IEC61131 结构化文本语言语法分析器
 * 支持：一个源文件中包含一个program和多个function
 */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

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
%token PROGRAM END_PROGRAM VAR END_VAR VAR_INPUT VAR_OUTPUT VAR_IN_OUT FUNCTION END_FUNCTION FUNCTION_BLOCK END_FUNCTION_BLOCK
%token BOOL_TYPE INT_TYPE REAL_TYPE STRING_TYPE
%token IF THEN ELSE ELSIF END_IF FOR TO BY DO END_FOR WHILE END_WHILE
%token CASE OF END_CASE RETURN

/* 运算符 */
%token ASSIGN EQ NE LT LE GT GE PLUS MINUS MUL DIV MOD AND OR NOT

/* 分隔符 */
%token SEMICOLON COLON COMMA DOT LPAREN RPAREN LBRACKET RBRACKET

/* 错误标识 */
%token ERROR

/* 非终结符类型 */
%type <node> compilation_unit program_and_functions program_decl function_list function_decl
%type <node> statement_list statement assignment_stmt function_call_stmt
%type <node> if_stmt for_stmt while_stmt case_stmt case_item case_list return_stmt
%type <node> expression logical_expr comparison term factor function_call
%type <node> argument_list argument_expression
%type <var_decl> var_section var_decl_list var_declaration parameter_list parameter_decl
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

/* 编译单元 - 顶层规则：支持函数列表+程序 或 只有程序 */
compilation_unit: program_and_functions
                {
                    $$ = $1;
                    ast_root = $$;
                }
                ;

/* 程序和函数组合 */
program_and_functions: function_list program_decl
                     {
                         $$ = create_compilation_unit_node($1, $2);
                     }
                     | program_decl
                     {
                         $$ = create_compilation_unit_node(NULL, $1);
                     }
                     ;

/* 程序声明 */
program_decl: PROGRAM IDENTIFIER var_section statement_list END_PROGRAM
            {
                $$ = create_program_node($2, $4);
            }
            | PROGRAM IDENTIFIER statement_list END_PROGRAM
            {
                $$ = create_program_node($2, $3);
            }
            | PROGRAM IDENTIFIER END_PROGRAM
            {
                $$ = create_program_node($2, NULL);
            }
            ;

/* 函数列表 - 支持多个函数声明 */
function_list: function_decl
             {
                 $$ = $1;
                 add_global_function($1);
             }
             | function_list function_decl
             {
                /* 将新函数添加到列表尾 */
                ASTNode *current = $1;
                while (current->next != NULL) {
                    current = current->next;
                }
                current->next = $2;
                $$ = $1;
                add_global_function($2);
             }
             ;

/* 函数声明 */
function_decl: FUNCTION IDENTIFIER LPAREN parameter_list RPAREN COLON data_type var_section statement_list END_FUNCTION
             {
                 /* 验证函数是否重复定义 */
                 if (validate_function_call($2, NULL) == 0) {
                     yyerror("函数重复定义");
                     YYERROR;
                 }
                 $$ = create_function_node($2, $7, $4, $9);
                 /* 设置变量声明到函数中 */
                 if ($8) {
                     add_function_var_decl($2, $8);
                 }
             }
             | FUNCTION IDENTIFIER LPAREN parameter_list RPAREN COLON data_type statement_list END_FUNCTION
             {
                /* 验证函数是否重复定义 */
                 if (validate_function_call($2, NULL) == 0) {
                     yyerror("函数重复定义");
                     YYERROR;
                 }
                 $$ = create_function_node($2, $7, $4, $8);
             }
             | FUNCTION IDENTIFIER COLON data_type var_section statement_list END_FUNCTION
             {
                /* 验证函数是否重复定义 */
                 if (validate_function_call($2, NULL) == 0) {
                     yyerror("函数重复定义");
                     YYERROR;
                 }
                 $$ = create_function_node($2, $4, NULL, $6);
             }
             | FUNCTION IDENTIFIER COLON data_type statement_list END_FUNCTION
             {
                /* 验证函数是否重复定义 */
                 if (validate_function_call($2, NULL) == 0) {
                     yyerror("函数重复定义");
                     YYERROR;
                 }
                 $$ = create_function_node($2, $4, NULL, $5);
             }
             ;

/* 变量声明段 */
var_section: /* empty */ { $$ = NULL; }
            | VAR var_decl_list END_VAR 
            { 
                $$ = $2; 
                /* 确保变量声明被正确处理 */
                if ($2) {
                    printf("Debug: 解析到变量声明段\n");
                }
            }
            ;

var_decl_list: var_declaration
                {
                    $$ = $1;
                    if ($1) {
                        printf("Debug: 添加变量 %s\n", $1->name);
                        add_global_variable($1);
                    }
                }
                | var_decl_list var_declaration
                {
                    if ($1 && $2) {
                        /* 将新变量添加到列表末尾 */
                        VarDecl *current = $1;
                        while (current->next != NULL) {
                            current = current->next;
                        }
                        current->next = $2;
                        $$ = $1;
                        printf("Debug: 添加变量 %s 到列表\n", $2->name);
                        add_global_variable($2);
                    }
                }
                ;

var_declaration: IDENTIFIER COLON data_type SEMICOLON
                {
                    $$ = create_var_decl($1, $3);
                    if ($$) {
                        printf("Debug: 创建变量声明 %s, 类型 %d\n", $1, $3);
                    }
                }
                ;

data_type: BOOL_TYPE   { $$ = TYPE_BOOL; }
         | INT_TYPE    { $$ = TYPE_INT; }
         | REAL_TYPE   { $$ = TYPE_REAL; }
         | STRING_TYPE { $$ = TYPE_STRING; }
         ;

/* 语句列表 */
statement_list: /* empty */ { $$ = NULL; }
              | statement
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
         | function_call_stmt SEMICOLON { $$ = $1; }
         | if_stmt                   { $$ = $1; }
         | for_stmt                  { $$ = $1; }
         | while_stmt                { $$ = $1; }
         | case_stmt                 { $$ = $1; }
         | return_stmt SEMICOLON     { $$ = $1; }
         ;

/* 赋值语句 */
assignment_stmt: IDENTIFIER ASSIGN expression
               {
                   $$ = create_assign_node($1, $3);
               }
               ;

/* 函数调用语句 */
function_call_stmt: function_call
                  {
                      $$ = $1;
                  }
                  ;

/* RETURN语句 */
return_stmt: RETURN
           {
               $$ = create_return_node(NULL);
           }
           | RETURN expression
           {
               $$ = create_return_node($2);
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
             $$ = create_statement_list($1);
         }
         | case_list case_item
         {
             $$ = add_statement($1, $2);
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
      | function_call
      {
          $$ = $1;
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
      | factor MUL factor
      {
          $$ = create_binary_op_node(OP_MUL, $1, $3);
      }
      | factor DIV factor
      {
          $$ = create_binary_op_node(OP_DIV, $1, $3);
      }
      ;

/* 函数调用 */
function_call: IDENTIFIER LPAREN RPAREN
             {
                 /* 验证函数是否存在 */
                 if (validate_function_call($1, NULL) != 0) {
                     yyerror("函数不存在或参数不匹配");
                     YYERROR;
                 }
                 $$ = create_function_call_node($1, NULL);
             }
             | IDENTIFIER LPAREN argument_list RPAREN
             {
                 /* 验证函数是否存在并检查参数 */
                 if (validate_function_call($1, $3) != 0) {
                     yyerror("函数不存在或参数不匹配");
                     YYERROR;
                 }
                 $$ = create_function_call_node($1, $3);
             }
             ;

/* 参数列表 */
argument_list: argument_expression
             {
                 $$ = create_argument_list($1);
             }
             | argument_list COMMA argument_expression
             {
                 $$ = add_argument($1, $3);
             }
             ;

/* 参数表达式 */
argument_expression: expression
                   {
                       $$ = $1;
                   }
                   ;

/* 函数参数列表 */
parameter_list: /* empty */ { $$ = NULL; }
              | parameter_decl
              {
                  $$ = $1;
              }
              | parameter_list COMMA parameter_decl
              {
                  $$ = add_parameter($1, $3);
              }
              ;

/* 参数声明 */
parameter_decl: IDENTIFIER COLON data_type
              {
                  $$ = create_var_decl($1, $3);
              }
              ;

%%

/* 错误处理函数 */
void yyerror(const char *msg) {
    printf("语法错误：第%d行 %s\n", line_num, msg);
}

/* 主函数 */
/*
int main(int argc, char **argv) {
    extern FILE *yyin;
    
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            printf("无法打开文件 %s\n", argv[1]);
            return 1;
        }
    }
    
    printf("开始解析IEC61131结构化文本程序...\n");
    
    if (yyparse() == 0) {
        printf("解析成功！\n\n");
        
        // 打印函数信息
        print_function_info();
        
        if (ast_root) {
            printf("=== 抽象语法树 ===\n");
            print_ast(ast_root, 0);
            printf("\n=== 解析完成 ===\n");
            
            // 释放AST内存
            free_ast(ast_root);
        }
    } else {
        printf("解析失败！\n");
    }
    
    if (argc > 1) {
        fclose(yyin);
    }
    
    // 清理全局符号表
    clear_global_functions();
    clear_global_variables();
    
    return 0;
}
*/