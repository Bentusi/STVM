/**
 * @file wcet.c
 * @brief WCET (Worst-Case Execution Time) 静态分析器实现
 *
 * 功能安全 (IEC 61508) 核心模块：
 * - 字节码控制流图 (CFG) 构建
 * - 基于 IPET (Implicit Path Enumeration Technique) 的最坏路径分析
 * - 循环边界自动检测与标注支持
 * - 函数调用链递归分析
 *
 * 分析策略：
 * 1. 构建基本块 (Basic Block)
 * 2. 识别循环结构 (Loop Detection)
 * 3. 展开循环至标注的最大迭代次数
 * 4. 累加最坏路径上的指令周期
 */

#include "wcet.h"
#include "mmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>

// ============================================================================
// 指令周期成本表
// ============================================================================

/**
 * @brief 每条指令的周期成本（基于 ARM Cortex-M4 确定性假设）
 *
 * 取值说明：
 * - base: 单条解释指令执行周期数（包括 switch/PC++ 开销）
 * - memory: 栈内存访问额外周期
 * - branch_penalty: 跳转/调用额外周期
 *
 * 实际值应通过目标平台测量校准。此处为保守估算。
 */
static const InstructionCost g_cost_table[OP_COUNT] = {
    // OP_PUSH
    [OP_PUSH]          = { .base = 12, .memory = 4, .branch_penalty = 0 },
    // OP_POP
    [OP_POP]           = { .base = 6,  .memory = 4, .branch_penalty = 0 },
    // OP_DUP
    [OP_DUP]           = { .base = 8,  .memory = 4, .branch_penalty = 0 },
    // OP_LOAD
    [OP_LOAD]          = { .base = 10, .memory = 8, .branch_penalty = 0 },
    // OP_STORE
    [OP_STORE]         = { .base = 10, .memory = 8, .branch_penalty = 0 },
    // 算术
    [OP_ADD]           = { .base = 8,  .memory = 0, .branch_penalty = 0 },
    [OP_SUB]           = { .base = 8,  .memory = 0, .branch_penalty = 0 },
    [OP_MUL]           = { .base = 10, .memory = 0, .branch_penalty = 0 },
    [OP_DIV]           = { .base = 14, .memory = 0, .branch_penalty = 0 },
    [OP_MOD]           = { .base = 14, .memory = 0, .branch_penalty = 0 },
    [OP_NEG]           = { .base = 6,  .memory = 0, .branch_penalty = 0 },
    // 比较
    [OP_EQ]            = { .base = 8,  .memory = 0, .branch_penalty = 0 },
    [OP_NE]            = { .base = 8,  .memory = 0, .branch_penalty = 0 },
    [OP_LT]            = { .base = 8,  .memory = 0, .branch_penalty = 0 },
    [OP_LE]            = { .base = 8,  .memory = 0, .branch_penalty = 0 },
    [OP_GT]            = { .base = 8,  .memory = 0, .branch_penalty = 0 },
    [OP_GE]            = { .base = 8,  .memory = 0, .branch_penalty = 0 },
    // 逻辑
    [OP_AND]           = { .base = 8,  .memory = 0, .branch_penalty = 0 },
    [OP_OR]            = { .base = 8,  .memory = 0, .branch_penalty = 0 },
    [OP_NOT]           = { .base = 6,  .memory = 0, .branch_penalty = 0 },
    [OP_XOR]           = { .base = 8,  .memory = 0, .branch_penalty = 0 },
    // 位运算
    [OP_BIT_AND]       = { .base = 8,  .memory = 0, .branch_penalty = 0 },
    [OP_BIT_OR]        = { .base = 8,  .memory = 0, .branch_penalty = 0 },
    [OP_BIT_XOR]       = { .base = 8,  .memory = 0, .branch_penalty = 0 },
    [OP_BIT_NOT]       = { .base = 6,  .memory = 0, .branch_penalty = 0 },
    [OP_SHL]           = { .base = 10, .memory = 0, .branch_penalty = 0 },
    [OP_SHR]           = { .base = 10, .memory = 0, .branch_penalty = 0 },
    // 控制流
    [OP_JMP]           = { .base = 4,  .memory = 0, .branch_penalty = 4 },
    [OP_JZ]            = { .base = 10, .memory = 4, .branch_penalty = 6 },
    [OP_JNZ]           = { .base = 10, .memory = 4, .branch_penalty = 6 },
    [OP_CALL]          = { .base = 20, .memory = 12,.branch_penalty = 8 },
    [OP_RET]           = { .base = 12, .memory = 8, .branch_penalty = 4 },
    // 其他
    [OP_HALT]          = { .base = 2,  .memory = 0, .branch_penalty = 0 },
    [OP_CALL_EXT]      = { .base = 30, .memory = 8, .branch_penalty = 8 },
    [OP_NOP]           = { .base = 2,  .memory = 0, .branch_penalty = 0 },
    // 数组
    [OP_LOAD_INDEXED]  = { .base = 14, .memory = 12,.branch_penalty = 0 },
    [OP_STORE_INDEXED] = { .base = 14, .memory = 12,.branch_penalty = 0 },
    // 质量位
    [OP_LOAD_VAL]      = { .base = 10, .memory = 8, .branch_penalty = 0 },
    [OP_LOAD_QUALITY]  = { .base = 10, .memory = 8, .branch_penalty = 0 },
    [OP_STORE_VAL]     = { .base = 10, .memory = 8, .branch_penalty = 0 },
    [OP_STORE_QUALITY] = { .base = 10, .memory = 8, .branch_penalty = 0 },
    // I/O
    [OP_IO_READ]       = { .base = 25, .memory = 8, .branch_penalty = 0 },
    [OP_IO_WRITE]      = { .base = 25, .memory = 8, .branch_penalty = 0 },
};

