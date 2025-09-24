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
ST源码 → 词法分析(Flex) → 语法分析(Bison) → AST → 字节码生成 → 字节码文件
                                                                    ↓
外部I/O ← 双机热备同步 ← 调试接口 ← 栈式虚拟机 ← 字节码加载器 ← 字节码文件
```

### 技术栈选择
- **词法分析**: GNU Flex (词法生成器)
- **语法分析**: GNU Bison (LALR解析器生成器)  
- **字节码格式**: 自定义二进制格式
- **虚拟机**: 栈式架构 + 局部变量表
- **调试协议**: 基于TCP/IP的调试代理
- **同步机制**: 共享内存 + 消息队列

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
%}

/* 正则表达式定义 */
DIGIT       [0-9]
LETTER      [a-zA-Z_]
IDENTIFIER  {LETTER}({LETTER}|{DIGIT})*
INTEGER     [+-]{DIGIT}+
REAL        [+-]{DIGIT}*\.{DIGIT}+([eE][+-]?{DIGIT}+)?
STRING      \"([^\"\\]|\\.)*\"

%%
/* 关键字识别 */
"VAR"           { return VAR; }
"END_VAR"       { return END_VAR; }
"IF"            { return IF; }
"THEN"          { return THEN; }
"ELSE"          { return ELSE; }
"END_IF"        { return END_IF; }
"WHILE"         { return WHILE; }
"DO"            { return DO; }
"END_WHILE"     { return END_WHILE; }
"FOR"           { return FOR; }
"TO"            { return TO; }
"BY"            { return BY; }
"END_FOR"       { return END_FOR; }
"FUNCTION"      { return FUNCTION; }
"END_FUNCTION"  { return END_FUNCTION; }

/* 数据类型关键字 */
"BOOL"          { return TYPE_BOOL; }
"INT"           { return TYPE_INT; }
"DINT"          { return TYPE_DINT; }
"REAL"          { return TYPE_REAL; }
"STRING"        { return TYPE_STRING; }
"TIME"          { return TYPE_TIME; }

/* 运算符和分隔符 */
":="            { return ASSIGN; }
"="             { return EQ; }
"<>"            { return NE; }
"<="            { return LE; }
">="            { return GE; }
"<"             { return LT; }
">"             { return GT; }
"AND"           { return AND; }
"OR"            { return OR; }
"NOT"           { return NOT; }

/* 字面量处理 */
{INTEGER}       { yylval.integer = atoi(yytext); return INTEGER_LITERAL; }
{REAL}          { yylval.real = atof(yytext); return REAL_LITERAL; }
{STRING}        { yylval.string = strdup(yytext); return STRING_LITERAL; }
{IDENTIFIER}    { yylval.string = strdup(yytext); return IDENTIFIER; }

/* 忽略空白字符 */
[ \t\n]+        { /* 忽略 */ }

/* 注释处理 */
"(*"            { /* 多行注释开始 */ BEGIN(COMMENT); }
<COMMENT>"*)"   { BEGIN(INITIAL); }
<COMMENT>.|\n   { /* 注释内容忽略 */ }
"//".*          { /* 单行注释忽略 */ }

%%
```

### 1.2 Bison语法定义 (st_parser.y)
```yacc
%{
/* 语法分析器头文件 */
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "symbol_table.h"

extern int yylex(void);
void yyerror(const char *s);
%}

/* 联合体定义，用于存储不同类型的语法元素 */
%union {
    int integer;
    double real;
    char *string;
    struct ast_node *node;
    struct type_info *type;
    struct symbol *symbol;
}

/* 终结符定义 */
%token <integer> INTEGER_LITERAL
%token <real> REAL_LITERAL
%token <string> STRING_LITERAL IDENTIFIER
%token VAR END_VAR IF THEN ELSE END_IF WHILE DO END_WHILE
%token FOR TO BY END_FOR FUNCTION END_FUNCTION
%token TYPE_BOOL TYPE_INT TYPE_DINT TYPE_REAL TYPE_STRING TYPE_TIME
%token ASSIGN EQ NE LE GE LT GT AND OR NOT

/* 非终结符类型定义 */
%type <node> program declaration_list declaration var_declaration
%type <node> statement_list statement assignment_statement
%type <node> if_statement while_statement for_statement
%type <node> expression term factor function_call
%type <type> type_specifier

/* 运算符优先级和结合性 */
%left OR
%left AND  
%left EQ NE LT LE GT GE
%left '+' '-'
%left '*' '/' 
%right NOT UMINUS
%left '(' ')'

%%

/* 程序入口 */
program: declaration_list statement_list {
    $$ = create_program_node($1, $2);
    set_ast_root($$);
}

/* 声明列表 */
declaration_list: /* empty */ { $$ = NULL; }
    | declaration_list declaration {
        $$ = append_declaration($1, $2);
    }

/* 变量声明 */
declaration: var_declaration { $$ = $1; }

var_declaration: VAR var_list END_VAR {
    $$ = create_var_declaration_node($2);
}

var_list: var_item { $$ = $1; }
    | var_list var_item {
        $$ = append_var_item($1, $2);
    }

var_item: IDENTIFIER ':' type_specifier ';' {
    $$ = create_var_item_node($1, $3);
    // 添加到符号表
    add_symbol($1, $3, current_scope);
}

/* 类型说明符 */
type_specifier: TYPE_BOOL { $$ = create_bool_type(); }
    | TYPE_INT { $$ = create_int_type(); }
    | TYPE_DINT { $$ = create_dint_type(); }
    | TYPE_REAL { $$ = create_real_type(); }
    | TYPE_STRING { $$ = create_string_type(); }

/* 语句列表 */
statement_list: /* empty */ { $$ = NULL; }
    | statement_list statement {
        $$ = append_statement($1, $2);
    }

/* 语句类型 */
statement: assignment_statement { $$ = $1; }
    | if_statement { $$ = $1; }
    | while_statement { $$ = $1; }
    | for_statement { $$ = $1; }

/* 赋值语句 */
assignment_statement: IDENTIFIER ASSIGN expression ';' {
    $$ = create_assignment_node($1, $3);
}

/* 条件语句 */
if_statement: IF expression THEN statement_list END_IF {
    $$ = create_if_node($2, $4, NULL);
}
    | IF expression THEN statement_list ELSE statement_list END_IF {
        $$ = create_if_node($2, $4, $6);
    }

/* 循环语句 */
while_statement: WHILE expression DO statement_list END_WHILE {
    $$ = create_while_node($2, $4);
}

/* 表达式处理 */
expression: term { $$ = $1; }
    | expression '+' expression {
        $$ = create_binary_op_node(OP_ADD, $1, $3);
    }
    | expression '-' expression {
        $$ = create_binary_op_node(OP_SUB, $1, $3);
    }
    | expression '*' expression {
        $$ = create_binary_op_node(OP_MUL, $1, $3);
    }
    | expression '/' expression {
        $$ = create_binary_op_node(OP_DIV, $1, $3);
    }
    | expression AND expression {
        $$ = create_binary_op_node(OP_AND, $1, $3);
    }
    | expression OR expression {
        $$ = create_binary_op_node(OP_OR, $1, $3);
    }
    | expression EQ expression {
        $$ = create_binary_op_node(OP_EQ, $1, $3);
    }
    | NOT expression {
        $$ = create_unary_op_node(OP_NOT, $2);
    }

term: factor { $$ = $1; }

factor: INTEGER_LITERAL {
    $$ = create_literal_node(LITERAL_INT, &$1);
}
    | REAL_LITERAL {
        $$ = create_literal_node(LITERAL_REAL, &$1);
    }
    | STRING_LITERAL {
        $$ = create_literal_node(LITERAL_STRING, $1);
    }
    | IDENTIFIER {
        $$ = create_identifier_node($1);
    }
    | '(' expression ')' {
        $$ = $2;
    }

%%

void yyerror(const char *s) {
    fprintf(stderr, "语法错误: %s\n", s);
}
```

## 第二阶段：AST与符号表管理

### 2.1 AST节点定义 (ast.h)
```c
/* AST节点类型枚举 */
typedef enum {
    AST_PROGRAM,        // 程序根节点
    AST_VAR_DECL,       // 变量声明
    AST_ASSIGNMENT,     // 赋值语句
    AST_IF_STMT,        // 条件语句
    AST_WHILE_STMT,     // 循环语句
    AST_FOR_STMT,       // for循环
    AST_BINARY_OP,      // 二元操作
    AST_UNARY_OP,       // 一元操作
    AST_LITERAL,        // 字面量
    AST_IDENTIFIER      // 标识符
} ast_node_type_t;

/* 操作符类型 */
typedef enum {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV,
    OP_AND, OP_OR, OP_NOT,
    OP_EQ, OP_NE, OP_LT, OP_LE, OP_GT, OP_GE
} operator_type_t;

/* AST节点基础结构 */
typedef struct ast_node {
    ast_node_type_t type;           // 节点类型
    struct source_location *loc;    // 源码位置信息
    struct type_info *data_type;    // 数据类型信息
    
    union {
        struct {                    // 程序节点
            struct ast_node *declarations;
            struct ast_node *statements;
        } program;
        
        struct {                    // 变量声明
            char *name;
            struct type_info *type;
        } var_decl;
        
        struct {                    // 赋值语句
            char *target;
            struct ast_node *value;
        } assignment;
        
        struct {                    // 条件语句
            struct ast_node *condition;
            struct ast_node *then_stmt;
            struct ast_node *else_stmt;
        } if_stmt;
        
        struct {                    // 二元操作
            operator_type_t op;
            struct ast_node *left;
            struct ast_node *right;
        } binary_op;
        
        struct {                    // 字面量
            enum { LITERAL_INT, LITERAL_REAL, LITERAL_STRING, LITERAL_BOOL } literal_type;
            union {
                int int_val;
                double real_val;
                char *string_val;
                int bool_val;
            } value;
        } literal;
        
        struct {                    // 标识符
            char *name;
        } identifier;
    } data;
    
    struct ast_node *next;          // 链表下一个节点
} ast_node_t;
```

