/*
 * IEC61131 内置字符串库实现
 * 提供标准字符串操作函数
 */

#include "libmgr.h"
#include "symbol_table.h"
#include "mmgr.h"
#include "vm.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* 内部辅助函数声明 */
static bool validate_string_arg(struct vm_value *arg, const char *func_name);
static bool validate_int_arg(struct vm_value *arg, const char *func_name);
static void set_string_error(const char *func_name, const char *error);
static char* safe_string_alloc(size_t size);
static int safe_string_copy(char *dest, const char *src, size_t dest_size);

/* ========== 基本字符串函数实现 ========== */

/* 获取字符串长度 */
static int builtin_strlen(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_string_error("LEN", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_string_arg(&args[0], "LEN")) {
        return -1;
    }
    
    result->type = VAL_INT;
    result->data.int_val = (int32_t)strlen(args[0].data.string_val);
    
    return 0;
}

/* 字符串连接 */
static int builtin_concat(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2) {
        set_string_error("CONCAT", "参数数量错误：期望2个参数");
        return -1;
    }
    
    if (!validate_string_arg(&args[0], "CONCAT") || !validate_string_arg(&args[1], "CONCAT")) {
        return -1;
    }
    
    const char *str1 = args[0].data.string_val;
    const char *str2 = args[1].data.string_val;
    
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    size_t total_len = len1 + len2 + 1;
    
    /* 检查长度限制 */
    if (total_len > 1024) { /* 设置合理的字符串长度限制 */
        set_string_error("CONCAT", "结果字符串过长");
        return -1;
    }
    
    char *new_str = safe_string_alloc(total_len);
    if (!new_str) {
        set_string_error("CONCAT", "内存分配失败");
        return -1;
    }
    
    strcpy(new_str, str1);
    strcat(new_str, str2);
    
    result->type = VAL_STRING;
    result->data.string_val = new_str;
    
    return 0;
}

/* 字符串比较 */
static int builtin_strcmp(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2) {
        set_string_error("COMPARE", "参数数量错误：期望2个参数");
        return -1;
    }
    
    if (!validate_string_arg(&args[0], "COMPARE") || !validate_string_arg(&args[1], "COMPARE")) {
        return -1;
    }
    
    const char *str1 = args[0].data.string_val;
    const char *str2 = args[1].data.string_val;
    
    int cmp_result = strcmp(str1, str2);
    
    result->type = VAL_INT;
    if (cmp_result < 0) {
        result->data.int_val = -1;
    } else if (cmp_result > 0) {
        result->data.int_val = 1;
    } else {
        result->data.int_val = 0;
    }
    
    return 0;
}

/* 字符串子串提取 */
static int builtin_substring(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 3) {
        set_string_error("SUBSTRING", "参数数量错误：期望3个参数（字符串，起始位置，长度）");
        return -1;
    }
    
    if (!validate_string_arg(&args[0], "SUBSTRING") || 
        !validate_int_arg(&args[1], "SUBSTRING") || 
        !validate_int_arg(&args[2], "SUBSTRING")) {
        return -1;
    }
    
    const char *source = args[0].data.string_val;
    int32_t start = args[1].data.int_val;
    int32_t length = args[2].data.int_val;
    
    int32_t source_len = (int32_t)strlen(source);
    
    /* 参数验证 */
    if (start < 1 || start > source_len) {
        set_string_error("SUBSTRING", "起始位置超出字符串范围（1-based）");
        return -1;
    }
    
    if (length < 0) {
        set_string_error("SUBSTRING", "长度不能为负数");
        return -1;
    }
    
    /* 调整参数为0-based */
    start--; /* ST语言使用1-based索引 */
    
    /* 计算实际提取长度 */
    if (start + length > source_len) {
        length = source_len - start;
    }
    
    if (length <= 0) {
        /* 返回空字符串 */
        char *empty_str = safe_string_alloc(1);
        if (!empty_str) {
            set_string_error("SUBSTRING", "内存分配失败");
            return -1;
        }
        empty_str[0] = '\0';
        result->type = VAL_STRING;
        result->data.string_val = empty_str;
        return 0;
    }
    
    /* 分配新字符串 */
    char *new_str = safe_string_alloc(length + 1);
    if (!new_str) {
        set_string_error("SUBSTRING", "内存分配失败");
        return -1;
    }
    
    strncpy(new_str, source + start, length);
    new_str[length] = '\0';
    
    result->type = VAL_STRING;
    result->data.string_val = new_str;
    
    return 0;
}

