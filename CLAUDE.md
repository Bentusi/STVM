# IEC 61131-3 ST语言编译器与虚拟机完整实现架构

一个完整的IEC61131结构化文本语言编译器和虚拟机实现，使用Flex和Bison构建，支持平台无关的字节码执行。采用模块化设计，职责清晰分离，支持库管理、主从同步、调试等高级功能。

## 特性

- **完整的词法分析**：支持IEC61131标准的关键字、标识符、字面量和运算符
- **语法分析**：使用Bison实现的语法分析器，生成抽象语法树(AST)
- **虚拟机执行**：平台无关的字节码虚拟机，支持控制流和表达式计算
- **数据类型支持**：BOOL、INT、REAL、STRING基本数据类型
- **控制结构**：IF-THEN-ELSE、FOR循环、WHILE循环、CASE分支
- **函数支持**：函数定义、函数调用、参数传递、返回值处理
- **模块化库系统**：库文件加载(.st/.stbc)、符号导入、别名支持
- **标准库集成**：数学、字符串、时间、I/O等内置函数库
- **表达式计算**：算术运算、比较运算、逻辑运算
- **调试支持**：断点、单步执行、变量监视
- **双机热备**：主备同步、故障切换
- **安全级设计**：MISRA合规、静态内存管理

## 系统总体架构

### 核心设计原则

#### 1. 职责分离原则
每个模块有明确的单一职责，避免功能重叠和耦合：
- **libmgr**: 专注于库文件处理和符号提取
- **symbol_table**: 专注于运行时符号管理
- **vm**: 专注于字节码执行
- **mmgr**: 专注于内存管理

#### 2. 依赖方向控制
```
┌─────────────────┐    ┌─────────────────┐
│      main       │    │   Interactive   │
│   (CLI控制器)    │    │     Mode        │
└─────────────────┘    └─────────────────┘
         │                       │
         └───────┬───────────────┘
                 │
    ┌─────────────────────────────┐
    │       Compiler              │
    │  - Lexer (Flex)             │
    │  - Parser (Bison)           │
    │  - Type Checker             │
    │  - Code Generator           │
    └─────────────────────────────┘
                 │
                 ▼
    ┌─────────────────────────────┐
    │     Symbol Table            │  ← 运行时符号管理
    │  - 作用域管理               │
    │  - 符号定义和查找           │
    │  - 类型管理                 │
    └─────────────────────────────┘
                 ▲
                 │ 符号注册接口
                 │
    ┌─────────────────────────────┐
    │    Library Manager          │  ← 库文件处理
    │  - 库文件加载(.st/.stbc)     │
    │  - 符号提取                 │
    │  - 别名处理                 │
    └─────────────────────────────┘
                 │
                 ▼
    ┌─────────────────────────────┐
    │    Virtual Machine          │  ← 字节码执行
    │  - 栈式执行引擎             │
    │  - 指令集实现               │
    │  - 主从同步                 │
    └─────────────────────────────┘
                 │
    ┌─────────────────────────────┐
    │    Memory Manager           │  ← 统一内存管理
    │  - 静态内存分配             │
    │  - 内存池管理               │
    └─────────────────────────────┘
```

### 模块层级设计

#### 第一层：应用控制层
```
┌─────────────────┐
│      main.c     │  - CLI命令行解析
│   (应用入口)     │  - 模式选择和控制
│                 │  - 错误处理和清理
└─────────────────┘
```

#### 第二层：编译器核心层
```
┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐
│   Lexer/Parser  │  │  Type Checker   │  │  Code Generator │
│  (词法语法分析)  │  │   (类型检查)     │  │   (代码生成)     │
│                 │  │                 │  │                 │
│ - st_lexer.l    │  │ - type_checker  │  │ - codegen.c     │
│ - st_parser.y   │  │ - 语义分析      │  │ - 字节码生成    │
│ - ast.c         │  │ - 类型推导      │  │ - 优化处理      │
└─────────────────┘  └─────────────────┘  └─────────────────┘
```