### 2.2 符号表管理 (symbol_table.h)
```c
/* 符号类型 */
typedef enum {
    SYMBOL_VAR,         // 变量
    SYMBOL_FUNCTION,    // 函数
    SYMBOL_TYPE         // 类型定义
} symbol_type_t;

/* 数据类型信息 */
typedef struct type_info {
    enum {
        TYPE_BOOL, TYPE_INT, TYPE_DINT, TYPE_REAL, 
        TYPE_STRING, TYPE_TIME, TYPE_ARRAY, TYPE_STRUCT
    } base_type;
    
    int size;           // 类型大小（字节）
    int alignment;      // 内存对齐要求
    
    union {
        struct {        // 数组类型
            struct type_info *element_type;
            int dimensions;
            int *sizes;
        } array;
        
        struct {        // 结构体类型
            int member_count;
            struct {
                char *name;
                struct type_info *type;
                int offset;
            } *members;
        } struct_type;
    } detail;
} type_info_t;

/* 符号表项 */
typedef struct symbol {
    char *name;                 // 符号名称
    symbol_type_t type;         // 符号类型
    type_info_t *data_type;     // 数据类型
    int scope_level;            // 作用域级别
    int address;                // 内存地址或偏移
    int is_global;              // 是否为全局变量
    int is_external;            // 是否为外部变量
    struct symbol *next;        // 链表下一项
} symbol_t;

/* 符号表管理接口 */
symbol_t* add_symbol(const char *name, type_info_t *type, int scope);
symbol_t* lookup_symbol(const char *name);
void enter_scope(void);
void exit_scope(void);
```

## 第三阶段：字节码设计与生成

### 3.1 字节码指令集定义
```c
/* 虚拟机指令操作码 */
typedef enum {
    // 数据移动指令
    OP_LOAD_CONST,      // 加载常量到栈顶
    OP_LOAD_LOCAL,      // 加载局部变量
    OP_LOAD_GLOBAL,     // 加载全局变量
    OP_STORE_LOCAL,     // 存储到局部变量
    OP_STORE_GLOBAL,    // 存储到全局变量
    OP_DUP,            // 复制栈顶元素
    OP_POP,            // 弹出栈顶元素
    
    // 算术运算指令
    OP_ADD_INT,        // 整数加法
    OP_SUB_INT,        // 整数减法
    OP_MUL_INT,        // 整数乘法
    OP_DIV_INT,        // 整数除法
    OP_ADD_REAL,       // 实数加法
    OP_SUB_REAL,       // 实数减法
    OP_MUL_REAL,       // 实数乘法
    OP_DIV_REAL,       // 实数除法
    
    // 逻辑运算指令
    OP_AND,            // 逻辑与
    OP_OR,             // 逻辑或
    OP_NOT,            // 逻辑非
    
    // 比较指令
    OP_EQ_INT,         // 整数相等比较
    OP_NE_INT,         // 整数不等比较
    OP_LT_INT,         // 整数小于比较
    OP_LE_INT,         // 整数小于等于比较
    OP_GT_INT,         // 整数大于比较
    OP_GE_INT,         // 整数大于等于比较
    
    // 控制流指令
    OP_JMP,            // 无条件跳转
    OP_JMP_FALSE,      // 条件跳转（false时跳转）
    OP_JMP_TRUE,       // 条件跳转（true时跳转）
    
    // 函数调用指令
    OP_CALL,           // 函数调用
    OP_RET,            // 函数返回
    
    // 类型转换指令
    OP_INT_TO_REAL,    // 整数转实数
    OP_REAL_TO_INT,    // 实数转整数
    
    // 外部接口指令
    OP_READ_INPUT,     // 读取输入变量
    OP_WRITE_OUTPUT,   // 写入输出变量
    
    // 调试和同步指令
    OP_BREAKPOINT,     // 调试断点
    OP_SYNC_GLOBAL,    // 全局变量同步
    
    // 虚拟机控制
    OP_HALT            // 停机指令
} opcode_t;

/* 字节码指令结构 */
typedef struct bytecode_instr {
    opcode_t opcode;        // 操作码
    union {
        int int_operand;    // 整数操作数
        double real_operand; // 实数操作数
        struct {
            int address;    // 地址操作数
            int size;       // 数据大小
        } addr_operand;
    } operand;
} bytecode_instr_t;

/* 字节码文件格式 */
typedef struct bytecode_file {
    struct {
        char magic[4];          // 魔数 "STBC"
        int version;            // 版本号
        int instr_count;        // 指令数量
        int const_pool_size;    // 常量池大小
        int global_var_size;    // 全局变量区大小
        int debug_info_size;    // 调试信息大小
    } header;
    
    bytecode_instr_t *instructions;     // 指令序列
    void *const_pool;                   // 常量池
    struct debug_info *debug_info;      // 调试信息
} bytecode_file_t;
```

### 3.2 字节码生成器 (codegen.c)
```c
/* 代码生成器上下文 */
typedef struct codegen_context {
    bytecode_instr_t *instructions;     // 指令缓冲区
    int instr_count;                    // 当前指令数量
    int instr_capacity;                 // 指令缓冲区容量
    
    void *const_pool;                   // 常量池
    int const_pool_size;                // 常量池大小
    
    struct label_table *labels;         // 标签表（用于跳转）
    int current_scope;                  // 当前作用域
    
    struct debug_context *debug_ctx;    // 调试信息生成
} codegen_context_t;

/* 生成字节码的主要函数 */
void generate_bytecode(ast_node_t *ast, codegen_context_t *ctx) {
    switch (ast->type) {
        case AST_PROGRAM:
            // 生成程序初始化代码
            generate_program_init(ctx);
            // 递归处理声明和语句
            generate_bytecode(ast->data.program.declarations, ctx);
            generate_bytecode(ast->data.program.statements, ctx);
            // 生成程序结束代码
            emit_instruction(ctx, OP_HALT, 0);
            break;
            
        case AST_ASSIGNMENT:
            // 先计算右值表达式
            generate_bytecode(ast->data.assignment.value, ctx);
            // 存储到目标变量
            symbol_t *sym = lookup_symbol(ast->data.assignment.target);
            if (sym->is_global) {
                emit_instruction(ctx, OP_STORE_GLOBAL, sym->address);
            } else {
                emit_instruction(ctx, OP_STORE_LOCAL, sym->address);
            }
            break;
            
        case AST_BINARY_OP:
            // 生成左操作数
            generate_bytecode(ast->data.binary_op.left, ctx);
            // 生成右操作数  
            generate_bytecode(ast->data.binary_op.right, ctx);
            // 生成相应的运算指令
            switch (ast->data.binary_op.op) {
                case OP_ADD:
                    if (ast->data_type->base_type == TYPE_INT) {
                        emit_instruction(ctx, OP_ADD_INT, 0);
                    } else {
                        emit_instruction(ctx, OP_ADD_REAL, 0);
                    }
                    break;
                // ... 其他运算符处理
            }
            break;
            
        case AST_IF_STMT:
            // 生成条件表达式
            generate_bytecode(ast->data.if_stmt.condition, ctx);
            // 条件跳转到else部分
            int else_label = create_label(ctx);
            emit_jump_instruction(ctx, OP_JMP_FALSE, else_label);
            // 生成then部分
            generate_bytecode(ast->data.if_stmt.then_stmt, ctx);
            // 跳转到结束
            int end_label = create_label(ctx);
            emit_jump_instruction(ctx, OP_JMP, end_label);
            // 放置else标签
            place_label(ctx, else_label);
            if (ast->data.if_stmt.else_stmt) {
                generate_bytecode(ast->data.if_stmt.else_stmt, ctx);
            }
            // 放置结束标签
            place_label(ctx, end_label);
            break;
            
        case AST_LITERAL:
            // 加载字面量常量
            int const_index = add_to_const_pool(ctx, &ast->data.literal);
            emit_instruction(ctx, OP_LOAD_CONST, const_index);
            break;
            
        case AST_IDENTIFIER:
            // 加载变量值
            symbol_t *var_sym = lookup_symbol(ast->data.identifier.name);
            if (var_sym->is_global) {
                emit_instruction(ctx, OP_LOAD_GLOBAL, var_sym->address);
            } else {
                emit_instruction(ctx, OP_LOAD_LOCAL, var_sym->address);
            }
            break;
    }
    
    // 递归处理链表中的下一个节点
    if (ast->next) {
        generate_bytecode(ast->next, ctx);
    }
}

/* 字节码文件序列化 */
int serialize_bytecode(codegen_context_t *ctx, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) return -1;
    
    bytecode_file_t bcfile;
    
    // 设置文件头
    memcpy(bcfile.header.magic, "STBC", 4);
    bcfile.header.version = 1;
    bcfile.header.instr_count = ctx->instr_count;
    bcfile.header.const_pool_size = ctx->const_pool_size;
    bcfile.header.global_var_size = get_global_var_size();
    
    // 写入文件头
    fwrite(&bcfile.header, sizeof(bcfile.header), 1, file);
    
    // 写入指令序列
    fwrite(ctx->instructions, 
           sizeof(bytecode_instr_t), 
           ctx->instr_count, file);
           
    // 写入常量池
    fwrite(ctx->const_pool, 1, ctx->const_pool_size, file);
    
    // 写入调试信息
    serialize_debug_info(ctx->debug_ctx, file);
    
    fclose(file);
    return 0;
}
```

