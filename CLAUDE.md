# IEC 61131-3 ST语言编译器与虚拟机完整实现架构

一个完整的IEC61131结构化文本语言编译器和虚拟机实现，使用Flex和Bison构建，支持平台无关的字节码执行。

## 特性

- **完整的词法分析**：支持IEC61131标准的关键字、标识符、字面量和运算符
- **语法分析**：使用Bison实现的语法分析器，生成抽象语法树(AST)
- **虚拟机执行**：平台无关的字节码虚拟机，支持控制流和表达式计算
- **数据类型支持**：BOOL、INT、REAL、STRING基本数据类型
- **控制结构**：IF-THEN-ELSE、FOR循环、WHILE循环、CASE分支
- **函数支持**：函数定义、函数调用、参数传递、返回值处理
- **模块化系统**：库导入、命名空间、第三方函数库支持
- **标准库集成**：数学、字符串、时间、I/O等标准函数库
- **表达式计算**：算术运算、比较运算、逻辑运算
- **调试支持**：断点、单步执行、变量监视
- **双机热备**：主备同步、故障切换
- **安全级设计**：MISRA合规、静态内存管理

## 构建要求

- GCC编译器
- Flex (词法分析器生成器)
- Bison (语法分析器生成器)
- Make构建工具

## 安装依赖

### Ubuntu/Debian
```bash
sudo apt-get install gcc flex bison make
```

### CentOS/RHEL
```bash
sudo yum install gcc flex bison make
```

### macOS
```bash
brew install flex bison gcc make
```

## 构建

```bash
# 构建编译器
make

# 运行测试
make test

# 清理生成文件
make clean

# 安装到系统
make install
```

## 使用方法

### 1. 编译文件
```bash
./stc program.st
```

### 2. 交互模式
```bash
./stc -i
```

### 3. 帮助信息
```bash
./stc -h
```

## 系统总体架构

### 核心模块划分
```
ST源码 → 词法分析(Flex) → 语法分析(Bison) → AST → 语义分析 → 字节码生成 → 字节码文件
                                                     ↓           ↓
库管理器 ← 导入解析器 ← 符号表管理 ← 类型检查器 ← AST遍历器
                                                     ↓
主从同步管理器 ← 外部I/O ← 调试接口 ← 栈式虚拟机 ← 字节码加载器 ← 字节码文件
     ↓                                                      ↑
备机虚拟机 ← 状态同步协议 ← 心跳检测 ← 数据一致性检查 ← 函数库加载器
```

### 技术栈选择
- **词法分析**: GNU Flex (词法生成器)
- **语法分析**: GNU Bison (LALR解析器生成器)  
- **字节码格式**: 自定义二进制格式 + 魔数验证
- **虚拟机**: 栈式架构 + 寄存器式局部变量
- **内存管理**: 静态预分配 + 内存池 (安全级设计)
- **调试协议**: 基于TCP/IP的调试代理
- **同步机制**: 基于心跳的主备切换 (简化版)

## 语法支持

### 程序结构
```
PROGRAM MyProgram

(* 库导入声明 *)
IMPORT MathLib;
IMPORT StringLib AS Str;
IMPORT MyCustomLib FROM "libs/custom.st";

VAR
    variable_declarations
END_VAR

    function_declarations
    
    statements

END_PROGRAM
```

### 库导入语法

#### 基本导入
```
(* 导入整个库 *)
IMPORT MathLib;

(* 使用别名导入 *)
IMPORT StringLibrary AS Str;

(* 从指定路径导入 *)
IMPORT CustomFunctions FROM "libs/custom.st";

(* 导入特定函数 *)
IMPORT (Sin, Cos, Tan) FROM MathLib;

(* 导入并使用别名 *)
IMPORT (Sin AS Sine, Cos AS Cosine) FROM MathLib;
```

#### 条件导入
```
(* 根据编译条件导入不同库 *)
{$IFDEF SIMULATION}
    IMPORT SimulationLib;
{$ELSE}
    IMPORT HardwareLib;
{$ENDIF}

(* 版本检查导入 *)
IMPORT MathLib VERSION >= "2.0";
```

### 库定义语法

#### 创建函数库文件 (MathLib.st)
```
LIBRARY MathLib VERSION "2.1.0"

(* 库级常量定义 *)
VAR_CONSTANT
    PI : REAL := 3.14159265359;
    E : REAL := 2.71828182846;
END_VAR

(* 导出函数声明 *)
FUNCTION Sin : REAL
VAR_INPUT
    angle : REAL;
END_VAR
EXPORT;  (* 标记为可导出 *)

FUNCTION Cos : REAL  
VAR_INPUT
    angle : REAL;
END_VAR
EXPORT;

(* 函数实现 *)
Sin := SIN_BUILTIN(NormalizeAngle(angle));
Cos := COS_BUILTIN(NormalizeAngle(angle));

END_LIBRARY
```

### 全局变量声明
```
VAR
    counter : INT;
    flag : BOOL;
    temperature : REAL;
    message : STRING;
END_VAR
```

### 函数定义

#### 无返回值函数 (FUNCTION_BLOCK)
```
FUNCTION_BLOCK FB_Counter
VAR_INPUT
    reset : BOOL;
    count_up : BOOL;
END_VAR
VAR_OUTPUT
    counter_value : INT;
END_VAR
VAR
    internal_count : INT;
END_VAR

IF reset THEN
    internal_count := 0;
ELSIF count_up THEN
    internal_count := internal_count + 1;
END_IF

counter_value := internal_count;

END_FUNCTION_BLOCK
```

