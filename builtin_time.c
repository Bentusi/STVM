#include "builtin_time.h"
#include "mmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* 时间库内部状态 */
static struct {
    bool is_initialized;
    time_t start_time;              // 系统启动时间
    int64_t system_tick_base;       // 系统tick基准
} g_time_lib = {0};

/* 定时器管理 */
#define MAX_TIMERS 32
typedef struct {
    bool active;
    int64_t start_time;
    int64_t duration;
    bool expired;
} builtin_timer_t;

static builtin_timer_t g_timers[MAX_TIMERS];
static uint32_t g_timer_count = 0;

/* 时间常量 */
#define MS_PER_SECOND    1000LL
#define MS_PER_MINUTE    (60LL * MS_PER_SECOND)
#define MS_PER_HOUR      (60LL * MS_PER_MINUTE)
#define MS_PER_DAY       (24LL * MS_PER_HOUR)

/* 获取当前时间戳（毫秒） */
static int64_t get_current_timestamp_ms(void) {
    time_t now = time(NULL);
    return (int64_t)now * MS_PER_SECOND;
}

/* 时间库初始化 */
int builtin_time_init(void) {
    if (g_time_lib.is_initialized) {
        return 0;
    }
    
    g_time_lib.start_time = time(NULL);
    g_time_lib.system_tick_base = get_current_timestamp_ms();
    
    /* 初始化定时器 */
    memset(g_timers, 0, sizeof(g_timers));
    g_timer_count = 0;
    
    g_time_lib.is_initialized = true;
    return 0;
}

/* 时间库清理 */
void builtin_time_cleanup(void) {
    if (!g_time_lib.is_initialized) {
        return;
    }
    
    memset(&g_time_lib, 0, sizeof(g_time_lib));
    memset(g_timers, 0, sizeof(g_timers));
    g_timer_count = 0;
}

/* ========== 时间创建函数 ========== */

/* 创建TIME类型（毫秒） */
int time_create_time_ms(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1 || args[0].type != VAL_INT) {
        return -1;
    }
    
    result->type = VAL_INT;  // TIME类型用INT表示（毫秒）
    result->data.int_val = args[0].data.int_val;
    return 0;
}

/* 创建TIME类型（秒） */
int time_create_time_s(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1 || args[0].type != VAL_INT) {
        return -1;
    }
    
    result->type = VAL_INT;
    result->data.int_val = args[0].data.int_val * MS_PER_SECOND;
    return 0;
}

/* 创建DATE类型 */
int time_create_date(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 3 || 
        args[0].type != VAL_INT || args[1].type != VAL_INT || args[2].type != VAL_INT) {
        return -1;
    }
    
    st_date_t date = {
        .year = (uint16_t)args[0].data.int_val,
        .month = (uint8_t)args[1].data.int_val,
        .day = (uint8_t)args[2].data.int_val
    };
    
    if (!validate_date(&date)) {
        return -1;
    }
    
    /* 简化：将日期编码为整数 YYYYMMDD */
    result->type = VAL_INT;
    result->data.int_val = date.year * 10000 + date.month * 100 + date.day;
    return 0;
}

/* 创建TOD类型 */
int time_create_tod(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count < 3 || arg_count > 4 ||
        args[0].type != VAL_INT || args[1].type != VAL_INT || args[2].type != VAL_INT) {
        return -1;
    }
    
    st_tod_t tod = {
        .hour = (uint8_t)args[0].data.int_val,
        .minute = (uint8_t)args[1].data.int_val,
        .second = (uint8_t)args[2].data.int_val,
        .millisecond = (arg_count == 4) ? (uint16_t)args[3].data.int_val : 0
    };
    
    if (!validate_tod(&tod)) {
        return -1;
    }
    
    /* 将TOD转换为从00:00:00开始的毫秒数 */
    result->type = VAL_INT;
    result->data.int_val = tod.hour * MS_PER_HOUR + 
                          tod.minute * MS_PER_MINUTE + 
                          tod.second * MS_PER_SECOND + 
                          tod.millisecond;
    return 0;
}