const InstructionCost* wcet_get_instruction_cost_table(void) {
    return g_cost_table;
}

uint32_t wcet_instruction_cost(Opcode opcode) {
    if (opcode >= OP_COUNT) return 0;
    const InstructionCost* cost = &g_cost_table[opcode];
    return (uint32_t)(cost->base + cost->memory + cost->branch_penalty);
}

// ============================================================================
// 基本块 (Basic Block) 结构
// ============================================================================

/**
 * @brief 基本块终结类型
 */
typedef enum {
    BT_FALLTHROUGH,  // 顺序执行
    BT_JUMP,         // 无条件跳转
    BT_BRANCH,       // 条件分支
    BT_CALL,         // 函数调用
    BT_RETURN,       // 函数返回
    BT_HALT          // 程序停止
} BlockTerminator;

/**
 * @brief 基本块
 */
typedef struct BasicBlock {
    uint32_t start_pc;          // 起始指令地址
    uint32_t end_pc;            // 结束指令地址 (exclusive)
    uint64_t cycle_cost;        // 本块的周期成本
    uint64_t instruction_count; // 指令数
    BlockTerminator terminator; // 终结类型
    uint32_t target_pc[2];      // 跳转目标 (-1 表示无)
    uint32_t target_count;      // 目标数
    bool visited;               // 分析中标记
    bool in_loop;               // 在循环中标记
    uint32_t loop_multiplier;   // 循环展开倍数
} BasicBlock;

#define WCET_MAX_BLOCKS 2048

// ============================================================================
// WCET 分析器内部状态
// ============================================================================

struct WcetAnalyzer {
    BytecodeModule* module;         // 目标模块
    WcetConfig config;              // 配置
    BasicBlock* blocks;             // 基本块数组
    uint32_t block_count;           // 基本块数量
    LoopBound loop_bounds[WCET_MAX_LOOP_BOUNDS]; // 循环边界标注
    uint32_t loop_bound_count;      // 标注数量
    Instruction* code;              // 快捷指针
    uint32_t code_size;             // 指令总数
    WcetResult result;              // 分析结果
    uint32_t func_call_stack[64];   // 函数调用链（防止递归死循环）
    uint32_t func_call_depth;       // 调用深度
};

// ============================================================================
// 配置与工具函数
// ============================================================================

