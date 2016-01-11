
/* @(#)31	1.7.4.2  src/htx/usr/lpp/htx/bin/eservd/hang_monitor.c, eserv_daemon, htxubuntu 12/16/14 01:58:44 */

#include "eservd.h"
#include "global.h"

/*****************************************************************************/
/*****  h a n g _ m o n i t o r ( )  *****************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     hang_monitor()                                        */
/*                                                                           */
/* DESCRIPTIVE NAME =  Monitors HE's for hang condition.                     */
/*                                                                           */
/* OUTPUT =            Error messages in HTX error log for hangs.            */
/*                                                                           */
/* NORMAL RETURN =     0 to indicate successful completion                   */
/*                                                                           */
/* ERROR RETURNS =     -1 to indicate no hardware exerciser programs         */
/*                         currently in memory.                              */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = send_message - outputs a message to the screen and    */
/*                                    the message log file                   */
/*                                                                           */
/*    DATA AREAS =     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                     SEMHEKEY semaphores (identified by the semhe_id       */
/*                        variable)                                          */
/*                                                                           */
/*****************************************************************************/

void hang_monitor()
{
  /*
   ***  variable definitions  *************************************************
   */
    char                workstr[256];    /* work string                       */

    //extern char         *program_name;   /* this program's name (argv[0])     */

    //extern int          HANG_MON_PERIOD; /* hang monitor period               */
    //extern int          semhe_id;        /* semaphore id                      */

    //extern union shm_pointers shm_addr;  /* shared memory union pointers      */

    int                 i;               /* loop counter     */

    long                clock;           /* current time in seconds           */
    long                elasped_time;    /* current elased time in seconds    */
    long                max_update_lapse;  /* max time between updates        */

    short int           hung_toggle;     /* toggles between 0 and 1 each time
					  a multiple of max_update_lapse
					  is reached by a hung HE.          */

    static char         monitor_name[] = "hang_monitor";

    struct htxshm_HE    *p_htxshm_HE;    /* pointer to a htxshm_HE struct     */

    struct semid_ds     sembuffer;       /* semaphore buffer                  */

    struct sigaction    sigvector;       /* structure for signals */


    union shm_pointers  shm_addr_wk;     /* work ptr into shm                 */
    DBTRACE(DBENTRY,("enter hang_monitor.c hang_monitor\n"));

  /*
   ***  beginning of executable code  *****************************************
   */
    program_name = monitor_name;  /* to make "hang_monitor appear as program
				   name in messages */
    (void) send_message("hang_monitor process started.", 0, HTX_SYS_INFO,
			HTX_SYS_MSG);

  /*
   ***  Set default SIGNAL handling  ******************************************
   */
    sigemptyset(&(sigvector.sa_mask));  /* do not block signals         */
    sigvector.sa_flags = 0;         /* do not restart system calls on sigs */
    sigvector.sa_handler = SIG_DFL;
    for (i = 1; i <= SIGMAX; i++)
	(void) sigaction(i, &sigvector, (struct sigaction *) NULL);

    sigaddset(&(sigvector.sa_mask), SIGINT);
    sigaddset(&(sigvector.sa_mask), SIGQUIT);
    sigaddset(&(sigvector.sa_mask), SIGTERM);

    sigvector.sa_handler = (void (*)(int)) hang_sig_end;
    (void) sigaction(SIGINT, &sigvector, (struct sigaction *) NULL);
    (void) sigaction(SIGQUIT, &sigvector, (struct sigaction *) NULL);
    (void) sigaction(SIGTERM, &sigvector, (struct sigaction *) NULL);

  /*
   ***  set up shared memory addressing  *************************************
   */
    shm_addr_wk.hdr_addr = shm_addr.hdr_addr;  /* copy addr to work space  */
    (shm_addr_wk.hdr_addr)++;         /* skip over header                  */

  /*
   ***  Loop to check each HE for hang  ***************************************
   */
    while (TRUE)
    {
	clock = time((long *) 0);
	p_htxshm_HE = shm_addr_wk.HE_addr;
	for (i=0; i<(shm_addr.hdr_addr)->max_entries; i++, p_htxshm_HE++)
	{
      /* determine HE status *************************************/

		if ((p_htxshm_HE->PID != 0)
			&& (p_htxshm_HE->tm_last_upd != 0)
			&& ((p_htxshm_HE->max_cycles == 0) || ((p_htxshm_HE->max_cycles != 0) && (p_htxshm_HE->cycles < p_htxshm_HE->max_cycles)))
			&& ((semctl(semhe_id, 0, GETVAL, &sembuffer) == 0)
			&& (semctl(semhe_id, ((i * SEM_PER_EXER) + SEM_GLOBAL), GETVAL, &sembuffer) == 0)
			&& (semctl(semhe_id, ((i * SEM_PER_EXER) + SEM_GLOBAL + 1), GETVAL, &sembuffer) == 0))) {
			/* If here, then the HE we are looking at
			(1) Is not DEAD!
			(2) Is not stopped by main memu option 1 or 2!
			(3) Has not reached max_cycles!, and
			(4) Is not stopped on an ERROR! */

		elasped_time = clock - p_htxshm_HE->tm_last_upd;
		max_update_lapse = (long) (p_htxshm_HE->max_run_tm +
					   p_htxshm_HE->idle_time);
		if (elasped_time > max_update_lapse)    /* Overdue? */
		{
		    hung_toggle = (elasped_time / max_update_lapse) % 2;
		    if (p_htxshm_HE->hung_flag != hung_toggle)      /* Issue message? */
		    {
			p_htxshm_HE->hung_flag = hung_toggle;      /* Marked flag */
			(void) sprintf(workstr, "%s for %s is HUNG!\n"
				       "Max run time (set in mdt) = %d seconds.\n"
				       "Current elasped time = %d seconds.",
				       p_htxshm_HE->HE_name, p_htxshm_HE->sdev_id,
				       p_htxshm_HE->max_run_tm,
				       (int)(clock - p_htxshm_HE->tm_last_upd));
			(void) send_message(workstr, 0, HTX_HE_SOFT_ERROR, HTX_SYS_MSG);
		    } /* endif */
		} /* endif */
		else
		{ /* If here, HE marked hung but no longer overdue! */
		    p_htxshm_HE->hung_flag = 0;   /* clear hung flag */
		} /* endif */
	    } /* endif */
	} /* end for */
	(void) sleep(HANG_MON_PERIOD);
    } /* end while */

    DBTRACE(DBEXIT,("leave hang_monitor.c hang_monitor\n"));
} /* hang_monitor() */


/*
 * NAME: hang_sig_end()
 *
 * FUNCTION: Handles SIGTERM, SIGQUIT, and SIGINT signals.
 *
 */

void hang_sig_end(int signal_number, int code, struct sigcontext *scp)
     /*
      * signal_number -- the number of the received signal
      * code -- unused
      * scp -- pointer to context structure
      */

{
    char workstr[512];
    DBTRACE(DBENTRY,("enter hang_monitor.c hang_sig_end\n"));

    (void) sprintf(workstr, "Received signal %d.  Exiting...", signal_number);
    (void) send_message(workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
    DBTRACE(DBEXIT,("exit(0) hang_monitor.c hang_sig_end\n"));
    exit(0);

    /* this message should never show up... */
    DBTRACE(DBEXIT,("return hang_monitor.c hang_sig_end\n"));
    return;
} /* hang_sig_end() */
