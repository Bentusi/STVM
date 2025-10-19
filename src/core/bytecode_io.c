/**
 * @file bytecode_io.c
 * @brief 字节码序列化/反序列化的实现
 * 
 * STBC文件格式：
 * 1. 文件头 (STBCHeader)
 * 2. 常量池
 * 3. 函数表
 * 4. 指令数组
 * 5. 行号表（可选）
 */

#include "bytecode_io.h"
#include "mmgr.h"
#include <string.h>

// CRC32表（简化版，用于校验和计算）
static uint32_t crc32_table[256];
static bool crc32_table_initialized = false;

/**
 * @brief 初始化CRC32表
 */
static void init_crc32_table(void) {
    if (crc32_table_initialized) return;
    
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
        crc32_table[i] = crc;
    }
    
    crc32_table_initialized = true;
}

/**
 * @brief 计算CRC32
 */
static uint32_t compute_crc32(const void* data, size_t length) {
    init_crc32_table();
    
    uint32_t crc = 0xFFFFFFFF;
    const uint8_t* bytes = (const uint8_t*)data;
    
    for (size_t i = 0; i < length; i++) {
        uint8_t index = (crc ^ bytes[i]) & 0xFF;
        crc = (crc >> 8) ^ crc32_table[index];
    }
    
    return ~crc;
}

/**
 * @brief 验证文件头
 */
bool bytecode_validate_header(const STBCHeader* header) {
    if (!header) return false;
    
    if (header->magic != STBC_MAGIC) {
        return false;
    }
    
    if (header->version_major != STBC_VERSION_MAJOR) {
        return false;
    }
    
    return true;
}

/**
 * @brief 保存常量池
 */
static ErrorCode save_constants(const BytecodeModule* module, FILE* fp) {
    for (uint32_t i = 0; i < module->const_count; i++) {
        Constant* constant = &module->constants[i];
        
        // 写入类型
        if (fwrite(&constant->type, sizeof(ConstantType), 1, fp) != 1) {
            return ERR_FILE_IO;
        }
        
        // 根据类型写入值
        switch (constant->type) {
            case CONST_INT:
                if (fwrite(&constant->int_val, sizeof(int32_t), 1, fp) != 1) {
                    return ERR_FILE_IO;
                }
                break;
                
            case CONST_REAL:
                if (fwrite(&constant->real_val, sizeof(double), 1, fp) != 1) {
                    return ERR_FILE_IO;
                }
                break;
                
            case CONST_BOOL:
                if (fwrite(&constant->bool_val, sizeof(bool), 1, fp) != 1) {
                    return ERR_FILE_IO;
                }
                break;
                
            case CONST_STRING: {
                // 写入字符串长度
                uint32_t len = constant->string_val ? strlen(constant->string_val) + 1 : 0;
                if (fwrite(&len, sizeof(uint32_t), 1, fp) != 1) {
                    return ERR_FILE_IO;
                }
                
                // 写入字符串内容
                if (len > 0) {
                    if (fwrite(constant->string_val, 1, len, fp) != len) {
                        return ERR_FILE_IO;
                    }
                }
                break;
            }
                
            default:
                // 其他类型暂不支持
                break;
        }
    }
    
    return OK;
}

/**
 * @brief 加载常量池
 */
static ErrorCode load_constants(BytecodeModule* module, FILE* fp, int32_t count) {
    for (int32_t i = 0; i < count; i++) {
        ConstantType type;
        // 读取类型
        if (fread(&type, sizeof(ConstantType), 1, fp) != 1) {
            return ERR_FILE_IO;
        }

        Constant constant;
        constant.type = type;

        switch (type) {
            case CONST_INT:
                if (fread(&constant.int_val, sizeof(int32_t), 1, fp) != 1) {
                    return ERR_FILE_IO;
                }
                break;
            case CONST_REAL:
                if (fread(&constant.real_val, sizeof(double), 1, fp) != 1) {
                    return ERR_FILE_IO;
                }
                break;
            case CONST_BOOL:
                if (fread(&constant.bool_val, sizeof(bool), 1, fp) != 1) {
                    return ERR_FILE_IO;
                }
                break;
            case CONST_STRING: {
                uint32_t len;
                if (fread(&len, sizeof(uint32_t), 1, fp) != 1) {
                    return ERR_FILE_IO;
                }
                if (len > 0) {
                    char* str = (char*)mmgr_alloc(len);
                    if (!str) return ERR_OUT_OF_MEMORY;
                    if (fread(str, 1, len, fp) != len) {
                        mmgr_free(str);
                        return ERR_FILE_IO;
                    }
                    constant.string_val = str;
                } else {
                    constant.string_val = NULL;
                }
                break;
            }
            default:
                // 其他类型暂不支持
                break;
        }
        // 追加到常量池
        if (module->const_count >= module->const_capacity) {
            // 扩容
            uint32_t new_capacity = module->const_capacity ? module->const_capacity * 2 : 8;
            Constant* new_constants = (Constant*)mmgr_alloc(sizeof(Constant) * new_capacity);
            if (!new_constants) return ERR_OUT_OF_MEMORY;
            if (module->constants && module->const_count > 0) {
                memcpy(new_constants, module->constants, sizeof(Constant) * module->const_count);
                mmgr_free(module->constants);
            }
            module->constants = new_constants;
            module->const_capacity = new_capacity;
        }
        module->constants[module->const_count++] = constant;
    }
    return OK;
}

