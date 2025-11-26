import serial
import threading
import serial.tools.list_ports
import re
from PyQt5.QtCore import pyqtSignal, QObject
from base_serial_reader import BaseSerialReader

class SerialReader(QObject, BaseSerialReader):
    dataReceived = pyqtSignal()

    def __init__(self):
        QObject.__init__(self)
        BaseSerialReader.__init__(self)
        self.ser = None
        self.thread = None

    def start(self):
        # Auto-select the only available COM port if none explicitly chosen
        if not self.selected_port:
            ports = [p.device for p in serial.tools.list_ports.comports()]
            if len(ports) == 1:
                self.selected_port = ports[0]
                print(f"[DEBUG] Only one COM port available, auto-selecting {self.selected_port}")
            else:
                print("[DEBUG] No COM port selected or multiple ports available, please select one.")
                return
        try:
            self.ser = serial.Serial(self.selected_port, 9600)
            self.running = True
            self.thread = threading.Thread(target=self.read_serial_data, daemon=True)
            self.thread.start()
            print(f"[DEBUG] Serial reader started on {self.selected_port}.")
        except serial.SerialException as e:
            print(f"[DEBUG] Failed to open port {self.selected_port}: {e}")

    def stop(self):
        self.running = False
        if self.thread:
            self.thread.join()
        if self.ser and self.ser.is_open:
            self.ser.close()
        print("[DEBUG] Serial reader stopped.")

    def read_serial_data(self):
        print("[DEBUG] Serial reading thread running.")
        while self.running:
            if self.ser and self.ser.in_waiting > 0:
                line = self.ser.readline().decode('utf-8', errors='ignore').strip()
                print(f"[DEBUG] Raw line read: '{line}'")

                # Determine payload: check for 'LoRa data:' or quoted payload
                if 'LoRa data:' in line:
                    data_string = line.split('LoRa data:')[1].strip()
                else:
                    start = line.find("'")
                    end = line.find("'", start + 1)
                    if start == -1 or end == -1:
                        print(f"[DEBUG] No payload marker found in line: {line}")
                        continue
                    data_string = line[start+1:end]

                # Allow only digits, minus, dot and spaces
                if not re.fullmatch(r'[-0-9. ]+', data_string):
                    print(f"[DEBUG] Invalid characters in payload, skipping: {data_string}")
                    continue

                parts = data_string.split()
                # Truncate or pad to exactly 20 items
                if len(parts) > 20:
                    parts = parts[:20]
                elif len(parts) < 20:
                    parts += ['0.0'] * (20 - len(parts))

                # Convert to floats
                try:
                    values = [float(p) for p in parts]
                except ValueError as e:
                    print(f"[DEBUG] Conversion error: {e}, using zeros")
                    values = [0.0] * 20

                # Unpack values into attributes
                (
                    self.velocity,
                    self.distance_travelled,
                    self.battery_volt,
                    self.battery_current,
                    self.battery_cell_LOW_volt,
                    self.battery_cell_HIGH_volt,
                    self.battery_cell_AVG_volt,
                    self.battery_cell_LOW_temp,
                    self.battery_cell_HIGH_temp,
                    self.battery_cell_AVG_temp,
                    self.battery_cell_ID_HIGH_temp,
                    self.battery_cell_ID_LOW_temp,
                    self.BMS_temp,
                    self.motor_current,
                    self.motor_temp,
                    self.motor_controller_temp,
                    self.MPPT1_watt,
                    self.MPPT2_watt,
                    self.MPPT3_watt,
                    self.MPPT_total_watt
                ) = values

                # Update latest values
                for var in self.available_data:
                    self.latest_values[var] = getattr(self, var)

                # Update history
                for key in self.available_data:
                    self.history[key].pop(0)
                    self.history[key].append(getattr(self, key))

                # Emit signal
                self.dataReceived.emit()