#### 第三层：运行时管理层
```
┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐
│  Symbol Table   │  │ Library Manager │  │ Master-Slave    │
│  (符号表管理)    │  │  (库管理器)      │  │   Sync          │
│                 │  │                 │  │  (主从同步)      │
│ - 符号定义查找   │  │ - 库文件加载    │  │ - 状态同步      │
│ - 作用域管理    │  │ - 符号提取      │  │ - 故障切换      │
│ - 类型管理      │  │ - 别名处理      │  │ - 心跳检测      │
└─────────────────┘  └─────────────────┘  └─────────────────┘
```

#### 第四层：执行引擎层
```
┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐
│ Virtual Machine │  │   Bytecode      │  │   Debugger      │
│  (虚拟机)        │  │  (字节码)        │  │   (调试器)       │
│                 │  │                 │  │                 │
│ - 栈式执行      │  │ - 指令集定义    │  │ - 断点管理      │
│ - 指令解释      │  │ - 文件格式      │  │ - 单步执行      │
│ - 函数调用      │  │ - 序列化        │  │ - 变量监视      │
└─────────────────┘  └─────────────────┘  └─────────────────┘
```

#### 第五层：基础服务层
```
┌─────────────────┐  ┌─────────────────┐
│ Memory Manager  │  │ Builtin Libraries│
│  (内存管理器)    │  │  (内置库)        │
│                 │  │                 │
│ - 静态分配      │  │ - 数学库        │
│ - 内存池        │  │ - 字符串库      │
│ - 生命周期管理   │  │ - I/O库         │
└─────────────────┘  └─────────────────┘
```

### 模块功能详细设计

#### 1. 应用控制层 (main.c)

**核心职责**：
- CLI命令行解析和模式控制
- 系统初始化和资源清理
- 错误处理和统计输出

**主要功能**：
```c
// 支持的运行模式
typedef enum {
    MODE_COMPILE_ONLY,      // 仅编译
    MODE_COMPILE_AND_RUN,   // 编译并执行
    MODE_RUN_BYTECODE,      // 执行字节码
    MODE_INTERACTIVE,       // 交互模式(REPL)
    MODE_DEBUG,             // 调试模式
    MODE_DISASSEMBLE,       // 反汇编模式
    MODE_SYNTAX_CHECK,      // 语法检查
    MODE_PROFILE,           // 性能分析
    MODE_FORMAT             // 代码格式化
} run_mode_t;
```

**工作流程**：
1. 解析命令行参数，确定运行模式
2. 初始化各子系统（内存、符号表、库管理器）
3. 根据模式执行相应操作
4. 清理资源并退出

#### 2. 编译器核心层

##### 2.1 词法语法分析器 (Lexer/Parser)

**核心职责**：
- 将ST源代码转换为标记流
- 构建抽象语法树(AST)
- 处理语法错误和恢复

**关键特性**：
```yacc
// 支持的ST语法结构
program: 
    import_list PROGRAM IDENTIFIER 
    declaration_list 
    statement_list 
    END_PROGRAM

// 库导入语法
import_declaration:
    IMPORT IDENTIFIER SEMICOLON                    // 基本导入
    | IMPORT IDENTIFIER AS IDENTIFIER SEMICOLON   // 别名导入
    | IMPORT IDENTIFIER FROM STRING_LITERAL       // 路径导入
```

##### 2.2 类型检查器 (Type Checker)

**核心职责**：
- 语义分析和类型检查
- 类型推导和转换
- 符号引用解析

**类型系统**：
```c
typedef enum {
    TYPE_BOOL_ID,     // 布尔类型
    TYPE_INT_ID,      // 整数类型  
    TYPE_REAL_ID,     // 实数类型
    TYPE_STRING_ID,   // 字符串类型
    TYPE_ARRAY_ID,    // 数组类型
    TYPE_STRUCT_ID,   // 结构体类型
    TYPE_FUNCTION_ID  // 函数类型
} base_type_t;
```

##### 2.3 代码生成器 (Code Generator)

**核心职责**：
- 将AST编译为字节码指令
- 代码优化和常量折叠
- 符号地址分配

