/*
 * IEC61131 字节码系统实现
 * 包括指令集、文件格式、生成器和调试功能
 */

#include "bytecode.h"
#include "mmgr.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* 指令信息表 */
static const instr_info_t g_instr_info_table[OP_INSTRUCTION_COUNT] = {
    /* 控制指令 */
    {"NOP", OPERAND_NONE, "No operation"},
    {"HALT", OPERAND_NONE, "Halt execution"},
    
    /* 常量加载指令 */
    {"LOAD_CONST_INT", OPERAND_INT, "Load integer constant"},
    {"LOAD_CONST_REAL", OPERAND_REAL, "Load real constant"},
    {"LOAD_CONST_BOOL", OPERAND_INT, "Load boolean constant"},
    {"LOAD_CONST_STRING", OPERAND_STRING, "Load string constant"},
    
    /* 变量操作指令 */
    {"LOAD_LOCAL", OPERAND_INT, "Load local variable"},
    {"LOAD_GLOBAL", OPERAND_INT, "Load global variable"},
    {"LOAD_PARAM", OPERAND_INT, "Load parameter"},
    {"STORE_LOCAL", OPERAND_INT, "Store to local variable"},
    {"STORE_GLOBAL", OPERAND_INT, "Store to global variable"},
    {"STORE_PARAM", OPERAND_INT, "Store to parameter"},
    
    /* 栈操作指令 */
    {"PUSH", OPERAND_NONE, "Push to stack"},
    {"POP", OPERAND_NONE, "Pop from stack"},
    {"DUP", OPERAND_NONE, "Duplicate top of stack"},
    {"SWAP", OPERAND_NONE, "Swap top two stack elements"},
    
    /* 算术运算指令 - 整数 */
    {"ADD_INT", OPERAND_NONE, "Integer addition"},
    {"SUB_INT", OPERAND_NONE, "Integer subtraction"},
    {"MUL_INT", OPERAND_NONE, "Integer multiplication"},
    {"DIV_INT", OPERAND_NONE, "Integer division"},
    {"MOD_INT", OPERAND_NONE, "Integer modulo"},
    {"NEG_INT", OPERAND_NONE, "Integer negation"},
    
    /* 算术运算指令 - 实数 */
    {"ADD_REAL", OPERAND_NONE, "Real addition"},
    {"SUB_REAL", OPERAND_NONE, "Real subtraction"},
    {"MUL_REAL", OPERAND_NONE, "Real multiplication"},
    {"DIV_REAL", OPERAND_NONE, "Real division"},
    {"NEG_REAL", OPERAND_NONE, "Real negation"},
    
    /* 逻辑运算指令 */
    {"AND_BOOL", OPERAND_NONE, "Boolean AND"},
    {"OR_BOOL", OPERAND_NONE, "Boolean OR"},
    {"XOR_BOOL", OPERAND_NONE, "Boolean XOR"},
    {"NOT_BOOL", OPERAND_NONE, "Boolean NOT"},
    
    /* 比较指令 - 整数 */
    {"EQ_INT", OPERAND_NONE, "Integer equal"},
    {"NE_INT", OPERAND_NONE, "Integer not equal"},
    {"LT_INT", OPERAND_NONE, "Integer less than"},
    {"LE_INT", OPERAND_NONE, "Integer less equal"},
    {"GT_INT", OPERAND_NONE, "Integer greater than"},
    {"GE_INT", OPERAND_NONE, "Integer greater equal"},
    
    /* 比较指令 - 实数 */
    {"EQ_REAL", OPERAND_NONE, "Real equal"},
    {"NE_REAL", OPERAND_NONE, "Real not equal"},
    {"LT_REAL", OPERAND_NONE, "Real less than"},
    {"LE_REAL", OPERAND_NONE, "Real less equal"},
    {"GT_REAL", OPERAND_NONE, "Real greater than"},
    {"GE_REAL", OPERAND_NONE, "Real greater equal"},
    
    /* 比较指令 - 字符串 */
    {"EQ_STRING", OPERAND_NONE, "String equal"},
    {"NE_STRING", OPERAND_NONE, "String not equal"},
    {"LT_STRING", OPERAND_NONE, "String less than"},
    {"LE_STRING", OPERAND_NONE, "String less equal"},
    {"GT_STRING", OPERAND_NONE, "String greater than"},
    {"GE_STRING", OPERAND_NONE, "String greater equal"},
    
    /* 类型转换指令 */
    {"INT_TO_REAL", OPERAND_NONE, "Convert int to real"},
    {"REAL_TO_INT", OPERAND_NONE, "Convert real to int"},
    {"INT_TO_STRING", OPERAND_NONE, "Convert int to string"},
    {"REAL_TO_STRING", OPERAND_NONE, "Convert real to string"},
    {"BOOL_TO_STRING", OPERAND_NONE, "Convert bool to string"},
    {"STRING_TO_INT", OPERAND_NONE, "Convert string to int"},
    {"STRING_TO_REAL", OPERAND_NONE, "Convert string to real"},
    
    /* 控制流指令 */
    {"JMP", OPERAND_ADDRESS, "Unconditional jump"},
    {"JMP_TRUE", OPERAND_ADDRESS, "Jump if true"},
    {"JMP_FALSE", OPERAND_ADDRESS, "Jump if false"},
    {"JMP_EQ", OPERAND_ADDRESS, "Jump if equal"},
    {"JMP_NE", OPERAND_ADDRESS, "Jump if not equal"},
    
    /* 函数调用指令 */
    {"CALL", OPERAND_ADDRESS, "Function call"},
    {"CALL_BUILTIN", OPERAND_INT, "Builtin function call"},
    {"CALL_LIBRARY", OPERAND_INT, "Library function call"},
    {"RET", OPERAND_NONE, "Function return"},
    {"RET_VALUE", OPERAND_NONE, "Return with value"},
    
    /* 数组操作指令 */
    {"ARRAY_LOAD", OPERAND_NONE, "Load array element"},
    {"ARRAY_STORE", OPERAND_NONE, "Store array element"},
    {"ARRAY_LEN", OPERAND_NONE, "Get array length"},
    
    /* 结构体操作指令 */
    {"STRUCT_LOAD", OPERAND_INT, "Load struct member"},
    {"STRUCT_STORE", OPERAND_INT, "Store struct member"},
    
    /* 调试指令 */
    {"DEBUG_PRINT", OPERAND_NONE, "Debug print"},
    {"BREAKPOINT", OPERAND_INT, "Breakpoint"},
    {"LINE_INFO", OPERAND_INT, "Line number info"},
    
    /* 同步指令 */
    {"SYNC_VAR", OPERAND_INT, "Synchronize variable"},
    {"SYNC_CHECKPOINT", OPERAND_NONE, "Synchronization checkpoint"}
};

