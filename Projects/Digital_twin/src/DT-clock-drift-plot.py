import serial
import time
import argparse
import threading
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from sklearn.linear_model import LinearRegression

class ClockDigitalTwin:
    def __init__(self, window_size=50):
        self.esp_raw_micros = []
        self.pc_ref_seconds = []
        self.model = LinearRegression()
        self.window_size = window_size
        
        # Arrays for the real-time plots
        self.plot_time = []
        self.plot_pc_time = []
        self.plot_esp_time = []
        self.plot_error = []
        self.plot_mult = []
        self.start_time = None

    def add_sample(self, esp_micros):
        now_pc = time.time()
        if self.start_time is None:
            self.start_time = now_pc
            
        self.esp_raw_micros.append([esp_micros])
        self.pc_ref_seconds.append(now_pc)
        
        if len(self.esp_raw_micros) > self.window_size:
            self.esp_raw_micros.pop(0)
            self.pc_ref_seconds.pop(0)

    def calculate_multiplier(self):
        if len(self.esp_raw_micros) < 5:
            return None
        self.model.fit(self.esp_raw_micros, self.pc_ref_seconds)
        multiplier = self.model.coef_[0] * 1_000_000
        return multiplier

def serial_thread(args, twin):
    phase_synced = False
    try:
        with serial.Serial(args.port, args.baudrate, timeout=1) as ser:
            print(f"Connected to {args.port} at {args.baudrate} baud. Waiting for ESP32...")
            time.sleep(2) # Wait for ESP32 reboot
            ser.reset_input_buffer() # Clear old data
            
            while True:
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
                        
                        mult = twin.calculate_multiplier()
                        if mult:
                            ser.write(f"SET_MULT:{mult:.8f}\n".encode())
                            pc_now = time.time()
                            error = pc_now - esp_thinks_wall
                            
                            # Print directly to terminal (as requested)
                            print(f"[PC] Time: {pc_now:.3f} | [ESP] Thinks: {esp_thinks_wall:.3f} | Error: {error*1000:.2f}ms | Mult: {mult:.8f}")
                            
                            # Log data for the plot
                            t = pc_now - twin.start_time
                            twin.plot_time.append(t)
                            twin.plot_pc_time.append(pc_now)
                            twin.plot_esp_time.append(esp_thinks_wall)
                            twin.plot_error.append(error * 1000)
                            twin.plot_mult.append(mult)
                            
                            # Limit array size so graph doesn't get overloaded (show last 100 points)
                            if len(twin.plot_time) > 100:
                                twin.plot_time.pop(0)
                                twin.plot_pc_time.pop(0)
                                twin.plot_esp_time.pop(0)
                                twin.plot_error.pop(0)
                                twin.plot_mult.pop(0)
                                
                    except (IndexError, ValueError) as parse_err:
                        print(f"Warning: Failed to parse line '{line}' - {parse_err}")
    except serial.SerialException as e:
        print(f"Error: Could not open port {args.port} ({e})")
    except KeyboardInterrupt:
        pass

def main():
    parser = argparse.ArgumentParser(description="ESP32 Clock Drift Digital Twin - Live Plot Tracker")
    parser.add_argument('--port', type=str, default='/dev/ttyUSB0', help='Serial port for ESP32')
    parser.add_argument('--baudrate', type=int, default=115200, help='Baud rate')
    args = parser.parse_args()

    twin = ClockDigitalTwin(window_size=50)
    
    # We must run the continuous serial reading loop in a background thread
    # otherwise matplotlib will hang waiting for lines.
    t = threading.Thread(target=serial_thread, args=(args, twin), daemon=True)
    t.start()

    # Set up matplotlib figure
    print("Opening Live Dashboard. Close the plot window to gracefully terminate the script.")
    
    # Use dark theme for a more tech-oriented aspect if available
    try:
        plt.style.use('dark_background')
    except:
        pass

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))
    fig.canvas.manager.set_window_title('ESP32 Clock Drift Real-Time Digital Twin')
    
    def animate(i):
        if not twin.plot_time:
            return
        
        ax1.clear()
        ax2.clear()
        
        # Plot 1: Error in ms
        ax1.plot(twin.plot_time, twin.plot_error, color='#00ffcc', linewidth=2, marker='o', markersize=4, label='Drift Error')
        ax1.set_title('PC vs ESP32 Wall Clock Time Error (ms)', color='white')
        ax1.set_ylabel('Error (ms)')
        ax1.grid(color='#333333', linestyle='--', linewidth=1)
        ax1.legend(loc='upper right')
        
        # Customize ticks
        ax1.tick_params(axis='x', colors='gray')
        ax1.tick_params(axis='y', colors='white')
        
        # Plot 2: Multiplier Variation
        ax2.plot(twin.plot_time, twin.plot_mult, color='#ff00ff', linewidth=2, marker='s', markersize=4, label='Drift Multiplier')
        ax2.set_title('Dynamic Drift Multiplier Tracking', color='white')
        ax2.set_xlabel('Elapsed Time (s)')
        ax2.set_ylabel('Multiplier')
        ax2.grid(color='#333333', linestyle='--', linewidth=1)
        ax2.legend(loc='upper right')
        
        # Customize ticks
        ax2.tick_params(axis='x', colors='gray')
        ax2.tick_params(axis='y', colors='white')
        
        plt.tight_layout()

    # Refresh the plot every 1000ms
    ani = animation.FuncAnimation(fig, animate, interval=1000, cache_frame_data=False)
    plt.show() # Blocks until window is closed

if __name__ == "__main__":
    main()
