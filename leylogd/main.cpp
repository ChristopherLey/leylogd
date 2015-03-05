//============================================================================
// Name       	: main.cpp
// Author      	: Christopher Ley <christopher.ley@uon.edu.au>
// Version     	: 1.3.3
// Project	   	: leylogd
// Created     	: 24/02/15
// Modified    	: 05/03/15
// Copyright   	: Do not modify or distribute without express written permission
//				: of the author
// Description 	: main file for leylogd daemon process ARM variant
// Notes	   	: leylogd responds to SIGHUP for timer adjustment, this means
//				:  in order to change sampling time you need to (in this order):
//				- 	echo "sec: <int seconds>, usec <int microseconds> "
//				                      > /etc/leylogd/leyld.conf
//				- 	kill -s 1 <PID of leylogd>
// *WARNING: leyld.conf needs to receive EXACTLY that form of argument
// *NOTE: <int second> is a integer second value i.e. 30 as is
// *NOTE: <int microseconds> is a integer micro-second value i.e. 30
//
//				: Version 1.2.x  stable;
//				- all init.d handlers and interrupts [stable v1.2]
//				- timer handlers, with SIGHUP reload configuration [stable v1.2]
//				: Version 1.3.x latest development;
//				- TMP102 data logging [stable v1.3.0]
//				- MPL3115A2 data logging [stable v1.3.3]
//				- independent logging file "/var/log/leyld.csv" [stable v1.3.1]
//
// GitHub		: https://github.com/ChristopherLey/leylogd.git
//============================================================================
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "become_daemon.h"
#include "TMP102.h"
#include "MPL3115A2_Altimeter.h"

/**************************** LOGGING FUNCTIONS  **************/
/****** Static File Pointers ******/
static FILE *logfp;		/* Log file stream */
static FILE *datafp;    /* Data file stream */
static const char *LOG_FILE = "/var/log/leyld.log";
static const char *DATA_FILE = "var/log/leyld.csv";
static const char *CONFIG_FILE = "/etc/leylogd/leyld.conf";

/****** Message Loggers ******/
/* Log Message */
void logMessage(const char *format,...)
{
	va_list argList;
	const char *TIMESTAMP_FMT = "%F %X";	/* = YYYY-MM-DD HH:MM:SS */
#define TS_BUF_SIZE sizeof("YYYY-MM-DD HH:MM:SS")	/* Includes '\0' */
	char timestamp[TS_BUF_SIZE];
	time_t t;
	struct tm *loc;

	t = time(NULL);
	loc = localtime(&t);
	if (loc == NULL || strftime(timestamp, TS_BUF_SIZE, TIMESTAMP_FMT, loc) == 0)
		fprintf(logfp, "??Unknown time??: ");
	else
		fprintf(logfp, "%s: ", timestamp);

	va_start(argList, format); /* stdarg.h macro */
	vfprintf(logfp, format, argList);
	fprintf(logfp, "\n");
	va_end(argList);
}
void dataLog(const char *format,...)
{
	/* stdarg.h macro */
	va_list argList;
	/*Timing*/
	static struct timeval start;
	struct timeval curr;
	static int initial = 0; //Number of calls to this function
	float time_precise = 0.0;

	if (initial == 0){
		//Initialise with header and timer
		if (gettimeofday(&start, NULL) == -1){
			logMessage("Error: gettimeofday; CallNum == 0");
		} else {
			logMessage("Data logging timer started");
			initial = 1;
			// dataLog expects a Header on first access
			va_start(argList, format); /* stdarg.h macro */
			vfprintf(datafp, format, argList);
			fprintf(datafp, "\n");
			va_end(argList);
		}
	}else {
		// Normal function
		if(gettimeofday(&curr, NULL) == -1){
			logMessage("Data logging timer failure!");
		} else {
			time_precise = curr.tv_sec - start.tv_sec + (curr.tv_usec - start.tv_usec)/1000000.0;
		}
		//print to datafile
		fprintf(datafp,"%f,",time_precise);
		va_start(argList, format); /* stdarg.h macro */
		vfprintf(datafp, format, argList);
		fprintf(datafp, "\n");
		va_end(argList);
	}
}
/* Open Log file */
static void logOpen(const char *logFilename, const char *dataFilename)
{
	mode_t m; /*mode of file*/

	m = umask(077); /* File mode creation mask */
	logfp = fopen(logFilename, "a");
	datafp = fopen(dataFilename, "a");
	umask(m);

	if(logfp == NULL || datafp == NULL){
		exit(EXIT_FAILURE);
	}
	setbuf(logfp, NULL); /* Disable stdio buffering */
	setbuf(datafp, NULL); /* Disable stdio buffering */

//	logMessage("Opened log file");
}
/* Close Log file */
static void logClose(void)
{
	logMessage("Closing log and data file");
	fclose(logfp);
	fclose(datafp);
}
/**************************************************************/