/* ========== 字节码生成器实现 ========== */

/* 创建字节码生成器 */
bytecode_generator_t* bytecode_generator_create(void) {
    bytecode_generator_t *gen = (bytecode_generator_t*)mmgr_alloc_general(sizeof(bytecode_generator_t));
    if (!gen) {
        return NULL;
    }
    
    memset(gen, 0, sizeof(bytecode_generator_t));
    
    /* 初始化指令缓冲区 */
    gen->instr_capacity = 1000;
    gen->instructions = (bytecode_instr_t*)mmgr_alloc_general(
        sizeof(bytecode_instr_t) * gen->instr_capacity);
    
    /* 初始化常量池 */
    gen->const_capacity = 100;
    gen->const_pool = (const_pool_item_t*)mmgr_alloc_general(
        sizeof(const_pool_item_t) * gen->const_capacity);
    
    /* 初始化变量表 */
    gen->var_capacity = 200;
    gen->var_table = (var_descriptor_t*)mmgr_alloc_general(
        sizeof(var_descriptor_t) * gen->var_capacity);
    
    /* 初始化函数表 */
    gen->func_capacity = 50;
    gen->func_table = (func_descriptor_t*)mmgr_alloc_general(
        sizeof(func_descriptor_t) * gen->func_capacity);
    
    if (!gen->instructions || !gen->const_pool || !gen->var_table || !gen->func_table) {
        bytecode_generator_destroy(gen);
        return NULL;
    }
    
    return gen;
}