void wcet_config_init(WcetConfig* config) {
    if (!config) return;
    config->cpu_frequency_mhz = 168.0;    // 默认 168MHz (STM32F4)
    config->max_function_depth = 64;      // 最大 64 层调用
    config->default_loop_bound = 0;       // 未标注则报错
    config->enable_unreachable_check = true;
    config->verbose = false;
}

/**
 * @brief 计算单条指令的周期成本
 */
static uint64_t compute_instruction_cycles(Opcode opcode) {
    if (opcode >= OP_COUNT) return 0;
    const InstructionCost* c = &g_cost_table[opcode];
    return (uint64_t)(c->base + c->memory + c->branch_penalty);
}

// ============================================================================
// 基本块构建
// ============================================================================

/**
 * @brief 构建所有基本块
 *
 * 从字节码中识别基本块边界：
 * 1. 程序入口点
 * 2. 跳转目标地址
 * 3. 跳转指令的下一条指令
 */
static ErrorCode wcet_build_blocks(WcetAnalyzer* a) {
    if (!a || !a->module || !a->module->instructions) return ERR_RUNTIME;

    a->code = a->module->instructions;
    a->code_size = a->module->instruction_count;

    // 第一步：标记所有基本块起始点
    bool* is_block_start = (bool*)mmgr_alloc(sizeof(bool) * a->code_size);
    if (!is_block_start) return ERR_OUT_OF_MEMORY;

    memset(is_block_start, 0, sizeof(bool) * a->code_size);

    // 指令 0 总是块起始
    is_block_start[0] = true;

    // 扫描所有指令，标记跳转目标和跳转后指令
    for (uint32_t i = 0; i < a->code_size; i++) {
        Instruction* instr = &a->code[i];
        Opcode op = instr->opcode;

        if (op == OP_JMP) {
            // 无条件跳转：目标地址是新块
            if (instr->operand < a->code_size) {
                is_block_start[instr->operand] = true;
            }
            if (i + 1 < a->code_size) {
                is_block_start[i + 1] = true;
            }
        } else if (op == OP_CALL || op == OP_CALL_EXT) {
            // 调用：operand 是函数表索引，不是地址
            // 不标记 operand 为块起始（避免虚假 CFG 边）
            // 调用返回后继续执行：i+1 是块起始
            if (i + 1 < a->code_size) {
                is_block_start[i + 1] = true;
            }
        } else if (op == OP_JZ || op == OP_JNZ) {
            // 条件跳转：目标地址是新块
            if (instr->operand < a->code_size) {
                is_block_start[instr->operand] = true;
            }
            // 下一条指令也可能被执行（fallthrough 路径）
            if (i + 1 < a->code_size) {
                is_block_start[i + 1] = true;
            }
        } else if (op == OP_RET || op == OP_HALT) {
            // 返回/停机后是新块
            if (i + 1 < a->code_size) {
                is_block_start[i + 1] = true;
            }
        }
    }

    // 第二步：构建基本块
    a->blocks = (BasicBlock*)mmgr_alloc(sizeof(BasicBlock) * WCET_MAX_BLOCKS);
    if (!a->blocks) {
        mmgr_free(is_block_start);
        return ERR_OUT_OF_MEMORY;
    }
    memset(a->blocks, 0, sizeof(BasicBlock) * WCET_MAX_BLOCKS);
    a->block_count = 0;

    uint32_t block_start = 0;
    for (uint32_t i = 1; i <= a->code_size; i++) {
        if (i == a->code_size || is_block_start[i]) {
            if (a->block_count >= WCET_MAX_BLOCKS) {
                mmgr_free(is_block_start);
                return ERR_RUNTIME;
            }

            BasicBlock* bb = &a->blocks[a->block_count++];
            bb->start_pc = block_start;
            bb->end_pc = i;
            bb->cycle_cost = 0;
            bb->instruction_count = 0;
            bb->terminator = BT_FALLTHROUGH;
            bb->target_pc[0] = UINT32_MAX;
            bb->target_pc[1] = UINT32_MAX;
            bb->target_count = 0;
            bb->visited = false;
            bb->in_loop = false;
            bb->loop_multiplier = 1;

            // 计算块内指令成本和类型
            for (uint32_t j = block_start; j < i; j++) {
                Instruction* instr = &a->code[j];
                bb->cycle_cost += compute_instruction_cycles(instr->opcode);
                bb->instruction_count++;

                // 只处理块的终结指令
                if (j == i - 1) {
                    Opcode op = instr->opcode;
                    if (op == OP_JMP) {
                        bb->terminator = BT_JUMP;
                        bb->target_pc[0] = instr->operand;
                        bb->target_count = 1;
                    } else if (op == OP_JZ || op == OP_JNZ) {
                        bb->terminator = BT_BRANCH;
                        bb->target_pc[0] = instr->operand;
                        bb->target_pc[1] = i; // fallthrough 到下个块
                        bb->target_count = 2;
                    } else if (op == OP_CALL) {
                        bb->terminator = BT_CALL;
                        // CALL operand 是函数表索引，需转换为实际地址
                        if (instr->operand < a->module->function_count) {
                            bb->target_pc[0] = a->module->functions[instr->operand].address;
                        } else {
                            bb->target_pc[0] = instr->operand;
                        }
                        bb->target_pc[1] = i; // 调用返回后继续
                        bb->target_count = 2;
                    } else if (op == OP_CALL_EXT) {
                        bb->terminator = BT_CALL;
                        bb->target_pc[0] = instr->operand; // 外部调用保持索引
                        bb->target_pc[1] = i;
                        bb->target_count = 2;
                    } else if (op == OP_RET) {
                        bb->terminator = BT_RETURN;
                        bb->target_count = 0;
                    } else if (op == OP_HALT) {
                        bb->terminator = BT_HALT;
                        bb->target_count = 0;
                    } else {
                        bb->terminator = BT_FALLTHROUGH;
                        bb->target_pc[0] = i;
                        bb->target_count = 1;
                    }
                }
            }

            block_start = i;
        }
    }

    mmgr_free(is_block_start);
    return OK;
}

