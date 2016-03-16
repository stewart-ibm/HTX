/* IBM_PROLOG_BEGIN_TAG */
/* 
 * Copyright 2003,2016 IBM International Business Machines Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 		 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* IBM_PROLOG_END_TAG */

/* @(#)09	1.5  src/htx/usr/lpp/htx/bin/hxsmsg/ck_values.c, htx_msg, htxubuntu 5/24/04 18:14:00 */

/*
 *   FUNCTIONS: ck_heartbeat_cycle
 *		ck_threshold
 *		ck_yn_flag
 */


#include "hxsmsg.h"

#define MAX_HEARTBEAT_CYCLE 3600
#define MIN_HEARTBEAT_CYCLE 1
#ifdef __HTX_LINUX__
#define UBSIZE 512
#endif
#define BAD_ATOI_CONVERSION 1

/*
 * NAME: ck_heartbeat_cycle()
 *                                                                    
 * FUNCTION: Checks the specified Stress Lab "heartbeat" cycle value.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function is called by the do_argv() function of the "hxsmsg"
 *      program which is a child process to the HTX supervisor program,
 *      "hxssup".
 *                                                                   
 * NOTES: 
 *
 *      The ck_heartbeat_cycle() function converts the string value of 
 *      the specified "heartbeat" cycle to a "int" value.  The "int" value
 *      is then checked to make sure that it is within the acceptable
 *      range:
 *
 *           MIN_HEARTBEAT_CYCLE < specified cycle < MAX_HEARTBEAT_CYCLE
 *
 *      If there is a problem with the conversion, a 1 is returned.
 * 
 *      If the specified cycle is smaller than the minimum cycle allowed
 *      (MIN_HEARTBEAT_CYCLE), the minimum allowed cycle is copied to the 
 *      cycle_string string.
 *
 *      If the specified cycle is larger than the maximum cycle allowed
 *      (MAX_HEARTBEAT_CYCLE), the maximum allowed cycle is copied to the
 *      cycle_string string.
 *
 *      If no errors or invalid values are detected, the GOOD value (0)
 *      is returned.
 *
 * RETURNS:
 *
 *      Returned Values:
 *      ---------------------------------
 *       0 -- Normal exit.  No errors.
 *       1 -- Error on atoi() conversion.
 *
 */  

short ck_heartbeat_cycle(char *cycle_string)
     /*
      * cycle_string -- "heartbeat" cycle specification: string
      */

	{

  /*
   ***  Local Data Definitions...  ********************************************
   */

  	char error_msg[256];       /* error message: string                   */

  	extern char *program_name; /* this program's name (argv[0])           */

	int errno_save;            /* errno save area                         */

  	short exit_code;           /* the exit code for the return() call     */
  	short heartbeat_cycle;     /* Stress Lab "heartbeat" cycle (sec)      */

  /*
   ***  Beginning of Executable Code...  **************************************
   */
  	exit_code = GOOD;

  	errno = 0;
  	heartbeat_cycle = atoi(cycle_string);
  	errno_save = errno;
  	if (errno_save == EBADF)              /* conversion error?            */
    	{
      		(void) sprintf(error_msg,
		"\n%s -- Error during atoi() conversion of the \
		specified Stress Lab \"heartbeat\" cycle (%s).\nerrno: %d (%s).\n",
		program_name,
                cycle_string,
		errno_save,
		strerror(errno_save));

      		(void) fprintf(stderr, "%s", error_msg);
      		(void) fflush(stderr);
      		exit_code = BAD_ATOI_CONVERSION;
    	}
  	else                            /* conversion OK                      */
    	{
      
      		if (heartbeat_cycle < MIN_HEARTBEAT_CYCLE)
		{
	  		(void) sprintf(error_msg,
			"\n%s -- Warning: Stress Lab \"heartbeat\" \
			cycle was specified as %d seconds.\nIt must be at least %d seconds.\n\
			So, the %s program has set it to the minimum value.\n",
			program_name,
			heartbeat_cycle,
			MIN_HEARTBEAT_CYCLE,
			program_name);

	  		(void) fprintf(stderr, "%s", error_msg);
	  		(void) fflush(stderr);
	  		(void) sprintf(cycle_string, "%d", MIN_HEARTBEAT_CYCLE);
		} /* endif */
      
      		else if (heartbeat_cycle > MAX_HEARTBEAT_CYCLE)
		{
	  		(void) sprintf(error_msg, "\n%s -- Warning: Stress \
			Lab \"heartbeat\" cycle was specified as %d seconds.\n\
			It must be no larger than %d seconds.\n\
			So, the %s program has set it to the maximum value.\n",
			program_name,
			heartbeat_cycle,
			MAX_HEARTBEAT_CYCLE,
			program_name);

	  		(void) fprintf(stderr, "%s", error_msg);
	  		(void) fflush(stderr);
	  		(void) sprintf(cycle_string, "%d", MAX_HEARTBEAT_CYCLE);
		} /* endif */
      
    	} /* endif */
 	return(exit_code);

} /* ck_heartbeat_cycle() */


