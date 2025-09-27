/*
 * IEC61131 内置I/O库实现
 * 提供标准I/O操作函数和调试打印功能
 */

#include "libmgr.h"
#include "symbol_table.h"
#include "mmgr.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>

/* I/O通道配置 */
#define MAX_ANALOG_CHANNELS 16
#define MAX_DIGITAL_CHANNELS 32
#define MAX_SERIAL_PORTS 4
#define MAX_FILE_HANDLES 10

/* 内部数据结构 */
typedef struct {
    double analog_inputs[MAX_ANALOG_CHANNELS];
    double analog_outputs[MAX_ANALOG_CHANNELS];
    bool digital_inputs[MAX_DIGITAL_CHANNELS];
    bool digital_outputs[MAX_DIGITAL_CHANNELS];
    bool channel_enabled[MAX_ANALOG_CHANNELS];
    FILE *file_handles[MAX_FILE_HANDLES];
    char file_names[MAX_FILE_HANDLES][256];
    bool simulation_mode;
} io_system_t;

/* 全局I/O系统实例 */
static io_system_t g_io_system = {0};
static bool g_io_initialized = false;

/* 调试输出配置 */
static FILE *g_debug_output = NULL;
static bool g_debug_enabled = true;
static char g_debug_buffer[1024];

/* 内部辅助函数声明 */
static bool validate_channel_range(int32_t channel, int32_t max_channels, const char *func_name);
static bool validate_file_handle(int32_t handle, const char *func_name);
static void set_io_error(const char *func_name, const char *error);
static int get_free_file_handle(void);
static void simulate_analog_input(int32_t channel);
static void simulate_digital_input(int32_t channel);

/* ========== 模拟量I/O函数实现 ========== */

/* 读取模拟量输入 */
static int builtin_read_analog_input(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_io_error("READ_ANALOG_INPUT", "参数数量错误：期望1个参数（通道号）");
        return -1;
    }
    
    if (args[0].type != VAL_INT) {
        set_io_error("READ_ANALOG_INPUT", "参数类型错误：期望INT类型");
        return -1;
    }
    
    int32_t channel = args[0].data.int_val;
    
    if (!validate_channel_range(channel, MAX_ANALOG_CHANNELS, "READ_ANALOG_INPUT")) {
        return -1;
    }
    
    /* 如果是仿真模式，生成仿真数据 */
    if (g_io_system.simulation_mode) {
        simulate_analog_input(channel);
    }
    
    result->type = VAL_REAL;
    result->data.real_val = g_io_system.analog_inputs[channel];
    
    return 0;
}

/* 写入模拟量输出 */
static int builtin_write_analog_output(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2) {
        set_io_error("WRITE_ANALOG_OUTPUT", "参数数量错误：期望2个参数（通道号，输出值）");
        return -1;
    }
    
    if (args[0].type != VAL_INT) {
        set_io_error("WRITE_ANALOG_OUTPUT", "第一个参数类型错误：期望INT类型");
        return -1;
    }
    
    if (args[1].type != VAL_REAL && args[1].type != VAL_INT) {
        set_io_error("WRITE_ANALOG_OUTPUT", "第二个参数类型错误：期望REAL或INT类型");
        return -1;
    }
    
    int32_t channel = args[0].data.int_val;
    double value = (args[1].type == VAL_REAL) ? args[1].data.real_val : (double)args[1].data.int_val;
    
    if (!validate_channel_range(channel, MAX_ANALOG_CHANNELS, "WRITE_ANALOG_OUTPUT")) {
        return -1;
    }
    
    /* 值范围检查 */
    if (value < 0.0 || value > 10.0) {
        set_io_error("WRITE_ANALOG_OUTPUT", "输出值超出范围[0.0, 10.0]");
        return -1;
    }
    
    g_io_system.analog_outputs[channel] = value;
    
    result->type = VAL_BOOL;
    result->data.bool_val = true;
    
    return 0;
}

/* ========== 数字量I/O函数实现 ========== */

