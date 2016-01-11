
/* @(#)25	1.6  src/htx/usr/lpp/htx/bin/hxsmsg/set_sig.c, htx_msg, htxubuntu 5/24/04 18:19:17 */

/*
 *   FUNCTIONS: set_signal_hdl
 */


#include "hxsmsg.h"

#ifdef __HTX_LINUX__
#define SIGMAX 32
#endif /* __HTX_LINUX__ */

/*
 * set_signal_hdl() error code definitions
 */
#define BAD_SIGNAL 1
#define BAD_SIGACTION 2


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

short set_signal_hdl(int signal_number,
		     void (*signal_hdl)(int, int, struct sigcontext *))

     /*
      * signal_number -- the number of the specified signal
      * signal_hdl -- the function of the signal handler
      */

{
  /*
   ***  Data and Functions Definitions/Declarations  **************************
   */
  	char error_msg[128];       /* error message string                    */

  	extern char *program_name; /* this program's name (argv[0])           */

  	int errno_save;            /* errno save area                         */

  	short exit_code;           /* exit program return code                */

  	struct sigaction sigvector;  /* structure for signal specifications   */

  /*
   ***  Beginning of Executable Code  *****************************************
   */
  	exit_code = GOOD;

  	if ((signal_number < 1) || (signal_number > SIGMAX))
    	{
      		(void) sprintf(error_msg,
		"%s -- Invalid signal (%d) passed to \
		set_signal_hdl().\n",
		program_name,
		signal_number);

      		(void) send_message(error_msg, 0, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
      		exit_code = BAD_SIGNAL;
    	}
  	else
    	{
      		sigvector.sa_handler = (void (*)(int)) signal_hdl;
                                         /* set signal handler function     */
      		sigemptyset(&(sigvector.sa_mask));  
					 /* do not block other signals      */
#ifndef __OS400__
	 	sigvector.sa_flags = SA_RESTART;    
#endif
					 /* restart system calls on signal  */

      		errno = 0;
      		if (sigaction(signal_number, &sigvector, (struct sigaction *) NULL)
	  	!= GOOD)
		{
	  		errno_save = errno;

	  		(void) sprintf(error_msg,
			"%s -- Error with sigaction() system call for %d \
			signal.\nerrno: %d (%s).\n",
			program_name,
			signal_number,
			errno_save,
			strerror(errno_save));

	  		(void) send_message(error_msg, errno_save, HTX_SYS_HARD_ERROR,
			HTX_SYS_MSG);
	  		exit_code = BAD_SIGACTION;
		} /* endif */
    	} /* endif */

  	return(exit_code);

} /* set_signal_hdl() */