// ============================================================================
// 循环检测
// ============================================================================

/**
 * @brief 检测基本块中的循环
 *
 * 如果一个块包含向回跳转（目标地址 <= 自身起始地址），
 * 则该块形成一个循环。
 */
static void wcet_detect_loops(WcetAnalyzer* a) {
    for (uint32_t i = 0; i < a->block_count; i++) {
        BasicBlock* bb = &a->blocks[i];
        if (bb->terminator == BT_JUMP && bb->target_pc[0] <= bb->start_pc) {
            // 向后跳转 = 循环
            bb->in_loop = true;
            a->result.analyzed_loops++;

            // 查找循环边界标注
            bool found_bound = false;
            for (uint32_t j = 0; j < a->loop_bound_count; j++) {
                if (a->loop_bounds[j].address == bb->start_pc) {
                    bb->loop_multiplier = a->loop_bounds[j].max_iterations;
                    found_bound = true;
                    break;
                }
            }

            if (!found_bound) {
                if (a->config.default_loop_bound > 0) {
                    bb->loop_multiplier = a->config.default_loop_bound;
                    if (a->config.verbose) {
                        printf("[WCET] 未标注循环 @PC=%u, 使用默认边界 %u\n",
                               bb->start_pc, a->config.default_loop_bound);
                    }
                } else {
                    // 未标注且无默认值 → 报错
                    snprintf(a->result.error_msg, sizeof(a->result.error_msg),
                             "Unbounded loop detected at PC=%u. Use @LOOP_MAX annotation.",
                             bb->start_pc);
                    a->result.analysis_complete = false;
                }
            }

            // 记录最大迭代次数
            if (bb->loop_multiplier > a->result.max_loop_iterations) {
                a->result.max_loop_iterations = bb->loop_multiplier;
            }
        }
    }
}

// ============================================================================
// 函数调用分析
// ============================================================================

/**
 * @brief 在地址 addr 中查找函数信息
 */
