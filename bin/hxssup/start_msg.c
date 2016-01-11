
/* @(#)57	1.2.4.2  src/htx/usr/lpp/htx/bin/hxssup/start_msg.c, htx_sup, htxubuntu 1/4/16 05:14:38 */

#include "hxssup.h"

#ifdef	__HTX_LINUX__
#define	SIGMAX	(SIGRTMAX)
#endif

#include <pthread.h>
#include "nfdef.h"
/*
 * start_msg() error code definitions
 */
#define BAD_FORK	(1)
#define BAD_EXEC	(2)

/*
 * NAME: start_msg_hdl()
 *
 * FUNCTION: Starts the HTX Message Handler Program, "hxsmsg", via the
 *           fork() and exec() system calls.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This function is called by the htx_profile() function (found in the
 *      hxssup.c module) of the HTX supervisor program, "hxssup".
 *
 * NOTES:
 *
 *      operation:
 *      ---------
 *
 *
 *
 * RETURNS:
 *
 *      Return Codes:
 *      ------------------------------------------------------------------
 *       0 -- Normal exit.
 *       1 -- Error in fork() system call.
 *       2 -- Error in exec() system call (passed via child process's
 *            exit() system call).
 *
 */

short	start_msg_hdl(int autostart)
{
	char auto_start_flag[4]; /* startup type (regular/auto)         */
	char workstr[128];	/* work string                         */

	extern char ERR_WRAP[];	/* htxerr file wrap keyword.           */
	extern char HTXPATH[];	/* HTX file system path spec           */
	extern char MAX_ERR_SAVE[]; /* htxerr.save max filesize            */
	extern char MAX_ERR_THRES[]; /* err log file wrap threshold.        */
	extern char MAX_MSG_SAVE[]; /* htxmsg.save max filesize            */
	extern char MAX_MSG_THRES[]; /* msg log file wrap threshold.        */
	extern char MSG_ARCHIVE[];          /* message archival mode.              */
	extern char MSG_WRAP[];	/* htxmsg file wrap keyword.           */
	extern char stress_cycle[]; /* # sec between "heartbeats"          */
	extern char stress_dev[]; /* stress "heartbeat" device           */

	extern int hxsmsg_PID;	/* message handler process id          */
/*	extern int msgqid;	 Message Log message queue id        */

/*	extern union shm_pointers shm_addr; shared memory address   */

	extern int wof_test_enabled;

	int errno_save;		/* errno save variable                 */
	int exec_rc;		/* exec() return code                  */
	int frkpid;		/* PID from fork()                     */
	int i;			/* loop counter                        */

	short return_code;	/* The function's return code          */

	struct sigaction sigvector;	/* structure for signals */

	pthread_t thd;

	errno = 0;

	return_code = 0;	/* Set a good return code.             */

	pthread_create(&thd, NULL, start_display,0);

	frkpid = fork();

	switch (frkpid) {
		case 0:		/* child process */
			sigemptyset(&(sigvector.sa_mask));	/* do not block other signals      */
			sigvector.sa_flags = 0;	/* no special processing */
			sigvector.sa_handler = SIG_DFL;	/* default action on signal */
			for (i = 1; i <= SIGMAX; i++)  {	/* set default signal processing */
				sigaction(i, &sigvector, (struct sigaction *) NULL);
			}

       			setpgrp();	/* change the process group so we don't
					   get signals meant for the supervisor
					   program */

       			htx_strcpy(workstr, HTXPATH);
       			htx_strcat(workstr, "/bin/hxsmsg");

			if (autostart == 0)  {
       				htx_strcpy(auto_start_flag, "no");
			}

			else  {
       				htx_strcpy(auto_start_flag, "yes");
			}

			errno = 0;
			exec_rc = execl(workstr, "hxsmsg", MAX_ERR_THRES, MAX_MSG_THRES, MAX_ERR_SAVE, MAX_MSG_SAVE, ERR_WRAP, MSG_WRAP, MSG_ARCHIVE, auto_start_flag, stress_dev, stress_cycle, (char *) 0);

			/*
			 * If we're here, the execl() system call failed!!!
			 */
			errno_save = errno;
			PRTMSG(MSGLINE, 0, ("Unable to load message handler.  errno = %d", errno_save));
       			getch();
			exit(BAD_EXEC);

		case -1:
			errno_save = errno;
			PRTMSG(MSGLINE, 0, ("Unable to fork for message handler.  errno = %d", errno));
       			getch();
			return_code = BAD_FORK;
			break;

		default:
			hxsmsg_PID = frkpid;
       		if (wof_test_enabled) { /* bind to any thread of core 0 */
       		    do_the_bind_proc(hxsmsg_PID);
       		}
       		sleep((unsigned int) 2);	/* give the hxsmsg child process a chance to start */
			break;
	}			/* endswitch */

	return (return_code);
}				/* start_msg_hdl() */


