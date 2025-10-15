/**
 * @file types.c
 * @brief 基础类型定义的实现
 */

#include "types.h"
#include "mmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief 获取类型名称字符串
 */
const char* type_to_string(DataType type) {
    switch (type) {
        case TYPE_VOID:     return "void";
        case TYPE_BOOL:     return "bool";
        case TYPE_INT:      return "int";
        case TYPE_REAL:     return "real";
        case TYPE_STRING:   return "string";
        case TYPE_ARRAY:    return "array";
        case TYPE_FUNCTION: return "function";
        default:            return "unknown";
    }
}

/**
 * @brief 创建简单类型信息
 */
TypeInfo* type_info_create(DataType type) {
    TypeInfo* ti = (TypeInfo*)mmgr_alloc(sizeof(TypeInfo));
    if (!ti) return NULL;
    
    memset(ti, 0, sizeof(TypeInfo));
    ti->base_type = type;
    ti->ref_count = 1;  // 初始引用计数为1
    
    return ti;
}

/**
 * @brief 创建数组类型信息
 */
TypeInfo* type_info_create_array(TypeInfo* elem_type, int32_t dimensions, int32_t* sizes) {
    if (!elem_type || dimensions <= 0) return NULL;
    
    TypeInfo* ti = (TypeInfo*)mmgr_alloc(sizeof(TypeInfo));
    if (!ti) return NULL;
    
    memset(ti, 0, sizeof(TypeInfo));
    ti->base_type = TYPE_ARRAY;
    ti->ref_count = 1;  // 初始引用计数为1
    ti->array_info.elem_type = type_info_retain(elem_type);  // 增加子类型引用计数
    ti->array_info.dimensions = dimensions;
    
    if (sizes) {
        ti->array_info.sizes = (int32_t*)mmgr_alloc(sizeof(int32_t) * dimensions);
        if (ti->array_info.sizes) {
            memcpy(ti->array_info.sizes, sizes, sizeof(int32_t) * dimensions);
        }
    } else {
        ti->array_info.sizes = NULL;
    }
    
    return ti;
}

/**
 * @brief 创建函数类型信息
 */
TypeInfo* type_info_create_function(TypeInfo* return_type, TypeInfo** param_types, int32_t param_count) {
    TypeInfo* ti = (TypeInfo*)mmgr_alloc(sizeof(TypeInfo));
    if (!ti) return NULL;
    
    memset(ti, 0, sizeof(TypeInfo));
    ti->base_type = TYPE_FUNCTION;
    ti->ref_count = 1;  // 初始引用计数为1
    ti->func_info.return_type = return_type ? type_info_retain(return_type) : NULL;  // 增加返回类型引用计数
    ti->func_info.param_count = param_count;
    
    if (param_count > 0 && param_types) {
        ti->func_info.param_types = (TypeInfo**)mmgr_alloc(sizeof(TypeInfo*) * param_count);
        if (ti->func_info.param_types) {
            // 复制指针并增加每个参数类型的引用计数
            for (int32_t i = 0; i < param_count; i++) {
                ti->func_info.param_types[i] = param_types[i] ? type_info_retain(param_types[i]) : NULL;
            }
        }
    } else {
        ti->func_info.param_types = NULL;
    }
    
    return ti;
}

/**
 * @brief 增加类型信息的引用计数
 */
TypeInfo* type_info_retain(TypeInfo* type_info) {
    if (!type_info) return NULL;
    type_info->ref_count++;
    return type_info;
}

/**
 * @brief 释放类型信息（减少引用计数，为0时真正释放）
 */
void type_info_free(TypeInfo* type_info) {
    if (!type_info) return;
    
    // 减少引用计数
    type_info->ref_count--;
    
    // 引用计数为0时才真正释放
    if (type_info->ref_count > 0) {
        return;
    }
    
    // 递归释放数组元素类型
    if (type_info->base_type == TYPE_ARRAY) {
        if (type_info->array_info.elem_type) {
            type_info_free(type_info->array_info.elem_type);
        }
        if (type_info->array_info.sizes) {
            mmgr_free(type_info->array_info.sizes);
        }
    }
    
    // 递归释放函数类型信息
    if (type_info->base_type == TYPE_FUNCTION) {
        if (type_info->func_info.return_type) {
            type_info_free(type_info->func_info.return_type);
        }
        if (type_info->func_info.param_types) {
            for (int32_t i = 0; i < type_info->func_info.param_count; i++) {
                if (type_info->func_info.param_types[i]) {
                    type_info_free(type_info->func_info.param_types[i]);
                }
            }
            mmgr_free(type_info->func_info.param_types);
        }
    }
    
    mmgr_free(type_info);
}

/**
 * @brief 检查两个类型是否相等
 */
bool type_info_equals(const TypeInfo* t1, const TypeInfo* t2) {
    if (!t1 || !t2) return false;
    if (t1->base_type != t2->base_type) return false;
    
    // 简单类型直接比较
    if (t1->base_type != TYPE_ARRAY && t1->base_type != TYPE_FUNCTION) {
        return true;
    }
    
    // 数组类型需要比较元素类型和维度
    if (t1->base_type == TYPE_ARRAY) {
        if (t1->array_info.dimensions != t2->array_info.dimensions) {
            return false;
        }
        return type_info_equals(t1->array_info.elem_type, t2->array_info.elem_type);
    }
    
    // 函数类型需要比较返回类型和参数类型
    if (t1->base_type == TYPE_FUNCTION) {
        if (t1->func_info.param_count != t2->func_info.param_count) {
            return false;
        }
        if (!type_info_equals(t1->func_info.return_type, t2->func_info.return_type)) {
            return false;
        }
        for (int32_t i = 0; i < t1->func_info.param_count; i++) {
            if (!type_info_equals(t1->func_info.param_types[i], t2->func_info.param_types[i])) {
                return false;
            }
        }
        return true;
    }
    
    return false;
}