#### 有返回值函数 (FUNCTION)
```
FUNCTION Add : INT
VAR_INPUT
    a : INT;
    b : INT;
END_VAR

Add := a + b;

END_FUNCTION

FUNCTION Factorial : INT
VAR_INPUT
    n : INT;
END_VAR

IF n <= 1 THEN
    Factorial := 1;
ELSE
    Factorial := n * Factorial(n - 1);
END_IF

END_FUNCTION
```

### 函数调用

#### 作为表达式的函数调用
```
VAR
    result : INT;
    max_val : REAL;
    flag : BOOL;
END_VAR

result := Add(10, 20);
max_val := Max(25.5, 30.2);
flag := IsEven(42);

// 函数调用作为参数
result := Add(Add(1, 2), Add(3, 4));

// 复杂表达式中的函数调用
IF IsEven(counter) AND (Max(temp1, temp2) > 25.0) THEN
    result := Add(counter, 5);
END_IF
```

### 参数传递方式

#### 按值传递 (VAR_INPUT)
```
FUNCTION TestByValue : INT
VAR_INPUT
    input_val : INT;  (* 按值传递，函数内修改不影响外部 *)
END_VAR

input_val := input_val * 2;  (* 只影响函数内部副本 *)
TestByValue := input_val;

END_FUNCTION
```

#### 按引用传递 (VAR_IN_OUT)
```
FUNCTION SwapValues
VAR_IN_OUT
    a : INT;  (* 按引用传递，函数内修改会影响外部 *)
    b : INT;
END_VAR
VAR
    temp : INT;
END_VAR

temp := a;
a := b;
b := temp;

END_FUNCTION
```

### 赋值语句
```
counter := 10;
flag := TRUE;
temperature := 25.5;
message := "test string";
val := getVal(counter + 1);
```

### 控制结构

#### IF语句
```
IF temperature > 30 THEN
    flag := TRUE;
ELSE
    flag := FALSE;
END_IF

IF temperature > 30 THEN
    flag := TRUE;
ELSIF temperature > 50 THEN
    flag := FALSE;
ELSE temperature > 60 THEN
   flag := TRUE;
END_IF
```

#### FOR循环
```
FOR i := 1 TO 10 DO
    sum := sum + i;
END_FOR
```

#### WHILE循环
```
WHILE counter > 0 DO
    counter := counter - 1;
END_WHILE
```

### 表达式
- 算术运算：`+`, `-`, `*`, `/`, `MOD`
- 比较运算：`=`, `<>`, `<`, `<=`, `>`, `>=`
- 逻辑运算：`AND`, `OR`, `NOT`

## 示例程序

### 完整的温度控制系统
```
PROGRAM AdvancedTemperatureControl
(* 导入所需库 *)
IMPORT MathLib;
IMPORT TimeLib;
IMPORT IOLib;
IMPORT ControlLib;
IMPORT StringLib AS Str;

VAR
    (* 控制器变量 *)
    pid_controller : ControlLib.PID_Controller;
    current_temp : REAL;
    target_temp : REAL := 25.0;
    heater_output : REAL;
    
    (* 状态和日志 *)
    system_status : STRING;
    log_message : STRING;
END_VAR

FUNCTION CalculateHeaterPower : INT
VAR_INPUT
    current : REAL;
    target : REAL;
END_VAR
VAR
    diff : REAL;
END_VAR

diff := target - current;
IF diff > 0.0 THEN
    CalculateHeaterPower := REAL_TO_INT(diff * 10.0);
ELSE
    CalculateHeaterPower := 0;
END_IF

END_FUNCTION

(* 初始化PID控制器 *)
pid_controller := ControlLib.InitPID(
    Kp := 2.0,
    Ki := 0.1,
    Kd := 0.5,
    OutputMin := 0.0,
    OutputMax := 100.0
);

(* 主控制循环 *)
current_temp := IOLib.ReadAnalogInput(0) * 100.0;
heater_output := CalculateHeaterPower(current_temp, target_temp);
IOLib.WriteAnalogOutput(0, heater_output / 100.0);

END_PROGRAM
```

### 阶乘计算 (使用递归函数)
```
PROGRAM FactorialExample
VAR
    n : INT;
    result : INT;
END_VAR

FUNCTION Factorial : INT
VAR_INPUT
    num : INT;
END_VAR

IF num <= 1 THEN
    Factorial := 1;
ELSE
    Factorial := num * Factorial(num - 1);
END_IF

END_FUNCTION

n := 5;
result := Factorial(n);  (* result = 120 *)

END_PROGRAM
```

## 第一阶段：词法语法分析 (flex + bison)

