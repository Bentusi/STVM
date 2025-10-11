# 第三阶段实施计划

## 当前状态分析

已完成：
- ✅ Flex词法分析器文件 (st_lexer.l)
- ✅ Bison语法分析器文件 (st_parser.y)  
- ✅ 类型检查器头文件和实现 (typecheck.h/c)
- ✅ Makefile已更新支持Flex/Bison

## 发现的问题

### 1. AST接口不匹配
现有AST定义使用的命名：
- `AST_ASSIGN` (不是 AST_ASSIGNMENT)
- `AST_IF`, `AST_WHILE`, `AST_FOR`, `AST_CASE`, `AST_RETURN`
- `BINOP_ADD` (不是 OP_ADD)
- 结构体字段: `program.declarations`, `program.statements` (不是 var_decls, body)

### 2. 需要适配的部分
- parser.y 中的AST节点创建调用
- typecheck.c 中的AST节点类型检查
- 测试代码中的字段访问

## 解决方案

### 选项A：修改parser和typecheck适配现有AST ✓ (推荐)
优点：
- 不破坏现有Phase 1-2代码
- 保持AST测试通过
- 只需修改新代码

缺点：
- 需要仔细核对所有字段名

### 选项B：修改AST以匹配parser
优点：
- parser代码更简洁

缺点：
- 需要修改大量现有代码
- 可能破坏Phase 1-2测试
- 风险较大

## 实施步骤

1. 详细查看AST头文件确定所有接口
2. 更新st_parser.y使用正确的AST创建函数
3. 更新typecheck.c使用正确的AST字段名
4. 更新测试代码
5. 编译测试验证

## AST接口映射表

| Parser概念 | AST节点类型 | 创建函数 | 主要字段 |
|-----------|------------|---------|---------|
| program | AST_PROGRAM | ast_create_program | declarations, functions, statements |
| var_decl | AST_VAR_DECL | ast_create_var_decl | name, type, initializer, is_const |
| function | AST_FUNCTION_DECL | ast_create_function_decl | name, parameters, return_type, declarations, body |
| import | AST_IMPORT | ast_create_import | module_name, symbols, symbol_count, aliases |
| assignment | AST_ASSIGN | ast_create_assign | target, value |
| if | AST_IF | ast_create_if | condition, then_branch, else_branch |
| while | AST_WHILE | ast_create_while | condition, body |
| for | AST_FOR | ast_create_for | variable, start, end, step, body |
| return | AST_RETURN | ast_create_return | value |
| binary_op | AST_BINARY_OP | ast_create_binary_op | op (BINOP_*), left, right |
| unary_op | AST_UNARY_OP | ast_create_unary_op | op (UNOP_*), operand |
| literal | AST_LITERAL | ast_create_literal | value |
| identifier | AST_IDENTIFIER | ast_create_identifier | name |
| function_call | AST_FUNCTION_CALL | ast_create_function_call | name, arguments, arg_count |
| array_access | AST_ARRAY_ACCESS | ast_create_array_access | array, indices, index_count |

