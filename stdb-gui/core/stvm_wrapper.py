"""
STVM C Library Wrapper
======================

This module provides Python bindings to the STVM C library using cffi.
It wraps the core VM, debugger, and force management functionality.
"""

import os
import sys
from typing import Optional, List, Tuple, Any
from dataclasses import dataclass
from enum import IntEnum
import cffi

# Add parent directory to path to find libstvm.so
STVM_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
STVM_LIB_PATH = os.path.join(STVM_ROOT, 'build', 'lib', 'libstvm.so')

# Quality flags matching types.h
class QualityFlag(IntEnum):
    GOOD = 0
    UNCERTAIN = 1
    BAD = 2
    ERROR = 3

# Data types matching types.h
class DataType(IntEnum):
    VOID = 0
    BOOL = 1
    INT = 2
    REAL = 3
    STRING = 4
    ARRAY = 5
    FUNCTION = 6
    QBOOL = 7
    QINT = 8
    QREAL = 9
    QSTRING = 10

@dataclass
class BreakpointInfo:
    """Breakpoint information"""
    file: str
    line: int
    enabled: bool
    condition: Optional[str] = None
    hit_count: int = 0

@dataclass
class VariableInfo:
    """Variable information"""
    name: str
    type: DataType
    value: Any
    quality: QualityFlag = QualityFlag.GOOD
    is_array: bool = False
    array_size: int = 0

@dataclass
class StackFrameInfo:
    """Stack frame information"""
    function_name: str
    file: str
    line: int
    level: int

