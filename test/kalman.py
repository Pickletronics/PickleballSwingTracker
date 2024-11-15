import numpy as np
import matplotlib.pyplot as plt

class KalmanFilter:
    def __init__(self, process_variance=1e-5, measurement_variance=1e-2, estimate_initial=0):
        """
        Initialize the Kalman Filter for 1D data.
        
        process_variance: Process noise covariance (Q), expected noise in the system (gyro)
        measurement_variance: Measurement noise covariance (R), expected noise in the measurements (accelerometer)
        estimate_initial: Initial estimate of the state (angle)
        """
        self.process_variance = process_variance
        self.measurement_variance = measurement_variance
        self.estimate = estimate_initial
        self.estimate_covariance = 1.0  # Initial uncertainty

    def update(self, measurement, gyro_rate, dt):
        """
        Perform the Kalman filter update step.
        
        measurement: Accelerometer measurement (angle)
        gyro_rate: Gyroscope angular rate
        dt: Time step (in seconds)
        """
        # Prediction step (predict angle from gyro)
        predicted_estimate = self.estimate + gyro_rate * dt
        predicted_covariance = self.estimate_covariance + self.process_variance
        
        # Kalman gain
        kalman_gain = predicted_covariance / (predicted_covariance + self.measurement_variance)
        
        # Update step (correct with accelerometer measurement)
        self.estimate = predicted_estimate + kalman_gain * (measurement - predicted_estimate)
        self.estimate_covariance = (1 - kalman_gain) * predicted_covariance
        
        return self.estimate


# # Example usage
# def simulate_data():
#     """Simulate accelerometer and gyroscope data for angle estimation."""
#     accel_data = [(0, 9.81, 0), (0.1, 9.8, 0.1), (0.2, 9.7, 0.2), (0.3, 9.6, 0.3)]  # Simulated accelerometer data (x, y, z)
#     gyro_data = [(0.01, 0.02, 0.03), (0.02, 0.03, 0.04), (0.03, 0.04, 0.05), (0.04, 0.05, 0.06)]  # Simulated gyroscope data (x, y, z)
#     dt = 0.1  # Time step in seconds
#     return accel_data, gyro_data, dt

# # Initialize Kalman filter
# kf = KalmanFilter(process_variance=1e-5, measurement_variance=1e-2)

# # Simulate the data
# accel_data, gyro_data, dt = simulate_data()

# # Apply Kalman filter to each time step
# filtered_angles = []
# for i in range(len(accel_data)):
#     # Accelerometer angle estimation (assuming gravity is present)
#     accel_angle = np.arctan2(accel_data[i][1], accel_data[i][2])  # Roll angle (using y and z axes)
    
#     # Gyroscope rate of change (assume we use gyro x-axis for simplicity)
#     gyro_rate = gyro_data[i][0]
    
#     # Update Kalman filter
#     filtered_angle = kf.update(accel_angle, gyro_rate, dt)
#     filtered_angles.append(filtered_angle)

# # Plot the filtered angles
# plt.plot(filtered_angles, label="Filtered Angle (Kalman)")
# plt.title('Filtered Angle using Kalman Filter')
# plt.xlabel('Time')
# plt.ylabel('Angle (rad)')
# plt.grid(True)
# plt.legend()
# plt.show()