/* 销毁字节码生成器 */
void bytecode_generator_destroy(bytecode_generator_t *gen) {
    if (!gen) return;
    
    /* 释放字符串常量内存 */
    for (uint32_t i = 0; i < gen->const_count; i++) {
        if (gen->const_pool[i].type == CONST_STRING) {
            if (gen->const_pool[i].value.string_val.data) {
                // 由MMGR管理，不需要显式释放
            }
        }
    }
    
    /* 其他内存由MMGR管理 */
}

/* 重置生成器状态 */
void bytecode_generator_reset(bytecode_generator_t *gen) {
    if (!gen) return;
    
    gen->instr_count = 0;
    gen->const_count = 0;
    gen->var_count = 0;
    gen->func_count = 0;
    gen->label_count = 0;
    gen->current_line = 0;
    gen->current_column = 0;
}

/* ========== 指令生成函数 ========== */

/* 生成无操作数指令 */
uint32_t bytecode_emit_instr(bytecode_generator_t *gen, opcode_t opcode) {
    if (!gen || gen->instr_count >= gen->instr_capacity) {
        return UINT32_MAX;
    }
    
    uint32_t index = gen->instr_count++;
    bytecode_instr_t *instr = &gen->instructions[index];
    
    instr->opcode = opcode;
    instr->operand_type = OPERAND_NONE;
    instr->source_line = gen->current_line;
    instr->source_column = gen->current_column;
    
    return index;
}

/* 生成带整数操作数的指令 */
uint32_t bytecode_emit_instr_int(bytecode_generator_t *gen, opcode_t opcode, int32_t operand) {
    if (!gen || gen->instr_count >= gen->instr_capacity) {
        return UINT32_MAX;
    }
    
    uint32_t index = gen->instr_count++;
    bytecode_instr_t *instr = &gen->instructions[index];
    
    instr->opcode = opcode;
    instr->operand_type = OPERAND_INT;
    instr->operand.int_operand = operand;
    instr->source_line = gen->current_line;
    instr->source_column = gen->current_column;
    
    return index;
}

/* 生成带实数操作数的指令 */
uint32_t bytecode_emit_instr_real(bytecode_generator_t *gen, opcode_t opcode, double operand) {
    if (!gen || gen->instr_count >= gen->instr_capacity) {
        return UINT32_MAX;
    }
    
    uint32_t index = gen->instr_count++;
    bytecode_instr_t *instr = &gen->instructions[index];
    
    instr->opcode = opcode;
    instr->operand_type = OPERAND_REAL;
    instr->operand.real_operand = operand;
    instr->source_line = gen->current_line;
    instr->source_column = gen->current_column;
    
    return index;
}

/* 生成带地址操作数的指令 */
uint32_t bytecode_emit_instr_addr(bytecode_generator_t *gen, opcode_t opcode, uint32_t address) {
    if (!gen || gen->instr_count >= gen->instr_capacity) {
        return UINT32_MAX;
    }
    
    uint32_t index = gen->instr_count++;
    bytecode_instr_t *instr = &gen->instructions[index];
    
    instr->opcode = opcode;
    instr->operand_type = OPERAND_ADDRESS;
    instr->operand.addr_operand = address;
    instr->source_line = gen->current_line;
    instr->source_column = gen->current_column;
    
    return index;
}

