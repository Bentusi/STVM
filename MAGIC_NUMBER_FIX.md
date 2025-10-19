# 魔数字节序修正报告

## 问题描述

使用 `hexdump` 查看 STBC 字节码文件时，魔数显示不正确：

```bash
$ hexdump -C examples/mathlib.stbc
00000000  43 42 54 53 01 00 00 00  00 00 00 00 00 00 00 00  |CBTS............|
```

显示为 `43 42 54 53` (CBTS)，而不是期望的 `53 54 42 43` (STBC)。

## 根本原因

这是一个**字节序**（Endianness）问题：

### 原始定义

```c
#define STBC_MAGIC 0x53544243  // "STBC" in ASCII
```

这个定义的含义：
- `0x53` = 'S'
- `0x54` = 'T'  
- `0x42` = 'B'
- `0x43` = 'C'

### 存储方式

在**小端序**（Little-Endian）系统上（x86/x64），`uint32_t` 的字节存储顺序是反向的：

```
内存地址:    [0]   [1]   [2]   [3]
值 0x53544243:  43    42    54    53
显示为:         'C'   'B'   'T'   'S'  → "CBTS"
```

## 解决方案

### 修改魔数定义

调整魔数值以匹配小端序存储，使其在文件中正确显示为 "STBC"：

```c
// 修改前
#define STBC_MAGIC 0x53544243  // "STBC" in ASCII

// 修改后
#define STBC_MAGIC 0x43425453  // "STBC" in little-endian
```

### 新的存储方式

```
内存地址:    [0]   [1]   [2]   [3]
值 0x43425453:  53    54    42    43
显示为:         'S'   'T'   'B'   'C'  → "STBC" ✓
```

## 验证结果

### 库文件

```bash
$ hexdump -C examples/mathlib.stbc | head -3
00000000  53 54 42 43 01 00 00 00  00 00 00 00 00 00 00 00  |STBC............|
00000010  0b 00 00 00 2b 00 00 00  d2 02 00 00 00 00 00 00  |....+...........|
00000020  1f eb 3b 30 00 00 00 00  00 00 00 00 00 00 00 00  |..;0............|
```

✅ 魔数正确显示为 `53 54 42 43` → "STBC"

### 程序文件

```bash
$ hexdump -C examples/test_mathlib_demo.stbc | head -3
00000000  53 54 42 43 01 00 00 00  00 00 00 00 07 00 00 00  |STBC............|
00000010  0d 00 00 00 06 00 00 00  24 00 00 00 01 00 00 00  |........$.......|
00000020  7e 4c b8 e1 00 00 00 00  02 00 00 00 00 00 00 00  |~L..............|
```

✅ 魔数正确显示为 `53 54 42 43` → "STBC"

### 功能测试

```bash
$ ./build/bin/stvm -r examples/test_mathlib_demo.stbc
Power(2, 10) = 1024
Factorial(5) = 120
GCD(48, 18) = 6
Fibonacci(10) = 55
Lerp(0.0, 100.0, 0.75) = 75.000000
```

✅ 程序运行正常

## STBC 文件格式说明

### 文件头结构

```c
typedef struct STBCHeader {
    uint32_t magic;                 // 魔数: 0x43425453 (小端序)
    uint16_t version_major;         // 主版本号: 1
    uint16_t version_minor;         // 次版本号: 0
    uint32_t entry_point;           // 入口点地址
    uint32_t global_var_count;      // 全局变量数量
    uint32_t constant_count;        // 常量池大小
    uint32_t function_count;        // 函数数量
    uint32_t instruction_count;     // 指令数量
    uint32_t library_dep_count;     // 库依赖数量
    uint32_t checksum;              // CRC32校验和
} STBCHeader;
```

### 十六进制布局

```
偏移量  字段                  大小    示例值              说明
------  ------------------  ------  ------------------  ---------------------------
0x00    magic               4 字节  53 54 42 43         "STBC" 魔数
0x04    version_major       2 字节  01 00               主版本 = 1
0x06    version_minor       2 字节  00 00               次版本 = 0
0x08    entry_point         4 字节  00 00 00 00         入口点 = 0
0x0C    global_var_count    4 字节  07 00 00 00         全局变量 = 7
0x10    constant_count      4 字节  0d 00 00 00         常量数 = 13
0x14    function_count      4 字节  06 00 00 00         函数数 = 6
0x18    instruction_count   4 字节  24 00 00 00         指令数 = 36
0x1C    library_dep_count   4 字节  01 00 00 00         库依赖数 = 1
0x20    checksum            4 字节  7e 4c b8 e1         CRC32 校验和
```