/* 查找子字符串位置 */
static int builtin_indexof(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2) {
        set_string_error("INDEXOF", "参数数量错误：期望2个参数（主字符串，子字符串）");
        return -1;
    }
    
    if (!validate_string_arg(&args[0], "INDEXOF") || !validate_string_arg(&args[1], "INDEXOF")) {
        return -1;
    }
    
    const char *haystack = args[0].data.string_val;
    const char *needle = args[1].data.string_val;
    
    char *found = strstr(haystack, needle);
    
    result->type = VAL_INT;
    if (found) {
        result->data.int_val = (int32_t)(found - haystack) + 1; /* 转换为1-based */
    } else {
        result->data.int_val = 0; /* 未找到返回0 */
    }
    
    return 0;
}

/* 从右侧查找子字符串位置 */
static int builtin_lastindexof(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2) {
        set_string_error("LASTINDEXOF", "参数数量错误：期望2个参数");
        return -1;
    }
    
    if (!validate_string_arg(&args[0], "LASTINDEXOF") || !validate_string_arg(&args[1], "LASTINDEXOF")) {
        return -1;
    }
    
    const char *haystack = args[0].data.string_val;
    const char *needle = args[1].data.string_val;
    
    size_t needle_len = strlen(needle);
    if (needle_len == 0) {
        result->type = VAL_INT;
        result->data.int_val = (int32_t)strlen(haystack) + 1;
        return 0;
    }
    
    char *last_found = NULL;
    char *current = (char*)haystack;
    
    while ((current = strstr(current, needle)) != NULL) {
        last_found = current;
        current++;
    }
    
    result->type = VAL_INT;
    if (last_found) {
        result->data.int_val = (int32_t)(last_found - haystack) + 1; /* 转换为1-based */
    } else {
        result->data.int_val = 0; /* 未找到返回0 */
    }
    
    return 0;
}

/* ========== 字符串转换函数 ========== */

/* 转换为大写 */
static int builtin_toupper(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_string_error("TOUPPER", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_string_arg(&args[0], "TOUPPER")) {
        return -1;
    }
    
    const char *source = args[0].data.string_val;
    size_t len = strlen(source);
    
    char *new_str = safe_string_alloc(len + 1);
    if (!new_str) {
        set_string_error("TOUPPER", "内存分配失败");
        return -1;
    }
    
    for (size_t i = 0; i <= len; i++) {
        new_str[i] = (char)toupper(source[i]);
    }
    
    result->type = VAL_STRING;
    result->data.string_val = new_str;
    
    return 0;
}

/* 转换为小写 */
static int builtin_tolower(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_string_error("TOLOWER", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_string_arg(&args[0], "TOLOWER")) {
        return -1;
    }
    
    const char *source = args[0].data.string_val;
    size_t len = strlen(source);
    
    char *new_str = safe_string_alloc(len + 1);
    if (!new_str) {
        set_string_error("TOLOWER", "内存分配失败");
        return -1;
    }
    
    for (size_t i = 0; i <= len; i++) {
        new_str[i] = (char)tolower(source[i]);
    }
    
    result->type = VAL_STRING;
    result->data.string_val = new_str;
    
    return 0;
}

/* 去除首尾空白字符 */
static int builtin_trim(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_string_error("TRIM", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_string_arg(&args[0], "TRIM")) {
        return -1;
    }
    
    const char *source = args[0].data.string_val;
    
    /* 跳过开头的空白字符 */
    while (isspace(*source)) {
        source++;
    }
    
    if (*source == '\0') {
        /* 字符串全是空白字符 */
        char *empty_str = safe_string_alloc(1);
        if (!empty_str) {
            set_string_error("TRIM", "内存分配失败");
            return -1;
        }
        empty_str[0] = '\0';
        result->type = VAL_STRING;
        result->data.string_val = empty_str;
        return 0;
    }
    
    /* 找到结尾的非空白字符 */
    const char *end = source + strlen(source) - 1;
    while (end > source && isspace(*end)) {
        end--;
    }
    
    size_t len = end - source + 1;
    char *new_str = safe_string_alloc(len + 1);
    if (!new_str) {
        set_string_error("TRIM", "内存分配失败");
        return -1;
    }
    
    strncpy(new_str, source, len);
    new_str[len] = '\0';
    
    result->type = VAL_STRING;
    result->data.string_val = new_str;
    
    return 0;
}

