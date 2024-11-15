# Script to parse sensor data to detect impacts
import csv
import matplotlib.pyplot as plt
from collections import Counter
from math import sqrt

# File path to the CSV file
# file_path = 'test_sensor_data.csv'
file_path = 'test_sensor_data_1.csv'

ticks = []
accel_data = []
gyro_data = []
mag_data = []

# Read the file and populate the lists
with open(file_path, 'r') as file:
    reader = csv.reader(file)
    for row in reader:
        # Convert each row element to an integer
        row = list(map(float, row))
        
        # Separate the data
        ticks.append(row[0])                       # Time in ticks
        accel_data.append(row[1:4])                # Accelerometer data x, y, z
        gyro_data.append(row[4:7])                 # Gyroscope data x, y, z
        mag_data.append(row[7:10])                 # Magnetometer data x, y, z

# Display a sample of the data
# print("Ticks:", ticks)
# print("Accelerometer data:", accel_data)
# print("Gyroscope data:", gyro_data[:5])
# print("Magnetometer data:", mag_data[:5])

# Estimate time
MS_PER_TICK = 10
S_PER_TICK = MS_PER_TICK/1000   
# print(S_PER_TICK)

samples_per_tick = Counter(ticks)
average_num_samples = sum(samples_per_tick.values()) / len(samples_per_tick)

print("Average Number of Samples:")
print("    ", round(average_num_samples, 1), "per", MS_PER_TICK, "ms")
print("    ", round(average_num_samples/S_PER_TICK, 1), "per sec")

est_sample_time = 0.0
time_sec = []
for tick, num_samples in samples_per_tick.items():

    # estimae time step
    est_time_step = S_PER_TICK/num_samples

    # update time list
    for iter in range(0, num_samples):
        time_sec.append(round(est_sample_time, 5))
        est_sample_time += est_time_step

# print(time_sec)

# calculate magnitude of acceleration
accel_mag = [sqrt(val[0]*val[0] + val[1]*val[1] + val[2]*val[2]) for val in accel_data]

# graph values - should be able to visually identify impacts
plt.plot(time_sec, accel_mag, marker='o', linestyle='-')
plt.xlabel('Time (sec)')
plt.ylabel('Acceleration (g)')
plt.title('Magnitude of Acceleration over Time')
plt.grid(True)
plt.show()

# Impact detection