class STVMWrapper:
    """
    Python wrapper for STVM C library.
    
    Provides high-level interface to:
    - VM execution control
    - Debugger functionality
    - Variable inspection
    - Force management
    - Performance monitoring
    """
    
    def __init__(self):
        """Initialize STVM wrapper"""
        self.ffi = cffi.FFI()
        self.lib = None
        self.vm = None
        self.is_initialized = False
        
        self._define_c_interface()
        self._load_library()
    
    def _define_c_interface(self):
        """Define C API using cffi"""
        self.ffi.cdef("""
            // Forward declarations
            typedef struct VM VM;
            typedef struct Debugger Debugger;
            typedef struct ForceManager ForceManager;
            typedef struct Value Value;
            
            // Data types (matching types.h)
            typedef enum {
                TYPE_VOID = 0,
                TYPE_BOOL = 1,
                TYPE_INT = 2,
                TYPE_REAL = 3,
                TYPE_STRING = 4,
                TYPE_ARRAY = 5,
                TYPE_FUNCTION = 6,
                TYPE_QBOOL = 7,
                TYPE_QINT = 8,
                TYPE_QREAL = 9,
                TYPE_QSTRING = 10
            } DataType;
            
            // Quality flags
            typedef enum {
                QUALITY_GOOD = 0,
                QUALITY_UNCERTAIN = 1,
                QUALITY_BAD = 2,
                QUALITY_ERROR = 3
            } QualityFlag;
            
            // Value structure (simplified)
            struct Value {
                DataType type;
                QualityFlag quality;
                union {
                    bool bool_val;
                    int int_val;
                    double real_val;
                    char string_val[256];
                };
            };
            
            // VM functions (to be implemented as exports from STVM)
            VM* vm_create(void);
            void vm_destroy(VM* vm);
            bool vm_load_bytecode(VM* vm, const char* filename);
            int vm_run(VM* vm);
            void vm_pause(VM* vm);
            void vm_resume(VM* vm);
            void vm_step(VM* vm);
            bool vm_is_running(VM* vm);
            int vm_get_current_line(VM* vm);
            const char* vm_get_current_file(VM* vm);
            
            // Debugger functions
            Debugger* vm_get_debugger(VM* vm);
            bool debugger_set_breakpoint(Debugger* dbg, const char* file, int line);
            bool debugger_remove_breakpoint(Debugger* dbg, const char* file, int line);
            int debugger_get_breakpoint_count(Debugger* dbg);
            
            // Variable inspection
            int vm_get_global_variable_count(VM* vm);
            const char* vm_get_global_variable_name(VM* vm, int index);
            Value* vm_get_global_variable_value(VM* vm, const char* name);
            
            // Force management
            ForceManager* vm_get_force_manager(VM* vm);
            bool force_variable(ForceManager* mgr, const char* name, Value value, bool persistent);
            bool unforce_variable(ForceManager* mgr, const char* name);
            int force_get_count(ForceManager* mgr);
            
            // Performance monitoring
            double vm_get_last_cycle_time(VM* vm);
            size_t vm_get_memory_usage(VM* vm);
            
            // Memory management
            void* malloc(size_t size);
            void free(void* ptr);
        """)
    
    def _load_library(self):
        """Load the STVM shared library"""
        try:
            # Try to load from build directory
            if os.path.exists(STVM_LIB_PATH):
                self.lib = self.ffi.dlopen(STVM_LIB_PATH)
                print(f"[STDB-GUI] Loaded STVM library from: {STVM_LIB_PATH}")
            else:
                # Try system library path
                self.lib = self.ffi.dlopen('libstvm.so')
                print("[STDB-GUI] Loaded STVM library from system path")
        except Exception as e:
            print(f"[STDB-GUI] Warning: Could not load STVM library: {e}")
            print("[STDB-GUI] Running in stub mode - some features will not work")
            self.lib = None
    
    def initialize(self, bytecode_file: Optional[str] = None) -> bool:
        """
        Initialize VM and optionally load bytecode
        
        Args:
            bytecode_file: Path to .stbc bytecode file
            
        Returns:
            True if initialization successful
        """
        if not self.lib:
            print("[STDB-GUI] Error: STVM library not loaded")
            return False
        
        try:
            # Create VM instance
            self.vm = self.lib.vm_create()
            if not self.vm:
                print("[STDB-GUI] Error: Failed to create VM instance")
                return False
            
            # Load bytecode if provided
            if bytecode_file:
                if not self.load_bytecode(bytecode_file):
                    return False
            
            self.is_initialized = True
            print("[STDB-GUI] VM initialized successfully")
            return True
            
        except Exception as e:
            print(f"[STDB-GUI] Error initializing VM: {e}")
            return False
    
    def cleanup(self):
        """Clean up VM resources"""
        if self.vm and self.lib:
            self.lib.vm_destroy(self.vm)
            self.vm = None
            self.is_initialized = False
    
    def load_bytecode(self, filename: str) -> bool:
        """
        Load bytecode file
        
        Args:
            filename: Path to .stbc file
            
        Returns:
            True if loaded successfully
        """
        if not self.vm or not self.lib:
            return False
        
        try:
            filename_c = self.ffi.new("char[]", filename.encode('utf-8'))
            result = self.lib.vm_load_bytecode(self.vm, filename_c)
            
            if result:
                print(f"[STDB-GUI] Loaded bytecode: {filename}")
            else:
                print(f"[STDB-GUI] Failed to load bytecode: {filename}")
            
            return result
        except Exception as e:
            print(f"[STDB-GUI] Error loading bytecode: {e}")
            return False
    
    # ========== Execution Control ==========
    
    def run(self) -> int:
        """Run the VM program"""
        if not self.vm or not self.lib:
            return -1
        return self.lib.vm_run(self.vm)
    
    def pause(self):
        """Pause VM execution"""
        if self.vm and self.lib:
            self.lib.vm_pause(self.vm)
    
    def resume(self):
        """Resume VM execution"""
        if self.vm and self.lib:
            self.lib.vm_resume(self.vm)
    
    def step(self):
        """Execute single step"""
        if self.vm and self.lib:
            self.lib.vm_step(self.vm)
    
    def is_running(self) -> bool:
        """Check if VM is running"""
        if not self.vm or not self.lib:
            return False
        return self.lib.vm_is_running(self.vm)
    
    def get_current_position(self) -> Tuple[str, int]:
        """
        Get current execution position
        
        Returns:
            (filename, line_number)
        """
        if not self.vm or not self.lib:
            return ("", 0)
        
        file_ptr = self.lib.vm_get_current_file(self.vm)
        line = self.lib.vm_get_current_line(self.vm)
        
        filename = self.ffi.string(file_ptr).decode('utf-8') if file_ptr else ""
        return (filename, line)
    
    # ========== Breakpoint Management ==========
    
    def set_breakpoint(self, file: str, line: int) -> bool:
        """Set a breakpoint"""
        if not self.vm or not self.lib:
            return False
        
        dbg = self.lib.vm_get_debugger(self.vm)
        if not dbg:
            return False
        
        file_c = self.ffi.new("char[]", file.encode('utf-8'))
        return self.lib.debugger_set_breakpoint(dbg, file_c, line)
    
    def remove_breakpoint(self, file: str, line: int) -> bool:
        """Remove a breakpoint"""
        if not self.vm or not self.lib:
            return False
        
        dbg = self.lib.vm_get_debugger(self.vm)
        if not dbg:
            return False
        
        file_c = self.ffi.new("char[]", file.encode('utf-8'))
        return self.lib.debugger_remove_breakpoint(dbg, file_c, line)
    
    def get_breakpoint_count(self) -> int:
        """Get number of breakpoints"""
        if not self.vm or not self.lib:
            return 0
        
        dbg = self.lib.vm_get_debugger(self.vm)
        if not dbg:
            return 0
        
        return self.lib.debugger_get_breakpoint_count(dbg)
    
    # ========== Variable Inspection ==========
    
    def get_global_variables(self) -> List[VariableInfo]:
        """Get all global variables"""
        if not self.vm or not self.lib:
            return []
        
        variables = []
        count = self.lib.vm_get_global_variable_count(self.vm)
        
        for i in range(count):
            name_ptr = self.lib.vm_get_global_variable_name(self.vm, i)
            if not name_ptr:
                continue
            
            name = self.ffi.string(name_ptr).decode('utf-8')
            value_ptr = self.lib.vm_get_global_variable_value(self.vm, name_ptr)
            
            if value_ptr:
                var_info = VariableInfo(
                    name=name,
                    type=DataType(value_ptr.type),
                    value=self._extract_value(value_ptr),
                    quality=QualityFlag(value_ptr.quality)
                )
                variables.append(var_info)
        
        return variables
    
    def _extract_value(self, value_ptr) -> Any:
        """Extract Python value from C Value structure"""
        if not value_ptr:
            return None
        
        value_type = DataType(value_ptr.type)
        
        if value_type in (DataType.BOOL, DataType.QBOOL):
            return bool(value_ptr.bool_val)
        elif value_type in (DataType.INT, DataType.QINT):
            return int(value_ptr.int_val)
        elif value_type in (DataType.REAL, DataType.QREAL):
            return float(value_ptr.real_val)
        elif value_type in (DataType.STRING, DataType.QSTRING):
            return self.ffi.string(value_ptr.string_val).decode('utf-8')
        else:
            return None
    
    # ========== Force Management ==========
    
    def force_variable(self, name: str, value: Any, var_type: DataType,
                      quality: QualityFlag = QualityFlag.GOOD,
                      persistent: bool = False) -> bool:
        """Force a variable to a specific value"""
        if not self.vm or not self.lib:
            return False
        
        force_mgr = self.lib.vm_get_force_manager(self.vm)
        if not force_mgr:
            return False
        
        # Create C Value structure
        c_value = self.ffi.new("struct Value*")
        c_value.type = var_type
        c_value.quality = quality
        
        # Set value based on type
        if var_type in (DataType.BOOL, DataType.QBOOL):
            c_value.bool_val = bool(value)
        elif var_type in (DataType.INT, DataType.QINT):
            c_value.int_val = int(value)
        elif var_type in (DataType.REAL, DataType.QREAL):
            c_value.real_val = float(value)
        elif var_type in (DataType.STRING, DataType.QSTRING):
            # Copy string to buffer
            str_bytes = str(value).encode('utf-8')[:255]
            self.ffi.memmove(c_value.string_val, str_bytes, len(str_bytes))
            c_value.string_val[len(str_bytes)] = b'\0'
        
        name_c = self.ffi.new("char[]", name.encode('utf-8'))
        return self.lib.force_variable(force_mgr, name_c, c_value[0], persistent)
    
    def unforce_variable(self, name: str) -> bool:
        """Remove force from a variable"""
        if not self.vm or not self.lib:
            return False
        
        force_mgr = self.lib.vm_get_force_manager(self.vm)
        if not force_mgr:
            return False
        
        name_c = self.ffi.new("char[]", name.encode('utf-8'))
        return self.lib.unforce_variable(force_mgr, name_c)
    
    def get_force_count(self) -> int:
        """Get number of forced variables"""
        if not self.vm or not self.lib:
            return 0
        
        force_mgr = self.lib.vm_get_force_manager(self.vm)
        if not force_mgr:
            return 0
        
        return self.lib.force_get_count(force_mgr)
    
    # ========== Performance Monitoring ==========
    
    def get_last_cycle_time(self) -> float:
        """Get last execution cycle time in milliseconds"""
        if not self.vm or not self.lib:
            return 0.0
        return self.lib.vm_get_last_cycle_time(self.vm)
    
    def get_memory_usage(self) -> int:
        """Get current memory usage in bytes"""
        if not self.vm or not self.lib:
            return 0
        return self.lib.vm_get_memory_usage(self.vm)
    
    def __del__(self):
        """Cleanup on deletion"""
        self.cleanup()