static FunctionEntry* find_function_at(WcetAnalyzer* a, uint32_t addr) {
    for (uint32_t i = 0; i < a->module->function_count; i++) {
        if (a->module->functions[i].address == addr) {
            return &a->module->functions[i];
        }
    }
    return NULL;
}

/**
 * @brief 递归分析函数调用链
 */
static uint64_t wcet_analyze_function(WcetAnalyzer* a, uint32_t start_addr,
                                       uint32_t depth, uint32_t* func_count);

/**
 * @brief WCET 分析的保守上界方法（带递归保护）
 */
static uint64_t wcet_compute_conservative_bound(WcetAnalyzer* a, uint32_t start_addr,
                                                  uint32_t depth, uint32_t* func_count) {
    // 检查是否已在调用链中（防止无限递归）
    for (uint32_t i = 0; i < a->func_call_depth; i++) {
        if (a->func_call_stack[i] == start_addr) return 0;
    }
    if (a->func_call_depth >= 64 || depth > a->config.max_function_depth) return 0;
    
    a->func_call_stack[a->func_call_depth++] = start_addr;
    
    bool* block_visited = (bool*)mmgr_alloc(sizeof(bool) * a->block_count);
    if (!block_visited) { a->func_call_depth--; return 0; }
    memset(block_visited, 0, sizeof(bool) * a->block_count);

    uint64_t total = 0;
    bool active = false;

    for (uint32_t i = 0; i < a->block_count; i++) {
        if (a->blocks[i].start_pc >= start_addr && !block_visited[i]) {
            BasicBlock* bb = &a->blocks[i];
            block_visited[i] = true;

            if (bb->start_pc == start_addr) active = true;
            if (!active) continue;

            uint64_t cost = bb->cycle_cost;
            if (bb->in_loop) {
                cost *= bb->loop_multiplier;
                a->result.loop_overhead += bb->cycle_cost * (bb->loop_multiplier - 1);
            }
            total += cost;

            if (bb->terminator == BT_CALL) {
                FunctionEntry* func = find_function_at(a, bb->target_pc[0]);
                if (func) {
                    (*func_count)++;
                    total += wcet_compute_conservative_bound(a, func->address,
                                                              depth + 1, func_count);
                    if (depth + 1 > a->result.max_function_depth) {
                        a->result.max_function_depth = depth + 1;
                    }
                }
            }

            if (bb->terminator == BT_RETURN || bb->terminator == BT_HALT) {
                break;
            }
        }
    }

    mmgr_free(block_visited);
    a->func_call_depth--;
    return total;
}

static uint64_t wcet_analyze_function(WcetAnalyzer* a, uint32_t start_addr,
                                       uint32_t depth, uint32_t* func_count) {
    return wcet_compute_conservative_bound(a, start_addr, depth, func_count);
}

// ============================================================================
// 公共 API
// ============================================================================

WcetAnalyzer* wcet_analyzer_create(BytecodeModule* module, const WcetConfig* config) {
    if (!module) return NULL;

    WcetAnalyzer* a = (WcetAnalyzer*)mmgr_alloc(sizeof(WcetAnalyzer));
    if (!a) return NULL;
    memset(a, 0, sizeof(WcetAnalyzer));

    a->module = module;
    if (config) {
        memcpy(&a->config, config, sizeof(WcetConfig));
    } else {
        wcet_config_init(&a->config);
    }

    a->loop_bound_count = 0;
    a->blocks = NULL;
    a->block_count = 0;

    return a;
}

void wcet_analyzer_free(WcetAnalyzer* analyzer) {
    if (!analyzer) return;
    if (analyzer->blocks) mmgr_free(analyzer->blocks);
    mmgr_free(analyzer);
}

ErrorCode wcet_add_loop_bound(WcetAnalyzer* analyzer, const char* func_name,
                               uint32_t address, uint32_t max_iterations) {
    if (!analyzer || !func_name) return ERR_INVALID_ARGUMENT;
    if (analyzer->loop_bound_count >= WCET_MAX_LOOP_BOUNDS) {
        return ERR_OUT_OF_MEMORY;
    }

    LoopBound* lb = &analyzer->loop_bounds[analyzer->loop_bound_count++];
    lb->function_name = mmgr_strdup(func_name);
    lb->address = address;
    lb->max_iterations = max_iterations;

    return OK;
}