/* 字符串替换 */
static int builtin_replace(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 3) {
        set_string_error("REPLACE", "参数数量错误：期望3个参数（源字符串，搜索串，替换串）");
        return -1;
    }
    
    if (!validate_string_arg(&args[0], "REPLACE") || 
        !validate_string_arg(&args[1], "REPLACE") || 
        !validate_string_arg(&args[2], "REPLACE")) {
        return -1;
    }
    
    const char *source = args[0].data.string_val;
    const char *search = args[1].data.string_val;
    const char *replace = args[2].data.string_val;
    
    size_t search_len = strlen(search);
    size_t replace_len = strlen(replace);
    
    if (search_len == 0) {
        /* 搜索串为空，返回原字符串的副本 */
        char *new_str = safe_string_alloc(strlen(source) + 1);
        if (!new_str) {
            set_string_error("REPLACE", "内存分配失败");
            return -1;
        }
        strcpy(new_str, source);
        result->type = VAL_STRING;
        result->data.string_val = new_str;
        return 0;
    }
    
    /* 计算需要替换的次数 */
    size_t count = 0;
    const char *temp = source;
    while ((temp = strstr(temp, search)) != NULL) {
        count++;
        temp += search_len;
    }
    
    if (count == 0) {
        /* 没有找到，返回原字符串的副本 */
        char *new_str = safe_string_alloc(strlen(source) + 1);
        if (!new_str) {
            set_string_error("REPLACE", "内存分配失败");
            return -1;
        }
        strcpy(new_str, source);
        result->type = VAL_STRING;
        result->data.string_val = new_str;
        return 0;
    }
    
    /* 计算新字符串长度 */
    size_t new_len = strlen(source) - count * search_len + count * replace_len;
    
    if (new_len > 1024) { /* 长度限制 */
        set_string_error("REPLACE", "结果字符串过长");
        return -1;
    }
    
    char *new_str = safe_string_alloc(new_len + 1);
    if (!new_str) {
        set_string_error("REPLACE", "内存分配失败");
        return -1;
    }
    
    /* 执行替换 */
    char *dest = new_str;
    const char *src = source;
    
    while ((temp = strstr(src, search)) != NULL) {
        /* 复制搜索串之前的部分 */
        size_t prefix_len = temp - src;
        strncpy(dest, src, prefix_len);
        dest += prefix_len;
        
        /* 复制替换串 */
        strcpy(dest, replace);
        dest += replace_len;
        
        /* 移动源指针 */
        src = temp + search_len;
    }
    
    /* 复制剩余部分 */
    strcpy(dest, src);
    
    result->type = VAL_STRING;
    result->data.string_val = new_str;
    
    return 0;
}

/* ========== 字符串判断函数 ========== */

/* 判断是否以指定字符串开始 */
static int builtin_startswith(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2) {
        set_string_error("STARTSWITH", "参数数量错误：期望2个参数");
        return -1;
    }
    
    if (!validate_string_arg(&args[0], "STARTSWITH") || !validate_string_arg(&args[1], "STARTSWITH")) {
        return -1;
    }
    
    const char *str = args[0].data.string_val;
    const char *prefix = args[1].data.string_val;
    
    size_t prefix_len = strlen(prefix);
    
    result->type = VAL_BOOL;
    if (strlen(str) >= prefix_len && strncmp(str, prefix, prefix_len) == 0) {
        result->data.bool_val = true;
    } else {
        result->data.bool_val = false;
    }
    
    return 0;
}

/* 判断是否以指定字符串结束 */
static int builtin_endswith(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2) {
        set_string_error("ENDSWITH", "参数数量错误：期望2个参数");
        return -1;
    }
    
    if (!validate_string_arg(&args[0], "ENDSWITH") || !validate_string_arg(&args[1], "ENDSWITH")) {
        return -1;
    }
    
    const char *str = args[0].data.string_val;
    const char *suffix = args[1].data.string_val;
    
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    
    result->type = VAL_BOOL;
    if (str_len >= suffix_len && 
        strcmp(str + str_len - suffix_len, suffix) == 0) {
        result->data.bool_val = true;
    } else {
        result->data.bool_val = false;
    }
    
    return 0;
}

