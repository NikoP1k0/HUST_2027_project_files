from PyQt5.QtCore import QObject, QThread, pyqtSignal

class ConnectionWorker(QObject):
    finished = pyqtSignal(bool)  # Emit True on success, False on failure

    def __init__(self, serial_reader, port):
        super().__init__()
        self.serial_reader = serial_reader
        self.port = port

    def run(self):
        # The blocking connection attempt.
        try:
            # This assumes that serial_reader.set_port() has already been called.
            self.serial_reader.start()  
            self.finished.emit(True)
        except Exception as e:
            print(f"[DEBUG] Failed to open port {self.port}: {e}")
            self.finished.emit(False)