### 1.1 Flex词法定义 (st_lexer.l)
```c
%{
/* 词法分析器头文件包含 */
#include "st_parser.tab.h"  // bison生成的头文件
#include "ast.h"            // AST节点定义
#include "memory_mgr.h"     // 静态内存管理

/* 全局变量声明 */
extern int yylineno;
extern int yycolumn;
%}

/* 状态定义 */
%x COMMENT

/* 正则表达式定义 */
DIGIT       [0-9]
LETTER      [a-zA-Z_]
IDENTIFIER  {LETTER}({LETTER}|{DIGIT})*
INTEGER     [+-]{DIGIT}+
REAL        [+-]{DIGIT}*\.{DIGIT}+([eE][+-]?{DIGIT}+)?
STRING      \"([^\"\\]|\\.)*\"

%%
/* 关键字识别 - 完整的ST关键字 */
"PROGRAM"       { return PROGRAM; }
"END_PROGRAM"   { return END_PROGRAM; }
"VAR"           { return VAR; }
"END_VAR"       { return END_VAR; }
"VAR_INPUT"     { return VAR_INPUT; }
"VAR_OUTPUT"    { return VAR_OUTPUT; }
"VAR_IN_OUT"    { return VAR_IN_OUT; }
"FUNCTION"      { return FUNCTION; }
"END_FUNCTION"  { return END_FUNCTION; }
"FUNCTION_BLOCK" { return FUNCTION_BLOCK; }
"END_FUNCTION_BLOCK" { return END_FUNCTION_BLOCK; }

/* 控制结构关键字 */
"IF"            { return IF; }
"THEN"          { return THEN; }
"ELSE"          { return ELSE; }
"ELSIF"         { return ELSIF; }
"END_IF"        { return END_IF; }
"CASE"          { return CASE; }
"OF"            { return OF; }
"END_CASE"      { return END_CASE; }
"FOR"           { return FOR; }
"TO"            { return TO; }
"BY"            { return BY; }
"DO"            { return DO; }
"END_FOR"       { return END_FOR; }
"WHILE"         { return WHILE; }
"END_WHILE"     { return END_WHILE; }
"RETURN"        { return RETURN; }

/* 数据类型关键字 */
"BOOL"          { return TYPE_BOOL; }
"INT"           { return TYPE_INT; }
"DINT"          { return TYPE_DINT; }
"REAL"          { return TYPE_REAL; }
"STRING"        { return TYPE_STRING; }
"TIME"          { return TYPE_TIME; }

/* 布尔常量 */
"TRUE"          { yylval.boolean = 1; return BOOL_LITERAL; }
"FALSE"         { yylval.boolean = 0; return BOOL_LITERAL; }

/* 运算符和分隔符 */
":="            { return ASSIGN; }
"="             { return EQ; }
"<>"            { return NE; }
"<="            { return LE; }
">="            { return GE; }
"<"             { return LT; }
">"             { return GT; }
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return MULTIPLY; }
"/"             { return DIVIDE; }
"MOD"           { return MOD; }
"AND"           { return AND; }
"OR"            { return OR; }
"NOT"           { return NOT; }

/* 分隔符 */
"("             { return LPAREN; }
")"             { return RPAREN; }
";"             { return SEMICOLON; }
":"             { return COLON; }
","             { return COMMA; }

/* 字面量处理 - 使用静态内存分配 */
{INTEGER}       { 
    yylval.integer = atoi(yytext); 
    return INTEGER_LITERAL; 
}
{REAL}          { 
    yylval.real = atof(yytext); 
    return REAL_LITERAL; 
}
{STRING}        { 
    yylval.string = allocate_string(yytext); 
    if (yylval.string == NULL) {
        return ERROR_TOKEN;
    }
    return STRING_LITERAL; 
}
{IDENTIFIER}    { 
    yylval.string = allocate_string(yytext);
    if (yylval.string == NULL) {
        return ERROR_TOKEN;
    }
    return IDENTIFIER; 
}

/* 空白字符处理 */
[ \t]+          { yycolumn += yyleng; }
\n              { yylineno++; yycolumn = 1; }

/* 注释处理 */
"(*"            { BEGIN(COMMENT); }
<COMMENT>"*)"   { BEGIN(INITIAL); }
<COMMENT>\n     { yylineno++; yycolumn = 1; }
<COMMENT>.      { /* 忽略注释内容 */ }

/* 错误处理 */
.               { 
    return ERROR_TOKEN; 
}

%%

int yywrap(void) {
    return 1;
}
```