/* 生成带字符串操作数的指令 */
uint32_t bytecode_emit_instr_string(bytecode_generator_t *gen, opcode_t opcode, const char *str) {
    if (!gen || !str || gen->instr_count >= gen->instr_capacity) {
        return UINT32_MAX;
    }
    
    /* 将字符串添加到常量池 */
    uint32_t str_index = bytecode_add_const_string(gen, str);
    if (str_index == UINT32_MAX) {
        return UINT32_MAX;
    }
    
    uint32_t index = gen->instr_count++;
    bytecode_instr_t *instr = &gen->instructions[index];
    
    instr->opcode = opcode;
    instr->operand_type = OPERAND_STRING;
    instr->operand.str_index = str_index;
    instr->source_line = gen->current_line;
    instr->source_column = gen->current_column;
    
    return index;
}

/* ========== 常量池管理 ========== */

/* 添加整数常量 */
uint32_t bytecode_add_const_int(bytecode_generator_t *gen, int32_t value) {
    if (!gen || gen->const_count >= gen->const_capacity) {
        return UINT32_MAX;
    }
    
    /* 检查是否已存在相同常量 */
    for (uint32_t i = 0; i < gen->const_count; i++) {
        if (gen->const_pool[i].type == CONST_INT && 
            gen->const_pool[i].value.int_val == value) {
            return i;
        }
    }
    
    /* 添加新常量 */
    uint32_t index = gen->const_count++;
    const_pool_item_t *item = &gen->const_pool[index];
    
    item->type = CONST_INT;
    item->value.int_val = value;
    
    return index;
}

/* 添加实数常量 */
uint32_t bytecode_add_const_real(bytecode_generator_t *gen, double value) {
    if (!gen || gen->const_count >= gen->const_capacity) {
        return UINT32_MAX;
    }
    
    /* 检查是否已存在相同常量 */
    for (uint32_t i = 0; i < gen->const_count; i++) {
        if (gen->const_pool[i].type == CONST_REAL && 
            gen->const_pool[i].value.real_val == value) {
            return i;
        }
    }
    
    /* 添加新常量 */
    uint32_t index = gen->const_count++;
    const_pool_item_t *item = &gen->const_pool[index];
    
    item->type = CONST_REAL;
    item->value.real_val = value;
    
    return index;
}

/* 添加布尔常量 */
uint32_t bytecode_add_const_bool(bytecode_generator_t *gen, bool value) {
    if (!gen || gen->const_count >= gen->const_capacity) {
        return UINT32_MAX;
    }
    
    /* 检查是否已存在相同常量 */
    for (uint32_t i = 0; i < gen->const_count; i++) {
        if (gen->const_pool[i].type == CONST_BOOL && 
            gen->const_pool[i].value.bool_val == value) {
            return i;
        }
    }
    
    /* 添加新常量 */
    uint32_t index = gen->const_count++;
    const_pool_item_t *item = &gen->const_pool[index];
    
    item->type = CONST_BOOL;
    item->value.bool_val = value;
    
    return index;
}

/* 添加字符串常量 */
uint32_t bytecode_add_const_string(bytecode_generator_t *gen, const char *str) {
    if (!gen || !str || gen->const_count >= gen->const_capacity) {
        return UINT32_MAX;
    }
    
    /* 检查是否已存在相同常量 */
    for (uint32_t i = 0; i < gen->const_count; i++) {
        if (gen->const_pool[i].type == CONST_STRING && 
            gen->const_pool[i].value.string_val.data &&
            strcmp(gen->const_pool[i].value.string_val.data, str) == 0) {
            return i;
        }
    }
    
    /* 添加新常量 */
    uint32_t index = gen->const_count++;
    const_pool_item_t *item = &gen->const_pool[index];
    
    item->type = CONST_STRING;
    item->value.string_val.length = strlen(str);
    item->value.string_val.data = mmgr_alloc_string(str);
    
    if (!item->value.string_val.data) {
        gen->const_count--; /* 回滚 */
        return UINT32_MAX;
    }
    
    return index;
}