## 第四阶段：栈式虚拟机实现

### 4.1 虚拟机核心结构
```c
/* 虚拟机运行时值类型 */
typedef enum {
    VAL_BOOL, VAL_INT, VAL_DINT, VAL_REAL, VAL_STRING
} value_type_t;

/* 虚拟机运行时值 */
typedef struct vm_value {
    value_type_t type;
    union {
        int bool_val;
        int int_val;
        long long dint_val;
        double real_val;
        char *string_val;
    } data;
} vm_value_t;

/* 虚拟机执行栈 */
typedef struct vm_stack {
    vm_value_t *data;       // 栈数据缓冲区
    int capacity;           // 栈容量
    int top;               // 栈顶指针
} vm_stack_t;

/* 函数调用帧 */
typedef struct call_frame {
    int return_address;     // 返回地址
    int local_var_base;     // 局部变量基地址
    int prev_frame_ptr;     // 上一帧指针
} call_frame_t;

/* 虚拟机实例 */
typedef struct st_vm {
    // 指令和数据
    bytecode_instr_t *instructions;    // 指令序列
    int instr_count;                   // 指令数量
    void *const_pool;                  // 常量池
    
    // 运行时状态
    int pc;                           // 程序计数器
    vm_stack_t *operand_stack;        // 操作数栈
    call_frame_t *call_stack;         // 调用栈
    int call_stack_top;               // 调用栈顶
    
    // 内存区域
    vm_value_t *global_vars;          // 全局变量区
    vm_value_t *local_vars;           // 局部变量区
    int global_var_count;             // 全局变量数量
    int local_var_count;              // 局部变量数量
    
    // 执行控制
    enum {
        VM_RUNNING,     // 正在运行
        VM_PAUSED,      // 暂停（调试）
        VM_STOPPED,     // 停止
        VM_ERROR        // 错误状态
    } state;
    
    // 调试支持
    int *breakpoints;                 // 断点列表
    int breakpoint_count;             // 断点数量
    struct debug_interface *debug_if; // 调试接口
    
    // I/O接口
    struct io_interface *io_if;       // 输入输出接口
    
    // 双机热备
    struct backup_interface *backup_if; // 备份同步接口
    
} st_vm_t;
```

### 4.2 虚拟机执行引擎
```c
/* 虚拟机主执行循环 */
int vm_execute(st_vm_t *vm) {
    while (vm->state == VM_RUNNING && vm->pc < vm->instr_count) {
        
        // 检查断点
        if (is_breakpoint_set(vm, vm->pc)) {
            vm->state = VM_PAUSED;
            notify_debugger(vm->debug_if, vm->pc);
            return VM_BREAKPOINT_HIT;
        }
        
        bytecode_instr_t *instr = &vm->instructions[vm->pc];
        
        switch (instr->opcode) {
            
            case OP_LOAD_CONST: {
                // 从常量池加载常量到栈顶
                vm_value_t *const_val = get_constant(vm, instr->operand.int_operand);
                stack_push(vm->operand_stack, const_val);
                break;
            }
            
            case OP_LOAD_GLOBAL: {
                // 加载全局变量到栈顶
                int addr = instr->operand.int_operand;
                vm_value_t *val = &vm->global_vars[addr];
                stack_push(vm->operand_stack, val);
                break;
            }
            
            case OP_STORE_GLOBAL: {
                // 从栈顶存储到全局变量
                int addr = instr->operand.int_operand;
                vm_value_t *val = stack_pop(vm->operand_stack);
                vm->global_vars[addr] = *val;
                
                // 触发双机同步
                if (vm->backup_if) {
                    sync_global_var(vm->backup_if, addr, val);
                }
                break;
            }
            
            case OP_ADD_INT: {
                // 整数加法
                vm_value_t *b = stack_pop(vm->operand_stack);
                vm_value_t *a = stack_pop(vm->operand_stack);
                vm_value_t result = {
                    .type = VAL_INT,
                    .data.int_val = a->data.int_val + b->data.int_val
                };
                stack_push(vm->operand_stack, &result);
                break;
            }
            
            case OP_ADD_REAL: {
                // 实数加法
                vm_value_t *b = stack_pop(vm->operand_stack);
                vm_value_t *a = stack_pop(vm->operand_stack);
                vm_value_t result = {
                    .type = VAL_REAL,
                    .data.real_val = a->data.real_val + b->data.real_val
                };
                stack_push(vm->operand_stack, &result);
                break;
            }
            
            case OP_JMP: {
                // 无条件跳转
                vm->pc = instr->operand.int_operand - 1; // -1因为循环末尾会+1
                break;
            }
            
            case OP_JMP_FALSE: {
                // 条件跳转（false时跳转）
                vm_value_t *condition = stack_pop(vm->operand_stack);
                if (!condition->data.bool_val) {
                    vm->pc = instr->operand.int_operand - 1;
                }
                break;
            }
            
            case OP_CALL: {
                // 函数调用
                int func_addr = instr->operand.int_operand;
                
                // 创建新的调用帧
                call_frame_t frame;
                frame.return_address = vm->pc + 1;
                frame.local_var_base = vm->local_var_count;
                frame.prev_frame_ptr = vm->call_stack_top;
                
                // 压入调用栈
                vm->call_stack[++vm->call_stack_top] = frame;
                
                // 跳转到函数入口
                vm->pc = func_addr - 1;
                break;
            }
            
            case OP_RET: {
                // 函数返回
                call_frame_t *frame = &vm->call_stack[vm->call_stack_top--];
                vm->pc = frame->return_address - 1;
                vm->local_var_count = frame->local_var_base;
                break;
            }
            
            case OP_READ_INPUT: {
                // 读取输入变量
                int input_addr = instr->operand.int_operand;
                vm_value_t input_val;
                if (vm->io_if && vm->io_if->read_input) {
                    vm->io_if->read_input(input_addr, &input_val);
                    stack_push(vm->operand_stack, &input_val);
                }
                break;
            }
            
            case OP_WRITE_OUTPUT: {
                // 写入输出变量
                int output_addr = instr->operand.int_operand;
                vm_value_t *output_val = stack_pop(vm->operand_stack);
                if (vm->io_if && vm->io_if->write_output) {
                    vm->io_if->write_output(output_addr, output_val);
                }
                break;
            }
            
            case OP_BREAKPOINT: {
                // 调试断点
                vm->state = VM_PAUSED;
                notify_debugger(vm->debug_if, vm->pc);
                return VM_BREAKPOINT_HIT;
            }
            
            case OP_SYNC_GLOBAL: {
                // 全局变量同步
                if (vm->backup_if) {
                    sync_all_globals(vm->backup_if, vm->global_vars, vm->global_var_count);
                }
                break;
            }
            
            case OP_HALT: {
                // 程序停止
                vm->state = VM_STOPPED;
                return VM_EXECUTION_COMPLETE;
            }
            
            default:
                // 未知指令错误
                vm->state = VM_ERROR;
                return VM_INVALID_INSTRUCTION;
        }
        
        vm->pc++; // 移动到下一条指令
        
        // 周期性检查外部事件
        if (vm->pc % 1000 == 0) {
            process_external_events(vm);
        }
    }
    
    return VM_EXECUTION_COMPLETE;
}

/* 单步执行（调试用） */
int vm_step(st_vm_t *vm) {
    if (vm->state != VM_PAUSED && vm->state != VM_RUNNING) {
        return VM_NOT_READY;
    }
    
    vm->state = VM_RUNNING;
    
    // 执行一条指令
    bytecode_instr_t *instr = &vm->instructions[vm->pc];
    
    // 记录执行前状态用于调试
    capture_vm_state_snapshot(vm);
    
    // 执行指令（复用上面的switch逻辑）
    int result = execute_single_instruction(vm, instr);
    
    // 暂停等待调试器控制
    vm->state = VM_PAUSED;
    
    // 通知调试器状态变化
    notify_debugger_step_complete(vm->debug_if, vm);
    
    return result;
}
```

## 第五阶段：调试接口实现

