/*
 * IEC61131 内置数学库实现
 * 提供标准数学函数和常量
 */

#include "libmgr.h"
#include "symbol_table.h"
#include "mmgr.h"
#include "vm.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 数学常量定义 */
#define MATH_PI     3.14159265358979323846
#define MATH_E      2.71828182845904523536
#define MATH_LN2    0.69314718055994530942
#define MATH_LN10   2.30258509299404568402
#define MATH_SQRT2  1.41421356237309504880
#define MATH_SQRT_PI 1.77245385090551602730

/* 内部辅助函数声明 */
static bool validate_real_arg(struct vm_value *arg, const char *func_name);
static bool validate_int_arg(struct vm_value *arg, const char *func_name);
static void set_math_error(const char *func_name, const char *error);

/* ========== 基本数学函数实现 ========== */

/* 绝对值函数 - 实数版本 */
static int builtin_abs_real(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("ABS", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "ABS")) {
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = fabs(args[0].data.real_val);
    
    return 0;
}

/* 绝对值函数 - 整数版本 */
static int builtin_abs_int(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("ABS_INT", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_int_arg(&args[0], "ABS_INT")) {
        return -1;
    }
    
    result->type = VAL_INT;
    result->data.int_val = abs(args[0].data.int_val);
    
    return 0;
}

/* 正弦函数 */
static int builtin_sin(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("SIN", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "SIN")) {
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = sin(args[0].data.real_val);
    
    return 0;
}

/* 余弦函数 */
static int builtin_cos(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("COS", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "COS")) {
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = cos(args[0].data.real_val);
    
    return 0;
}

/* 正切函数 */
static int builtin_tan(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("TAN", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "TAN")) {
        return -1;
    }
    
    double angle = args[0].data.real_val;
    
    /* 检查特殊值（π/2的奇数倍） */
    double normalized = fmod(angle, MATH_PI);
    if (fabs(normalized - MATH_PI/2) < 1e-10 || fabs(normalized + MATH_PI/2) < 1e-10) {
        set_math_error("TAN", "参数为π/2的奇数倍，结果为无穷大");
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = tan(angle);
    
    return 0;
}

/* 反正弦函数 */
static int builtin_asin(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("ASIN", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "ASIN")) {
        return -1;
    }
    
    double value = args[0].data.real_val;
    if (value < -1.0 || value > 1.0) {
        set_math_error("ASIN", "参数超出定义域[-1, 1]");
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = asin(value);
    
    return 0;
}

/* 反余弦函数 */
static int builtin_acos(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("ACOS", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "ACOS")) {
        return -1;
    }
    
    double value = args[0].data.real_val;
    if (value < -1.0 || value > 1.0) {
        set_math_error("ACOS", "参数超出定义域[-1, 1]");
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = acos(value);
    
    return 0;
}

/* 反正切函数 */
static int builtin_atan(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("ATAN", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "ATAN")) {
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = atan(args[0].data.real_val);
    
    return 0;
}