/* ========== 变量和函数管理 ========== */

/* 添加变量描述符 */
uint32_t bytecode_add_variable(bytecode_generator_t *gen, const char *name, 
                               uint32_t type, uint32_t size, bool is_global) {
    if (!gen || !name || gen->var_count >= gen->var_capacity) {
        return UINT32_MAX;
    }
    
    uint32_t index = gen->var_count++;
    var_descriptor_t *var = &gen->var_table[index];
    
    strncpy(var->name, name, sizeof(var->name) - 1);
    var->name[sizeof(var->name) - 1] = '\0';
    var->type = type;
    var->size = size;
    var->is_global = is_global;
    var->offset = 0; /* 稍后计算 */
    
    return index;
}

/* 添加函数描述符 */
uint32_t bytecode_add_function(bytecode_generator_t *gen, const char *name,
                               uint32_t param_count, uint32_t return_type) {
    if (!gen || !name || gen->func_count >= gen->func_capacity) {
        return UINT32_MAX;
    }
    
    uint32_t index = gen->func_count++;
    func_descriptor_t *func = &gen->func_table[index];
    
    strncpy(func->name, name, sizeof(func->name) - 1);
    func->name[sizeof(func->name) - 1] = '\0';
    func->param_count = param_count;
    func->return_type = return_type;
    func->address = 0; /* 稍后设置 */
    func->local_var_size = 0; /* 稍后计算 */
    
    return index;
}

/* ========== 标签和跳转管理 ========== */

/* 创建标签 */
uint32_t bytecode_create_label(bytecode_generator_t *gen, const char *name) {
    if (!gen || !name || gen->label_count >= 256) {
        return UINT32_MAX;
    }
    
    /* 检查标签是否已存在 */
    for (uint32_t i = 0; i < gen->label_count; i++) {
        if (strcmp(gen->labels[i].name, name) == 0) {
            return i;
        }
    }
    
    /* 创建新标签 */
    uint32_t index = gen->label_count++;
    strncpy(gen->labels[index].name, name, sizeof(gen->labels[index].name) - 1);
    gen->labels[index].name[sizeof(gen->labels[index].name) - 1] = '\0';
    gen->labels[index].address = 0;
    gen->labels[index].is_resolved = false;
    
    return index;
}

/* 标记标签位置 */
void bytecode_mark_label(bytecode_generator_t *gen, const char *name) {
    if (!gen || !name) return;
    
    for (uint32_t i = 0; i < gen->label_count; i++) {
        if (strcmp(gen->labels[i].name, name) == 0) {
            gen->labels[i].address = gen->instr_count;
            gen->labels[i].is_resolved = true;
            return;
        }
    }
}

/* 获取标签地址 */
uint32_t bytecode_get_label_address(bytecode_generator_t *gen, const char *name) {
    if (!gen || !name) return UINT32_MAX;
    
    for (uint32_t i = 0; i < gen->label_count; i++) {
        if (strcmp(gen->labels[i].name, name) == 0 && gen->labels[i].is_resolved) {
            return gen->labels[i].address;
        }
    }
    
    return UINT32_MAX;
}

/* 修补跳转指令 */
void bytecode_patch_jump(bytecode_generator_t *gen, uint32_t instr_index, uint32_t target_addr) {
    if (!gen || instr_index >= gen->instr_count) return;
    
    bytecode_instr_t *instr = &gen->instructions[instr_index];
    if (instr->operand_type == OPERAND_ADDRESS) {
        instr->operand.addr_operand = target_addr;
    }
}

/* ========== 文件生成和I/O ========== */

