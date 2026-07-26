#!/usr/bin/env python3
"""Live plot of the MPU6050 CSV stream from the ESP32.

Expects one sample per line:
    accelX,accelY,accelZ,gyroX,gyroY,gyroZ,accelAngle,gyroAngle
with accel in g, gyro in deg/s, angles in degrees.

Usage:
    python plot_mpu.py                    # uses DEFAULT_PORT
    python plot_mpu.py /dev/cu.usbserial-0001
    python -m serial.tools.list_ports     # to find your port
"""

import sys
from collections import deque

import matplotlib.pyplot as plt
import serial
from matplotlib.animation import FuncAnimation

DEFAULT_PORT = "/dev/cu.usbserial-0001"
BAUD = 115200
WINDOW = 300  # samples held on screen
CHANNELS = 8

ACCEL_RANGE = 2.0  # g, matches ACCEL_CONFIG default
GYRO_RANGE = 250.0  # deg/s, matches GYRO_CONFIG default
ANGLE_RANGE = 90.0  # degrees on screen; widen if the gyro drifts past it

port = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_PORT
ser = serial.Serial(port, BAUD, timeout=1)

buffers = [deque([0.0] * WINDOW, maxlen=WINDOW) for _ in range(CHANNELS)]

fig, (ax_accel, ax_gyro, ax_angle) = plt.subplots(3, 1, figsize=(10, 9), sharex=True)
fig.canvas.manager.set_window_title(f"MPU6050 on {port}")

accel_lines = [
    ax_accel.plot([], [], label=n)[0] for n in ("accel X", "accel Y", "accel Z")
]
gyro_lines = [ax_gyro.plot([], [], label=n)[0] for n in ("gyro X", "gyro Y", "gyro Z")]
angle_lines = [
    ax_angle.plot([], [], label="accel angle", color="tab:purple", linewidth=1.0)[0],
    ax_angle.plot([], [], label="gyro angle", color="tab:orange", linewidth=1.4)[0],
]

ax_accel.set_ylim(-ACCEL_RANGE, ACCEL_RANGE)
ax_accel.set_ylabel("g")
ax_accel.legend(loc="upper right", ncol=3)
ax_accel.grid(alpha=0.3)

ax_gyro.set_ylim(-GYRO_RANGE, GYRO_RANGE)
ax_gyro.set_ylabel("deg/s")
ax_gyro.legend(loc="upper right", ncol=3)
ax_gyro.grid(alpha=0.3)

ax_angle.set_ylim(-ANGLE_RANGE, ANGLE_RANGE)
ax_angle.set_ylabel("degrees")
ax_angle.set_xlabel("samples")
ax_angle.legend(loc="upper right", ncol=2)
ax_angle.grid(alpha=0.3)

for ax in (ax_accel, ax_gyro, ax_angle):
    ax.set_xlim(0, WINDOW)
    ax.axhline(0, color="black", linewidth=0.6, alpha=0.4)


def drain_serial():
    """Consume every pending line so the plot never lags behind the port."""
    while ser.in_waiting:
        raw = ser.readline().decode("utf-8", errors="ignore").strip()
        if not raw:
            continue
        parts = raw.split(",")
        if len(parts) != CHANNELS:
            continue  # boot messages, partial lines
        try:
            values = [float(p) for p in parts]
        except ValueError:
            continue  # garbage on reset
        for buf, value in zip(buffers, values):
            buf.append(value)


def update(_frame):
    drain_serial()
    x = range(WINDOW)
    for line, buf in zip(accel_lines + gyro_lines + angle_lines, buffers):
        line.set_data(x, buf)
    return accel_lines + gyro_lines + angle_lines


ani = FuncAnimation(fig, update, interval=30, blit=True, cache_frame_data=False)

try:
    plt.show()
finally:
    ser.close()
