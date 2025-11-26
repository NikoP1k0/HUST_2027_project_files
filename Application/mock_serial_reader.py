import random
import math
from PyQt5.QtCore import pyqtSignal, QObject
from base_serial_reader import BaseSerialReader

##############################
# Mock Serial Reader (for testing)
##############################
class MockSerialReader(QObject, BaseSerialReader):
    dataReceived = pyqtSignal()

    def __init__(self):
        QObject.__init__(self)
        BaseSerialReader.__init__(self)
        self.i = 0  # a counter to simulate changing data
        self.driving_phase = "startup"  # startup, accelerating, cruising, decelerating
        self.target_speed = 0.0
        self.acceleration_phase = 0  # 0-100 for smooth acceleration
        
        # Initialize variables with realistic car startup values
        self.velocity = 0.0
        self.distance_travelled = 0.0
        self.battery_volt = 140.0  # Maximum battery pack voltage
        self.battery_current = 0.0  # No current draw at startup
        self.battery_cell_LOW_volt = 3.7  # Typical Li-ion cell voltage
        self.battery_cell_HIGH_volt = 3.7
        self.battery_cell_AVG_volt = 3.7
        self.battery_cell_LOW_temp = 20.0  # Ambient temperature
        self.battery_cell_HIGH_temp = 20.0
        self.battery_cell_AVG_temp = 20.0
        self.battery_cell_ID_HIGH_temp = 1  # Cell ID numbers
        self.battery_cell_ID_LOW_temp = 1
        self.BMS_temp = 20.0
        self.motor_current = 0.0  # No motor current at startup
        self.motor_temp = 20.0  # Ambient temperature
        self.motor_controller_temp = 20.0
        self.MPPT1_watt = 0.0  # No solar power at startup
        self.MPPT2_watt = 0.0
        self.MPPT3_watt = 0.0
        self.MPPT_total_watt = 0.0
        self.rssi = -70.0  # Signal strength in dBm

        self.latest_values = {
            "velocity": self.velocity,
            "distance_travelled": self.distance_travelled,
            "battery_volt": self.battery_volt,
            "battery_current": self.battery_current,
            "battery_cell_LOW_volt": self.battery_cell_LOW_volt,
            "battery_cell_HIGH_volt": self.battery_cell_HIGH_volt,
            "battery_cell_AVG_volt": self.battery_cell_AVG_volt,
            "battery_cell_LOW_temp": self.battery_cell_LOW_temp,
            "battery_cell_HIGH_temp": self.battery_cell_HIGH_temp,
            "battery_cell_AVG_temp": self.battery_cell_AVG_temp,
            "battery_cell_ID_HIGH_temp": self.battery_cell_ID_HIGH_temp,
            "battery_cell_ID_LOW_temp": self.battery_cell_ID_LOW_temp,
            "BMS_temp": self.BMS_temp,
            "motor_current": self.motor_current,
            "motor_temp": self.motor_temp,
            "motor_controller_temp": self.motor_controller_temp,
            "MPPT1_watt": self.MPPT1_watt,
            "MPPT2_watt": self.MPPT2_watt,
            "MPPT3_watt": self.MPPT3_watt,
            "MPPT_total_watt": self.MPPT_total_watt,
            "rssi": self.rssi
        }
        # Populate the history with the initial values.
        for key, value in self.latest_values.items():
            self.history[key].pop(0)
            self.history[key].append(value)

    def start(self):
        # No action needed for the mock.
        pass

    def stop(self):
        pass

    def update(self):
        # Simulate realistic car driving phases
        self.i += 1
        
        # Phase transitions based on time
        if self.i < 5:  # First 5 seconds: startup phase
            self.driving_phase = "startup"
            self.target_speed = 0.0
        elif self.i < 60:  # Next 55 seconds: acceleration phase
            self.driving_phase = "accelerating"
            self.target_speed = min(80.0, (self.i - 5) * 1.5)  # Accelerate to 80 km/h
        elif self.i < 180:  # Next 2 minutes: cruising phase
            self.driving_phase = "cruising"
            self.target_speed = 80.0 + random.uniform(-2, 2)  # Maintain ~80 km/h with small variations
        else:  # After 3 minutes: deceleration phase
            self.driving_phase = "decelerating"
            self.target_speed = max(0.0, 80.0 - (self.i - 180) * 0.4)  # Decelerate to 0
        
        # Smooth velocity changes
        velocity_diff = self.target_speed - self.velocity
        if abs(velocity_diff) > 0.1:
            self.velocity += velocity_diff * 0.1  # Smooth acceleration/deceleration
        else:
            self.velocity = self.target_speed
        
        # Update distance travelled (convert km/h to km/s and accumulate)
        self.distance_travelled += (self.velocity / 3600.0)  # km/h to km/s
        
        # Battery simulation - realistic EV behavior
        if self.driving_phase == "startup":
            # Battery voltage stable during startup
            self.battery_volt = 140.0 + random.uniform(-0.5, 0.5)
            self.battery_current = random.uniform(-0.5, 0.5)  # Small parasitic drain
        elif self.driving_phase == "accelerating":
            # High current draw during acceleration
            self.battery_current = 50.0 + random.uniform(-5, 5)
            self.battery_volt = 140.0 - (self.velocity / 80.0) * 15.0 + random.uniform(-1, 1)
        elif self.driving_phase == "cruising":
            # Moderate current draw during cruising
            self.battery_current = 25.0 + random.uniform(-3, 3)
            self.battery_volt = 125.0 + random.uniform(-2, 2)
        else:  # decelerating
            # Regenerative braking - negative current
            self.battery_current = -10.0 + random.uniform(-2, 2)
            self.battery_volt = 140.0 + random.uniform(-1, 1)
        
        # Individual cell voltages (slight variations)
        # For 140V system: 140V / 3.7V per cell = ~38 cells in series
        base_cell_volt = self.battery_volt / 38  # 38 cells in series for 140V system
        self.battery_cell_LOW_volt = base_cell_volt - random.uniform(0.05, 0.15)
        self.battery_cell_HIGH_volt = base_cell_volt + random.uniform(0.05, 0.15)
        self.battery_cell_AVG_volt = (self.battery_cell_LOW_volt + self.battery_cell_HIGH_volt) / 2
        
        # Temperature simulation - increases with current draw and time
        temp_increase = abs(self.battery_current) * 0.2 + (self.i / 1000.0)
        self.battery_cell_LOW_temp = 20.0 + temp_increase + random.uniform(-1, 1)
        self.battery_cell_HIGH_temp = 20.0 + temp_increase + random.uniform(2, 5)
        self.battery_cell_AVG_temp = (self.battery_cell_LOW_temp + self.battery_cell_HIGH_temp) / 2
        
        # Cell ID tracking (simulate which cells are hottest/coldest)
        self.battery_cell_ID_HIGH_temp = random.randint(1, 38)
        self.battery_cell_ID_LOW_temp = random.randint(1, 38)
        
        # BMS temperature (follows battery temp)
        self.BMS_temp = self.battery_cell_AVG_temp + random.uniform(-2, 2)
        
        # Motor simulation
        if self.velocity > 0:
            # Motor current proportional to power demand
            power_demand = self.velocity * 0.5 + abs(self.battery_current) * 0.1
            self.motor_current = power_demand + random.uniform(-2, 2)
            
            # Motor temperature increases with current and time
            motor_temp_increase = abs(self.motor_current) * 0.3 + (self.i / 2000.0)
            self.motor_temp = 20.0 + motor_temp_increase + random.uniform(-1, 1)
            self.motor_controller_temp = self.motor_temp + random.uniform(-2, 2)
        else:
            # Motor off when stationary
            self.motor_current = 0.0
            self.motor_temp = 20.0 + random.uniform(-1, 1)
            self.motor_controller_temp = 20.0 + random.uniform(-1, 1)
        
        # MPPT simulation - solar panels (simulate day/night cycle)
        if 10 < self.i < 120:  # Simulate sunlight during middle of simulation
            sun_intensity = math.sin((self.i - 10) * math.pi / 110)  # 0 to 1
            self.MPPT1_watt = sun_intensity * random.uniform(200, 300)
            self.MPPT2_watt = sun_intensity * random.uniform(150, 250)
            self.MPPT3_watt = sun_intensity * random.uniform(180, 280)
        else:
            # No solar power at night/startup
            self.MPPT1_watt = random.uniform(0, 10)
            self.MPPT2_watt = random.uniform(0, 10)
            self.MPPT3_watt = random.uniform(0, 10)
        
        self.MPPT_total_watt = self.MPPT1_watt + self.MPPT2_watt + self.MPPT3_watt

        # Simulate RSSI signal strength (varies with distance and obstacles)
        # Base RSSI around -70 dBm with some variation
        base_rssi = -70.0
        # Add some variation based on time and driving phase
        if self.driving_phase == "startup":
            rssi_variation = random.uniform(-5, 5)  # Stable signal during startup
        elif self.driving_phase == "accelerating":
            rssi_variation = random.uniform(-10, 0)  # Slightly worse during acceleration
        elif self.driving_phase == "cruising":
            rssi_variation = random.uniform(-15, 5)  # More variation during cruising
        else:  # decelerating
            rssi_variation = random.uniform(-8, 2)  # Improving signal when slowing down
        
        self.rssi = base_rssi + rssi_variation

        self.latest_values = {
            "velocity": self.velocity,
            "distance_travelled": self.distance_travelled,
            "battery_volt": self.battery_volt,
            "battery_current": self.battery_current,
            "battery_cell_LOW_volt": self.battery_cell_LOW_volt,
            "battery_cell_HIGH_volt": self.battery_cell_HIGH_volt,
            "battery_cell_AVG_volt": self.battery_cell_AVG_volt,
            "battery_cell_LOW_temp": self.battery_cell_LOW_temp,
            "battery_cell_HIGH_temp": self.battery_cell_HIGH_temp,
            "battery_cell_AVG_temp": self.battery_cell_AVG_temp,
            "battery_cell_ID_HIGH_temp": self.battery_cell_ID_HIGH_temp,
            "battery_cell_ID_LOW_temp": self.battery_cell_ID_LOW_temp,
            "BMS_temp": self.BMS_temp,
            "motor_current": self.motor_current,
            "motor_temp": self.motor_temp,
            "motor_controller_temp": self.motor_controller_temp,
            "MPPT1_watt": self.MPPT1_watt,
            "MPPT2_watt": self.MPPT2_watt,
            "MPPT3_watt": self.MPPT3_watt,
            "MPPT_total_watt": self.MPPT_total_watt,
            "rssi": self.rssi
        }
        # Update the history arrays for each variable.
        for key, value in self.latest_values.items():
            self.history[key].pop(0)
            self.history[key].append(value)
