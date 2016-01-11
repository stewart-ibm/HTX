
/* @(#)13	1.6.4.1  src/htx/usr/lpp/htx/bin/hxsmsg/do_argv.c, htx_msg, htxubuntu 8/3/12 07:56:07 */

/*
 *   FUNCTIONS: do_argv
 */


#include "hxsmsg.h"

#if defined(__HTX_LINUX__) || defined(__OS400__)
#include   <unistd.h>
#define W_ACC W_OK
#else
#include <sys/limits.h>
#include <sys/access.h>
#endif /* __HTX_LINUX__ */

#define ARGC_EXPECTED 11

/*
 * Error code #define's for do_argv()
 */
#define BAD_ERR_THRES 0x0001
#define BAD_MSG_THRES 0x0002
#define BAD_ERR_SAVE_THRES 0x0004
#define BAD_MSG_SAVE_THRES 0x0008
#define BAD_ERR_WRAP_FLAG 0x0010
#define BAD_MSG_WRAP_FLAG 0x0020
#define BAD_AUTO_START_FLAG 0x0040
#define BAD_HEARTBEAT_DEV 0x0080
#define BAD_HEARTBEAT_CYCLE 0x0100
#define BAD_ARGC 0x0200


/*
 * NAME: do_argv()
 *                                                                    
 * FUNCTION: Processes argv[] parameters.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function is called by the main() function of the "hxsmsg"
 *      program which is a child process to the HTX supervisor program,
 *      "hxssup".
 *                                                                   
 * NOTES: 
 *
 *      The do_argv() function takes the parameters passed to the "hxsmsg"
 *      via the argv[] string array and checks them for valid values.
 *
 *      If the argv[] parameters are valid, they are converted and/or
 *      copied to variables with meaningful names.
 *
 * RETURNS:
 *
 *     Returned Values:
 *     -------------------------------------------------------------------
 *       0 (0x0000) -- Normal exit.  The expected number of parameters were
 *                     passed and their values were valid.
 *       1 (0x0001) -- Bad Error Log threshold (argv[1]) parameter.
 *       2 (0x0002) -- Bad Message Log threshold (argv[2]) parameter.
 *       4 (0x0004) -- Bad Error Save Log threshold (argv[3]) parameter.
 *       8 (0x0008) -- Bad Message Save Log threshold (argv[4]) parameter.
 *      16 (0x0010) -- Bad Error Log wrap flag (argv[5]) parameter.
 *      32 (0x0020) -- Bad Message Log wrap flag (argv[6]) parameter.
 *      64 (0x0040) -- Bad Auto-Start flag (argv[7]) parameter.
 *     128 (0x0080) -- Bad "Heartbeat" device (argv[8]) parameter.
 *     256 (0x0100) -- Bad "Heartbeat" cycle time (argv[9]) parameter.
 *     512 (0x0200) -- Bad number of passed arguements (argc).
 *
 *      NOTE: Each of the error codes listed above take up a single bit
 *            position.  Thus, multiple error conditions are indicated
 *            by "OR'ing" the indicated error codes into a composite
 *            return code.
 *
 *            For example, if the Error Log threshold value is bad
 *            (error code 0x0001) and the "Heartbeat" cycle time is bad
 *            (error code 0x0100), the return code would be 0x0001 | 0x0100
 *            = 0x0101 (decimal 257).
 *
 */  

short do_argv(char *argv[], int argc, off_t *p_err_thres, off_t *p_msg_thres,
	      off_t *p_err_save_thres, off_t *p_msg_save_thres,
              tbool *p_err_wrap_flag, tbool *p_msg_wrap_flag, tbool *p_msg_archive_flag,
              tbool *p_auto_start_flag, char **p_heartbeat_dev,
              char *p_heartbeat_cycle, size_t max_cycle_string)
/*
 *    argv[] -- parameter list
 *    argc -- number of parameters
 *    p_err_thres -- ptr to Error Log wrap/max-size threshold
 *    p_msg_thres -- ptr to Message Log wrap/max-size thres
 *    p_err_save_thres -- ptr to Err Save Log wrap/max-size thres
 *    p_msg_save_thres -- ptr to Msg Save Log wrap/max-size thres
 *    p_err_wrap_flag -- ptr to Error Log Wrap Flag
 *    p_msg_wrap_flag -- ptr to Message Log Wrap Flag
 *    p_auto_start_flag -- ptr to system Auto-Start Flag
 *    p_heartbeat_dev -- ptr to Stress Lab "heartbeat" dev string
 *    p_heartbeat_cycle -- ptr to Stress Lab "heartbeat" cyc time(sec)
 *    max_cycle_string -- maximum p_heartbeat_cycle string length
 */