### 5.2 库导入语法扩展 (st_parser.y)
```yacc
%{
/* 语法分析器头文件 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symbol_table.h"
#include "memory_mgr.h"

extern int yylex(void);
extern int yylineno;
extern int yycolumn;

void yyerror(const char *s);

/* 全局变量 */
static ast_node_t *g_ast_root = NULL;
%}

/* 联合体定义，用于存储不同类型的语法元素 */
%union {
    int integer;
    double real;
    int boolean;
    char *string;
    struct ast_node *node;
    struct type_info *type;
}

/* 终结符定义 - 完整的token定义 */
%token <integer> INTEGER_LITERAL
%token <real> REAL_LITERAL
%token <boolean> BOOL_LITERAL
%token <string> STRING_LITERAL IDENTIFIER

/* 关键字token */
%token PROGRAM END_PROGRAM VAR END_VAR VAR_INPUT VAR_OUTPUT VAR_IN_OUT
%token FUNCTION END_FUNCTION FUNCTION_BLOCK END_FUNCTION_BLOCK
%token IF THEN ELSE ELSIF END_IF CASE OF END_CASE
%token FOR TO BY DO END_FOR WHILE END_WHILE RETURN

/* 数据类型token */
%token TYPE_BOOL TYPE_INT TYPE_DINT TYPE_REAL TYPE_STRING TYPE_TIME

/* 运算符token */
%token ASSIGN EQ NE LE GE LT GT
%token PLUS MINUS MULTIPLY DIVIDE MOD AND OR NOT

/* 分隔符token */
%token LPAREN RPAREN SEMICOLON COLON COMMA ERROR_TOKEN

/* 新增的库管理相关token */
%token IMPORT FROM AS LIBRARY END_LIBRARY EXPORT VERSION

/* 非终结符类型定义 */
%type <node> program declaration_list declaration var_declaration function_declaration
%type <node> statement_list statement assignment_statement
%type <node> if_statement for_statement while_statement
%type <node> expression additive_expr multiplicative_expr unary_expr primary_expr
%type <type> type_specifier
%type <node> import_list import_declaration library_declaration

/* 运算符优先级和结合性 */
%right ASSIGN
%left OR
%left AND
%left EQ NE
%left LT LE GT GE
%left PLUS MINUS
%left MULTIPLY DIVIDE MOD
%right NOT UMINUS
%left LPAREN RPAREN

%%

/* 程序入口 */
program: 
    import_list PROGRAM IDENTIFIER 
    declaration_list 
    statement_list 
    END_PROGRAM {
        $$ = create_program_with_imports($1, $3, $4, $5);
        g_ast_root = $$;
    }
    | PROGRAM IDENTIFIER 
      declaration_list 
      statement_list 
      END_PROGRAM {
        $$ = create_program_node($2, $3, $4);
        g_ast_root = $$;
    }
    ;

/* 导入列表 */
import_list:
    import_declaration {
        $$ = $1;
    }
    | import_list import_declaration {
        $$ = append_import($1, $2);
    }
    ;

/* 导入声明 */
import_declaration:
    IMPORT IDENTIFIER SEMICOLON {
        $$ = create_import_node($2, NULL, NULL);
    }
    | IMPORT IDENTIFIER AS IDENTIFIER SEMICOLON {
        $$ = create_import_node($2, $4, NULL);
    }
    | IMPORT IDENTIFIER FROM STRING_LITERAL SEMICOLON {
        $$ = create_import_node($2, NULL, $4);
    }
    ;

/* 库声明（用于创建库文件） */
library_declaration:
    LIBRARY IDENTIFIER VERSION STRING_LITERAL
    declaration_list
    END_LIBRARY {
        $$ = create_library_node($2, $4, $5);
    }
    ;

/* 声明列表 */
declaration_list: 
    /* empty */ { $$ = NULL; }
    | declaration_list declaration {
        $$ = append_declaration($1, $2);
    }
    ;

/* 声明 */
declaration: 
    var_declaration { $$ = $1; }
    | function_declaration { $$ = $1; }
    ;

/* 变量声明 - 支持多种变量类型 */
var_declaration:
    VAR var_list END_VAR {
        $$ = create_var_declaration_node(VAR_LOCAL, $2);
    }
    | VAR_INPUT var_list END_VAR {
        $$ = create_var_declaration_node(VAR_INPUT_TYPE, $2);
    }
    | VAR_OUTPUT var_list END_VAR {
        $$ = create_var_declaration_node(VAR_OUTPUT_TYPE, $2);
    }
    ;

var_list: 
    var_item {
        $$ = $1;
    }
    | var_list var_item {
        $$ = append_var_item($1, $2);
    }
    ;

var_item: 
    IDENTIFIER COLON type_specifier SEMICOLON {
        $$ = create_var_item_node($1, $3, NULL);
        /* 添加到符号表 */
        add_symbol($1, $3, get_current_scope());
    }
    | IDENTIFIER COLON type_specifier ASSIGN expression SEMICOLON {
        $$ = create_var_item_node($1, $3, $5);
        /* 添加到符号表 */
        add_symbol($1, $3, get_current_scope());
    }
    ;

/* 函数声明 */
function_declaration:
    FUNCTION IDENTIFIER COLON type_specifier
    declaration_list
    statement_list
    END_FUNCTION {
        $$ = create_function_node($2, NULL, $4, $5, $6);
    }
    ;

/* 类型说明符 */
type_specifier: 
    TYPE_BOOL { $$ = create_basic_type(TYPE_BOOL_ID); }
    | TYPE_INT { $$ = create_basic_type(TYPE_INT_ID); }
    | TYPE_DINT { $$ = create_basic_type(TYPE_DINT_ID); }
    | TYPE_REAL { $$ = create_basic_type(TYPE_REAL_ID); }
    | TYPE_STRING { $$ = create_basic_type(TYPE_STRING_ID); }
    ;

/* 语句列表 */
statement_list: 
    /* empty */ { $$ = NULL; }
    | statement_list statement {
        $$ = append_statement($1, $2);
    }
    ;

/* 语句类型 */
statement: 
    assignment_statement { $$ = $1; }
    | if_statement { $$ = $1; }
    | for_statement { $$ = $1; }
    | while_statement { $$ = $1; }
    | RETURN expression SEMICOLON { $$ = create_return_statement($2); }
    | RETURN SEMICOLON { $$ = create_return_statement(NULL); }
    ;

/* 赋值语句 */
assignment_statement: 
    IDENTIFIER ASSIGN expression SEMICOLON {
        $$ = create_assignment_node($1, $3);
    }
    ;

/* 条件语句 */
if_statement: 
    IF expression THEN statement_list END_IF {
        $$ = create_if_node($2, $4, NULL);
    }
    | IF expression THEN statement_list ELSE statement_list END_IF {
        $$ = create_if_node($2, $4, $6);
    }
    ;

/* FOR循环 */
for_statement: 
    FOR IDENTIFIER ASSIGN expression TO expression DO statement_list END_FOR {
        $$ = create_for_node($2, $4, $6, NULL, $8);
    }
    ;

/* WHILE循环 */
while_statement: 
    WHILE expression DO statement_list END_WHILE {
        $$ = create_while_node($2, $4);
    }
    ;

/* 表达式处理 */
expression: 
    additive_expr { $$ = $1; }
    | expression EQ additive_expr {
        $$ = create_binary_op_node(OP_EQ, $1, $3);
    }
    | expression NE additive_expr {
        $$ = create_binary_op_node(OP_NE, $1, $3);
    }
    | expression LT additive_expr {
        $$ = create_binary_op_node(OP_LT, $1, $3);
    }
    | expression LE additive_expr {
        $$ = create_binary_op_node(OP_LE, $1, $3);
    }
    | expression GT additive_expr {
        $$ = create_binary_op_node(OP_GT, $1, $3);
    }
    | expression GE additive_expr {
        $$ = create_binary_op_node(OP_GE, $1, $3);
    }
    | expression AND additive_expr {
        $$ = create_binary_op_node(OP_AND, $1, $3);
    }
    | expression OR additive_expr {
        $$ = create_binary_op_node(OP_OR, $1, $3);
    }
    ;

additive_expr:
    multiplicative_expr { $$ = $1; }
    | additive_expr PLUS multiplicative_expr {
        $$ = create_binary_op_node(OP_ADD, $1, $3);
    }
    | additive_expr MINUS multiplicative_expr {
        $$ = create_binary_op_node(OP_SUB, $1, $3);
    }
    ;

multiplicative_expr:
    unary_expr { $$ = $1; }
    | multiplicative_expr MULTIPLY unary_expr {
        $$ = create_binary_op_node(OP_MUL, $1, $3);
    }
    | multiplicative_expr DIVIDE unary_expr {
        $$ = create_binary_op_node(OP_DIV, $1, $3);
    }
    | multiplicative_expr MOD unary_expr {
        $$ = create_binary_op_node(OP_MOD, $1, $3);
    }
    ;

unary_expr:
    primary_expr { $$ = $1; }
    | NOT unary_expr {
        $$ = create_unary_op_node(OP_NOT, $2);
    }
    | MINUS unary_expr %prec UMINUS {
        $$ = create_unary_op_node(OP_NEG, $2);
    }
    ;

primary_expr:
    INTEGER_LITERAL {
        $$ = create_literal_node(LITERAL_INT, &$1);
    }
    | REAL_LITERAL {
        $$ = create_literal_node(LITERAL_REAL, &$1);
    }
    | BOOL_LITERAL {
        $$ = create_literal_node(LITERAL_BOOL, &$1);
    }
    | STRING_LITERAL {
        $$ = create_literal_node(LITERAL_STRING, $1);
    }
    | IDENTIFIER {
        $$ = create_identifier_node($1);
    }
    | LPAREN expression RPAREN {
        $$ = $2;
    }
    ;

%%

/* 错误处理函数实现 */
void yyerror(const char *s) {
    fprintf(stderr, "语法错误 (行 %d): %s\n", yylineno, s);
}

/* 获取AST根节点 */
ast_node_t* get_ast_root(void) {
    return g_ast_root;
}
```

