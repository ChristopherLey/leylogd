//============================================================================
// Name        	: TMP102.h
// Author      	: Christopher Ley <christopher.ley@uon.edu.au>
// Version     	: 1.3.1
// Project	   	: leylogd
// Created     	: 04/03/15
// Modified    	: 04/03/15
// Copyright   	: Do not modify or distribute without express written permission
//				: of the author
// Description 	: TMP102 class definition file
// GitHub		: https://github.com/ChristopherLey/leylogd.git
//===========================================================================

#include "TMP102.h"
#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/ioctl.h>
#include <unistd.h>
using namespace std;

#define MAX_BUS 64
#define CONFIG_REGISTER 0x01
#define TEMP_REGISTER 0x00

TMP102::TMP102(I2C_BUS bus, TMP102_ADDR address,TMP102_CONFIG_MSB msb, TMP102_CONFIG_LSB lsb){
	// Constructor
	I2CBus = bus;
	I2CAddress = address;
	setConfigurationRegister(msb,lsb);
}

int TMP102::readTemperature(){
//	logMessage("Starting Temperature Read");
	// pointer to device address
	char namebuf[MAX_BUS];
	// overloaded safer formated string
	snprintf(namebuf,sizeof(namebuf),"/dev/i2c-%d",I2CBus);
	int file;
	if ((file = open(namebuf, O_RDWR)) < 0){
		logMessage("Failed to open TMP102 Sensor on %s ISC bus",namebuf);
		return(1);
	}
	if (ioctl(file, I2C_SLAVE, I2CAddress) < 0){
		logMessage("I2C_SALVE address %s failed",I2CAddress);
		return(2);
	}
	char buf[1] = {TEMP_REGISTER};
	if(write(file, buf, 1) != 1){
		logMessage("Failed to address Temperature register");
		return(3);
	}
	int bytesRead = read(file, this->dataBuffer, 2);
	if (bytesRead == -1){
		logMessage("Failure to read Byte Stream in readTemperature()");
	}
	else if (bytesRead != 2){
		logMessage("Incorrect read value in TMP102");
		return(4);
	}
	else{
//		logMessage("Number of bytes read was %d",bytesRead);
		logMessage("Raw Data (Hex): 0x%02x\t 0x%02x",this->dataBuffer[0],this->dataBuffer[1]);

		this->temperature = convertTemperature(this->dataBuffer[0],this->dataBuffer[1]);
		logMessage("Temperature %f degC", this->temperature);
	}

	close(file);
	return(0);
}

int TMP102::setConfigurationRegister(TMP102_CONFIG_MSB msb,TMP102_CONFIG_LSB lsb){
	// Write buffer
	char namebuf[MAX_BUS];
	snprintf(namebuf, sizeof(namebuf),"/dev/i2c-%d", I2CBus);
	int file;
	if ((file = open(namebuf, O_RDWR)) < 0){
		logMessage("Failed to open TMP102 Sensor on %s I2C Bus",namebuf);
		return(1);
	}
	if (ioctl(file, I2C_SLAVE, I2CAddress) < 0){
		logMessage("I2C_SALVE address %s failed",I2CAddress);
		return(2);
	}
	char buffer[3] = {CONFIG_REGISTER, msb, lsb};
	if (write(file, buffer, 3) != 3){
		logMessage("Failure to write TMP102 configuration register.");
	}
	close(file);
	logMessage("Finished TMP102 Configuration Resister Write");
	return(0);
}

float TMP102::convertTemperature(int msb, int lsb){
	// Conversion type
	short tempValue;
	if ((lsb & 0x01) == 0x01){ // 13bit
		if((msb & 0x80) == 0x80){ // negative 13 bit number (shortened 2s complement)
			tempValue = 0xe000 | (msb<<5) | (lsb>>3);
		}
		else{
			tempValue = (msb<<5) | (lsb>>3);
		}
	}
	else{ // 12bit
		if((msb & 0x80) == 0x80){ // negative 12 bit number (shortened 2s complement)
			tempValue = 0xf000 | (msb<<4) | (lsb>>4);
		}
		else{
			tempValue = (msb<<4) | (lsb>>4);
		}
	}
//	logMessage("int value of temp: %d",tempValue);
	return(0.0625*((float)tempValue));
}
TMP102::~TMP102(void){};//Destructor

