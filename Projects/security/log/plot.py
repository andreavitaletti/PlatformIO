import numpy as np
import matplotlib.pyplot as plt

# Function to read a file line by line and generate a vector
def read_file_to_vector(file_path):
    with open(file_path, 'r') as file:
        vector = [float(line.strip()) for line in file]
    return np.array(vector)

# Example parameters
sampling_rate = 1000  # Hz
T = 1.0 / sampling_rate  # Sampling interval
t = np.linspace(0.0, 1.0, sampling_rate, endpoint=False)  # Time vector

# Example signal: sum of two sine waves
freq1 = 50  # Frequency of first sine wave (Hz)
freq2 = 120  # Frequency of second sine wave (Hz)
signal = np.sin(2 * np.pi * freq1 * t) + 0.5 * np.sin(2 * np.pi * freq2 * t)

signal = read_file_to_vector("test.txt")

# Compute FFT
n = len(t)  # Number of samples
y_fft = np.fft.fft(signal)  # FFT computation
y_fft_magnitude = 2.0 / n * np.abs(y_fft[:n // 2])  # Magnitude of FFT
frequencies = np.fft.fftfreq(n, T)[:n // 2]  # Corresponding frequencies

# Plot the signal and its FFT
plt.figure(figsize=(12, 6))

# Time-domain signal
plt.subplot(2, 1, 1)
plt.plot(t, signal, label="Signal")
plt.title("Time-Domain Signal")
plt.xlabel("Time [s]")
plt.ylabel("Amplitude")
plt.legend()

# Frequency-domain signal (FFT)
plt.subplot(2, 1, 2)
plt.plot(frequencies, y_fft_magnitude, label="FFT Magnitude")
plt.title("Frequency-Domain Signal")
plt.xlabel("Frequency [Hz]")
plt.ylabel("Magnitude")
plt.legend()

plt.tight_layout()
plt.show()
