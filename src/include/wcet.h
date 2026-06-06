/**
 * @file wcet.h
 * @brief WCET (Worst-Case Execution Time) 分析器
 * 
 * 功能安全 (IEC 61508) 要求：
 * - 静态分析字节码的最坏执行路径
 * - 为每条指令分配周期成本
 * - 支持循环边界标注 (@LOOP_MAX)
 * - 输出 WCET 估算结果用于周期时间验证
 */

#ifndef STVM_WCET_H
#define STVM_WCET_H

#include "bytecode.h"
#include "error.h"
#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// 指令周期成本表（基于 ARM Cortex-M4 参考值，可校准）
// ============================================================================

/**
 * @brief 指令周期成本结构
 * 
 * 每条指令的时钟周期数：
 * - base: 基础周期数（取指+解码）
 * - memory: 内存访问额外周期（LOAD/STORE 类型指令）
 * - branch_penalty: 跳转惩罚周期（JMP/JZ/JNZ）
 */
typedef struct {
    uint8_t base;           // 基础周期数
    uint8_t memory;         // 内存访问额外周期
    uint8_t branch_penalty; // 分支惩罚周期
} InstructionCost;

/**
 * @brief 获取所有指令的周期成本表
 * @return 指令成本数组（索引为 Opcode）
 */
const InstructionCost* wcet_get_instruction_cost_table(void);

/**
 * @brief 获取指定指令的总周期成本
 * @param opcode 操作码
 * @return 总周期数
 */
uint32_t wcet_instruction_cost(Opcode opcode);

// ============================================================================
// 循环边界标注
// ============================================================================

/**
 * @brief 循环边界条目
 * 
 * 用于标注程序中循环的最大迭代次数。
 * 在源代码中使用 @LOOP_MAX(n) 注解指定。
 */
typedef struct {
    char* function_name;    // 函数名
    uint32_t address;       // 循环入口地址（字节码偏移）
    uint32_t max_iterations;// 最大迭代次数
} LoopBound;

#define WCET_MAX_LOOP_BOUNDS 64

// ============================================================================
// WCET 分析结果
// ============================================================================

/**
 * @brief WCET 分析结果
 */
typedef struct {
    // 最坏路径信息
    uint64_t max_instructions;      // 最坏路径上的指令总数
    uint64_t max_cycles;            // 最坏路径上的总周期数
    uint64_t max_function_depth;    // 最大函数调用深度
    uint32_t max_loop_iterations;   // 单循环最大迭代次数
    uint32_t analyzed_functions;    // 已分析的函数数
    uint32_t analyzed_loops;        // 已分析的循环数
    uint32_t unreachable_code;      // 不可达代码块数
    
    // 路径明细
    uint64_t instructions_no_loops; // 不含循环的线性路径指令数
    uint64_t loop_overhead;         // 循环带来的额外指令数
    
    // 时间估算
    double wcet_microseconds;       // WCET 微秒数 (假设 CPU 频率)
    double cpu_frequency_mhz;       // CPU 频率（用于计算微秒）
    
    // 状态
    bool analysis_complete;         // 分析是否完整
    char error_msg[256];            // 错误信息
} WcetResult;

// ============================================================================
// WCET 分析配置
// ============================================================================

/**
 * @brief WCET 分析器配置
 */
typedef struct {
    double cpu_frequency_mhz;        // CPU 时钟频率（默认 168.0 = 168MHz）
    uint32_t max_function_depth;     // 最大分析深度（0=无限制）
    uint32_t default_loop_bound;     // 未标注循环的默认最大迭代次数（0=报错）
    bool enable_unreachable_check;   // 是否检测不可达代码
    bool verbose;                    // 是否输出详细信息
} WcetConfig;

/**
 * @brief 初始化 WCET 配置为默认值
 * @param config 配置指针
 */
void wcet_config_init(WcetConfig* config);

// ============================================================================
// WCET 分析器 API
// ============================================================================

/**
 * @brief WCET 分析器结构
 */
typedef struct WcetAnalyzer WcetAnalyzer;

/**
 * @brief 创建 WCET 分析器
 * @param module 字节码模块
 * @param config 分析配置
 * @return 分析器实例
 */
WcetAnalyzer* wcet_analyzer_create(BytecodeModule* module, const WcetConfig* config);

/**
 * @brief 释放 WCET 分析器
 * @param analyzer 分析器实例
 */
void wcet_analyzer_free(WcetAnalyzer* analyzer);

/**
 * @brief 添加循环边界标注
 * @param analyzer 分析器
 * @param func_name 函数名
 * @param address 循环入口地址
 * @param max_iterations 最大迭代次数
 * @return OK 或错误码
 */
ErrorCode wcet_add_loop_bound(WcetAnalyzer* analyzer, const char* func_name,
                               uint32_t address, uint32_t max_iterations);

/**
 * @brief 运行 WCET 分析
 * @param analyzer 分析器
 * @param entry_function 入口函数名（NULL = 从模块入口点开始）
 * @param result 输出分析结果
 * @return OK 或错误码
 */
ErrorCode wcet_analyze(WcetAnalyzer* analyzer, const char* entry_function,
                        WcetResult* result);

/**
 * @brief 打印 WCET 分析报告
 * @param result 分析结果
 */
void wcet_print_report(const WcetResult* result);

/**
 * @brief 验证周期预算是否满足 WCET 要求
 * @param result WCET 分析结果
 * @param cycle_time_ms 目标周期时间（毫秒）
 * @param safety_margin_percent 安全裕度百分比（如 20 = 需要 20% 余量）
 * @return true = 通过验证
 */
bool wcet_validate_cycle_budget(const WcetResult* result, 
                                 uint32_t cycle_time_ms,
                                 uint32_t safety_margin_percent);

/**
 * @brief 从 ST 源文件解析 @LOOP_MAX 标注
 * @param source_file ST 源文件路径
 * @param bounds 输出的循环边界数组
 * @param max_bounds 数组容量
 * @return 解析到的标注数量
 */
uint32_t wcet_parse_loop_bounds_from_source(const char* source_file,
                                              LoopBound* bounds,
                                              uint32_t max_bounds);

#endif // STVM_WCET_H