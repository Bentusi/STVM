#ifndef BUILTIN_TIME_H
#define BUILTIN_TIME_H

#include "vm.h"
#include <stdint.h>
#include <stdbool.h>

/* 时间库错误码 */
typedef enum {
    TIME_ERROR_NONE = 0,
    TIME_ERROR_INVALID_FORMAT,
    TIME_ERROR_INVALID_DURATION,
    TIME_ERROR_OVERFLOW,
    TIME_ERROR_NULL_POINTER
} time_error_t;

/* 时间数据类型 */
typedef struct st_time {
    int64_t milliseconds;       // 以毫秒为单位的时间值
} st_time_t;

typedef struct st_date {
    uint16_t year;              // 年份 (1970-2099)
    uint8_t month;              // 月份 (1-12)
    uint8_t day;                // 日 (1-31)
} st_date_t;

typedef struct st_tod {
    uint8_t hour;               // 小时 (0-23)
    uint8_t minute;             // 分钟 (0-59)
    uint8_t second;             // 秒 (0-59)
    uint16_t millisecond;       // 毫秒 (0-999)
} st_tod_t;

typedef struct st_dt {
    st_date_t date;             // 日期部分
    st_tod_t time;              // 时间部分
} st_dt_t;

/* 时间库初始化和清理 */
int builtin_time_init(void);
void builtin_time_cleanup(void);

/* 时间创建函数 */
int time_create_time_ms(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int time_create_time_s(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int time_create_date(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int time_create_tod(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int time_create_dt(struct vm_value *args, uint32_t arg_count, struct vm_value *result);

/* 时间获取函数 */
int time_get_current_time(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int time_get_current_date(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int time_get_current_tod(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int time_get_current_dt(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int time_get_system_tick(struct vm_value *args, uint32_t arg_count, struct vm_value *result);

/* 时间转换函数 */
int time_to_milliseconds(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int time_to_seconds(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int time_to_minutes(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int time_to_hours(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int time_to_days(struct vm_value *args, uint32_t arg_count, struct vm_value *result);

/* 时间算术运算 */
int time_add_time(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int time_sub_time(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int time_mul_time(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int time_div_time(struct vm_value *args, uint32_t arg_count, struct vm_value *result);

/* 时间比较函数 */
int time_eq(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int time_ne(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int time_lt(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int time_le(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int time_gt(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int time_ge(struct vm_value *args, uint32_t arg_count, struct vm_value *result);

/* 日期时间操作 */
int date_add_time(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int date_sub_time(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int date_diff(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int date_get_year(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int date_get_month(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int date_get_day(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int date_get_weekday(struct vm_value *args, uint32_t arg_count, struct vm_value *result);

/* 时间格式化和解析 */
int time_to_string(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int date_to_string(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int tod_to_string(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int dt_to_string(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int string_to_time(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int string_to_date(struct vm_value *args, uint32_t arg_count, struct vm_value *result);

/* 定时器函数 */
int timer_start(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int timer_stop(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int timer_reset(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int timer_elapsed(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int timer_is_expired(struct vm_value *args, uint32_t arg_count, struct vm_value *result);

/* 延时函数 */
int delay_ms(struct vm_value *args, uint32_t arg_count, struct vm_value *result);
int delay_s(struct vm_value *args, uint32_t arg_count, struct vm_value *result);

/* 工具函数 */
bool is_leap_year(uint16_t year);
uint8_t get_days_in_month(uint16_t year, uint8_t month);
int64_t datetime_to_timestamp(const st_dt_t *dt);
void timestamp_to_datetime(int64_t timestamp, st_dt_t *dt);
bool validate_date(const st_date_t *date);
bool validate_tod(const st_tod_t *tod);

#endif /* BUILTIN_TIME_H */
