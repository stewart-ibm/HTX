
/* @(#)22	1.6  src/htx/usr/lpp/htx/bin/hxsmsg/rm_file.c, htx_msg, htxubuntu 5/24/04 18:18:19 */

/*
 *   FUNCTIONS: rm_file
 */


#include "hxsmsg.h"

#if defined(__HTX_LINUX__) || defined(__OS400__)
#include<unistd.h>
#define  E_ACC		F_OK
#define  W_ACC		W_OK
#else
#include <sys/access.h>
#endif /* __HTX_LINUX__ */

/*
 * Error code #define's for rm_file()
 */
#define BAD_RM 1
#define BAD_ACCESS 2

/*
 * NAME: rm_file()
 *                                                                    
 * FUNCTION: Removes a file
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function is called by the open_logs() and check_logs() functions
 *      of the "hxsmsg" program which is a child process of the HTX supervisor
 *      program, "hxssup". 
 *
 * NOTES: 
 *
 *      operation:
 *      ---------
 *
 *      if the file exists and I have write permission
 *        remove the file.
 *
 *      return
 * 
 *               
 * RETURNS: 
 *
 *      Return Codes:
 *      -------------
 *         0 -- Normal exit.  No errors.
 *         1 -- Unable to remove file.
 *         2 -- access error.
 *
 *
 */  

short rm_file(char *filename)
     /*
      * filename -- name of file to be removed
      */

{
  /*
   ***  Data and Functions Definitions/Declarations  *******************
   */
  	char error_msg[128];       /* string for error messages              */

  	extern char *program_name; /* this program's name (argv[0])          */

  	int access_mode;           /* type of access                         */
  	int errno_save;            /* errno save area                        */

  	short exit_code;           /* function exit code                     */

  /*
   ***  Beginning of Executable Code  **********************************
   */
  	exit_code = GOOD;          /* set good exit code                     */
  	access_mode = W_ACC | E_ACC;
				/* file exists and I have write permission   */

  	errno = 0;
  	if (access(filename, access_mode) == GOOD)
    	{
      		errno = 0;
      		if (remove(filename) != GOOD)
		{
	  		errno_save = errno;

	  		(void) sprintf(error_msg,
			"\n%s -- Error: Unable to remove %s.\n\
			errno: %d (%s).\n",
			program_name,
			filename,
			errno_save,
			strerror(errno_save));

	  		(void) fprintf(stderr, "%s", error_msg);
	  		(void) fflush(stderr);
	  		exit_code = BAD_RM;
		} /* endif */
    	}
  	else
    	{
      		if (errno != ENOENT)
		{
	  		errno_save = errno;

	  		(void) sprintf(error_msg, 
			"\n%s -- Error: Unable to access %s.\n\
			errno: %d (%s).\n",
			program_name,
			filename,
			errno_save,
			strerror(errno_save));

	  		(void) fprintf(stderr, "%s", error_msg);
	  		(void) fflush(stderr);
	  		exit_code = BAD_ACCESS;
		} /* endif */
    	} /* endif */
    
  	return(exit_code);

} /* rm_file() */