/**
 * @brief 保存函数表
 */
static ErrorCode save_functions(const BytecodeModule* module, FILE* fp) {
    for (uint32_t i = 0; i < module->function_count; i++) {
        FunctionEntry* func = &module->functions[i];
        
        // 写入函数名长度和内容
        uint32_t name_len = func->name ? strlen(func->name) + 1 : 0;
        if (fwrite(&name_len, sizeof(uint32_t), 1, fp) != 1) {
            return ERR_FILE_IO;
        }
        if (name_len > 0) {
            if (fwrite(func->name, 1, name_len, fp) != name_len) {
                return ERR_FILE_IO;
            }
        }
        // 写入函数信息
        if (fwrite(&func->address, sizeof(uint32_t), 1, fp) != 1) {
            return ERR_FILE_IO;
        }
        if (fwrite(&func->param_count, sizeof(int32_t), 1, fp) != 1) {
            return ERR_FILE_IO;
        }
        if (fwrite(&func->local_count, sizeof(int32_t), 1, fp) != 1) {
            return ERR_FILE_IO;
        }
        if (fwrite(&func->return_type, sizeof(DataType), 1, fp) != 1) {
            return ERR_FILE_IO;
        }
        // 写入参数类型数组
        if (func->param_count > 0) {
            if (func->param_types) {
                if (fwrite(func->param_types, sizeof(DataType), func->param_count, fp) != (size_t)func->param_count) {
                    return ERR_FILE_IO;
                }
            } else {
                // 如果没有param_types，写入默认值（TYPE_INT）
                for (int32_t j = 0; j < func->param_count; j++) {
                    DataType default_type = TYPE_INT;
                    if (fwrite(&default_type, sizeof(DataType), 1, fp) != 1) {
                        return ERR_FILE_IO;
                    }
                }
            }
        }
    }
    
    return OK;
}

/**
 * @brief 加载函数表
 */
static ErrorCode load_functions(BytecodeModule* module, FILE* fp, int32_t count) {
    for (int32_t i = 0; i < count; i++) {
        // 读取函数名
        uint32_t name_len;
        if (fread(&name_len, sizeof(uint32_t), 1, fp) != 1) {
            return ERR_FILE_IO;
        }
        
        char* name = NULL;
        if (name_len > 0) {
            name = (char*)mmgr_alloc(name_len);
            if (!name) return ERR_OUT_OF_MEMORY;
            
            if (fread(name, 1, name_len, fp) != name_len) {
                mmgr_free(name);
                return ERR_FILE_IO;
            }
        }
        
        // 读取函数信息
        uint32_t address;
        int32_t param_count, local_count;
        DataType return_type;
        if (fread(&address, sizeof(uint32_t), 1, fp) != 1) {
            if (name) mmgr_free(name);
            return ERR_FILE_IO;
        }
        if (fread(&param_count, sizeof(int32_t), 1, fp) != 1) {
            if (name) mmgr_free(name);
            return ERR_FILE_IO;
        }
        if (fread(&local_count, sizeof(int32_t), 1, fp) != 1) {
            if (name) mmgr_free(name);
            return ERR_FILE_IO;
        }
        if (fread(&return_type, sizeof(DataType), 1, fp) != 1) {
            if (name) mmgr_free(name);
            return ERR_FILE_IO;
        }
        // 读取参数类型数组
        DataType* param_types = NULL;
        if (param_count > 0) {
            param_types = (DataType*)mmgr_alloc(sizeof(DataType) * param_count);
            if (!param_types) {
                if (name) mmgr_free(name);
                return ERR_OUT_OF_MEMORY;
            }
            if (fread(param_types, sizeof(DataType), param_count, fp) != (size_t)param_count) {
                mmgr_free(param_types);
                if (name) mmgr_free(name);
                return ERR_FILE_IO;
            }
        }
        // 添加函数
        bytecode_add_function(module, name, address, param_count, local_count, return_type, param_types);
        if (param_types) mmgr_free(param_types);
        if (name) mmgr_free(name);
    }
    
    return OK;
}

