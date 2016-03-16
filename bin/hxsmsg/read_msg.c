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

/* @(#)21	1.7  src/htx/usr/lpp/htx/bin/hxsmsg/read_msg.c, htx_msg, htxubuntu 5/24/04 18:18:04 */

/*
 *   FUNCTIONS: read_message
 */


#include "hxsmsg.h"

#if defined(__HTX_LINUX__) || defined(__OS400__)
#define  msgxrcv(a,b,c,d,e)		msgrcv(a,b,c,d,e)
#endif /* __HTX_LINUX__ */

/*
 * Error code #define's for read_message()
 */
#define BAD_MSG_RECV (mtyp_t) -1


/*
 * NAME: read_message()
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
 *      read message from ipc message queue
 *
 *      if problem getting message
 *          print error message
 *
 *      return(message type)
 * 
 *               
 * RETURNS: 
 *
 *      Return Codes:
 *      -------------
 *        -1 -- error reading message
 *      x>=0 -- message type received from ipc message queue
 *
 *
 */  

mtyp_t read_message(struct htx_msg_xbuf *p_message_buffer)
     /* 
      * p_message_buffer -- pointer to the message buffer
      */

{
  /*
   ***  Data and Functions Definitions/Declarations  **************************
   */
  	char error_msg[128];       /* error message string                   */

  	extern char *program_name; /* this program's name (argv[0])          */

  	extern int msgqid;         /* ipc message queue id                   */

  	int errno_save;            /* errno save area                        */

  	mtyp_t exit_code;          /* exit program return code               */

  /*
   ***  Beginning of Executable Code  *****************************************
   */
  	errno = 0;
  	if (msgxrcv(msgqid, p_message_buffer,
	      (sizeof(struct htx_msg_xbuf) - sizeof(mtyp_t)),
	      -HTX_SYS_FINAL_MSG, MSG_NOERROR)
      	== -1)
    	{
      		errno_save = errno;

      		(void) sprintf(error_msg,
		"%s -- Error receiving message with msgxrcv() call.\n\
		errno: %d (%s).\n", 
		program_name,
		errno_save,
		strerror(errno_save));

      		(void) fprintf(stderr, "%s", error_msg);
      		(void) fflush(stderr);
      		exit_code = BAD_MSG_RECV;
    	}
  	else
	{
    		exit_code = p_message_buffer->mtype;
	}

  	return(exit_code);

} /* read_message */