# Stub implementation for testing without compiled STVM
class STVMWrapperStub(STVMWrapper):
    """Stub implementation for development/testing without STVM library"""
    
    def __init__(self):
        """Initialize stub"""
        self.is_initialized = False
        self._breakpoints = []
        self._variables = [
            VariableInfo("counter", DataType.INT, 42, QualityFlag.GOOD),
            VariableInfo("temperature", DataType.QREAL, 25.5, QualityFlag.GOOD),
            VariableInfo("alarm", DataType.QBOOL, True, QualityFlag.BAD),
        ]
        self._current_file = "main.st"
        self._current_line = 1
        self._is_running = False
        print("[STDB-GUI] Using stub STVM wrapper (for testing)")
    
    def _load_library(self):
        """Override - no library to load"""
        pass
    
    def _define_c_interface(self):
        """Override - no C interface needed"""
        pass
    
    def initialize(self, bytecode_file: Optional[str] = None) -> bool:
        """Stub initialize"""
        self.is_initialized = True
        return True
    
    def cleanup(self):
        """Stub cleanup"""
        self.is_initialized = False
    
    def load_bytecode(self, filename: str) -> bool:
        """Stub load"""
        return True
    
    def run(self) -> int:
        """Stub run"""
        self._is_running = True
        return 0
    
    def pause(self):
        """Stub pause"""
        self._is_running = False
    
    def resume(self):
        """Stub resume"""
        self._is_running = True
    
    def step(self):
        """Stub step"""
        self._current_line += 1
    
    def is_running(self) -> bool:
        """Stub is_running"""
        return self._is_running
    
    def get_current_position(self) -> Tuple[str, int]:
        """Stub get position"""
        return (self._current_file, self._current_line)
    
    def set_breakpoint(self, file: str, line: int) -> bool:
        """Stub set breakpoint"""
        self._breakpoints.append((file, line))
        return True
    
    def remove_breakpoint(self, file: str, line: int) -> bool:
        """Stub remove breakpoint"""
        try:
            self._breakpoints.remove((file, line))
            return True
        except ValueError:
            return False
    
    def get_breakpoint_count(self) -> int:
        """Stub get breakpoint count"""
        return len(self._breakpoints)
    
    def get_global_variables(self) -> List[VariableInfo]:
        """Stub get variables"""
        return self._variables
    
    def force_variable(self, name: str, value: Any, var_type: DataType,
                      quality: QualityFlag = QualityFlag.GOOD,
                      persistent: bool = False) -> bool:
        """Stub force"""
        return True
    
    def unforce_variable(self, name: str) -> bool:
        """Stub unforce"""
        return True
    
    def get_force_count(self) -> int:
        """Stub force count"""
        return 0
    
    def get_last_cycle_time(self) -> float:
        """Stub cycle time"""
        return 1.23
    
    def get_memory_usage(self) -> int:
        """Stub memory"""
        return 1024 * 1024 * 45  # 45 MB
