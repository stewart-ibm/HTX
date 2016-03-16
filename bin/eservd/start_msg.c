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


#include "eservd.h"
#include "global.h"

#if defined(__OS400__)   /* 400 */
#include <spawn.h>
#endif

/*
 * start_msg() error code definitions
 */
#define BAD_FORK 1
#define BAD_EXEC 2

/*
 * NAME: start_msg_hdl()
 *
 * FUNCTION: Starts the HTX Message Handler Program, "hxsmsg", via the
 *           fork() and exec() system calls.
 *
 * EXECUTION ENVIRONMENT:
 *
 *	This function is called by the htx_profile() function (found in the
 *      hxssup.c module) of the HTX supervisor program, "hxssup".
 *
 * NOTES:
 *
 *      operation:
 *      ---------
 *
 *
 *
 * RETURNS:
 *
 *      Return Codes:
 *      ------------------------------------------------------------------
 *       0 -- Normal exit.
 *       1 -- Error in fork() system call.
 *       2 -- Error in exec() system call (passed via child process's
 *            exit() system call).
 *
 */

short start_msg_hdl(int autostart)
{
  /*
   ***  Data and Functions Definitions/Declarations  **************************
   */
    char auto_start_flag[4];            /* startup type (regular/auto)         */
    char workstr[128];                  /* work string                         */

    extern char ERR_WRAP[];             /* htxerr file wrap keyword.           */
    extern char HTXPATH[];              /* HTX file system path spec           */
    extern char MAX_ERR_SAVE[];         /* htxerr.save max filesize            */
    extern char MAX_ERR_THRES[];        /* err log file wrap threshold.        */
    extern char MAX_MSG_SAVE[];         /* htxmsg.save max filesize            */
    extern char MAX_MSG_THRES[];        /* msg log file wrap threshold.        */
    extern char MSG_ARCHIVE[];          /* message archive mode.               */
    extern char MSG_WRAP[];             /* htxmsg file wrap keyword.           */
    extern char stress_cycle[];         /* # sec between "heartbeats"          */
    extern char stress_dev[];           /* stress "heartbeat" device           */

    //extern int hxsmsg_PID;              /* message handler process id          */
    //extern int msgqid;                  /* Message Log message queue id        */

    //extern union shm_pointers shm_addr; /* shared memory address               */

#ifdef __HTX_LINUX__
    extern int wof_test_enabled;
#endif
    int errno_save;                     /* errno save variable                 */
    int exec_rc;                        /* exec() return code                  */
    int frkpid;                         /* PID from fork()                     */
    int i;                              /* loop counter                        */

    short return_code;                  /* The function's return code          */

    struct sigaction sigvector;         /* structure for signals               */

#if defined(__OS400__)   /* 400 */
  /* os400 */
    char * spw_path = "/usr/lpp/htx/bin/hxsmsg";
    char * spw_path2 = "hxsmsg";
    char * spw_argv[11];
    char * spw_env[1];
    struct inheritance spw_inherit;
    DBTRACE(DBENTRY,("enter OS400 start_msg.c start_msg_hdl\n"));

    return_code = 0;                    /* Set a good return code.             */
    spw_argv[0] = spw_path2;
    spw_argv[1] = MAX_ERR_THRES;
    spw_argv[2] = MAX_MSG_THRES;
    spw_argv[3] = MAX_ERR_SAVE;
    spw_argv[4] = MAX_MSG_SAVE;
    spw_argv[5] = ERR_WRAP;
    spw_argv[6] = MSG_WRAP;
    if (autostart == 0)
	htx_strcpy(auto_start_flag, "no");
    else
	htx_strcpy(auto_start_flag, "yes");
    spw_argv[7] = auto_start_flag;
    spw_argv[8] = stress_dev;
    spw_argv[9] = stress_cycle;
    spw_argv[10] = NULL;

    spw_env[0] = NULL;

  /* clean up signals to default action and no masks */
    memset(&spw_inherit,0x00,sizeof(spw_inherit));
    spw_inherit.flags = SPAWN_SETSIGMASK | SPAWN_SETSIGDEF;
    spw_inherit.pgroup = SPAWN_NEWPGROUP;
    sigemptyset(&(spw_inherit.sigmask));
    sigfillset(&(spw_inherit.sigdefault));

    frkpid = spawn(spw_path, 0, NULL, &spw_inherit, spw_argv, spw_env);

    switch (frkpid) {
	case -1:
	    errno_save = errno;
	    //PRTMSG(MSGLINE, 0, ("Unable to fork for message handler.  errno = %d", errno));
	    print_log(LOGERR,"Unable to fork for message handler.  errno = %d", errno);
	    //getch();
	    return_code = BAD_FORK;
	    break;

	default:
	    hxsmsg_PID = frkpid;
#ifdef __HTX_LINUX__
        if (wof_test_enabled) { /* bind to any thread of core 0  - currently on linux only.*/
	        do_the_bind_proc(hxsmsg_PID);
	    }
#endif
        sleep((unsigned int) 2);	/* give the hxsmsg child process a chance to start */
	    break;
    }			/* endswitch */

    DBTRACE(DBEXIT,("return OS400 start_msg.c start_msg_hdl\n"));
    return (return_code);

#else
    DBTRACE(DBENTRY,("enter start_msg.c start_msg_hdl\n"));

  /*
   ***  Beginning of Executable Code  *****************************************
   */
    return_code = 0;                    /* Set a good return code.             */
    errno = 0;
    frkpid = fork();

    switch(frkpid)
    {
	case 0:                    /* child process                              */
	    if ( listener != 0 ) {
		close(listener);
	    }
	    if ( fd_num != 0 ) {
		close(fd_num);
	    }

	    sigemptyset(&(sigvector.sa_mask));  /* do not block other signals      */
	    sigvector.sa_flags = 0;             /* no special processing           */
	    sigvector.sa_handler = SIG_DFL;     /* default action on signal        */

	    for (i = 1; i <= SIGMAX; i++)   /* set default signal processing       */
		(void) sigaction(i, &sigvector, (struct sigaction *) NULL);

	    (void) setpgrp();      /* change the process group so we don't
				get signals meant for the supervisor
				program */

	    (void) strcpy(workstr, HTXPATH);
	    (void) strcat(workstr, "/bin/hxsmsg");

	    if (autostart == 0)
		(void) strcpy(auto_start_flag, "no");
	    else
		(void) strcpy(auto_start_flag, "yes");

	    errno = 0;
	    exec_rc = execl(workstr, "hxsmsg", MAX_ERR_THRES, MAX_MSG_THRES,
			    MAX_ERR_SAVE, MAX_MSG_SAVE, ERR_WRAP, MSG_WRAP, MSG_ARCHIVE,
			    auto_start_flag, stress_dev, stress_cycle, (char *) 0);

      /*
       * If we're here, the execl() system call failed!!!
       */
	    errno_save = errno;
	    print_log(LOGERR,"Unable to load message handler.  errno = %d", errno_save);
	    //PRTMSG(MSGLINE, 0,
		     //  ("Unable to load message handler.  errno = %d", errno_save));
	    //(void) getch();
	    DBTRACE(DBEXIT,("exit start_msg.c start_msg_hdl\n"));
	    exit(BAD_EXEC);

	case -1:
	    errno_save = errno;
	    print_log(LOGERR,"Unable to fork for message handler.  errno = %d", errno);
	    //PRTMSG(MSGLINE, 0,
		     //  ("Unable to fork for message handler.  errno = %d", errno));
	    //(void) getch();
	    DBTRACE(DBEXIT,("return start_msg.c start_msg_hdl\n"));
	    return_code = BAD_FORK;
	    break;

	default:
	    hxsmsg_PID = frkpid;
	    (void) sleep((unsigned int) 2);   /* give the hxsmsg child process a
					   chance to start                   */
	    break;
    } /* endswitch */

    DBTRACE(DBEXIT,("return start_msg.c start_msg_hdl\n"));
    return(return_code);
#endif
} /* start_msg_hdl() */