## 第二阶段：AST与符号表管理

### 2.1 AST节点定义 (ast.h)
```c
#ifndef AST_H
#define AST_H

#include <stdint.h>
#include <stdbool.h>

/* AST节点类型枚举 */
typedef enum {
    /* 顶层节点 */
    AST_PROGRAM,        // 程序根节点
    
    /* 声明节点 */
    AST_VAR_DECL,       // 变量声明
    AST_VAR_ITEM,       // 变量项
    AST_FUNCTION_DECL,  // 函数声明
    
    /* 语句节点 */
    AST_ASSIGNMENT,     // 赋值语句
    AST_IF_STMT,        // 条件语句
    AST_FOR_STMT,       // FOR循环
    AST_WHILE_STMT,     // WHILE循环
    AST_RETURN_STMT,    // RETURN语句
    
    /* 表达式节点 */
    AST_BINARY_OP,      // 二元操作
    AST_UNARY_OP,       // 一元操作
    AST_LITERAL,        // 字面量
    AST_IDENTIFIER,     // 标识符
    
    /* 列表节点 */
    AST_DECLARATION_LIST, // 声明列表
    AST_STATEMENT_LIST    // 语句列表
} ast_node_type_t;

/* 操作符类型 */
typedef enum {
    /* 算术运算 */
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    /* 逻辑运算 */
    OP_AND, OP_OR, OP_NOT,
    /* 比较运算 */
    OP_EQ, OP_NE, OP_LT, OP_LE, OP_GT, OP_GE,
    /* 一元运算 */
    OP_NEG
} operator_type_t;

/* 字面量类型 */
typedef enum { 
    LITERAL_INT, 
    LITERAL_REAL, 
    LITERAL_STRING, 
    LITERAL_BOOL 
} literal_type_t;

/* 变量类型 */
typedef enum {
    VAR_LOCAL,          // 局部变量
    VAR_INPUT_TYPE,     // 输入变量
    VAR_OUTPUT_TYPE     // 输出变量
} var_category_t;

/* 源码位置信息 */
typedef struct source_location {
    int line;           // 行号
    int column;         // 列号
    char *filename;     // 文件名
} source_location_t;

/* 前向声明 */
struct type_info;

/* AST节点基础结构 */
typedef struct ast_node {
    ast_node_type_t type;           // 节点类型
    source_location_t *loc;         // 源码位置信息
    struct type_info *data_type;    // 数据类型信息
    
    union {
        /* 程序节点 */
        struct {
            char *name;                    // 程序名
            struct ast_node *declarations; // 声明列表
            struct ast_node *statements;   // 语句列表
        } program;
        
        /* 变量声明节点 */
        struct {
            var_category_t category;       // 变量类别
            struct ast_node *var_list;     // 变量列表
        } var_decl;
        
        /* 变量项节点 */
        struct {
            char *name;                    // 变量名
            struct type_info *type;        // 类型
            struct ast_node *init_value;   // 初始值
        } var_item;
        
        /* 函数声明节点 */
        struct {
            char *name;                    // 函数名
            struct type_info *return_type; // 返回类型
            struct ast_node *declarations; // 局部声明
            struct ast_node *statements;   // 语句列表
        } function_decl;
        
        /* 赋值语句节点 */
        struct {
            char *target;                  // 赋值目标
            struct ast_node *value;        // 赋值值
        } assignment;
        
        /* 条件语句节点 */
        struct {
            struct ast_node *condition;    // 条件表达式
            struct ast_node *then_stmt;    // THEN语句
            struct ast_node *else_stmt;    // ELSE语句
        } if_stmt;
        
        /* FOR循环节点 */
        struct {
            char *var_name;                // 循环变量
            struct ast_node *start_value;  // 起始值
            struct ast_node *end_value;    // 结束值
            struct ast_node *by_value;     // 步长值
            struct ast_node *statements;   // 循环体
        } for_stmt;
        
        /* WHILE循环节点 */
        struct {
            struct ast_node *condition;    // 循环条件
            struct ast_node *statements;   // 循环体
        } while_stmt;
        
        /* RETURN语句节点 */
        struct {
            struct ast_node *return_value; // 返回值
        } return_stmt;
        
        /* 二元操作节点 */
        struct {
            operator_type_t op;            // 操作符
            struct ast_node *left;         // 左操作数  
            struct ast_node *right;        // 右操作数
        } binary_op;
        
        /* 一元操作节点 */
        struct {
            operator_type_t op;            // 操作符
            struct ast_node *operand;      // 操作数
        } unary_op;
        
        /* 字面量节点 */
        struct {
            literal_type_t literal_type;   // 字面量类型
            union {
                int int_val;               // 整数值
                double real_val;           // 实数值
                char *string_val;          // 字符串值
                bool bool_val;             // 布尔值
            } value;
        } literal;
        
        /* 标识符节点 */
        struct {
            char *name;                    // 标识符名
        } identifier;
        
    } data;
    
    struct ast_node *next;          // 链表下一个节点
} ast_node_t;

/* AST节点创建函数声明 */
ast_node_t* create_program_node(const char *name, ast_node_t *declarations, 
                               ast_node_t *statements);
ast_node_t* create_var_declaration_node(var_category_t category, ast_node_t *var_list);
ast_node_t* create_var_item_node(const char *name, struct type_info *type, 
                                 ast_node_t *init_value);
ast_node_t* create_function_node(const char *name, ast_node_t *parameters,
                                struct type_info *return_type, ast_node_t *declarations,
                                ast_node_t *statements);
ast_node_t* create_assignment_node(const char *target, ast_node_t *value);
ast_node_t* create_if_node(ast_node_t *condition, ast_node_t *then_stmt,
                          ast_node_t *else_stmt);
ast_node_t* create_for_node(const char *var_name, ast_node_t *start_value,
                           ast_node_t *end_value, ast_node_t *by_value,
                           ast_node_t *statements);
ast_node_t* create_while_node(ast_node_t *condition, ast_node_t *statements);
ast_node_t* create_return_statement(ast_node_t *return_value);
ast_node_t* create_binary_op_node(operator_type_t op, ast_node_t *left, ast_node_t *right);
ast_node_t* create_unary_op_node(operator_type_t op, ast_node_t *operand);
ast_node_t* create_literal_node(literal_type_t type, const void *value);
ast_node_t* create_identifier_node(const char *name);

/* AST操作函数 */
ast_node_t* append_declaration(ast_node_t *list, ast_node_t *node);
ast_node_t* append_statement(ast_node_t *list, ast_node_t *node);
ast_node_t* append_var_item(ast_node_t *list, ast_node_t *node);

/* 全局AST根节点访问 */
ast_node_t* get_ast_root(void);

#endif /* AST_H */
```

