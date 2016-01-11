
/* @(#)11	1.5  src/htx/usr/lpp/htx/bin/hxsmsg/cls_file.c, htx_msg, htxubuntu 5/24/04 18:14:32 */

/*
 *   FUNCTIONS: close_file
 */


#include <htx_local.h>

#ifdef __HTX_LINUX__
#include <unistd.h>
#endif /* __HTX_LINUX__ */


/*
 * close_file() error code definitions
 */
#define BAD_FILEID 1
#define BAD_CLOSE 2


/*
 * NAME: close_file()
 *                                                                    
 * FUNCTION: Closes the specified file and checks for errors.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function is called by the clean_up() and check_wrap functions
 *      of the "hxsmsg" program.
 *
 *      The message handler program, "hxsmsg", is always a child process
 *      of the HTX supervisor program, "hxssup".
 *
 * NOTES: 
 *
 *      operation:
 *      ---------
 *
 *      check for valid fileid
 *      close file
 *
 *      return
 * 
 *               
 * RETURNS: 
 *
 *      Return Codes:
 *      -------------
 *      0 -- Normal exit.
 *      1 -- Bad fileid parameter.
 *      2 -- Error on close() of file.
 *
 *
 */  

short close_file(int *fileid_ptr, char *filename)
     /*
      * fileid_ptr -- pointer to a file descriptor
      * filename -- name of the file
      */

{
  /*
   ***  Data and Functions Definitions/Declarations  **************************
   */
  	char error_msg[128];       /* error message string                    */

  	extern char *program_name; /* this program's name (argv[0])           */

  	int errno_save;            /* errno save area                         */

  	short exit_code;           /* exit program return code                */

  /*
   ***  Beginning of Executable Code  *****************************************
   */
  	exit_code = GOOD;

  	if (*fileid_ptr < 0)
    	{
      		(void) sprintf(error_msg, 
		"/n%s -- Error in close_file().\n\
		Invalid file id (%d) for %s.\n",
		program_name, 
		*fileid_ptr, 
		filename);

      		(void) fprintf(stderr, "%s", error_msg);
      		(void) fflush(stderr);
      		exit_code = BAD_FILEID;
    	}
  	else
    	{
      		errno = 0;
      		if (close(*fileid_ptr) != GOOD)
		{
	  		errno_save = errno;

	  		(void) sprintf(error_msg,
			"/n%s -- Error close() for %s.\nerrno: %d (%s).\n",
			program_name, 
			filename,
			errno_save,
			strerror(errno_save));

	  		(void) fprintf(stderr, "%s", error_msg);
	  		(void) fflush(stderr);
	  		exit_code = BAD_CLOSE;
		} /* endif */
      		*fileid_ptr = -1;
    	} /* endif */

  	return(exit_code);

} /* close_file() */

