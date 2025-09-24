/*
 * filepath: /home/jiang/src/lexer/st_lexer.l
 * IEC61131 结构化文本语言词法分析器
 * 支持：变量声明、基本数据类型、控制结构、表达式、数组
 */

%{
/* 词法分析器头文件包含 */
#include "st_parser.tab.h"  // bison生成的头文件
#include "ast.h"            // AST节点定义
#include "mmgr.h"           // 静态内存管理
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 全局变量声明 */
int yylineno = 1;           // 行号计数器
int yycolumn = 1;           // 列号计数器
extern YYSTYPE yylval;      // bison语义值

/* 行列号更新函数 */
void update_column(void);
void update_line(void);

/* 字符串处理函数 */
char* process_string_literal(const char *str);
char* process_time_literal(const char *str);
char* process_date_literal(const char *str);
%}

/* Flex选项设置 */
%option noyywrap
%option yylineno

/* 状态定义 */
%x COMMENT
%x LINE_COMMENT
%x PRAGMA_STATE

/* 正则表达式定义 */
DIGIT           [0-9]
HEX_DIGIT       [0-9A-Fa-f]
LETTER          [a-zA-Z_]
IDENTIFIER      {LETTER}({LETTER}|{DIGIT})*

/* 数值字面量 - 修正支持负数 */
INTEGER         [+-]?{DIGIT}+
REAL            [+-]?{DIGIT}*\.{DIGIT}+([eE][+-]?{DIGIT}+)?
HEX_INTEGER     [+-]?16#{HEX_DIGIT}+
OCT_INTEGER     [+-]?8#[0-7]+
BIN_INTEGER     [+-]?2#[01]+

/* 字符串和时间字面量 */
STRING          \"([^\"\\]|\\.)*\"
WSTRING         \"([^\"\\]|\\.)*\"
TIME_LITERAL    [+-]?T#{DIGIT}+(ms|s|m|h|d)
DATE_LITERAL    D#{DIGIT}{4}-{DIGIT}{1,2}-{DIGIT}{1,2}
TOD_LITERAL     TOD#{DIGIT}{1,2}:{DIGIT}{1,2}:{DIGIT}{1,2}(\.{DIGIT}+)?
DT_LITERAL      DT#{DIGIT}{4}-{DIGIT}{1,2}-{DIGIT}{1,2}-{DIGIT}{1,2}:{DIGIT}{1,2}:{DIGIT}{1,2}(\.{DIGIT}+)?

/* 编译指令 */
PRAGMA          \{\$[^}]*\}

/* 空白字符 */
WHITESPACE      [ \t]+
NEWLINE         \r?\n

%%

/* ========== 程序结构关键字 ========== */
"PROGRAM"           { update_column(); return PROGRAM; }
"END_PROGRAM"       { update_column(); return END_PROGRAM; }
"FUNCTION"          { update_column(); return FUNCTION; }
"END_FUNCTION"      { update_column(); return END_FUNCTION; }
"FUNCTION_BLOCK"    { update_column(); return FUNCTION_BLOCK; }
"END_FUNCTION_BLOCK" { update_column(); return END_FUNCTION_BLOCK; }

/* ========== 库和导入关键字 ========== */
"LIBRARY"           { update_column(); return LIBRARY; }
"END_LIBRARY"       { update_column(); return END_LIBRARY; }
"IMPORT"            { update_column(); return IMPORT; }
"FROM"              { update_column(); return FROM; }
"AS"                { update_column(); return AS; }
"EXPORT"            { update_column(); return EXPORT; }
"VERSION"           { update_column(); return VERSION; }

/* ========== 变量声明关键字 ========== */
"VAR"               { update_column(); return VAR; }
"END_VAR"           { update_column(); return END_VAR; }
"VAR_INPUT"         { update_column(); return VAR_INPUT; }
"VAR_OUTPUT"        { update_column(); return VAR_OUTPUT; }
"VAR_IN_OUT"        { update_column(); return VAR_IN_OUT; }
"VAR_EXTERNAL"      { update_column(); return VAR_EXTERNAL; }
"VAR_GLOBAL"        { update_column(); return VAR_GLOBAL; }
"VAR_ACCESS"        { update_column(); return VAR_ACCESS; }
"VAR_TEMP"          { update_column(); return VAR_TEMP; }
"VAR_CONFIG"        { update_column(); return VAR_CONFIG; }
"VAR_CONSTANT"      { update_column(); return VAR_CONSTANT; }