### 2.2 符号表管理 (symbol_table.h)
```c
#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdint.h>
#include <stdbool.h>

/* 符号类型 */
typedef enum {
    SYMBOL_VAR,         // 变量
    SYMBOL_FUNCTION     // 函数
} symbol_type_t;

/* 数据类型信息 */
typedef struct type_info {
    enum {
        TYPE_BOOL_ID, TYPE_INT_ID, TYPE_DINT_ID, TYPE_REAL_ID, 
        TYPE_STRING_ID, TYPE_TIME_ID
    } base_type;
    
    uint32_t size;          // 类型大小（字节）
    bool is_constant;       // 是否常量类型
} type_info_t;

/* 符号表项 */
typedef struct symbol {
    char *name;                 // 符号名称
    symbol_type_t symbol_type;  // 符号类型
    type_info_t *data_type;     // 数据类型
    
    uint32_t scope_level;       // 作用域级别
    uint32_t address;           // 内存地址或偏移
    
    /* 符号属性标志 */
    bool is_global;             // 是否为全局符号
    bool is_initialized;        // 是否已初始化
    
    struct symbol *next;        // 链表下一项
} symbol_t;

/* 作用域管理 - 简化版 */
typedef struct scope {
    uint32_t level;             // 作用域级别
    symbol_t *symbols;          // 符号链表
    struct scope *parent;       // 父作用域
    uint32_t symbol_count;      // 符号数量
} scope_t;

/* 符号表管理接口 */
int init_symbol_table(void);
void cleanup_symbol_table(void);

/* 作用域管理 */
scope_t* enter_scope(void);
scope_t* exit_scope(void);
scope_t* get_current_scope(void);

/* 符号操作 */
symbol_t* add_symbol(const char *name, type_info_t *data_type, uint32_t scope_level);
symbol_t* lookup_symbol(const char *name);

/* 类型操作 */
type_info_t* create_basic_type(int base_type);

#endif /* SYMBOL_TABLE_H */
```

