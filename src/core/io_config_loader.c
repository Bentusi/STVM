/**
 * @file io_config_loader.c
 * @brief 简化的 IO 配置文件加载器
 * 
 * 注意：这是一个简化版本,仅支持基本的 JSON 格式解析
 * 未来可以替换为完整的 JSON 解析库 (如 cJSON)
 */

#include "iomgr.h"
#include "mmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief 简单的配置解析器 - 仅解析我们需要的字段
 * 
 * 支持的格式示例:
 * {
 *   "address": "%IX0.0",
 *   "type": "digital_in",
 *   "pin": 27
 * }
 */

typedef struct {
    char address[32];
    char type[32];
    int pin;
    bool valid;
} SimpleIOConfig;

/**
 * @brief 解析一行配置
 */
static bool parse_config_line(const char* line, const char* key, char* value, size_t value_size) {
    // 查找 "key"
    char search_key[128];
    snprintf(search_key, sizeof(search_key), "\"%s\"", key);
    
    const char* key_pos = strstr(line, search_key);
    if (!key_pos) return false;
    
    // 查找 :
    const char* colon = strchr(key_pos, ':');
    if (!colon) return false;
    
    // 跳过空格
    colon++;
    while (*colon && isspace(*colon)) colon++;
    
    // 如果是字符串 (以 " 开头)
    if (*colon == '"') {
        colon++;  // 跳过开头的 "
        const char* end_quote = strchr(colon, '"');
        if (!end_quote) return false;
        
        size_t len = end_quote - colon;
        if (len >= value_size) len = value_size - 1;
        strncpy(value, colon, len);
        value[len] = '\0';
        return true;
    }
    
    // 如果是数字
    if (isdigit(*colon) || *colon == '-') {
        const char* start = colon;
        while (*colon && (isdigit(*colon) || *colon == '-' || *colon == '.')) colon++;
        
        size_t len = colon - start;
        if (len >= value_size) len = value_size - 1;
        strncpy(value, start, len);
        value[len] = '\0';
        return true;
    }
    
    return false;
}

/**
 * @brief 将类型字符串转换为 IODeviceType
 */
static IODeviceType string_to_device_type(const char* type_str) {
    if (strcmp(type_str, "digital_in") == 0) return IO_DEV_GPIO_IN;
    if (strcmp(type_str, "digital_out") == 0) return IO_DEV_GPIO_OUT;
    if (strcmp(type_str, "analog_in") == 0) return IO_DEV_ADC;
    if (strcmp(type_str, "analog_out") == 0) return IO_DEV_DAC;
    if (strcmp(type_str, "pwm") == 0) return IO_DEV_PWM;
    if (strcmp(type_str, "encoder") == 0) return IO_DEV_ENCODER;
    return IO_DEV_GPIO_IN;  // 默认
}

/**
 * @brief 从配置文件加载 IO 点
 * 
 * @param mgr IO 管理器
 * @param config_file 配置文件路径
 * @return ErrorCode
 */
ErrorCode io_manager_load_config_simple(IOManager* mgr, const char* config_file) {
    if (!mgr || !config_file) {
        return ERR_INVALID_ARGUMENT;
    }
    
    FILE* f = fopen(config_file, "r");
    if (!f) {
        fprintf(stderr, "错误：无法打开配置文件 '%s'\n", config_file);
        return ERR_NOT_FOUND;
    }
    
    char line[512];
    SimpleIOConfig current_config = {0};
    bool in_point = false;
    int points_added = 0;
    
    printf("正在加载 IO 配置文件: %s\n", config_file);
    
    while (fgets(line, sizeof(line), f)) {
        // 检测是否进入一个新的配置块 (简化判断)
        if (strstr(line, "\"address\"")) {
            in_point = true;
            memset(&current_config, 0, sizeof(current_config));
        }
        
        if (in_point) {
            char value[128];
            
            // 解析 address
            if (parse_config_line(line, "address", value, sizeof(value))) {
                strncpy(current_config.address, value, sizeof(current_config.address) - 1);
            }
            
            // 解析 type
            if (parse_config_line(line, "type", value, sizeof(value))) {
                strncpy(current_config.type, value, sizeof(current_config.type) - 1);
            }
            
            // 解析 pin
            if (parse_config_line(line, "pin", value, sizeof(value))) {
                current_config.pin = atoi(value);
            }
            
            // 检测配置块结束 (遇到 } 或下一个配置)
            if (strchr(line, '}') && current_config.address[0] != '\0') {
                // 添加这个 IO 点
                IOAddress addr;
                if (io_address_parse(current_config.address, &addr) == OK) {
                    IOPointConfig config;
                    config.address = addr;
                    config.device_type = string_to_device_type(current_config.type);
                    config.access_mode = (config.device_type == IO_DEV_GPIO_IN || 
                                         config.device_type == IO_DEV_ADC || 
                                         config.device_type == IO_DEV_ENCODER) 
                                        ? IO_ACCESS_READ : IO_ACCESS_WRITE;
                    config.hardware_path = NULL;
                    config.hardware_address = current_config.pin;
                    config.scale = 1.0;
                    config.offset = 0.0;
                    config.enable_filter = false;
                    config.filter_samples = 1;
                    
                    ErrorCode err = io_manager_add_point(mgr, &config);
                    if (err == OK) {
                        printf("  已添加: %s (%s, pin %d)\n", 
                               current_config.address, current_config.type, current_config.pin);
                        points_added++;
                    }
                }
                
                in_point = false;
            }
        }
    }
    
    fclose(f);
    
    printf("配置加载完成，共添加 %d 个 IO 点\n", points_added);
    
    return points_added > 0 ? OK : ERR_NOT_FOUND;
}
