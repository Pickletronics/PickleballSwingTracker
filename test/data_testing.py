# Script to parse sensor data to detect impacts
import csv
import matplotlib.pyplot as plt
from collections import Counter
from math import sqrt, pi
import numpy as np
import re

def process_log_to_csv(input_file, output_file):
    # Open the input log file and output CSV file
    with open(input_file, 'r') as infile, open(output_file, 'w', newline='') as csvfile:
        csv_writer = csv.writer(csvfile)  # Create a CSV writer object
        recording = False

        for line in infile:
            # Check for "Hold detected!" marker
            if "Hold detected!" in line:
                recording = not recording
                if not recording:  # Stop recording
                    break
                continue  # Skip writing the marker line itself

            # Write numeric data to the CSV file if recording
            if recording:
                # Match lines with comma-separated numeric data
                match = re.match(r"^[A-Za-z0-9]+(,-?\d+){9}$", line.strip())
                if match:
                    # Split the line into a list
                    data = line.strip().split(',')

                    # Convert the first value (hexadecimal) to integer
                    data[0] = str(int(data[0], 16))

                    # Split line into a list and write to CSV
                    csv_writer.writerow(data)



def on_key(event):
    # if esc pressed, quit
    if event.key == 'q':
        quit()
    # otherwise, close current plot
    plt.close()  # Close the plot

def plot_data(ticks, accel_vals, plot_title, y_range, units, impacts=[]):
    plt.figure()
    plt.plot(ticks, accel_vals, marker='o', linestyle='-')
    plt.xlabel('Time (ms)')
    plt.ylabel(units)
    plt.title(plot_title)
    if y_range:
        plt.ylim(y_range[0], y_range[1])  # Set y-axis limits to 16-bit integer range
    plt.grid(True)

    for time, accel in impacts:
        plt.scatter(time, accel, color="red", marker="*", s=200, label="Impact" if impacts[0] else "")

    # close graph on key press
    plt.gcf().canvas.mpl_connect('key_press_event', on_key)
    # Display the plot
    plt.show()



def calculate_samples_per_second(values):
    # Calculate differences between consecutive samples
    differences = [values[i + 1] - values[i] for i in range(len(values) - 1)]
    
    # Total time in milliseconds
    total_time_ms = sum(differences)
    
    # Convert total time to seconds
    total_time_s = total_time_ms / 1000
    
    # Samples per second
    samples_per_second = len(values) / total_time_s if total_time_s > 0 else 0
    
    return samples_per_second






def moving_average_filter(data, window_size):
    """
    Applies a moving average filter to the input data.

    Parameters:
    - data: List of numerical values (e.g., x-axis accelerometer readings)
    - window_size: The size of the moving average window

    Returns:
    - filtered_data: List of filtered values
    """
    if window_size < 1:
        raise ValueError("Window size must be at least 1.")
    if window_size > len(data):
        raise ValueError("Window size must not exceed the length of the data.")
    
    filtered_data = []
    cumulative_sum = 0
    for i in range(len(data)):
        cumulative_sum += data[i]
        if i >= window_size:
            cumulative_sum -= data[i - window_size]
        filtered_data.append(cumulative_sum / min(window_size, i + 1))
    return filtered_data

# Define the Kalman filter parameters
class KalmanFilter:
    def __init__(self, process_noise, measurement_noise, estimation_error):
        self.q = process_noise  # Process noise covariance
        self.r = measurement_noise  # Measurement noise covariance
        self.p = estimation_error  # Estimation error covariance
        self.k = 0  # Kalman gain
        self.x = 0  # Estimated value (initial guess)

    def update(self, measurement):
        # Prediction step
        self.p = self.p + self.q

        # Update step
        self.k = self.p / (self.p + self.r)
        self.x = self.x + self.k * (measurement - self.x)
        self.p = (1 - self.k) * self.p

        return self.x