## 第三阶段：字节码设计与生成

### 3.1 字节码指令集定义
```c
#ifndef BYTECODE_H
#define BYTECODE_H

#include <stdint.h>

/* 虚拟机指令操作码 - 简化指令集 */
typedef enum {
    /* 数据移动指令 */
    OP_NOP = 0x00,              // 空操作
    OP_LOAD_CONST_INT,          // 加载整数常量
    OP_LOAD_CONST_REAL,         // 加载实数常量
    OP_LOAD_CONST_BOOL,         // 加载布尔常量
    
    OP_LOAD_LOCAL,              // 加载局部变量
    OP_LOAD_GLOBAL,             // 加载全局变量
    OP_STORE_LOCAL,             // 存储到局部变量
    OP_STORE_GLOBAL,            // 存储到全局变量
    
    /* 算术运算指令 */
    OP_ADD_INT,                 // 整数加法
    OP_SUB_INT,                 // 整数减法
    OP_MUL_INT,                 // 整数乘法
    OP_DIV_INT,                 // 整数除法
    OP_MOD_INT,                 // 整数取模
    OP_NEG_INT,                 // 整数取负
    
    OP_ADD_REAL,                // 实数加法
    OP_SUB_REAL,                // 实数减法
    OP_MUL_REAL,                // 实数乘法
    OP_DIV_REAL,                // 实数除法
    OP_NEG_REAL,                // 实数取负
    
    /* 逻辑运算指令 */
    OP_AND_BOOL,                // 逻辑与
    OP_OR_BOOL,                 // 逻辑或
    OP_NOT_BOOL,                // 逻辑非
    
    /* 比较指令 */
    OP_EQ_INT,                  // 整数相等
    OP_NE_INT,                  // 整数不等
    OP_LT_INT,                  // 整数小于
    OP_LE_INT,                  // 整数小于等于
    OP_GT_INT,                  // 整数大于
    OP_GE_INT,                  // 整数大于等于
    
    OP_EQ_REAL,                 // 实数相等
    OP_NE_REAL,                 // 实数不等
    OP_LT_REAL,                 // 实数小于
    OP_LE_REAL,                 // 实数小于等于
    OP_GT_REAL,                 // 实数大于
    OP_GE_REAL,                 // 实数大于等于
    
    /* 控制流指令 */
    OP_JMP,                     // 无条件跳转
    OP_JMP_TRUE,                // 真值跳转
    OP_JMP_FALSE,               // 假值跳转
    
    /* 函数调用指令 */
    OP_CALL,                    // 函数调用
    OP_RET,                     // 函数返回
    
    /* 虚拟机控制 */
    OP_HALT                     // 停机指令
    
} opcode_t;

/* 字节码指令结构 */
typedef struct bytecode_instr {
    opcode_t opcode;            // 操作码
    union {
        int32_t int_operand;    // 整数操作数
        double real_operand;    // 实数操作数
        uint32_t addr_operand;  // 地址操作数
    } operand;
} bytecode_instr_t;

/* 字节码文件格式 - 简化版 */
typedef struct bytecode_file {
    struct {
        char magic[4];          // 魔数 "STBC"
        uint32_t version;       // 版本号
        uint32_t instr_count;   // 指令数量
        uint32_t const_pool_size; // 常量池大小
        uint32_t global_var_size; // 全局变量区大小
    } header;
    
    bytecode_instr_t *instructions;     // 指令序列
    void *const_pool;                   // 常量池
} bytecode_file_t;

#endif /* BYTECODE_H */
```

## 第四阶段：栈式虚拟机实现 - 修正版