ErrorCode wcet_analyze(WcetAnalyzer* analyzer, const char* entry_function,
                        WcetResult* result) {
    if (!analyzer || !result) return ERR_INVALID_ARGUMENT;

    memset(result, 0, sizeof(WcetResult));
    memcpy(&analyzer->result, result, sizeof(WcetResult));
    analyzer->result.analysis_complete = true;
    analyzer->result.cpu_frequency_mhz = analyzer->config.cpu_frequency_mhz;

    // 步骤 1：构建基本块
    ErrorCode err = wcet_build_blocks(analyzer);
    if (err != OK) {
        snprintf(result->error_msg, sizeof(result->error_msg),
                 "Failed to build basic blocks");
        return err;
    }

    if (analyzer->config.verbose) {
        printf("[WCET] 构建了 %u 个基本块\n", analyzer->block_count);
    }

    // 步骤 2：检测循环
    wcet_detect_loops(analyzer);
    if (!analyzer->result.analysis_complete) {
        memcpy(result, &analyzer->result, sizeof(WcetResult));
        if (analyzer->blocks) { mmgr_free(analyzer->blocks); analyzer->blocks = NULL; }
        return ERR_RUNTIME;
    }

    // 步骤 3：从入口点分析
    uint32_t entry_addr = analyzer->module->entry_point;

    if (entry_function) {
        // 查找指定入口函数
        FunctionEntry* func = bytecode_find_function(analyzer->module, entry_function);
        if (!func) {
            // 不区分大小写查找
            for (uint32_t i = 0; i < analyzer->module->function_count; i++) {
                if (strcasecmp(analyzer->module->functions[i].name, entry_function) == 0) {
                    func = &analyzer->module->functions[i];
                    break;
                }
            }
            if (!func) {
                snprintf(result->error_msg, sizeof(result->error_msg),
                         "Entry function '%s' not found", entry_function);
                if (analyzer->blocks) { mmgr_free(analyzer->blocks); analyzer->blocks = NULL; }
                return ERR_NOT_FOUND;
            }
        }
        entry_addr = func->address;
    }

    analyzer->func_call_depth = 0;
    memset(analyzer->func_call_stack, 0, sizeof(analyzer->func_call_stack));
    uint32_t func_count = 0;
    uint64_t total_cycles = wcet_analyze_function(analyzer, entry_addr, 0, &func_count);

    // 计算总指令数
    uint64_t total_instructions = 0;
    for (uint32_t i = 0; i < analyzer->block_count; i++) {
        BasicBlock* bb = &analyzer->blocks[i];
        uint64_t inst_count = bb->instruction_count;
        if (bb->in_loop) {
            inst_count *= bb->loop_multiplier;
        }
        total_instructions += inst_count;
    }

    // 填充结果
    analyzer->result.max_cycles = total_cycles;
    analyzer->result.max_instructions = total_instructions;
    analyzer->result.analyzed_functions = func_count;
    analyzer->result.instructions_no_loops = total_instructions - 
        analyzer->result.loop_overhead / wcet_instruction_cost(OP_NOP); // 估算

    // 计算微秒数：cycles / (freq_MHz)
    if (analyzer->config.cpu_frequency_mhz > 0.0) {
        analyzer->result.wcet_microseconds = 
            (double)total_cycles / analyzer->config.cpu_frequency_mhz;
    }

    // 复制结果
    memcpy(result, &analyzer->result, sizeof(WcetResult));

    // 清理临时数据
    if (analyzer->blocks) {
        mmgr_free(analyzer->blocks);
        analyzer->blocks = NULL;
    }

    return OK;
}