/* 两参数反正切函数 */
static int builtin_atan2(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2) {
        set_math_error("ATAN2", "参数数量错误：期望2个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "ATAN2") || !validate_real_arg(&args[1], "ATAN2")) {
        return -1;
    }
    
    double y = args[0].data.real_val;
    double x = args[1].data.real_val;
    
    if (x == 0.0 && y == 0.0) {
        set_math_error("ATAN2", "参数(0, 0)未定义");
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = atan2(y, x);
    
    return 0;
}

/* ========== 指数和对数函数 ========== */

/* 平方根函数 */
static int builtin_sqrt(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("SQRT", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "SQRT")) {
        return -1;
    }
    
    double value = args[0].data.real_val;
    if (value < 0.0) {
        set_math_error("SQRT", "负数开平方根");
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = sqrt(value);
    
    return 0;
}

/* 立方根函数 */
static int builtin_cbrt(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("CBRT", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "CBRT")) {
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = cbrt(args[0].data.real_val);
    
    return 0;
}

/* 幂函数 */
static int builtin_power(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2) {
        set_math_error("POWER", "参数数量错误：期望2个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "POWER") || !validate_real_arg(&args[1], "POWER")) {
        return -1;
    }
    
    double base = args[0].data.real_val;
    double exponent = args[1].data.real_val;
    
    /* 检查特殊情况 */
    if (base == 0.0 && exponent <= 0.0) {
        set_math_error("POWER", "0的非正数次幂未定义");
        return -1;
    }
    
    if (base < 0.0 && floor(exponent) != exponent) {
        set_math_error("POWER", "负数的非整数次幂未定义");
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = pow(base, exponent);
    
    /* 检查结果是否有效 */
    if (isnan(result->data.real_val) || isinf(result->data.real_val)) {
        set_math_error("POWER", "计算结果溢出或未定义");
        return -1;
    }
    
    return 0;
}

/* 指数函数 e^x */
static int builtin_exp(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("EXP", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "EXP")) {
        return -1;
    }
    
    double value = args[0].data.real_val;
    if (value > 700.0) {  // 防止溢出
        set_math_error("EXP", "指数过大，结果溢出");
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = exp(value);
    
    return 0;
}

/* 以2为底的指数函数 */
static int builtin_exp2(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("EXP2", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "EXP2")) {
        return -1;
    }
    
    double value = args[0].data.real_val;
    if (value > 1000.0) {  // 防止溢出
        set_math_error("EXP2", "指数过大，结果溢出");
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = exp2(value);
    
    return 0;
}

/* 自然对数函数 */
static int builtin_log(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("LOG", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "LOG")) {
        return -1;
    }
    
    double value = args[0].data.real_val;
    if (value <= 0.0) {
        set_math_error("LOG", "对数的参数必须为正数");
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = log(value);
    
    return 0;
}

/* 以10为底的对数函数 */
static int builtin_log10(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("LOG10", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "LOG10")) {
        return -1;
    }
    
    double value = args[0].data.real_val;
    if (value <= 0.0) {
        set_math_error("LOG10", "对数的参数必须为正数");
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = log10(value);
    
    return 0;
}

/* 以2为底的对数函数 */
static int builtin_log2(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("LOG2", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "LOG2")) {
        return -1;
    }
    
    double value = args[0].data.real_val;
    if (value <= 0.0) {
        set_math_error("LOG2", "对数的参数必须为正数");
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = log2(value);
    
    return 0;
}

/* ========== 取整和舍入函数 ========== */

/* 向下取整函数 */
static int builtin_floor(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("FLOOR", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "FLOOR")) {
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = floor(args[0].data.real_val);
    
    return 0;
}

/* 向上取整函数 */
static int builtin_ceil(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("CEIL", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "CEIL")) {
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = ceil(args[0].data.real_val);
    
    return 0;
}

/* 四舍五入函数 */
static int builtin_round(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("ROUND", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "ROUND")) {
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = round(args[0].data.real_val);
    
    return 0;
}

/* 截断函数 */
static int builtin_trunc(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("TRUNC", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "TRUNC")) {
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = trunc(args[0].data.real_val);
    
    return 0;
}

/* 小数部分函数 */
static int builtin_frac(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("FRAC", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "FRAC")) {
        return -1;
    }
    
    double value = args[0].data.real_val;
    result->type = VAL_REAL;
    result->data.real_val = value - trunc(value);
    
    return 0;
}

/* ========== 比较和最值函数 ========== */

/* 最大值函数 */
static int builtin_max(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2) {
        set_math_error("MAX", "参数数量错误：期望2个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "MAX") || !validate_real_arg(&args[1], "MAX")) {
        return -1;
    }
    
    double a = args[0].data.real_val;
    double b = args[1].data.real_val;
    
    result->type = VAL_REAL;
    result->data.real_val = (a > b) ? a : b;
    
    return 0;
}

/* 最小值函数 */
static int builtin_min(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2) {
        set_math_error("MIN", "参数数量错误：期望2个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "MIN") || !validate_real_arg(&args[1], "MIN")) {
        return -1;
    }
    
    double a = args[0].data.real_val;
    double b = args[1].data.real_val;
    
    result->type = VAL_REAL;
    result->data.real_val = (a < b) ? a : b;
    
    return 0;
}

/* 符号函数 */
static int builtin_sign(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("SIGN", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "SIGN")) {
        return -1;
    }
    
    double value = args[0].data.real_val;
    
    result->type = VAL_INT;
    if (value > 0.0) {
        result->data.int_val = 1;
    } else if (value < 0.0) {
        result->data.int_val = -1;
    } else {
        result->data.int_val = 0;
    }
    
    return 0;
}

/* ========== 角度转换函数 ========== */

/* 弧度转角度 */
static int builtin_rad_to_deg(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("RAD_TO_DEG", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "RAD_TO_DEG")) {
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = args[0].data.real_val * 180.0 / MATH_PI;
    
    return 0;
}

/* 角度转弧度 */
static int builtin_deg_to_rad(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("DEG_TO_RAD", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "DEG_TO_RAD")) {
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = args[0].data.real_val * MATH_PI / 180.0;
    
    return 0;
}

/* ========== 特殊数学函数 ========== */

/* 双曲正弦函数 */
static int builtin_sinh(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("SINH", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "SINH")) {
        return -1;
    }
    
    double value = args[0].data.real_val;
    if (fabs(value) > 700.0) {  // 防止溢出
        set_math_error("SINH", "参数过大，结果溢出");
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = sinh(value);
    
    return 0;
}

