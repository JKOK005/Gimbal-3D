"""
3D Gimbal for smart phones

The system consists of 2 levels of interfaces. 
	The Raspberry Pi takes care of intensive computation, state tracking and PID controls
	This information is given to the Arduino which is in charge of low level motor controls

	Information flow:
		IMU readings -> Raspberry -> Arduino -> Motors -> IMU readings

Main script that performs the tasks of getting the reading of the IMU and computing the resultant angle 

The Raspberry Pi will first query the IMU for the gyro readings and acceleration values.
The values are then used to determine the actual state of the system in roll / pitch / yaw 
Gyro readings are integrated across a time step based on the sampling interval of the Raspberry
Accelerometer values are transformed based on a mapping function to estimated states of the system
We then apply a Kalman filtering which tracks the system across time
The Kalman filter will have higher weightage towards the Gyro readings during the first initial time steps but this
weightage will diminish as the Gyro becomes less accurate over time

In order to ensure that constant polling of the IMU states are performed, a threading module is incorporated to split the tasks
between sensor collection, PID control and state tracking of the system. 
We link all thread modules together using global variables with locks 

The PID control mitigates the error which is defined as the descrepancy between the Gimbal's current state and the desired state
(which we assume to be perfectly horizontal). 

Dependencies:

User input:
"""

import threading
import time
import numpy as np
import IMU_math as IMU
import copy.copy as cp

