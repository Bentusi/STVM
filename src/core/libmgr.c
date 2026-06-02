/**
 * @file libmgr.c
 * @brief 库管理器实现
 * 
 * 负责：
 * 1. 加载和卸载.stbc库文件
 * 2. 符号导入和解析
 * 3. 库依赖管理
 * 4. 符号冲突检测
 */

#include "libmgr.h"
#include "bytecode_io.h"
#include "mmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief 创建库管理器
 */
LibraryManager* libmgr_create(SymbolTable* global_symtbl) {
    LibraryManager* mgr = (LibraryManager*)mmgr_alloc(sizeof(LibraryManager));
    if (!mgr) return NULL;
    
    memset(mgr, 0, sizeof(LibraryManager));
    mgr->global_symtbl = global_symtbl;
    mgr->libraries = NULL;
    mgr->imports = NULL;
    mgr->search_path_count = 0;
    
    // 添加默认搜索路径
    libmgr_add_search_path(mgr, ".");
    libmgr_add_search_path(mgr, "./lib");
    libmgr_add_search_path(mgr, "/usr/local/lib/stvm");
    
    return mgr;
}

/**
 * @brief 获取第一个加载的库（用于遍历）
 */
LoadedLibrary* libmgr_get_first_library(LibraryManager* mgr) {
    if (!mgr) return NULL;
    return mgr->libraries;
}

/**
 * @brief 释放已加载库
 */
static void free_loaded_library(LoadedLibrary* lib) {
    if (!lib) return;
    
    if (lib->name) mmgr_free(lib->name);
    if (lib->path) mmgr_free(lib->path);
    // 重要：先释放符号表，因为它可能引用module中的字符串
    if (lib->symbols) symtbl_free(lib->symbols);
    // 然后再释放模块
    if (lib->module) bytecode_module_free(lib->module);
    
    mmgr_free(lib);
}

/**
 * @brief 释放已导入符号
 */
static void free_imported_symbol(ImportedSymbol* sym) {
    if (!sym) return;
    
    if (sym->name) mmgr_free(sym->name);
    if (sym->original_name) mmgr_free(sym->original_name);
    
    mmgr_free(sym);
}

/**
 * @brief 释放库管理器
 */
void libmgr_free(LibraryManager* mgr) {
    if (!mgr) return;
    
    // 释放已加载库
    LoadedLibrary* lib = mgr->libraries;
    while (lib) {
        LoadedLibrary* next = lib->next;
        free_loaded_library(lib);
        lib = next;
    }
    
    // 释放已导入符号
    ImportedSymbol* sym = mgr->imports;
    while (sym) {
        ImportedSymbol* next = sym->next;
        free_imported_symbol(sym);
        sym = next;
    }
    
    mmgr_free(mgr);
}

/**
 * @brief 添加库搜索路径
 */
bool libmgr_add_search_path(LibraryManager* mgr, const char* path) {
    if (!mgr || !path) return false;
    
    if (mgr->search_path_count >= 256) {
        return false;  // 搜索路径已满
    }
    
    strncpy(mgr->search_paths[mgr->search_path_count], path, 511);
    mgr->search_paths[mgr->search_path_count][511] = '\0';
    mgr->search_path_count++;
    
    return true;
}

/**
 * @brief 在搜索路径中查找库文件
 */
static char* find_library_file(LibraryManager* mgr, const char* filename) {
    static char fullpath[1024];
    
    // 首先尝试直接打开文件（可能是相对路径或绝对路径）
    FILE* f = fopen(filename, "rb");
    if (f) {
        fclose(f);
        strncpy(fullpath, filename, 1023);
        fullpath[1023] = '\0';
        return fullpath;
    }
    
    // 在搜索路径中查找
    for (int i = 0; i < mgr->search_path_count; i++) {
        snprintf(fullpath, sizeof(fullpath), "%s/%s", mgr->search_paths[i], filename);
        f = fopen(fullpath, "rb");
        if (f) {
            fclose(f);
            return fullpath;
        }
    }
    
    return NULL;
}

/**
 * @brief 从库模块构建符号表
 */