/* 创建DT类型 */
int time_create_dt(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2 || args[0].type != VAL_INT || args[1].type != VAL_INT) {
        return -1;
    }
    
    /* 简化：DT由DATE和TOD组合而成 */
    /* 实际实现中应该转换为时间戳 */
    result->type = VAL_INT;
    result->data.int_val = args[0].data.int_val;  // 使用DATE部分
    return 0;
}

/* ========== 时间获取函数 ========== */

/* 获取当前系统时间 */
int time_get_current_time(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 0) {
        return -1;
    }
    
    int64_t current_ms = get_current_timestamp_ms();
    int64_t elapsed_ms = current_ms - g_time_lib.system_tick_base;
    
    result->type = VAL_INT;
    result->data.int_val = (int32_t)(elapsed_ms % (24 * MS_PER_HOUR));  // 当日时间
    return 0;
}

/* 获取当前日期 */
int time_get_current_date(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 0) {
        return -1;
    }
    
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    
    result->type = VAL_INT;
    result->data.int_val = (tm_info->tm_year + 1900) * 10000 + 
                          (tm_info->tm_mon + 1) * 100 + 
                          tm_info->tm_mday;
    return 0;
}

/* 获取当前时刻 */
int time_get_current_tod(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 0) {
        return -1;
    }
    
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    
    result->type = VAL_INT;
    result->data.int_val = tm_info->tm_hour * MS_PER_HOUR + 
                          tm_info->tm_min * MS_PER_MINUTE + 
                          tm_info->tm_sec * MS_PER_SECOND;
    return 0;
}

/* 获取当前日期时间 */
int time_get_current_dt(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 0) {
        return -1;
    }
    
    result->type = VAL_INT;
    result->data.int_val = (int32_t)time(NULL);  // 简化为Unix时间戳
    return 0;
}

/* 获取系统tick */
int time_get_system_tick(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 0) {
        return -1;
    }
    
    int64_t current_ms = get_current_timestamp_ms();
    result->type = VAL_INT;
    result->data.int_val = (int32_t)(current_ms - g_time_lib.system_tick_base);
    return 0;
}

/* ========== 时间转换函数 ========== */

/* 转换为毫秒 */
int time_to_milliseconds(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1 || args[0].type != VAL_INT) {
        return -1;
    }
    
    result->type = VAL_INT;
    result->data.int_val = args[0].data.int_val;  // 已经是毫秒
    return 0;
}

/* 转换为秒 */
int time_to_seconds(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1 || args[0].type != VAL_INT) {
        return -1;
    }
    
    result->type = VAL_INT;
    result->data.int_val = args[0].data.int_val / MS_PER_SECOND;
    return 0;
}

/* 转换为分钟 */
int time_to_minutes(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1 || args[0].type != VAL_INT) {
        return -1;
    }
    
    result->type = VAL_INT;
    result->data.int_val = args[0].data.int_val / MS_PER_MINUTE;
    return 0;
}

/* 转换为小时 */
int time_to_hours(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1 || args[0].type != VAL_INT) {
        return -1;
    }
    
    result->type = VAL_INT;
    result->data.int_val = args[0].data.int_val / MS_PER_HOUR;
    return 0;
}

/* 转换为天 */
int time_to_days(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1 || args[0].type != VAL_INT) {
        return -1;
    }
    
    result->type = VAL_INT;
    result->data.int_val = args[0].data.int_val / MS_PER_DAY;
    return 0;
}

/* ========== 时间算术运算 ========== */

/* 时间加法 */
int time_add_time(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2 || args[0].type != VAL_INT || args[1].type != VAL_INT) {
        return -1;
    }
    
    result->type = VAL_INT;
    result->data.int_val = args[0].data.int_val + args[1].data.int_val;
    return 0;
}

/* 时间减法 */
int time_sub_time(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2 || args[0].type != VAL_INT || args[1].type != VAL_INT) {
        return -1;
    }
    
    result->type = VAL_INT;
    result->data.int_val = args[0].data.int_val - args[1].data.int_val;
    return 0;
}

