import serial
import time

# Match your Arduino's port and baud rate
ser = serial.Serial('COM3', 115200, timeout=5)  # Windows: COM3, Mac/Linux: /dev/ttyUSB0
time.sleep(2)  # Wait for Arduino to reset after connection

with open('output/maggie-baseball/binary_steps.bin', 'rb') as f:
    data = f.read()

total = len(data)

for i, byte in enumerate(data):
    ser.write(bytes([byte]))                          # Send one byte
    line = ser.readline().decode('utf-8', errors='replace').strip()     # Wait for Arduino response

    if not line:
        print(f"[{i+1}/{total}] WARNING: timeout on byte 0x{byte:02X}")
    else:
        print(f"[{i+1}/{total}] sent 0x{byte:02X} | Arduino: {line}")

print("Done")
ser.close()