/**************** CONFIGURATION HANDLERS **********************/
static void readConfigFile(const char *configFilename, int *config)
{
	FILE *configfp;
#define SBUF_SIZE 100
	char str[SBUF_SIZE];

	configfp = fopen(configFilename, "r");
	if(configfp != NULL && fgets(str, SBUF_SIZE, configfp) != NULL) {	/* Ignore nonexistent file */
		sscanf(str,"%*s %d%*c %*s %d",&config[0],&config[1]);
		logMessage("Read config file: %d, %d", config[0],config[1]);
		fclose(configfp);
	} else {
		logMessage("Couldn't open and/or read configuration file");
		//Defaults
		config[0] = 30;
		config[1] = 1;
	}
}
/**************************************************************/

/************************ TIMER HANDLER ***********************/
static int setTimer(struct itimerval *itv, int *config)
{
	itv->it_value.tv_sec = config[0];
	itv->it_value.tv_usec = config[1];
	itv->it_interval.tv_sec = config[0];
	itv->it_interval.tv_usec = config[1];
	if (setitimer(ITIMER_REAL, itv, 0) == -1){
		return -1;
	} else {
		return 0;
	}
}
/**************************************************************/

/**************************** INTERRUPT HANDLERS **************/
/****** Atomic interrupt flags ******/
/* Set nonzero on receipt of interrupt,set as volatile so that the compiler
 * dosen't store as a register value and helps with re-entrancy issues, the
 * atomic identifier ensures the global flag is atomic,i.e. can be changed
 * in one clock cycle! */
static volatile sig_atomic_t termReceived = 0;
static volatile sig_atomic_t alrmReceived = 0;
static volatile sig_atomic_t hupReceived = 0;

/****** Interrupt Handler Function ******/
static void interruptHandler(int sig)
{
    /* A Quick interrupt handler to try to avoid re-entrancy */
    switch(sig)
    {
        case SIGHUP:
            hupReceived = 1;
            break;
        case SIGINT:
        case SIGTERM:
            termReceived = 1;
            break;
        case SIGALRM:
            alrmReceived = 1;
            break;
    }
}
/**************************************************************/

/**************************** MAIN ****************************/
int main(int argc, char *argv[])
{
/* Set up interrupt handler */
	struct sigaction act; // defined by signal.h
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	act.sa_handler = interruptHandler;
	/* signals to handle */
	sigaction(SIGHUP, &act, NULL); // catch hangup signal
	sigaction(SIGTERM, &act, NULL); // catch terminate (--stop|stop) signal
	sigaction(SIGINT, &act, NULL); // other terminate signal
	sigaction(SIGALRM, &act, NULL);// catch timer alarm

/* Set up Daemon Process */
	if(becomeDaemon(0) == -1){
		exit(EXIT_FAILURE);
	}

/* Open Log file */
	int config[2];
	logOpen(LOG_FILE,DATA_FILE);
	readConfigFile(CONFIG_FILE,config);
	int count;
	if (argc > 1){
		for(count = 1; count < argc; count++){
			 logMessage(argv[count]);
		}
	}

/* Set up Timers */
	struct itimerval itv;
	/* Set timer values*/
	if(setTimer(&itv,config) == -1){
		logMessage("Fatal Timer error!");
		exit(EXIT_FAILURE);
	}
	dataLog("Time,Temperature_TMP102,Pressure_MPL,Temperature_MPL"); // Write header to data csv file;

/* Initialise TMP102 Sensor */
	TMP102 TempSensor1(I2C1, Ground, Default_MSB, CR_8Hz_13bit);
/* Initialise MPL3115A2 Sensor */
	MPL3115A2_Altimeter altimeter(I2C1,Standard);

	/* Final Message b4 loop*/
	logMessage("Initialised");
	float temp_tmp102, temp_mpl, pressure_mpl;

	for(;;){ /*ever*/
		if(termReceived != 0){
			/* Close Program [SIGTERM || SIGINT] */
			termReceived = 0;
			logClose();
			exit(EXIT_SUCCESS);
		}else if(alrmReceived != 0){
			/* Data Logging [SIGALRM]*/
			temp_tmp102 = TempSensor1.readTemperature(); // TODO Change to pointer input;
			altimeter.readSensor(Barometer,&pressure_mpl,&temp_mpl);
			dataLog("%f,%f,%f",temp_tmp102,pressure_mpl,temp_mpl);
			alrmReceived = 0;
		}else if(hupReceived != 0){
			/* Re-initialise parameters [SIGHUP] */
			logMessage("Hang-up Received");
			readConfigFile(CONFIG_FILE,config);
			// Reinitialise Parameters
			if(setTimer(&itv,config) == -1){
				logMessage("Fatal Timer error!");
				exit(EXIT_FAILURE);
			}
			hupReceived = 0;
		}else{
			pause(); /* suspend until a signal is received.*/
		}
	}
	exit(EXIT_SUCCESS);
}
