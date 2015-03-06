//============================================================================
// Name        	: MPL3115A2_Altimeter.h
// Author      	: Christopher Ley <christopher.ley@uon.edu.au>
// Version     	: 1.3.3
// Project	   	: leylogd
// Created     	: 05/03/15
// Modified    	: 05/03/15
// Copyright   	: Do not modify or distribute without express written permission
//				: of the author
// Description 	: MPL3115A2_Altimeter header file
// GitHub		: https://github.com/ChristopherLey/leylogd.git
//=========================================================================

#ifndef MPL3115A2_ALTIMETER_H_
#define MPL3115A2_ALTIMETER_H_

#include "I2C_interface.h"

#define MPL3115A2_I2C_BUFFER 0x80

enum ALTIMETER_REG_ADDR {
	STATUS =		0x00,
	OUT_P_MSB =		0x01,
	OUT_P_CSB =		0x02,
	OUT_P_LSB =		0x03,
	OUT_T_MSB = 	0x04,
	OUT_T_LSB = 	0x05,
	DR_STATUS =		0x06,
	OUT_P_DELTA_MSB = 0x07,
	OUT_P_DELTA_CSB = 0x08,
	OUT_P_DELTA_LSB = 0x09,
	OUT_T_DELTA_MSB = 0x0a,
	OUT_T_DELTA_LSB = 0x0b,
	WHO_AM_I =		0x0c,
	F_STATUS = 		0x0d,
	F_DATA =		0x0e,
	F_SETUP = 		0x0f,
	TIME_DLY =		0x10,
	SYSMOD =		0x11,
	INT_SOURCE = 	0x12,
	PT_DATA_CFG =	0x13,
	BAR_IN_MSB =	0x14,
	BAR_IN_LSB = 	0x15,
	P_TGT_MSB =		0x16,
	P_TGT_LSB =		0x17,
	T_TGT =			0x18,
	P_WND_MSB =		0x19,
	P_WND_LSB =		0x1A,
	T_WND =			0x1b,
	P_MIN_MSB =		0x1c,
	P_MIN_CSB =		0x1d,
	P_MIN_LSB =		0x1e,
	T_MIN_MSB = 	0x1f,
	T_MIN_LSB =		0x20,
	P_MAX_MSB = 	0x21,
	P_MAX_CSB =		0x22,
	P_MAX_LSB = 	0x23,
	T_MAX_MSB =		0x24,
	T_MAX_LSB =		0x25,
	CTRL_REG1 =		0x26, //Main CTRL Register
	CTRL_REG2 =		0x27, //Delay Sampling & Alarm Register
	CTRL_REG3 =		0x28, //Interrupt CTRL Register
	CTRL_REG4 =		0x29, //Interrupt Enable Register
	CTRL_REG5 =		0x2a, //Interrupt Configuration Register
	OFF_P =			0x2b,
	OFF_T =			0x2c,
	OFF_H =			0x2d
};
enum CTRL_REG1_FLAGS { // Default 0x00
	SBYB = 	0x01,
	OST =	0x02,
	RST = 	0x04,
	OS0 = 	0x08,
	OS1 = 	0x10,
	OS2 = 	0x20,
	RAW = 	0x40,
	ALT = 	0x80
};
enum I2C_ADDR {
	Standard = 0x60
};

enum STATE {
	Barometer = 0x00,
	Altimeter = 0x01
};

extern void logMessage(const char *format,...); //error reporting

class MPL3115A2_Altimeter {
private:
	char I2CAddress;
	int I2CBus;
	char dataBuffer[MPL3115A2_I2C_BUFFER];
	char CtrlRegState;
	STATE readState;
public:
	//Constructor
	MPL3115A2_Altimeter(I2C_BUS bus,I2C_ADDR addr,STATE readtype);
	//Destructor
	virtual ~MPL3115A2_Altimeter();
	//Interface Functions
	int readSensor(float *pressure,float *temp);

};


#endif /* MPL3115A2_ALTIMETER_H_ */
