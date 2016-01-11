
/* @(#)41	1.5  src/htx/usr/lpp/htx/bin/hxstats/set_sig.c, htx_stats, htxubuntu 5/24/04 18:21:53 */


#include "hxstats.h"

/*
 * set_signal_hdl() error code definitions
 */
#define BAD_SIGNAL	(1)
#define BAD_SIGACTION	(2)


#ifdef	__HTX_LINUX__
#define	SIGMAX	(SIGRTMAX)
#endif

/*
 * NAME: set_signal_hdl()
 *                                                                    
 * FUNCTION: Sets up a signal handler for the specified signal.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function is called by the main() and SIGCHLD() functions
 *      of the "hxsmsg" program.  The message handler program, "hxsmsg",
 *      is always a child process of the HTX supervisor program, "hxssup".
 *                                                                   
 * NOTES: 
 *
 *      operation:
 *      ----------
 *      Check signal number
 *      setup sigaction struct
 *      call sigaction() system call
 *
 *               
 * RETURNS: 
 *
 *      Return Codes:
 *      ------------------------------------------------------------------
 *       0 -- Normal exit.
 *       1 -- Invalid signal number specified.
 *       2 -- Error from sigaction() system call.
 *
 *
 */  

/*
 * signal_number -- the number of the specified signal
 * signal_hdl -- the function of the signal handler
 */

short set_signal_hdl(int signal_number, void (*signal_hdl)(int, int, struct sigcontext *))
{
	char	error_msg[128];		/* error message string */
	extern char *program_name;	/* this program's name (argv[0]) */
	int	errno_save;		/* errno save area */
	short	exit_code;		/* exit program return code */
	struct sigaction sigvector;	/* structure for signal specifications */

	exit_code = GOOD;

	if ((signal_number < 1) || (signal_number > SIGMAX))  {
		sprintf(error_msg, "%s -- Invalid signal (%d) passed to set_signal_hdl().\n", program_name, signal_number);
		send_message(error_msg, 0, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
		exit_code = BAD_SIGNAL;
	}

	else  {
		/*
		 * set signal handler function
		 */

		sigvector.sa_handler = (void (*)(int)) signal_hdl;

		/*
		 * do not block other signals
		 */
		sigemptyset(&(sigvector.sa_mask));

		/*
		 * do not restart system calls on signal
		 */
		sigvector.sa_flags = 0;

		errno = 0;
		if(sigaction(signal_number, &sigvector, (struct sigaction *) NULL)!= GOOD)  {
			errno_save = errno;
			sprintf(error_msg, "%s -- Error with sigaction() system call for %d signal.\nerrno: %d (%s).\n", program_name, signal_number, errno_save, strerror(errno_save));
			send_message(error_msg, errno_save, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
			exit_code = BAD_SIGACTION;
		} /* endif */
	} /* endif */

	return(exit_code);
} /* set_signal_hdl() */

