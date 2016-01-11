
/* @(#)17	1.8  src/htx/usr/lpp/htx/bin/hxsmsg/max_filesz.c, htx_msg, htxubuntu 11/6/05 23:46:40 */

/*
 *   FUNCTIONS: get_max_filesize
 */



#include <ulimit.h>
#include <sys/param.h>
#include "hxsmsg.h"

/*
 * Error code #define's for get_max_filesize()
 */
#define BAD_ULIMIT (off_t) -1

#if defined(__HTX_LINUX__) 
#define GET_FSIZE UL_GETFSIZE
#define UBSIZE 512 
#elif defined(__OS400__)
#define GET_FSIZE 1
#define UBSIZE 512
#endif /* __HTX_LINUX__ */


/*
 * NAME: get_max_filesize()
 *                                                                    
 * FUNCTION: Gets the maximum filesize for the process running this program.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function is called by the do_argv() function of the "hxsmsg"
 *      program which is a child process to the HTX supervisor program,
 *      "hxssup".
 *                                                                   
 * NOTES: 
 *
 *      The get_max_filesize function gets the maximum file size that the 
 *      current process is allowed.  This value is obtained with the "ulimit"
 *      system subroutine call.
 * 
 *      If an error occurs obtaining the maximum file size value, a -1 is 
 *      returned to the calling function; otherwise, the maximum file size
 *      value is returned.
 *
 * RETURNS:
 *
 *      Returned Values:
 *      ----------------
 *         -1 -- Error obtaining maximum file size of the current process.
 *        n>0 -- The current processes maximum allowed file size.
 *
 */

off_t get_max_filesize()
     /*
      * no parameters
      */

{
  /*
   ***  Local Data Definitions...  ********************************************
   */

  	char error_msg[128];       /* error message : string                  */

  	extern char *program_name; /* this program's name                     */

 	int errno_save;            /* errno save area                         */

 	off_t max_file_size;       /* maximum file size allowed to process    */
  /*
   ***  Beginning of Executable Code...  **************************************
   */
  	errno = 0;
#if defined(__OS400__)
        max_file_size = ulimit(GET_FSIZE,5120*512);
	if ( 0 == max_file_size ) max_file_size = 5120*512;
#else
  	max_file_size = ulimit(GET_FSIZE, (off_t) 0); 
#endif 

  	if (max_file_size <= -1) 
    	{																				
      		errno_save = errno;

      		(void) sprintf(error_msg, 
		"\n%s -- Unexpected error from ulimit(GET_FSIZE).\n\
		errno: %d (%s).\n",
		program_name,
		errno_save,
		strerror(errno_save));

      		(void) fprintf(stderr, "%s", error_msg);
      		(void) fflush(stderr);
      		max_file_size = BAD_ULIMIT;
    	} else {
#if !defined(__HTX_LINUX__) && !defined(__OS400__)
           max_file_size *= UBSIZE; /* size in bytes = (#blkx) * blocksize */
#endif
         }/* endif */
  	return(max_file_size);
  
} /* get_max_filesize() */
