
/* @(#)20	1.6  src/htx/usr/lpp/htx/bin/hxsmsg/open_logs.c, htx_msg, htxubuntu 5/24/04 18:17:48 */

/*
 *   FUNCTIONS: open_logs
 *		set_file_offsets
 */


#include "hxsmsg.h"
#include <fcntl.h>

#if defined(__HTX_LINUX__) || defined(__OS400__)
#include <unistd.h>
#include <sys/stat.h>
#else
#include <sys/mode.h>
#endif

/*
 * Error code #define's for open_logs()
 */
#define BAD_CHECK_LOGS 0x0001
#define BAD_CKPT_RM 0x0002
#define BAD_ERR_RM 0x0004
#define BAD_MSG_RM 0x0008
#define BAD_ERR_SAVE_RM 0x0010
#define BAD_MSG_SAVE_RM 0x0020
#define BAD_ERR_OPEN 0x0040
#define BAD_MSG_OPEN 0x0080
#define BAD_ERR_SAVE_OPEN 0x0100
#define BAD_MSG_SAVE_OPEN 0x0200
#define BAD_SET_OFFSETS 0x0400

/*
 * Error code #define's for set_file_offsets()
 */
#define BAD_ERR_LSEEK 0x0001
#define BAD_MSG_LSEEK 0x0002
#define BAD_ERR_SAVE_LSEEK 0x0004
#define BAD_MSG_SAVE_LSEEK 0x0008


/*
 * NAME: open_logs()
 *                                                                    
 * FUNCTION: Opens the HTX message and error logs.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function is called by the main() function of the "hxsmsg"
 *      program which is a child process of the HTX supervisor program,
 *      "hxssup". 
 *
 * NOTES: 
 *
 *      operation:
 *      ---------
 *
 *      if auto-start environment
 *        check the existing (if any) logs
 *      else
 *        remove any existing "check-point" or log files
 *       
 *      if no errors
 *        open log files
 *        if auto-start environment
 *          set log file pointers (lseek())
 *
 *      return
 * 
 *               
 * RETURNS: 
 *
 *      Return Codes:
 *      -------------
 *         0 (0x0000) -- Normal exit.  No errors.
 *         1 (0x0001) -- Unable to check log
 *         2 (0x0002) -- Unable to remove check-point file
 *         4 (0x0004) -- Unable to remove error log file
 *         8 (0x0008) -- Unable to remove message log file
 *        16 (0x0010) -- Unable to remove error log savefile
 *        32 (0x0020) -- Unable to remove message log save file
 *        64 (0x0040) -- Unable to open error log file
 *       128 (0x0080) -- Unable to open message log file
 *       256 (0x0100) -- Unable to open error save log file
 *       512 (0x0200) -- Unable to open message save log file
 *      1024 (0x0400) -- Error setting file offsets
 *
 *      NOTE: Each of the error codes listed above take up a single bit
 *            position.  Thus, multiple error conditions are indicated
 *            by "OR'ing" the indicated error codes into a composite
 *            return code.
 *
 *            For example, if the remove of the check-point file failed
 *            (error code 0x0002) and the open of the error log file failed
 *            (error code 0x0040), the return code would be 0x0002 | 0x0040
 *            = 0x0042 (decimal 66).
 *
 *
 */  

short open_logs(off_t err_thres, off_t msg_thres, off_t err_save_thres,
		off_t msg_save_thres, tbool auto_start_flag)
     /*
      * err_thres -- max err log file size threshold
      * msg_thres -- max msg log file size threshold
      * err_save_thres -- max save err log file size threshold
      * msg_save_thres -- max save msg log file size threshold
      * auto_start_flag -- system auto-start flag
      */