/* 双曲余弦函数 */
static int builtin_cosh(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("COSH", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "COSH")) {
        return -1;
    }
    
    double value = args[0].data.real_val;
    if (fabs(value) > 700.0) {  // 防止溢出
        set_math_error("COSH", "参数过大，结果溢出");
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = cosh(value);
    
    return 0;
}

/* 双曲正切函数 */
static int builtin_tanh(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_math_error("TANH", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "TANH")) {
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = tanh(args[0].data.real_val);
    
    return 0;
}

/* 模运算（浮点数版本） */
static int builtin_fmod(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2) {
        set_math_error("FMOD", "参数数量错误：期望2个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "FMOD") || !validate_real_arg(&args[1], "FMOD")) {
        return -1;
    }
    
    double dividend = args[0].data.real_val;
    double divisor = args[1].data.real_val;
    
    if (divisor == 0.0) {
        set_math_error("FMOD", "除数不能为零");
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = fmod(dividend, divisor);
    
    return 0;
}

/* 余数函数 */
static int builtin_remainder(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2) {
        set_math_error("REMAINDER", "参数数量错误：期望2个参数");
        return -1;
    }
    
    if (!validate_real_arg(&args[0], "REMAINDER") || !validate_real_arg(&args[1], "REMAINDER")) {
        return -1;
    }
    
    double dividend = args[0].data.real_val;
    double divisor = args[1].data.real_val;
    
    if (divisor == 0.0) {
        set_math_error("REMAINDER", "除数不能为零");
        return -1;
    }
    
    result->type = VAL_REAL;
    result->data.real_val = remainder(dividend, divisor);
    
    return 0;
}

/* ========== 辅助函数实现 ========== */

/* 验证实数参数 */
static bool validate_real_arg(struct vm_value *arg, const char *func_name) {
    if (arg->type != VAL_REAL && arg->type != VAL_INT) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "%s: 参数类型错误，期望REAL或INT类型", func_name);
        set_math_error(func_name, error_msg);
        return false;
    }
    
    /* 如果是整数，转换为实数 */
    if (arg->type == VAL_INT) {
        double real_val = (double)arg->data.int_val;
        arg->type = VAL_REAL;
        arg->data.real_val = real_val;
    }
    
    /* 检查是否为有效数值 */
    if (isnan(arg->data.real_val)) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "%s: 参数为NaN（非数值）", func_name);
        set_math_error(func_name, error_msg);
        return false;
    }
    
    return true;
}

/* 验证整数参数 */
static bool validate_int_arg(struct vm_value *arg, const char *func_name) {
    if (arg->type != VAL_INT) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "%s: 参数类型错误，期望INT类型", func_name);
        set_math_error(func_name, error_msg);
        return false;
    }
    
    return true;
}

/* 设置数学错误 */
static void set_math_error(const char *func_name, const char *error) {
    char full_error[512];
    snprintf(full_error, sizeof(full_error), "MathLib.%s: %s", func_name, error);
    libmgr_set_error(full_error);
}

/* ========== 库初始化函数 ========== */