/* ========== 控制结构关键字 ========== */
"IF"                { update_column(); return IF; }
"THEN"              { update_column(); return THEN; }
"ELSE"              { update_column(); return ELSE; }
"ELSIF"             { update_column(); return ELSIF; }
"END_IF"            { update_column(); return END_IF; }

"CASE"              { update_column(); return CASE; }
"OF"                { update_column(); return OF; }
"END_CASE"          { update_column(); return END_CASE; }

"FOR"               { update_column(); return FOR; }
"TO"                { update_column(); return TO; }
"BY"                { update_column(); return BY; }
"DO"                { update_column(); return DO; }
"END_FOR"           { update_column(); return END_FOR; }

"WHILE"             { update_column(); return WHILE; }
"END_WHILE"         { update_column(); return END_WHILE; }

"REPEAT"            { update_column(); return REPEAT; }
"UNTIL"             { update_column(); return UNTIL; }
"END_REPEAT"        { update_column(); return END_REPEAT; }

"EXIT"              { update_column(); return EXIT; }
"CONTINUE"          { update_column(); return CONTINUE; }
"RETURN"            { update_column(); return RETURN; }

/* ========== 数据类型关键字 - 基本类型 ========== */
"BOOL"              { update_column(); return TYPE_BOOL; }
"BYTE"              { update_column(); return TYPE_BYTE; }
"WORD"              { update_column(); return TYPE_WORD; }
"DWORD"             { update_column(); return TYPE_DWORD; }
"LWORD"             { update_column(); return TYPE_LWORD; }

/* 整数类型 */
"SINT"              { update_column(); return TYPE_SINT; }
"INT"               { update_column(); return TYPE_INT; }
"DINT"              { update_column(); return TYPE_DINT; }
"LINT"              { update_column(); return TYPE_LINT; }
"USINT"             { update_column(); return TYPE_USINT; }
"UINT"              { update_column(); return TYPE_UINT; }
"UDINT"             { update_column(); return TYPE_UDINT; }
"ULINT"             { update_column(); return TYPE_ULINT; }

/* 浮点类型 */
"REAL"              { update_column(); return TYPE_REAL; }
"LREAL"             { update_column(); return TYPE_LREAL; }

/* 字符串和时间类型 */
"STRING"            { update_column(); return TYPE_STRING; }
"WSTRING"           { update_column(); return TYPE_WSTRING; }
"TIME"              { update_column(); return TYPE_TIME; }
"DATE"              { update_column(); return TYPE_DATE; }
"TIME_OF_DAY"       { update_column(); return TYPE_TIME_OF_DAY; }
"TOD"               { update_column(); return TYPE_TIME_OF_DAY; }
"DATE_AND_TIME"     { update_column(); return TYPE_DATE_AND_TIME; }
"DT"                { update_column(); return TYPE_DATE_AND_TIME; }

/* 结构化类型 */
"ARRAY"             { update_column(); return TYPE_ARRAY; }
"STRUCT"            { update_column(); return TYPE_STRUCT; }
"END_STRUCT"        { update_column(); return END_STRUCT; }
"UNION"             { update_column(); return TYPE_UNION; }
"END_UNION"         { update_column(); return END_UNION; }
"ENUM"              { update_column(); return TYPE_ENUM; }
"END_ENUM"          { update_column(); return END_ENUM; }
"TYPE"              { update_column(); return TYPE; }
"END_TYPE"          { update_column(); return END_TYPE; }

/* ========== 布尔常量 ========== */
"TRUE"              { update_column(); yylval.boolean = 1; return BOOL_LITERAL; }
"FALSE"             { update_column(); yylval.boolean = 0; return BOOL_LITERAL; }

/* ========== 运算符 - 算术运算 ========== */
":="                { update_column(); return ASSIGN; }
"+"                 { update_column(); return PLUS; }
"-"                 { update_column(); return MINUS; }
"*"                 { update_column(); return MULTIPLY; }
"/"                 { update_column(); return DIVIDE; }
"MOD"               { update_column(); return MOD; }
"**"                { update_column(); return POWER; }

/* 比较运算符 */
"="                 { update_column(); return EQ; }
"<>"                { update_column(); return NE; }
"<"                 { update_column(); return LT; }
"<="                { update_column(); return LE; }
">"                 { update_column(); return GT; }
">="                { update_column(); return GE; }