/* 读取数字量输入 */
static int builtin_read_digital_input(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_io_error("READ_DIGITAL_INPUT", "参数数量错误：期望1个参数（通道号）");
        return -1;
    }
    
    if (args[0].type != VAL_INT) {
        set_io_error("READ_DIGITAL_INPUT", "参数类型错误：期望INT类型");
        return -1;
    }
    
    int32_t channel = args[0].data.int_val;
    
    if (!validate_channel_range(channel, MAX_DIGITAL_CHANNELS, "READ_DIGITAL_INPUT")) {
        return -1;
    }
    
    /* 如果是仿真模式，生成仿真数据 */
    if (g_io_system.simulation_mode) {
        simulate_digital_input(channel);
    }
    
    result->type = VAL_BOOL;
    result->data.bool_val = g_io_system.digital_inputs[channel];
    
    return 0;
}

/* 写入数字量输出 */
static int builtin_write_digital_output(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2) {
        set_io_error("WRITE_DIGITAL_OUTPUT", "参数数量错误：期望2个参数（通道号，输出值）");
        return -1;
    }
    
    if (args[0].type != VAL_INT || args[1].type != VAL_BOOL) {
        set_io_error("WRITE_DIGITAL_OUTPUT", "参数类型错误：期望(INT, BOOL)类型");
        return -1;
    }
    
    int32_t channel = args[0].data.int_val;
    bool value = args[1].data.bool_val;
    
    if (!validate_channel_range(channel, MAX_DIGITAL_CHANNELS, "WRITE_DIGITAL_OUTPUT")) {
        return -1;
    }
    
    g_io_system.digital_outputs[channel] = value;
    
    result->type = VAL_BOOL;
    result->data.bool_val = true;
    
    return 0;
}

/* ========== 脉冲输出函数 ========== */

/* 生成脉冲输出 */
static int builtin_generate_pulse(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2) {
        set_io_error("GENERATE_PULSE", "参数数量错误：期望2个参数（通道号，脉冲宽度ms）");
        return -1;
    }
    
    if (args[0].type != VAL_INT || args[1].type != VAL_INT) {
        set_io_error("GENERATE_PULSE", "参数类型错误：期望(INT, INT)类型");
        return -1;
    }
    
    int32_t channel = args[0].data.int_val;
    int32_t duration_ms = args[1].data.int_val;
    
    if (!validate_channel_range(channel, MAX_DIGITAL_CHANNELS, "GENERATE_PULSE")) {
        return -1;
    }
    
    if (duration_ms <= 0 || duration_ms > 10000) {
        set_io_error("GENERATE_PULSE", "脉冲宽度必须在1-10000毫秒之间");
        return -1;
    }
    
    /* 简化实现：直接设置输出状态 */
    g_io_system.digital_outputs[channel] = true;
    
    /* 在实际硬件中这里会启动定时器，定时后自动清除输出 */
    /* 这里只是模拟，立即记录脉冲事件 */
    if (g_debug_enabled) {
        printf("IOLib: 通道%d生成%dms脉冲\n", channel, duration_ms);
    }
    
    result->type = VAL_BOOL;
    result->data.bool_val = true;
    
    return 0;
}

/* ========== 文件I/O函数实现 ========== */

/* 打开文件 */
static int builtin_file_open(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2) {
        set_io_error("FILE_OPEN", "参数数量错误：期望2个参数（文件名，模式）");
        return -1;
    }
    
    if (args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
        set_io_error("FILE_OPEN", "参数类型错误：期望(STRING, STRING)类型");
        return -1;
    }
    
    const char *filename = args[0].data.string_val;
    const char *mode = args[1].data.string_val;
    
    if (!filename || !mode) {
        set_io_error("FILE_OPEN", "文件名或模式不能为空");
        return -1;
    }
    
    /* 验证模式字符串 */
    if (strcmp(mode, "r") != 0 && strcmp(mode, "w") != 0 && 
        strcmp(mode, "a") != 0 && strcmp(mode, "r+") != 0) {
        set_io_error("FILE_OPEN", "不支持的文件打开模式");
        return -1;
    }
    
    /* 获取空闲文件句柄 */
    int handle = get_free_file_handle();
    if (handle < 0) {
        set_io_error("FILE_OPEN", "无可用文件句柄");
        return -1;
    }
    
    /* 打开文件 */
    FILE *fp = fopen(filename, mode);
    if (!fp) {
        set_io_error("FILE_OPEN", "文件打开失败");
        return -1;
    }
    
    g_io_system.file_handles[handle] = fp;
    strncpy(g_io_system.file_names[handle], filename, sizeof(g_io_system.file_names[handle]) - 1);
    
    result->type = VAL_INT;
    result->data.int_val = handle;
    
    return 0;
}

