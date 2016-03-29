import smbus
import math
import time

# Registers
power_mgmt_1 = 0x6b
power_mgmt_2 = 0x6c

bus = smbus.SMBus(1) #I2C port 1
address = 0x68 # This is the address value via the i2cdetect command or chip user guide

def read_word(adr):
	high = bus.read_byte_data(address, adr)
	low = bus.read_byte_data(address, adr+1)
	val = (high << 8) + low
	return val

def read_word_2c(adr):
	val= read_word(adr)
	if (val >= 0x8000):
		return - ((65535 - val) + 1)
	else:
		return val

#Now wake the 6050 up as it starts in sleep mode
bus.write_byte_data(address, power_mgmt_1, 0)

#use the default scale to 2g
SCALE = 65536.0/4

i=0
while True:
	i = i+1
	print "------------"
	print "acceleromater data, cnt =%d" %i
	accel_xout = read_word_2c(0x3b)
	accel_yout = read_word_2c(0x3d)
	accel_zout = read_word_2c(0x3f)

	accel_xout_scaled = accel_xout / SCALE
	accel_yout_scaled = accel_yout / SCALE
	accel_zout_scaled = accel_zout / SCALE	

	print "accel_xout: ", accel_xout, " scaled: %.3f" %accel_xout_scaled
	print "accel_yout: ", accel_yout, " scaled: %.3f" %accel_yout_scaled
	print "accel_zout: ", accel_zout, " scaled: %.3f" %accel_zout_scaled

	time.sleep(1)