{
  /*
   ***  Data and Functions Definitions/Declarations  **************************
   */

  	extern ckpt checkpt;       /* Checkpoint file structure               */

  	extern int err_log;        /* error log file descriptor               */
  	extern int err_save_log;   /* error log save file descriptor          */
  	extern int msg_log;        /* message log file descriptor             */
  	extern int msg_save_log;   /* message log save file descriptor        */

 	int mode;                  /* specifies file permissions for open()   */
  	int oflag;                 /* open flag (sets open() attributes)      */

  	short exit_code;           /* function exit code                      */


  /*
   ***  Beginning of Executable Code  *****************************************
   */
  	exit_code = GOOD;

  	if (auto_start_flag == TRUE)    /* auto-start environment?           */
    	{
      		oflag = O_CREAT | O_WRONLY;
      		if (check_logs(err_thres, msg_thres, err_save_thres, msg_save_thres)
	  	!= GOOD)
		{
			exit_code |= BAD_CHECK_LOGS;
		}
    	}
  	else
    	{
      		oflag = O_CREAT | O_EXCL | O_WRONLY;

      		if (rm_file(CKPT_FILE) != GOOD)
		{
			exit_code |= BAD_CKPT_RM;
		}
      		if (rm_file(ERR_LOG) != GOOD)
		{
			exit_code |= BAD_ERR_RM;
		}
      		if (rm_file(MSG_LOG) != GOOD)
		{
			exit_code |= BAD_MSG_RM;
		}
      		if (rm_file(ERR_SAVE_LOG) != GOOD)
		{
			exit_code |= BAD_ERR_SAVE_RM;
		}
      		if (rm_file(MSG_SAVE_LOG) != GOOD)
		{
			exit_code |= BAD_MSG_SAVE_RM;
		}
    	} /* endif */

  	if (exit_code == GOOD)
    	{
      		mode= S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; 
                                 /* set permission to -rw-rw-rw-             */

      		if ((err_log = open_file(ERR_LOG, oflag, mode)) == -1)
		{
			exit_code |= BAD_ERR_OPEN;
		}

      		if ((msg_log = open_file(MSG_LOG, oflag, mode)) == -1)
		{
			exit_code |= BAD_MSG_OPEN;
		}

      		if ((err_save_thres > 0) &&
	  	(err_save_thres > checkpt.err_save_offset))
		{
			if ((err_save_log = open_file(ERR_SAVE_LOG, oflag, mode)) == -1)	
			{
	  			exit_code |= BAD_ERR_SAVE_OPEN;
			}
		}

      		if ((msg_save_thres > 0) &&
	  	(msg_save_thres > checkpt.msg_save_offset))
		{
			if ((msg_save_log = open_file(MSG_SAVE_LOG, oflag, mode)) == -1)
			{
	  			exit_code |= BAD_MSG_SAVE_OPEN;
			}
		}

      /*
       * If this is an auto-startup, we need to set the file pointers
       * to their last valid values during the previous run...
       */
      		if (auto_start_flag == TRUE)
		{
			if (set_file_offsets() != GOOD)
			{
	  			exit_code |= BAD_SET_OFFSETS;
			}
		}

    	} /* endif */

  	return(exit_code);
	
} /* open_logs() */




/*
 * NAME: set_file_offsets()
 *                                                                    
 * FUNCTION: Sets the log file offsets according to check-point file 
 *           data.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function is called by the open_logs() function of the "hxsmsg"
 *      program which is a child process of the HTX supervisor program,
 *      "hxssup". 
 *
 * NOTES: 
 *
 *      operation:
 *      ---------
 *
 *      if error log file open
 *        set file offset
 *
 *      if message log file open
 *        set file offset
 *
 *      if error save log file open
 *        set file offset
 *
 *      if message save log file open
 *        set file offset
 *
 *      return
 * 
 *               
 * RETURNS: 
 *
 *      Return Codes:
 *      -------------
 *         0 (0x0000) -- Normal exit.  No errors.
 *         1 (0x0001) -- Unable to set file ptr on error log file
 *         2 (0x0002) -- Unable to set file ptr on message log file
 *         4 (0x0004) -- Unable to set file ptr on error save log file
 *         8 (0x0008) -- Unable to ser file ptr on message save log file
 *
 *      NOTE: Each of the error codes listed above take up a single bit
 *            position.  Thus, multiple error conditions are indicated
 *            by "OR'ing" the indicated error codes into a composite
 *            return code.
 *
 *            For example, if the lseek() system call failed on both the
 *            error log file (error code 0x0001) and the error save log file
 *            (error code 0x0004), the return code would be 0x0001 | 0x0004
 *            = 0x0005 (decimal 5).
 *
 *
 */

