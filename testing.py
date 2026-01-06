import matplotlib.pyplot as plt
import numpy as np

STEP = 0.01
END = 300

MAX_FREQ = 5
MIN_FREQ = 0.00

pixels = [0.25, 0.48, 0.99, 0.01, 0.00]

x = np.arange(0, END, STEP)

y = np.array([])

total_steps = END/STEP

pixel_length = total_steps/len(pixels)

for i in range(len(pixels)):
    start_idx = int(i*pixel_length)
    end_idx = int((i+1)*pixel_length)
    x_sub = x[start_idx:end_idx]

    pixel_freq = (pixels[i] * (MAX_FREQ - MIN_FREQ)) + MIN_FREQ
    pixel_amplitude = pixels[i]
    y = np.append(y, pixel_amplitude * np.sin(pixel_freq * x_sub))

# Create plot
plt.plot(x, y)
plt.title("Simple Line Plot")
plt.xlabel("X-axis")
plt.ylabel("Y-axis")
plt.xlim(left=0)
plt.show()