def main():

    """
        ISOLATE DATA FROM PUTTY LOG
    """

    # Input and output file paths
    input_file = "putty.log"  # Replace with your log file name
    output_file = "data_dump.csv"

    # Run the processing function
    process_log_to_csv(input_file, output_file)
    print(f"Data written to {output_file}")

    ticks = []
    accel_data = []
    gyro_data = []
    mag_data = []

    # Read the file and populate the lists
    with open(output_file, 'r') as file:
        reader = csv.reader(file)
        for row in reader:
            # Convert each row element to an integer
            row = list(map(int, row))
            
            # Separate the data
            ticks.append(row[0])                       # Time in ticks
            accel_data.append(row[1:4])                # Accelerometer data x, y, z
            gyro_data.append(row[4:7])                 # Gyroscope data x, y, z
            mag_data.append(row[7:10])                 # Magnetometer data x, y, z


    """
        SAMPLES PER SECOND
    """

    samples_per_sec = calculate_samples_per_second(ticks)
    print(f"Samples per second: {samples_per_sec:.2f}")

    time_ms = [tick - ticks[0] for tick in ticks]





    """
        UNFILTERED ACCEL (g) AND GYRO (rad per sec)
    """

    # Scale accel data to be in g's
    ACCEL_RANGE = 8 # +/- 4g
    ACCEL_SCALE = ACCEL_RANGE / 65536

    accel_x = [val[0] * ACCEL_SCALE for val in accel_data]
    accel_y = [val[1] * ACCEL_SCALE for val in accel_data]
    accel_z = [val[2] * ACCEL_SCALE for val in accel_data]

    accel_mag = [sqrt(x**2 + y**2 + z**2) for x, y, z in zip(accel_x, accel_y, accel_z)]

    plot_data(time_ms, accel_x, "Accel X over Time", (-4,4), "g")
    plot_data(time_ms, accel_y, "Accel Y over Time", (-4,4), "g")
    plot_data(time_ms, accel_z, "Accel Z over Time", (-4,4), "g")

    # Scale gyro data to be in rad per second
    GYRO_SCALE = 0.01526  # ±500°/s 
    DEG_TO_RAD = pi / 180
    gyro_x = [val[0] * GYRO_SCALE * DEG_TO_RAD for val in gyro_data]
    gyro_y = [val[1] * GYRO_SCALE * DEG_TO_RAD for val in gyro_data]
    gyro_z = [val[2] * GYRO_SCALE * DEG_TO_RAD for val in gyro_data]

    plot_data(time_ms, gyro_x, "Gyro X over Time", (-10,10), "rad/s")
    plot_data(time_ms, gyro_y, "Gyro Y over Time", (-10,10), "rad/s")
    plot_data(time_ms, gyro_z, "Gyro Z over Time", (-10,10), "rad/s")





    """
        DETECT IMPACTS
    """
 
    IMPACT_THRESHOLD = 1.0  # Threshold for significant impact in m/s²/ms
    impacts = []

    # Lists to store the rate of change of acceleration
    rate_of_change_list = []

    # Calculate the rate of change
    peak_found = False
    for i in range(1, len(accel_mag)):
        delta = accel_mag[i] - accel_mag[i - 1]  # Change in acceleration magnitude
        delta_time = time_ms[i] - time_ms[i - 1]  # Time difference in ms
        rate_of_change = delta / delta_time  # Rate of change in m/s^2/ms
        rate_of_change_list.append(rate_of_change)

        # Check if the rate of change exceeds the threshold
        if rate_of_change > IMPACT_THRESHOLD:
            peak_found = True
        # check window - find peak of impact

        if peak_found:
            if accel_mag[i] > accel_mag[i - 1] and accel_mag[i] > accel_mag[i + 1]:
                impacts.append((time_ms[i], accel_mag[i]))  # Add the time and magnitude of the peak
                peak_found = False

    plot_data(time_ms[1:], rate_of_change_list, "Impacts", 0, "m/s²/ms")

    # plot magnitude of acceleration and impacts detected
    plot_data(time_ms, accel_mag, "Accel Mag over Time", (0,8), "g", impacts=impacts)





    """
        FILTER OUT GRAVITY
    """

    # apply moving average filer to accelerometer data - nullifies impact
    window_size = 10
    accel_x_mavg = moving_average_filter(accel_x, window_size)
    accel_y_mavg = moving_average_filter(accel_y, window_size)
    accel_z_mavg = moving_average_filter(accel_z, window_size)

    # Convert accel to meters per second
    accel_x_mps = np.array(accel_x_mavg) * 9.81
    accel_y_mps = np.array(accel_y_mavg) * 9.81
    accel_z_mps = np.array(accel_z_mavg) * 9.81

    plot_data(time_ms, accel_x_mps, "Filtered Accel X over Time (Moving Average)", 0, "m/s²")
    plot_data(time_ms, accel_y_mps, "Filtered Accel Y over Time (Moving Average)", 0, "m/s²")
    plot_data(time_ms, accel_z_mps, "Filtered Accel Z over Time (Moving Average)", 0, "m/s²")




    # Filter for gravity using a low-pass filter
    alpha = 0.9  # Smoothing factor
    gravity_x = np.zeros(len(accel_x_mps))
    gravity_y = np.zeros(len(accel_y_mps))
    gravity_z = np.zeros(len(accel_z_mps))
    for i in range(1, len(accel_x)):
        gravity_x[i] = alpha * gravity_x[i-1] + (1 - alpha) * accel_x_mps[i]
        gravity_y[i] = alpha * gravity_y[i-1] + (1 - alpha) * accel_y_mps[i]
        gravity_z[i] = alpha * gravity_z[i-1] + (1 - alpha) * accel_z_mps[i]

    gravity_mag = np.sqrt(gravity_x**2 + gravity_y**2 + gravity_z**2)

    # reconstructed gravity plot
    plot_data(time_ms, gravity_mag, "Filtered Gravity (Low Pass)", 0, "m/s²")

    # Subtract gravity to isolate user acceleration
    user_accel_x = accel_x_mps - gravity_x
    user_accel_y = accel_y_mps - gravity_y
    user_accel_z = accel_z_mps - gravity_z

    plot_data(time_ms, user_accel_x, "Filtered Accel X over Time (Moving Average + Low Pass)", 0, "m/s²")
    plot_data(time_ms, user_accel_y, "Filtered Accel Y over Time (Moving Average + Low Pass)", 0, "m/s²")
    plot_data(time_ms, user_accel_z, "Filtered Accel Z over Time (Moving Average + Low Pass)", 0, "m/s²")




    # Complementary filter to filter gravity and isolate user acceleration
    alpha = 0.98  # Complementary filter constant (higher value gives more weight to accelerometer)
    gravity_x = np.zeros(len(accel_x_mps))
    gravity_y = np.zeros(len(accel_y_mps))
    gravity_z = np.zeros(len(accel_z_mps))

    # Assume the initial gravity vector is aligned with the first accelerometer reading
    gravity_x[0] = accel_x_mps[0]
    gravity_y[0] = accel_y_mps[0]
    gravity_z[0] = accel_z_mps[0]

    # Process each sample using the complementary filter
    for i in range(1, len(accel_x_mps)):
        # Get time difference between samples (in seconds)
        dt = (time_ms[i] - time_ms[i-1]) / 1000.0  # Convert milliseconds to seconds
        
        # Assuming you have gyroscope data in rad/s (gyro_x, gyro_y, gyro_z)
        # Integrate gyroscope data to get change in angle (angular velocity)
        # This will update the gravity vector based on rotation
        gravity_x[i] = alpha * (gravity_x[i-1] + gyro_x[i-1] * dt) + (1 - alpha) * accel_x_mps[i]
        gravity_y[i] = alpha * (gravity_y[i-1] + gyro_y[i-1] * dt) + (1 - alpha) * accel_y_mps[i]
        gravity_z[i] = alpha * (gravity_z[i-1] + gyro_z[i-1] * dt) + (1 - alpha) * accel_z_mps[i]

    # Calculate the magnitude of the filtered gravity vector
    gravity_mag = np.sqrt(gravity_x**2 + gravity_y**2 + gravity_z**2)

    # Reconstructed gravity plot (after complementary filter)
    plot_data(time_ms, gravity_mag, "Filtered Gravity (Complementary Filter)", 0, "m/s²")

    # Subtract gravity to isolate user acceleration
    user_accel_x = accel_x_mps - gravity_x
    user_accel_y = accel_y_mps - gravity_y
    user_accel_z = accel_z_mps - gravity_z

    # Plot user acceleration (filtered)
    plot_data(time_ms, user_accel_x, "Filtered Accel X over Time (Moving Average + Complementary Filter)", 0, "m/s²")
    plot_data(time_ms, user_accel_y, "Filtered Accel Y over Time (Moving Average + Complementary Filter)", 0, "m/s²")
    plot_data(time_ms, user_accel_z, "Filtered Accel Z over Time (Moving Average + Complementary Filter)", 0, "m/s²")




    # Filter for gravity using Kalman filter
    kf_x = KalmanFilter(process_noise=0.01, measurement_noise=0.1, estimation_error=1)
    kf_y = KalmanFilter(process_noise=0.01, measurement_noise=0.1, estimation_error=1)
    kf_z = KalmanFilter(process_noise=0.01, measurement_noise=0.1, estimation_error=1)

    # Filter gravity using Kalman filters
    gravity_x = np.zeros(len(accel_x_mps))
    gravity_y = np.zeros(len(accel_y_mps))
    gravity_z = np.zeros(len(accel_z_mps))

    for i in range(len(accel_x_mps)):
        gravity_x[i] = kf_x.update(accel_x_mps[i])
        gravity_y[i] = kf_y.update(accel_y_mps[i])
        gravity_z[i] = kf_z.update(accel_z_mps[i])

    # Calculate the magnitude of the filtered gravity
    gravity_mag = np.sqrt(gravity_x**2 + gravity_y**2 + gravity_z**2)

    # reconstructed gravity plot
    plot_data(time_ms, gravity_mag, "Filtered Gravity (Kalman)", 0, "m/s²")

    # Subtract gravity to isolate user acceleration
    user_accel_x = accel_x_mps - gravity_x
    user_accel_y = accel_y_mps - gravity_y
    user_accel_z = accel_z_mps - gravity_z

    plot_data(time_ms, user_accel_x, "Filtered Accel X over Time (Moving Average + Kalman)", 0, "m/s²")
    plot_data(time_ms, user_accel_y, "Filtered Accel Y over Time (Moving Average + Kalman)", 0, "m/s²")
    plot_data(time_ms, user_accel_z, "Filtered Accel Z over Time (Moving Average + Kalman)", 0, "m/s²")





    """
        ESTIMATE VELOCITY - currently uses Kalman user accel values
    """

    velocity_x = np.zeros(len(accel_x_mps))
    velocity_y = np.zeros(len(accel_y_mps))
    velocity_z = np.zeros(len(accel_z_mps))

    # Calculate time between samples
    dt = np.diff(time_ms)

    for i in range(1, len(accel_x)):
        # Compute acceleration magnitude at each time step
        accel_mag = np.sqrt(user_accel_x[i-1]**2 + user_accel_y[i-1]**2 + user_accel_z[i-1]**2)

        # Only integrate if the acceleration magnitude exceeds the threshold
        if accel_mag > 0.1:
            velocity_x[i] = velocity_x[i-1] + user_accel_x[i-1] * dt[i-1]
            velocity_y[i] = velocity_y[i-1] + user_accel_y[i-1] * dt[i-1]
            velocity_z[i] = velocity_z[i-1] + user_accel_z[i-1] * dt[i-1]
        else:
            # If acceleration is below threshold, don't integrate and set velocity to previous value
            velocity_x[i] = velocity_x[i-1]
            velocity_y[i] = velocity_y[i-1]
            velocity_z[i] = velocity_z[i-1]

    velocity_magnitude = np.sqrt(velocity_x**2 + velocity_y**2 + velocity_z**2)

    plot_data(time_ms, velocity_x, "Estimated X Velocity over Time", 0, "m/s")
    plot_data(time_ms, velocity_y, "Estimated Y Velocity over Time", 0, "m/s")
    plot_data(time_ms, velocity_z, "Estimated Z Velocity over Time", 0, "m/s")
    plot_data(time_ms, velocity_magnitude, "Estimated Velocity over Time", 0, "m/s")

    # Estimated velocity at impacts
    for impact in impacts:
        impact_time_idx = time_ms.index(impact[0])
        print(f"Estimated velocity at {impact[0]}: {velocity_magnitude[impact_time_idx]:.2f} m/s")


    # NOTES
    """
        - Filtering of gravity's acceleration is extremely poor with just accel/gyro
        - Introducing magnetometer may increase estimation?
        - To try: Extended Kalman filter, Madgwick filter, Complementary filter
    """


    # End program - press any button
    input()

if __name__ == "__main__":
    main()