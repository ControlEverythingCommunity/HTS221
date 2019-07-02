// Distributed with a free-will license.
// Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
// HTS221
// This code is designed to work with the HTS221_I2CS I2C Mini Module available from ControlEverything.com.
// https://www.controleverything.com/content/Humidity?sku=HTS221_I2CS#tabs-0-product_tabset-2

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>

void main() 
{
	// Create I2C bus
	int file;
	char *bus = "/dev/i2c-1";
	if((file = open(bus, O_RDWR)) < 0) 
	{
		printf("Failed to open the bus. \n");
		exit(1);
	}
	// Get I2C device, HTS221 I2C address is 0x5F(95)
	ioctl(file, I2C_SLAVE, 0x5F);

	// Select average configuration register(0x10)
	// Temperature average samples = 16, humidity average samples = 32(0x1B)
	char config[2] = {0};
	config[0] = 0x10;
	config[1] = 0x1B;
	write(file, config, 2);
	sleep(1);
	// Select control register1(0x20)
	// Power on, block data update, data rate o/p = 1 Hz(0x85)
	config[0] = 0x20;
	config[1] = 0x85;
	write(file, config, 2);
	sleep(1);

	// Read Calibration values from the non-volatile memory of the device
	// Humidity Calibration values
	//Read 1 byte of data from address(0x30)
	char reg[1] = {0x30};
	write(file, reg, 1);
	char data[4] = {0};
	if(read(file, data, 1) != 1)
	{
		printf("Erorr : Input/output Erorr \n");
		exit(1);
	}
	char data_0 = data[0];
	// Read 1 byte of data from address(0x31)
	reg[0] = 0x31;
	write(file, reg, 1);
	read(file, data, 1);
	char data_1 = data[0];

	int H0 = data_0 / 2;
	int H1 = data_1 / 2;
	
	//Read 1 byte of data from address(0x36)
	reg[0] = 0x36;
	write(file, reg, 1);
	if(read(file, data, 1) != 1)
	{
		printf("Erorr : Input/output Erorr \n");
		exit(1);
	}
	data_0 = data[0];
	// Read 1 byte of data from address(0x37)
	reg[0] = 0x37;
	write(file, reg, 1);
	read(file, data, 1);
	data_1 = data[0];

	int H2 = (data_1 * 256) + data_0;
	
	//Read 1 byte of data from address(0x3A)
	reg[0] = 0x3A;
	write(file, reg, 1);
	if(read(file, data, 1) != 1)
	{
		printf("Erorr : Input/output Erorr \n");
		exit(1);
	}
	data_0 = data[0];
	// Read 1 byte of data from address(0x3B)
	reg[0] = 0x3B;
	write(file, reg, 1);
	read(file, data, 1);
	data_1 = data[0];

	int H3 = (data_1 * 256) + data_0;
	
	// Temperature Calibration values
	// Read 1 byte of data from address(0x32)
	reg[0] = 0x32;
	write(file, reg, 1);
	read(file, data, 1);
	
	int T0 = data[0];
	
	// Read 1 byte of data from address(0x33)
	reg[0] = 0x33;
	write(file, reg, 1);
	read(file, data, 1);
	
	int T1 = data[0];
	
	// Read 1 byte of data from address(0x35)
	reg[0] = 0x35;
	write(file, reg, 1);
	read(file, data, 1);
	
	int raw = data[0];
	
	// Convert the temperature Calibration values to 10-bits
	T0 = ((raw & 0x03) * 256) + T0;
	T1 = ((raw & 0x0C) * 64) + T1;
	
	//Read 1 byte of data from address(0x3C)
	reg[0] = 0x3C;
	write(file, reg, 1);
	if(read(file, data, 1) != 1)
	{
		printf("Erorr : Input/output Erorr \n");
		exit(1);
	}
	data_0 = data[0];
	// Read 1 byte of data from address(0x3D)
	reg[0] = 0x3D;
	write(file, reg, 1);
	read(file, data, 1);
	data_1 = data[0];

	int T2 = (data_1 * 256) + data_0;
	
	//Read 1 byte of data from address(0x3E)
	reg[0] = 0x3E;
	write(file, reg, 1);
	if(read(file, data, 1) != 1)
	{
		printf("Erorr : Input/output Erorr \n");
		exit(1);
	}
	data_0 = data[0];
	// Read 1 byte of data from address(0x3F)
	reg[0] = 0x3F;
	write(file, reg, 1);
	read(file, data, 1);
	data_1 = data[0];

	int T3 = (data_1 * 256) + data_0;
	
	// Read 4 bytes of data(0x28 | 0x80)
	// hum msb, hum lsb, temp msb, temp lsb
	reg[0] = 0x28 | 0x80;
	write(file, reg, 1);
	//char data[4] = {0};
	if(read(file, data, 4) != 4)
	{
		printf("Erorr : Input/output Erorr \n");
		exit(1);
	}
	else
	{
		// Convert the data
		int hum = (data[1] * 256) + data[0]; 
		int temp = (data[3] * 256) + data[2];
		if(temp > 32767)
		{
			temp -= 65536;
		}
		float humidity = ((1.0 * H1) - (1.0 * H0)) * (1.0 * hum - 1.0 * H2) / (1.0 * H3 - 1.0 * H2) + (1.0 * H0);
		float cTemp = ((T1 - T0) / 8.0) * (temp - T2) / (T3 - T2) + (T0 / 8.0);
		float fTemp = (cTemp * 1.8 ) + 32;
		
		// Output data to screen
		printf("Relative humidity : %.2f %%RH \n", humidity);
		printf("Temperature in Celsius : %.2f C \n", cTemp);
		printf("Temperature in Fahrenheit : %.2f F \n", fTemp);
	}
}