**字节码指令集**：
```c
typedef enum {
    // 数据操作
    OP_LOAD_CONST_INT, OP_LOAD_CONST_REAL, OP_LOAD_CONST_BOOL,
    OP_LOAD_LOCAL, OP_LOAD_GLOBAL, OP_STORE_LOCAL, OP_STORE_GLOBAL,
    
    // 算术运算
    OP_ADD_INT, OP_SUB_INT, OP_MUL_INT, OP_DIV_INT, OP_MOD_INT,
    OP_ADD_REAL, OP_SUB_REAL, OP_MUL_REAL, OP_DIV_REAL,
    
    // 逻辑运算
    OP_AND_BOOL, OP_OR_BOOL, OP_NOT_BOOL,
    
    // 比较运算
    OP_EQ_INT, OP_NE_INT, OP_LT_INT, OP_LE_INT, OP_GT_INT, OP_GE_INT,
    
    // 控制流
    OP_JMP, OP_JMP_TRUE, OP_JMP_FALSE, OP_CALL, OP_RET,
    
    // 系统控制
    OP_HALT, OP_NOP
} opcode_t;
```

#### 3. 运行时管理层

##### 3.1 符号表管理器 (Symbol Table)

**核心职责**：
- 运行时符号定义和查找
- 作用域管理（全局、函数、块）
- 类型信息管理

**接口设计**：
```c
// 符号定义接口
symbol_t* symbol_table_define_variable(symbol_table_t *table, const char *name, 
                                       type_info_t *type, var_category_t category);
symbol_t* symbol_table_define_function(symbol_table_t *table, const char *name,
                                       type_info_t *return_type, void *implementation);

// 库符号注册接口（关键）
symbol_t* symbol_table_register_library_function(symbol_table_t *table, const char *name,
                                                 const char *qualified_name, type_info_t *type,
                                                 void *implementation, const char *library_name);

// 符号查找接口
symbol_t* symbol_table_lookup(symbol_table_t *table, const char *name);
symbol_t* symbol_table_lookup_function(symbol_table_t *table, const char *name);
```

**作用域管理**：
```c
typedef enum {
    SCOPE_GLOBAL,       // 全局作用域
    SCOPE_PROGRAM,      // 程序作用域
    SCOPE_FUNCTION,     // 函数作用域
    SCOPE_BLOCK         // 块作用域
} scope_type_t;
```

##### 3.2 库管理器 (Library Manager)

**核心职责**：
- 库文件加载和解析(.st/.stbc文件)
- 符号提取和限定名生成
- 向符号表注册库符号

**关键功能**：
```c
// 库文件加载
int libmgr_load_library(library_manager_t *mgr, const char *name, 
                        const char *path, const char *alias);

// 符号注册到符号表（关键接口）
int libmgr_register_symbols_to_table(library_manager_t *mgr, const char *lib_name,
                                     struct symbol_table *symbol_table);

// 库信息查询
library_info_t* libmgr_get_library_info(library_manager_t *mgr, const char *name_or_alias);
```

**别名机制**：
```c
// 支持别名调用: math.sin(), str.length()
typedef struct library_symbol {
    char name[64];                    // 原始名: "sin"
    char qualified_name[128];         // 限定名: "math.sin"
    lib_symbol_type_t type;           // 符号类型
    void *implementation;             // 实现指针
    bool is_exported;                 // 是否导出
} library_symbol_t;
```

##### 3.3 主从同步管理器 (Master-Slave Sync)

**核心职责**：
- 主从节点状态同步
- 故障检测和切换
- 数据一致性保证

**同步机制**：
```c
typedef struct master_slave_sync {
    node_role_t role;                 // 节点角色（主/从）
    sync_state_t state;               // 同步状态
    
    // 网络通信
    int socket_fd;                    // 通信套接字
    struct sockaddr_in peer_addr;     // 对等节点地址
    
    // 同步数据
    uint32_t sync_sequence;           // 同步序列号
    sync_message_t message_buffer;    // 消息缓冲区
    
    // 健康监控
    uint64_t last_heartbeat;          // 最后心跳时间
    uint32_t missed_heartbeats;       // 丢失心跳计数
} master_slave_sync_t;
```

#### 4. 执行引擎层

##### 4.1 虚拟机 (Virtual Machine)