/* 判断是否包含指定字符串 */
static int builtin_contains(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2) {
        set_string_error("CONTAINS", "参数数量错误：期望2个参数");
        return -1;
    }
    
    if (!validate_string_arg(&args[0], "CONTAINS") || !validate_string_arg(&args[1], "CONTAINS")) {
        return -1;
    }
    
    const char *str = args[0].data.string_val;
    const char *substr = args[1].data.string_val;
    
    result->type = VAL_BOOL;
    result->data.bool_val = (strstr(str, substr) != NULL);
    
    return 0;
}

/* 判断字符串是否为空 */
static int builtin_isempty(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_string_error("ISEMPTY", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_string_arg(&args[0], "ISEMPTY")) {
        return -1;
    }
    
    const char *str = args[0].data.string_val;
    
    result->type = VAL_BOOL;
    result->data.bool_val = (strlen(str) == 0);
    
    return 0;
}

/* ========== 类型转换函数 ========== */

/* 整数转字符串 */
static int builtin_int_to_string(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_string_error("INT_TO_STRING", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_int_arg(&args[0], "INT_TO_STRING")) {
        return -1;
    }
    
    int32_t value = args[0].data.int_val;
    
    char *new_str = safe_string_alloc(32); /* 足够存放32位整数 */
    if (!new_str) {
        set_string_error("INT_TO_STRING", "内存分配失败");
        return -1;
    }
    
    snprintf(new_str, 32, "%d", value);
    
    result->type = VAL_STRING;
    result->data.string_val = new_str;
    
    return 0;
}

/* 实数转字符串 */
static int builtin_real_to_string(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count < 1 || arg_count > 2) {
        set_string_error("REAL_TO_STRING", "参数数量错误：期望1-2个参数");
        return -1;
    }
    
    if (args[0].type != VAL_REAL && args[0].type != VAL_INT) {
        set_string_error("REAL_TO_STRING", "第一个参数必须是REAL或INT类型");
        return -1;
    }
    
    double value = (args[0].type == VAL_REAL) ? args[0].data.real_val : (double)args[0].data.int_val;
    int precision = 6; /* 默认精度 */
    
    if (arg_count == 2) {
        if (!validate_int_arg(&args[1], "REAL_TO_STRING")) {
            return -1;
        }
        precision = args[1].data.int_val;
        if (precision < 0 || precision > 15) {
            set_string_error("REAL_TO_STRING", "精度必须在0-15之间");
            return -1;
        }
    }
    
    char *new_str = safe_string_alloc(64); /* 足够存放双精度浮点数 */
    if (!new_str) {
        set_string_error("REAL_TO_STRING", "内存分配失败");
        return -1;
    }
    
    snprintf(new_str, 64, "%.*f", precision, value);
    
    result->type = VAL_STRING;
    result->data.string_val = new_str;
    
    return 0;
}

/* 字符串转整数 */
static int builtin_string_to_int(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_string_error("STRING_TO_INT", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_string_arg(&args[0], "STRING_TO_INT")) {
        return -1;
    }
    
    const char *str = args[0].data.string_val;
    char *endptr;
    
    long value = strtol(str, &endptr, 10);
    
    if (endptr == str || *endptr != '\0') {
        set_string_error("STRING_TO_INT", "字符串格式不是有效的整数");
        return -1;
    }
    
    if (value < INT32_MIN || value > INT32_MAX) {
        set_string_error("STRING_TO_INT", "数值超出INT范围");
        return -1;
    }
    
    result->type = VAL_INT;
    result->data.int_val = (int32_t)value;
    
    return 0;
}