static SymbolTable* build_symbol_table_from_module(BytecodeModule* module) {
    SymbolTable* symtbl = symtbl_init();
    if (!symtbl) return NULL;
    
    // 导出所有函数
    for (uint32_t i = 0; i < module->function_count; i++) {
        FunctionEntry* func = &module->functions[i];
        if (!func->name) continue;
        
        // 创建函数类型信息
        TypeInfo* return_type = type_info_create(func->return_type);
        
        // 使用FunctionEntry中的参数类型信息
        TypeInfo** param_types = NULL;
        if (func->param_count > 0) {
            param_types = (TypeInfo**)mmgr_alloc(sizeof(TypeInfo*) * func->param_count);
            for (int32_t j = 0; j < func->param_count; j++) {
                // 如果有参数类型信息就使用，否则默认为int
                DataType param_type = (func->param_types && func->param_types[j] != TYPE_VOID) 
                                       ? func->param_types[j] : TYPE_INT;
                param_types[j] = type_info_create(param_type);
            }
        }
        
        // 注册函数符号
        symtbl_define_function(symtbl, func->name, return_type, param_types, func->param_count);
        
        // 释放参数类型
        if (param_types) {
            for (int32_t j = 0; j < func->param_count; j++) {
                type_info_free(param_types[j]);
            }
            mmgr_free(param_types);
        }
        type_info_free(return_type);
    }
    
    return symtbl;
}

/**
 * @brief 提取库名称（从文件名）
 */
static char* extract_library_name(const char* filename) {
    const char* basename = strrchr(filename, '/');
    basename = basename ? basename + 1 : filename;
    
    // 去掉.stbc后缀
    size_t len = strlen(basename);
    if (len > 5 && strcmp(basename + len - 5, ".stbc") == 0) {
        len -= 5;
    }
    
    char* name = (char*)mmgr_alloc(len + 1);
    strncpy(name, basename, len);
    name[len] = '\0';
    
    return name;
}

/**
 * @brief 查找已加载的库
 */
LoadedLibrary* libmgr_find_library(LibraryManager* mgr, const char* name) {
    if (!mgr || !name) return NULL;
    
    LoadedLibrary* lib = mgr->libraries;
    while (lib) {
        if (lib->name && strcmp(lib->name, name) == 0) {
            return lib;
        }
        lib = lib->next;
    }
    
    return NULL;
}

/**
 * @brief 加载库文件
 */
ErrorCode libmgr_load_library(LibraryManager* mgr, const char* filename) {
    if (!mgr || !filename) return ERR_RUNTIME;
    
    // 提取库名称
    char* lib_name = extract_library_name(filename);
    
    // 检查是否已加载
    if (libmgr_find_library(mgr, lib_name)) {
        mmgr_free(lib_name);
        return OK;  // 已加载，直接返回
    }
    
    // 查找库文件
    char* fullpath = find_library_file(mgr, filename);
    if (!fullpath) {
        mmgr_free(lib_name);
        fprintf(stderr, "Error: Cannot find library file: %s\n", filename);
        return ERR_FILE_IO;
    }
    
    // 加载字节码模块
    BytecodeModule* module = bytecode_load(fullpath);
    if (!module) {
        mmgr_free(lib_name);
        fprintf(stderr, "Error: Failed to load library: %s\n", fullpath);
        return ERR_FILE_IO;
    }
    
    // 构建符号表
    SymbolTable* symbols = build_symbol_table_from_module(module);
    if (!symbols) {
        bytecode_module_free(module);
        mmgr_free(lib_name);
        return ERR_OUT_OF_MEMORY;
    }
    
    // 创建库记录
    LoadedLibrary* lib = (LoadedLibrary*)mmgr_alloc(sizeof(LoadedLibrary));
    if (!lib) {
        symtbl_free(symbols);
        bytecode_module_free(module);
        mmgr_free(lib_name);
        return ERR_OUT_OF_MEMORY;
    }
    
    lib->name = lib_name;
    lib->path = mmgr_strdup(fullpath);
    lib->module = module;
    lib->symbols = symbols;
    lib->next = mgr->libraries;
    mgr->libraries = lib;
    
    // printf("[libmgr] Loaded library: %s (%u functions)\n", 
    //        lib_name, module->function_count);
    
    return OK;
}

/**
 * @brief 查找已导入的符号
 */
ImportedSymbol* libmgr_find_import(LibraryManager* mgr, const char* name) {
    if (!mgr || !name) return NULL;
    
    ImportedSymbol* sym = mgr->imports;
    while (sym) {
        if (sym->name && strcmp(sym->name, name) == 0) {
            return sym;
        }
        sym = sym->next;
    }
    
    return NULL;
}

/**
 * @brief 导入库中的符号
 */