/* 关闭文件 */
static int builtin_file_close(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_io_error("FILE_CLOSE", "参数数量错误：期望1个参数（文件句柄）");
        return -1;
    }
    
    if (args[0].type != VAL_INT) {
        set_io_error("FILE_CLOSE", "参数类型错误：期望INT类型");
        return -1;
    }
    
    int32_t handle = args[0].data.int_val;
    
    if (!validate_file_handle(handle, "FILE_CLOSE")) {
        return -1;
    }
    
    if (g_io_system.file_handles[handle]) {
        fclose(g_io_system.file_handles[handle]);
        g_io_system.file_handles[handle] = NULL;
        g_io_system.file_names[handle][0] = '\0';
    }
    
    result->type = VAL_BOOL;
    result->data.bool_val = true;
    
    return 0;
}

/* 写入文件行 */
static int builtin_file_write_line(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 2) {
        set_io_error("FILE_WRITE_LINE", "参数数量错误：期望2个参数（文件句柄，内容）");
        return -1;
    }
    
    if (args[0].type != VAL_INT || args[1].type != VAL_STRING) {
        set_io_error("FILE_WRITE_LINE", "参数类型错误：期望(INT, STRING)类型");
        return -1;
    }
    
    int32_t handle = args[0].data.int_val;
    const char *content = args[1].data.string_val;
    
    if (!validate_file_handle(handle, "FILE_WRITE_LINE")) {
        return -1;
    }
    
    if (!content) {
        set_io_error("FILE_WRITE_LINE", "写入内容不能为空");
        return -1;
    }
    
    FILE *fp = g_io_system.file_handles[handle];
    if (fprintf(fp, "%s\n", content) < 0) {
        set_io_error("FILE_WRITE_LINE", "文件写入失败");
        return -1;
    }
    
    fflush(fp); /* 确保数据写入 */
    
    result->type = VAL_BOOL;
    result->data.bool_val = true;
    
    return 0;
}

/* 读取文件行 */
static int builtin_file_read_line(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_io_error("FILE_READ_LINE", "参数数量错误：期望1个参数（文件句柄）");
        return -1;
    }
    
    if (args[0].type != VAL_INT) {
        set_io_error("FILE_READ_LINE", "参数类型错误：期望INT类型");
        return -1;
    }
    
    int32_t handle = args[0].data.int_val;
    
    if (!validate_file_handle(handle, "FILE_READ_LINE")) {
        return -1;
    }
    
    FILE *fp = g_io_system.file_handles[handle];
    static char line_buffer[1024];
    
    if (fgets(line_buffer, sizeof(line_buffer), fp) == NULL) {
        /* 文件结束或错误 */
        char *empty_str = (char*)mmgr_alloc_string("");
        result->type = VAL_STRING;
        result->data.string_val = empty_str;
        return 0;
    }
    
    /* 移除换行符 */
    size_t len = strlen(line_buffer);
    if (len > 0 && line_buffer[len - 1] == '\n') {
        line_buffer[len - 1] = '\0';
    }
    
    char *line_str = (char*)mmgr_alloc_string(line_buffer);
    result->type = VAL_STRING;
    result->data.string_val = line_str;
    
    return 0;
}

/* ========== 调试打印函数实现 ========== */

