import numpy as np
import matplotlib.pyplot as plt

def complementary_filter(accel_data, gyro_data, dt, alpha=0.98):
    """
    Apply a complementary filter to combine accelerometer and gyroscope data.

    accel_data: Tuple of accelerometer data (ax, ay, az), assuming gravity is present
    gyro_data: Tuple of gyroscope data (gx, gy, gz), in radians per second
    dt: Time step between measurements (in seconds)
    alpha: Filter coefficient (controls how much weight is given to accelerometer vs gyro)
    """
    # Initialize the angle estimate
    angle = np.arctan2(accel_data[1], accel_data[2])  # Assuming accelerometer x, y, z axes

    # Initialize the filtered angles (assuming initial conditions are zero)
    angle_x, angle_y = 0, 0

    angles = []  # Store the angles for plotting

    for i in range(len(accel_data)):
        # Accelerometer angle calculation (using pitch and roll)
        accel_angle_x = np.arctan2(accel_data[i][1], accel_data[i][2])  # Roll angle
        accel_angle_y = np.arctan2(accel_data[i][0], accel_data[i][2])  # Pitch angle

        # Gyroscope angle change
        gyro_angle_x = gyro_data[i][0] * dt  # Change in angle (roll)
        gyro_angle_y = gyro_data[i][1] * dt  # Change in angle (pitch)

        # Apply complementary filter
        angle_x = alpha * (angle_x + gyro_angle_x) + (1 - alpha) * accel_angle_x
        angle_y = alpha * (angle_y + gyro_angle_y) + (1 - alpha) * accel_angle_y

        angles.append((angle_x, angle_y))

    return np.array(angles)

# # Example usage
# # Simulate accelerometer (accel) and gyroscope (gyro) data
# # Here we simulate data with some noise for demonstration

# # Accelerometer data (x, y, z): Assuming gravity is present
# accel_data = [(0, 9.81, 0), (0.1, 9.8, 0.1), (0.2, 9.7, 0.2), (0.3, 9.6, 0.3)]  # Simplified data

# # Gyroscope data (x, y, z): Simulated angular velocities in radians per second
# gyro_data = [(0.01, 0.02, 0.03), (0.02, 0.03, 0.04), (0.03, 0.04, 0.05), (0.04, 0.05, 0.06)]

# # Time step between measurements (seconds)
# dt = 0.1

# # Apply complementary filter
# angles = complementary_filter(accel_data, gyro_data, dt, alpha=0.98)

# # Plot the filtered angles
# plt.figure()
# plt.subplot(2, 1, 1)
# plt.plot(angles[:, 0], label="Filtered Roll Angle")
# plt.title('Filtered Roll Angle')
# plt.xlabel('Time')
# plt.ylabel('Angle (rad)')

# plt.subplot(2, 1, 2)
# plt.plot(angles[:, 1], label="Filtered Pitch Angle")
# plt.title('Filtered Pitch Angle')
# plt.xlabel('Time')
# plt.ylabel('Angle (rad)')

# plt.tight_layout()
# plt.show()