/*
 * NAME: ck_threshold()
 *                                                                    
 * FUNCTION: Checks the specified maximum log threshold value.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function is called by the do_argv() function of the "hxsmsg"
 *      program which is a child process to the HTX supervisor program,
 *      "hxssup".
 *                                                                   
 * NOTES: 
 *
 *      The ck_threshold() function converts the string value of a log's 
 *      wrap/max-size threshold to an "off_t" value.  The "off_t" value
 *      is then checked to make sure that:
 *
 *          (1) the string-to-off_t conversion was successful, and,
 *          (2) the resultant "off_t" value is valid.
 *
 *      If there is a problem with the conversion or a negative threshold
 *      value was specified, a -1 is returned.
 * 
 *      If the specified threshold is larger than the maximum threshold
 *      allowed, the value of the maximum threshold is returned.
 *
 *      If no errors or invalid values are detected, the "off_t" value of
 *      the specified threshold is returned.
 *
 * RETURNS:
 *
 *      Returned Values:
 *      ----------------
 *         -1 -- Error converting the string threshold value or a negative 
 *               threshold was specified.
 *        n>0 -- The "off_t" value of the specified wrap/max-size threshold.
 *
 */  

off_t ck_threshold(char *thres_string, char *log_type, off_t max_threshold)
     /*
      * thres_string -- threshold specification: string
      * log_type -- the type threshold: string
      * max_threshold -- maximum allowed threshold value
      */

{

  /*
   ***  Local Data Definitions...  ********************************************
   */

  	char error_msg[1024];      /* error message : string                  */

  	extern char *program_name; /* this program's name (argv[0])           */

  	int errno_save;            /* errno save area                         */

  	off_t log_threshold;       /* log wrap/max-size threshold             */

  /*
   ***  Beginning of Executable Code...  **************************************
   */

  	errno = 0;
  	log_threshold = (off_t) atol(thres_string); 
				  /* convert string to off_t val             */
#ifdef __HTX_LINUX__
	log_threshold = log_threshold / UBSIZE;  
#endif
	errno_save = errno;

  	if (errno_save != 0)          /* bad conversion?                     */
    	{
      		(void) sprintf(error_msg,
		"\n%s -- Error during atol() conversion of the \
		specified %s Log wrap/max-size threshold (\"%s\"):\n\
		errno: %d (%s).\n",
		program_name,
		log_type,
                thres_string,
		errno_save,
		strerror(errno_save));

      		(void) fprintf(stderr, "%s", error_msg);
      		(void) fprintf(stderr, "log_threshold = %d.\n", (int)log_threshold);
      		(void) fflush(stderr);
      		log_threshold = -1;
    	}
  	else                               /* conversion OK                  */
    	{
		if (log_threshold < 0)     /* threshold valid number?        */
		{
	  		(void) sprintf(error_msg,
			"\n%s -- %s Log wrap/max-size threshold (%s) must \
			be a non-negative number.\n",
			program_name, 
			log_type, 
			thres_string);

	  		(void) fprintf(stderr, "%s", error_msg);
	  		(void) fflush(stderr);
	  		log_threshold = -1;
		} /* endif */
      
      		else if (log_threshold > max_threshold)  /* threshold too big?*/
		{
	  		(void) sprintf(error_msg,
			"\n%s -- Warning: %s Log wrap/max-size threshold \
			(%d) too big.\nThreshold set to %d by %s program.\n",
                        program_name,
			log_type,
                        (int)log_threshold,
                        (int)max_threshold,
                        program_name);

	  		(void) fprintf(stderr, "%s", error_msg);
	  		(void) fflush(stderr);
	  		log_threshold = max_threshold; /* set good threshold  */
		} /* endif */
    	} /* endif */
  	return(log_threshold);

} /* ck_threshold() */



/*
 * NAME: ck_yn_flag()
 *                                                                    
 * FUNCTION: Checks the specified yes/no flag to make sure that a "y/Y" or
 *           "n/N" was really specified.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	This function is called by the do_argv() function of the "hxsmsg"
 *      program which is a child process to the HTX supervisor program,
 *      "hxssup".
 *                                                                   
 * NOTES: 
 *
 *      The ck_yn_flag() function checks the string passed as "flag_string"
 *      to make sure that it really begins with a "y/Y" or a "n/N"; and,
 *      copies the "flag_string" string address value to the "good_name" 
 *      character pointer.
 *
 *      If the string value is valid, a 0 is returned.
 *  
 *      if the string is not valid, a -1 is returned and an error message is
 *      printed on stderr.
 *
 * RETURNS:
 *
 *      Returned Values:
 *      ----------------
 *         -1 -- Invlaid yes/no flag string.
 *          0 -- Valid yes/no flag string.
 *
 */  

short ck_yn_flag(char *flag_string, char *flag_type, tbool *p_tbool_flag)
     /*
      * flag_string -- flag specification: string
      * flag_type -- type of flag: string
      * p_tbool_flag -- pointer to boolean variable for final result
      */

{

  /*
   ***  Local Data Definitions...  ********************************************
   */
  	char error_msg[1024];      /* error message : string                  */

  	extern char *program_name; /* this program's name (argv[0])           */

  	short rc;                  /* return code                             */

  /*
   ***  Beginning of Executable Code...  **************************************
   */

  	rc = 0;
  	if ((*flag_string == 'y') || (*flag_string == 'Y'))
	{
    		*p_tbool_flag = TRUE;
	}
  	else if ((*flag_string == 'n') || (*flag_string == 'N'))
	{
    		*p_tbool_flag = FALSE;
	}
  	else
    	{
      		(void) sprintf(error_msg,
		"\n%s -- Invalid %s Flag (%s).\n",
		program_name,
		flag_type,
		flag_string);

      		(void) fprintf(stderr, "%s", error_msg);
      		(void) fflush(stderr);
      		rc = -1;
    	} /* endif */

  	return(rc);

} /* ck_yn_flag() */
