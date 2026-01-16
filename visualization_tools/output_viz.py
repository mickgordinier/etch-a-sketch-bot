import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, Button
import matplotlib.animation as animation

# -------- Read commands from a txt file --------
with open("output/output_cat.txt", "r") as f:
    commands = [line.strip() for line in f if line.strip() and line.strip() != "END OF INSTRUCTIONS"]

# Convert commands into vectors
move_map = {
    "UP": (0, -1),
    "DOWN": (0, 1),
    "LEFT": (-1, 0),
    "RIGHT": (1, 0)
}

# Interpolate steps into single-pixel movements
path = [(0, 0)]  # Start at origin
for cmd in commands:
    direction, distance = cmd.split()
    distance = int(distance)
    dx, dy = move_map[direction]
    last_x, last_y = path[-1]
    for _ in range(distance):
        last_x += dx
        last_y += dy
        path.append((last_x, last_y))

# Extract X and Y coordinates
xs, ys = zip(*path)

fig, ax = plt.subplots()
plt.subplots_adjust(bottom=0.35)

# Line showing the full drawn path
line, = ax.plot([], [], color='blue', lw=1)

# Marker for current position
current_point, = ax.plot([], [], 'ro', markersize=6)

ax.set_xlim(min(xs)-10, max(xs)+10)
ax.set_ylim(max(ys)+10, min(ys)-10)  # Flip Y-axis
ax.set_aspect('equal')
ax.set_title("Step-by-step Drawer Visualization")

# Slider to scroll through steps manually
ax_slider = plt.axes([0.25, 0.2, 0.5, 0.03])
slider = Slider(ax_slider, 'Step', 0, len(xs)-1, valinit=0, valstep=1)

# Speed slider (1 = slow, larger = faster)
ax_speed = plt.axes([0.25, 0.12, 0.5, 0.03])
speed_slider = Slider(ax_speed, 'Speed', 1, 300, valinit=100, valstep=1)

# Play/Pause button
ax_button = plt.axes([0.45, 0.05, 0.1, 0.04])
button = Button(ax_button, 'Play')

is_playing = False  # Global flag

def update(val):
    step = int(slider.val)
    line.set_data(xs[:step+1], ys[:step+1])
    current_point.set_data([xs[step]], [ys[step]])
    fig.canvas.draw_idle()

slider.on_changed(update)

# Animation function
def animate(frame):
    global is_playing
    if is_playing:
        current = int(slider.val)
        # Move forward by speed pixels per frame
        speed = int(speed_slider.val)
        next_step = min(current + speed, len(xs) - 1)
        slider.set_val(next_step)
        if next_step == len(xs) - 1:
            is_playing = False

def toggle_play(event):
    global is_playing
    is_playing = not is_playing
    button.label.set_text("Pause" if is_playing else "Play")

button.on_clicked(toggle_play)

# FuncAnimation calls animate repeatedly
ani = animation.FuncAnimation(fig, animate, interval=20)

plt.show()