/* 时间乘法 */
int time_mul_time(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2 || args[0].type != VAL_INT || args[1].type != VAL_INT) {
        return -1;
    }
    
    result->type = VAL_INT;
    result->data.int_val = args[0].data.int_val * args[1].data.int_val;
    return 0;
}

/* 时间除法 */
int time_div_time(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2 || args[0].type != VAL_INT || args[1].type != VAL_INT) {
        return -1;
    }
    
    if (args[1].data.int_val == 0) {
        return -1;  // 除零错误
    }
    
    result->type = VAL_INT;
    result->data.int_val = args[0].data.int_val / args[1].data.int_val;
    return 0;
}

/* ========== 时间比较函数 ========== */

/* 时间相等 */
int time_eq(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2 || args[0].type != VAL_INT || args[1].type != VAL_INT) {
        return -1;
    }
    
    result->type = VAL_BOOL;
    result->data.bool_val = (args[0].data.int_val == args[1].data.int_val);
    return 0;
}

/* 时间不等 */
int time_ne(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2 || args[0].type != VAL_INT || args[1].type != VAL_INT) {
        return -1;
    }
    
    result->type = VAL_BOOL;
    result->data.bool_val = (args[0].data.int_val != args[1].data.int_val);
    return 0;
}

/* 时间小于 */
int time_lt(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2 || args[0].type != VAL_INT || args[1].type != VAL_INT) {
        return -1;
    }
    
    result->type = VAL_BOOL;
    result->data.bool_val = (args[0].data.int_val < args[1].data.int_val);
    return 0;
}

/* 时间小于等于 */
int time_le(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2 || args[0].type != VAL_INT || args[1].type != VAL_INT) {
        return -1;
    }
    
    result->type = VAL_BOOL;
    result->data.bool_val = (args[0].data.int_val <= args[1].data.int_val);
    return 0;
}

/* 时间大于 */
int time_gt(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2 || args[0].type != VAL_INT || args[1].type != VAL_INT) {
        return -1;
    }
    
    result->type = VAL_BOOL;
    result->data.bool_val = (args[0].data.int_val > args[1].data.int_val);
    return 0;
}

/* 时间大于等于 */
int time_ge(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2 || args[0].type != VAL_INT || args[1].type != VAL_INT) {
        return -1;
    }
    
    result->type = VAL_BOOL;
    result->data.bool_val = (args[0].data.int_val >= args[1].data.int_val);
    return 0;
}

/* ========== 定时器函数 ========== */

/* 启动定时器 */
int timer_start(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1 || args[0].type != VAL_INT || g_timer_count >= MAX_TIMERS) {
        return -1;
    }
    
    uint32_t timer_id = g_timer_count++;
    g_timers[timer_id].active = true;
    g_timers[timer_id].start_time = get_current_timestamp_ms();
    g_timers[timer_id].duration = args[0].data.int_val;
    g_timers[timer_id].expired = false;
    
    result->type = VAL_INT;
    result->data.int_val = timer_id;
    return 0;
}

/* 停止定时器 */
int timer_stop(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1 || args[0].type != VAL_INT) {
        return -1;
    }
    
    uint32_t timer_id = args[0].data.int_val;
    if (timer_id >= g_timer_count) {
        return -1;
    }
    
    g_timers[timer_id].active = false;
    
    result->type = VAL_BOOL;
    result->data.bool_val = true;
    return 0;
}

/* 重置定时器 */
int timer_reset(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1 || args[0].type != VAL_INT) {
        return -1;
    }
    
    uint32_t timer_id = args[0].data.int_val;
    if (timer_id >= g_timer_count) {
        return -1;
    }
    
    g_timers[timer_id].start_time = get_current_timestamp_ms();
    g_timers[timer_id].expired = false;
    
    result->type = VAL_BOOL;
    result->data.bool_val = true;
    return 0;
}

/* 获取定时器已过时间 */
int timer_elapsed(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1 || args[0].type != VAL_INT) {
        return -1;
    }
    
    uint32_t timer_id = args[0].data.int_val;
    if (timer_id >= g_timer_count || !g_timers[timer_id].active) {
        return -1;
    }
    
    int64_t elapsed = get_current_timestamp_ms() - g_timers[timer_id].start_time;
    
    result->type = VAL_INT;
    result->data.int_val = (int32_t)elapsed;
    return 0;
}

