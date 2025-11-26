import sys
from PyQt5.QtWidgets import QApplication
from plot_app import PlotApp
from serial_reader import SerialReader
from mock_serial_reader import MockSerialReader
# from widget.loading_screen_widget import LoadingScreen

if __name__ == "__main__":
    use_mock = True # Change to True if you want to use fake data for testing
    serial_reader = MockSerialReader() if use_mock else SerialReader()
    app = QApplication(sys.argv)
    # Show the loading screen first
    # loading_screen = LoadingScreen("assets/HUST_big_logo.png")
    # loading_screen.show()
    # loading_screen.exec_()  # Block until the loading screen is closed
    main_window = PlotApp(serial_reader)
    main_window.showMaximized()
    if not use_mock:
        serial_reader.start()
    try:
        sys.exit(app.exec_())
    finally:
        serial_reader.stop()