/* 初始化内置数学库 */
int builtin_math_init(void) {
    if (!libmgr_is_initialized()) {
        return -1;
    }
    
    /* 创建数学库 */
    library_info_t *math_lib = libmgr_find_library("MathLib");
    if (!math_lib) {
        libmgr_set_error("MathLib库未找到，请先调用libmgr_init_builtin_math()");
        return -1;
    }
    
    /* 获取类型信息 */
    type_info_t *real_type = create_basic_type(TYPE_REAL_ID);
    type_info_t *int_type = create_basic_type(TYPE_INT_ID);
    
    if (!real_type || !int_type) {
        libmgr_set_error("无法创建基础类型");
        return -1;
    }
    
    /* 注册三角函数 */
    if (!libmgr_register_builtin_function("MathLib", "Sin", builtin_sin, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "Cos", builtin_cos, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "Tan", builtin_tan, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "Asin", builtin_asin, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "Acos", builtin_acos, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "Atan", builtin_atan, real_type, &real_type, 1)) {
        libmgr_set_error("注册三角函数失败");
        return -1;
    }
    
    /* 注册反正切函数（两参数版本） */
    type_info_t *real_params[2] = {real_type, real_type};
    if (!libmgr_register_builtin_function("MathLib", "Atan2", builtin_atan2, real_type, real_params, 2)) {
        libmgr_set_error("注册Atan2函数失败");
        return -1;
    }
    
    /* 注册指数和对数函数 */
    if (!libmgr_register_builtin_function("MathLib", "Sqrt", builtin_sqrt, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "Cbrt", builtin_cbrt, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "Power", builtin_power, real_type, real_params, 2) ||
        !libmgr_register_builtin_function("MathLib", "Exp", builtin_exp, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "Exp2", builtin_exp2, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "Log", builtin_log, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "Log10", builtin_log10, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "Log2", builtin_log2, real_type, &real_type, 1)) {
        libmgr_set_error("注册指数对数函数失败");
        return -1;
    }
    
    /* 注册取整函数 */
    if (!libmgr_register_builtin_function("MathLib", "Floor", builtin_floor, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "Ceil", builtin_ceil, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "Round", builtin_round, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "Trunc", builtin_trunc, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "Frac", builtin_frac, real_type, &real_type, 1)) {
        libmgr_set_error("注册取整函数失败");
        return -1;
    }
    
    /* 注册比较和最值函数 */
    if (!libmgr_register_builtin_function("MathLib", "Max", builtin_max, real_type, real_params, 2) ||
        !libmgr_register_builtin_function("MathLib", "Min", builtin_min, real_type, real_params, 2) ||
        !libmgr_register_builtin_function("MathLib", "Sign", builtin_sign, int_type, &real_type, 1)) {
        libmgr_set_error("注册比较函数失败");
        return -1;
    }
    
    /* 注册绝对值函数 */
    if (!libmgr_register_builtin_function("MathLib", "Abs", builtin_abs_real, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "AbsInt", builtin_abs_int, int_type, &int_type, 1)) {
        libmgr_set_error("注册绝对值函数失败");
        return -1;
    }
    
    /* 注册角度转换函数 */
    if (!libmgr_register_builtin_function("MathLib", "RadToDeg", builtin_rad_to_deg, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "DegToRad", builtin_deg_to_rad, real_type, &real_type, 1)) {
        libmgr_set_error("注册角度转换函数失败");
        return -1;
    }
    
    /* 注册双曲函数 */
    if (!libmgr_register_builtin_function("MathLib", "Sinh", builtin_sinh, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "Cosh", builtin_cosh, real_type, &real_type, 1) ||
        !libmgr_register_builtin_function("MathLib", "Tanh", builtin_tanh, real_type, &real_type, 1)) {
        libmgr_set_error("注册双曲函数失败");
        return -1;
    }
    
    /* 注册模运算函数 */
    if (!libmgr_register_builtin_function("MathLib", "FMod", builtin_fmod, real_type, real_params, 2) ||
        !libmgr_register_builtin_function("MathLib", "Remainder", builtin_remainder, real_type, real_params, 2)) {
        libmgr_set_error("注册模运算函数失败");
        return -1;
    }
    
    /* 注册数学常量 */
    double pi_value = MATH_PI;
    double e_value = MATH_E;
    double ln2_value = MATH_LN2;
    double ln10_value = MATH_LN10;
    double sqrt2_value = MATH_SQRT2;
    double sqrt_pi_value = MATH_SQRT_PI;
    
    if (!libmgr_register_builtin_constant("MathLib", "PI", real_type, &pi_value) ||
        !libmgr_register_builtin_constant("MathLib", "E", real_type, &e_value) ||
        !libmgr_register_builtin_constant("MathLib", "LN2", real_type, &ln2_value) ||
        !libmgr_register_builtin_constant("MathLib", "LN10", real_type, &ln10_value) ||
        !libmgr_register_builtin_constant("MathLib", "SQRT2", real_type, &sqrt2_value) ||
        !libmgr_register_builtin_constant("MathLib", "SQRT_PI", real_type, &sqrt_pi_value)) {
        libmgr_set_error("注册数学常量失败");
        return -1;
    }
    
    return 0;
}

/* 获取数学库版本信息 */
const char* builtin_math_get_version(void) {
    return "1.0.0";
}

/* 获取数学库描述 */
const char* builtin_math_get_description(void) {
    return "IEC61131 ST内置数学函数库，提供三角函数、指数对数、取整舍入、比较最值等功能";
}

/* 获取支持的函数列表 */
const char** builtin_math_get_function_list(void) {
    static const char* functions[] = {
        "Sin", "Cos", "Tan", "Asin", "Acos", "Atan", "Atan2",
        "Sqrt", "Cbrt", "Power", "Exp", "Exp2", "Log", "Log10", "Log2",
        "Floor", "Ceil", "Round", "Trunc", "Frac",
        "Max", "Min", "Sign", "Abs", "AbsInt",
        "RadToDeg", "DegToRad",
        "Sinh", "Cosh", "Tanh",
        "FMod", "Remainder",
        NULL
    };
    
    return functions;
}

/* 获取支持的常量列表 */
const char** builtin_math_get_constant_list(void) {
    static const char* constants[] = {
        "PI", "E", "LN2", "LN10", "SQRT2", "SQRT_PI",
        NULL
    };
    
    return constants;
}
