
/* @(#)19	1.7  src/htx/usr/lpp/htx/bin/hxsmsg/open_ipc.c, htx_msg, htxubuntu 5/20/09 06:18:46 */

/*
 *   FUNCTIONS: open_ipc
 */


#include "hxsmsg.h"

#if defined(__HTX_LINUX__) || defined(__OS400__)
#include <unistd.h>
#include<sys/stat.h>
#include<fcntl.h>
#else
#include <sys/mode.h>
#endif /* __HTX_LINUX__ */

/*
 * Error code #define's for open_ipc()
 */
#define BAD_MSGGET 1


/*
 * NAME: open_ipc()
 *                                                                    
 * FUNCTION: Opens an ipc message queue.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function is called by the main() function of the "hxsmsg"
 *      program.  The message handler program, "hxsmsg", is always a child
 *      process of the HTX supervisor program, "hxssup".
 *
 * NOTES: 
 *
 *      operation:
 *      ---------
 *
 *      get ipc message queue
 *
 *      if problem getting queue
 *          print error message
 *
 *      return
 * 
 *               
 * RETURNS: 
 *
 *      Return Codes:
 *      -------------
 *         0 -- Normal exit.  No errors.
 *         1 -- msgget() call failed.
 *
 *
 */  

short open_ipc()
     /*
      * no parameters
      */

{
  /*
   ***  Data and Functions Definitions/Declarations  **************************
   */
  	char error_msg[128];       /* error message string                   */
    struct msqid_ds temp_msg_ds; /* data structure to hold temp qbyte for msgctl */

  	extern char *program_name; /* this program's name (argv[0])          */

  	extern int msgqid;         /* ipc message queue id                   */
#ifdef __HTX_LINUX__
	extern int queue_id;       /* display device queue id                */
#endif

  	int errno_save;            /* errno save area                        */
  	int msg_flag;              /* flag for msgget() call                 */

  	short exit_code;           /* exit program return code               */

  /*
   ***  Beginning of Executable Code  *****************************************
   */
  	msg_flag = IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
  	msg_flag |= S_IROTH | S_IWOTH;

  	errno = 0;
  	msgqid = msgget(MSGLOGKEY, msg_flag);

    msgctl(msgqid, IPC_STAT, &temp_msg_ds); /* Setting the default settings. */
    temp_msg_ds.msg_qbytes = (8*1024*1024); /* Making it equal to 8MB (double the system default of 4MB */
    msgctl(msgqid, IPC_SET, &temp_msg_ds);  /* Restoring the settings (with the changed msg queue size). */
    
  	if (msgqid == -1)                        /* Problem accessing log?   */
    	{
      		errno_save = errno;

      		(void) sprintf(error_msg, 
		"\n%s: Unable to access HTX message queue.\n\
		errno: %d (%s).\n",
		program_name,
		errno_save,
		strerror(errno_save));

      		(void) fprintf(stderr, "%s", error_msg);
      		(void) fflush(stderr);
      		exit_code = BAD_MSGGET;
    	}
  	else
   	exit_code = GOOD;
	
#ifdef __HTX_LINUX__
	queue_id = msgget(DISPKEY, msg_flag);
        if (queue_id == -1)                      /* Problem accessing log?   */
        {
                errno_save = errno;

                (void) sprintf(error_msg,
                "\n%s: Unable to access DISPLAY message queue.\n\
                errno: %d (%s).\n",
                program_name,
                errno_save,
                strerror(errno_save));

                (void) fprintf(stderr, "%s", error_msg);
                (void) fflush(stderr);
                exit_code = BAD_MSGGET;
        }
        else
        exit_code = GOOD;
#endif


  	return(exit_code);

} /* open_ipc() */