void wcet_print_report(const WcetResult* result) {
    if (!result) return;

    printf("\n");
    printf("============================================================\n");
    printf("  WCET (Worst-Case Execution Time) 分析报告\n");
    printf("============================================================\n");
    printf("  分析状态:       %s\n", result->analysis_complete ? "完整" : "不完整");
    printf("  CPU 频率:       %.1f MHz\n", result->cpu_frequency_mhz);
    printf("------------------------------------------------------------\n");
    printf("  最坏路径指令数:  %llu\n", (unsigned long long)result->max_instructions);
    printf("  最坏路径周期数:  %llu\n", (unsigned long long)result->max_cycles);
    printf("  估算 WCET:       %.2f µs\n", result->wcet_microseconds);
    printf("------------------------------------------------------------\n");
    printf("  线性路径指令数:  %llu\n", (unsigned long long)result->instructions_no_loops);
    printf("  循环额外开销:    %llu 指令\n", (unsigned long long)result->loop_overhead);
    printf("  最大函数深度:    %llu\n", (unsigned long long)result->max_function_depth);
    printf("  最大循环迭代:    %u\n", result->max_loop_iterations);
    printf("------------------------------------------------------------\n");
    printf("  已分析函数数:    %u\n", result->analyzed_functions);
    printf("  已分析循环数:    %u\n", result->analyzed_loops);
    printf("  不可达代码块:    %u\n", result->unreachable_code);
    printf("============================================================\n");

    if (!result->analysis_complete && result->error_msg[0]) {
        printf("  ⚠ 警告: %s\n", result->error_msg);
        printf("============================================================\n");
    }
    printf("\n");
}

bool wcet_validate_cycle_budget(const WcetResult* result,
                                 uint32_t cycle_time_ms,
                                 uint32_t safety_margin_percent) {
    if (!result || cycle_time_ms == 0) return false;

    double budget_us = (double)cycle_time_ms * 1000.0;
    double margin_factor = 1.0 + (double)safety_margin_percent / 100.0;
    double required_budget_us = result->wcet_microseconds * margin_factor;

    printf("\n  --- 周期预算验证 ---\n");
    printf("  WCET:          %.2f µs\n", result->wcet_microseconds);
    printf("  目标周期:      %.2f µs (%u ms)\n", budget_us, cycle_time_ms);
    printf("  安全裕度:      %u%%\n", safety_margin_percent);
    printf("  需要预算:      %.2f µs\n", required_budget_us);
    printf("  剩余:          %.2f µs\n", budget_us - required_budget_us);

    if (required_budget_us <= budget_us) {
        printf("  结果:          ✓ 通过 (WCET + 裕度 ≤ 周期预算)\n\n");
        return true;
    } else {
        printf("  结果:          ✗ 失败 (WCET + 裕度 > 周期预算)\n\n");
        return false;
    }
}

uint32_t wcet_parse_loop_bounds_from_source(const char* source_file,
                                              LoopBound* bounds,
                                              uint32_t max_bounds) {
    if (!source_file || !bounds || max_bounds == 0) return 0;

    FILE* fp = fopen(source_file, "r");
    if (!fp) return 0;

    uint32_t count = 0;
    char line[1024];

    while (count < max_bounds && fgets(line, sizeof(line), fp)) {
        // 查找 @LOOP_MAX(N) 或 @LOOP_MAX( N ) 标注
        const char* annotation = strstr(line, "@LOOP_MAX");
        if (!annotation) continue;

        // 提取 N 值
        const char* open = strchr(annotation, '(');
        const char* close = open ? strchr(open, ')') : NULL;
        if (!open || !close || close <= open + 1) continue;

        char num_str[32];
        size_t len = (size_t)(close - open - 1);
        if (len >= sizeof(num_str)) len = sizeof(num_str) - 1;
        memcpy(num_str, open + 1, len);
        num_str[len] = '\0';

        uint32_t max_iter = (uint32_t)atoi(num_str);
        if (max_iter == 0) continue;

        // 查找该标注所在位置的函数名
        // 回退到最近的 FUNCTION 或 PROGRAM 声明
        // 简化处理：使用 "main" 作为默认函数名
        bounds[count].function_name = mmgr_strdup("main");
        bounds[count].address = 0; // 地址需要编译后才能确定
        bounds[count].max_iterations = max_iter;
        count++;
    }

    fclose(fp);
    return count;
}