import os
import csv
import datetime
from network import insert_into_db
import serial.tools.list_ports
from PyQt5.QtWidgets import (
    QMainWindow, QVBoxLayout, QWidget, QComboBox, QLabel, QHBoxLayout, QTextEdit, QPushButton, QScrollArea, QSizePolicy
)
from PyQt5.QtGui import QMovie, QPixmap, QIcon, QFont
from PyQt5.QtCore import QTimer, QThread, Qt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
import matplotlib.pyplot as plt
from mock_serial_reader import MockSerialReader
from connection_worker import ConnectionWorker
import matplotlib.dates as mdates
import matplotlib.ticker as mticker
from widget.battery_widget import VerticalBatteryWidget
from widget.speedometer_widget import SpeedometerWidget
from data_manager import SerialDataManager

# Graph type constants
GRAPH_TYPES = ["Velocity", "Battery Temps", "Motor Temps", "MPPT power", "Power trade-off"]
MAX_GRAPHS = 4

class PlotApp(QMainWindow):
    def __init__(self, serial_reader):
        super().__init__()
        self.serial_reader = serial_reader
        self.dark_mode = False  # Default to light mode
        
        # Initialize data manager first
        self.data_manager = SerialDataManager()
        
        # Get the directory where this script is located
        script_dir = os.path.dirname(os.path.abspath(__file__))
        self.logo_path = os.path.join(script_dir, 'assets', 'HUST_big_logo.png')
        self.loading_gif_path = os.path.join(script_dir, 'assets', 'loading.gif')
        
        # Critical thresholds for warning system
        self.critical_thresholds = {
            "battery_volt": {"min": 100.0, "max": 150.0},  # V
            "battery_current": {"min": -50.0, "max": 80.0},  # A
            "battery_cell_LOW_volt": {"min": 3.0, "max": 4.2},  # V per cell
            "battery_cell_HIGH_volt": {"min": 3.0, "max": 4.2},  # V per cell
            "battery_cell_LOW_temp": {"min": -10.0, "max": 50.0},  # °C
            "battery_cell_HIGH_temp": {"min": -10.0, "max": 50.0},  # °C
            "motor_temp": {"min": -10.0, "max": 80.0},  # °C
            "motor_controller_temp": {"min": -10.0, "max": 70.0},  # °C
            "BMS_temp": {"min": -10.0, "max": 60.0},  # °C
        }

        self.setWindowTitle("HUST - Henry Live Data")
        self.setGeometry(100, 100, 1280, 900)

        # Set the window icon to the logo
        self.setWindowIcon(QIcon(self.logo_path))
        
        # Set custom font
        self.setup_fonts()

        # Apply the default (light) style sheet
        self.apply_theme()

    def setup_fonts(self):
        """Setup custom fonts for the application."""
        # Try to use a modern font, fallback to system default
        font_families = [
            "Segoe UI",  # Windows modern
            "SF Pro Display",  # macOS modern
            "Ubuntu",  # Linux modern
            "Arial",  # Fallback
            "Helvetica"  # Final fallback
        ]
        
        self.app_font = QFont()
        for font_family in font_families:
            self.app_font.setFamily(font_family)
            if self.app_font.exactMatch():
                break
        
        # Set font properties
        self.app_font.setPointSize(9)
        self.app_font.setWeight(QFont.Normal)
        
        # Apply font to the application
        self.setFont(self.app_font)

        # Main layout: graphs on the left, COM port selection and messages on the right.
        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)
        self.main_layout = QHBoxLayout(self.central_widget)

        # --- Left layout ---
        self.left_layout = QVBoxLayout()

        # Buttons for graphs - styled container
        self.graph_buttons_container = QWidget()
        self.graph_buttons_container.setObjectName("graph_buttons_container")
        self.graph_buttons_layout = QHBoxLayout(self.graph_buttons_container)
        self.graph_buttons_layout.setSpacing(15)
        self.graph_buttons_layout.setContentsMargins(15, 8, 15, 8)  # Reduced vertical padding to make box thinner

        # Graph containers for individual graphs with their dropdowns
        self.graph_containers = []
        self.graph_dropdowns = []
        self.figures = []
        self.axes = []
        self.canvases = []

        # Time Interval Selector with better grouping
        time_interval_container = QWidget()
        time_interval_layout = QHBoxLayout(time_interval_container)
        time_interval_layout.setContentsMargins(0, 0, 0, 0)
        time_interval_layout.setSpacing(8)
        time_interval_label = QLabel("Time Interval:")
        time_interval_label.setStyleSheet("font-weight: bold;")
        self.time_unit_dropdown = QComboBox()
        self.time_unit_dropdown.addItems(["2 s", "1 min"])
        self.time_unit_dropdown.currentTextChanged.connect(self.update_all_graphs)
        self.time_unit_dropdown.setMinimumWidth(80)
        time_interval_layout.addWidget(time_interval_label)
        time_interval_layout.addWidget(self.time_unit_dropdown)
        self.graph_buttons_layout.addWidget(time_interval_container)

        # Add a spacer between time interval and buttons
        self.graph_buttons_layout.addSpacing(20)

        # Add buttons for adding and removing graphs with better grouping
        self.add_graph_button = QPushButton("+ Add Graph")
        self.add_graph_button.clicked.connect(self.add_graph)
        self.add_graph_button.setStyleSheet("color: white;")
        self.remove_graph_button = QPushButton("- Remove Graph")
        self.remove_graph_button.clicked.connect(self.remove_graph)
        self.remove_graph_button.setStyleSheet("color: white;")
        
        # Dark mode toggle button
        self.dark_mode_button = QPushButton("◐ Dark Mode")
        self.dark_mode_button.clicked.connect(self.toggle_dark_mode)
        self.dark_mode_button.setStyleSheet("color: white;")
        
        # Group buttons with consistent spacing
        buttons_layout = QHBoxLayout()
        buttons_layout.setSpacing(8)
        buttons_layout.setContentsMargins(0, 0, 0, 0)
        buttons_layout.addWidget(self.remove_graph_button)
        buttons_layout.addWidget(self.add_graph_button)
        
        # Add another separator before dark mode button
        separator2 = QWidget()
        separator2.setFixedWidth(2)
        separator2.setMinimumHeight(30)
        separator2.setObjectName("separator")
        buttons_layout.addWidget(separator2)
        buttons_layout.addWidget(self.dark_mode_button)
        
        self.graph_buttons_layout.addLayout(buttons_layout)
        
        # Add stretch to push logo to the right
        self.graph_buttons_layout.addStretch()
        
        # Add logo to the right side
        self.logo_label = QLabel(self)
        self.logo_label.setObjectName("logo_label")  # Set object name for CSS styling
        if os.path.exists(self.logo_path):
            # Load and scale the logo with transparency support
            self.logo_pixmap = QPixmap(self.logo_path)
            # Convert to format that supports transparency
            self.logo_pixmap = self.logo_pixmap.scaled(220, 70, Qt.KeepAspectRatio, Qt.SmoothTransformation)
            self.logo_label.setPixmap(self.logo_pixmap)
        else:
            print(f"[DEBUG] Logo file not found at: {self.logo_path}")
        self.logo_label.setMinimumSize(180, 60)
        self.logo_label.setMaximumSize(250, 80)
        self.logo_label.setAlignment(Qt.AlignCenter)
        self.logo_label.setStyleSheet("background: transparent; border: none;")
        self.logo_label.setAttribute(Qt.WA_TranslucentBackground, True)
        self.logo_label.setAutoFillBackground(False)
        self.logo_label.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Preferred)
        
        # RSSI Signal Strength Indicator
        self.rssi_label = QLabel("RSSI: -- dBm")
        self.rssi_label.setStyleSheet("color: #888888; font-weight: bold; font-size: 12px; padding: 5px; border: 2px solid #4472c4; border-radius: 4px;")
        self.rssi_label.setFixedSize(120, 30)
        self.rssi_label.setAlignment(Qt.AlignCenter)
        self.graph_buttons_layout.addWidget(self.rssi_label)

        self.graph_buttons_layout.setAlignment(Qt.AlignLeft)

        self.left_layout.addWidget(self.graph_buttons_container)

        self.main_layout.addLayout(self.left_layout, 8)

    
        # --- Right layout ---
        self.right_layout = QVBoxLayout()
        self.right_layout.setContentsMargins(5, 5, 5, 5)
        self.battery_and_speedometer_layout = QHBoxLayout()

        # Port Selector - moved to top right
        self.port_selector_layout = QHBoxLayout()
        self.port_selector_layout.setSpacing(5)
        self.port_selector_layout.setContentsMargins(0, 0, 0, 0)
        self.port_label = QLabel("COM Port:")
        self.port_label.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
        self.port_dropdown = QComboBox()
        self.port_dropdown.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Fixed)
        self.port_dropdown.setMaximumWidth(120)  # Limit dropdown width
        self.populate_com_ports()
        self.port_dropdown.currentTextChanged.connect(self.update_com_port)
        self.port_selector_layout.addWidget(self.port_label)
        self.port_selector_layout.addWidget(self.port_dropdown)
        self.port_selector_layout.addWidget(self.logo_label)  # Add logo next to COM port
        self.port_selector_layout.setAlignment(Qt.AlignLeft)  # Align to right
        self.right_layout.addLayout(self.port_selector_layout)

        self.text_box = QTextEdit()
        self.text_box.setReadOnly(True)
        self.text_box.setText("Application Ready.")
        self.text_box.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Preferred)
        self.text_box.setMaximumHeight(60)  # Smaller text box height
        self.right_layout.addWidget(self.text_box)

        # Create a scrollable area for live data
        self.live_data_scroll = QScrollArea()
        self.live_data_scroll.setWidgetResizable(True)
        self.live_data_scroll.setFixedHeight(450)  # Adjusted height for better layout
        self.live_data_scroll.setVerticalScrollBarPolicy(Qt.ScrollBarAsNeeded)
        self.live_data_scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        
        # Create the content widget for live data
        self.live_data_widget = QWidget()
        self.live_data_widget.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Preferred)
        self.live_data_layout = QVBoxLayout(self.live_data_widget)
        self.live_data_layout.setContentsMargins(5, 5, 5, 5)
        self.live_data_layout.setSpacing(2)
        self.live_data_layout.setSizeConstraint(QVBoxLayout.SetMinAndMaxSize)
        
        # Create labels and value boxes for each data field
        self.data_labels = {}
        self.data_values = {}
        for key in self.serial_reader.available_data:
            # Create a horizontal layout for each data row
            row_layout = QHBoxLayout()
            row_layout.setContentsMargins(0, 0, 0, 0)
            row_layout.setSpacing(5)
            
            # Create label (fixed width, bold)
            field = self.data_manager.get_field(key)
            if field:
                display_name = field.display_name
                unit = field.unit
                label_text = f"{display_name}:"
            else:
                label_text = f"{key}:"
            
            label = QLabel(label_text)
            label.setStyleSheet("font-size: 12px; font-weight: bold; padding: 2px;")
            label.setFixedWidth(150)  # Fixed width for alignment
            label.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
            label.setAlignment(Qt.AlignRight | Qt.AlignVCenter)
            
            # Create value box (separate text box)
            value_box = QLabel("--")
            value_box.setStyleSheet("font-size: 12px; padding: 2px; background-color: rgba(255,255,255,0.1); border: 1px solid rgba(255,255,255,0.2); border-radius: 3px;")
            value_box.setFixedWidth(100)  # Fixed width for values
            value_box.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
            value_box.setAlignment(Qt.AlignCenter)
            value_box.setWordWrap(True)
            
            # Add unit label if available
            if field and field.unit:
                unit_label = QLabel(f" {field.unit}")
                unit_label.setStyleSheet("font-size: 10px;")
                unit_label.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
                unit_label.setAlignment(Qt.AlignLeft | Qt.AlignVCenter)
                row_layout.addWidget(label)
                row_layout.addWidget(value_box)
                row_layout.addWidget(unit_label)
            else:
                row_layout.addWidget(label)
                row_layout.addWidget(value_box)
            
            row_layout.addStretch()  # Push content to the left
            
            # Store references
            self.data_labels[key] = label
            self.data_values[key] = value_box
            
            # Add row to main layout
            self.live_data_layout.addLayout(row_layout)
        
        self.live_data_scroll.setWidget(self.live_data_widget)
        self.right_layout.addWidget(self.live_data_scroll)

        # Add a loading label to indicate data is being received.
        self.loading_label = QLabel(self)
        self.loading_movie = QMovie(self.loading_gif_path)  # Ensure "loading.gif" exists in your folder
        self.loading_label.setMovie(self.loading_movie)
        self.loading_label.setVisible(False)
        # Add the loading label next to the COM port dropdown:
        self.port_selector_layout.addWidget(self.loading_label)

        # Speed Gauge Widget
        self.speedometer_widget = SpeedometerWidget()
        self.speedometer_widget.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
        self.battery_and_speedometer_layout.addWidget(self.speedometer_widget)

        # Battery Widget
        self.battery_widget = VerticalBatteryWidget()
        self.battery_widget.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)
        self.battery_widget.setFixedSize(100, 300)  # Increased size to match speedometer height
        self.battery_and_speedometer_layout.addWidget(self.battery_widget)

        # Set fixed size for battery and speedometer container
        self.battery_and_speedometer_layout.setContentsMargins(0, 0, 0, 0)
        self.battery_and_speedometer_layout.setSpacing(30)  # Increased spacing to prevent overlap
        self.right_layout.addLayout(self.battery_and_speedometer_layout)

        # Create a container widget for the right layout to control its size
        self.right_container = QWidget()
        self.right_container.setLayout(self.right_layout)
        self.right_container.setMaximumWidth(450)  # Increased maximum width
        self.right_container.setMinimumWidth(400)  # Increased minimum width
        self.right_container.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Preferred)
        self.main_layout.addWidget(self.right_container, 2)

        # Add initial graphs
        self.add_graph()
        self.add_graph()

        # Connect serial data signal to update the graphs.
        if hasattr(self.serial_reader, 'dataReceived'):
            self.serial_reader.dataReceived.connect(self.update_all_graphs)

        
        # --- CSV Logging Setup ---
        self.log_buffer = []  # buffer to accumulate log rows
        self.log_filename = "can_data_log.csv"
        # If the CSV file does not exist, create it and write a header row.
        if not os.path.exists(self.log_filename):
            with open(self.log_filename, "w", newline="") as f:
                writer = csv.writer(f)
                header = ["timestamp"] + self.serial_reader.available_data
                writer.writerow(header)
        # A timer to flush the log buffer to file once per minute.
        self.log_timer = QTimer(self)
        self.log_timer.timeout.connect(self.flush_log_buffer)
        self.log_timer.start(60000)  # 60000 ms = 1 minute

        # Log each new CAN message.
        self.serial_reader.dataReceived.connect(self.on_new_data)

        # number of samples in your buffer
        self.max_samples = len(self.serial_reader.history[self.serial_reader.available_data[0]])

        # Recover any saved CAN data from the CSV file.
        self.recover_csv_data()
        self.update_all_graphs()

        # For the mock reader, use a timer to update data every second.
        if isinstance(self.serial_reader, MockSerialReader):
            self.timer = QTimer(self)
            self.timer.timeout.connect(self.mock_update)
            self.timer.start(1000)

    def apply_theme(self):
        """Apply the current theme (light or dark) to the application."""
        if self.dark_mode:
            # Dark theme
            stylesheet = """
                QMainWindow {
                    background-color: #1a206c;
                }
                QWidget {
                    background-color: #1a206c;
                }
                QLabel {
                    color: #b0c4de;
                    font-weight: 500;
                    background-color: #1a206c;
                    padding: 2px 5px;
                    border-radius: 3px;
                    font-family: "Segoe UI", "SF Pro Display", "Ubuntu", Arial, Helvetica;
                    font-size: 9pt;
                }
                QComboBox {
                    background-color: #e8f0ff;
                    color: #1a206c;
                    border: 2px solid #4472c4;
                    padding: 5px;
                    border-radius: 4px;
                    font-family: "Segoe UI", "SF Pro Display", "Ubuntu", Arial, Helvetica;
                    font-size: 9pt;
                }
                QComboBox:hover {
                    background-color: #d0e0f0;
                    border: 2px solid #5b9bd5;
                }
                QComboBox::drop-down {
                    border: none;
                }
                QComboBox QAbstractItemView {
                    background-color: #e8f0ff;
                    selection-background-color: #5b9bd5;
                    color: #1a206c;
                }
                QPushButton {
                    background-color: #2d4a7c;
                    color: #e0e8f5;
                    border: 2px solid #4472c4;
                    padding: 8px 15px;
                    border-radius: 5px;
                    font-family: "Segoe UI", "SF Pro Display", "Ubuntu", Arial, Helvetica;
                    font-size: 9pt;
                }
                QPushButton:hover {
                    background-color: #3a5a8c;
                    border: 2px solid #5b9bd5;
                }
                QPushButton:pressed {
                    background-color: #1f3a5c;
                    border: 2px solid #4472c4;
                }
                QPushButton:disabled {
                    background-color: #3a4a6c;
                    color: #888888;
                    border: 2px solid #555555;
                }
                QTextEdit {
                    background-color: #2d4a7c;
                    color: #e0e8f5;
                    border: 2px solid #4472c4;
                    border-radius: 4px;
                    padding: 5px;
                }
                QScrollArea {
                    background-color: #2d4a7c;
                    border: 2px solid #4472c4;
                    border-radius: 4px;
                }
                QScrollArea QWidget {
                    background-color: #2d4a7c;
                }
                QWidget#graph_buttons_container {
                    background-color: #2d4a7c;
                    border: 2px solid #4472c4;
                    border-radius: 8px;
                }
                QWidget#separator {
                    background-color: #4472c4;
                    border-radius: 1px;
                }
                QWidget#graph_container {
                    background-color: #2d4a7c;
                    border: 1px solid #4472c4;
                    border-radius: 6px;
                }
                QLabel#logo_label {
                    background: transparent;
                    border: none;
                }
            """
            # When in dark mode, button should say "Light Mode" to switch back
            if hasattr(self, 'dark_mode_button'):
                self.dark_mode_button.setText("◐ Light Mode")
        else:
            # Light theme (original)
            stylesheet = """
                QMainWindow {
                    background-color: #e8f0ff;
                }
                QWidget {
                    background-color: #e8f0ff;
                }
                QLabel {
                    color: #1a206c;
                    font-weight: 500;
                    background-color: #e8f0ff;
                    padding: 2px 5px;
                    border-radius: 3px;
                    font-family: "Segoe UI", "SF Pro Display", "Ubuntu", Arial, Helvetica;
                    font-size: 9pt;
                }
                QComboBox {
                    background-color: #f0f8ff;
                    color: #1a206c;
                    border: 2px solid #4472c4;
                    padding: 5px;
                    border-radius: 4px;
                    font-family: "Segoe UI", "SF Pro Display", "Ubuntu", Arial, Helvetica;
                    font-size: 9pt;
                }
                QComboBox:hover {
                    background-color: #e0f0ff;
                    border: 2px solid #5b9bd5;
                }
                QComboBox::drop-down {
                    border: none;
                }
                QComboBox QAbstractItemView {
                    background-color: #f0f8ff;
                    selection-background-color: #5b9bd5;
                    color: #1a206c;
                }
                QPushButton {
                    background-color: #4472c4;
                    color: white;
                    border: 2px solid #5b9bd5;
                    padding: 8px 15px;
                    border-radius: 5px;
                    font-family: "Segoe UI", "SF Pro Display", "Ubuntu", Arial, Helvetica;
                    font-size: 9pt;
                }
                QPushButton:hover {
                    background-color: #4472c4;
                    border: 2px solid #5b9bd5;
                }
                QPushButton:pressed {
                    background-color: #3a5a8c;
                    border: 2px solid #4472c4;
                }
                QPushButton:disabled {
                    background-color: #a8c8e8;
                    color: #666666;
                    border: 2px solid #888888;
                }
                QTextEdit {
                    background-color: #d4e4f7;
                    color: #1a206c;
                    border: 2px solid #7fb3d3;
                    border-radius: 4px;
                    padding: 5px;
                }
                QScrollArea {
                    background-color: #d4e4f7;
                    border: 2px solid #7fb3d3;
                    border-radius: 4px;
                }
                QScrollArea QWidget {
                    background-color: #d4e4f7;
                }
                QWidget#graph_buttons_container {
                    background-color: #d4e4f7;
                    border: 2px solid #7fb3d3;
                    border-radius: 8px;
                }
                QWidget#separator {
                    background-color: #7fb3d3;
                    border-radius: 1px;
                }
                QWidget#graph_container {
                    background-color: #d4e4f7;
                    border: 1px solid #7fb3d3;
                    border-radius: 6px;
                }
                QLabel#logo_label {
                    background: transparent;
                    border: none;
                }
            """
            # When in light mode, button should say "Dark Mode" to switch
            if hasattr(self, 'dark_mode_button'):
                self.dark_mode_button.setText("◐ Dark Mode")
        
        self.setStyleSheet(stylesheet)
    
    def toggle_dark_mode(self):
        """Toggle between light and dark mode."""
        self.dark_mode = not self.dark_mode
        self.apply_theme()
        # Update figure backgrounds for existing graphs
        plot_style = self.get_plot_style()
        for figure in self.figures:
            figure.patch.set_facecolor(plot_style['figure_bg'])
        # Update all graphs to reflect the new theme
        self.update_all_graphs()
    
    def get_plot_colors(self):
        """Get the appropriate color palette based on current theme."""
        if self.dark_mode:
            # Lighter colors for dark background
            return ['#60a5fa', '#93c5fd', '#7fb3d3', '#5b9bd5', '#3e81f0', '#a4c8e8', '#b0d4ff']
        else:
            # Darker colors for light background
            return ['#1e3a8a', '#2563eb', '#3b82f6', '#4472c4', '#2d6fc7', '#5b9bd5', '#6366f1']
    
    def _add_annotation(self, ax, text, y_pos, plot_style):
        """Helper method to add styled annotations to plots."""
        ax.annotate(text, xy=(0.05, y_pos), xycoords='axes fraction', fontsize=10,
                   horizontalalignment='left', verticalalignment='top',
                   bbox=dict(boxstyle="round,pad=0.3", edgecolor=plot_style['annotation_edge'],
                            facecolor=plot_style['annotation_bg'], linewidth=1.5),
                   color=plot_style['annotation_text'])

    def get_plot_style(self):
        """Get the plot styling parameters based on current theme."""
        if self.dark_mode:
            return {
                'bg_color': '#2d4a7c',
                'spine_color': '#7fb3d3',
                'tick_color': '#b0c4de',
                'title_color': '#e0e8f5',
                'label_color': '#e0e8f5',
                'legend_bg': '#2d4a7c',
                'legend_edge': '#7fb3d3',
                'legend_text': '#e0e8f5',
                'annotation_bg': '#3a5a8c',
                'annotation_edge': '#7fb3d3',
                'annotation_text': '#e0e8f5',
                'default_line': '#60a5fa',
                'figure_bg': '#1a206c'
            }
        else:
            return {
                'bg_color': '#f0f7ff',
                'spine_color': '#4472c4',
                'tick_color': '#1a206c',
                'title_color': '#1a206c',
                'label_color': '#1a206c',
                'legend_bg': '#e8f0ff',
                'legend_edge': '#4472c4',
                'legend_text': '#1a206c',
                'annotation_bg': '#d4e4f7',
                'annotation_edge': '#4472c4',
                'annotation_text': '#1a206c',
                'default_line': '#2563eb',
                'figure_bg': '#e8f0ff'
            }

    def populate_com_ports(self):
        ports = [port.device for port in serial.tools.list_ports.comports()]
        self.port_dropdown.clear()
        self.port_dropdown.addItems(ports)

    def update_com_port(self, port):
        self.serial_reader.set_port(port)
        if port:
            self.start_loading()
            # Create a QThread and ConnectionWorker.
            self.connection_thread = QThread()
            self.connection_worker = ConnectionWorker(self.serial_reader, port)
            self.connection_worker.moveToThread(self.connection_thread)
            self.connection_thread.started.connect(self.connection_worker.run)
            self.connection_worker.finished.connect(self.on_connection_finished)
            self.connection_worker.finished.connect(self.connection_thread.quit)
            self.connection_worker.finished.connect(self.connection_worker.deleteLater)
            self.connection_thread.finished.connect(self.connection_thread.deleteLater)
            self.connection_thread.start()

    def mock_update(self):
        self.serial_reader.update()
        self.update_all_graphs()

    def get_y_label(self, variable):
        """Return an appropriate unit string based on the variable name."""
        return self.data_manager.get_unit(variable)

    def get_rounded_timestamp(self):
        """
        Get the current timestamp rounded to the nearest second.
        If microseconds are 500,000 or more, round up; otherwise, round down.
        Returns a string in ISO format without decimals.
        """
        now = datetime.datetime.now()
        # Add 500,000 microseconds and then replace microseconds with 0.
        rounded = (now + datetime.timedelta(microseconds=500000)).replace(microsecond=0)
        return rounded.isoformat()

    def update_graph(self, graph_id):
        # Determine the sample interval in seconds from the dropdown.
        time_option = self.time_unit_dropdown.currentText()
        if time_option == "2 s":
            sample_interval = 2
        elif time_option == "1 min":
            sample_interval = 60
        else:
            sample_interval = 2

        # Get the current time (for graphing, we don't require rounding here).
        now = datetime.datetime.now()

        # Select the appropriate axis and dropdown based on graph_id
        ax = self.axes[graph_id - 1]
        canvas = self.canvases[graph_id - 1]
        selected_var = self.graph_dropdowns[graph_id - 1].currentText()

        ax.clear()

        # Define graph configurations using the data manager
        graph_configs = {
            "Velocity": {
                "data_keys": ["velocity"],
                "ylabel": self.data_manager.get_unit("velocity"),
                "annotations": []
            },
            "Battery Temps": {
                "data_keys": ["battery_cell_LOW_temp", "battery_cell_HIGH_temp", "battery_cell_AVG_temp", "BMS_temp"],
                "ylabel": self.data_manager.get_unit("battery_cell_LOW_temp"),
                "annotations": [
                    ("battery_cell_ID_LOW_temp", "Low Temp ID: {value}"),
                    ("battery_cell_ID_HIGH_temp", "High Temp ID: {value}")
                ]
            },
            "Motor Temps": {
                "data_keys": ["motor_temp", "motor_controller_temp"],
                "ylabel": self.data_manager.get_unit("motor_temp"),
                "annotations": []
            },
            "MPPT power": {
                "data_keys": ["MPPT1_watt", "MPPT2_watt", "MPPT3_watt", "MPPT_total_watt"],
                "ylabel": self.data_manager.get_unit("MPPT1_watt"),
                "annotations": []
            },
            "Power trade-off": {
                "data_keys": ["battery_current", "battery_volt", "MPPT_total_watt"],
                "ylabel": "W",
                "annotations": [],
                "custom_processing": True
            }
        }

        config = graph_configs.get(selected_var, {
            "data_keys": [selected_var],
            "ylabel": self.data_manager.get_unit(selected_var),
            "annotations": []
        })

        if config.get("custom_processing", False):
            # Handle power trade-off calculation
            battery_current = self.serial_reader.history.get("battery_current", [])
            battery_voltage = self.serial_reader.history.get("battery_volt", [])
            total_mppt_watt = self.serial_reader.history.get("MPPT_total_watt", [])

            min_length = min(len(battery_current), len(battery_voltage), len(total_mppt_watt))
            battery_power = [battery_current[i] * battery_voltage[i] for i in range(min_length)]

            data = {
                "Battery Power": battery_power,
                "Total MPPT Power": total_mppt_watt[:min_length],
            }
            ylabel = config["ylabel"]
            
            # Add power difference annotation
            power_difference = -battery_power[-1] + total_mppt_watt[-1]
            config["annotations"].append(("custom", f"Power Difference: {power_difference:.2f} W"))
        else:
            # Standard processing
            if len(config["data_keys"]) == 1:
                data = self.serial_reader.history.get(config["data_keys"][0], [])
            else:
                data = {key: self.serial_reader.history.get(key, []) for key in config["data_keys"]}
            ylabel = config["ylabel"]

        # Add annotations
        plot_style = self.get_plot_style()
        for i, (key, template) in enumerate(config["annotations"]):
            if key == "custom":
                self._add_annotation(ax, template, 0.95 - i * 0.1, plot_style)
            else:
                value = self.serial_reader.latest_values.get(key, "N/A")
                text = template.format(value=value)
                self._add_annotation(ax, text, 0.95 - i * 0.1, plot_style)

        # Get theme-appropriate colors and styling
        blue_colors = self.get_plot_colors()
        plot_style = self.get_plot_style()
        
        # Plot the data
        if isinstance(data, dict):
            color_idx = 0
            for label, values in data.items():
                times = [now - datetime.timedelta(seconds=(len(values) - 1 - i) * sample_interval)
                        for i in range(len(values))]
                x = mdates.date2num(times)
                color = blue_colors[color_idx % len(blue_colors)]
                ax.plot(x, values, '-', label=self.data_manager.get_display_name(label), 
                       color=color, linewidth=2)
                color_idx += 1
            ax.legend(loc='best', framealpha=0.9, facecolor=plot_style['legend_bg'], 
                     edgecolor=plot_style['legend_edge'], labelcolor=plot_style['legend_text'])
        else:
            times = [now - datetime.timedelta(seconds=(len(data) - 1 - i) * sample_interval)
                    for i in range(len(data))]
            x = mdates.date2num(times)
            ax.plot(x, data, '-', color=plot_style['default_line'], linewidth=2)

        # Set fixed y-axis limits based on data type
        self._set_y_axis_limits(ax, selected_var, data, config)
        
        # Style the plot based on current theme
        ax.set_facecolor(plot_style['bg_color'])
        ax.spines['top'].set_visible(False)
        ax.spines['right'].set_visible(False)
        ax.spines['left'].set_color(plot_style['spine_color'])
        ax.spines['bottom'].set_color(plot_style['spine_color'])
        ax.tick_params(colors=plot_style['tick_color'])
        
        # Title removed as requested
        ax.set_xlabel("Time", color=plot_style['label_color'], fontsize=10)
        ax.set_ylabel(ylabel, color=plot_style['label_color'], fontsize=10)
        ax.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M:%S'))
        ax.xaxis.set_major_locator(mticker.MaxNLocator(4))
        canvas.draw()

    def _set_y_axis_limits(self, ax, selected_var, data, config):
        """Set fixed y-axis limits based on data type, expanding only if data exceeds limits."""
        # Handle special cases
        if config.get("custom_processing", False):
            # For power trade-off graph, use power-related limits
            max_expected = 5000.0  # 5kW max power
            if isinstance(data, dict) and data:
                current_max = max(max(values) if values else 0 for values in data.values())
            else:
                current_max = 0
        else:
            # Get the maximum expected value for this data type
            if isinstance(data, dict):
                # For multi-line plots, check all data series
                max_expected = 0
                current_max = 0
                for key, values in data.items():
                    if values:
                        field_max = self.data_manager.get_max_expected_value(key)
                        max_expected = max(max_expected, field_max)
                        current_max = max(current_max, max(values))
            else:
                # For single-line plots
                if data:
                    max_expected = self.data_manager.get_max_expected_value(selected_var)
                    current_max = max(data)
                else:
                    max_expected = 100.0  # Default
                    current_max = 0
        
        # Set y-axis limits
        if current_max > max_expected:
            # If current data exceeds expected max, expand the axis
            y_max = current_max * 1.1  # Add 10% margin
        else:
            # Use the fixed expected maximum
            y_max = max_expected
        
        # Set minimum - allow negative values for power (can be negative when consuming)
        if any(key in selected_var.lower() for key in ['power', 'watt', 'trade']):
            y_min = min(0, current_max * 0.1) if data else -1000  # Allow negative power
        elif any(key in selected_var.lower() for key in ['temp', 'temperature']):
            y_min = min(0, current_max * 0.1) if data else 0
        else:
            y_min = 0
        
        ax.set_ylim(y_min, y_max)

    def update_all_graphs(self):
        for i in range(1, len(self.graph_dropdowns) + 1):
            self.update_graph(i)
        self.update_battery_widget()
        self.update_speedometer_widget()
        self.update_live_data_display()
        self.update_rssi_display()

    def update_battery_widget(self):
        battery_volt = self.serial_reader.latest_values.get("battery_volt", 0.0)
        self.battery_widget.setVoltage(battery_volt)

    def update_speedometer_widget(self):
        velocity = self.serial_reader.latest_values.get("velocity", 0.0)
        distance = self.serial_reader.latest_values.get("distance_travelled", 0.0)
        self.speedometer_widget.updateSpeedometer(velocity, distance)

    def update_live_data_display(self):
        """Update the live data display with current serial values."""
        for key, value_box in self.data_values.items():
            current_value = self.serial_reader.latest_values.get(key, None)
            field = self.data_manager.get_field(key)
            
            # Check for critical values
            warning_icon = ""
            if current_value is not None and key in self.critical_thresholds:
                thresholds = self.critical_thresholds[key]
                if current_value < thresholds["min"] or current_value > thresholds["max"]:
                    warning_icon = "⚠️ "
            
            if current_value is not None:
                # Format the value based on its type
                if isinstance(current_value, (int, float)):
                    if abs(current_value) < 0.01 and current_value != 0:
                        formatted_value = f"{current_value:.4f}"
                    elif abs(current_value) < 1:
                        formatted_value = f"{current_value:.3f}"
                    else:
                        formatted_value = f"{current_value:.2f}"
                else:
                    formatted_value = str(current_value)
                
                value_text = f"{warning_icon}{formatted_value}"
            else:
                value_text = "--"
            
            value_box.setText(value_text)

    def update_rssi_display(self):
        """Update the RSSI signal strength display."""
        rssi_value = self.serial_reader.latest_values.get("rssi", None)
        if rssi_value is not None:
            # Format RSSI value
            formatted_rssi = f"{rssi_value:.1f}"
            
            # Color code based on signal strength
            if rssi_value >= -50:
                color = "#00ff00"  # Green - Excellent
                status = "Excellent"
            elif rssi_value >= -70:
                color = "#ffff00"  # Yellow - Good
                status = "Good"
            elif rssi_value >= -85:
                color = "#ff8800"  # Orange - Fair
                status = "Fair"
            else:
                color = "#ff0000"  # Red - Poor
                status = "Poor"
            
            self.rssi_label.setText(f"RSSI: {formatted_rssi} dBm")
            self.rssi_label.setStyleSheet(f"color: {color}; font-weight: bold; font-size: 12px; padding: 5px; border: 2px solid #4472c4; border-radius: 4px;")
        else:
            self.rssi_label.setText("RSSI: -- dBm")
            self.rssi_label.setStyleSheet("color: #888888; font-weight: bold; font-size: 12px; padding: 5px; border: 2px solid #4472c4; border-radius: 4px;")

    def on_new_data(self):
        """
        This slot is called every time a new CAN data packet is received.
        It formats the latest values as a CSV row (prefixed with a rounded timestamp)
        and appends it to the logging buffer.
        """
        timestamp = self.get_rounded_timestamp()
        data_list = [str(self.serial_reader.latest_values.get(key, "")) for key in self.serial_reader.available_data]
        csv_row = [timestamp] + data_list
        self.log_buffer.append(csv_row)
        insert_into_db(self.serial_reader.latest_values)
        self.update_all_graphs()

    def flush_log_buffer(self):
        """Write the contents of the log buffer to the CSV file and clear the buffer."""
        if not self.log_buffer:
            return
        try:
            with open(self.log_filename, "a", newline="") as f:
                writer = csv.writer(f)
                writer.writerows(self.log_buffer)
        except Exception as e:
            print(f"[DEBUG] Error writing to log file: {e}")
        self.log_buffer.clear()

    def recover_csv_data(self):
        """Read only the *last* max_samples lines and fill history."""
        if not os.path.exists(self.log_filename):
            return

        try:
            with open(self.log_filename, "r", newline="") as f:
                reader = csv.reader(f)
                rows = list(reader)
        except Exception as e:
            print(f"[DEBUG] Error reading CSV file: {e}")
            return

        # Drop header
        if rows and rows[0][0].lower() == "timestamp":
            rows = rows[1:]

        # Take only the tail
        recent = rows[-self.max_samples:]
        start = self.max_samples - len(recent)

        # Prepare empty buffers
        recovered = {key: [0.0] * self.max_samples for key in self.serial_reader.available_data}

        for idx, row in enumerate(recent):
            # Fill each variable
            for j, var in enumerate(self.serial_reader.available_data):
                try:
                    val = float(row[1+j])
                except (ValueError, IndexError):
                    val = 0.0
                recovered[var][start+idx] = val

        # Commit back into reader
        for var in self.serial_reader.available_data:
            self.serial_reader.history[var] = recovered[var].copy()
            # latest_values should reflect the last recovered sample
            self.serial_reader.latest_values[var] = recovered[var][-1]

        print("[DEBUG] CSV data recovery complete.")

    def start_loading(self):
        self.loading_label.setVisible(True)
        self.loading_movie.start()

    def stop_loading(self):
        self.loading_movie.stop()
        self.loading_label.setVisible(False)

    def on_connection_finished(self, success):
        self.stop_loading()
        if not success:
            print(f"[DEBUG] Failed to open port {self.serial_reader.selected_port}")

    def add_graph(self):
        if len(self.graph_dropdowns) >= MAX_GRAPHS:
            return

        # Create a container for this graph and its dropdown
        graph_container = QWidget()
        graph_container.setObjectName("graph_container")
        container_layout = QVBoxLayout(graph_container)
        container_layout.setContentsMargins(10, 10, 10, 10)
        container_layout.setSpacing(5)

        # Create dropdown selector for this graph
        dropdown_layout = QHBoxLayout()
        dropdown_layout.setContentsMargins(0, 0, 0, 0)
        dropdown_layout.setSpacing(10)
        
        graph_label = QLabel(f"Graph {len(self.graph_dropdowns) + 1}:")
        graph_label.setStyleSheet("font-weight: bold;")
        
        graph_dropdown = QComboBox()
        graph_dropdown.addItems(GRAPH_TYPES)
        graph_dropdown.setCurrentText("Velocity")
        graph_dropdown.currentTextChanged.connect(lambda _, id=len(self.graph_dropdowns) + 1: self.update_graph(id))
        
        dropdown_layout.addWidget(graph_label)
        dropdown_layout.addWidget(graph_dropdown)
        dropdown_layout.addStretch()  # Push dropdown to the left
        
        container_layout.addLayout(dropdown_layout)

        # Create the figure and canvas
        figure, ax = plt.subplots()
        plot_style = self.get_plot_style()
        figure.patch.set_facecolor(plot_style['figure_bg'])
        canvas = FigureCanvas(figure)
        
        container_layout.addWidget(canvas)

        # Store references
        self.graph_containers.append(graph_container)
        self.graph_dropdowns.append(graph_dropdown)
        self.figures.append(figure)
        self.axes.append(ax)
        self.canvases.append(canvas)

        # Add the container to the main layout
        self.left_layout.addWidget(graph_container)

        # Update all graphs to reflect the changes
        self.update_all_graphs()

        # Update buttons
        self.remove_graph_button.setDisabled(False)
        if len(self.graph_dropdowns) >= MAX_GRAPHS:
            self.add_graph_button.setDisabled(True)

    def remove_graph(self):
        if len(self.graph_dropdowns) <= 1:
            return

        # Remove the last graph container and all its components
        graph_container = self.graph_containers.pop()
        graph_dropdown = self.graph_dropdowns.pop()
        figure = self.figures.pop()
        ax = self.axes.pop()
        canvas = self.canvases.pop()

        # Remove the container from the layout
        self.left_layout.removeWidget(graph_container)
        graph_container.deleteLater()

        # Update all graphs to reflect the changes
        self.update_all_graphs()

        # Update buttons
        self.add_graph_button.setDisabled(False)
        if len(self.graph_dropdowns) <= 1:
            self.remove_graph_button.setDisabled(True)