ErrorCode libmgr_import_symbol(LibraryManager* mgr, const char* library_name,
                                const char* symbol_name, const char* alias) {
    if (!mgr || !library_name || !symbol_name) return ERR_RUNTIME;
    
    // 提取库名（去除路径和扩展名）
    char* lib_name = extract_library_name(library_name);
    
    // 查找库
    LoadedLibrary* lib = libmgr_find_library(mgr, lib_name);
    if (!lib) {
        fprintf(stderr, "Error: Library not loaded: %s\n", library_name);
        mmgr_free(lib_name);
        return ERR_NAME;
    }
    
    mmgr_free(lib_name);
    
    // 在库的符号表中查找符号
    Symbol* symbol = symtbl_lookup(lib->symbols, symbol_name);
    if (!symbol) {
        fprintf(stderr, "Error: Symbol not found in library %s: %s\n", 
                library_name, symbol_name);
        return ERR_NAME;
    }
    
    // 使用的名称（别名或原名）
    const char* import_name = alias ? alias : symbol_name;
    
    // 构建完全限定名（library_path.symbol_name）
    char qualified_name[512];
    snprintf(qualified_name, sizeof(qualified_name), "%s.%s", library_name, symbol_name);
    
    // 检查是否已导入
    if (libmgr_find_import(mgr, import_name)) {
        fprintf(stderr, "Error: Symbol already imported: %s\n", import_name);
        return ERR_NAME;
    }
    
    // 检查全局符号表中是否有冲突
    Symbol* existing = symtbl_lookup(mgr->global_symtbl, import_name);
    if (existing) {
        fprintf(stderr, "Error: Symbol name conflict: %s\n", import_name);
        return ERR_NAME;
    }
    
    // 创建导入记录
    ImportedSymbol* import = (ImportedSymbol*)mmgr_alloc(sizeof(ImportedSymbol));
    if (!import) return ERR_OUT_OF_MEMORY;
    
    import->name = mmgr_strdup(import_name);  // 使用导入名称（别名或原名）
    import->original_name = mmgr_strdup(symbol_name);
    import->library = lib;
    import->symbol = symbol;
    import->next = mgr->imports;
    mgr->imports = import;
    
    // 将符号注册到全局符号表
    // 使用导入名称（别名或原名），这样代码中可以直接使用简单名称
    TypeInfo* type_copy = type_info_retain(symbol->type);
    Symbol* global_sym = symtbl_define_variable(mgr->global_symtbl, import_name, 
                                                 type_copy, false);
    if (global_sym) {
        global_sym->kind = symbol->kind;
        global_sym->address = symbol->address;
        global_sym->param_count = symbol->param_count;
        global_sym->local_count = symbol->local_count;
        global_sym->is_library = true;
        global_sym->library_name = mmgr_strdup(library_name);
        global_sym->qualified_name = mmgr_strdup(qualified_name);  // 保存完全限定名
    } else {
        // 如果定义失败，释放retain的类型
        type_info_free(type_copy);
    }
    
    // printf("[libmgr] Imported: %s\n", qualified_name);
    // if (alias) {
    //     printf("         as %s\n", alias);
    // }
    
    return OK;
}

/**
 * @brief 导入库中的所有符号
 */
ErrorCode libmgr_import_all(LibraryManager* mgr, const char* library_name) {
    if (!mgr || !library_name) return ERR_RUNTIME;
    
    // 提取库名（去除路径和扩展名）
    char* lib_name = extract_library_name(library_name);
    
    LoadedLibrary* lib = libmgr_find_library(mgr, lib_name);
    if (!lib) {
        fprintf(stderr, "Error: Library not loaded: %s\n", library_name);
        mmgr_free(lib_name);
        return ERR_NAME;
    }
    
    mmgr_free(lib_name);
    
    // 遍历库的符号表，导入所有符号
    // 注意：这里需要符号表提供遍历接口
    printf("[libmgr] Import all from %s (feature not fully implemented)\n", library_name);
    
    return OK;
}

/**
 * @brief 解析符号引用（支持module.symbol格式）
 */
