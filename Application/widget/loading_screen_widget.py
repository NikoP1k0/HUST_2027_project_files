from PyQt5.QtWidgets import QApplication, QDialog, QVBoxLayout, QLabel, QProgressBar
from PyQt5.QtGui import QPixmap
from PyQt5.QtCore import Qt, QTimer
import sys

class LoadingScreen(QDialog):
    def __init__(self, image_path, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Loading...")
        self.setFixedSize(600, 300)
        self.setWindowFlags(Qt.FramelessWindowHint)

        # Layout
        layout = QVBoxLayout(self)

        # Loading Image
        self.image_label = QLabel(self)
        pixmap = QPixmap(image_path)
        self.image_label.setPixmap(pixmap.scaled(400, 400, Qt.KeepAspectRatio))
        self.image_label.setAlignment(Qt.AlignCenter)
        layout.addWidget(self.image_label)

        # Loading Bar
        self.progress_bar = QProgressBar(self)
        self.progress_bar.setRange(0, 100)
        self.progress_bar.setValue(0)
        layout.addWidget(self.progress_bar)

        # Timer to simulate loading progress
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update_progress)
        self.timer.start(25)  # Update every 100 ms

        self.current_progress = 0

    def update_progress(self):
        if self.current_progress < 100:
            self.current_progress += 4
            self.progress_bar.setValue(min(self.current_progress, 100))
        else:
            self.timer.stop()
            self.accept()  # Close the loading screen
