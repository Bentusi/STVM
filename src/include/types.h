/**
 * @file types.h
 * @brief 基础类型定义 - ST语言数据类型和运行时值表示
 */

#ifndef STVM_TYPES_H
#define STVM_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief 数据类型枚举
 */
typedef enum {
    TYPE_VOID,      // 无类型（用于函数无返回值）
    TYPE_BOOL,      // 布尔类型
    TYPE_INT,       // 整型（32位）
    TYPE_REAL,      // 实数类型（双精度浮点）
    TYPE_STRING,    // 字符串类型
    TYPE_ARRAY,     // 数组类型
    TYPE_FUNCTION   // 函数类型
} DataType;

/**
 * @brief 运行时值表示（带类型标记）
 */
typedef struct {
    DataType type;
    union {
        bool bool_val;
        int32_t int_val;
        double real_val;
        char* string_val;       // 字符串指针（由内存管理器管理）
        struct {
            void* data;         // 数组数据指针
            int32_t length;     // 数组长度
            DataType elem_type; // 元素类型
        } array_val;
        struct {
            uint32_t address;   // 函数入口地址
            int32_t param_count;// 参数个数
        } func_val;
    };
} Value;

/**
 * @brief 类型信息（用于编译时类型检查）
 * 
 * 所有权规则：
 * - TypeInfo使用引用计数管理生命周期
 * - 创建时ref_count=1
 * - type_info_retain增加ref_count
 * - type_info_free减少ref_count，为0时释放
 * - 子类型（elem_type, return_type, param_types）被引用时自动retain
 */
typedef struct TypeInfo {
    DataType base_type;         // 基础类型
    int32_t ref_count;          // 引用计数
    
    // 数组类型扩展信息
    struct {
        struct TypeInfo* elem_type; // 元素类型（递归定义，支持多维数组）
        int32_t dimensions;         // 维度数
        int32_t* sizes;             // 每个维度的大小（动态数组时为NULL）
    } array_info;
    
    // 函数类型扩展信息
    struct {
        struct TypeInfo* return_type;   // 返回类型
        struct TypeInfo** param_types;  // 参数类型数组
        int32_t param_count;            // 参数个数
    } func_info;
} TypeInfo;

/**
 * @brief 获取类型名称字符串
 * @param type 数据类型
 * @return 类型名称
 */
const char* type_to_string(DataType type);

/**
 * @brief 创建简单类型信息
 * @param type 数据类型
 * @return 类型信息指针
 */
TypeInfo* type_info_create(DataType type);

/**
 * @brief 创建数组类型信息
 * @param elem_type 元素类型
 * @param dimensions 维度数
 * @param sizes 每个维度的大小数组
 * @return 类型信息指针
 */
TypeInfo* type_info_create_array(TypeInfo* elem_type, int32_t dimensions, int32_t* sizes);

/**
 * @brief 创建函数类型信息
 * @param return_type 返回类型
 * @param param_types 参数类型数组
 * @param param_count 参数个数
 * @return 类型信息指针
 */
TypeInfo* type_info_create_function(TypeInfo* return_type, TypeInfo** param_types, int32_t param_count);

/**
 * @brief 增加类型信息的引用计数
 * @param type_info 类型信息指针
 * @return 同一个类型信息指针（便于链式调用）
 */
TypeInfo* type_info_retain(TypeInfo* type_info);

/**
 * @brief 释放类型信息（减少引用计数，为0时真正释放）
 * @param type_info 类型信息指针
 */
void type_info_free(TypeInfo* type_info);

/**
 * @brief 检查两个类型是否相等
 * @param t1 类型1
 * @param t2 类型2
 * @return 相等返回true，否则返回false
 */
bool type_info_equals(const TypeInfo* t1, const TypeInfo* t2);

/**
 * @brief 检查类型是否可以隐式转换
 * @param from 源类型
 * @param to 目标类型
 * @return 可以转换返回true，否则返回false
 */
bool type_info_can_convert(const TypeInfo* from, const TypeInfo* to);

/**
 * @brief 创建一个Value值
 * @param type 值类型
 * @return 初始化的Value
 */
Value value_create(DataType type);

/**
 * @brief 将Value转换为布尔值
 * @param v 值
 * @return 布尔值
 */
bool value_to_bool(Value v);

/**
 * @brief 将Value转换为整数
 * @param v 值
 * @return 整数值
 */
int32_t value_to_int(Value v);

/**
 * @brief 将Value转换为实数
 * @param v 值
 * @return 实数值
 */
double value_to_real(Value v);

/**
 * @brief 将Value转换为字符串（用于调试输出）
 * @param v 值
 * @param buffer 输出缓冲区
 * @param size 缓冲区大小
 */
void value_to_string(Value v, char* buffer, size_t size);

/**
 * @brief 复制Value值（深拷贝）
 * @param v 源值
 * @return 新的值
 */
Value value_copy(Value v);

/**
 * @brief 释放Value值持有的资源
 * @param v 值指针
 */
void value_free(Value* v);

#endif // STVM_TYPES_H
