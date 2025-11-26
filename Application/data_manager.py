from dataclasses import dataclass
from typing import Dict, List, Optional

@dataclass
class DataField:
    """Represents a single data field with its properties."""
    key: str
    display_name: str
    unit: str
    category: str
    description: str = ""

class SerialDataManager:
    """Manages and organizes serial data fields for easier access."""
    
    # Define all data fields with their properties
    FIELDS = {
        # Vehicle data
        "velocity": DataField("velocity", "Velocity", "km/h", "vehicle", "Current vehicle speed"),
        "distance_travelled": DataField("distance_travelled", "Distance Travelled", "km", "vehicle", "Total distance covered"),
        
        # Battery data
        "battery_volt": DataField("battery_volt", "Battery Voltage", "V", "battery", "Total battery pack voltage"),
        "battery_current": DataField("battery_current", "Battery Current", "A", "battery", "Battery current (positive = charging)"),
        "battery_cell_LOW_volt": DataField("battery_cell_LOW_volt", "Lowest Cell Voltage", "V", "battery", "Lowest individual cell voltage"),
        "battery_cell_HIGH_volt": DataField("battery_cell_HIGH_volt", "Highest Cell Voltage", "V", "battery", "Highest individual cell voltage"),
        "battery_cell_AVG_volt": DataField("battery_cell_AVG_volt", "Average Cell Voltage", "V", "battery", "Average cell voltage"),
        "battery_cell_LOW_temp": DataField("battery_cell_LOW_temp", "Lowest Cell Temperature", "°C", "battery", "Lowest cell temperature"),
        "battery_cell_HIGH_temp": DataField("battery_cell_HIGH_temp", "Highest Cell Temperature", "°C", "battery", "Highest cell temperature"),
        "battery_cell_AVG_temp": DataField("battery_cell_AVG_temp", "Average Cell Temperature", "°C", "battery", "Average cell temperature"),
        "battery_cell_ID_HIGH_temp": DataField("battery_cell_ID_HIGH_temp", "High Temp Cell ID", "", "battery", "Cell ID with highest temperature"),
        "battery_cell_ID_LOW_temp": DataField("battery_cell_ID_LOW_temp", "Low Temp Cell ID", "", "battery", "Cell ID with lowest temperature"),
        "BMS_temp": DataField("BMS_temp", "BMS Temperature", "°C", "battery", "Battery Management System temperature"),
        
        # Motor data
        "motor_current": DataField("motor_current", "Motor Current", "A", "motor", "Motor current consumption"),
        "motor_temp": DataField("motor_temp", "Motor Temperature", "°C", "motor", "Motor temperature"),
        "motor_controller_temp": DataField("motor_controller_temp", "Motor Controller Temperature", "°C", "motor", "Motor controller temperature"),
        
        # MPPT data
        "MPPT1_watt": DataField("MPPT1_watt", "MPPT 1 Power", "W", "mppt", "MPPT 1 power output"),
        "MPPT2_watt": DataField("MPPT2_watt", "MPPT 2 Power", "W", "mppt", "MPPT 2 power output"),
        "MPPT3_watt": DataField("MPPT3_watt", "MPPT 3 Power", "W", "mppt", "MPPT 3 power output"),
        "MPPT_total_watt": DataField("MPPT_total_watt", "Total MPPT Power", "W", "mppt", "Total MPPT power output"),
        
        # Communication data
        "rssi": DataField("rssi", "Signal Strength", "dBm", "communication", "LoRa signal strength"),
    }
    
    @classmethod
    def get_field(cls, key: str) -> Optional[DataField]:
        """Get a data field by its key."""
        return cls.FIELDS.get(key)
    
    @classmethod
    def get_fields_by_category(cls, category: str) -> List[DataField]:
        """Get all fields in a specific category."""
        return [field for field in cls.FIELDS.values() if field.category == category]
    
    @classmethod
    def get_all_keys(cls) -> List[str]:
        """Get all field keys."""
        return list(cls.FIELDS.keys())
    
    @classmethod
    def get_display_name(cls, key: str) -> str:
        """Get display name for a field key."""
        field = cls.get_field(key)
        return field.display_name if field else key
    
    @classmethod
    def get_unit(cls, key: str) -> str:
        """Get unit for a field key."""
        field = cls.get_field(key)
        return field.unit if field else ""
    
    @classmethod
    def get_category(cls, key: str) -> str:
        """Get category for a field key."""
        field = cls.get_field(key)
        return field.category if field else "unknown"
    
    @classmethod
    def get_max_expected_value(cls, key: str) -> float:
        """Get maximum expected value for a field key."""
        max_values = {
            # Vehicle data
            "velocity": 100.0,  # km/h
            "distance_travelled": 1000.0,  # km
            
            # Battery data
            "battery_volt": 150.0,  # V (140V + safety margin)
            "battery_current": 100.0,  # A
            "battery_cell_LOW_volt": 4.0,  # V per cell
            "battery_cell_HIGH_volt": 4.0,  # V per cell
            "battery_cell_AVG_volt": 4.0,  # V per cell
            "battery_cell_LOW_temp": 60.0,  # °C
            "battery_cell_HIGH_temp": 60.0,  # °C
            "battery_cell_AVG_temp": 60.0,  # °C
            "battery_cell_ID_HIGH_temp": 40.0,  # Cell ID (0-38)
            "battery_cell_ID_LOW_temp": 40.0,  # Cell ID (0-38)
            "BMS_temp": 80.0,  # °C
            
            # Motor data
            "motor_current": 200.0,  # A
            "motor_temp": 120.0,  # °C
            "motor_controller_temp": 100.0,  # °C
            
            # MPPT data
            "MPPT1_watt": 1000.0,  # W
            "MPPT2_watt": 1000.0,  # W
            "MPPT3_watt": 1000.0,  # W
            "MPPT_total_watt": 3000.0,  # W
        }
        return max_values.get(key, 100.0)  # Default to 100 if not specified