**核心职责**：
- 字节码指令执行
- 函数调用和栈管理
- 与符号表和同步系统集成

**虚拟机架构**：
```c
typedef struct st_vm {
    // 指令和数据
    bytecode_instr_t *instructions;    // 指令序列
    uint32_t pc;                       // 程序计数器
    
    // 运行时栈（静态分配）
    vm_stack_t operand_stack;          // 操作数栈
    call_frame_t call_stack[MAX_CALL_FRAMES]; // 调用栈
    
    // 内存区域（静态分配）
    vm_value_t global_vars[MAX_GLOBAL_VARS]; // 全局变量区
    
    // 系统集成
    symbol_table_t *symbol_table;      // 符号表引用
    library_manager_t *lib_mgr;        // 库管理器引用
    master_slave_sync_t *sync_mgr;     // 同步管理器引用
    
    // 执行状态
    vm_state_t state;                  // 运行状态
} st_vm_t;
```

**指令执行循环**：
```c
int vm_execute(st_vm_t *vm) {
    while (vm->state == VM_RUNNING && vm->pc < vm->instr_count) {
        bytecode_instr_t *instr = &vm->instructions[vm->pc];
        
        switch (instr->opcode) {
            case OP_LOAD_CONST_INT:
                // 加载整数常量到栈
                break;
            case OP_ADD_INT:
                // 整数加法运算
                break;
            case OP_CALL:
                // 函数调用
                break;
            // ... 其他指令
        }
        
        vm->pc++;
        
        // 处理同步消息
        if (vm->sync_mgr && vm->sync_enabled) {
            vm_process_sync_messages(vm);
        }
    }
}
```

##### 4.2 字节码系统 (Bytecode)

**核心职责**：
- 字节码文件格式定义
- 序列化和反序列化
- 字节码验证和加载

**文件格式**：
```c
typedef struct bytecode_file {
    struct {
        char magic[4];              // 魔数 "STBC"
        uint32_t version;           // 版本号
        uint32_t instr_count;       // 指令数量
        uint32_t const_pool_size;   // 常量池大小
        uint32_t symbol_table_size; // 符号表大小
    } header;
    
    bytecode_instr_t *instructions; // 指令序列
    void *const_pool;               // 常量池
    symbol_info_t *symbol_table;    // 符号表
} bytecode_file_t;
```

#### 5. 基础服务层

##### 5.1 内存管理器 (Memory Manager)

**核心职责**：
- 统一的内存分配和释放
- 静态内存池管理
- 内存泄漏检测

**内存管理策略**：
```c
// 静态内存池
typedef struct memory_pool {
    char data[POOL_SIZE];           // 内存池数据
    uint32_t allocated;             // 已分配大小
    uint32_t peak_usage;            // 峰值使用量
} memory_pool_t;

// 内存管理接口
void* mmgr_alloc(size_t size);
void mmgr_free(void *ptr);
char* mmgr_alloc_string(const char *str);
size_t mmgr_get_total_allocated(void);
```

##### 5.2 内置库系统 (Builtin Libraries)

**数学库 (builtin_math.c)**：
```c
// 数学函数实现
int builtin_sin(vm_value_t *args, uint32_t arg_count, vm_value_t *result);
int builtin_cos(vm_value_t *args, uint32_t arg_count, vm_value_t *result);
int builtin_sqrt(vm_value_t *args, uint32_t arg_count, vm_value_t *result);
int builtin_abs(vm_value_t *args, uint32_t arg_count, vm_value_t *result);
```

**字符串库 (builtin_string.c)**：
```c
// 字符串函数实现
int builtin_strlen(vm_value_t *args, uint32_t arg_count, vm_value_t *result);
int builtin_strcmp(vm_value_t *args, uint32_t arg_count, vm_value_t *result);
int builtin_strcat(vm_value_t *args, uint32_t arg_count, vm_value_t *result);
```

**I/O库 (builtin_io.c)**：
```c
// I/O函数实现
int builtin_print(vm_value_t *args, uint32_t arg_count, vm_value_t *result);
int builtin_read_int(vm_value_t *args, uint32_t arg_count, vm_value_t *result);
int builtin_write_file(vm_value_t *args, uint32_t arg_count, vm_value_t *result);
```

