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

/* @(#)53	1.2  src/htx/usr/lpp/htx/bin/hxssup/nf.c, htx_sup, htxubuntu 5/24/04 13:45:03 */

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <curses.h>
#include <string.h>
#include "nfdef.h"
#include <hxiconv.h>

int get_queue_id(int * id)
{
	int msg_flag;
	msg_flag = IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
        msg_flag |= S_IROTH | S_IWOTH;

        *id = msgget(DISPKEY, msg_flag);
	

	if( *id == -1)
	{
		return(-1);
	}
	else
	{
		return(0);
	}

}



void *start_display(void)
{
  /*
   ***  Data and Functions Definitions/Declarations  **************************
   */

  	int main_exit_code;        /* Exit code passed via exit() system call */
  	int queue_id;              /* File id for pipe from hxsmsg            */
	struct disp_msg msg_buf;
	char program_name[10];
	WINDOW *win,*wintxt;
	char temp[30];

	extern int run_type_disp;
	extern void end_it(int);

  /*
   ***  Beginning of Executable Code  *****************************************
   */
  	strcpy(program_name, "newfet");

  	main_exit_code = 0;             /* Set good exit code                 */
 
  	if ( get_queue_id(&queue_id ) != 0)
	{
    		main_exit_code = 1;
		PRTMSG(LINES-1,0,("Not able to get the message qid"));
	}

  	else
    	{
      		while(1)
		{
	  		if(msgrcv(queue_id, &msg_buf ,
			 (sizeof(msg_buf) - sizeof(long)),0,MSG_NOERROR ) == -1)
			{
				if(queue_id == -1)
				{
					return(0);	
				}
				
			}
			else
			{
				if(LINES<24 || COLS <80)
				{
					PRTMSG(1,0,("Increase the window resolution to 23 - 81\n"));
				}
				else if(run_type_disp)
				{
					strcpy(temp,msg_buf.msg);
					move(23,52);
					attrset(A_BOLD);
					addstr(temp);
					endwin();
					move(19,52);
					refresh();
					sleep(2);
				}
			}
		}
    	} 


} /* main() */


