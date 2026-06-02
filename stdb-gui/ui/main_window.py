"""
STDB-GUI Main Window
====================

Main application window with dock-based layout for the STVM debugger GUI.
"""

import os
from typing import Optional
from PySide6.QtWidgets import (
    QMainWindow, QDockWidget, QToolBar, QStatusBar,
    QMessageBox, QFileDialog, QApplication
)
from PySide6.QtCore import Qt, Signal, Slot, QTimer
from PySide6.QtGui import QAction, QKeySequence, QIcon
import qtawesome as qta

from ..core.stvm_wrapper import STVMWrapper, STVMWrapperStub


class MainWindow(QMainWindow):
    """
    Main window for STDB-GUI debugger.
    
    Features:
    - Dock-based layout with code editor, variables, console, etc.
    - Menu bar and toolbar for common actions
    - Signal/slot based communication between components
    - Status bar for VM state display
    """
    
    # Signals
    bytecode_loaded = Signal(str)  # Emitted when bytecode is loaded
    execution_started = Signal()   # Emitted when VM starts running
    execution_paused = Signal()    # Emitted when VM pauses
    execution_stopped = Signal()   # Emitted when VM stops
    step_executed = Signal()       # Emitted after single step
    breakpoint_hit = Signal(str, int)  # Emitted when breakpoint is hit
    
    def __init__(self):
        """Initialize main window"""
        super().__init__()
        
        # VM wrapper
        self.stvm: Optional[STVMWrapper] = None
        self.current_bytecode: Optional[str] = None
        
        # Status
        self.is_running = False
        
        # UI Components (to be created)
        self.code_editor = None
        self.variable_view = None
        self.breakpoint_view = None
        self.stack_view = None
        self.console_widget = None
        self.force_manager = None
        self.performance_chart = None
        
        # Docks
        self.dock_variables = None
        self.dock_breakpoints = None
        self.dock_stack = None
        self.dock_console = None
        self.dock_force = None
        self.dock_performance = None
        
        # Timer for updating UI during execution
        self.update_timer = QTimer()
        self.update_timer.timeout.connect(self._update_runtime_info)
        
        # Setup UI
        self._setup_window()
        self._create_actions()
        self._create_menus()
        self._create_toolbars()
        self._create_status_bar()
        self._create_docks()
        
        # Initialize STVM
        self._initialize_stvm()
        
        # Apply initial state
        self._update_ui_state()
    
    def _setup_window(self):
        """Setup main window properties"""
        self.setWindowTitle("STDB-GUI - STVM Debugger")
        self.setMinimumSize(1200, 800)
        self.resize(1400, 900)
        
        # Set window icon (if available)
        # self.setWindowIcon(QIcon("resources/icons/stdb.png"))
    
    def _initialize_stvm(self):
        """Initialize STVM wrapper"""
        try:
            # Try real wrapper first
            self.stvm = STVMWrapper()
            if not self.stvm.lib:
                # Fall back to stub
                self.stvm = STVMWrapperStub()
        except Exception as e:
            print(f"[STDB-GUI] Warning: Using stub STVM: {e}")
            self.stvm = STVMWrapperStub()
        
        # Initialize VM
        self.stvm.initialize()
    
    # ========== Actions ==========
    
    def _create_actions(self):
        """Create menu and toolbar actions"""
        
        # File actions
        self.action_open = QAction(qta.icon('fa5s.folder-open'), "&Open Bytecode...", self)
        self.action_open.setShortcut(QKeySequence.Open)
        self.action_open.setStatusTip("Open STVM bytecode file (.stbc)")
        self.action_open.triggered.connect(self._on_open_bytecode)
        
        self.action_recent = QAction("Recent Files", self)
        
        self.action_exit = QAction(qta.icon('fa5s.times'), "E&xit", self)
        self.action_exit.setShortcut(QKeySequence.Quit)
        self.action_exit.setStatusTip("Exit application")
        self.action_exit.triggered.connect(self.close)
        
        # Execution actions
        self.action_run = QAction(qta.icon('fa5s.play', color='green'), "&Run", self)
        self.action_run.setShortcut(QKeySequence("F5"))
        self.action_run.setStatusTip("Run program")
        self.action_run.triggered.connect(self._on_run)
        
        self.action_pause = QAction(qta.icon('fa5s.pause', color='orange'), "&Pause", self)
        self.action_pause.setShortcut(QKeySequence("F6"))
        self.action_pause.setStatusTip("Pause execution")
        self.action_pause.triggered.connect(self._on_pause)
        
        self.action_stop = QAction(qta.icon('fa5s.stop', color='red'), "&Stop", self)
        self.action_stop.setShortcut(QKeySequence("Shift+F5"))
        self.action_stop.setStatusTip("Stop execution")
        self.action_stop.triggered.connect(self._on_stop)
        
        self.action_step_over = QAction(qta.icon('fa5s.step-forward'), "Step &Over", self)
        self.action_step_over.setShortcut(QKeySequence("F10"))
        self.action_step_over.setStatusTip("Step over (single line)")
        self.action_step_over.triggered.connect(self._on_step)
        
        self.action_step_into = QAction(qta.icon('fa5s.sign-in-alt'), "Step &Into", self)
        self.action_step_into.setShortcut(QKeySequence("F11"))
        self.action_step_into.setStatusTip("Step into function")
        self.action_step_into.triggered.connect(self._on_step_into)
        
        self.action_step_out = QAction(qta.icon('fa5s.sign-out-alt'), "Step O&ut", self)
        self.action_step_out.setShortcut(QKeySequence("Shift+F11"))
        self.action_step_out.setStatusTip("Step out of function")
        self.action_step_out.triggered.connect(self._on_step_out)
        
        # Breakpoint actions
        self.action_toggle_breakpoint = QAction(qta.icon('fa5s.circle', color='red'),
                                                "Toggle &Breakpoint", self)
        self.action_toggle_breakpoint.setShortcut(QKeySequence("F9"))
        self.action_toggle_breakpoint.setStatusTip("Toggle breakpoint on current line")
        self.action_toggle_breakpoint.triggered.connect(self._on_toggle_breakpoint)
        
        self.action_clear_breakpoints = QAction("Clear All Breakpoints", self)
        self.action_clear_breakpoints.triggered.connect(self._on_clear_breakpoints)
        
        # View actions
        self.action_view_variables = QAction("&Variables", self)
        self.action_view_variables.setCheckable(True)
        self.action_view_variables.setChecked(True)
        
        self.action_view_breakpoints = QAction("&Breakpoints", self)
        self.action_view_breakpoints.setCheckable(True)
        self.action_view_breakpoints.setChecked(True)
        
        self.action_view_stack = QAction("Call &Stack", self)
        self.action_view_stack.setCheckable(True)
        self.action_view_stack.setChecked(True)
        
        self.action_view_console = QAction("&Console", self)
        self.action_view_console.setCheckable(True)
        self.action_view_console.setChecked(True)
        
        self.action_view_force = QAction("&Force Manager", self)
        self.action_view_force.setCheckable(True)
        self.action_view_force.setChecked(False)
        
        self.action_view_performance = QAction("&Performance", self)
        self.action_view_performance.setCheckable(True)
        self.action_view_performance.setChecked(False)
        
        # Help actions
        self.action_about = QAction(qta.icon('fa5s.info-circle'), "&About", self)
        self.action_about.triggered.connect(self._on_about)
        
        self.action_help = QAction(qta.icon('fa5s.question-circle'), "&Help", self)
        self.action_help.setShortcut(QKeySequence.HelpContents)
        self.action_help.triggered.connect(self._on_help)
    
    def _create_menus(self):
        """Create menu bar"""
        menubar = self.menuBar()
        
        # File menu
        file_menu = menubar.addMenu("&File")
        file_menu.addAction(self.action_open)
        file_menu.addAction(self.action_recent)
        file_menu.addSeparator()
        file_menu.addAction(self.action_exit)
        
        # Debug menu
        debug_menu = menubar.addMenu("&Debug")
        debug_menu.addAction(self.action_run)
        debug_menu.addAction(self.action_pause)
        debug_menu.addAction(self.action_stop)
        debug_menu.addSeparator()
        debug_menu.addAction(self.action_step_over)
        debug_menu.addAction(self.action_step_into)
        debug_menu.addAction(self.action_step_out)
        debug_menu.addSeparator()
        debug_menu.addAction(self.action_toggle_breakpoint)
        debug_menu.addAction(self.action_clear_breakpoints)
        
        # View menu
        view_menu = menubar.addMenu("&View")
        view_menu.addAction(self.action_view_variables)
        view_menu.addAction(self.action_view_breakpoints)
        view_menu.addAction(self.action_view_stack)
        view_menu.addAction(self.action_view_console)
        view_menu.addAction(self.action_view_force)
        view_menu.addAction(self.action_view_performance)
        
        # Help menu
        help_menu = menubar.addMenu("&Help")
        help_menu.addAction(self.action_help)
        help_menu.addSeparator()
        help_menu.addAction(self.action_about)
    
    def _create_toolbars(self):
        """Create toolbars"""
        
        # Main toolbar
        main_toolbar = QToolBar("Main Toolbar")
        main_toolbar.setObjectName("MainToolbar")
        self.addToolBar(main_toolbar)
        
        main_toolbar.addAction(self.action_open)
        main_toolbar.addSeparator()
        main_toolbar.addAction(self.action_run)
        main_toolbar.addAction(self.action_pause)
        main_toolbar.addAction(self.action_stop)
        main_toolbar.addSeparator()
        main_toolbar.addAction(self.action_step_over)
        main_toolbar.addAction(self.action_step_into)
        main_toolbar.addAction(self.action_step_out)
        main_toolbar.addSeparator()
        main_toolbar.addAction(self.action_toggle_breakpoint)
    
    def _create_status_bar(self):
        """Create status bar"""
        self.statusBar().showMessage("Ready")
    
    def _create_docks(self):
        """Create dock widgets (placeholders for now)"""
        
        # TODO: Replace with actual widget implementations
        from PySide6.QtWidgets import QLabel, QTextEdit, QTreeView, QTableView, QListView
        
        # Variables dock
        self.dock_variables = QDockWidget("Variables", self)
        self.dock_variables.setObjectName("VariablesDock")
        self.variable_view = QTreeView()  # Placeholder
        self.dock_variables.setWidget(self.variable_view)
        self.addDockWidget(Qt.RightDockWidgetArea, self.dock_variables)
        
        # Breakpoints dock
        self.dock_breakpoints = QDockWidget("Breakpoints", self)
        self.dock_breakpoints.setObjectName("BreakpointsDock")
        self.breakpoint_view = QTableView()  # Placeholder
        self.dock_breakpoints.setWidget(self.breakpoint_view)
        self.addDockWidget(Qt.RightDockWidgetArea, self.dock_breakpoints)
        
        # Stack dock
        self.dock_stack = QDockWidget("Call Stack", self)
        self.dock_stack.setObjectName("StackDock")
        self.stack_view = QListView()  # Placeholder
        self.dock_stack.setWidget(self.stack_view)
        self.addDockWidget(Qt.RightDockWidgetArea, self.dock_stack)
        
        # Console dock
        self.dock_console = QDockWidget("Console", self)
        self.dock_console.setObjectName("ConsoleDock")
        self.console_widget = QTextEdit()  # Placeholder
        self.console_widget.setReadOnly(True)
        self.dock_console.setWidget(self.console_widget)
        self.addDockWidget(Qt.BottomDockWidgetArea, self.dock_console)
        
        # Force manager dock (hidden by default)
        self.dock_force = QDockWidget("Force Manager", self)
        self.dock_force.setObjectName("ForceDock")
        self.force_manager = QTreeView()  # Placeholder
        self.dock_force.setWidget(self.force_manager)
        self.addDockWidget(Qt.LeftDockWidgetArea, self.dock_force)
        self.dock_force.hide()
        
        # Performance dock (hidden by default)
        self.dock_performance = QDockWidget("Performance", self)
        self.dock_performance.setObjectName("PerformanceDock")
        self.performance_chart = QLabel("Performance chart placeholder")  # Placeholder
        self.dock_performance.setWidget(self.performance_chart)
        self.addDockWidget(Qt.BottomDockWidgetArea, self.dock_performance)
        self.dock_performance.hide()
        
        # Code editor (central widget)
        from PySide6.QtWidgets import QTextEdit
        self.code_editor = QTextEdit()  # Placeholder - will be QScintilla
        self.code_editor.setReadOnly(True)
        self.setCentralWidget(self.code_editor)
        
        # Connect view menu actions to dock visibility
        self.action_view_variables.toggled.connect(self.dock_variables.setVisible)
        self.action_view_breakpoints.toggled.connect(self.dock_breakpoints.setVisible)
        self.action_view_stack.toggled.connect(self.dock_stack.setVisible)
        self.action_view_console.toggled.connect(self.dock_console.setVisible)
        self.action_view_force.toggled.connect(self.dock_force.setVisible)
        self.action_view_performance.toggled.connect(self.dock_performance.setVisible)
        
        # Tabify right-side docks
        self.tabifyDockWidget(self.dock_variables, self.dock_breakpoints)
        self.tabifyDockWidget(self.dock_breakpoints, self.dock_stack)
        self.dock_variables.raise_()  # Show variables tab by default
    
    # ========== Action Handlers ==========
    
    @Slot()
    def _on_open_bytecode(self):
        """Handle open bytecode action"""
        filename, _ = QFileDialog.getOpenFileName(
            self,
            "Open Bytecode File",
            "",
            "STVM Bytecode (*.stbc);;All Files (*)"
        )
        
        if filename:
            self._load_bytecode(filename)
    
    def _load_bytecode(self, filename: str):
        """Load bytecode file"""
        if not self.stvm:
            QMessageBox.warning(self, "Error", "STVM not initialized")
            return
        
        if self.stvm.load_bytecode(filename):
            self.current_bytecode = filename
            self.bytecode_loaded.emit(filename)
            self.statusBar().showMessage(f"Loaded: {os.path.basename(filename)}")
            self._log(f"Loaded bytecode: {filename}")
        else:
            QMessageBox.critical(self, "Error", f"Failed to load bytecode: {filename}")
    
    @Slot()
    def _on_run(self):
        """Handle run action"""
        if not self.current_bytecode:
            QMessageBox.warning(self, "No Bytecode", "Please load a bytecode file first")
            return
        
        self.is_running = True
        self.execution_started.emit()
        self._update_ui_state()
        self._log("Execution started")
        
        # Start UI update timer
        self.update_timer.start(100)  # Update every 100ms
        
        # Run in background (TODO: use QThread)
        # For now, just update state
        self.stvm.run()
    
    @Slot()
    def _on_pause(self):
        """Handle pause action"""
        if self.stvm:
            self.stvm.pause()
        
        self.is_running = False
        self.execution_paused.emit()
        self._update_ui_state()
        self._log("Execution paused")
        self.update_timer.stop()
    
    @Slot()
    def _on_stop(self):
        """Handle stop action"""
        if self.stvm:
            self.stvm.pause()
        
        self.is_running = False
        self.execution_stopped.emit()
        self._update_ui_state()
        self._log("Execution stopped")
        self.update_timer.stop()
    
    @Slot()
    def _on_step(self):
        """Handle step over action"""
        if self.stvm:
            self.stvm.step()
            self.step_executed.emit()
            self._log("Step executed")
            self._update_runtime_info()
    
    @Slot()
    def _on_step_into(self):
        """Handle step into action"""
        # TODO: Implement step into
        self._log("Step into (not implemented)")
    
    @Slot()
    def _on_step_out(self):
        """Handle step out action"""
        # TODO: Implement step out
        self._log("Step out (not implemented)")
    
    @Slot()
    def _on_toggle_breakpoint(self):
        """Handle toggle breakpoint action"""
        # TODO: Get current line from code editor
        self._log("Toggle breakpoint (not implemented)")
    
    @Slot()
    def _on_clear_breakpoints(self):
        """Handle clear all breakpoints action"""
        # TODO: Implement
        self._log("Clear all breakpoints (not implemented)")
    
    @Slot()
    def _on_about(self):
        """Handle about action"""
        QMessageBox.about(
            self,
            "About STDB-GUI",
            "<h3>STDB-GUI v1.0.0</h3>"
            "<p>Graphical debugger for STVM (Structured Text Virtual Machine)</p>"
            "<p>Built with PySide6 (Qt6)</p>"
            "<p>Copyright © 2024</p>"
        )
    
    @Slot()
    def _on_help(self):
        """Handle help action"""
        QMessageBox.information(
            self,
            "Help",
            "<h3>STDB-GUI Quick Help</h3>"
            "<p><b>F5</b> - Run</p>"
            "<p><b>F6</b> - Pause</p>"
            "<p><b>Shift+F5</b> - Stop</p>"
            "<p><b>F9</b> - Toggle Breakpoint</p>"
            "<p><b>F10</b> - Step Over</p>"
            "<p><b>F11</b> - Step Into</p>"
            "<p><b>Shift+F11</b> - Step Out</p>"
        )
    
    # ========== UI State Management ==========
    
    def _update_ui_state(self):
        """Update UI state based on VM status"""
        has_bytecode = self.current_bytecode is not None
        
        # Enable/disable actions
        self.action_run.setEnabled(has_bytecode and not self.is_running)
        self.action_pause.setEnabled(has_bytecode and self.is_running)
        self.action_stop.setEnabled(has_bytecode and self.is_running)
        self.action_step_over.setEnabled(has_bytecode and not self.is_running)
        self.action_step_into.setEnabled(has_bytecode and not self.is_running)
        self.action_step_out.setEnabled(has_bytecode and not self.is_running)
        self.action_toggle_breakpoint.setEnabled(has_bytecode)
    
    @Slot()
    def _update_runtime_info(self):
        """Update runtime information during execution"""
        if not self.stvm:
            return
        
        # Get current position
        file, line = self.stvm.get_current_position()
        
        # Update status bar
        if file and line > 0:
            self.statusBar().showMessage(f"Running: {file}:{line}")
        
        # TODO: Update variable view
        # TODO: Update stack view
        # TODO: Update performance chart
    
    def _log(self, message: str):
        """Log message to console"""
        if self.console_widget:
            self.console_widget.append(f"[STDB] {message}")
    
    # ========== Cleanup ==========
    
    def closeEvent(self, event):
        """Handle window close event"""
        # Stop execution if running
        if self.is_running:
            self._on_stop()
        
        # Cleanup STVM
        if self.stvm:
            self.stvm.cleanup()
        
        event.accept()
