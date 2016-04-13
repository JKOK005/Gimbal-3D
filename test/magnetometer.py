import smbus
import math
import time

# Registers
power_mgmt_1 = 0x6b
power_mgmt_2 = 0x6c

bus = smbus.SMBus(1) #I2C port 1

def read_word_magnetometer(adr):
	low = bus.read_byte_data(address, adr)
	high= bus.read_byte_data(address, adr+1)
	val = (high << 8) + low
	return val

def read_word_2c_magnetometer(adr):
	val= read_word(adr)
	if (val >= 0x8000):
		return - ((65535 - val) + 1)
	else:
		return val

#Now set up magnetometer
bus.write_byte(0x0A, 0x12) #turns on 16-bit, continuous measurement settings

i=0
while True:
	i = i+1
	print "------------"
	print "acceleromater data, cnt =%d" %i
	accel_xout = read_word_2c_magnetometer(0x03)
	accel_yout = read_word_2c_magnetometer(0x05)
	accel_zout = read_word_2c_magnetometer(0x07)


	time.sleep(1)
