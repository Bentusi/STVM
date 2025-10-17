%{
/**
 * @file st_lexer.l
 * @brief ST语言词法分析器 (Flex)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "st_parser.tab.h"
#include "mmgr.h"

/* 当前列号 */
int yycolumn = 1;

/* 更新位置信息 */
#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;

%}

%option noyywrap
%option yylineno

/* 定义正则表达式片段 */
DIGIT       [0-9]
LETTER      [a-zA-Z_]
IDENTIFIER  {LETTER}({LETTER}|{DIGIT})*
INTEGER     {DIGIT}+
REAL        {DIGIT}+\.{DIGIT}+([eE][+-]?{DIGIT}+)?
STRING      '([^'\n]|'')*'

%%

    /* 关键字 (不区分大小写) */
(?i:PROGRAM)        { return TOKEN_PROGRAM; }
(?i:END_PROGRAM)    { return TOKEN_END_PROGRAM; }
(?i:BEGIN)          { return TOKEN_BEGIN; }
(?i:FUNCTION)       { return TOKEN_FUNCTION; }
(?i:END_FUNCTION)   { return TOKEN_END_FUNCTION; }
(?i:PRINT)          { return TOKEN_PRINT; }
(?i:VAR)            { return TOKEN_VAR; }
(?i:VAR_INPUT)      { return TOKEN_VAR_INPUT; }
(?i:VAR_OUTPUT)     { return TOKEN_VAR_OUTPUT; }
(?i:END_VAR)        { return TOKEN_END_VAR; }
(?i:IF)             { return TOKEN_IF; }
(?i:THEN)           { return TOKEN_THEN; }
(?i:ELSIF)          { return TOKEN_ELSIF; }
(?i:ELSE)           { return TOKEN_ELSE; }
(?i:END_IF)         { return TOKEN_END_IF; }
(?i:CASE)           { return TOKEN_CASE; }
(?i:OF)             { return TOKEN_OF; }
(?i:END_CASE)       { return TOKEN_END_CASE; }
(?i:FOR)            { return TOKEN_FOR; }
(?i:TO)             { return TOKEN_TO; }
(?i:BY)             { return TOKEN_BY; }
(?i:DO)             { return TOKEN_DO; }
(?i:END_FOR)        { return TOKEN_END_FOR; }
(?i:WHILE)          { return TOKEN_WHILE; }
(?i:END_WHILE)      { return TOKEN_END_WHILE; }
(?i:REPEAT)         { return TOKEN_REPEAT; }
(?i:UNTIL)          { return TOKEN_UNTIL; }
(?i:END_REPEAT)     { return TOKEN_END_REPEAT; }
(?i:RETURN)         { return TOKEN_RETURN; }
(?i:EXIT)           { return TOKEN_EXIT; }
(?i:IMPORT)         { return TOKEN_IMPORT; }
(?i:AS)             { return TOKEN_AS; }

    /* 数据类型关键字 */
(?i:BOOL)           { return TOKEN_BOOL; }
(?i:INT)            { return TOKEN_INT; }
(?i:REAL)           { return TOKEN_REAL; }
(?i:STRING)         { return TOKEN_STRING; }
(?i:ARRAY)          { return TOKEN_ARRAY; }

    /* 布尔字面量 */
(?i:TRUE)           { yylval.bool_value = 1; return TOKEN_TRUE; }
(?i:FALSE)          { yylval.bool_value = 0; return TOKEN_FALSE; }

    /* 逻辑运算符关键字 */
(?i:MOD)            { return TOKEN_MOD; }
(?i:AND)            { return TOKEN_AND; }
(?i:OR)             { return TOKEN_OR; }
(?i:XOR)            { return TOKEN_XOR; }
(?i:NOT)            { return TOKEN_NOT; }

    /* 整数字面量 */
{INTEGER}           { 
                        yylval.int_value = atoll(yytext); 
                        return TOKEN_INTEGER_LITERAL; 
                    }

    /* 实数字面量 */
{REAL}              { 
                        yylval.real_value = atof(yytext); 
                        return TOKEN_REAL_LITERAL; 
                    }

    /* 字符串字面量 */
{STRING}            {
                        /* 去掉首尾引号，处理转义的单引号 '' -> ' */
                        int len = yyleng - 2;
                        char* str = mmgr_alloc(len + 1);
                        int j = 0;
                        for (int i = 1; i < yyleng - 1; i++) {
                            str[j++] = yytext[i];
                            if (yytext[i] == '\'' && yytext[i+1] == '\'') {
                                i++; /* 跳过第二个单引号 */
                            }
                        }
                        str[j] = '\0';
                        yylval.string_value = str;
                        return TOKEN_STRING_LITERAL;
                    }

    /* 标识符 */
{IDENTIFIER}        { 
                        yylval.string_value = mmgr_strdup(yytext); 
                        return TOKEN_IDENTIFIER; 
                    }

    /* 运算符 */
":="                { return TOKEN_ASSIGN; }
"="                 { return TOKEN_EQ; }
"<>"                { return TOKEN_NE; }
"<="                { return TOKEN_LE; }
">="                { return TOKEN_GE; }
"<"                 { return TOKEN_LT; }
">"                 { return TOKEN_GT; }
"+"                 { return TOKEN_PLUS; }
"-"                 { return TOKEN_MINUS; }
"*"                 { return TOKEN_MULTIPLY; }
"/"                 { return TOKEN_DIVIDE; }

    /* 分隔符 */
";"                 { return TOKEN_SEMICOLON; }
":"                 { return TOKEN_COLON; }
","                 { return TOKEN_COMMA; }
"."                 { return TOKEN_DOT; }
".."                { return TOKEN_RANGE; }
"("                 { return TOKEN_LPAREN; }
")"                 { return TOKEN_RPAREN; }
"["                 { return TOKEN_LBRACKET; }
"]"                 { return TOKEN_RBRACKET; }

    /* 行注释 // ... */
"//"[^\n]*          { /* 忽略行注释 */ }

    /* 块注释 (* ... *) */
"(*"                {
                        int c;
                        while ((c = input()) != 0) {
                            if (c == '\n') {
                                yylineno++;
                                yycolumn = 1;
                            } else if (c == '*') {
                                if ((c = input()) == ')') {
                                    break;
                                } else {
                                    unput(c);
                                }
                            }
                        }
                    }

    /* 块注释 (* ... *) */
"(*"                {
                        int c;
                        while ((c = input()) != 0) {
                            if (c == '\n') {
                                yylineno++;
                                yycolumn = 1;
                            } else if (c == '*') {
                                if ((c = input()) == ')') {
                                    break;
                                } else {
                                    unput(c);
                                }
                            }
                        }
                    }

    /* 空白字符 */
[ \t\r]+            { /* 忽略空白 */ }
\n                  { yylineno++; yycolumn = 1; }

    /* 其他字符报错 */
.                   { 
                        fprintf(stderr, "Lexical error at line %d, column %d: unexpected character '%s'\n", 
                                yylineno, yycolumn, yytext);
                        return TOKEN_ERROR;
                    }

%%

/* 初始化词法分析器 */
void lexer_init(FILE* file) {
    yyin = file;
    yylineno = 1;
    yycolumn = 1;
}

/* 清理词法分析器 */
void lexer_cleanup(void) {
    /* Flex会自动清理 */
}