class gimbal_driver(threading.Thread):
	"""
	gimbal_driver class that the user calls to initialize all necessary threads and start the operation.
	"""
	def __init__(self):
		self.global_IMU_reading 	= {"reading": None, "val":{"type":np.ndarray, "len":6}, "lock":threading.Lock()}				# Global IMU state reading for gyro and accelerometer
		self.global_state_gyro 		= {"reading": None, "val":{"type":np.ndarray, "len":3}, "lock":threading.Lock()}				# Global gimbal state based only from Gyro readings
		self.global_state_accel		= {"reading": None, "val":{"type":np.ndarray, "len":3}, "lock":threading.Lock()}				# Global gimbal state based only from accelerometer readings
		self.desired_state 			= {"reading": np.array([0,0,0]), "val":{"type":np.ndarray, "len":3}, "lock":threading.Lock()}	# Global user desired sate of gimbal after filtering 
		self.global_true_state 		= {"reading": np.array([0,0,0]), "val":{"type":np.ndarray, "len":3}, "lock":threading.Lock()}	# Global true sate of gimbal after filtering 

		self.smpl_time = 10		# 10 ms frequency of sampling time
		self.P_k_k = np.matrix([0,0,0],[0,0,0],[0,0,0])

	def init_threads(self, obj):
		"""
		Initializes the necessary threads with the target functions

		param: obj: gimbal_driver object
		rtype: None
		""" 
		self.IMU_poller_trd 	= threading.Thread(target=obj.update_IMU_reading, name="IMU_poller")
		self.filtering_trd 		= threading.Thread(target=obj.update_true_state, name="filter_thread")
		self.signal_trd 		= threading.Thread(target=obj.update_signal, name="signal_thread")
		self.desired_state_trd 	= threading.Thread(target=obj.update_desired_state, name="desired_state_thread")

		self.thread_collector 	= [self.IMU_poller_trd, self.filtering_trd, self.signal_trd, self.desired_state_trd]
		for j in self.thread_collector:
			j.daemon = True 				# Sets all threads to Daemon thread
			j.start()						# Starts the running of all threads
			time.sleep(0.1)

		print("Total thread count: " + str(len(self.thread_collector)))

	def update_IMU_reading(self):
		"""
		Function will poll the IMU for readings from the Gyro and Accelerometer
		This will update the global_IMU_reading variable in the form np.array([AccX, AccY, AccZ, OmegaX, OmegaY, OmegaZ])

		global_IMU_reading -> Read and update variable

		param: None
		rtype: None
		"""
		''' Code to poll the IMU for various readings '''
		# Spins until the main thread dies
		while(thread.current_thread.is_alive()):
			prev_time = time.time()
			update = IMU.get_IMU_reading()
			self.__access_global_var(glob=self.global_IMU_reading, update=update, thrd_name=threading.current_thread().getName())
			run_time = time.time() - prev_time
			time.sleep(max(0, self.smpl_time - run_time))			# Enforces consistent sampling time of the IMU
		return
		# print("IMU update")
		# print(threading.current_thread().getName())

	def update_true_state(self):
		"""
		Function constantly polls for an update on the IMU readings. Once IMU_poller_trd updates the readings,
		filtering_trd will run both the integrative process on the Gryo readings and the trigonometric calculations 
		from the accelerometer readings. It then performs a kalman filtering process to track the Gimbal's true state

		global_state_gyro -> Updated based on integrating readings 
		global_state_accel -> Updated based on trigonometric formula 
		global_true_state -> Updated based on kalman filtering

		param: None
		rtype: None
		"""
		# Spins until the main thread dies
		while(threading.current_thread.is_alive()):
			IMU_readings = self.__access_global_var(glob=self.global_IMU_reading, thrd_name=threading.current_thread().getName())
			state_prev = self.__access_global_var(glob=self.global_true_state, thrd_name=threading.current_thread().getName())
			gyro_readings = IMU_readings[3:]; accel_readings = IMU_readings[:3]

			gyro_state = self.__get_state_from_gyro(gyro_readings, state_prev)		# Performs integration to get true state from gyro
			accel_state = self.__get_state_from_accel(accel_readings)				# Performs trigo to get true state from accelerometer
			est_state = math.mean(gyro_state, accel_state)							# Take the average state
			x_k_k, P_update = self.__get_state_from_kalman(z_k=est_state, u_k=gyro_readings)			# Apply kalman filtering

			self.__access_global_var(glob=self.global_state_gyro, update=gyro_state, thrd_name=threading.current_thread().getName())
			self.__access_global_var(glob=self.global_state_accel, update=accel_state, thrd_name=threading.current_thread().getName())
			self.__access_global_var(glob=self.global_true_state, update=x_k_k, thrd_name=threading.current_thread().getName())
			self.P_k_k = P_update
		return
		# print("True state update")
		# print(threading.current_thread().getName())

	def update_signal(self):
		"""
		Function polls for changes in global_true_state. It then sends a signal to the arduino to turn the motor based on
		the error betweent global_true_state and the desired state (roll / pitch / yaw) of the gimbal.

		param: None
		rtype: None
		"""
		while(threading.current_thread.is_alive()):
			pass
		return		
		# print("Signal update")
		# print(threading.current_thread().getName())

	def update_desired_state(self):
		"""
		Function updates the desired state of the gimbal by reading inputs and updating desired_state variable
		We define the desired state as the state the user wants the gimbal to tilt

		param: None	
		rtype: None
		"""
		print("Desired state update")
		print(threading.current_thread().getName())

	def __access_global_var(self, glob, update=None, thrd_name=None):
		"""
		Checks the target value of each call and will perform the necessary updating functions

		param: 	glob: 			The name of the global variable that is being targeted
				update:			The new changes to be pushed to glob. If None, then we treat the caller as a request for the value and returns desired vale
				thrd_name: 		The name of the active thread calling it

		rtype: self.glob['reading'] type
		"""
		with glob['lock']:
			if update == None:
				return glob['reading']
			else:
				assert type(update) is glob['val']['type'] and len(update) is glob['val']['len']		# Enforces valid update
				glob['reading'] = update
		return

	def __get_state_from_gyro(self, gyro_reading, state_prev):
		"""
		Performs integrative process based on sampling time 
		gyro_readings is the raw readings from the IMU in np.array([OmegaX, OmegaY, OmegaZ])
		state_prev is the current state of the system at time t-1. We want to determine the system's state at time t
		"""
		delta_theta = IMU.get_integrate_gyro(gyro_reading, self.smpl_time)		# get_integrate_gyro from function gyro_math.py
		state_new = state_prev + delta_theta
		return state_new

	def __get_state_from_accel(self, accel_reading):
	"""
	Performs integrative process based on sampling time 
	"""
		pass

	def __get_state_from_kalman(self, z_k, u_k):
	""" 
	Handles the kalman filtering to obtain the true state of the gimbal 
	Reference: http://blog.tkjelectronics.dk/2012/09/a-practical-approach-to-kalman-filter-and-how-to-implement-it/

	true_state is the state of the system at time K-1
	est_state is the state of the system at time K, which is the measured state based on IMU readings

	We want to perform the filtering to get the new true_state of the system at time K

	param: 	z_k: The measured state of the system at time K
			u+k: The measured gyro readings at time K
	"""
		# IMu parameters
		R_measure 	= 0.03	# Measurement noise variance for Roll / Pitch / Yaw
		Q_angle	 	= 0.001	# Accelerometer bias in X / Y / Z
		Q_gyro	 	= 0.003	# Gyro bias in Roll / Pitch / Yaw

		# Kalman filtering parameters
		F = np.matrix([1,-self.smpl_time],[0,1])		# State transition matrix
		B = np.matrix([self.smpl_time],[0])	

		"""Start Kalman filtering from here onwards"""

		Q_matrix 	= np.matrix([Q_angle,0],[0,Q_gyro])
		x_km1_km1 	= self.__access_global_var(glob=self.global_true_state, update=None, thrd_name=None)

		return IMU.kalman_filter(F,B,R_measure,Q_matrix,u_k,x_km1_km1,z_k)

if __name__ == "__main__":
	launch_obj = gimbal_driver()
	launch_obj.init_threads(launch_obj)
	print("Press q to exit")
	
	while(input() != 'q'):
		# Continue execution of programme unless told to stop
		pass

	