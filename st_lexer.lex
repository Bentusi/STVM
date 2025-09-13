/*
 * filepath: /home/jiang/src/lexer/st_lexer.lex
 * IEC61131 结构化文本语言词法分析器
 * 支持：变量声明、基本数据类型、控制结构、表达式
 */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "st_parser.tab.h"

  /* 行号计数 */
int line_num = 1;
%}

  /* 词法规则定义 */
%option noyywrap

  /* 正则表达式定义 */
DIGIT      [0-9]
LETTER     [a-zA-Z]
ID         {LETTER}({LETTER}|{DIGIT}|_)*
INTEGER    {DIGIT}+
REAL       {DIGIT}+\.{DIGIT}+
STRING     \"[^\"]*\"
COMMENT    \(\*[^*]*\*+([^)*][^*]*\*+)*\)

%%

  /* IEC61131关键字 */
"PROGRAM"            { return PROGRAM; }
"END_PROGRAM"        { return END_PROGRAM; }
"VAR"                { return VAR; }
"END_VAR"            { return END_VAR; }
"VAR_INPUT"          { return VAR_INPUT; }
"VAR_OUTPUT"         { return VAR_OUTPUT; }
"VAR_IN_OUT"         { return VAR_IN_OUT; }
"FUNCTION"           { return FUNCTION; }
"END_FUNCTION"       { return END_FUNCTION; }
"FUNCTION_BLOCK"     { return FUNCTION_BLOCK; }
"END_FUNCTION_BLOCK" { return END_FUNCTION_BLOCK; }

  /* 数据类型关键字 */
"BOOL"          { return BOOL_TYPE; }
"INT"           { return INT_TYPE; }
"REAL"          { return REAL_TYPE; }
"STRING"        { return STRING_TYPE; }

  /* 控制结构关键字 */
"IF"            { return IF; }
"THEN"          { return THEN; }
"ELSE"          { return ELSE; }
"ELSIF"         { return ELSIF; }
"END_IF"        { return END_IF; }
"FOR"           { return FOR; }
"TO"            { return TO; }
"BY"            { return BY; }
"DO"            { return DO; }
"END_FOR"       { return END_FOR; }
"WHILE"         { return WHILE; }
"END_WHILE"     { return END_WHILE; }
"CASE"          { return CASE; }
"OF"            { return OF; }
"END_CASE"      { return END_CASE; }

  /* 布尔常量 */
"TRUE"          { yylval.bool_val = 1; return BOOL_LITERAL; }
"FALSE"         { yylval.bool_val = 0; return BOOL_LITERAL; }

  /* 标识符和字面量 */
{ID}            { yylval.str_val = strdup(yytext); return IDENTIFIER; }
{INTEGER}       { yylval.int_val = atoi(yytext); return INT_LITERAL; }
{REAL}          { yylval.real_val = atof(yytext); return REAL_LITERAL; }
{STRING}        { 
                  yylval.str_val = strdup(yytext + 1); 
                  yylval.str_val[strlen(yylval.str_val) - 1] = '\0'; 
                  return STRING_LITERAL; 
                }

  /* 运算符 */
":="            { return ASSIGN; }
"="             { return EQ; }
"<>"            { return NE; }
"<"             { return LT; }
"<="            { return LE; }
">"             { return GT; }
">="            { return GE; }
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return MUL; }
"/"             { return DIV; }
"MOD"           { return MOD; }
"AND"           { return AND; }
"OR"            { return OR; }
"NOT"           { return NOT; }

  /* 分隔符 */
";"             { return SEMICOLON; }
":"             { return COLON; }
","             { return COMMA; }
"."             { return DOT; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"["             { return LBRACKET; }
"]"             { return RBRACKET; }

  /* 空白字符和注释 */
{COMMENT}       {   /* 忽略注释 */ }
[ \t\r]+        {   /* 忽略空白 */ }
\n              { line_num++; }

  /* 错误处理 */
.               { 
                  printf("错误：第%d行未识别字符 '%s'\n", line_num, yytext); 
                  return ERROR; 
                }

%%

  /* 用户代码段 */
void yyerror(const char *msg) {
    printf("词法错误：第%d行 %s\n", line_num, msg);
}
