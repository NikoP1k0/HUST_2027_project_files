import pymysql
import pymysql.cursors

# ==========================
# Database Credentials
# ==========================
DB_HOST = "194.47.13.187"
DB_USER = "remoteuser"
DB_PASSWORD = "mhsRuS84s6K6baslP9G7LGH"
DB_NAME = "Hust"

# ==========================
# Database Connection Function
# ==========================
def connect_db():
    return pymysql.connect(
        host=DB_HOST,
        user=DB_USER,
        password=DB_PASSWORD,
        database=DB_NAME,
        cursorclass=pymysql.cursors.DictCursor
    )

# ==========================
# Insert Data into MySQL
# ==========================
def insert_into_db(latest_values):
    """Inserts the latest values into the corresponding database tables."""
    conn = connect_db()
    try:
        with conn.cursor() as cursor:
            # Battery Data Table
            cursor.execute("""
                INSERT INTO `Battery Data Table`
                (timestamp, battery_volt, battery_current, battery_cell_low_volt, battery_cell_high_volt, battery_cell_average_volt,
                 battery_cell_low_temp, battery_cell_high_temp, battery_cell_average_temp)
                VALUES (NOW(), %s, %s, %s, %s, %s, %s, %s, %s)
            """, (
                latest_values["battery_volt"],
                latest_values["battery_current"],
                latest_values["battery_cell_LOW_volt"],
                latest_values["battery_cell_HIGH_volt"],
                latest_values["battery_cell_AVG_volt"],
                latest_values["battery_cell_LOW_temp"],
                latest_values["battery_cell_HIGH_temp"],
                latest_values["battery_cell_AVG_temp"]
            ))

            # Motor Data Table
            cursor.execute("""
                INSERT INTO `Motor Data Table`
                (timestamp, motor_current, motor_temp, motor_controller_temp)
                VALUES (NOW(), %s, %s, %s)
            """, (
                latest_values["motor_current"],
                latest_values["motor_temp"],
                latest_values["motor_controller_temp"]
            ))

            # MPPT Data Table
            cursor.execute("""
                INSERT INTO `MPPT Data Table`
                (timestamp, MPPT1_watt, MPPT2_watt, MPPT3_watt, MPPT_total_watt)
                VALUES (NOW(), %s, %s, %s, %s)
            """, (
                latest_values["MPPT1_watt"],
                latest_values["MPPT2_watt"],
                latest_values["MPPT3_watt"],
                latest_values["MPPT_total_watt"]
            ))

            # Vehicle Data Table
            cursor.execute("""
                INSERT INTO `Vehicle Data Table`
                (timestamp, velocity, distance_travelled)
                VALUES (NOW(), %s, %s)
            """, (
                latest_values["velocity"],
                latest_values["distance_travelled"]
            ))

        conn.commit()
    except pymysql.MySQLError as e:
        print("❌ MySQL Error:", e)
    finally:
        conn.close()