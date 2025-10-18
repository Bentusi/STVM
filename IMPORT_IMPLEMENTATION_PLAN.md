# IMPORT 功能完整实现计划

## 目标

实现完整的IMPORT功能，包括：
1. 导入外部函数库
2. 支持别名避免命名冲突  
3. 支持库文件生成

## 当前状态分析

### ✅ 已完成
1. **libmgr.h/c** - 库管理器基础框架
   - 加载.stbc库文件
   - 符号导入和解析
   - 别名支持
   - 搜索路径管理

2. **AST支持** - AST_IMPORT节点
   - import结构定义
   - ast_create_import函数

3. **词法/语法** - IMPORT关键字
   - TOKEN_IMPORT
   - TOKEN_AS
   - 基础语法规则

### ❌ 需要完善

1. **语法扩展**
   - 支持: IMPORT 'library.stbc';
   - 支持: IMPORT 'library.stbc' AS alias;
   - 支持: IMPORT func1, func2 FROM 'library.stbc';
   - 支持: IMPORT func1 AS alias1 FROM 'library.stbc';

2. **类型检查集成**
   - 在typecheck阶段处理IMPORT节点
   - 调用libmgr加载库
   - 验证导入符号

3. **代码生成集成**
   - 处理库函数调用
   - 链接外部函数

4. **库文件生成**
   - 添加编译选项 -L/--library
   - 生成可导入的.stbc库文件
   - 导出函数签名信息

5. **命令行工具**
   - stvm -L output.stbc input.st  (生成库)
   - stvm --lib-path ./libs  (添加搜索路径)
   - stvm --list-imports  (列出导入)

## 实现步骤

### Phase 1: 语法完善
- [ ] 更新st_parser.y支持完整IMPORT语法
- [ ] 更新st_lexer.l添加FROM关键字
- [ ] 测试语法解析

### Phase 2: 类型检查集成
- [ ] 在typecheck.c中处理AST_IMPORT节点
- [ ] 调用libmgr_load_library
- [ ] 调用libmgr_import_symbol
- [ ] 处理别名

### Phase 3: 代码生成
- [ ] 在codegen.c中处理导入函数调用
- [ ] 链接外部函数地址

### Phase 4: 库文件生成
- [ ] 添加bytecode_save_as_library函数
- [ ] 导出函数签名和类型信息
- [ ] 命令行选项

### Phase 5: 测试和文档
- [ ] 创建测试库
- [ ] 创建测试程序
- [ ] 编写文档

## 语法设计

### 导入整个库
```st
IMPORT 'math.stbc';              (* 导入math库的所有函数 *)
```

### 导入并使用别名
```st
IMPORT 'mylib.stbc' AS mylib;    (* 使用mylib前缀访问 *)
mylib.function();
```

### 导入特定符号
```st
IMPORT Add, Multiply FROM 'math.stbc';
```

### 导入带别名
```st
IMPORT Add AS Plus FROM 'math.stbc';
```

### 混合导入
```st
IMPORT Add, Multiply AS Mul FROM 'math.stbc';
```

## 文件格式

### .stbc库文件
- 标准字节码格式
- 包含函数签名
- 包含类型信息
- 可以被其他程序导入

### 示例工作流

1. 编写库代码 (mathlib.st):
```st
FUNCTION Add(a: INT, b: INT): INT
VAR
END_VAR
    Add := a + b;
END_FUNCTION

FUNCTION Multiply(a: INT, b: INT): INT
VAR
END_VAR
    Multiply := a * b;
END_FUNCTION
```

2. 编译为库:
```bash
stvm -L mathlib.stbc mathlib.st
```

3. 使用库 (main.st):
```st
IMPORT 'mathlib.stbc';

PROGRAM Main
VAR
    x, y, sum, product: INT;
END_VAR
    x := 10;
    y := 20;
    sum := Add(x, y);
    product := Multiply(x, y);
    PRINT('Sum: %d, Product: %d\n', sum, product);
END_PROGRAM
```

4. 编译并运行:
```bash
stvm main.st
```

