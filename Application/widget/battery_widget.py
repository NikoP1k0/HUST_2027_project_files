from PyQt5.QtWidgets import QWidget, QLabel, QVBoxLayout
from PyQt5.QtGui import QPainter, QColor, QPen, QFont
from PyQt5.QtCore import Qt

class BatteryWidget(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.voltage = 140.0
        self.max_voltage = 140.0  # Set the maximum voltage for the battery
        self.setFixedSize(70, 220)  # Set the size of the battery widget

    def setVoltage(self, voltage):
        self.voltage = voltage
        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)

        # Draw the battery outline
        painter.setPen(QPen(QColor("#3e81f0"), 3, join=Qt.BevelJoin))
        painter.setBrush(Qt.NoBrush)
        painter.drawRect(10, 10, 50, 200)  # Battery body
        painter.drawRect(25, 2, 20, 8) # Battery cap

        # Calculate the fill level based on the voltage
        fill_level = max(0, min(1, self.voltage / self.max_voltage))
        fill = int(fill_level * 200)

        # Draw the battery fill
        if (self.voltage > 0.5 * self.max_voltage):
            painter.setBrush(QColor(0, 255, 0))  # Green color for the fill
        elif(self.voltage > 0.2 * self.max_voltage):
            painter.setBrush(QColor(250, 250, 0)) # Yellow when in middle  
        else: 
            painter.setBrush(QColor(250, 0, 0)) # Red color when low
        
        painter.drawRect(10, 210, 50, -fill)

        painter.end()

class VerticalBatteryWidget(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.battery_widget = BatteryWidget(self)
        self.percentage_label = QLabel(self)
        self.percentage_label.setFont(QFont("Times", 12))
        self.percentage_label.setAlignment(Qt.AlignCenter)

        layout = QVBoxLayout()
        layout.addWidget(self.battery_widget)
        layout.addWidget(self.percentage_label)
        layout.setAlignment(Qt.AlignBottom)
        self.setLayout(layout)

        self.updatePercentageLabel()

    def setVoltage(self, voltage):
        self.battery_widget.setVoltage(voltage)
        self.updatePercentageLabel()

    def updatePercentageLabel(self):
        percentage = (self.battery_widget.voltage / self.battery_widget.max_voltage) * 100
        self.percentage_label.setText(f"{percentage:.1f} %")
