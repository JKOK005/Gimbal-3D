import serial
import time

ser = serial.Serial("COM3", baudrate = 9600, timeout = 5)
time.sleep(1)
switcher = True
# q = ' '

try:
	for i in range(20):
		if(switcher):
			ser.write(bytes('P:0.40;R:2.93;Y:3.65@', 'utf-8'))		# MUST limit float to 3 dp and in Radians
		else:
			ser.write(bytes('P:-0.40;R:8.12;Y:-6.59@', 'utf-8'))
		time.sleep(1)
		ser.flush()
		switcher = not switcher

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