/* 字符串转实数 */
static int builtin_string_to_real(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_string_error("STRING_TO_REAL", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_string_arg(&args[0], "STRING_TO_REAL")) {
        return -1;
    }
    
    const char *str = args[0].data.string_val;
    char *endptr;
    
    double value = strtod(str, &endptr);
    
    if (endptr == str || *endptr != '\0') {
        set_string_error("STRING_TO_REAL", "字符串格式不是有效的实数");
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = value;
    
    return 0;
}

/* ========== 辅助函数实现 ========== */

/* 验证字符串参数 */
static bool validate_string_arg(struct vm_value *arg, const char *func_name) {
    if (arg->type != VAL_STRING) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "%s: 参数类型错误，期望STRING类型", func_name);
        set_string_error(func_name, error_msg);
        return false;
    }
    
    if (arg->data.string_val == NULL) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "%s: 字符串参数为NULL", func_name);
        set_string_error(func_name, error_msg);
        return false;
    }
    
    return true;
}

/* 验证整数参数 */
static bool validate_int_arg(struct vm_value *arg, const char *func_name) {
    if (arg->type != VAL_INT) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "%s: 参数类型错误，期望INT类型", func_name);
        set_string_error(func_name, error_msg);
        return false;
    }
    
    return true;
}

/* 设置字符串错误 */
static void set_string_error(const char *func_name, const char *error) {
    char full_error[512];
    snprintf(full_error, sizeof(full_error), "StringLib.%s: %s", func_name, error);
    libmgr_set_error(full_error);
}

/* 安全字符串分配 */
static char* safe_string_alloc(size_t size) {
    if (size == 0 || size > 1024) {
        return NULL;
    }
    
    char *str = (char*)mmgr_alloc_string_with_length(NULL, size);
    if (str) {
        memset(str, 0, size);
    }
    
    return str;
}

/* 安全字符串复制 */
static int safe_string_copy(char *dest, const char *src, size_t dest_size) {
    if (!dest || !src || dest_size == 0) {
        return -1;
    }
    
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
    
    return 0;
}

/* ========== 库初始化函数 ========== */

/* 初始化内置字符串库 */
int builtin_string_init(void) {
    if (!libmgr_is_initialized()) {
        return -1;
    }
    
    /* 创建字符串库 */
    library_info_t *string_lib = libmgr_find_library("StringLib");
    if (!string_lib) {
        libmgr_set_error("StringLib库未找到，请先调用libmgr_init_builtin_string()");
        return -1;
    }
    
    /* 获取类型信息 */
    type_info_t *string_type = create_basic_type(TYPE_STRING_ID);
    type_info_t *int_type = create_basic_type(TYPE_INT_ID);
    type_info_t *bool_type = create_basic_type(TYPE_BOOL_ID);
    type_info_t *real_type = create_basic_type(TYPE_REAL_ID);
    
    if (!string_type || !int_type || !bool_type || !real_type) {
        libmgr_set_error("无法创建基础类型");
        return -1;
    }
    
    /* 注册基本字符串函数 */
    if (!libmgr_register_builtin_function("StringLib", "Len", builtin_strlen, int_type, &string_type, 1)) {
        libmgr_set_error("注册Len函数失败");
        return -1;
    }
    
    type_info_t *string_params[2] = {string_type, string_type};
    if (!libmgr_register_builtin_function("StringLib", "Concat", builtin_concat, string_type, string_params, 2) ||
        !libmgr_register_builtin_function("StringLib", "Compare", builtin_strcmp, int_type, string_params, 2)) {
        libmgr_set_error("注册字符串基本函数失败");
        return -1;
    }
    
    /* 注册子字符串函数 */
    type_info_t *substring_params[3] = {string_type, int_type, int_type};
    if (!libmgr_register_builtin_function("StringLib", "Substring", builtin_substring, string_type, substring_params, 3)) {
        libmgr_set_error("注册Substring函数失败");
        return -1;
    }
    
    /* 注册查找函数 */
    if (!libmgr_register_builtin_function("StringLib", "IndexOf", builtin_indexof, int_type, string_params, 2) ||
        !libmgr_register_builtin_function("StringLib", "LastIndexOf", builtin_lastindexof, int_type, string_params, 2)) {
        libmgr_set_error("注册字符串查找函数失败");
        return -1;
    }
    
    /* 注册转换函数 */
    if (!libmgr_register_builtin_function("StringLib", "ToUpper", builtin_toupper, string_type, &string_type, 1) ||
        !libmgr_register_builtin_function("StringLib", "ToLower", builtin_tolower, string_type, &string_type, 1) ||
        !libmgr_register_builtin_function("StringLib", "Trim", builtin_trim, string_type, &string_type, 1)) {
        libmgr_set_error("注册字符串转换函数失败");
        return -1;
    }
    
    /* 注册替换函数 */
    type_info_t *replace_params[3] = {string_type, string_type, string_type};
    if (!libmgr_register_builtin_function("StringLib", "Replace", builtin_replace, string_type, replace_params, 3)) {
        libmgr_set_error("注册Replace函数失败");
        return -1;
    }
    
    /* 注册判断函数 */
    if (!libmgr_register_builtin_function("StringLib", "StartsWith", builtin_startswith, bool_type, string_params, 2) ||
        !libmgr_register_builtin_function("StringLib", "EndsWith", builtin_endswith, bool_type, string_params, 2) ||
        !libmgr_register_builtin_function("StringLib", "Contains", builtin_contains, bool_type, string_params, 2) ||
        !libmgr_register_builtin_function("StringLib", "IsEmpty", builtin_isempty, bool_type, &string_type, 1)) {
        libmgr_set_error("注册字符串判断函数失败");
        return -1;
    }
    
    /* 注册类型转换函数 */
    if (!libmgr_register_builtin_function("StringLib", "IntToString", builtin_int_to_string, string_type, &int_type, 1) ||
        !libmgr_register_builtin_function("StringLib", "StringToInt", builtin_string_to_int, int_type, &string_type, 1) ||
        !libmgr_register_builtin_function("StringLib", "StringToReal", builtin_string_to_real, real_type, &string_type, 1)) {
        libmgr_set_error("注册类型转换函数失败");
        return -1;
    }
    
    /* 注册实数转字符串函数（支持精度参数） */
    type_info_t *real_to_str_params1[1] = {real_type};
    type_info_t *real_to_str_params2[2] = {real_type, int_type};
    if (!libmgr_register_builtin_function("StringLib", "RealToString", builtin_real_to_string, string_type, real_to_str_params1, 1)) {
        /* 尝试注册重载版本 - 简化实现中可能不支持 */
        if (!libmgr_register_builtin_function("StringLib", "RealToStringWithPrecision", builtin_real_to_string, string_type, real_to_str_params2, 2)) {
            libmgr_set_error("注册RealToString函数失败");
            return -1;
        }
    }
    
    return 0;
}

