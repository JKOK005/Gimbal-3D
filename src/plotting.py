"""
Library to handling all plotting and visual functions
We store the most recent xx values of the IMU as well as the computed true state value for plotting
"""

import matplotlib.pyplot as plt
import numpy as np
import time

class visual():
	def __init__(self):
		self.IMU_reading_gyro_x 	= []		# Tracks the individual state of the IMU across time as a list of numpy arrays
		self.IMU_reading_gyro_y 	= []
		self.IMU_reading_gyro_z 	= []
		self.IMU_reading_accel_x 	= []
		self.IMU_reading_accel_y 	= []
		self.IMU_reading_accel_z 	= []

		self.true_state_gyro_x 		= []		# Tracks the individual true state of the system based on Kalman filtering as a list of numpy arrays
		self.true_state_gyro_y 		= []
		self.true_state_gyro_z 		= []
		self.true_state_accel_x 	= []
		self.true_state_accel_y 	= []
		self.true_state_accel_z 	= []

		self.length_limit = 20				# Holds a total of xx readings

	def update_IMU_reading(self, update):
		tmp = update.tolist()

		if len(self.IMU_reading_gyro_x) == self.length_limit:
			# Delete first element before updating list
			del self.IMU_reading_gyro_x[0]
			del self.IMU_reading_gyro_y[0]
			del self.IMU_reading_gyro_z[0]
			del self.IMU_reading_accel_x[0]
			del self.IMU_reading_accel_y[0]
			del self.IMU_reading_accel_z[0]

		self.IMU_reading_gyro_x.append(tmp[0])
		self.IMU_reading_gyro_y.append(tmp[1])
		self.IMU_reading_gyro_z.append(tmp[2])
		self.IMU_reading_accel_x.append(tmp[3])
		self.IMU_reading_accel_y.append(tmp[4])
		self.IMU_reading_accel_z.append(tmp[5])

	def update_true_state(self, update):
		tmp = update.tolist()

		if len(self.true_state_gyro_x) == self.length_limit:
			# Delete first element before updating list
			del self.true_state_gyro_x[0]
			del self.true_state_gyro_y[0]
			del self.true_state_gyro_z[0]
			del self.true_state_accel_x[0]
			del self.true_state_accel_y[0]
			del self.true_state_accel_z[0]

		self.true_state_gyro_x.append(tmp[0])
		self.true_state_gyro_y.append(tmp[1])
		self.true_state_gyro_z.append(tmp[2])
		self.true_state_accel_x.append(tmp[3])
		self.true_state_accel_y.append(tmp[4])
		self.true_state_accel_z.append(tmp[5])

	def IMU_reading_plot(self):
		plt.ion()
		fig = plt.figure(1)

		# Gyro_x reading
		ax1 = fig.add_subplot(3,2,1)
		ax1.plot(self.IMU_reading_gyro_x, 'b')

		# Gyro_y reading
		#ax2 = fig.add_subplot(3,2,2)
		#ax2.plot(self.IMU_reading_gyro_y, 'r')

		# Accel_z reading
		#ax3 = fig.add_subplot(3,2,3)
		#ax3.plot(self.IMU_reading_gyro_y, 'g')

		# Accel_x reading
		#ax4 = fig.add_subplot(3,2,4)
		#ax4.plot(self.IMU_reading_accel_x, 'r')

		# Accel_y reading
		#ax5 = fig.add_subplot(3,2,5)
		#ax5.plot(self.IMU_reading_accel_y, 'b')

		# Accel_z reading
		ax6 = fig.add_subplot(3,2,6)
		ax6.plot(self.IMU_reading_accel_z, 'g')

		fig.canvas.draw()
		time.sleep(0.1)
		plt.clf()
		return

	def true_state_plot(self):
		plt.ion()
		fig = plt.figure(1)

		# Gyro_x reading
		ax1 = fig.add_subplot(3,2,1)
		ax1.plot(self.IMU_reading_gyro_x, 'b')

		# Gyro_y reading
		ax2 = fig.add_subplot(3,2,2)
		ax2.plot(self.IMU_reading_gyro_y, 'r')

		# Accel_z reading
		ax3 = fig.add_subplot(3,2,3)
		ax3.plot(self.IMU_reading_gyro_y, 'g')

		# Accel_x reading
		ax4 = fig.add_subplot(3,2,4)
		ax4.plot(self.IMU_reading_accel_x, 'r')

		# Accel_y reading
		ax5 = fig.add_subplot(3,2,5)
		ax5.plot(self.IMU_reading_accel_y, 'b')

		# Accel_z reading
		ax6 = fig.add_subplot(3,2,6)
		ax6.plot(self.IMU_reading_accel_z, 'g')

		fig.canvas.draw()
		return