### 5.1 调试协议定义
```c
/* 调试消息类型 */
typedef enum {
    DEBUG_MSG_SET_BREAKPOINT,       // 设置断点
    DEBUG_MSG_CLEAR_BREAKPOINT,     // 清除断点
    DEBUG_MSG_STEP_INTO,            // 单步进入
    DEBUG_MSG_STEP_OVER,            // 单步跳过
    DEBUG_MSG_CONTINUE,             // 继续执行
    DEBUG_MSG_PAUSE,                // 暂停执行
    DEBUG_MSG_GET_VARIABLES,        // 获取变量值
    DEBUG_MSG_SET_VARIABLE,         // 设置变量值
    DEBUG_MSG_GET_CALL_STACK,       // 获取调用栈
    DEBUG_MSG_EVALUATE_EXPRESSION,  // 表达式求值
} debug_msg_type_t;

/* 调试消息结构 */
typedef struct debug_message {
    debug_msg_type_t type;          // 消息类型
    int sequence_id;                // 序列号
    int data_length;                // 数据长度
    char data[];                    // 消息数据
} debug_message_t;

/* 调试接口 */
typedef struct debug_interface {
    int socket_fd;                  // TCP套接字
    pthread_t debug_thread;         // 调试线程
    
    // 回调函数
    void (*on_breakpoint_hit)(st_vm_t *vm, int pc);
    void (*on_step_complete)(st_vm_t *vm);
    void (*on_execution_paused)(st_vm_t *vm);
    
    // 调试状态
    int is_connected;               // 是否连接到调试器
    struct vm_state_snapshot *snapshots; // 状态快照历史
    
} debug_interface_t;

/* 调试服务器主循环 */
void* debug_server_thread(void *arg) {
    st_vm_t *vm = (st_vm_t*)arg;
    debug_interface_t *debug_if = vm->debug_if;
    
    // 监听调试器连接
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(DEBUG_PORT);
    
    bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(listen_fd, 1);
    
    while (1) {
        // 等待调试器连接
        debug_if->socket_fd = accept(listen_fd, NULL, NULL);
        debug_if->is_connected = 1;
        
        printf("调试器已连接\n");
        
        // 处理调试命令
        while (debug_if->is_connected) {
            debug_message_t msg;
            int bytes = recv(debug_if->socket_fd, &msg, sizeof(msg), 0);
            if (bytes <= 0) {
                debug_if->is_connected = 0;
                break;
            }
            
            handle_debug_command(vm, &msg);
        }
        
        close(debug_if->socket_fd);
        printf("调试器已断开连接\n");
    }
    
    return NULL;
}

/* 处理调试命令 */
void handle_debug_command(st_vm_t *vm, debug_message_t *msg) {
    switch (msg->type) {
        case DEBUG_MSG_SET_BREAKPOINT: {
            int pc = *(int*)msg->data;
            set_breakpoint(vm, pc);
            send_debug_response(vm->debug_if, msg->sequence_id, "OK");
            break;
        }
        
        case DEBUG_MSG_STEP_INTO: {
            vm_step(vm);
            break;
        }
        
        case DEBUG_MSG_CONTINUE: {
            vm->state = VM_RUNNING;
            break;
        }
        
        case DEBUG_MSG_GET_VARIABLES: {
            char *var_json = serialize_variables_to_json(vm);
            send_debug_response(vm->debug_if, msg->sequence_id, var_json);
            free(var_json);
            break;
        }
        
        case DEBUG_MSG_SET_VARIABLE: {
            // 解析变量名和新值
            char *var_name = msg->data;
            char *new_value = msg->data + strlen(var_name) + 1;
            set_variable_value(vm, var_name, new_value);
            send_debug_response(vm->debug_if, msg->sequence_id, "OK");
            break;
        }
    }
}
```

## 第六阶段：双机热备接口

### 6.1 热备同步机制
```c
/* 双机热备配置 */
typedef struct backup_config {
    char *partner_ip;               // 伙伴机IP地址
    int sync_port;                  // 同步端口
    int heartbeat_interval;         // 心跳间隔（毫秒）
    int failover_timeout;           // 故障切换超时
    enum {
        BACKUP_ROLE_PRIMARY,        // 主机
        BACKUP_ROLE_SECONDARY       // 备机
    } role;
} backup_config_t;

/* 同步消息类型 */
typedef enum {
    SYNC_MSG_HEARTBEAT,            // 心跳消息
    SYNC_MSG_GLOBAL_VAR_UPDATE,    // 全局变量更新
    SYNC_MSG_STATE_SNAPSHOT,       // 状态快照
    SYNC_MSG_ROLE_SWITCH,          // 角色切换
} sync_msg_type_t;

/* 双机热备接口 */
typedef struct backup_interface {
    backup_config_t config;         // 配置信息
    
    int sync_socket;                // 同步套接字
    pthread_t sync_thread;          // 同步线程
    pthread_t heartbeat_thread;     // 心跳线程
    
    struct {
        int is_partner_alive;       // 伙伴机是否存活
        uint64_t last_heartbeat;    // 最后心跳时间
        int sync_lag;               // 同步延迟
    } status;
    
    // 同步缓冲区
    struct sync_buffer {
        vm_value_t *global_vars_shadow;  // 全局变量影子副本
        int dirty_flags[MAX_GLOBAL_VARS]; // 脏标记数组
        pthread_mutex_t mutex;            // 互斥锁
    } sync_buffer;
    
} backup_interface_t;

/* 全局变量同步函数 */
void sync_global_var(backup_interface_t *backup_if, int var_addr, vm_value_t *value) {
    if (backup_if->config.role != BACKUP_ROLE_PRIMARY) {
        return; // 只有主机才同步变量到备机
    }
    
    pthread_mutex_lock(&backup_if->sync_buffer.mutex);
    
    // 更新影子副本
    backup_if->sync_buffer.global_vars_shadow[var_addr] = *value;
    backup_if->sync_buffer.dirty_flags[var_addr] = 1;
    
    pthread_mutex_unlock(&backup_if->sync_buffer.mutex);
    
    // 立即发送关键变量更新
    if (is_critical_variable(var_addr)) {
        send_var_update_immediately(backup_if, var_addr, value);
    }
}

/* 同步线程主循环 */
void* backup_sync_thread(void *arg) {
    backup_interface_t *backup_if = (backup_interface_t*)arg;
    
    while (1) {
        if (backup_if->config.role == BACKUP_ROLE_PRIMARY) {
            // 主机：发送变量更新
            send_dirty_variables(backup_if);
        } else {
            // 备机：接收变量更新
            receive_variable_updates(backup_if);
        }
        
        usleep(SYNC_INTERVAL_US);
    }
    
    return NULL;
}

/* 发送脏变量 */
void send_dirty_variables(backup_interface_t *backup_if) {
    pthread_mutex_lock(&backup_if->sync_buffer.mutex);
    
    for (int i = 0; i < MAX_GLOBAL_VARS; i++) {
        if (backup_if->sync_buffer.dirty_flags[i]) {
            // 构造同步消息
            struct sync_message {
                sync_msg_type_t type;
                int var_addr;
                vm_value_t value;
                uint64_t timestamp;
            } msg;
            
            msg.type = SYNC_MSG_GLOBAL_VAR_UPDATE;
            msg.var_addr = i;
            msg.value = backup_if->sync_buffer.global_vars_shadow[i];
            msg.timestamp = get_current_timestamp();
            
            // 发送到备机
            send(backup_if->sync_socket, &msg, sizeof(msg), 0);
            
            // 清除脏标记
            backup_if->sync_buffer.dirty_flags[i] = 0;
        }
    }
    
    pthread_mutex_unlock(&backup_if->sync_buffer.mutex);
}

/* 故障切换处理 */
void handle_failover(st_vm_t *vm) {
    backup_interface_t *backup_if = vm->backup_if;
    
    if (backup_if->config.role == BACKUP_ROLE_SECONDARY) {
        printf("检测到主机故障，开始故障切换...\n");
        
        // 切换为主机角色
        backup_if->config.role = BACKUP_ROLE_PRIMARY;
        
        // 恢复虚拟机执行
        if (vm->state == VM_PAUSED) {
            vm->state = VM_RUNNING;
        }
        
        // 通知外部系统角色变化
        notify_role_change(backup_if, BACKUP_ROLE_PRIMARY);
        
        printf("故障切换完成，当前为主机\n");
    }
}
```

## 架构说明

### 编译流程
1. **词法分析** (st_lexer.lex) - 将源代码转换为标记流
2. **语法分析** (st_parser.y) - 解析标记并构建AST
3. **语义分析** - 类型检查、函数签名验证、作用域分析
4. **代码生成** (vm.c) - 将AST编译为虚拟机字节码
5. **执行** (vm.c) - 在虚拟机中执行字节码

### 文件结构
```
/home/jiang/src/lexer/
├── st_lexer.lex         # 词法规则定义
├── st_parser.y          # 语法规则定义
├── ast.h/ast.c          # 抽象语法树
├── symbol_table.h/c     # 符号表管理
├── function.h/c         # 函数定义和调用处理
├── scope.h/c            # 作用域管理
├── library_manager.h/c  # 库管理器
├── import_resolver.h/c  # 导入解析器
├── namespace.h/c        # 命名空间管理
├── vm.h/vm.c           # 虚拟机实现
├── main.c              # 主程序
├── Makefile            # 构建脚本
├── library_config.json # 库配置文件
├── libs/               # 标准库目录
│   ├── standard/       # 标准库
│   │   ├── MathLib.st
│   │   ├── StringLib.st
│   │   ├── TimeLib.st
│   │   └── IOLib.st
│   └── custom/         # 自定义库
│       └── ControlLib.st
└── CLAUDE.md           # 技术文档
```

### 虚拟机指令集
- **数据操作**：LOAD_INT, LOAD_REAL, LOAD_VAR, STORE_VAR
- **算术运算**：ADD, SUB, MUL, DIV, NEG
- **比较运算**：EQ, NE, LT, LE, GT, GE
- **逻辑运算**：AND, OR, NOT
- **控制流**：JMP, JZ, JNZ
- **函数调用**：CALL, RET, PUSH_PARAM, POP_PARAM
- **栈操作**：PUSH_FRAME, POP_FRAME, ALLOC_LOCAL
- **模块操作**：LOAD_LIB, RESOLVE_SYMBOL, CALL_EXTERNAL
- **命名空间**：ENTER_NS, EXIT_NS, RESOLVE_NS
- **系统**：HALT, NOP

### 函数调用机制

#### 调用栈管理
```
调用栈帧结构：
+------------------+
| 返回地址         |
+------------------+
| 上一帧指针       |
+------------------+
| 局部变量1        |
+------------------+
| 局部变量2        |
+------------------+
| ...              |
+------------------+
| 参数N            |
+------------------+
| 参数2            |
+------------------+
| 参数1            |
+------------------+
```

#### 参数传递协议
1. **函数调用前**：将参数按顺序压入操作数栈
2. **CALL指令**：创建新栈帧，设置返回地址
3. **函数执行**：在新栈帧中执行函数体
4. **RET指令**：恢复上一栈帧，返回结果

### 库加载和链接机制