/**
 * @brief 检查类型是否可以隐式转换
 */
bool type_info_can_convert(const TypeInfo* from, const TypeInfo* to) {
    if (!from || !to) return false;
    if (type_info_equals(from, to)) return true;
    
    // INT 可以转换为 REAL
    if (from->base_type == TYPE_INT && to->base_type == TYPE_REAL) {
        return true;
    }
    
    // BOOL 可以转换为 INT
    if (from->base_type == TYPE_BOOL && to->base_type == TYPE_INT) {
        return true;
    }
    
    return false;
}

/**
 * @brief 创建一个Value值
 */
Value value_create(DataType type) {
    Value v;
    memset(&v, 0, sizeof(Value));
    v.type = type;
    
    switch (type) {
        case TYPE_BOOL:
            v.bool_val = false;
            break;
        case TYPE_INT:
            v.int_val = 0;
            break;
        case TYPE_REAL:
            v.real_val = 0.0;
            break;
        case TYPE_STRING:
            v.string_val = NULL;
            break;
        case TYPE_ARRAY:
            v.array_val.data = NULL;
            v.array_val.length = 0;
            v.array_val.elem_type = TYPE_VOID;
            break;
        case TYPE_FUNCTION:
            v.func_val.address = 0;
            v.func_val.param_count = 0;
            break;
        default:
            break;
    }
    
    return v;
}

/**
 * @brief 将Value转换为布尔值
 */
bool value_to_bool(Value v) {
    switch (v.type) {
        case TYPE_BOOL:
            return v.bool_val;
        case TYPE_INT:
            return v.int_val != 0;
        case TYPE_REAL:
            return v.real_val != 0.0;
        case TYPE_STRING:
            return v.string_val != NULL && v.string_val[0] != '\0';
        default:
            return false;
    }
}

/**
 * @brief 将Value转换为整数
 */
int32_t value_to_int(Value v) {
    switch (v.type) {
        case TYPE_BOOL:
            return v.bool_val ? 1 : 0;
        case TYPE_INT:
            return v.int_val;
        case TYPE_REAL:
            return (int32_t)v.real_val;
        default:
            return 0;
    }
}

/**
 * @brief 将Value转换为实数
 */
double value_to_real(Value v) {
    switch (v.type) {
        case TYPE_BOOL:
            return v.bool_val ? 1.0 : 0.0;
        case TYPE_INT:
            return (double)v.int_val;
        case TYPE_REAL:
            return v.real_val;
        default:
            return 0.0;
    }
}

/**
 * @brief 将Value转换为字符串（用于调试输出）
 */
void value_to_string(Value v, char* buffer, size_t size) {
    if (!buffer || size == 0) return;
    
    switch (v.type) {
        case TYPE_VOID:
            snprintf(buffer, size, "void");
            break;
        case TYPE_BOOL:
            snprintf(buffer, size, "%s", v.bool_val ? "true" : "false");
            break;
        case TYPE_INT:
            snprintf(buffer, size, "%d", v.int_val);
            break;
        case TYPE_REAL:
            snprintf(buffer, size, "%g", v.real_val);
            break;
        case TYPE_STRING:
            snprintf(buffer, size, "\"%s\"", v.string_val ? v.string_val : "");
            break;
        case TYPE_ARRAY:
            snprintf(buffer, size, "[array:%d]", v.array_val.length);
            break;
        case TYPE_FUNCTION:
            snprintf(buffer, size, "<function@%u>", v.func_val.address);
            break;
        default:
            snprintf(buffer, size, "<unknown>");
            break;
    }
}

/**
 * @brief 复制Value值（深拷贝）
 */
Value value_copy(Value v) {
    Value new_val = v;
    
    // 字符串需要深拷贝
    if (v.type == TYPE_STRING && v.string_val) {
        new_val.string_val = mmgr_strdup(v.string_val);
    }
    
    // 数组需要深拷贝（这里简化处理，实际可能需要更复杂的逻辑）
    if (v.type == TYPE_ARRAY && v.array_val.data) {
        // TODO: 实现数组深拷贝
        // 当前版本暂不支持
        new_val.array_val.data = NULL;
        new_val.array_val.length = 0;
    }
    
    return new_val;
}

/**
 * @brief 释放Value值持有的资源
 */
void value_print(Value* v) {
    if (!v) {
        printf("NULL");
        return;
    }
    
    switch (v->type) {
        case TYPE_INT:
            printf("%d", v->int_val);
            break;
        case TYPE_REAL:
            printf("%g", v->real_val);
            break;
        case TYPE_BOOL:
            printf("%s", v->bool_val ? "true" : "false");
            break;
        case TYPE_STRING:
            printf("\"%s\"", v->string_val ? v->string_val : "");
            break;
        case TYPE_ARRAY:
            printf("[array len=%d]", v->array_val.length);
            break;
        default:
            printf("<?>");
            break;
    }
}

void value_free(Value* v) {
    if (!v) return;
    
    // 释放字符串
    if (v->type == TYPE_STRING && v->string_val) {
        mmgr_free(v->string_val);
        v->string_val = NULL;
    }
    
    // 释放数组
    if (v->type == TYPE_ARRAY && v->array_val.data) {
        mmgr_free(v->array_val.data);
        v->array_val.data = NULL;
        v->array_val.length = 0;
    }
}
