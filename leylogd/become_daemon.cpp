//============================================================================
// Name        	: become_daemon.cpp
// Author      	: Christopher Ley <christopher.ley@uon.edu.au>
// Version     	: 1.2
// Project	   	: leylogd
// Created     	: 24/02/15
// Modified    	: 04/03/15
// Copyright   	: Do not modify or distribute without express written permission
//				: of the author
// Description 	: Daemon process creation definition file
// GitHub		: https://github.com/ChristopherLey/leylogd.git
//===========================================================================

#include <sys/stat.h>
#include <fcntl.h>
#include "become_daemon.h"
#include <stdlib.h>
#include <unistd.h>

int becomeDaemon(int flags)
{
	int maxfd, fd;

	switch(fork()){		/*Become a background process */
	case -1: return -1;	/* Failure */
	case 0: break;		/* Child falls through... */
	default: _exit(EXIT_SUCCESS);
	/* no break */
	}

	if (setsid() == -1)	/* Become leader of new session */
		return -1;

	switch (fork()) {	/* Ensure we are not session leader */
	case -1: return -1;
	case 0: break;
	default: _exit(EXIT_SUCCESS);
	/* no break*/
	}

	if (!(flags & BD_NO_UMASK0))
		umask(0);		/* Clear file mode creation mask */

	if (!(flags & BD_NO_CHDIR))
		chdir("/");	/* Change to root directory */

	if(!(flags & BD_NO_CLOSE_FILES)) { /* Close all open files */
		maxfd = sysconf(_SC_OPEN_MAX);
		if(maxfd == -1)					/* Limit is indeterminate.. */
			maxfd = BD_MAX_CLOSE;		/* so take a guess */

		for (fd = 0; fd < maxfd; fd++)
			close(fd);
	}

	if (!(flags & BD_NO_REOPEN_STD_FDS)) {
		close(STDIN_FILENO);			/* Reopen standard fd's to /dev/null */

		fd = open("/dev/null", O_RDWR);
		if (fd != STDIN_FILENO)		/* 'fd' should be 0 */
			return -1;
		if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
			return -1;
	}

	return 0;
}