/* 逻辑运算符 */
"AND"               { update_column(); return AND; }
"OR"                { update_column(); return OR; }
"XOR"               { update_column(); return XOR; }
"NOT"               { update_column(); return NOT; }

/* 位运算符 */
"AND"               { update_column(); return AND; }  /* 位与 */
"OR"                { update_column(); return OR; }   /* 位或 */
"XOR"               { update_column(); return XOR; }  /* 位异或 */

/* ========== 分隔符和标点符号 ========== */
"("                 { update_column(); return LPAREN; }
")"                 { update_column(); return RPAREN; }
"["                 { update_column(); return LBRACKET; }
"]"                 { update_column(); return RBRACKET; }
"{"                 { update_column(); return LBRACE; }
"}"                 { update_column(); return RBRACE; }
";"                 { update_column(); return SEMICOLON; }
":"                 { update_column(); return COLON; }
","                 { update_column(); return COMMA; }
"."                 { update_column(); return DOT; }
".."                { update_column(); return RANGE; }

/* ========== 数值字面量处理 - 修正负数支持 ========== */
{INTEGER}           { 
    update_column(); 
    yylval.integer = atoi(yytext); 
    return INTEGER_LITERAL; 
}

{HEX_INTEGER}       { 
    update_column(); 
    /* 处理带符号的十六进制数 */
    if (yytext[0] == '+' || yytext[0] == '-') {
        int sign = (yytext[0] == '-') ? -1 : 1;
        yylval.integer = sign * (int)strtol(yytext + 4, NULL, 16); /* 跳过符号和"16#" */
    } else {
        yylval.integer = (int)strtol(yytext + 3, NULL, 16); /* 跳过"16#" */
    }
    return INTEGER_LITERAL; 
}

{OCT_INTEGER}       { 
    update_column(); 
    /* 处理带符号的八进制数 */
    if (yytext[0] == '+' || yytext[0] == '-') {
        int sign = (yytext[0] == '-') ? -1 : 1;
        yylval.integer = sign * (int)strtol(yytext + 3, NULL, 8); /* 跳过符号和"8#" */
    } else {
        yylval.integer = (int)strtol(yytext + 2, NULL, 8); /* 跳过"8#" */
    }
    return INTEGER_LITERAL; 
}

{BIN_INTEGER}       { 
    update_column(); 
    /* 处理带符号的二进制数 */
    if (yytext[0] == '+' || yytext[0] == '-') {
        int sign = (yytext[0] == '-') ? -1 : 1;
        yylval.integer = sign * (int)strtol(yytext + 3, NULL, 2); /* 跳过符号和"2#" */
    } else {
        yylval.integer = (int)strtol(yytext + 2, NULL, 2); /* 跳过"2#" */
    }
    return INTEGER_LITERAL; 
}

{REAL}              { 
    update_column(); 
    yylval.real = atof(yytext); 
    return REAL_LITERAL; 
}

/* ========== 字符串字面量处理 ========== */
{STRING}            { 
    update_column(); 
    yylval.string = process_string_literal(yytext); 
    if (yylval.string == NULL) {
        return ERROR_TOKEN;
    }
    return STRING_LITERAL; 
}

{WSTRING}           { 
    update_column(); 
    yylval.string = process_string_literal(yytext); 
    if (yylval.string == NULL) {
        return ERROR_TOKEN;
    }
    return WSTRING_LITERAL; 
}

/* ========== 时间和日期字面量 ========== */
{TIME_LITERAL}      { 
    update_column(); 
    yylval.string = process_time_literal(yytext); 
    if (yylval.string == NULL) {
        return ERROR_TOKEN;
    }
    return TIME_LITERAL; 
}

{DATE_LITERAL}      { 
    update_column(); 
    yylval.string = process_date_literal(yytext); 
    if (yylval.string == NULL) {
        return ERROR_TOKEN;
    }
    return DATE_LITERAL; 
}

{TOD_LITERAL}       { 
    update_column(); 
    yylval.string = mmgr_alloc_string(yytext); 
    if (yylval.string == NULL) {
        return ERROR_TOKEN;
    }
    return TOD_LITERAL; 
}

{DT_LITERAL}        { 
    update_column(); 
    yylval.string = mmgr_alloc_string(yytext); 
    if (yylval.string == NULL) {
        return ERROR_TOKEN;
    }
    return DT_LITERAL; 
}