{
  /*
   ***  Local Data Definitions & Function Prototypes...  **********************
   */
  	extern char *program_name;     /* this program's name (argv[0])  */

  	off_t max_file_size;           /* maximum file size allowed to process*/
  	off_t max_threshold;           /* maximum file threshold allowed      */

  	short exit_code;                 /* this function's exit code         */

  /*
   ***  Beginning of Executable Code...  **************************************
   */
  	exit_code = GOOD;

 	program_name = argv[0];

  /* Make sure that the expected number of arguements were passed in argv[]. */
  	if (argc != ARGC_EXPECTED)
    	{
      		(void) fprintf(stderr, "\n%s: Unexpected number of passed arguements.",
		     program_name);
      		exit_code |= BAD_ARGC;
    	}
  	else
    	{
      
      /*
       * Calculate the maximum threshold where,
       *     max threshold = max file size - max length of a single disk write.
       */
      
      		max_file_size = get_max_filesize();
   
   		if (max_file_size == -1)  /* error getting max file size? */
		#ifndef AIXVER2
		max_threshold = LONG_MAX - DISK_BUF_SIZE;
		#else
        	max_threshold = MAXLONG - DISK_BUF_SIZE;
		#endif
      		else
        	max_threshold = max_file_size - DISK_BUF_SIZE;

  
      /* Check the argv[] values and convert or copy them as approprite */
  
      		if ((*p_err_thres = ck_threshold(argv[1], "Error", max_threshold))== -1)
		{
			exit_code |= BAD_ERR_THRES;
		}
      
      		if ((*p_msg_thres = ck_threshold(argv[2], "Message", max_threshold)) == -1)
		{
			exit_code |= BAD_MSG_THRES;
		}
      
      		if ((*p_err_save_thres = ck_threshold(argv[3], "Error Save",  max_threshold)) == -1)
		{
			exit_code |= BAD_ERR_SAVE_THRES;
		}
      
      		if ((*p_msg_save_thres = ck_threshold(argv[4], "Message Save",max_threshold)) == -1)
		{
			exit_code |= BAD_MSG_SAVE_THRES;
		}
      
      		if (ck_yn_flag(argv[5], "Error Log Wrap", p_err_wrap_flag) != GOOD)
		{
			exit_code |= BAD_ERR_WRAP_FLAG;
		}
      
      		if (ck_yn_flag(argv[6], "Message Log Wrap", p_msg_wrap_flag) != GOOD)
		{
			exit_code |= BAD_MSG_WRAP_FLAG;
		}

      		if (ck_yn_flag(argv[7], "Message Archive", p_msg_archive_flag) != GOOD)
		{
			exit_code |= BAD_MSG_WRAP_FLAG;
		}
      
      		if (ck_yn_flag(argv[8], "Auto-Startup", p_auto_start_flag) != GOOD)
		{
			exit_code |= BAD_AUTO_START_FLAG;
		}
      
      		*p_heartbeat_dev = argv[9];
      		if (htx_strcmp(*p_heartbeat_dev, "/dev/null") != 0)
		{
	  		if (access(*p_heartbeat_dev, W_ACC) != 0)
	    		{
	      			(void) fprintf(stderr, "\n%s -- write access to the specified \
				stress device (%s) is not permitted.\n", program_name, *p_heartbeat_dev);
	      			(void) fflush(stderr);
	      			exit_code |= BAD_HEARTBEAT_DEV;
	    		} /* endif */
	  
	  		(void) htx_strncpy(p_heartbeat_cycle, argv[10], max_cycle_string);
	  		if (ck_heartbeat_cycle(p_heartbeat_cycle) == -1)
			{
	    			exit_code |= BAD_HEARTBEAT_CYCLE;
			}
		} /* endif */

    	} /* endif */
 
  	return(exit_code);

} /* do_argv() */