### 文件结构

```
+----------------------+
| Header (40 bytes)    |  包含魔数和元数据
+----------------------+
| Constant Pool        |  常量数据
+----------------------+
| Function Table       |  函数信息
+----------------------+
| Instructions         |  字节码指令
+----------------------+
| Library Dependencies |  库依赖列表
+----------------------+
```

## 技术背景

### 字节序类型

1. **大端序** (Big-Endian)
   - 高位字节在前
   - 网络字节序
   - PowerPC, SPARC
   - 示例: `0x12345678` → `12 34 56 78`

2. **小端序** (Little-Endian)
   - 低位字节在前
   - x86/x64 架构
   - 示例: `0x12345678` → `78 56 34 12`

### 为什么选择小端序魔数？

**优势**:
1. ✅ 与系统架构一致（x86/x64）
2. ✅ hexdump 直接可读
3. ✅ 避免字节序转换
4. ✅ 符合 Linux 文件格式惯例

**示例**:
- ELF 文件: `7f 45 4c 46` → `.ELF`
- PNG 文件: `89 50 4e 47` → `.PNG`
- JPEG 文件: `ff d8 ff e0` → (二进制魔数)

### 兼容性考虑

如果未来需要在大端序系统上运行，可以：

1. **运行时转换** (推荐):
   ```c
   uint32_t read_magic = /* 从文件读取 */;
   if (read_magic == STBC_MAGIC) {
       // 小端序系统
   } else if (read_magic == 0x53544243) {
       // 大端序系统 - 需要转换所有多字节字段
   }
   ```

2. **编译时检测**:
   ```c
   #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
       #define STBC_MAGIC 0x43425453
   #else
       #define STBC_MAGIC 0x53544243
   #endif
   ```

## 迁移指南

### 对于现有字节码文件

**重要**: 魔数值改变后，旧的字节码文件无法被新版本识别。

**解决方案**: 重新编译所有字节码文件

```bash
# 查找所有 .stbc 文件
find . -name "*.stbc" -type f

# 删除旧文件
find . -name "*.stbc" -type f -delete

# 重新编译库
stvm -c examples/mathlib.st --library

# 重新编译程序
stvm -c examples/*.st
```

### 版本检查

新版本编译器会拒绝旧格式的字节码：

```c
bool bytecode_validate_header(const STBCHeader* header) {
    if (header->magic != STBC_MAGIC) {
        return false;  // 魔数不匹配
    }
    // ...
}
```

错误信息:
```
错误：无法加载字节码文件 'old_file.stbc'
```

## 验证魔数的方法

### 方法 1: hexdump

```bash
hexdump -C file.stbc | head -1
# 应该显示: 53 54 42 43 |STBC|
```

### 方法 2: xxd

```bash
xxd file.stbc | head -1
# 应该显示: 00000000: 5354 4243 ...
```

### 方法 3: od

```bash
od -An -tx1 -N4 file.stbc
# 应该显示: 53 54 42 43
```

### 方法 4: Python

```python
with open('file.stbc', 'rb') as f:
    magic = f.read(4)
    print(magic.hex())  # 应该显示: 53544243
    print(magic)        # 应该显示: b'STBC'
```

## 总结

### 变更内容

- **文件**: `src/include/bytecode_io.h`
- **变更**: `STBC_MAGIC` 从 `0x53544243` 改为 `0x43425453`
- **原因**: 修正小端序系统上的魔数显示

### 影响

- ✅ 魔数在 hexdump 中正确显示为 "STBC"
- ✅ 符合 Linux 文件格式惯例
- ✅ 提高可读性和调试便利性
- ⚠️ 需要重新编译所有字节码文件

### 测试状态

- ✅ 编译成功，无错误
- ✅ 魔数显示正确
- ✅ 程序运行正常
- ✅ 动态链接工作正常
- ✅ 静态链接工作正常

---

**修正时间**: 2025年10月19日  
**STVM版本**: v1.0.0  
**影响范围**: 所有 .stbc 字节码文件
