//============================================================================
// Name        	: become_daemon.h
// Author      	: Christopher Ley <christopher.ley@uon.edu.au>
// Version     	: 1.2
// Project	   	: leylogd
// Created     	: 24/02/15
// Modified    	: 04/03/15
// Copyright   	: Do not modify or distribute without express written permission
//				: of the author
// Description 	: Daemon process creation definition header file
// GitHub		: https://github.com/ChristopherLey/leylogd.git
//===========================================================================
#ifndef BECOME_DAEMON_H	/* Prevent double inclusion */
#define BECOME_DAEMON_H


/* Bit-mask values for 'flags' argument of becomeDaemon() */
#define BD_NO_CHDIR				01		/* Don't chdir("/") */
#define BD_NO_CLOSE_FILES		02		/* Don't close all open files */
#define BD_NO_REOPEN_STD_FDS	04		/* Don't reopen stdin, stdout, and stderr to /dev/null */
#define BD_NO_UMASK0			010		/* Don't do a umask(0) */
#define BD_MAX_CLOSE			8192	/* Maximum file descriptors to close if sysconf(_SC_OPEN_MAX) is indeterminate */

int becomeDaemon(int flags);

#endif
