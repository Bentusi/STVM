"""
STDB-GUI - Graphical Debugger for STVM
=======================================

A professional debugging interface for the Structured Text Virtual Machine (STVM).

Features:
- Code editing with syntax highlighting
- Breakpoint management
- Variable inspection (global, local, watched)
- Call stack visualization
- Force variable management with quality bits
- IO simulator integration
- Hot reload verification
- Performance monitoring (execution cycles, memory usage)
"""

__version__ = '1.0.0'
__author__ = 'STVM Development Team'
__license__ = 'MIT'

from .core.stvm_wrapper import STVMWrapper
from .ui.main_window import MainWindow

__all__ = ['STVMWrapper', 'MainWindow']