/* 生成最终字节码文件 */
int bytecode_generate_file(bytecode_generator_t *gen, bytecode_file_t *bc_file) {
    if (!gen || !bc_file) {
        return -1;
    }
    
    memset(bc_file, 0, sizeof(bytecode_file_t));
    
    /* 填充文件头 */
    memcpy(bc_file->header.magic, BYTECODE_MAGIC, 4);
    bc_file->header.version = BYTECODE_VERSION;
    bc_file->header.flags = 0;
    bc_file->header.instr_count = gen->instr_count;
    bc_file->header.const_pool_size = gen->const_count;
    bc_file->header.var_count = gen->var_count;
    bc_file->header.func_count = gen->func_count;
    bc_file->header.entry_point = 0; /* 默认从第一条指令开始 */
    
    /* 复制指令序列 */
    if (gen->instr_count > 0) {
        size_t instr_size = sizeof(bytecode_instr_t) * gen->instr_count;
        bc_file->instructions = (bytecode_instr_t*)mmgr_alloc_general(instr_size);
        if (!bc_file->instructions) {
            return -1;
        }
        memcpy(bc_file->instructions, gen->instructions, instr_size);
    }
    
    /* 复制常量池 */
    if (gen->const_count > 0) {
        size_t const_size = sizeof(const_pool_item_t) * gen->const_count;
        bc_file->const_pool = (const_pool_item_t*)mmgr_alloc_general(const_size);
        if (!bc_file->const_pool) {
            return -1;
        }
        memcpy(bc_file->const_pool, gen->const_pool, const_size);
    }
    
    /* 复制变量表 */
    if (gen->var_count > 0) {
        size_t var_size = sizeof(var_descriptor_t) * gen->var_count;
        bc_file->var_table = (var_descriptor_t*)mmgr_alloc_general(var_size);
        if (!bc_file->var_table) {
            return -1;
        }
        memcpy(bc_file->var_table, gen->var_table, var_size);
    }
    
    /* 复制函数表 */
    if (gen->func_count > 0) {
        size_t func_size = sizeof(func_descriptor_t) * gen->func_count;
        bc_file->func_table = (func_descriptor_t*)mmgr_alloc_general(func_size);
        if (!bc_file->func_table) {
            return -1;
        }
        memcpy(bc_file->func_table, gen->func_table, func_size);
    }
    
    bc_file->is_loaded = true;
    
    return 0;
}

/* 保存字节码文件 */
int bytecode_save_file(const bytecode_file_t *bc_file, const char *filename) {
    if (!bc_file || !filename) {
        return -1;
    }
    
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        return -1;
    }
    
    /* 写入文件头 */
    if (fwrite(&bc_file->header, sizeof(bytecode_header_t), 1, fp) != 1) {
        fclose(fp);
        return -1;
    }
    
    /* 写入指令序列 */
    if (bc_file->header.instr_count > 0) {
        size_t written = fwrite(bc_file->instructions, sizeof(bytecode_instr_t),
                               bc_file->header.instr_count, fp);
        if (written != bc_file->header.instr_count) {
            fclose(fp);
            return -1;
        }
    }
    
    /* 写入常量池 */
    if (bc_file->header.const_pool_size > 0) {
        for (uint32_t i = 0; i < bc_file->header.const_pool_size; i++) {
            const const_pool_item_t *item = &bc_file->const_pool[i];
            
            /* 写入常量类型 */
            if (fwrite(&item->type, sizeof(const_type_t), 1, fp) != 1) {
                fclose(fp);
                return -1;
            }
            
            /* 写入常量数据 */
            switch (item->type) {
                case CONST_INT:
                    if (fwrite(&item->value.int_val, sizeof(int32_t), 1, fp) != 1) {
                        fclose(fp);
                        return -1;
                    }
                    break;
                case CONST_REAL:
                    if (fwrite(&item->value.real_val, sizeof(double), 1, fp) != 1) {
                        fclose(fp);
                        return -1;
                    }
                    break;
                case CONST_BOOL:
                    if (fwrite(&item->value.bool_val, sizeof(bool), 1, fp) != 1) {
                        fclose(fp);
                        return -1;
                    }
                    break;
                case CONST_STRING:
                    /* 写入字符串长度 */
                    if (fwrite(&item->value.string_val.length, sizeof(uint32_t), 1, fp) != 1) {
                        fclose(fp);
                        return -1;
                    }
                    /* 写入字符串数据 */
                    if (item->value.string_val.length > 0) {
                        if (fwrite(item->value.string_val.data, 1, 
                                  item->value.string_val.length, fp) != item->value.string_val.length) {
                            fclose(fp);
                            return -1;
                        }
                    }
                    break;
            }
        }
    }
    
    /* 写入变量表 */
    if (bc_file->header.var_count > 0) {
        size_t written = fwrite(bc_file->var_table, sizeof(var_descriptor_t),
                               bc_file->header.var_count, fp);
        if (written != bc_file->header.var_count) {
            fclose(fp);
            return -1;
        }
    }
    
    /* 写入函数表 */
    if (bc_file->header.func_count > 0) {
        size_t written = fwrite(bc_file->func_table, sizeof(func_descriptor_t),
                               bc_file->header.func_count, fp);
        if (written != bc_file->header.func_count) {
            fclose(fp);
            return -1;
        }
    }
    
    fclose(fp);
    return 0;
}