/* ========== 标识符处理 ========== */
{IDENTIFIER}        { 
    update_column(); 
    yylval.string = mmgr_alloc_string(yytext);
    if (yylval.string == NULL) {
        return ERROR_TOKEN;
    }
    return IDENTIFIER; 
}

/* ========== 编译器指令处理 ========== */
{PRAGMA}            { 
    update_column(); 
    BEGIN(PRAGMA_STATE);
    yylval.string = mmgr_alloc_string(yytext);
    return PRAGMA;
}

<PRAGMA_STATE>\}    { 
    update_column(); 
    BEGIN(INITIAL); 
}

<PRAGMA_STATE>.     { 
    update_column(); 
    /* 处理编译指令内容 */ 
}

/* ========== 空白字符处理 ========== */
{WHITESPACE}        { update_column(); }
{NEWLINE}           { update_line(); }

/* ========== 注释处理 ========== */
"(*"                { 
    update_column(); 
    BEGIN(COMMENT); 
}

<COMMENT>"*)"       { 
    update_column(); 
    BEGIN(INITIAL); 
}

<COMMENT>{NEWLINE}  { update_line(); }

<COMMENT>.          { 
    update_column(); 
    /* 忽略注释内容 */ 
}

"//"                { 
    update_column(); 
    BEGIN(LINE_COMMENT); 
}

<LINE_COMMENT>{NEWLINE} { 
    update_line(); 
    BEGIN(INITIAL); 
}

<LINE_COMMENT>.     { 
    update_column(); 
    /* 忽略单行注释内容 */ 
}

/* ========== 错误处理 ========== */
.                   { 
    update_column(); 
    fprintf(stderr, "词法错误: 未识别字符 '%c' 在行 %d, 列 %d\n", 
            yytext[0], yylineno, yycolumn);
    return ERROR_TOKEN; 
}

%%

/* ========== 辅助函数实现 ========== */

/* 更新列号 */
void update_column(void) {
    yycolumn += yyleng;
}

/* 更新行号 */
void update_line(void) {
    yylineno++;
    yycolumn = 1;
}

/* 处理字符串字面量 */
char* process_string_literal(const char *str) {
    if (!str) return NULL;
    
    int len = strlen(str);
    if (len < 2) return NULL; /* 至少需要两个引号 */
    
    /* 分配字符串（去掉首尾引号） */
    char *processed = mmgr_alloc_string_with_length(str + 1, len - 2);
    if (!processed) return NULL;
    
    /* 处理转义字符 */
    char *src = processed;
    char *dst = processed;
    
    while (*src) {
        if (*src == '\\' && *(src + 1)) {
            src++; /* 跳过反斜杠 */
            switch (*src) {
                case 'n':  *dst++ = '\n'; break;
                case 't':  *dst++ = '\t'; break;
                case 'r':  *dst++ = '\r'; break;
                case '\\': *dst++ = '\\'; break;
                case '"':  *dst++ = '"';  break;
                case '\'': *dst++ = '\''; break;
                default:   *dst++ = *src; break; /* 保持原字符 */
            }
        } else {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
    
    return processed;
}

/* 处理时间字面量 - 修正支持负时间 */
char* process_time_literal(const char *str) {
    if (!str) return NULL;
    
    /* 检查是否为负时间 */
    int is_negative = 0;
    const char* start = str;
    
    if (str[0] == '-') {
        is_negative = 1;
        start = str + 1; /* 跳过负号 */
    } else if (str[0] == '+') {
        start = str + 1; /* 跳过正号 */
    }
    
    /* 验证格式：T#数字+单位 */
    if (start[0] != 'T' || start[1] != '#') {
        return NULL;
    }
    
    /* 简单处理：直接存储原始字符串 */
    /* 实际实现中可以解析时间值并转换为标准格式 */
    return mmgr_alloc_string(str);
}

/* 处理日期字面量 */
char* process_date_literal(const char *str) {
    if (!str) return NULL;
    
    /* 简单处理：直接存储原始字符串 */
    /* 实际实现中可以解析日期值并转换为标准格式 */
    return mmgr_alloc_string(str);
}

/* yywrap函数 - Flex要求 */
int yywrap(void) {
    return 1; /* 表示没有更多输入文件 */
}

/* 获取当前位置信息 */
void get_current_position(int *line, int *column) {
    if (line) *line = yylineno;
    if (column) *column = yycolumn;
}

/* 重置词法分析器状态 */
void reset_lexer_state(void) {
    yylineno = 1;
    yycolumn = 1;
    BEGIN(INITIAL);
}
