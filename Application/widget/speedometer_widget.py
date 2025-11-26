from PyQt5.QtWidgets import QWidget
from PyQt5.QtGui import QPainter, QColor, QPen, QFont
from PyQt5.QtCore import Qt, QRectF, QPoint
import math

class SpeedometerWidget(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.distance = 0.0
        self.velocity = 0.0
        self.max_velocity = 200.0  # Set the maximum velocity for the speedometer
        self.setFixedSize(300, 300)  # Set the size of the speedometer widget
        self.center = QPoint()
        self.center.setX(150)
        self.center.setY(150)    

    def updateSpeedometer(self, velocity, distance):
        self.velocity = velocity
        self.distance = distance
        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)

        # Draw the speedometer background
        painter.setPen(QPen(QColor("#3e81f0"), 3))
        painter.drawEllipse(10, 10, 280, 280)
        painter.setBrush(QColor(255, 255, 255))  # Light blue background
        painter.drawEllipse(10, 10, 280, 280)

        # Draw the speedometer scale
        painter.setPen(QPen(Qt.black, 2))
        for i in range(0, 201, 20):
            angle = math.radians(i * 1.2 - 210)
            x = 150 + 118 * math.cos(angle)
            y = 150 + 118 * math.sin(angle)
            painter.drawText(QRectF(x - 20, y - 20, 40, 40), Qt.AlignCenter, str(i))

            # Draw thick line behind each number
            painter.setPen(QPen(Qt.black, 3))
            x = int(150 + 130 * math.cos(angle))
            y = int(150 + 130 * math.sin(angle))
            x_edge = int(150 + 139 * math.cos(angle))
            y_edge = int(150 + 139 * math.sin(angle))
            painter.drawLine(x, y, x_edge, y_edge)

            # Draw 5 thin lines between each number
            if i < 200:
                for j in range(1, 6):
                    thin_angle = angle + j * (4.0 * math.pi / 180)
                    thin_x = int(150 + 132 * math.cos(thin_angle))
                    thin_y = int(150 + 132 * math.sin(thin_angle))
                    thin_x_edge = int(150 + 139 * math.cos(thin_angle))
                    thin_y_edge = int(150 + 139 * math.sin(thin_angle))
                    painter.setPen(QPen(Qt.black, 1))
                    painter.drawLine(thin_x_edge, thin_y_edge, thin_x, thin_y)

        # Draw the needle
        angle = math.radians(self.velocity * 1.2 - 210)
        x =  int(160 + 120 * math.cos(angle))
        y = int(150 + 120 * math.sin(angle))
        p = QPoint()
        p.setX(x)
        p.setY(y)
        painter.setPen(QPen(QColor(20, 52, 164), 2)) # Dark blue needle
        painter.drawLine(self.center, p)

        # Draw a circle in the middle
        painter.setBrush(Qt.black)
        painter.drawEllipse(self.center, 7, 7)

        # Draw the velocity value
        painter.setFont(QFont('Arial', 20))
        painter.setPen(QPen(Qt.black, 2))
        painter.drawText(QRectF(50, 190, 200, 50), Qt.AlignCenter, f"{self.velocity:.1f} km/h")

        # Draw the distance travelled value
        painter.setFont(QFont('Arial', 12))
        painter.drawText(QRectF(50, 250, 200, 20), Qt.AlignCenter, f"{self.distance:.1f} km")

        painter.end()
