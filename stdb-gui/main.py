#!/usr/bin/env python3
"""
STDB-GUI - Graphical Debugger for STVM
=======================================

Main entry point for the application.
"""

import sys
import os
from PySide6.QtWidgets import QApplication
from PySide6.QtCore import Qt

# Add project root to path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from ui.main_window import MainWindow


def main():
    """Main entry point"""
    
    # Enable high DPI support
    QApplication.setHighDpiScaleFactorRoundingPolicy(
        Qt.HighDpiScaleFactorRoundingPolicy.PassThrough
    )
    
    # Create application
    app = QApplication(sys.argv)
    app.setApplicationName("STDB-GUI")
    app.setOrganizationName("STVM")
    app.setApplicationVersion("1.0.0")
    
    # Set application style (optional - can be configured later)
    # Try to use dark theme if available
    try:
        import qdarkstyle
        app.setStyleSheet(qdarkstyle.load_stylesheet(qt_api='pyside6'))
        print("[STDB-GUI] Dark theme loaded")
    except ImportError:
        print("[STDB-GUI] Running with default theme (install qdarkstyle for dark theme)")
    
    # Create and show main window
    window = MainWindow()
    window.show()
    
    # Run event loop
    return app.exec()


if __name__ == '__main__':
    sys.exit(main())