/* ST调试打印 - 基础版本 */
static int builtin_debug_print(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_io_error("DEBUG_PRINT", "参数数量错误：期望1个参数");
        return -1;
    }
    
    if (!g_debug_enabled) {
        result->type = VAL_BOOL;
        result->data.bool_val = true;
        return 0;
    }
    
    FILE *output = g_debug_output ? g_debug_output : stdout;
    
    /* 添加时间戳 */
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    fprintf(output, "[%02d:%02d:%02d] ST_DEBUG: ", 
            tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    
    /* 根据参数类型打印值 */
    switch (args[0].type) {
        case VAL_INT:
            fprintf(output, "%d\n", args[0].data.int_val);
            break;
        case VAL_REAL:
            fprintf(output, "%.6f\n", args[0].data.real_val);
            break;
        case VAL_BOOL:
            fprintf(output, "%s\n", args[0].data.bool_val ? "TRUE" : "FALSE");
            break;
        case VAL_STRING:
            fprintf(output, "%s\n", args[0].data.string_val ? args[0].data.string_val : "(null)");
            break;
        default:
            fprintf(output, "<unknown type>\n");
            break;
    }
    
    fflush(output);
    
    result->type = VAL_BOOL;
    result->data.bool_val = true;
    
    return 0;
}

/* ST调试打印格式化版本 */
static int builtin_debug_printf(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count < 1) {
        set_io_error("DEBUG_PRINTF", "参数数量错误：至少需要1个参数（格式字符串）");
        return -1;
    }
    
    if (args[0].type != VAL_STRING) {
        set_io_error("DEBUG_PRINTF", "第一个参数必须是格式字符串");
        return -1;
    }
    
    if (!g_debug_enabled) {
        result->type = VAL_BOOL;
        result->data.bool_val = true;
        return 0;
    }
    
    const char *format = args[0].data.string_val;
    if (!format) {
        set_io_error("DEBUG_PRINTF", "格式字符串不能为空");
        return -1;
    }
    
    FILE *output = g_debug_output ? g_debug_output : stdout;
    
    /* 添加时间戳和前缀 */
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    fprintf(output, "[%02d:%02d:%02d] ST_DEBUG: ", 
            tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    
    /* 简化的格式化处理 - 只支持基本格式符 */
    const char *p = format;
    uint32_t arg_index = 1;
    
    while (*p && arg_index <= arg_count) {
        if (*p == '%' && *(p + 1)) {
            char specifier = *(p + 1);
            if (arg_index < arg_count) {
                switch (specifier) {
                    case 'd':
                        if (args[arg_index].type == VAL_INT) {
                            fprintf(output, "%d", args[arg_index].data.int_val);
                        }
                        break;
                    case 'f':
                        if (args[arg_index].type == VAL_REAL) {
                            fprintf(output, "%.6f", args[arg_index].data.real_val);
                        } else if (args[arg_index].type == VAL_INT) {
                            fprintf(output, "%.6f", (double)args[arg_index].data.int_val);
                        }
                        break;
                    case 's':
                        if (args[arg_index].type == VAL_STRING) {
                            fprintf(output, "%s", args[arg_index].data.string_val ? args[arg_index].data.string_val : "(null)");
                        }
                        break;
                    case 'b':
                        if (args[arg_index].type == VAL_BOOL) {
                            fprintf(output, "%s", args[arg_index].data.bool_val ? "TRUE" : "FALSE");
                        }
                        break;
                    case '%':
                        fprintf(output, "%%");
                        arg_index--; /* 不消耗参数 */
                        break;
                    default:
                        fprintf(output, "%%%c", specifier);
                        arg_index--; /* 不消耗参数 */
                        break;
                }
                arg_index++;
            }
            p += 2;
        } else {
            fputc(*p, output);
            p++;
        }
    }
    
    /* 输出剩余的格式字符串 */
    while (*p) {
        fputc(*p, output);
        p++;
    }
    
    fprintf(output, "\n");
    fflush(output);
    
    result->type = VAL_BOOL;
    result->data.bool_val = true;
    
    return 0;
}

/* 控制台输出函数 */
static int builtin_write_line(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_io_error("WRITE_LINE", "参数数量错误：期望1个参数");
        return -1;
    }
    
    /* 根据参数类型输出到控制台 */
    switch (args[0].type) {
        case VAL_INT:
            printf("%d\n", args[0].data.int_val);
            break;
        case VAL_REAL:
            printf("%.6f\n", args[0].data.real_val);
            break;
        case VAL_BOOL:
            printf("%s\n", args[0].data.bool_val ? "TRUE" : "FALSE");
            break;
        case VAL_STRING:
            printf("%s\n", args[0].data.string_val ? args[0].data.string_val : "(null)");
            break;
        default:
            printf("<unknown type>\n");
            break;
    }
    
    fflush(stdout);
    
    result->type = VAL_BOOL;
    result->data.bool_val = true;
    
    return 0;
}

