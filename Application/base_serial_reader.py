##############################
# Base Serial Reader
##############################
class BaseSerialReader:
    def __init__(self):
        # List of all variables sent by the LoRa packet.
        # Note: distance_travelled will be used as a continuously updated number,
        # not a graph.
        self.available_data = [
            "velocity",
            "distance_travelled",
            "battery_volt",
            "battery_current",
            "battery_cell_LOW_volt",
            "battery_cell_HIGH_volt",
            "battery_cell_AVG_volt",
            "battery_cell_LOW_temp",
            "battery_cell_HIGH_temp",
            "battery_cell_AVG_temp",
            "battery_cell_ID_HIGH_temp",
            "battery_cell_ID_LOW_temp",
            "BMS_temp",
            "motor_current",
            "motor_temp",
            "motor_controller_temp",
            "MPPT1_watt",
            "MPPT2_watt",
            "MPPT3_watt",
            "MPPT_total_watt",
            "rssi"
        ]
        # Build a history for each variable (100 samples), initially all zeros.
        self.history = { key: [0.0] * 100 for key in self.available_data }
        self.latest_values = { key: 0.0 for key in self.available_data }
        self.running = False
        self.selected_port = None

    def start(self):
        pass

    def stop(self):
        pass

    def set_port(self, port):
        self.selected_port = port
        print(f"[DEBUG] COM Port set to: {port}")