### 4.1 虚拟机核心结构 - 简化版
```c
#ifndef VM_H
#define VM_H

#include "bytecode.h"
#include <stdint.h>
#include <stdbool.h>

/* 虚拟机运行时值类型 */
typedef enum {
    VAL_BOOL, VAL_INT, VAL_REAL, VAL_STRING
} value_type_t;

/* 虚拟机运行时值 */
typedef struct vm_value {
    value_type_t type;
    union {
        bool bool_val;
        int32_t int_val;
        double real_val;
        char *string_val;
    } data;
} vm_value_t;

/* 虚拟机执行栈 - 静态分配 */
#define MAX_STACK_SIZE 1000
typedef struct vm_stack {
    vm_value_t data[MAX_STACK_SIZE];    // 静态栈数据
    int32_t top;                        // 栈顶指针
} vm_stack_t;

/* 函数调用帧 - 静态分配 */
#define MAX_CALL_FRAMES 100
typedef struct call_frame {
    uint32_t return_address;    // 返回地址
    int32_t local_var_base;     // 局部变量基地址
} call_frame_t;

/* 虚拟机实例 - 静态内存分配 */
#define MAX_GLOBAL_VARS 500
typedef struct st_vm {
    // 指令和数据
    bytecode_instr_t *instructions;    // 指令序列
    uint32_t instr_count;              // 指令数量
    void *const_pool;                  // 常量池
    
    // 运行时状态
    uint32_t pc;                       // 程序计数器
    vm_stack_t operand_stack;          // 操作数栈（静态）
    call_frame_t call_stack[MAX_CALL_FRAMES]; // 调用栈（静态）
    int32_t call_stack_top;            // 调用栈顶
    
    // 内存区域 - 静态分配
    vm_value_t global_vars[MAX_GLOBAL_VARS]; // 全局变量区（静态）
    uint32_t global_var_count;         // 全局变量数量
    
    // 执行控制
    enum {
        VM_RUNNING,     // 正在运行
        VM_PAUSED,      // 暂停（调试）
        VM_STOPPED,     // 停止
        VM_ERROR        // 错误状态
    } state;
    
    /* 主从同步支持 */
    master_slave_sync_t *sync_mgr;     // 同步管理器
    bool sync_enabled;                 // 是否启用同步
    
} st_vm_t;

/* 虚拟机接口函数 */
int vm_init(st_vm_t *vm);
int vm_load_bytecode(st_vm_t *vm, const char *filename);
int vm_execute(st_vm_t *vm);
void vm_cleanup(st_vm_t *vm);

/* 栈操作函数 */
int stack_push(vm_stack_t *stack, const vm_value_t *value);
vm_value_t* stack_pop(vm_stack_t *stack);
vm_value_t* stack_top(vm_stack_t *stack);

/* 同步相关接口 */
int vm_enable_sync(st_vm_t *vm, const sync_config_t *config, node_role_t role);
int vm_disable_sync(st_vm_t *vm);
int vm_sync_variable(st_vm_t *vm, uint32_t var_index);
int vm_process_sync_messages(st_vm_t *vm);

#endif /* VM_H */
```

## 架构说明

### 编译流程
1. **词法分析** (st_lexer.l) - 将源代码转换为标记流
2. **语法分析** (st_parser.y) - 解析标记并构建AST
3. **语义分析** - 基础类型检查和符号表管理
4. **代码生成** - 将AST编译为虚拟机字节码
5. **执行** - 在虚拟机中执行字节码

### 文件结构
```
/
├── st_lexer.l               # 词法规则定义（支持库导入）
├── st_parser.y              # 语法规则定义（支持库导入）
├── ast.h/ast.c              # 抽象语法树（支持导入节点）
├── symbol_table.h/c         # 符号表管理（支持库符号）
├── library_manager.h/c      # 库管理器
├── builtin_math.c           # 内置数学库
├── builtin_string.c         # 内置字符串库
├── builtin_io.c             # 内置I/O库
├── master_slave_sync.h/c    # 主从同步系统
├── memory_mgr.h/c           # 静态内存管理
├── bytecode.h/c             # 字节码定义
├── codegen.h/c              # 代码生成器（支持库调用）
├── vm.h/vm.c                # 虚拟机实现（支持同步）
├── main.c                   # 主程序
├── Makefile                 # 构建脚本
└── CLAUDE.md                # 技术文档
```

### 虚拟机指令集
- **数据操作**：LOAD_CONST_INT, LOAD_CONST_REAL, LOAD_LOCAL, STORE_LOCAL
- **算术运算**：ADD_INT, SUB_INT, MUL_INT, DIV_INT, ADD_REAL, SUB_REAL
- **比较运算**：EQ_INT, NE_INT, LT_INT, EQ_REAL, NE_REAL, LT_REAL
- **逻辑运算**：AND_BOOL, OR_BOOL, NOT_BOOL
- **控制流**：JMP, JMP_TRUE, JMP_FALSE
- **函数调用**：CALL, RET
- **系统**：HALT, NOP

## 单人开发模式实施计划（AI辅助代码生成）

### 开发模式定位

#### 人机协作分工
- **开发者负责**：
  - 基础ST语言语法规则定义（核心子集）
  - 简化的指令集设计
  - 基本架构决策和测试用例设计
  - 集成测试和功能验证

- **Copilot负责**：
  - 根据规范生成词法/语法分析器代码
  - AST和符号表的标准实现
  - 静态内存管理代码生成
  - 虚拟机执行引擎实现
  - 基础测试框架代码

### 进度计划（10周）

#### 第1-2周：核心前端实现
- 你：定义基础ST语法子集（变量声明、赋值、IF、FOR、WHILE、函数）
- 我：生成flex/bison文件、AST结构、基础符号表

#### 第3-4周：语义分析与内存管理
- 你：设计静态内存分配策略、基础类型系统
- 我：生成静态内存管理器、类型检查器、作用域管理

#### 第5-6周：字节码系统与库管理
- 你：设计库导入语法、内置函数接口、库文件格式
- 我：生成库管理器、内置数学/字符串/I/O库、库符号解析器

#### 第7-8周：虚拟机与主从同步
- 你：设计主从同步协议、角色切换机制、网络通信策略
- 我：生成主从同步管理器、UDP通信模块、状态同步逻辑

#### 第9-10周：集成测试与优化
- 你：设计完整测试场景（包含库调用和主从切换）
- 我：生成集成测试框架、性能监控、示例程序

### 新增功能优势

#### 函数库导入系统
- **内置库支持**：数学、字符串、I/O函数即插即用
- **静态链接**：编译时解析，运行时零开销
- **符号命名空间**：支持库别名，避免符号冲突
- **版本管理**：简单的版本检查机制

#### 主从数据同步系统
- **实时同步**：全局变量修改立即同步到从机
- **故障切换**：主机故障时从机自动接管
- **健康监控**：心跳检测和连接状态管理
- **数据一致性**：校验和保证数据传输正确性