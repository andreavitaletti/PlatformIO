import serial
import time
import argparse
from sklearn.linear_model import LinearRegression

class ClockDigitalTwin:
    def __init__(self, window_size=50):
        self.esp_raw_micros = []
        self.pc_ref_seconds = []
        self.model = LinearRegression()
        self.window_size = window_size

    def add_sample(self, esp_micros):
        # Record the "Gold Standard" time from the PC
        now_pc = time.time()
        
        self.esp_raw_micros.append([esp_micros])
        self.pc_ref_seconds.append(now_pc)
        
        # Keep only the most recent samples to adapt to temperature changes
        if len(self.esp_raw_micros) > self.window_size:
            self.esp_raw_micros.pop(0)
            self.pc_ref_seconds.pop(0)

    def calculate_multiplier(self):
        if len(self.esp_raw_micros) < 5:
            return None # Not enough data yet
        
        # Fit model: PC_Time = (m * ESP_Micros) + c
        self.model.fit(self.esp_raw_micros, self.pc_ref_seconds)
        
        # The slope 'm' tells us how many PC seconds pass per 1 ESP32 microsecond
        # We multiply by 1,000,000 because the ESP32 works in microseconds
        multiplier = self.model.coef_[0] * 1_000_000
        return multiplier
def main():
    parser = argparse.ArgumentParser(description="ESP32 Clock Drift Digital Twin")
    parser.add_argument('--port', type=str, default='/dev/ttyUSB0', help='Serial port for ESP32')
    parser.add_argument('--baudrate', type=int, default=115200, help='Baud rate')
    args = parser.parse_args()

    twin = ClockDigitalTwin(window_size=50)
    phase_synced = False
    
    try:
        with serial.Serial(args.port, args.baudrate, timeout=1) as ser:
            print(f"Connected to {args.port} at {args.baudrate} baud. Waiting for ESP32...")
            time.sleep(2) # Wait for ESP32 reboot
            ser.reset_input_buffer() # Clear old data
            
            while True:
                # 1. INITIAL PHASE SYNC
                if not phase_synced:
                    now = time.time()
                    ser.write(f"SET_PHASE:{now:.3f}\n".encode())
                    phase_synced = True
                    print(f"Sent initial Phase (Wall Clock): {now:.3f}")

                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if not line:
                    continue

                if "ACK" in line:
                    print(f"ESP32: {line}")
                    
                if "RAW_MICROS" in line:
                    try:
                        # Parse: RAW_MICROS:12345,CUR_WALL:17000.123
                        parts = line.split(",")
                        raw_micros = int(parts[0].split(":")[1])
                        esp_thinks_wall = float(parts[1].split(":")[1])
                        
                        twin.add_sample(raw_micros)
                        
                        # 2. FREQUENCY ADJUSTMENT
                        mult = twin.calculate_multiplier()
                        if mult:
                            ser.write(f"SET_MULT:{mult:.8f}\n".encode())
                            error = time.time() - esp_thinks_wall
                            print(f"[PC] Time: {time.time():.3f} | [ESP] Thinks: {esp_thinks_wall:.3f} | Error: {error*1000:.2f}ms | Mult: {mult:.8f}")
                    except (IndexError, ValueError) as parse_err:
                        print(f"Warning: Failed to parse line '{line}' - {parse_err}")
    except serial.SerialException as e:
        print(f"Error: Could not open port {args.port}. Make sure the ESP32 is connected and the port is correct. ({e})")
    except KeyboardInterrupt:
        print("\\nExiting...")

if __name__ == "__main__":
    main()