/* ========== 系统信息函数 ========== */

/* 获取系统时间戳 */
static int builtin_get_timestamp(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 0) {
        set_io_error("GET_TIMESTAMP", "参数数量错误：期望0个参数");
        return -1;
    }
    
    result->type = VAL_INT;
    result->data.int_val = (int32_t)time(NULL);
    
    return 0;
}

/* 延时函数 */
static int builtin_delay(struct vm_value *args, uint32_t arg_count, struct vm_value *result) {
    if (arg_count != 1) {
        set_io_error("DELAY", "参数数量错误：期望1个参数（延时毫秒）");
        return -1;
    }
    
    if (args[0].type != VAL_INT) {
        set_io_error("DELAY", "参数类型错误：期望INT类型");
        return -1;
    }
    
    int32_t delay_ms = args[0].data.int_val;
    
    if (delay_ms < 0 || delay_ms > 60000) {
        set_io_error("DELAY", "延时时间必须在0-60000毫秒之间");
        return -1;
    }
    
    /* 使用标准库延时 */
    usleep(delay_ms / 1000);
    
    result->type = VAL_BOOL;
    result->data.bool_val = true;
    
    return 0;
}

/* ========== 辅助函数实现 ========== */

/* 验证通道范围 */
static bool validate_channel_range(int32_t channel, int32_t max_channels, const char *func_name) {
    if (channel < 0 || channel >= max_channels) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "%s: 通道号%d超出范围[0, %d]", 
                 func_name, channel, max_channels - 1);
        set_io_error(func_name, error_msg);
        return false;
    }
    return true;
}

/* 验证文件句柄 */
static bool validate_file_handle(int32_t handle, const char *func_name) {
    if (handle < 0 || handle >= MAX_FILE_HANDLES) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "%s: 文件句柄%d无效", func_name, handle);
        set_io_error(func_name, error_msg);
        return false;
    }
    
    if (!g_io_system.file_handles[handle]) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "%s: 文件句柄%d未打开", func_name, handle);
        set_io_error(func_name, error_msg);
        return false;
    }
    
    return true;
}

/* 设置I/O错误 */
static void set_io_error(const char *func_name, const char *error) {
    char full_error[512];
    snprintf(full_error, sizeof(full_error), "IOLib.%s: %s", func_name, error);
    libmgr_set_error(full_error);
}

/* 获取空闲文件句柄 */
static int get_free_file_handle(void) {
    for (int i = 0; i < MAX_FILE_HANDLES; i++) {
        if (g_io_system.file_handles[i] == NULL) {
            return i;
        }
    }
    return -1;
}

/* 模拟模拟量输入 */
static void simulate_analog_input(int32_t channel) {
    /* 生成0-10V范围的模拟数据 */
    static double sim_values[MAX_ANALOG_CHANNELS] = {0};
    static double sim_increments[MAX_ANALOG_CHANNELS] = {0.1, 0.2, 0.15, 0.3};
    
    sim_values[channel] += sim_increments[channel % 4];
    if (sim_values[channel] > 10.0) {
        sim_values[channel] = 0.0;
    }
    
    g_io_system.analog_inputs[channel] = sim_values[channel];
}

/* 模拟数字量输入 */
static void simulate_digital_input(int32_t channel) {
    /* 简单的周期性变化 */
    static int sim_counter = 0;
    sim_counter++;
    
    g_io_system.digital_inputs[channel] = (sim_counter % (10 + channel)) < 5;
}

/* ========== 库初始化函数 ========== */

/* 初始化I/O系统 */
static int io_system_init(void) {
    if (g_io_initialized) {
        return 0;
    }
    
    memset(&g_io_system, 0, sizeof(io_system_t));
    
    /* 默认启用仿真模式 */
    g_io_system.simulation_mode = true;
    
    /* 初始化调试输出 */
    g_debug_output = stdout;
    g_debug_enabled = true;
    
    g_io_initialized = true;
    return 0;
}