Symbol* libmgr_resolve_symbol(LibraryManager* mgr, const char* qualified_name) {
    if (!mgr || !qualified_name) return NULL;
    
    // 查找点号分隔符
    const char* dot = strchr(qualified_name, '.');
    if (!dot) {
        // 没有点号，直接在全局符号表和导入列表中查找
        Symbol* sym = symtbl_lookup(mgr->global_symtbl, qualified_name);
        if (sym) return sym;
        
        ImportedSymbol* import = libmgr_find_import(mgr, qualified_name);
        return import ? import->symbol : NULL;
    }
    
    // 有点号，分离库名和符号名
    size_t lib_name_len = dot - qualified_name;
    char lib_name[256];
    strncpy(lib_name, qualified_name, lib_name_len);
    lib_name[lib_name_len] = '\0';
    
    const char* symbol_name = dot + 1;
    
    // 查找库
    LoadedLibrary* lib = libmgr_find_library(mgr, lib_name);
    if (!lib) return NULL;
    
    // 在库的符号表中查找符号
    return symtbl_lookup(lib->symbols, symbol_name);
}

/**
 * @brief 卸载库
 */
ErrorCode libmgr_unload_library(LibraryManager* mgr, const char* name) {
    if (!mgr || !name) return ERR_RUNTIME;
    
    LoadedLibrary** lib_ptr = &mgr->libraries;
    while (*lib_ptr) {
        LoadedLibrary* lib = *lib_ptr;
        if (lib->name && strcmp(lib->name, name) == 0) {
            // 移除所有引用该库的导入
            ImportedSymbol** import_ptr = &mgr->imports;
            while (*import_ptr) {
                ImportedSymbol* import = *import_ptr;
                if (import->library == lib) {
                    *import_ptr = import->next;
                    free_imported_symbol(import);
                } else {
                    import_ptr = &import->next;
                }
            }
            
            // 移除库
            *lib_ptr = lib->next;
            free_loaded_library(lib);
            
            printf("[libmgr] Unloaded library: %s\n", name);
            return OK;
        }
        lib_ptr = &lib->next;
    }
    
    fprintf(stderr, "Error: Library not found: %s\n", name);
    return ERR_NAME;
}

/**
 * @brief 打印已加载的库列表
 */
void libmgr_dump_libraries(LibraryManager* mgr) {
    if (!mgr) return;
    
    printf("=== Loaded Libraries ===\n");
    int count = 0;
    LoadedLibrary* lib = mgr->libraries;
    while (lib) {
        printf("[%d] %s\n", count++, lib->name);
        printf("    Path: %s\n", lib->path);
        printf("    Functions: %u\n", lib->module->function_count);
        printf("    Constants: %u\n", lib->module->const_count);
        lib = lib->next;
    }
    printf("Total: %d libraries\n", count);
    printf("========================\n");
}

/**
 * @brief 打印已导入的符号列表
 */
void libmgr_dump_imports(LibraryManager* mgr) {
    if (!mgr) return;
    
    printf("=== Imported Symbols ===\n");
    int count = 0;
    ImportedSymbol* sym = mgr->imports;
    while (sym) {
        printf("[%d] %s", count++, sym->name);
        if (strcmp(sym->name, sym->original_name) != 0) {
            printf(" (from %s)", sym->original_name);
        }
        printf(" <- %s\n", sym->library->name);
        sym = sym->next;
    }
    printf("Total: %d symbols\n", count);
    printf("========================\n");
}

/**
 * @brief 获取已加载库的数量
 */
uint32_t libmgr_get_library_count(LibraryManager* mgr) {
    if (!mgr) return 0;
    
    uint32_t count = 0;
    LoadedLibrary* lib = mgr->libraries;
    while (lib) {
        count++;
        lib = lib->next;
    }
    return count;
}

/**
 * @brief 根据索引获取库名称
 */
const char* libmgr_get_library_name(LibraryManager* mgr, uint32_t index) {
    if (!mgr) return NULL;
    
    uint32_t i = 0;
    LoadedLibrary* lib = mgr->libraries;
    while (lib) {
        if (i == index) {
            return lib->name;
        }
        i++;
        lib = lib->next;
    }
    return NULL;
}

/**
 * @brief 获取库的字节码模块
 */
BytecodeModule* libmgr_get_library_module(LibraryManager* mgr, const char* name) {
    if (!mgr || !name) return NULL;
    
    LoadedLibrary* lib = libmgr_find_library(mgr, name);
    if (lib) {
        return lib->module;
    }
    return NULL;
}

/**
 * @brief 获取库的完整路径
 */
const char* libmgr_get_library_path(LibraryManager* mgr, const char* name) {
    if (!mgr || !name) return NULL;
    
    LoadedLibrary* lib = libmgr_find_library(mgr, name);
    if (lib) {
        return lib->path;
    }
    return NULL;
}
