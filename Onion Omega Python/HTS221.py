# Distributed with a free-will license.
# Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
# HTS221
# This code is designed to work with the HTS221_I2CS I2C Mini Module available from ControlEverything.com.
# https://www.controleverything.com/content/Humidity?sku=HTS221_I2CS#tabs-0-product_tabset-2

from OmegaExpansion import onionI2C
import time

# Get I2C bus
i2c = onionI2C.OnionI2C()

# HTS221 address, 0x5F(95)
# Select average configuration register, 0x10(16)
#		0x1B(27)	Temperature average samples = 256, Humidity average samples = 512
i2c.writeByte(0x5F, 0x10, 0x1B)
# HTS221 address, 0x5F(95)
# Select control register1, 0x20(32)
#		0x85(133)	Power ON, Continuous update, Data output rate = 1 Hz
i2c.writeByte(0x5F, 0x20, 0x85)

time.sleep(0.5)

# HTS221 address, 0x5F(95)
# Read Calibration values from non-volatile memory of the device
# Humidity Calibration values
# Read data back from 0x30(48), 1 byte
val = i2c.readBytes(0x5F, 0x30, 1)
H0 = val[0] / 2

# Read data back from 0x31(49), 1 byte
val = i2c.readBytes(0x5F, 0x31, 1)
H1 = val[0] / 2

# Read data back from 0x36(54), 2 bytes
val0 = i2c.readBytes(0x5F, 0x36, 1)
val1 = i2c.readBytes(0x5F, 0x37, 1)
H2 = ((val1[0] & 0xFF) * 256) + (val0[0] & 0xFF)

# Read data back from 0x3A(58), 2 bytes
val0 = i2c.readBytes(0x5F, 0x3A, 1)
val1 = i2c.readBytes(0x5F, 0x3B, 1)
H3 = ((val1[0] & 0xFF) * 256) + (val0[0] & 0xFF)

# Temperature Calibration values
# Read data back from 0x32(50), 1 byte
T0 = i2c.readBytes(0x5F, 0x32, 1)
T0[0] = (T0[0] & 0xFF)

# Read data back from 0x32(51), 1 byte
T1 = i2c.readBytes(0x5F, 0x33, 1)
T1[0] = (T1[0] & 0xFF)

# Read data back from 0x35(53), 1 byte
raw = i2c.readBytes(0x5F, 0x35, 1)
raw[0] = (raw[0] & 0x0F)

# Convert the temperature Calibration values to 10-bits
T0[0] = ((raw[0] & 0x03) * 256) + T0[0]
T1[0] = ((raw[0] & 0x0C) * 64) + T1[0]

# Read data back from 0x3C(60), 2 bytes
val0 = i2c.readBytes(0x5F, 0x3C, 1)
val1 = i2c.readBytes(0x5F, 0x3D, 1)
T2 = ((val1[0] & 0xFF) * 256) + (val0[0] & 0xFF)

# Read data back from 0x3E(62), 2 bytes
val0 = i2c.readBytes(0x5F, 0x3E, 1)
val1 = i2c.readBytes(0x5F, 0x3F, 1)
T3 = ((val1[0] & 0xFF) * 256) + (val0[0] & 0xFF)

# Read data back from 0x28(40) with command register 0x80(128), 4 bytes
# humidity msb, humidity lsb, temp msb, temp lsb
data = i2c.readBytes(0x5F, 0x28 | 0x80, 4)

# Convert the data
humidity = (data[1] * 256) + data[0]
humidity = ((1.0 * H1) - (1.0 * H0)) * (1.0 * humidity - 1.0 * H2) / (1.0 * H3 - 1.0 * H2) + (1.0 * H0)
temp = (data[3] * 256) + data[2]
if temp > 32767 :
	temp -= 65536
cTemp = ((T1[0] - T0[0]) / 8.0) * (temp - T2) / (T3 - T2) + (T0[0] / 8.0)
fTemp = (cTemp * 1.8 ) + 32

# Output data to screen
print "Relative Humidity : %.2f %%" %humidity
print "Temperature in Celsius : %.2f C" %cTemp
print "Temperature in Fahrenheit : %.2f F" %fTemp