/**
 * @brief 保存指令数组
 */
static ErrorCode save_instructions(const BytecodeModule* module, FILE* fp) {
    // 直接写入指令数组
    size_t count = module->instruction_count;
    if (count > 0) {
        if (fwrite(module->instructions, sizeof(Instruction), count, fp) != count) {
            return ERR_FILE_IO;
        }
    }
    
    return OK;
}

/**
 * @brief 加载指令数组
 */
static ErrorCode load_instructions(BytecodeModule* module, FILE* fp, int32_t count) {
    if (count <= 0) return OK;
    
    // 分配指令数组
    module->instructions = (Instruction*)mmgr_alloc(sizeof(Instruction) * count);
    if (!module->instructions) {
        return ERR_OUT_OF_MEMORY;
    }
    
    // 读取指令
    if (fread(module->instructions, sizeof(Instruction), count, fp) != (size_t)count) {
        return ERR_FILE_IO;
    }
    
    module->instruction_count = count;
    module->instruction_capacity = count;
    
    return OK;
}

/**
 * @brief 保存字节码到文件
 */
ErrorCode bytecode_save(const BytecodeModule* module, const char* filename) {
    if (!module || !filename) {
        return ERR_RUNTIME;
    }
    
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        return ERR_FILE_IO;
    }
    
    ErrorCode err = bytecode_save_to_stream(module, fp);
    fclose(fp);
    
    return err;
}

/**
 * @brief 保存字节码到流
 */
ErrorCode bytecode_save_to_stream(const BytecodeModule* module, FILE* fp) {
    if (!module || !fp) {
        return ERR_RUNTIME;
    }
    
    // 创建文件头
    STBCHeader header;
    header.magic = STBC_MAGIC;
    header.version_major = STBC_VERSION_MAJOR;
    header.version_minor = STBC_VERSION_MINOR;
    header.entry_point = module->entry_point;
    header.global_var_count = module->global_count;
    header.constant_count = module->const_count;
    header.function_count = module->function_count;
    header.instruction_count = module->instruction_count;
    header.library_dep_count = module->library_dep_count;
    header.checksum = bytecode_compute_checksum(module);
    
    // 写入文件头
    if (fwrite(&header, sizeof(STBCHeader), 1, fp) != 1) {
        return ERR_FILE_IO;
    }
    
    // 写入常量池
    ErrorCode err = save_constants(module, fp);
    if (err != OK) return err;
    
    // 写入函数表
    err = save_functions(module, fp);
    if (err != OK) return err;
    
    // 写入指令数组
    err = save_instructions(module, fp);
    if (err != OK) return err;
    
    // 写入库依赖信息(新增)
    for (uint32_t i = 0; i < module->library_dep_count; i++) {
        uint32_t len = strlen(module->library_deps[i]);
        // 写入字符串长度
        if (fwrite(&len, sizeof(uint32_t), 1, fp) != 1) {
            return ERR_FILE_IO;
        }
        // 写入字符串内容
        if (fwrite(module->library_deps[i], 1, len, fp) != len) {
            return ERR_FILE_IO;
        }
    }
    
    return OK;
}

/**
 * @brief 从文件加载字节码
 */
BytecodeModule* bytecode_load(const char* filename) {
    if (!filename) return NULL;
    FILE* fp = fopen(filename, "rb");
    if (!fp) return NULL;
    BytecodeModule* module = bytecode_load_from_stream(fp);
    fclose(fp);
    return module;
}

/**
 * @brief 从流加载字节码
 */