#### 编译时库处理
1. **导入解析**：解析IMPORT语句，确定需要加载的库
2. **依赖检查**：验证库版本兼容性和依赖关系
3. **符号解析**：将库中的符号加入当前命名空间
4. **类型检查**：验证库函数调用的参数类型匹配
5. **代码生成**：生成调用外部库函数的字节码

#### 运行时库加载
1. **动态加载**：运行时按需加载库文件
2. **符号绑定**：将函数调用绑定到实际的库函数地址
3. **内存管理**：管理库的内存分配和释放
4. **错误处理**：处理库加载失败和运行时错误

## 编译构建系统

### Makefile示例
```makefile
# Makefile for ST Language Compiler and VM

CC = gcc
FLEX = flex
BISON = bison
CFLAGS = -Wall -Wextra -std=c99 -O2 -g
LDFLAGS = -lpthread -lm

# 源文件目录
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# 编译器源文件
COMPILER_SRCS = $(SRC_DIR)/st_lexer.l $(SRC_DIR)/st_parser.y \
                $(SRC_DIR)/ast.c $(SRC_DIR)/symbol_table.c \
                $(SRC_DIR)/codegen.c $(SRC_DIR)/main_compiler.c

# 虚拟机源文件  
VM_SRCS = $(SRC_DIR)/vm_core.c $(SRC_DIR)/vm_stack.c \
          $(SRC_DIR)/debug_interface.c $(SRC_DIR)/backup_interface.c \
          $(SRC_DIR)/io_interface.c $(SRC_DIR)/main_vm.c

# 生成的文件
GENERATED_SRCS = $(BUILD_DIR)/st_lexer.c $(BUILD_DIR)/st_parser.tab.c

# 目标程序
COMPILER_TARGET = $(BIN_DIR)/st_compiler
VM_TARGET = $(BIN_DIR)/st_vm

.PHONY: all clean compiler vm test

all: compiler vm

# 编译器目标
compiler: $(COMPILER_TARGET)

$(COMPILER_TARGET): $(GENERATED_SRCS) $(COMPILER_SRCS:.l=.c) $(COMPILER_SRCS:.y=.c)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -I$(BUILD_DIR) $^ -o $@ $(LDFLAGS)

# 虚拟机目标
vm: $(VM_TARGET)

$(VM_TARGET): $(VM_SRCS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) $^ -o $@ $(LDFLAGS)

# 生成词法分析器
$(BUILD_DIR)/st_lexer.c: $(SRC_DIR)/st_lexer.l
	@mkdir -p $(BUILD_DIR)
	$(FLEX) -o $@ $<

# 生成语法分析器
$(BUILD_DIR)/st_parser.tab.c: $(SRC_DIR)/st_parser.y
	@mkdir -p $(BUILD_DIR)
	$(BISON) -d -o $@ $<

# 测试目标
test: all
	@echo "运行测试用例..."
	./test/run_tests.sh

# 清理
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# 安装
install: all
	cp $(COMPILER_TARGET) /usr/local/bin/
	cp $(VM_TARGET) /usr/local/bin/
```

## 深度实现方案与开发计划

### 总体实现策略

#### 1. 分阶段递增式开发
```
阶段1: 最小可用原型 (MVP) - 2周
├── 基础词法语法分析
├── 简单AST构建
├── 基本字节码生成
└── 最简虚拟机执行

阶段2: 核心功能完善 - 3周
├── 完整语法支持
├── 符号表和类型检查
├── 优化字节码指令集
└── 栈式虚拟机优化

阶段3: 高级特性 - 4周
├── 调试接口实现
├── I/O接口框架
├── 错误处理和恢复
└── 基础测试框架

阶段4: 工业级特性 - 6周
├── 双机热备系统
├── 性能优化
├── 完整测试覆盖
└── 文档和工具链
```

#### 2. 核心技术难点分析

**难点1: 词法语法分析的鲁棒性**
```c
/* 问题：ST语言的语法复杂性和错误恢复 */
// 解决方案：分层错误处理机制
typedef struct error_context {
    int error_count;
    int max_errors;
    struct error_info {
        int line, column;
        char *message;
        enum { ERR_LEXICAL, ERR_SYNTAX, ERR_SEMANTIC } type;
    } *errors;
    
    // 错误恢复策略
    enum {
        RECOVERY_PANIC,     // 恐慌模式恢复
        RECOVERY_PHRASE,    // 短语级恢复
        RECOVERY_STATEMENT  // 语句级恢复
    } recovery_mode;
} error_context_t;

/* 实现要点 */
void yyerror_with_recovery(const char *msg) {
    record_error(current_error_ctx, yylineno, yycolumn, msg, ERR_SYNTAX);
    
    // 智能错误恢复：跳过到下一个同步点
    int sync_tokens[] = {';', END_IF, END_WHILE, END_FOR, 0};
    while (yychar != YYEOF) {
        for (int i = 0; sync_tokens[i]; i++) {
            if (yychar == sync_tokens[i]) {
                return; // 找到同步点，继续解析
            }
        }
        yychar = yylex(); // 跳过当前token
    }
}
```

**难点2: 字节码优化和指令选择**
```c
/* 问题：如何生成高效的字节码指令序列 */
// 解决方案：多阶段优化管道
typedef struct optimization_pass {
    char *name;
    int (*optimize)(bytecode_instr_t *instrs, int count);
    int priority;
} optimization_pass_t;

/* 优化过程 */
optimization_pass_t opt_passes[] = {
    {"常量折叠", constant_folding_pass, 100},
    {"死代码消除", dead_code_elimination_pass, 90},
    {"跳转优化", jump_optimization_pass, 80},
    {"指令合并", instruction_combining_pass, 70},
    {NULL, NULL, 0}
};

int apply_optimizations(codegen_context_t *ctx) {
    int total_eliminated = 0;
    
    for (int i = 0; opt_passes[i].name; i++) {
        printf("应用优化: %s\n", opt_passes[i].name);
        int eliminated = opt_passes[i].optimize(ctx->instructions, ctx->instr_count);
        total_eliminated += eliminated;
        
        if (eliminated > 0) {
            printf("  消除了 %d 条指令\n", eliminated);
            ctx->instr_count -= eliminated;
        }
    }
    
    return total_eliminated;
}
```

**难点3: 虚拟机的实时性能优化**
```c
/* 问题：确保虚拟机执行的确定性时间 */
// 解决方案：指令执行时间预测和调度
typedef struct instr_timing {
    opcode_t opcode;
    int min_cycles;     // 最少执行周期
    int max_cycles;     // 最多执行周期
    int avg_cycles;     // 平均执行周期
} instr_timing_t;

/* 执行时间预测表 */
instr_timing_t timing_table[] = {
    {OP_LOAD_CONST, 1, 2, 1},
    {OP_ADD_INT, 1, 1, 1},
    {OP_ADD_REAL, 2, 4, 3},
    {OP_JMP, 1, 1, 1},
    {OP_CALL, 5, 10, 7},
    // ...
};

/* 实时调度器 */
typedef struct rt_scheduler {
    uint64_t cycle_budget;      // 周期预算
    uint64_t cycles_used;       // 已使用周期
    int preemption_point;       // 抢占点
    
    struct {
        int enabled;
        uint64_t deadline;      // 截止时间
        int priority;           // 优先级
    } task_info;
} rt_scheduler_t;

int vm_execute_with_timing(st_vm_t *vm, rt_scheduler_t *scheduler) {
    scheduler->cycles_used = 0;
    
    while (vm->pc < vm->instr_count) {
        bytecode_instr_t *instr = &vm->instructions[vm->pc];
        
        // 预测指令执行时间
        int predicted_cycles = get_instruction_timing(instr->opcode);
        
        // 检查是否超出时间预算
        if (scheduler->cycles_used + predicted_cycles > scheduler->cycle_budget) {
            // 保存执行状态，让出CPU
            save_vm_context(vm);
            return VM_TIME_SLICE_EXPIRED;
        }
        
        // 执行指令
        uint64_t start_time = get_cpu_cycles();
        int result = execute_single_instruction(vm, instr);
        uint64_t end_time = get_cpu_cycles();
        
        scheduler->cycles_used += (end_time - start_time);
        
        if (result != VM_SUCCESS) {
            return result;
        }
        
        vm->pc++;
        
        // 检查抢占点
        if (vm->pc == scheduler->preemption_point) {
            return VM_PREEMPTED;
        }
    }
    
    return VM_EXECUTION_COMPLETE;
}
```

