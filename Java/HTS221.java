// Distributed with a free-will license.
// Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
// HTS221
// This code is designed to work with the HTS221_I2CS I2C Mini Module available from ControlEverything.com.
// https://www.controleverything.com/content/Humidity?sku=HTS221_I2CS#tabs-0-product_tabset-2

import com.pi4j.io.i2c.I2CBus;
import com.pi4j.io.i2c.I2CDevice;
import com.pi4j.io.i2c.I2CFactory;
import java.io.IOException;

public class HTS221
{
	public static void main(String args[]) throws Exception
	{
		// Create I2CBus
		I2CBus bus = I2CFactory.getInstance(I2CBus.BUS_1);
		// Get I2C device, HTS221 I2C address is 0x5F(95)
		I2CDevice device = bus.getDevice(0x5F);

		// Select average configuration register
		// Temperature average samples = 16, humidity average samples = 32
		device.write(0x10, (byte)0x1B);
		// Select control register1
		// Power on, block data update, data rate o/p = 1 Hz
		device.write(0x20, (byte)0x85);
		Thread.sleep(500);

		// Read Calliberation values from the non-volatile memory of the device
		// Humidity calliberation values
		byte[] val = new byte[2];
		// Read 1 byte of data from address 0x30(48)
		val[0] = (byte)device.read(0x30);
		// Read 1 byte of data from address 0x31(49)
		val[1] = (byte)device.read(0x31);
		int H0 = (val[0] & 0xFF) / 2;
		int H1 = (val[1] & 0xFF) / 2;
		

		// Read 1 byte of data from address 0x36(54)
		val[0] = (byte)device.read(0x36);
		// Read 1 byte of data from address 0x37(55)
		val[1] = (byte)device.read(0x37);
		int H2 = ((val[1] & 0xFF) * 256) + (val[0] & 0xFF);

		// Read 1 byte of data from address 0x3A(58)
		val[0] = (byte)device.read(0x3A);
		// Read 1 byte of data from address 0x3B(59)
		val[1] = (byte)device.read(0x3B);
		int H3 = ((val[1] & 0xFF) * 256) + (val[0] & 0xFF);

		// Temperature calliberation values
		// Read 1 byte of data from address 0x32(50)
		int T0 = ((byte)device.read(0x32) & 0xFF);
		// Read 1 byte of data from address 0x33(51)
		int T1 = ((byte)device.read(0x33) & 0xFF);

		// Read 1 byte of data from address 0x35(53)
		int raw = ((byte)device.read(0x35) & 0x0F);
		
		// Convert the temperature calliberation values to 10-bits
		T0 = ((raw & 0x03) * 256) + T0;
		T1 = ((raw & 0x0C) * 64) + T1;

		// Read 1 byte of data from address 0x3C(60)
		val[0] = (byte)device.read(0x3C);
		// Read 1 byte of data from address 0x3D(61)
		val[1] = (byte)device.read(0x3D);
		int T2 = ((val[1] & 0xFF) * 256) + (val[0] & 0xFF);

		// Read 1 byte of data from address 0x3E(62)
		val[0] = (byte)device.read(0x3E);
		// Read 1 byte of data from address 0x3F(63)
		val[1] = (byte)device.read(0x3F);
		int T3 = ((val[1] & 0xFF) * 256) + (val[0] & 0xFF);
		
		// Read 4 bytes of data
		// hum msb, hum lsb, temp msb, temp lsb
		byte[] data = new byte[4];
		device.read(0x28 | 0x80, data, 0, 4);

		// Convert the data
		int hum = ((data[1] & 0xFF) * 256) + (data[0] & 0xFF); 
		int temp = ((data[3] & 0xFF) * 256) + (data[2] & 0xFF);
		if(temp > 32767)
		{
			temp -= 65536;
		}
		double humidity = ((1.0 * H1) - (1.0 * H0)) * (1.0 * hum - 1.0 * H2) / (1.0 * H3 - 1.0 * H2) + (1.0 * H0);
		double cTemp = ((T1 - T0) / 8.0) * (temp - T2) / (T3 - T2) + (T0 / 8.0);
		double fTemp = (cTemp * 1.8 ) + 32;

		// Output data to screen
		System.out.printf("Relative Humidity : %.2f %% RH %n", humidity);
		System.out.printf("Temperature in Celsius : %.2f C %n", cTemp);
		System.out.printf("Temperature in Farhenheit : %.2f F %n", fTemp);
	}
}
