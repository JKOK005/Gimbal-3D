"""
Library to handling all plotting and visual functions
"""

import matplotlib.pyplot as plt
import numpy as np

class visual():
	def __init__(self):
		self.IMU_state_container = []		# Tracks the state of the IMU across time
		self.true_state_container = []		# Tracks the true state of the system based on Kalman filtering