/* ========== 指令信息和调试函数 ========== */

/* 获取指令信息 */
const instr_info_t* bytecode_get_instr_info(opcode_t opcode) {
    if (opcode >= OP_INSTRUCTION_COUNT) {
        return NULL;
    }
    return &g_instr_info_table[opcode];
}

/* 获取操作码名称 */
const char* bytecode_get_opcode_name(opcode_t opcode) {
    const instr_info_t *info = bytecode_get_instr_info(opcode);
    return info ? info->name : "UNKNOWN";
}

/* 获取操作数类型 */
operand_type_t bytecode_get_operand_type(opcode_t opcode) {
    const instr_info_t *info = bytecode_get_instr_info(opcode);
    return info ? info->operand_type : OPERAND_NONE;
}

/* 反汇编单条指令 */
void bytecode_disassemble_instr(const bytecode_instr_t *instr, char *buffer, size_t buffer_size) {
    if (!instr || !buffer || buffer_size == 0) {
        return;
    }
    
    const char *opcode_name = bytecode_get_opcode_name(instr->opcode);
    
    switch (instr->operand_type) {
        case OPERAND_NONE:
            snprintf(buffer, buffer_size, "%s", opcode_name);
            break;
        case OPERAND_INT:
            snprintf(buffer, buffer_size, "%s %d", opcode_name, instr->operand.int_operand);
            break;
        case OPERAND_REAL:
            snprintf(buffer, buffer_size, "%s %.6f", opcode_name, instr->operand.real_operand);
            break;
        case OPERAND_ADDRESS:
            snprintf(buffer, buffer_size, "%s 0x%04x", opcode_name, instr->operand.addr_operand);
            break;
        case OPERAND_STRING:
            snprintf(buffer, buffer_size, "%s [str_%u]", opcode_name, instr->operand.str_index);
            break;
        default:
            snprintf(buffer, buffer_size, "%s <??>", opcode_name);
            break;
    }
}

/* 反汇编整个字节码文件 */
void bytecode_disassemble_file(const bytecode_file_t *bc_file, FILE *output) {
    if (!bc_file || !output) return;
    
    fprintf(output, "; ST Bytecode Disassembly\n");
    fprintf(output, "; Version: 0x%08x\n", bc_file->header.version);
    fprintf(output, "; Instructions: %u\n", bc_file->header.instr_count);
    fprintf(output, "; Constants: %u\n", bc_file->header.const_pool_size);
    fprintf(output, "\n");
    
    /* 反汇编指令 */
    for (uint32_t i = 0; i < bc_file->header.instr_count; i++) {
        const bytecode_instr_t *instr = &bc_file->instructions[i];
        char instr_str[256];
        
        bytecode_disassemble_instr(instr, instr_str, sizeof(instr_str));
        fprintf(output, "%04x: %s", i, instr_str);
        
        if (instr->source_line > 0) {
            fprintf(output, "  ; line %u", instr->source_line);
        }
        fprintf(output, "\n");
    }
}