/* 初始化内置I/O库 */
int builtin_io_init(void) {
    if (!libmgr_is_initialized()) {
        return -1;
    }
    
    /* 初始化I/O系统 */
    if (io_system_init() != 0) {
        return -1;
    }
    
    /* 创建I/O库 */
    library_info_t *io_lib = libmgr_find_library("IOLib");
    if (!io_lib) {
        libmgr_set_error("IOLib库未找到，请先调用libmgr_init_builtin_io()");
        return -1;
    }
    
    /* 获取类型信息 */
    type_info_t *int_type = create_basic_type(TYPE_INT_ID);
    type_info_t *real_type = create_basic_type(TYPE_REAL_ID);
    type_info_t *bool_type = create_basic_type(TYPE_BOOL_ID);
    type_info_t *string_type = create_basic_type(TYPE_STRING_ID);
    
    if (!int_type || !real_type || !bool_type || !string_type) {
        libmgr_set_error("无法创建基础类型");
        return -1;
    }
    
    /* 注册模拟量I/O函数 */
    if (!libmgr_register_builtin_function("IOLib", "ReadAnalogInput", builtin_read_analog_input, real_type, &int_type, 1)) {
        libmgr_set_error("注册ReadAnalogInput函数失败");
        return -1;
    }
    
    type_info_t *write_analog_params[2] = {int_type, real_type};
    if (!libmgr_register_builtin_function("IOLib", "WriteAnalogOutput", builtin_write_analog_output, bool_type, write_analog_params, 2)) {
        libmgr_set_error("注册WriteAnalogOutput函数失败");
        return -1;
    }
    
    /* 注册数字量I/O函数 */
    if (!libmgr_register_builtin_function("IOLib", "ReadDigitalInput", builtin_read_digital_input, bool_type, &int_type, 1)) {
        libmgr_set_error("注册ReadDigitalInput函数失败");
        return -1;
    }
    
    type_info_t *write_digital_params[2] = {int_type, bool_type};
    if (!libmgr_register_builtin_function("IOLib", "WriteDigitalOutput", builtin_write_digital_output, bool_type, write_digital_params, 2)) {
        libmgr_set_error("注册WriteDigitalOutput函数失败");
        return -1;
    }
    
    /* 注册脉冲输出函数 */
    type_info_t *pulse_params[2] = {int_type, int_type};
    if (!libmgr_register_builtin_function("IOLib", "GeneratePulse", builtin_generate_pulse, bool_type, pulse_params, 2)) {
        libmgr_set_error("注册GeneratePulse函数失败");
        return -1;
    }
    
    /* 注册文件I/O函数 */
    type_info_t *file_open_params[2] = {string_type, string_type};
    if (!libmgr_register_builtin_function("IOLib", "FileOpen", builtin_file_open, int_type, file_open_params, 2) ||
        !libmgr_register_builtin_function("IOLib", "FileClose", builtin_file_close, bool_type, &int_type, 1)) {
        libmgr_set_error("注册文件操作函数失败");
        return -1;
    }
    
    type_info_t *file_write_params[2] = {int_type, string_type};
    if (!libmgr_register_builtin_function("IOLib", "FileWriteLine", builtin_file_write_line, bool_type, file_write_params, 2) ||
        !libmgr_register_builtin_function("IOLib", "FileReadLine", builtin_file_read_line, string_type, &int_type, 1)) {
        libmgr_set_error("注册文件读写函数失败");
        return -1;
    }
    
    /* 注册调试打印函数 */
    if (!libmgr_register_builtin_function("IOLib", "DebugPrint", builtin_debug_print, bool_type, &string_type, 1) ||
        !libmgr_register_builtin_function("IOLib", "WriteLine", builtin_write_line, bool_type, &string_type, 1)) {
        libmgr_set_error("注册调试打印函数失败");
        return -1;
    }
    
    /* 注册格式化调试打印（简化版本，只支持字符串格式） */
    if (!libmgr_register_builtin_function("IOLib", "DebugPrintf", builtin_debug_printf, bool_type, &string_type, 1)) {
        libmgr_set_error("注册DebugPrintf函数失败");
        return -1;
    }
    
    /* 注册系统函数 */
    type_info_t *no_params = NULL;
    if (!libmgr_register_builtin_function("IOLib", "GetTimestamp", builtin_get_timestamp, int_type, no_params, 0) ||
        !libmgr_register_builtin_function("IOLib", "Delay", builtin_delay, bool_type, &int_type, 1)) {
        libmgr_set_error("注册系统函数失败");
        return -1;
    }
    
    return 0;
}

