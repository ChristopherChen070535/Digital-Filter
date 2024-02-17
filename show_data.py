import matplotlib.pyplot as plt
import numpy as np


def plot_impulse_response(filename):
    data = np.loadtxt(filename)
    n = len(data)

    # Use sample index as the x-axis
    sample_index = np.arange(n)

    # Plot impulse response using stem without markers
    plt.figure()
    plt.stem(sample_index, data, linefmt='b-', markerfmt='', basefmt='r-')
    plt.title(filename)  # Set the title to the filename
    plt.xlabel('Sample Index')
    plt.ylabel('Amplitude')
    plt.grid(True)
    
    # Save the plot as a PNG file in the current working directory
    plt.savefig(filename.replace('.txt', '_show.png'))  # Save with the filename as the filename (replace '.txt' with '_stem.png')
    plt.show()

# Define the filenames
hL_filenames = ['hL1024.txt', 'hL32.txt', 'hL8.txt']
hR_filenames = ['hR1024.txt', 'hR32.txt', 'hR8.txt']

# Iterate through the filenames and plot impulse responses with stems
for filename in hL_filenames:
    plot_impulse_response(filename)

for filename in hR_filenames:
    plot_impulse_response(filename)
print("Impulse response Data successfully plotted.")




# Function to read data from a text file
def read_spectrum_file(filename):
    with open(filename, 'r') as file:
        data = [float(line.strip()) for line in file]
    return np.array(data)

# Function to plot log spectrum
def plot_log_spectrum(freq_values, data, title, xlabel, ylabel):
    plt.plot(freq_values, data[:n_fft//2])
    plt.title(title)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)

# File paths for YL.txt and YR.txt
yl_filenames = ['YL1024.txt', 'YL32.txt', 'YL8.txt']
yr_filenames = ['YR1024.txt', 'YR32.txt', 'YR8.txt']

# Frequency values for the x-axis
fs = 48000.0  # Assuming the same sample rate as in the C code
n_fft = 1200
freq_values = np.fft.fftfreq(n_fft, d=1/fs)[:n_fft//2]

# Plotting the log spectrum for each file
for i in range(len(yl_filenames)):
    yl_filename = yl_filenames[i]
    yr_filename = yr_filenames[i]

    # Read data from the files
    yl_data = read_spectrum_file(yl_filename)
    yr_data = read_spectrum_file(yr_filename)

    # Plotting the log spectrum
    plt.figure(figsize=(10, 6))

    # Left Channel
    plt.subplot(2, 1, 1)
    plot_log_spectrum(freq_values, yl_data, f'Left Channel Log Spectrum ({yl_filename})', 'Frequency (Hz)', 'Magnitude (dB)')

    # Right Channel
    plt.subplot(2, 1, 2)
    plot_log_spectrum(freq_values, yr_data, f'Right Channel Log Spectrum ({yr_filename})', 'Frequency (Hz)', 'Magnitude (dB)')

    plt.tight_layout()
    
    # Save the figure as a PNG file
    plt.savefig(f'spectrum_plot_{i+1}.png')
    plt.show()

print("Log Spectrum Data successfully plotted.")