BytecodeModule* bytecode_load_from_stream(FILE* fp) {
    if (!fp) return NULL;
    
    // 读取文件头
    STBCHeader header;
    if (fread(&header, sizeof(STBCHeader), 1, fp) != 1) {
        return NULL;
    }
    
    // 验证文件头
    if (!bytecode_validate_header(&header)) {
        return NULL;
    }
    
    // 创建字节码模块
    BytecodeModule* module = bytecode_module_create();
    if (!module) return NULL;
    
    // 设置基本信息
    module->entry_point = header.entry_point;
    module->global_count = header.global_var_count;
    
    // 加载常量池
    ErrorCode err = load_constants(module, fp, header.constant_count);
    if (err != OK) {
        bytecode_module_free(module);
        return NULL;
    }
    
    // 加载函数表
    err = load_functions(module, fp, header.function_count);
    if (err != OK) {
        bytecode_module_free(module);
        return NULL;
    }
    
    // 释放 bytecode_module_create() 分配的初始指令数组
    // 因为 load_instructions() 会重新分配
    if (module->instructions) {
        mmgr_free(module->instructions);
        module->instructions = NULL;
    }
    
    // 加载指令数组
    err = load_instructions(module, fp, header.instruction_count);
    if (err != OK) {
        bytecode_module_free(module);
        return NULL;
    }
    
    // 加载库依赖信息(新增)
    module->library_dep_count = header.library_dep_count;
    if (header.library_dep_count > 0) {
        module->library_deps = (char**)mmgr_alloc(sizeof(char*) * header.library_dep_count);
        if (!module->library_deps) {
            bytecode_module_free(module);
            return NULL;
        }
        
        for (uint32_t i = 0; i < header.library_dep_count; i++) {
            // 读取字符串长度
            uint32_t len;
            if (fread(&len, sizeof(uint32_t), 1, fp) != 1) {
                bytecode_module_free(module);
                return NULL;
            }
            
            // 分配并读取字符串
            module->library_deps[i] = (char*)mmgr_alloc(len + 1);
            if (!module->library_deps[i]) {
                bytecode_module_free(module);
                return NULL;
            }
            
            if (fread(module->library_deps[i], 1, len, fp) != len) {
                bytecode_module_free(module);
                return NULL;
            }
            module->library_deps[i][len] = '\0';
        }
    }
    
    // 验证校验和
    if (!bytecode_verify_checksum(module, header.checksum)) {
        // 警告：校验和不匹配（不一定是致命错误）
        fprintf(stderr, "Warning: Checksum mismatch in bytecode file\n");
    }
    
    return module;
}

/**
 * @brief 计算校验和
 */
uint32_t bytecode_compute_checksum(const BytecodeModule* module) {
    if (!module) return 0;
    
    uint32_t checksum = 0;
    
    // 对指令数组计算校验和
    if (module->instructions && module->instruction_count > 0) {
        checksum ^= compute_crc32(module->instructions, 
                                   module->instruction_count * sizeof(Instruction));
    }
    
    // 对常量池计算校验和（简化版，只计算数量）
    checksum ^= module->const_count;
    
    // 对函数表计算校验和（简化版，只计算数量）
    checksum ^= module->function_count;
    
    return checksum;
}

/**
 * @brief 验证校验和
 */
bool bytecode_verify_checksum(const BytecodeModule* module, uint32_t expected_checksum) {
    if (!module) return false;
    
    uint32_t actual_checksum = bytecode_compute_checksum(module);
    return actual_checksum == expected_checksum;
}

/**
 * @brief 将字节码模块保存为库文件
 * 
 * 库文件格式在标准 .stbc 格式基础上添加了导出符号表信息，
 * 使得其他模块可以导入并使用库中定义的函数。
 */
ErrorCode bytecode_save_library(const BytecodeModule* module, const struct SymbolTable* symtbl, const char* filename) {
    if (!module || !filename) {
        return ERR_RUNTIME;
    }
    
    // 库文件就是标准的字节码文件，包含所有函数定义
    // LibraryManager 会从字节码中提取函数签名信息
    // 这里直接使用标准保存函数即可
    // symtbl 参数保留用于未来可能的扩展
    (void)symtbl;  // 避免未使用参数警告
    
    return bytecode_save(module, filename);
    
    // 注意：符号表信息已经编码在字节码的函数表中
    // 包括函数名、参数类型、返回类型等
    // 所以不需要单独保存符号表
}