/* 检查定时器是否到期 */
int timer_is_expired(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1 || args[0].type != VAL_INT) {
        return -1;
    }
    
    uint32_t timer_id = args[0].data.int_val;
    if (timer_id >= g_timer_count || !g_timers[timer_id].active) {
        result->type = VAL_BOOL;
        result->data.bool_val = false;
        return 0;
    }
    
    int64_t elapsed = get_current_timestamp_ms() - g_timers[timer_id].start_time;
    bool expired = elapsed >= g_timers[timer_id].duration;
    
    if (expired) {
        g_timers[timer_id].expired = true;
    }
    
    result->type = VAL_BOOL;
    result->data.bool_val = expired;
    return 0;
}

/* ========== 延时函数 ========== */

/* 毫秒延时 */
int delay_ms(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1 || args[0].type != VAL_INT) {
        return -1;
    }
    
    /* 简化实现：在实际PLC中这会是非阻塞的 */
    /* 这里只返回成功状态 */
    result->type = VAL_BOOL;
    result->data.bool_val = true;
    return 0;
}

/* 秒延时 */
int delay_s(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1 || args[0].type != VAL_INT) {
        return -1;
    }
    
    result->type = VAL_BOOL;
    result->data.bool_val = true;
    return 0;
}

/* ========== 工具函数实现 ========== */

/* 判断闰年 */
bool is_leap_year(uint16_t year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/* 获取月份天数 */
uint8_t get_days_in_month(uint16_t year, uint8_t month) {
    static const uint8_t days_per_month[] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    
    if (month < 1 || month > 12) {
        return 0;
    }
    
    if (month == 2 && is_leap_year(year)) {
        return 29;
    }
    
    return days_per_month[month - 1];
}

/* 验证日期 */
bool validate_date(const st_date_t *date) {
    if (!date || date->year < 1970 || date->year > 2099) {
        return false;
    }
    
    if (date->month < 1 || date->month > 12) {
        return false;
    }
    
    uint8_t max_days = get_days_in_month(date->year, date->month);
    return (date->day >= 1 && date->day <= max_days);
}

/* 验证时间 */
bool validate_tod(const st_tod_t *tod) {
    if (!tod) {
        return false;
    }
    
    return (tod->hour <= 23 && tod->minute <= 59 && 
            tod->second <= 59 && tod->millisecond <= 999);
}

/* 日期时间转时间戳 */
int64_t datetime_to_timestamp(const st_dt_t *dt) {
    if (!dt || !validate_date(&dt->date) || !validate_tod(&dt->time)) {
        return -1;
    }
    
    /* 简化实现：返回近似时间戳 */
    struct tm tm_info = {0};
    tm_info.tm_year = dt->date.year - 1900;
    tm_info.tm_mon = dt->date.month - 1;
    tm_info.tm_mday = dt->date.day;
    tm_info.tm_hour = dt->time.hour;
    tm_info.tm_min = dt->time.minute;
    tm_info.tm_sec = dt->time.second;
    
    time_t timestamp = mktime(&tm_info);
    return (int64_t)timestamp * MS_PER_SECOND + dt->time.millisecond;
}

/* 时间戳转日期时间 */
void timestamp_to_datetime(int64_t timestamp, st_dt_t *dt) {
    if (!dt || timestamp < 0) {
        return;
    }
    
    time_t time_sec = timestamp / MS_PER_SECOND;
    uint16_t ms = timestamp % MS_PER_SECOND;
    
    struct tm *tm_info = localtime(&time_sec);
    if (tm_info) {
        dt->date.year = tm_info->tm_year + 1900;
        dt->date.month = tm_info->tm_mon + 1;
        dt->date.day = tm_info->tm_mday;
        dt->time.hour = tm_info->tm_hour;
        dt->time.minute = tm_info->tm_min;
        dt->time.second = tm_info->tm_sec;
        dt->time.millisecond = ms;
    }
}