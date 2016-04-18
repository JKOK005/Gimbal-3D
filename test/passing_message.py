import serial
import time

ser = serial.Serial("COM3", baudrate = 115200, timeout = 1)
time.sleep(1)
switcher = True
# q = ' '
counter = 0

try:
	for i in range(50):
		now = time.time()
		if(not switcher):
			ser.write(bytes('P:1.111;R:1.111;Y:1.111@', 'utf-8'))		# MUST limit float to 3 dp and in Radians

		else:
			ser.write(bytes('P:2.222;R:2.222;Y:-2.222@', 'utf-8'))	

		counter += 1
		# R = ser.readline()
		ser.flush()		
		# print(ser.in_waiting)
		# print(str(R))		
		# print(str(counter) + " " + str(time.time()-now))
		time.sleep(0.5)
		# switcher = not switcher

	while(True):
		# q = input("type")
		# if q == 'a': 
		# 	ser.write(b'P:1.111;R:5.555;Y:8.888@')
		print("Im waiting")
		R = ser.readline()
		print(ser.in_waiting)
		print(str(R))
		time.sleep(1)

except KeyboardInterrupt:
	ser.close()