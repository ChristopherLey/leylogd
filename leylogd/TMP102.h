//============================================================================
// Name        	: TMP102.h
// Author      	: Christopher Ley <christopher.ley@uon.edu.au>
// Version     	: 1.3.1
// Project	   	: leylogd
// Created     	: 04/03/15
// Modified    	: 04/03/15
// Copyright   	: Do not modify or distribute without express written permission
//				: of the author
// Description 	: TMP102 header file
// GitHub		: https://github.com/ChristopherLey/leylogd.git
//===========================================================================
#ifndef TMP102_H_
#define TMP102_H_

#define TMP102_I2C_BUFFER 0x02
#include "I2C_interface.h"

enum TMP102_CONFIG_LSB {
	CR_025Hz_12bit 	= 0x20,
	CR_025Hz_13bit 	= 0x30,
	CR_1Hz_12bit 	= 0x60,
	CR_1Hz_13bit 	= 0x70,
	CR_4Hz_12bit 	= 0xa0,
	CR_4Hz_13bit 	= 0xb0,
	CR_8Hz_12bit 	= 0xe0,
	CR_8Hz_13bit 	= 0xf0
};

enum TMP102_CONFIG_MSB {
	Default_MSB = 0x60
};

enum TMP102_ADDR {
	Ground 	= 0x48,
	V_plus 	= 0x49,
	SDA 	= 0x4a,
	SCL		= 0x4b
};

extern void logMessage(const char *format,...); //error reporting

class TMP102 {

private:
	char I2CAddress;
	int I2CBus;
	char dataBuffer[TMP102_I2C_BUFFER];
	float temperature; // accurate to 0.0625 degC

	float convertTemperature(int msb, int lsb);
public:
	// Constructor
	TMP102(I2C_BUS bus, TMP102_ADDR address,TMP102_CONFIG_MSB msb, TMP102_CONFIG_LSB lsb);
	int setConfigurationRegister(TMP102_CONFIG_MSB msb,TMP102_CONFIG_LSB lsb);
	// Interface Functions
	float readTemperature();

	virtual ~TMP102(); // Destructor
};

#endif /* TMP102_H_ */