/* 获取字符串库版本信息 */
const char* builtin_string_get_version(void) {
    return "1.0.0";
}

/* 获取字符串库描述 */
const char* builtin_string_get_description(void) {
    return "IEC61131 ST内置字符串处理库，提供字符串操作、查找、转换、判断等功能";
}

/* 获取支持的函数列表 */
const char** builtin_string_get_function_list(void) {
    static const char* functions[] = {
        "Len", "Concat", "Compare", "Substring", 
        "IndexOf", "LastIndexOf",
        "ToUpper", "ToLower", "Trim", "Replace",
        "StartsWith", "EndsWith", "Contains", "IsEmpty",
        "IntToString", "RealToString", "RealToStringWithPrecision",
        "StringToInt", "StringToReal",
        NULL
    };
    
    return functions;
}

/* 字符串库功能演示 */
int builtin_string_demo(void) {
    printf("=== StringLib功能演示 ===\n");
    
    /* 模拟一些函数调用 */
    struct vm_value args[3];
    struct vm_value result;
    
    /* 演示字符串长度 */
    args[0].type = VAL_STRING;
    args[0].data.string_val = "Hello World";
    
    if (builtin_strlen(args, 1, &result) == 0) {
        printf("Len(\"Hello World\") = %d\n", result.data.int_val);
    }
    
    /* 演示字符串连接 */
    args[0].type = VAL_STRING;
    args[0].data.string_val = "Hello";
    args[1].type = VAL_STRING;
    args[1].data.string_val = " World";
    
    if (builtin_concat(args, 2, &result) == 0) {
        printf("Concat(\"Hello\", \" World\") = \"%s\"\n", result.data.string_val);
    }
    
    /* 演示字符串查找 */
    args[0].type = VAL_STRING;
    args[0].data.string_val = "Hello World";
    args[1].type = VAL_STRING;
    args[1].data.string_val = "World";
    
    if (builtin_indexof(args, 2, &result) == 0) {
        printf("IndexOf(\"Hello World\", \"World\") = %d\n", result.data.int_val);
    }
    
    printf("=========================\n");
    
    return 0;
}
