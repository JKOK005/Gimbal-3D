import serial
import time

ser = serial.Serial("COM3", baudrate = 9600, timeout = 10)
time.sleep(1)
switcher = True

try:
	for i in range(20):
		if(switcher):
			ser.write(b'@HellofromPython')
			time.sleep(1)
		else:
			ser.write(b'@Arduinotester')
			time.sleep(1)
		switcher = not switcher
	# R = ser.readline()
	# print(R)
	# while(True):
	# tm = time.time()
	# ecd = bytes("$ Pitch = " + str(tm) + " ; Roll = " + str(tm) + " ; Yaw = " + str(tm) + " ; @", 'utf-8')
	# print(ecd)
		# ser.write(b'3')
	# print("writing")
	# time.sleep(1)
	# response = ser.read(1)
	# print(response)
		# R = ser.readline()
		# print(R)
	while(True):
		R = ser.readline()
		ser.flush()
		print(str(R))
		time.sleep(1)

except KeyboardInterrupt:
	ser.close()