/* 打印文件信息 */
void bytecode_print_file_info(const bytecode_file_t *bc_file) {
    if (!bc_file) return;
    
    printf("=== Bytecode File Information ===\n");
    printf("Magic: %.4s\n", bc_file->header.magic);
    printf("Version: 0x%08x\n", bc_file->header.version);
    printf("Instructions: %u\n", bc_file->header.instr_count);
    printf("Constants: %u\n", bc_file->header.const_pool_size);
    printf("Variables: %u\n", bc_file->header.var_count);
    printf("Functions: %u\n", bc_file->header.func_count);
    printf("Entry Point: 0x%04x\n", bc_file->header.entry_point);
    printf("================================\n");
}

/* 验证字节码文件 */
bool bytecode_verify_file(const bytecode_file_t *bc_file) {
    if (!bc_file) return false;
    
    /* 检查魔数 */
    if (memcmp(bc_file->header.magic, BYTECODE_MAGIC, 4) != 0) {
        return false;
    }
    
    /* 检查版本 */
    if (!bytecode_check_version(bc_file->header.version)) {
        return false;
    }
    
    /* 检查入口点 */
    if (bc_file->header.entry_point >= bc_file->header.instr_count) {
        return false;
    }
    
    /* 验证指令 */
    if (!bytecode_validate_instructions(bc_file)) {
        return false;
    }
    
    return true;
}

/* 检查版本兼容性 */
bool bytecode_check_version(uint32_t file_version) {
    /* 简单的版本检查：主版本号必须匹配 */
    uint32_t file_major = (file_version >> 16) & 0xFFFF;
    uint32_t current_major = (BYTECODE_VERSION >> 16) & 0xFFFF;
    
    return file_major == current_major;
}

/* 验证指令序列 */
bool bytecode_validate_instructions(const bytecode_file_t *bc_file) {
    if (!bc_file || !bc_file->instructions) return false;
    
    for (uint32_t i = 0; i < bc_file->header.instr_count; i++) {
        const bytecode_instr_t *instr = &bc_file->instructions[i];
        
        /* 检查操作码有效性 */
        if (instr->opcode >= OP_INSTRUCTION_COUNT) {
            return false;
        }
        
        /* 检查操作数类型匹配 */
        operand_type_t expected_type = bytecode_get_operand_type(instr->opcode);
        if (instr->operand_type != expected_type) {
            return false;
        }
        
        /* 检查地址操作数范围 */
        if (instr->operand_type == OPERAND_ADDRESS) {
            if (instr->operand.addr_operand >= bc_file->header.instr_count) {
                return false;
            }
        }
        
        /* 检查字符串索引范围 */
        if (instr->operand_type == OPERAND_STRING) {
            if (instr->operand.str_index >= bc_file->header.const_pool_size) {
                return false;
            }
        }
    }
    
    return true;
}

/* 获取错误字符串 */
const char* bytecode_get_error_string(bytecode_error_t error) {
    switch (error) {
        case BYTECODE_OK:
            return "成功";
        case BYTECODE_ERROR_INVALID_FILE:
            return "无效的字节码文件";
        case BYTECODE_ERROR_VERSION_MISMATCH:
            return "版本不匹配";
        case BYTECODE_ERROR_CORRUPTED_DATA:
            return "数据损坏";
        case BYTECODE_ERROR_INVALID_INSTRUCTION:
            return "无效指令";
        case BYTECODE_ERROR_INVALID_OPERAND:
            return "无效操作数";
        case BYTECODE_ERROR_MEMORY_ERROR:
            return "内存错误";
        case BYTECODE_ERROR_IO_ERROR:
            return "I/O错误";
        default:
            return "未知错误";
    }
}