**时间库 (builtin_time.c)**：
```c
// 时间函数实现
int builtin_now(vm_value_t *args, uint32_t arg_count, vm_value_t *result);
int builtin_sleep(vm_value_t *args, uint32_t arg_count, vm_value_t *result);
```

### 模块依赖关系

#### 依赖层次图
```
Level 1: Application Layer
    main.c

Level 2: Compiler Layer  
    lexer/parser → ast.c → type_checker.c → codegen.c

Level 3: Runtime Management Layer
    symbol_table.c ← libmgr.c
    ms_sync.c

Level 4: Execution Engine Layer
    vm.c → bytecode.c
    debugger.c

Level 5: Base Service Layer
    mmgr.c
    builtin_*.c
```

#### 关键接口依赖
```
1. libmgr → symbol_table (符号注册接口)
   libmgr_register_symbols_to_table()

2. vm → symbol_table (符号查找接口)
   symbol_table_lookup_function()

3. vm → libmgr (库管理器设置)
   vm_set_library_manager()

4. codegen → symbol_table (符号地址分配)
   symbol_table_define_variable()

5. 所有模块 → mmgr (内存分配)
   mmgr_alloc(), mmgr_free()
```

### 工作流程设计

#### 编译流程
```
1. main.c 解析命令行，选择编译模式
2. libmgr 加载所需的库文件
3. lexer/parser 解析源代码生成AST
4. libmgr 将库符号注册到symbol_table
5. type_checker 进行语义分析和类型检查
6. codegen 生成字节码并保存文件
```

#### 执行流程
```
1. main.c 解析命令行，选择执行模式
2. vm 创建并初始化虚拟机实例
3. vm 加载字节码文件
4. libmgr 加载程序依赖的库
5. libmgr 将库符号注册到symbol_table
6. vm 设置符号表和库管理器引用
7. vm 执行字节码指令循环
```

#### 库加载流程
```
1. libmgr 检测文件类型(.st 或 .stbc)
2. libmgr 解析库文件提取符号信息
3. libmgr 生成别名限定名(alias.symbol)
4. libmgr 调用symbol_table注册接口
5. symbol_table 将库符号添加到全局作用域
6. vm 通过symbol_table查找并调用库函数
```

### 技术特性

#### 安全级设计
- **静态内存管理**：预分配内存池，避免动态分配
- **边界检查**：数组访问和栈操作的边界验证
- **类型安全**：严格的类型检查和转换规则
- **错误处理**：完善的错误码和异常处理机制

#### 性能优化
- **零拷贝设计**：符号表使用引用而非拷贝
- **哈希表优化**：符号查找使用哈希表加速
- **指令缓存**：字节码指令的局部性优化
- **栈式架构**：减少寄存器分配的复杂度

#### 可扩展性
- **模块化设计**：清晰的接口边界和职责分离
- **插件架构**：内置库的统一注册机制
- **配置化**：编译选项和运行时参数的灵活配置
- **调试支持**：完整的调试器接口和断点系统

### 构建要求

- GCC编译器
- Flex (词法分析器生成器)
- Bison (语法分析器生成器)
- Make构建工具

### 文件结构
```
./
├── main.c                   # 应用控制层 - CLI和模式控制
├── st_lexer.lex             # 词法规则定义（支持库导入）
├── st_parser.y              # 语法规则定义（支持库导入）
├── ast.h/ast.c              # 抽象语法树（支持导入节点）
├── symbol_table.h/c         # 符号表管理器（运行时符号管理）
├── libmgr.h/c               # 库管理器（库文件处理）
├── codegen.h/c              # 代码生成器
├── vm.h/vm.c                # 虚拟机实现（栈式执行引擎）
├── bytecode.h/c             # 字节码系统
├── mmgr.h/c                 # 静态内存管理器
├── builtin_math.c           # 内置数学库
├── builtin_string.c         # 内置字符串库
├── builtin_io.c             # 内置I/O库
├── builtin_time.c           # 内置时间库
├── Makefile                 # 构建脚本
└── CLAUDE.md                # 架构设计文档
```