short set_file_offsets()

{
  /*
   ***  Data and Functions Definitions/Declarations  **************************
   */
  	char error_msg[256];       /* error message string                    */

 	extern char *program_name; /* this program's name (argv[0])           */

  	extern ckpt checkpt;       /* log files checkpoint structure          */

  	extern int err_log;        /* error log file descriptor               */
  	extern int err_save_log;   /* error log save file descriptor          */
  	extern int msg_log;        /* message log file descriptor             */
  	extern int msg_save_log;   /* message log save file descriptor        */

  	int errno_save;            /* save area for errno                     */

  	short exit_code;           /* function exit code                      */


  /*
   ***  Beginning of Executable Code  *****************************************
   */
  	exit_code = GOOD;
  
  	if (err_log != -1)
    	{
      		errno = 0;
      		if (lseek(err_log, checkpt.err_offset, SEEK_SET) == -1)
		{
	  		errno_save = errno;
	  		(void) sprintf(error_msg, 
			"\n%s -- Error on lseek() SEEK_SET for %s.\n\
			errno: %d (%s).\n",
			program_name,
			ERR_LOG,
			errno_save,
			strerror(errno_save));
	  
	  		(void) fprintf(stderr, "%s", error_msg);
	  		(void) fflush(stderr);
	 		exit_code |= BAD_ERR_LSEEK;
		} /* endif */
    	} /* endif */
  
  	if (msg_log != -1)
    	{
      		errno = 0;
      		if (lseek(msg_log, checkpt.msg_offset, SEEK_SET) == -1)
		{
	  		errno_save = errno;
	  		(void) sprintf(error_msg, 
			"\n%s -- Error on lseek() SEEK_SET for %s.\n\
			errno: %d (%s).\n",
			program_name,
			MSG_LOG,
			errno_save,
			strerror(errno_save));
	  
	  		(void) fprintf(stderr, "%s", error_msg);
	  		(void) fflush(stderr);
	  		exit_code |= BAD_MSG_LSEEK;
		} /* endif */
    	} /* endif */
  
  	if (err_save_log != -1)
    	{
      		errno = 0;
      		if (lseek(err_save_log, checkpt.err_save_offset, SEEK_SET)
	  	== -1)
		{
	  		errno_save = errno;
	  		(void) sprintf(error_msg, 
			"\n%s -- Error on lseek() SEEK_SET for %s.\n\
			errno: %d (%s).\n",
			program_name,
			ERR_SAVE_LOG,
			errno_save,
			strerror(errno_save));
	  
	  		(void) fprintf(stderr, "%s", error_msg);
	  		(void) fflush(stderr);
	  		exit_code |= BAD_ERR_SAVE_LSEEK;
		} /* endif */
    	} /* endif */
  
  	if (msg_save_log != -1)
    	{
      		errno = 0;
      		if (lseek(msg_save_log, checkpt.msg_save_offset, SEEK_SET)
	  	== -1)
		{
	  		errno_save = errno;
	  		(void) sprintf(error_msg, 
			"\n%s -- Error on lseek() SEEK_SET for %s.\n\
			errno: %d (%s).\n",
			program_name,
			MSG_SAVE_LOG,
			errno_save,
			strerror(errno_save));
	  
	  		(void) fprintf(stderr, "%s", error_msg);
	  		(void) fflush(stderr);
	  		exit_code |= BAD_MSG_SAVE_LSEEK;
		} /* endif */
    	} /* endif */

 	return(exit_code);
  
} /* set_file_offsets() */
