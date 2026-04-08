import math

SAMPLES = 1024
SAMPLING_FREQUENCY = 2000
TARGET_FREQ = 100

def generate_data():
    data = []
    for i in range(SAMPLES):
        t = i / SAMPLING_FREQUENCY
        # Simulating the same signal from the original C++ code
        val = 50.0 * math.sin(2.0 * math.pi * TARGET_FREQ * t) + 20.0 * math.sin(2.0 * math.pi * (0.5 * TARGET_FREQ) * t)
        data.append(f"{val:.6f}")
    
    with open("buffer_data.txt", "w") as f:
        f.write("\n".join(data))
    
    print("Data written to buffer_data.txt.")
    print("Upload this file to GitHub, then click 'Raw' on GitHub and copy the raw URL.")

if __name__ == "__main__":
    generate_data()
