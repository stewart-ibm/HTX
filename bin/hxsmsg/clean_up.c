
/* @(#)10	1.5  src/htx/usr/lpp/htx/bin/hxsmsg/clean_up.c, htx_msg, htxubuntu 5/24/04 18:14:15 */

/*
 *   FUNCTIONS: clean_up
 */


#include "hxsmsg.h"

/*
 * clean_up() error code definitions
 */
#define BAD_MSGQ_CLOSE 0x0001
#define BAD_CKPT_CLOSE 0x0002
#define BAD_ERR_CLOSE 0x0004
#define BAD_ERR_SAVE_CLOSE 0x0008
#define BAD_MSG_CLOSE 0x0010
#define BAD_MSG_SAVE_CLOSE 0x0020
#define BAD_SEND_SIGNAL 0x0040
#define BAD_PIPE_0_CLOSE 0x0080
#define BAD_PIPE_1_CLOSE 0x0100
#define BAD_SET_SIGNAL_HDL 0x0200



/*
 * NAME: clean_up()
 *                                                                    
 * FUNCTION: Cleans up the hxsmsg operating environment prior to exit.
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
 *      close the IPC message queue
 *      close all log files
 *      send a SIGTERM signal to hxsstress
 *      close the pipe devices
 *
 *      return
 * 
 *               
 * RETURNS: 
 *
 *      Return Codes:
 *      ------------------------------------------------------------------
 *        0 (0x0000) -- Normal exit.
 *        1 (0x0001) -- Error on msgctl IPC_RMID if message queue
 *        2 (0x0002) -- Error on close_file() of ckpt file.
 *        4 (0x0004) -- Error on close_file() of error log file.
 *        8 (0x0008) -- Error on close_file() of error save log file.
 *       16 (0x0010) -- Error on close_file() of message log file.
 *       32 (0x0020) -- Error on close_file() of message save log file.
 *       64 (0x0040) -- Error with send_signal() function.
 *      128 (0x0080) -- Error with close_file() of pipe_dev[0].
 *      256 (0x0100) -- Error with close_file() of pipe_dev[1].
 *      512 (0x0200) -- Error with set_signal_hdl().
 *
 *      NOTE: Each of the error codes listed above take up a single bit
 *            position.  Thus, multiple error conditions are indicated
 *            by "OR'ing" the indicated error codes into a composite
 *            return code.
 *
 *            For example, if the close() call for the error save log file
 *            failed (error code 0x0008) and the send_signal() call failed
 *            (error code 0x0040), the return code would be 0x0008 | 0x0040
 *            = 0x0048 (decimal 72).
 *
 *
 */  

short clean_up()
     /*
      * no parameters
      */

{
  /*
   ***  Data and Functions Definitions/Declarations  **************************
   */
  	char error_msg[128];       /* error message string                    */

  	extern char *program_name; /* this program's name (argv[0])           */

  	extern int ckpt_file;      /* Checkpoint File File Descriptor         */
  	extern int err_log;        /* Error Log File Descriptor               */
  	extern int err_save_log;   /* Error Save Log File Descriptor          */
  	extern int msg_log;        /* Message Log File Descriptor             */
  	extern int msg_save_log;   /* Message Save Log File Descriptor        */
  	extern int msgqid;         /* Message queue id                        */
#ifdef __HTX_LINUX__
	extern int queue_id;
#endif
  	extern int pipe_dev[];     /* pipe file id's                          */

  	extern pid_t hxsstress_PID;  /* hxsstress "heartbeat" program PID     */

  	int errno_save;            /* errno save area                         */

  	short exit_code;           /* exit program return code                */

  /*
   ***  Beginning of Executable Code  *****************************************
   */
  	exit_code = GOOD;

  	errno = 0;
  	if (msgqid != -1)
    	{
      		if (msgctl(msgqid, IPC_RMID, (struct msqid_ds *) 0) != GOOD)
		{
	  		errno_save = errno;
	  
	  		(void) sprintf(error_msg, 
			"\n%s -- Error on msgctl IPC_RMID of the message \
			queue.\nerrno: %d (%s).\n", 
			program_name,
			errno_save,
			strerror(errno_save));
	  
	  		(void) fprintf(stderr, "%s", error_msg);
	  		(void) fflush(stderr);
	  		exit_code |= BAD_MSGQ_CLOSE;
		} /* endif */
      		msgqid = -1;
    	} /* endif */
 
#ifdef __HTX_LINUX__
	if (queue_id != -1)
        {
                if (msgctl(queue_id, IPC_RMID, (struct msqid_ds *) 0) != GOOD)
                {
                        errno_save = errno;

                        (void) sprintf(error_msg,
                        "\n%s -- Error on msgctl IPC_RMID of the message \
                        queue.\nerrno: %d (%s).\n",
                        program_name,
                        errno_save,
                        strerror(errno_save));

                        (void) fprintf(stderr, "%s", error_msg);
                        (void) fflush(stderr);
                        exit_code |= BAD_MSGQ_CLOSE;
                } /* endif */
                queue_id = -1;
        } /* endif */
#endif


 	if (ckpt_file != -1)
	{
    		if (close_file(&ckpt_file, CKPT_FILE) != GOOD)
		{
      			exit_code |= BAD_CKPT_CLOSE;
		}
	}

  	if (err_log != -1)
	{
    		if (close_file(&err_log, ERR_LOG) != GOOD)
		{
      			exit_code |= BAD_ERR_CLOSE;
		}
	}

  	if (err_save_log != -1)
	{
    		if (close_file(&err_save_log, ERR_SAVE_LOG) != GOOD)
		{
      			exit_code |= BAD_ERR_SAVE_CLOSE;
		}
	}
  	
	if (msg_log != -1) 
	{
    		if (close_file(&msg_log, MSG_LOG) != GOOD)
		{
		      	exit_code |= BAD_MSG_CLOSE;
		}
	}

  	if (msg_save_log != -1)
	{
    		if (close_file(&msg_save_log, MSG_SAVE_LOG) != GOOD)
		{
	      		exit_code |= BAD_MSG_SAVE_CLOSE;
		}
	}

  	if (hxsstress_PID != -1)
    	{
      		if (set_signal_hdl(SIGCHLD,(void (*)(int, int, struct sigcontext *)) SIG_IGN) != GOOD )
		{
			exit_code |= BAD_SET_SIGNAL_HDL;
		}
      		if (send_signal(hxsstress_PID, SIGTERM) != GOOD)
		{
			exit_code |= BAD_SEND_SIGNAL;
		}
    	} /* endif */

  	if (pipe_dev[0] != -1)
	{
    		if (close_file(&pipe_dev[0], "pipe_dev[0]") != GOOD)
		{
      			exit_code |= BAD_PIPE_0_CLOSE;
		}
	}

  	if (pipe_dev[1] != -1)
	{
    		if (close_file(&pipe_dev[1], "pipe_dev[1]") != GOOD)
		{
      			exit_code |= BAD_PIPE_1_CLOSE;
		}
	}

  	return(exit_code);

} /* clean_up() */