/* ========== 公共接口函数 ========== */

/* 设置仿真模式 */
int builtin_io_set_simulation_mode(bool enabled) {
    if (!g_io_initialized) {
        return -1;
    }
    
    g_io_system.simulation_mode = enabled;
    return 0;
}

/* 设置调试输出文件 */
int builtin_io_set_debug_output(const char *filename) {
    if (g_debug_output && g_debug_output != stdout) {
        fclose(g_debug_output);
    }
    
    if (filename && strlen(filename) > 0) {
        g_debug_output = fopen(filename, "a");
        if (!g_debug_output) {
            g_debug_output = stdout;
            return -1;
        }
    } else {
        g_debug_output = stdout;
    }
    
    return 0;
}

/* 启用/禁用调试输出 */
void builtin_io_enable_debug(bool enabled) {
    g_debug_enabled = enabled;
}

/* 获取I/O库版本信息 */
const char* builtin_io_get_version(void) {
    return "1.0.0";
}

/* 获取I/O库描述 */
const char* builtin_io_get_description(void) {
    return "IEC61131 ST内置I/O库，提供模拟量/数字量I/O、文件操作、调试打印等功能";
}

/* 获取支持的函数列表 */
const char** builtin_io_get_function_list(void) {
    static const char* functions[] = {
        "ReadAnalogInput", "WriteAnalogOutput",
        "ReadDigitalInput", "WriteDigitalOutput", "GeneratePulse",
        "FileOpen", "FileClose", "FileWriteLine", "FileReadLine",
        "DebugPrint", "DebugPrintf", "WriteLine",
        "GetTimestamp", "Delay",
        NULL
    };
    
    return functions;
}

/* I/O库功能演示 */
int builtin_io_demo(void) {
    printf("=== IOLib功能演示 ===\n");
    
    struct vm_value args[3];
    struct vm_value result;
    
    /* 演示模拟量输入 */
    args[0].type = VAL_INT;
    args[0].data.int_val = 0;
    
    if (builtin_read_analog_input(args, 1, &result) == 0) {
        printf("ReadAnalogInput(0) = %.3f V\n", result.data.real_val);
    }
    
    /* 演示数字量输出 */
    args[0].type = VAL_INT;
    args[0].data.int_val = 1;
    args[1].type = VAL_BOOL;
    args[1].data.bool_val = true;
    
    if (builtin_write_digital_output(args, 2, &result) == 0) {
        printf("WriteDigitalOutput(1, TRUE) = %s\n", result.data.bool_val ? "成功" : "失败");
    }
    
    /* 演示调试打印 */
    args[0].type = VAL_STRING;
    args[0].data.string_val = "IOLib演示完成";
    
    if (builtin_debug_print(args, 1, &result) == 0) {
        printf("调试打印功能正常\n");
    }
    
    printf("===================\n");
    
    return 0;
}

/* 清理I/O库资源 */
void builtin_io_cleanup(void) {
    if (!g_io_initialized) {
        return;
    }
    
    /* 关闭所有打开的文件 */
    for (int i = 0; i < MAX_FILE_HANDLES; i++) {
        if (g_io_system.file_handles[i]) {
            fclose(g_io_system.file_handles[i]);
            g_io_system.file_handles[i] = NULL;
        }
    }
    
    /* 关闭调试输出文件 */
    if (g_debug_output && g_debug_output != stdout) {
        fclose(g_debug_output);
        g_debug_output = stdout;
    }
    
    g_io_initialized = false;
}
