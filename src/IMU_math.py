"""
Library to handle all functionalities of the gyro
This includes reading and integrating the reading
"""
import numpy as np
from np.linalg import inv
import np.transpose as tps
import math

def get_IMU_reading():
	"""
	Gets the IMU reading based on the circuit diagram
	"""
	pass

def get_integrate_gyro(gyro_reading, smpl_time):
	"""
	Integrates the gyro reading given a sampling time.
	Integration is performed numerically

	param: 	gyro_reading -> numpy.ndarray type that is the reading of the angular velocities
			smpl_time -> float type that is the sampling rate of the IMU

	rtype:	numpy.ndarray type that is the result after performing integration 
	"""

	assert type(gyro_reading) is np.ndarray and type(smpl_time) is float
	return gyro_reading*smpl_time

def get_state_accel(accel_reading):
	"""
	Gets the state of the system based on accelerometer readings
	"""
	pass

def kalman_filter(F, B, R, Q, u_k, x_km1_km1, z_k):
	"""
	Function to implement the Kalman kalman filter.
	Assumptions made are that the distribution of the system is gaussian with mean and variance
	All bias from accelerometer and gyro are assumed to be constant

	Theory:
	Assume that systems true state at time k given state at time k-1 is modelled as:
	>>> x_k_km1 = F*x_km1_km1 + B*u_k + w_k
	Where w_k is the process noise 

	The measured value of the system at time k relates to x_k_km1 by:
	>>> z_k = H*x_k_km1 + v_k
	v_k is the noise 

	Start by calculating x_k_km1 ignoring the noise present
	>>> x_k_km1 = F*x_km1_km1 + B*u_k

	Calculate the innovation (y_k) using
	>>> y_k = z_k - H*x_k_km1

	Calculate the error covariance matrix P_k_km1 using
	>>> P_k_km1 = F *P_km1_km1 *F' + Q

	Calculate the innovation covariance matrix using
	>>> S_k = H *P_k_km1 *H' + R

	Calculate the kalman gain using
	>>> K_k = P_k_km1 *H' *inv(S_k)

	Calculate the true state x_k_k using
	>>> x_k_k = x_k_km1 + K_k *y_k

	Calculate the error covariance matrix at time k 
	>>> P_k_k = (I - K_k *H) *P_k_km1

	param: 	F: State transition matrix 							- 2 x 2
			B: Mapping matrix for control input					- 2 x 1
			R: Measurement covariance matrix 					- 1 x 1			
			Q: Noise covariance matrix 							- 2 x 2
			u_k: measured angular velocity at time k 			- 1 x 1
			x_km1_km1: The true state of the system at time k-1	- 2 x 1
			z_k: The measured state of the system at time k 	- 1 x 1

	rtype: 	x_k_k: The true state of the system at time k
			P_k_k: The error covariance matrix at time k
	"""
	H = np.matrix([1, 0])			# Maps true state to measured reading

	x_k_km1 = F*x_km1_km1 + B*u_k	
	y_k = z_k - H*x_k_km1			
	P_k_km1 = F *P_km1_km1 *tps(F) + Q
	S_k = H *P_k_km1 *tps(H) + R
	K_k = P_k_km1 *tps(H) *inv(S_k)
	x_k_k = x_k_km1 + K_k *y_k
	P_k_k = (np.identity(3) - K_k *H) *P_k_km1

	return x_k_k, P_k_k