**难点4: 双机热备的一致性保证**
```c
/* 问题：确保主备机状态完全一致 */
// 解决方案：状态机同步 + 操作日志
typedef struct consistency_manager {
    // 状态快照
    struct {
        uint64_t sequence_number;   // 序列号
        uint32_t checksum;          // 校验和
        size_t state_size;          // 状态大小
        void *state_data;           // 状态数据
    } snapshot;
    
    // 操作日志
    struct operation_log {
        uint64_t log_id;            // 日志ID
        enum {
            OP_GLOBAL_VAR_WRITE,    // 全局变量写入
            OP_IO_OUTPUT,           // I/O输出
            OP_FUNCTION_CALL        // 函数调用
        } op_type;
        uint64_t timestamp;         // 时间戳
        size_t data_size;           // 数据大小
        char data[];                // 操作数据
    } *op_log;
    
    int log_capacity;
    int log_count;
    pthread_mutex_t log_mutex;
} consistency_manager_t;

/* 状态一致性检查 */
int verify_consistency(st_vm_t *primary, st_vm_t *backup) {
    // 1. 比较全局变量
    for (int i = 0; i < primary->global_var_count; i++) {
        if (!vm_value_equals(&primary->global_vars[i], &backup->global_vars[i])) {
            log_error("全局变量 %d 不一致", i);
            return INCONSISTENT_GLOBAL_VAR;
        }
    }
    
    // 2. 比较程序计数器
    if (primary->pc != backup->pc) {
        log_error("程序计数器不一致: primary=%d, backup=%d", 
                  primary->pc, backup->pc);
        return INCONSISTENT_PC;
    }
    
    // 3. 比较栈状态
    if (primary->operand_stack->top != backup->operand_stack->top) {
        log_error("栈顶指针不一致");
        return INCONSISTENT_STACK;
    }
    
    return CONSISTENT;
}

/* 增量同步机制 */
void incremental_sync(backup_interface_t *backup_if, 
                     consistency_manager_t *cm) {
    pthread_mutex_lock(&backup_if->sync_buffer.mutex);
    
    // 发送未同步的操作日志
    for (int i = backup_if->last_synced_log; i < cm->log_count; i++) {
        struct operation_log *op = &cm->op_log[i];
        
        struct sync_message msg = {
            .type = SYNC_MSG_OPERATION_LOG,
            .sequence = op->log_id,
            .timestamp = op->timestamp,
            .data_size = sizeof(*op) + op->data_size
        };
        
        // 发送操作日志
        send(backup_if->sync_socket, &msg, sizeof(msg), 0);
        send(backup_if->sync_socket, op, msg.data_size, 0);
    }
    
    backup_if->last_synced_log = cm->log_count;
    
    pthread_mutex_unlock(&backup_if->sync_buffer.mutex);
}
```

### 详细开发计划

#### 第1阶段：MVP实现 (14天)

**Day 1-3: 环境搭建和基础框架**
```bash
# 目标：建立完整的构建环境
├── 配置flex/bison工具链
├── 创建基础目录结构
├── 编写Makefile构建脚本
└── 实现最基本的头文件框架

# 交付物
├── 可编译的空项目框架
├── 基础的错误处理机制
└── 简单的内存管理功能
```

**Day 4-7: 词法语法分析器**
```c
/* 实现重点 */
// 1. ST语言核心语法子集
VAR declarations, basic types (BOOL, INT, REAL)
Assignment statements (identifier := expression)
IF-THEN-ELSE statements
WHILE-DO loops
Basic expressions (+, -, *, /, AND, OR, NOT)

// 2. 错误处理和恢复
typedef struct parse_result {
    ast_node_t *ast;
    error_context_t *errors;
    int success;
} parse_result_t;

parse_result_t* parse_st_program(const char *source) {
    parse_result_t *result = malloc(sizeof(parse_result_t));
    
    // 初始化词法分析器
    yy_scan_string(source);
    
    // 解析并构建AST
    if (yyparse() == 0) {
        result->ast = get_ast_root();
        result->success = 1;
    } else {
        result->success = 0;
    }
    
    result->errors = get_error_context();
    return result;
}
```

**Day 8-10: AST和符号表**
```c
/* 实现重点 */
// 1. AST节点的完整实现
ast_node_t* create_program_node(ast_node_t *decls, ast_node_t *stmts) {
    ast_node_t *node = malloc(sizeof(ast_node_t));
    node->type = AST_PROGRAM;
    node->data.program.declarations = decls;
    node->data.program.statements = stmts;
    
    // 设置源码位置信息
    node->loc = create_source_location(yylineno, yycolumn);
    
    return node;
}

// 2. 作用域管理
typedef struct scope {
    int level;                      // 作用域级别
    symbol_t *symbols;              // 符号链表
    struct scope *parent;           // 父作用域
    struct scope *children;         // 子作用域链表
} scope_t;

scope_t *current_scope = NULL;

void enter_scope(void) {
    scope_t *new_scope = malloc(sizeof(scope_t));
    new_scope->level = current_scope ? current_scope->level + 1 : 0;
    new_scope->symbols = NULL;
    new_scope->parent = current_scope;
    new_scope->children = NULL;
    
    current_scope = new_scope;
}
```

**Day 11-14: 基础字节码生成和虚拟机**
```c
/* 实现重点 */
// 1. 最小指令集 (15个核心指令)
OP_LOAD_CONST, OP_LOAD_GLOBAL, OP_STORE_GLOBAL,
OP_ADD_INT, OP_SUB_INT, OP_MUL_INT, OP_DIV_INT,
OP_AND, OP_OR, OP_NOT,
OP_EQ_INT, OP_LT_INT,
OP_JMP, OP_JMP_FALSE, OP_HALT

// 2. 简单虚拟机执行器
int vm_execute_simple(st_vm_t *vm) {
    while (vm->pc < vm->instr_count) {
        bytecode_instr_t *instr = &vm->instructions[vm->pc];
        
        switch (instr->opcode) {
            case OP_LOAD_CONST:
                stack_push_int(vm->operand_stack, instr->operand.int_operand);
                break;
                
            case OP_ADD_INT: {
                int b = stack_pop_int(vm->operand_stack);
                int a = stack_pop_int(vm->operand_stack);
                stack_push_int(vm->operand_stack, a + b);
                break;
            }
            
            case OP_HALT:
                return VM_SUCCESS;
                
            default:
                return VM_INVALID_INSTRUCTION;
        }
        
        vm->pc++;
    }
    
    return VM_SUCCESS;
}
```

#### 第2阶段：核心功能完善 (21天)

**Day 15-21: 完整语法支持**
```c
/* 扩展语法规则 */
// 1. FOR循环语句
for_statement: FOR IDENTIFIER ASSIGN expression TO expression DO 
               statement_list END_FOR {
    $$ = create_for_node($2, $4, $6, NULL, $8);
}
    | FOR IDENTIFIER ASSIGN expression TO expression BY expression DO 
  statement_list END_FOR {
    $$ = create_for_node($2, $4, $6, $8, $10);
}

// 2. 函数声明和调用
function_declaration: FUNCTION IDENTIFIER '(' parameter_list ')' 
                     ':' type_specifier declaration_list statement_list 
                     END_FUNCTION {
    $$ = create_function_node($2, $4, $7, $8, $9);
}

// 3. 数组和结构体支持
type_specifier: TYPE_ARRAY '[' INTEGER_LITERAL TO INTEGER_LITERAL ']' 
                OF type_specifier {
    $$ = create_array_type($8, $3, $5);
}
    | TYPE_STRUCT '{' member_declaration_list '}' {
    $$ = create_struct_type($2, $4);
}

/* 成员声明列表 */
member_declaration_list: /* empty */ { $$ = NULL; }
    | member_declaration_list member_declaration {
        $$ = append_member_declaration($1, $2);
    }

/* 成员声明 */
member_declaration: IDENTIFIER ':' type_specifier ';' {
    $$ = create_member_declaration_node($1, $3);
}
```

**Day 22-28: 类型系统和语义分析**
```c
/* 实现重点 */
// 1. 类型推导和检查
type_info_t* infer_expression_type(ast_node_t *expr) {
    switch (expr->type) {
        case AST_BINARY_OP:
            return infer_binary_op_type(expr);
        case AST_LITERAL:
            return get_literal_type(expr);
        case AST_IDENTIFIER:
            return lookup_variable_type(expr->data.identifier.name);
        default:
            return NULL;
    }
}

// 2. 隐式类型转换
int can_convert_type(type_info_t *from, type_info_t *to) {
    // INT -> REAL 允许
    if (from->base_type == TYPE_INT && to->base_type == TYPE_REAL) {
        return 1;
    }
    
    // BOOL -> INT 允许
    if (from->base_type == TYPE_BOOL && to->base_type == TYPE_INT) {
        return 1;
    }
    
    // 相同类型
    if (from->base_type == to->base_type) {
        return 1;
    }
    
    return 0;
}

// 3. 函数和过程的参数验证
void validate_function_call(ast_node_t *func_call) {
    // 查找函数符号
    symbol_t *func_symbol = lookup_symbol(func_call->data.function_call.name);
    
    if (func_symbol && func_symbol->type == SYMBOL_FUNCTION) {
        // 检查参数数量
        if (func_call->data.function_call.args_count != func_symbol->data.function.param_count) {
            report_error("参数数量不匹配");
        }
        
        // 检查每个参数类型
        for (int i = 0; i < func_call->data.function_call.args_count; i++) {
            ast_node_t *arg = func_call->data.function_call.args[i];
            type_info_t *expected_type = func_symbol->data.function.param_types[i];
            type_info_t *actual_type = infer_expression_type(arg);
            
            if (!can_convert_type(actual_type, expected_type)) {
                report_error("参数类型不匹配");
            }
        }
    }
}
```

