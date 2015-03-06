//============================================================================
// Name        	: MPL3115A2_Altimeter.cpp
// Author      	: Christopher Ley <christopher.ley@uon.edu.au>
// Version     	: 1.3.3
// Project	   	: leylogd
// Created     	: 05/03/15
// Modified    	: 05/03/15
// Copyright   	: Do not modify or distribute without express written permission
//				: of the author
// Description 	: MPL3115A2_Altimeter class definition file
// GitHub		: https://github.com/ChristopherLey/leylogd.git
//=========================================================================

#include "MPL3115A2_Altimeter.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
using namespace std;

#define MAX_BUS 64

MPL3115A2_Altimeter::MPL3115A2_Altimeter(I2C_BUS bus,I2C_ADDR addr,STATE readtype){
	I2CBus = bus;
	I2CAddress = addr;
	/* Configure Sensor */
	char namebuf[MAX_BUS];
	snprintf(namebuf, sizeof(namebuf), "/dev/i2c-%d", I2CBus);
	int file;
	if ((file = open(namebuf, O_RDWR)) < 0){
		logMessage("Failed to open MPL115 Sensor on %s ISC bus",namebuf);
	}
    if (ioctl(file, I2C_SLAVE, I2CAddress) < 0){
    	logMessage("I2C_SALVE address %s failed [MPL115]",I2CAddress);
    }
    char config_buffer[2];
	config_buffer[0] = 0x26;
	config_buffer[1] = 0x00;
    if ( write(file, config_buffer, 2) != 2) {
		logMessage("MPL115: Failure to configure register 0x26");
	}
    config_buffer[0] = 0x13;
    config_buffer[1] = 0x07;
    if ( write(file, config_buffer, 2) != 2) {
    	logMessage("MPL115: Failure to configure register 0x13");
	}
	if(readtype){ //Altimeter
		this->readState = readtype;
	    config_buffer[0] = 0x26;
	    config_buffer[1] = 0x80;
	    if ( write(file, config_buffer, 2) != 2) {
	    	logMessage("MPL115: Failure to configure register 0x26");
		}
	}else { //Barometer
		this->readState = readtype;
	    config_buffer[0] = 0x26;
	    config_buffer[1] = 0x00;
	    if ( write(file, config_buffer, 2) != 2) {
	    	logMessage("MPL115: Failure to configure register 0x26");
		}
	}
	close(file);
	logMessage("Succesfully Configured MPL3115A2 (config: %02x->%02x,%02x->%02x)",config_buffer[0],config_buffer[1],0x13,0x07);
}

int MPL3115A2_Altimeter::readSensor(float *pressure,float *temp){
	// Standard I2C Interface
	char namebuf[MAX_BUS];
	snprintf(namebuf, sizeof(namebuf), "/dev/i2c-%d", I2CBus);
	int file;
	if ((file = open(namebuf, O_RDWR)) < 0){
		logMessage("Failed to open MPL115 Sensor on %s ISC bus",namebuf);
		return(-1);
	}
    if (ioctl(file, I2C_SLAVE, I2CAddress) < 0){
    	logMessage("I2C_SALVE address %s failed [MPL115]",I2CAddress);
		return(-1);
    }
    char config_buffer[2];
	if(readState){ //Altimeter
	    config_buffer[0] = 0x26;
	    config_buffer[1] = 0x82;
	    if ( write(file, config_buffer, 2) != 2) {
	    	logMessage("MPL115: Failure to configure register 0x26");
			return(-1);
		}
	}else { //Barometer
	    config_buffer[0] = 0x26;
	    config_buffer[1] = 0x02;
	    if ( write(file, config_buffer, 2) != 2) {
	    	logMessage("MPL115: Failure to configure register 0x26");
			return(-1);
		}
	}
	config_buffer[0] = 0x00;
	char test = 0x00;
	int timeout = 0;
	while(!(test & 0x08)){
		if(write(file, config_buffer, 1) != 1){
			logMessage("MPL115:Failed to write status byte");
			return(-1);
		}
		int bytesRead = read(file, &test, 1);
		if (bytesRead == -1){
			logMessage("MPL115:Failed to read status byte");
		}
//		if(!(test & 0x08)){
//			logMessage("Status is not ready = 0x%02x",test);
//		}
		timeout++;
		if(timeout > 30){
			logMessage("MPL115 Error(count= %d, status: %02x): Timeout!",timeout,test);
			return(-1);
		}
	}
	char databuffer[6];
	int databytesRead = read(file, databuffer, 6);
	if (databytesRead == -1){
		logMessage("Failure to read data bytes!!");
	}
//	for(int i = 0;i<6;i++){
//		logMessage("Byte %#04x,Hex:0x%02x,Dec:%d",i,databuffer[i],databuffer[i]);
//	}
	if(readState) { //Altimeter
		*pressure = ((databuffer[1]<<16) | (databuffer[2]<<8) | (databuffer[3]))/(float)(1<<8);
	} else { //Barometer
		*pressure = ((databuffer[1]<<16) | (databuffer[2]<<8) | (databuffer[3]))/(float)(1<<6);
	}
	*temp = ((databuffer[4]<<8) | (databuffer[5]))/(float)(1<<8);
//	logMessage("Bar Pressure = %f Pa ",*pressure);
//	logMessage("MPL Temperature = %f degC",*temp);
	close(file);
//	logMessage("Finished writing the pressure sensor");
	return(0);

}
MPL3115A2_Altimeter::~MPL3115A2_Altimeter(void){};//Destructor
