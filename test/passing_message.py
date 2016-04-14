import serial
import time

ser = serial.Serial("COM3", baudrate = 9600, timeout = 5)
time.sleep(1)
switcher = True
# q = ' '

try:
	for i in range(9):
		if(switcher):
			ser.write(bytes('P:0.200;R:2.931;Y:3.653@', 'utf-8'))		# MUST limit float to 3 dp and in Radians
			time.sleep(1)
		else:
			ser.write(bytes('P:-0.200;R:8.121;Y:6.592@', 'utf-8'))
			time.sleep(1)
		# switcher = not switcher

	while(True):
		# q = input("type")
		# if q == 'a': 
		# 	ser.write(b'P:1.111;R:5.555;Y:8.888@')
		print("Im waiting")
		R = ser.readline()
		ser.flush()
		print(ser.in_waiting)
		print(str(R))
		time.sleep(1)

except KeyboardInterrupt:
	ser.close()