**Day 29-35: 字节码优化
```c
/* 优化算法实现 */
// 1. 常量折叠
int constant_folding_pass(bytecode_instr_t *instrs, int count) {
    int eliminated = 0;
    
    for (int i = 0; i < count - 2; i++) {
        // 检查模式: LOAD_CONST a, LOAD_CONST b, ADD_INT
        if (instrs[i].opcode == OP_LOAD_CONST &&
            instrs[i+1].opcode == OP_LOAD_CONST &&
            instrs[i+2].opcode == OP_ADD_INT) {
            
            // 计算结果
            int result = instrs[i].operand.int_operand + 
                        instrs[i+1].operand.int_operand;
            
            // 替换为单个LOAD_CONST指令
            instrs[i].operand.int_operand = result;
            
            // 标记删除后两条指令
            instrs[i+1].opcode = OP_NOP;
            instrs[i+2].opcode = OP_NOP;
            
            eliminated += 2;
        }
    }
    
    return eliminated;
}

// 2. 死代码消除
int dead_code_elimination_pass(bytecode_instr_t *instrs, int count) {
    int *reachable = calloc(count, sizeof(int));
    
    // 标记从入口点可达的指令
    mark_reachable_instructions(instrs, count, 0, reachable);
    
    // 删除不可达指令
    int eliminated = 0;
    for (int i = 0; i < count; i++) {
        if (!reachable[i]) {
            instrs[i].opcode = OP_NOP;
            eliminated++;
        }
    }
    
    free(reachable);
    return eliminated;
}

// 3. 跳转优化
int jump_optimization_pass(bytecode_instr_t *instrs, int count) {
    int eliminated = 0;
    
    for (int i = 0; i < count - 1; i++) {
        // 检查模式: JMP a, JMP b
        if (instrs[i].opcode == OP_JMP &&
            instrs[i+1].opcode == OP_JMP) {
            
            // 合并为单个JMP指令
            instrs[i].operand.int_operand = instrs[i+1].operand.int_operand;
            
            // 标记删除后一条指令
            instrs[i+1].opcode = OP_NOP;
            eliminated++;
        }
    }
    
    return eliminated;
}

// 4. 指令合并
int instruction_combining_pass(bytecode_instr_t *instrs, int count) {
    int eliminated = 0;
    
    for (int i = 0; i < count - 1; i++) {
        // 检查模式: LOAD_CONST a; LOAD_CONST b; ADD_INT
        if (instrs[i].opcode == OP_LOAD_CONST &&
            instrs[i+1].opcode == OP_LOAD_CONST &&
            instrs[i+2].opcode == OP_ADD_INT) {
            
            // 合并为单个LOAD_CONST指令
            instrs[i].operand.int_operand += instrs[i+1].operand.int_operand;
            
            // 标记删除后两条指令
            instrs[i+1].opcode = OP_NOP;
            instrs[i+2].opcode = OP_NOP;
            
            eliminated += 2;
        }
    }
    
    return eliminated;
}
```

#### 第3阶段：高级特性 (28天)

**Day 36-49: 调试接口**
```c
/* 调试协议实现 */
// 1. 调试信息生成
typedef struct debug_info {
    struct source_map {
        int pc;                     // 字节码地址
        int source_line;            // 源码行号
        int source_column;          // 源码列号
        char *source_file;          // 源文件名
    } *source_maps;
    int source_map_count;
    
    struct variable_info {
        char *name;                 // 变量名
        int address;                // 内存地址
        type_info_t *type;          // 变量类型
        int scope_start_pc;         // 作用域开始PC
        int scope_end_pc;           // 作用域结束PC
    } *variables;
    int variable_count;
} debug_info_t;

// 2. 断点管理
typedef struct breakpoint {
    int id;                         // 断点ID
    int pc;                         // 断点位置
    enum {
        BP_CODE,                    // 代码断点
        BP_DATA,                    // 数据断点
        BP_CONDITIONAL              // 条件断点
    } type;
    
    // 条件断点的条件表达式
    char *condition;
    
    int hit_count;                  // 命中次数
    int enabled;                    // 是否启用
} breakpoint_t;
```

**Day 50-56: I/O接口框架**
```c
/* I/O驱动抽象层 */
// 1. 驱动接口标准化
typedef struct io_driver_interface {
    char *driver_name;              // 驱动名称
    char *version;                  // 版本信息
    
    // 初始化和清理
    int (*init)(struct io_config *config);
    void (*cleanup)(void);
    
    // 数字I/O操作
    int (*read_digital_input)(int channel);
    int (*write_digital_output)(int channel, int value);
    
    // 模拟I/O操作
    double (*read_analog_input)(int channel);
    int (*write_analog_output)(int channel, double value);
    
    // 状态查询
    int (*get_channel_status)(int channel);
    int (*get_driver_status)(void);
    
    // 错误处理
    char* (*get_last_error)(void);
    
} io_driver_interface_t;

// 2. 插件式驱动加载
typedef struct driver_registry {
    io_driver_interface_t **drivers;
    int driver_count;
    int driver_capacity;
    
    // 动态库句柄
    void **driver_handles;
} driver_registry_t;

int load_io_driver(driver_registry_t *registry, const char *driver_path) {
    void *handle = dlopen(driver_path, RTLD_LAZY);
    if (!handle) {
        log_error("无法加载驱动: %s", dlerror());
        return -1;
    }
    
    // 获取驱动接口函数
    io_driver_interface_t* (*get_driver_interface)(void);
    get_driver_interface = dlsym(handle, "get_driver_interface");
    
    if (!get_driver_interface) {
        log_error("驱动接口函数未找到");
        dlclose(handle);
        return -1;
    }
    
    // 注册驱动
    io_driver_interface_t *driver = get_driver_interface();
    register_driver(registry, driver, handle);
    
    return 0;
}
```

**Day 57-63: 错误处理和恢复**
```c
/* 异常处理机制 */
// 1. 异常类型定义
typedef enum {
    EXC_NONE,                       // 无异常
    EXC_DIVIDE_BY_ZERO,            // 除零错误
    EXC_ARRAY_OUT_OF_BOUNDS,       // 数组越界
    EXC_STACK_OVERFLOW,            // 栈溢出
    EXC_STACK_UNDERFLOW,           // 栈下溢
    EXC_TYPE_MISMATCH,             // 类型不匹配
    EXC_UNINITIALIZED_VARIABLE,    // 未初始化变量
    EXC_IO_ERROR,                  // I/O错误
    EXC_TIMEOUT,                   // 超时
    EXC_MEMORY_ERROR               // 内存错误
} exception_type_t;

// 2. 异常处理栈
typedef struct exception_handler {
    exception_type_t exception_mask; // 处理的异常类型掩码
    int handler_pc;                  // 异常处理程序地址
    int stack_level;                 // 栈级别
    struct exception_handler *next;  // 下一个处理器
} exception_handler_t;

/* 异常抛出和捕获 */
void throw_exception(st_vm_t *vm, exception_type_t exc_type, const char *message) {
    // 查找合适的异常处理器
    exception_handler_t *handler = find_exception_handler(vm, exc_type);
    
    if (handler) {
        // 恢复栈状态
        restore_stack_level(vm, handler->stack_level);
        
        // 跳转到异常处理程序
        vm->pc = handler->handler_pc;
        
        // 设置异常信息
        set_exception_info(vm, exc_type, message);
    } else {
        // 未处理异常，停止虚拟机
        vm->state = VM_ERROR;
        log_fatal("未处理异常: %s", message);
    }
}
```

#### 第4阶段：工业级特性 (42天)

**Day 64-84: 双机热备系统**
```c
/* 关键实现 */
// 1. 分布式状态同步
typedef struct distributed_state {
    // 全局状态版本号
    uint64_t global_version;
    
    // 各节点状态
    struct node_state {
        char node_id[32];           // 节点ID
        enum {
            NODE_PRIMARY,           // 主节点
            NODE_SECONDARY,         // 备节点
            NODE_OBSERVER          // 观察者节点
        } role;
        
        uint64_t last_heartbeat;    // 最后心跳时间
        uint64_t state_version;     // 状态版本
        int is_healthy;             // 健康状态
    } nodes[MAX_NODES];
    
    int node_count;
    int primary_node_index;
    
    // 共识算法状态
    struct consensus_state {
        uint64_t term;              // 任期
        int voted_for;              // 投票给谁
        int commit_index;           // 提交索引
        int last_applied;           // 最后应用索引
    } consensus;
    
} distributed_state_t;

// 2. Raft共识算法实现
int raft_request_vote(distributed_state_t *state, int candidate_id, 
                     uint64_t candidate_term, uint64_t last_log_index) {
    // 检查任期
    if (candidate_term < state->consensus.term) {
        return VOTE_REJECTED;  // 任期过旧
    }
    
    // 更新任期
    if (candidate_term > state->consensus.term) {
        state->consensus.term = candidate_term;
        state->consensus.voted_for = -1;  // 重置投票
    }
    
    // 检查是否已投票
    if (state->consensus.voted_for != -1 && 
        state->consensus.voted_for != candidate_id) {
        return VOTE_REJECTED;  // 已投给其他候选人
    }
    
    // 检查日志新旧程度
    if (last_log_index < get_last_log_index(state)) {
        return VOTE_REJECTED;  // 候选人日志过旧
    }
    
    // 投票给候选人
    state->consensus.voted_for = candidate_id;
    return VOTE_GRANTED;
}

// 3. 状态机复制
typedef struct state_machine_command {
    uint64_t command_id;            // 命令ID
    enum {
        CMD_SET_VARIABLE,           // 设置变量
        CMD_CALL_FUNCTION,          // 调用函数
        CMD_IO_OPERATION           // I/O操作
    } command_type;
    
    size_t data_size;               // 数据大小
    char data[];                    // 命令数据
} state_machine_command_t;

int apply_state_machine_command(st_vm_t *vm, state_machine_command_t *cmd) {
    switch (cmd->command_type) {
        case CMD_SET_VARIABLE: {
            struct var_set_data *data = (struct var_set_data*)cmd->data;
            vm_value_t *var = &vm->global_vars[data->var_index];
            *var = data->new_value;
            
            // 记录到操作日志
            log_operation(vm, "SET_VAR", data->var_index, &data->new_value);
            break;
        }
        
        case CMD_CALL_FUNCTION: {
            struct func_call_data *data = (struct func_call_data*)cmd->data;
            
            // 保存当前状态
            save_vm_state(vm);
            
            // 调用函数
            int result = call_function(vm, data->func_addr, data->args, data->arg_count);
            
            if (result != VM_SUCCESS) {
                // 恢复状态
                restore_vm_state(vm);
                return result;
            }
            
            break;
        }
        
        case CMD_IO_OPERATION: {
            struct io_op_data *data = (struct io_op_data*)cmd->data;
            
            if (data->is_output) {
                write_output_variable(vm->io_if, data->addr, &data->value);
            } else {
                // 输入操作通常不需要复制到备机
                log_warning("收到输入操作命令，可能有错误");
            }
            
            break;
        }
    }
    
    return VM_SUCCESS;
}
```

**Day 85-98: 性能优化**
```c
/* 性能优化重点 */
// 1. 指令级并行优化
typedef struct instruction_pipeline {
    struct pipeline_stage {
        char *stage_name;
        int (*process)(bytecode_instr_t *instr, void *context);
        int latency;                // 延迟周期数
    } stages[MAX_PIPELINE_STAGES];
    
    int stage_count;
    
    // 流水线缓冲区
    struct pipeline_buffer {
        bytecode_instr_t *instructions[PIPELINE_DEPTH];
        int stage_mask[PIPELINE_DEPTH];  // 每个指令所在的阶段掩码
    } buffer;
    
} instruction_pipeline_t;

// 2. 内存访问优化
typedef struct memory_optimizer {
    // 缓存友好的数据布局
    struct {
        vm_value_t *hot_variables;      // 热点变量
        vm_value_t *cold_variables;     // 冷变量
        int hot_count;
        int cold_count;
    } variable_layout;
    
    // 内存预取
    struct {
        int prefetch_distance;          // 预取距离
        int *access_pattern;            // 访问模式
        int pattern_length;
    } prefetch_info;
    
    // 内存池
    struct memory_pool {
        void *pool_start;
        size_t pool_size;
        size_t used_size;
        struct free_block {
            size_t size;
            struct free_block *next;
        } *free_list;
    } mem_pool;
    
    
} memory_optimizer_t;

// 3. JIT编译器雏形
typedef struct jit_compiler {
    // 热点检测
    struct {
        int *instruction_counters;      // 指令执行计数器
        int *basic_block_counters;      // 基本块计数器
        int hot_threshold;              // 热点阈值
    } profiler;
    
    // 机器码生成
    struct {
        unsigned char *code_buffer;     // 机器码缓冲区
        size_t buffer_size;
        size_t code_size;
        
        // 寄存器分配
        struct {
            int *virtual_to_physical;   // 虚拟寄存器到物理寄存器映射
            int *register_usage;        // 寄存器使用情况
        } reg_alloc;
    } codegen;
    
} jit_compiler_t;

// 热点代码检测
int detect_hot_spots(jit_compiler_t *jit, st_vm_t *vm) {
    int hot_spots_found = 0;
    
    for (int i = 0; i < vm->instr_count; i++) {
        if (jit->profiler.instruction_counters[i] > jit->profiler.hot_threshold) {
            // 发现热点指令
            mark_for_compilation(jit, i);
            hot_spots_found++;
        }
    }
    
    return hot_spots_found;
}
```

**Day 99-105: 完整测试覆盖**
```c
/* 测试框架实现 */
// 1. 单元测试框架
typedef struct test_case {
    char *test_name;                    // 测试名称
    int (*test_function)(void);         // 测试函数
    char *description;                  // 测试描述
    enum {
        TEST_PENDING,                   // 待执行
        TEST_PASSED,                    // 通过
        TEST_FAILED,                    // 失败
        TEST_SKIPPED                    // 跳过
    } status;
    char *failure_message;              // 失败消息
} test_case_t;

// 2. 自动化测试工具
typedef struct test_runner {
    test_case_t *test_cases;
    int test_count;
    int passed_count;
    int failed_count;
    int skipped_count;
    
    // 测试配置
    struct {
        int verbose_output;             // 详细输出
        int stop_on_failure;           // 失败时停止
        char *test_filter;             // 测试过滤器
        int timeout_seconds;           // 超时时间
    } config;
    
    // 性能统计
    struct {
        double total_time;             // 总执行时间
        double average_time;           // 平均执行时间
        double max_time;               // 最大执行时间
        double min_time;               // 最小执行时间
    } perf_stats;
    
} test_runner_t;

// 3. 覆盖率统计
typedef struct coverage_analyzer {
    // 代码覆盖率
    struct {
        int *line_hit_count;           // 行覆盖计数
        int total_lines;               // 总行数
        int covered_lines;             // 已覆盖行数
        double line_coverage_percent;   // 行覆盖率百分比
    } line_coverage;
    
    // 分支覆盖率
    struct {
        int *branch_hit_count;         // 分支覆盖计数
        int total_branches;            // 总分支数
        int covered_branches;          // 已覆盖分支数
        double branch_coverage_percent; // 分支覆盖率百分比
    } branch_coverage;
    
    // 函数覆盖率
    struct {
        int *function_hit_count;       // 函数覆盖计数
        int total_functions;           // 总函数数
        int covered_functions;         // 已覆盖函数数
        double function_coverage_percent; // 函数覆盖率百分比
    } function_coverage;
    
} coverage_analyzer_t;
```

### 关键成功因素
```
├── 代码审查流程
│   ├── 每个模块必须经过同行审查
│   ├── 使用静态分析工具(cppcheck, valgrind)
│   └── 遵循编码规范和注释标准
├── 持续集成
│   ├── 自动构建和测试
│   ├── 性能回归检测
│   └── 内存泄漏检测
└── 渐进式部署
    ├── 内部测试验证
    ├── 小规模试点应用
    └── 全面生产部署
```


## 单人开发模式实施计划（AI辅助代码生成）

### 开发模式定位

#### 人机协作分工
- **你（开发者）负责**：
  - 语法和词法规则的完整定义（ST语言所有特性）
  - 指令集设计与接口规范
  - 关键架构决策、测试场景设计、性能调优策略
  - 集成测试、最终验收和上线部署

- **我（Copilot）负责**：
  - 按照你的规范自动生成标准化代码实现
  - 数据结构、函数接口、错误处理、内存管理等代码
  - 自动生成文档、注释、构建脚本、测试框架
  - 按需生成辅助工具和样例代码

### 进度计划（10周）

#### 第1-2周：完整词法/语法/AST/符号表
- 你：编写完整ST语言flex/bison规则，AST节点类型、符号表接口、类型系统设计
- 我：生成词法分析器、语法分析器、AST结构体和管理代码、符号表实现、类型系统实现

#### 第3-4周：语义分析与类型检查
- 你：确定类型检查算法的时间复杂度上限、符号表查找策略、作用域深度限制
- 我：生成确定性的语义分析器、固定时间复杂度的类型推导、静态符号表管理

#### 第5-6周：字节码系统与优化
- 你：设计字节码指令集的安全约束、优化pass的执行时间上限、代码生成的内存使用模式
- 我：生成确定性的代码生成器、时间有界的优化算法、安全的字节码序列化

#### 第7-8周：虚拟机安全实现
- 你：定义虚拟机的执行时间配额、栈溢出检测、异常处理机制
- 我：生成带安全检查的执行引擎、栈边界保护、确定性的指令分发器

#### 第9周：调试支持与I/O接口
- 你：设计调试协议、断点管理、变量监视、I/O驱动接口规范
- 我：生成调试服务器、断点管理器、变量检查器、I/O驱动框架

#### 第10周：双机热备与集成测试
- 你：设计一致性算法、状态同步策略、故障检测机制、测试场景
- 我：生成热备同步引擎、故障切换器、一致性验证器、自动化测试框架

### 工作流说明

1. 你先制定规范（接口、数据结构、语法规则等），并在CLAUDE.md或相关头文件中明确
2. 我根据你的规范自动生成实现代码、测试代码、文档和工具
3. 你进行代码审查、集成和测试，反馈优化建议
4. 我根据反馈持续辅助生成和完善代码

### 安全质量保证

#### 静态分析要求
```bash
# MISRA C 2012 检查工具
- PC-lint Plus (商业)
- Polyspace Bug Finder (商业)
- Cppcheck (开源，部分MISRA规则)
- 自定义脚本检查特定规则

# 内存安全检查
- 栈使用分析
- 静态内存池使用统计
- 死代码检测
- 未使用变量检测
```

#### 运行时验证
```c
/* 内存完整性检查 */
static bool verify_memory_integrity(void) {
    uint32_t calculated_checksum = 0U;
    const uint8_t* memory_ptr = (const uint8_t*)&g_memory_manager;
    size_t memory_size = sizeof(static_memory_manager_t) - sizeof(uint32_t);
    
    /* 计算除checksum字段外的所有内存校验和 */
    for (size_t i = 0U; i < memory_size; i++) {
        calculated_checksum += memory_ptr[i];
    }
    
    return (calculated_checksum == g_memory_manager.memory_checksum);
}

/* 栈溢出检查 */
static bool check_stack_overflow(const safe_vm_t* const vm) {
    return (vm->stack_top < MAX_STACK_DEPTH) && 
           (vm->call_stack_top < MAX_CALL_FRAMES);
}
```

这个重新设计的架构完全满足安全级软件的要求：
1. **内存确定性**：所有内存在编译时静态分配，零动态分配
2. **MISRA合规**：遵循MISRA C 2012关键规则
3. **可预测性**：所有算法都有明确的时间和空间复杂度上限
4. **故障安全**：完善的错误检查和恢复机制
5. **可验证性**：支持静态分析和形式化验证

## 扩展功能

未来可扩展的功能：
- 标准库函数（数学、字符串、时间等）
- 数组和结构体数据类型
- 指针和引用类型
- 异常处理机制
- 模块化和命名空间
- 调试和单步执行功能
- 优化编译器后端

## 错误处理

编译器提供详细的错误信息：
- 词法错误：未识别的字符或标记
- 语法错误：不符合语法规则的构造
- 语义错误：类型不匹配、未定义变量、函数签名错误等
- 运行时错误：栈溢出、除零、函数调用错误等

## 许可证

MIT License - 详见LICENSE文件