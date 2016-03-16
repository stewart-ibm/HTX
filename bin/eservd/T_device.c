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
#include <unistd.h>

#if !defined(__HTX_LINUX__) && !defined(__OS400__)   /* 400 */
#define SEMCTL(x,y,z,a) (void)semctl(x,y,z,a)
#else
#define SEMCTL(x,y,z,a) semctl_arg.val=a, (void)semctl(x,y,z,semctl_arg)
#endif

#ifdef _DR_HTX_
#include <sys/dr.h>             /* DR reconfig changes */
#endif

#ifdef __OS400__
#include <qshell.h>
#define system QzshSystem
#endif

#if defined(__HTX_LINUX__) || defined(__OS400__)
extern char * regcmp (char *arg1, char *arg2);
extern char * regex (char *arg1, char *arg2);
#endif
extern void external_child_death(int child_PID);
/* extern thtx_message msg_rcv;   use definition from global.h */
extern char reg_exp[10];
/*
 ***  Externals global to this module **************************************
 */
extern union shm_pointers shm_addr;     /* shared memory union pointers      */
extern int semhe_id;            /* semaphore id                      */
extern int slow_shutd_wait;     /* configure from .htx_profile       */
extern int system_call;
extern char HTXPATH[];          /* HTX file system path         */
extern int editor_PID;
extern int shell_PID;
#ifdef _DR_HTX_
extern int semDR_id;
#endif

#define NO_EDITOR_SHELL		(!editor_PID) && (!shell_PID)

/*
 ***  Globals ***********************************************************
 */

/*****************************************************************************/
/*****  g e t _ d s t  *******************************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     get_dst                                               */
/*                                                                           */
/* DESCRIPTIVE NAME =  Get Device Status                                     */
/*                                                                           */
/* FUNCTION =          Determines the status of a device and returns it.     */
/*                                                                           */
/* INPUT =             Hardware Exerciser entry in shared memory structure.  */
/*                     Position of HE entry in shared memory structure.      */
/*                                                                           */
/* OUTPUT =            Device status.                                        */
/*                                                                           */
/* NORMAL RETURN =     Pointer to status string.                             */
/*                                                                           */
/* ERROR RETURNS =     None                                                  */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES =                                                       */
/*                                                                           */
/*    DATA AREAS =     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                     SEMHEKEY semaphores (identified by the semhe_id       */
/*                        variable)                                          */
/*                                                                           */
/*    MACROS =                                                               */
/*                                                                           */
/*****************************************************************************/
char *
get_dst (struct htxshm_HE *p_HE, int shm_pos, enum t_dev_status *status)
{
    static char *devstatus[11] = {       /* storage for returned status string */
      /* 0 */ "UK",
      /* 1 */ "CP",
      /* 2 */ "DD",
      /* 3 */ "ER",
      /* 4 */ "HG",
      /* 5 */ "RN",
      /* 6 */ "SG",
      /* 7 */ "ST",
      /* 8 */ "TM",
      /* 9 */ "DT",
      /* 10 */ "  "
    };

    struct semid_ds sembuffer;   /* semaphore buffer                  */
    long clock;                  /* current time in seconds           */
    DBTRACE(DBENTRY,("enter T_device.c get_dst\n"));

    *status = DST_UK;            /* init return status */

    if ((p_HE->PID == 0) && (ECGEXER_HDR (shm_pos)->started == 0)
	&& (p_HE->is_child))
	*status = DST_IN;
    else if (p_HE->PID == 0) {
	if (p_HE->DR_term == 1)
	    *status = DST_DT;
	else if (p_HE->user_term == 1)    /* did operator rq termination */
	    *status = DST_TM;      /* dead */
	else
	    *status = DST_DD;      /* terminated */

    }                            /* if */
   /* its not dead yet */
    else {
	clock = time ((long *) 0);        /* get current time */
	if (p_HE->user_term)
	    *status = DST_SG;      /* SIGTERM'd but not dead */
	else if ((semctl (ECGEXER_SEMID (shm_pos), 0, GETVAL,     /* system running ?  */
			  &sembuffer) != 0) || (semctl (ECGEXER_SEMID (shm_pos),  /* HE running ? */
							((ECGEXER_POS (shm_pos)
							  * SEM_PER_EXER) + SEM_GLOBAL), GETVAL,
							&sembuffer) != 0)) {
	    *status = DST_ST;      /* if not, HE is stopped. */
	}
	else if (semctl (ECGEXER_SEMID (shm_pos), /* stopped on error? */
			 ((ECGEXER_POS (shm_pos) * SEM_PER_EXER) + SEM_GLOBAL + 1),
			 GETVAL, &sembuffer)
		 != 0) {
	    *status = DST_ER;      /* HE is stopped on an error. */
	}
	else if ((p_HE->max_cycles != 0)  /* complete? */
		 &&(p_HE->cycles >= p_HE->max_cycles)) {
	    *status = DST_CP;      /* HE is done. */
	}
	else if ((clock - p_HE->tm_last_upd) >
		 ((long)(p_HE->max_run_tm + p_HE->idle_time))) {
	    *status = DST_HG;      /* HE is hung. */
	}
	else {
	    *status = DST_RN;      /* HE is running. */
	}
    }                            /* else */

    DBTRACE(DBEXIT,("return T_device.c get_dst\n"));
    return (devstatus[*status]);
}                               /* get_dst */

/*****************************************************************************/
/*****  t e r m i n a t e _ e x e r c i s e r  *******************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     terminate_exerciser                                   */
/*                                                                           */
/* DESCRIPTIVE NAME =  Terminate hardware exerciser                          */
/*                                                                           */
/* FUNCTION =          Kills a hardware exerciser by sending it a SIGTERM    */
/*                     signal.                                               */
/*                                                                           */
/* INPUT =             p_HE       - points to HE share memory entry          */
/*                     shm_pos - position of HE entry in shared memory.      */
/*                               This is used to compute the number of the   */
/*                               activate/halt and the stop-on-error sema-   */
/*                               phore numbers for that HE.                  */
/*                                                                           */
/* OUTPUT =            kills hardware exerciser programs                     */
/*                                                                           */
/*                     Updated system semaphore structure to reflect the     */
/*                     state of the HE's which are killed.                   */
/*                                                                           */
/* NORMAL RETURN =     N/A - void function                                   */
/*                                                                           */
/* ERROR RETURNS =     N/A - void function                                   */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*                                                                           */
/*    OTHER ROUTINES =                                                       */
/*                     send_message() - outputs a message to the screen and  */
/*                                     the message log file                  */
/*                                                                           */
/*    DATA AREAS =     HTXPATH - pointer to a string which contains the path */
/*                         to the top level of the HTX file hierarchy        */
/*                         (usually "/usr/lpp/htx").                         */
/*                     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                                                                           */
/*    MACROS =         PRTMSG1 - Print message CURSES macro                  */
/*                              (defined in common/hxiconv.h)                */
/*                     CLRLIN                                                */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
int
terminate_exerciser (struct htxshm_HE *p_HE, int shm_pos, int flag,
                     char *result_msg)
{
    struct semid_ds sembuffer;   /* semaphore buffer                  */
    boolean_t dummy;
    char *msg_text;
    char reg_expr[256];
    int rc = 0, rc_res = 0;
    int kill_errno;
    int max_entries, PID;
    int semval, do_pause = 0;
    DBTRACE(DBENTRY,("enter T_device.c terminate_exerciser\n"));

  /***************************************************************************/
  /**  beginning of executable code  *****************************************/
  /***************************************************************************/

    msg_text = malloc (MSG_TEXT_SIZE);

   /* make max_entries local so that it does not change on us */
    max_entries = ECG_MAX_ENTRIES;
    print_log(LOGMSG,"terminate_exerciser called for %s\n", p_HE->sdev_id);

   /* save run state */
    semval =
      semctl (ECGEXER_SEMID (shm_pos), (ECGEXER_POS (shm_pos) * SEM_PER_EXER + SEM_GLOBAL),
	      GETVAL, &sembuffer);

    PID = p_HE->PID;             /* improve atomicity */
   /* if the exerciser is not dead */
   /* and system is running */
    if (PID != 0
	&& p_HE->tm_last_upd != 0
	&& (semctl (ECGEXER_SEMID (shm_pos), 0, GETVAL, &sembuffer) == 0))
    {                            /* global wait */
      /*
	 && (semctl(semhe_id, (shm_pos * SEM_PER_EXER + SEM_GLOBAL), GETVAL, &sembuffer) == 0)      )
       */
      /* HE running */
      /* make sure that HE is set to running state */
	SEMCTL (ECGEXER_SEMID (shm_pos), (ECGEXER_POS (shm_pos) * SEM_PER_EXER + SEM_GLOBAL),
		SETVAL, 0);
	SEMCTL (ECGEXER_SEMID (shm_pos), (ECGEXER_POS (shm_pos) * SEM_PER_EXER + SEM_GLOBAL + 1),
		SETVAL, 0);
	(void) sprintf (msg_text, "Termination of %s requested by operator",
			p_HE->sdev_id);
	(void) send_message (msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	(void) alarm (slow_shutd_wait);   /* config in .htx_profile */
	if (!flag) {
	    p_HE->user_term = 1;
	    p_HE->DR_term = 0;
	}                         /* affects status display */
	else {
	    p_HE->user_term = 0;
	    p_HE->DR_term = 1;
	}                         /* affects status display */
	p_HE->max_cycles = 0;
	//if (!flag) {
	    //(void) wcolorout(stdscr, NORMAL | B_BLACK | F_WHITE);
	    //CLRLIN(MSGLINE, 0);
	    //PRTMSG1(MSGLINE, 69, ("waiting..."));
	    //}
#if !defined(__OS400__)  /* 400 */
	if (getpid () == getpgid (PID))
	    do_pause = 1;
	print_log(LOGMSG,"pid = %d pgid = %d daemon_pid=%d\n", PID, getpgid (PID),
		getpid ());
#else
	do_pause = 1;
#endif
	rc = kill (PID, SIGTERM);
	kill_errno = errno;       /* save it for possible error msg */
	if (rc == 0) {
	    if (do_pause)
		(void) pause ();    /* wait for any signal */
	    else
		external_child_death (PID);
#if !defined(__OS400__)  /* 400 */
	    if (!alarm (0)) {
		print_log(LOGMSG," not alarm\n");
		fflush (stdout);
		(void) sprintf (msg_text,
				"Sending SIGKILL to %s for %s after %d seconds.",
				p_HE->HE_name, p_HE->sdev_id, slow_shutd_wait);
		(void) send_message (msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);
		(void) kill (PID, SIGKILL); /* no mercy */
	    }                      /* if */
#endif
	    sleep (1);
/* ((p_shm_hdr)->num_entries)++; *//* let child_death do it */
	    print_log(LOGMSG," execing script \n");
	    fflush (stdout);
	    strcpy (reg_expr, "^");
	    strcat (reg_expr, p_HE->HE_name);
	    strcat (reg_expr, ".*cleanup[\t ]*$");
	    rc = exec_HE_script (reg_expr, p_HE->sdev_id, &dummy);
	    print_log(LOGMSG," execed script \n");
	    fflush (stdout);
	}                         /* endif */
	else {
	    (void) alarm ((unsigned) 0);   /* cancel alarm               */
	    p_HE->user_term = 0;   /* affects status display */
	    p_HE->DR_term = 0;     /* affects status display */
	    sprintf (msg_text, "Failed to terminate %s (PID %d) for %s\n",
		     p_HE->HE_name, PID, p_HE->sdev_id);
	    (void) send_message (msg_text, 0, HTX_HE_SOFT_ERROR, HTX_SYS_MSG);
	    msg_text[strlen (msg_text) - 1] = '\0';        /* clip off new line */
	    //PRTMSG(MSGLINE, 0, ("%s.  See HTX error log.", msg_text));
	    sprintf (result_msg, "%s.  See HTX error log.", msg_text);
	    rc_res = -1;
	    if (kill_errno == ESRCH) {     /* no such process */
		sprintf (msg_text,
			 "No such process.  Perhaps it is already deceased?\n");
		strcat (result_msg,
			"No such process.  Perhaps it is already deceased?\n");
		rc_res = -1;
	    }
	    else
		sprintf (msg_text, "Errno = %d.\n", kill_errno);
	    (void) send_message (msg_text, 0, HTX_HE_SOFT_ERROR, HTX_SYS_MSG);
	}                         /* else */

      /* restore HE activate/halt state  */
	SEMCTL (ECGEXER_SEMID (shm_pos), (ECGEXER_POS (shm_pos) * SEM_PER_EXER + SEM_GLOBAL),
		SETVAL, semval);

    }                            /* endif PID != 0 */
    else {
	//PRTMSG(MSGLINE, 0, ("System must be running to terminate exercisers."));
	print_log(LOGMSG,"System must be running to terminate exercisers.");
	sprintf (result_msg, "%s",
		 "System must be running to terminate exercisers.");
	rc_res = -1;

    }                            /* else */

    free (msg_text);
    msg_text = NULL;

    DBTRACE(DBEXIT,("return T_device.c terminate_exerciser\n"));
    return rc_res;
}                               /* terminate_exerciser */

#ifdef _DR_HTX_
void
DR_handler (int sig, int code, struct sigcontext *scp)
{
    FILE *fp, *fpdr;
    char workstr[128];           /* work string  */
    char cmd[128], filename[128], mdtfile[128];

    int i, j, rc = 0;            /* loop counter                      */
    int num_entries;             /* local number of entries           */
    static int n_cpus_dr = 0;    /* number of cpus before DR                     */
    int cpus_dr;                 /* number of cpus after DR         */
    int num_tries = 0;

    struct htxshm_hdr *hdr = 0;

    dr_info_t DRinfo;
    DBTRACE(DBENTRY,("enter T_device.c DR_handler\n"));

    hdr = (struct htxshm_hdr *) shm_addr.hdr_addr;
    if (hdr->shutdown == 1) {
	sprintf (workstr,
		 "DR operation while HTX shutdown..No action by Supervisor\n");
	(void) send_message (workstr, errno, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
	DBTRACE(DBEXIT,("return/a T_device.c DR_handler\n"));
	return;
    }


    do {
	rc = dr_reconfig (DR_QUERY, &DRinfo);
    } while ((rc < 0) && (errno == 4));
    if (rc < 0) {
	if (NO_EDITOR_SHELL)
	    print_log(LOGMSG,"dr_reconfig system call failed...");
	sprintf (workstr, "dr_reconfig system call failed..errno = %d", errno);
	(void) send_message (workstr, errno, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
	sleep (1);
	//if (NO_EDITOR_SHELL) CLRLIN(MSGLINE, 0);
	DBTRACE(DBEXIT,("return/b T_device.c DR_handler\n"));
	return;
    }

    sprintf (workstr, "dr_reconfig output:\
		     pre:%d, check:%d, doit:%d, post:%d, posterror:%d,\
		     add: %d, rem:%d, cpu:%d, mem:%d, lcpu:%d, bcpu:%d\n", DRinfo.pre, DRinfo.check, DRinfo.doit, DRinfo.post, DRinfo.posterror, DRinfo.add, DRinfo.rem, DRinfo.cpu, DRinfo.mem, DRinfo.lcpu, DRinfo.bcpu);
    (void) send_message (workstr, errno, HTX_SYS_INFO, HTX_SYS_MSG);


    if (DRinfo.cpu != 1) {
	DBTRACE(DBEXIT,("return/c T_device.c DR_handler\n"));
	return;
    }

    if (DRinfo.check == 1) {
	if (NO_EDITOR_SHELL)
	    print_log(LOGMSG,"DR operation in CHECK phase..");
	workstr[0] = 0;
	sprintf (workstr, "DR operation in CHECK phase..");
	sleep (1);
	(void) send_message (workstr, errno, HTX_SYS_INFO, HTX_SYS_MSG);
	//if (NO_EDITOR_SHELL) CLRLIN(MSGLINE, 0);
	DBTRACE(DBEXIT,("return/d T_device.c DR_handler\n"));
	return;
    }

    if (DRinfo.pre == 1) {
	if (NO_EDITOR_SHELL)
	    print_log(LOGMSG,"DR operation in PRE phase..");
	workstr[0] = 0;

	if ((fp = popen ("lsdev -C | grep proc | wc -l", "r")) == NULL) {
	    //if (NO_EDITOR_SHELL) CLRLIN(MSGLINE, 0);
	    if (NO_EDITOR_SHELL)
		print_log(LOGERR,"popen error in DR handler");
	    workstr[0] = 0;
	    sprintf (workstr, "popen error in DR handler");
	    (void) send_message (workstr, errno, HTX_SYS_INFO, HTX_SYS_MSG);
	    sleep (1);
	    //if (NO_EDITOR_SHELL) CLRLIN(MSGLINE, 0);
	    n_cpus_dr = 0;
	    DBTRACE(DBEXIT,("return/e T_device.c DR_handler\n"));
	    return;
	}

	workstr[0] = 0;
	if (fgets (workstr, 1024, fp) == NULL) {
	    sprintf (workstr, "fgets does not give n_cpus_dr in DR handler");
	    (void) send_message (workstr, errno, HTX_SYS_INFO, HTX_SYS_MSG);
	    n_cpus_dr = 0;
	    pclose (fp);
	    DBTRACE(DBEXIT,("return/f T_device.c DR_handler\n"));
	    return;
	}

	sscanf (workstr, "%d", &n_cpus_dr);
	pclose (fp);

	sprintf (workstr, "DR operation in PRE phase..No. of CPUs = %d",
		 n_cpus_dr);
	(void) send_message (workstr, errno, HTX_SYS_INFO, HTX_SYS_MSG);
	//if (NO_EDITOR_SHELL) CLRLIN(MSGLINE, 0);

	DBTRACE(DBEXIT,("return/g T_device.c DR_handler\n"));
	return;
    }                            /* End of PRE phase operation */

    if ((DRinfo.post == 1) || (DRinfo.posterror == 1)) {


	if (NO_EDITOR_SHELL)
	    print_log(LOGMSG,"DR operation in POST/POST-Error phase..");
	workstr[0] = 0;
	sprintf (workstr, "DR operation in POST/POST-Error phase..");
	sleep (1);
	(void) send_message (workstr, errno, HTX_SYS_INFO, HTX_SYS_MSG);
	//if (NO_EDITOR_SHELL) CLRLIN(MSGLINE, 0);

	SEMCTL (semDR_id, 0, SETVAL, 1);
	do {
	    sleep (1);
	    if ((fp = popen ("lsdev -C | grep proc | wc -l", "r")) == NULL) {
		//if (NO_EDITOR_SHELL) CLRLIN(MSGLINE, 0);
		if (NO_EDITOR_SHELL)
		    print_log(LOGERR,"popen error in DR handler");
		workstr[0] = 0;
		sprintf (workstr, "popen error in DR handler in POST phase");
		(void) send_message (workstr, errno, HTX_SYS_HARD_ERROR,
				     HTX_SYS_MSG);
		sleep (1);
		//if (NO_EDITOR_SHELL) CLRLIN(MSGLINE, 0);
		SEMCTL (semDR_id, 0, SETVAL, 0);
		DBTRACE(DBEXIT,("return/h T_device.c DR_handler\n"));
		return;
	    }

	    workstr[0] = 0;
	    if (fgets (workstr, 1024, fp) == NULL) {
		SEMCTL (semDR_id, 0, SETVAL, 0);
		pclose (fp);
		DBTRACE(DBEXIT,("return/i T_device.c DR_handler\n"));
		return;
	    }

	    sscanf (workstr, "%d", &cpus_dr);
	    pclose (fp);
	} while ((cpus_dr == n_cpus_dr) && (num_tries++ < 10)
		 && (DRinfo.posterror != 1));

	if ((num_tries >= 10) && (n_cpus_dr > 0)) {
	    sprintf (workstr, "In PostPhase DR, ODM still not updated, %d, %d\n",
		     cpus_dr, n_cpus_dr);
	    (void) send_message (workstr, errno, HTX_SYS_HARD_ERROR,
				 HTX_SYS_MSG);
	    SEMCTL (semDR_id, 0, SETVAL, 0);
	    DBTRACE(DBEXIT,("return/j T_device.c DR_handler\n"));
	    return;
	}

      /* Create a new MDT having only processor stanzas */

	sprintf (cmd, "cat /dev/null | htxconf.awk > /usr/lpp/htx/mdt/mdt.%d",
		 getpid ());
	system_call = TRUE;
	system (cmd);
	system_call = FALSE;

      /* Restart the exercisers as formed in the new MDT file */
	mdtfile[0] = 0;
	workstr[0] = 0;
	sprintf (mdtfile, "mdt.%d", getpid ());
	if (NO_EDITOR_SHELL)
	    print_log(LOGMSG,"Restarting effected xercisers..");
	sprintf (workstr, "Restarting effected xercisers from MDT file %s %d",
		 mdtfile, cpus_dr);
	(void) send_message (workstr, errno, HTX_SYS_INFO, HTX_SYS_MSG);
	reconfig_restart (mdtfile);
	if (NO_EDITOR_SHELL)
	    print_log(LOGMSG,"DR restart done..");
	usleep (500);
	//if (NO_EDITOR_SHELL) CLRLIN(MSGLINE, 0);

	SEMCTL (semDR_id, 0, SETVAL, 0);

	DBTRACE(DBEXIT,("return/k T_device.c DR_handler\n"));
	return;
    }                            /* End of POST and POSTERROR phase of DR */

    DBTRACE(DBEXIT,("return T_device.c DR_handler\n"));
    return;
}
#endif

/*****************************************************************************/
/*****  T _ d e v i c e  *****************************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     T_device                                              */
/*                                                                           */
/* DESCRIPTIVE NAME =  Terminate hardware exerciser                          */
/*                                                                           */
/* FUNCTION =          Kills a hardware exerciser by sending it a SIGTERM    */
/*                     signal.                                               */
/*                                                                           */
/* INPUT =             Operator specifies device to terminate.               */
/*                                                                           */
/* OUTPUT =            kills hardware exerciser programs                     */
/*                                                                           */
/*                     Updated system semaphore structure to reflect the     */
/*                     state of the HE's which are killed.                   */
/*                                                                           */
/* NORMAL RETURN =     0 to indicate successful completion                   */
/*                                                                           */
/* ERROR RETURNS =     -1 to indicate no hardware exerciser programs         */
/*                         currently in memory.                              */
/*                     -2 to indicate an unsuccessful attempt to allocate    */
/*                         memory for the activate/halt device table.        */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = display_scn -  displays screen image                  */
/*                     init_ahd_tbl - initializes and sorts the list of      */
/*                                    devices (HEs)                          */
/*                     send_message - outputs a message to the screen and    */
/*                                    the message log file                   */
/*                     get_dst - returns status of a device in test.         */
/*                     terminate_exerciser - kills an HE and runs cleanup    */
/*                                    scripts by exec_HE_scripts().          */
/*                                                                           */
/*    DATA AREAS =     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                     SEMHEKEY semaphores (identified by the semhe_id       */
/*                        variable)                                          */
/*                     __loc1 - pointer to reg. expression matching string.  */
/*                                                                           */
/*    MACROS =         CLRLIN - Clear line CURSES macro                      */
/*                              (defined in common/hxiconv.h)                */
/*                     PRTMSG - Print message CURSES macro                   */
/*                              (defined in common/hxiconv.h)                */
/*                                                                           */
/*****************************************************************************/

int
T_device (char *result_msg, char **dev_list, int num_list, int *num_disp)
{
  /***************************************************************************/
  /**  variable definitions  *************************************************/
  /***************************************************************************/
    char *cmp_regexp;            /* ptr to compiled reg expression    */
    char ret[3];                 /* return array for regex()          */
    char *work;                  /* work int */
    char workstr[128];           /* work string                       */

    enum t_dev_status dev_status;        /* return from get_dst               */
    extern char *__loc1;


    int i, ii;                   /* loop counter                      */
    int max_strt_ent;            /* max starting entry                */
    int num_entries = 0;         /* local number of shm HE entries    */
    int strt_ent = 1;            /* first entry on screen             */
    int workint;

    struct ahd_entry *p_td_table = NULL; /* points to begin td seq tbl    */
    struct ahd_entry *p_into_table;      /* points into td sequence table     */

    struct htxshm_HE *p_htxshm_HE;       /* pointer to a htxshm_HE struct     */


    int num_found, found, tmp, rc = 0;
    int loop, max_loops, max_found = 0, ecg_cnt, found_ecg;
    DBTRACE(DBENTRY,("enter T_device.c T_device\n"));

  /***************************************************************************/
  /**  beginning of executable code  *****************************************/
  /***************************************************************************/

    memset (result_msg, 0, 80);

    sprintf (result_msg, "T_device:No Valid device or ecg name provided");
    strt_ent = (msg_rcv.indx > 0) ? msg_rcv.indx : 1;

    *num_disp = 0;
    max_found = 0;
    loop = 0;

    if (num_list == 0) {
	sprintf (result_msg, "T_device:No valid exerciser list or ECG name provided. Kindly make sure that ECG is not in Unloaded/Inactive state.");
	DBTRACE(DBEXIT,("return/a -1 T_device.c T_device\n"));
	return -1;
    }

    if (num_list == -1)
	max_loops = 1;
    else {
	if (num_ecg == 0) {
	    max_loops = 1;
	    strcpy (ecg[0], "/ecg.all");
	}
	else
	    max_loops = num_ecg;
    }

    print_log(LOGMSG,"max_loop = %d\n", max_loops);

    for (loop = 0; loop < max_loops; loop++) {
      /********** for command line client *********/
	if (num_list != -1) {     //If selected ecgs have to be searched
	  for (ecg_cnt = 0; ecg_cnt < num_ecgs; ecg_cnt++) {
	      found_ecg = 0;
	      cur_ecg_pos = ecg_cnt;
	      //print_log(LOGMSG,"T_device: num_list = %d ECGNAME = :%s: ecgname = :%s:\n",num_list, ECGNAME,ecg[loop]);
	      PUT_FULL_ECG;
	      if (strcmp (full_name, ecg[loop]) == 0) {
		  print_log(LOGMSG,"T_device: found ECGNAME = :%s/%s: ecgname = :%s:\n",
			  ECGPATH, ECGNAME, ecg[loop]);
		  cur_ecg_pos = ecg_cnt;
		  found_ecg = 1;
		  break;
	      }
	  }
	}
      /********** Done: for command line client *********/
	else if (num_list == -1) {
	    found_ecg = 1;
	    print_log(LOGMSG,"name = %s/%s status  %s\n", ECGPATH, ECGNAME, ECGSTATUS);
	}

	if (!found_ecg)
	    continue;
	found_ecg = 0;


	REM_CUR->max_entries = ECG_MAX_ENTRIES;

	if (ECG_MAX_ENTRIES == 0) {       /* no HE programs?        */
	    print_log(LOGMSG,"There are no Hardware Exerciser programs currently defined.");
	    sprintf (result_msg,
		     "T_device:There are no Hardware Exerciser programs currently defined.");
	    continue;              //return(-1);
	}
	else {                    /* some HE's defined.                */
	    num_entries = init_ahd_tbl (&p_td_table);
	    REM_CUR->num_entries = num_entries;
	    if (num_entries <= 0) {
		sprintf (result_msg,
			 "T_device:Number of Entries in %s/%s is %d\n", ECGPATH,
			 ECGNAME, num_entries);
		print_log(LOGMSG,"T_device:Number of Entries in %s/%s is %d\n", ECGPATH,
			ECGNAME, num_entries);
		if (p_td_table != NULL) {
		    free (p_td_table);       /* release memory for td order tbl */
		    p_td_table = NULL;
		}
		continue;           //return(num_entries);
	    }
	 /* problem in init fcn - bye!      */
	    else {                 /* init fcn ran ok.                */
		if (num_entries <= 13) {    /* 13 or fewer entries?          */
		    max_strt_ent = 1;
		}
		else {              /* more than 13 entries          */
		    max_strt_ent = num_entries;
		}                   /* endif */
	    }                      /* endif */
	}                         /* endif */

      /***********************************************************************/
      /**  build screen data for the current strt_ent  ***********************/
      /***********************************************************************/
	p_into_table = p_td_table + strt_ent - 1;
	*num_disp = num_entries - strt_ent + 1;
	if (*num_disp > 13)
	    *num_disp = 13;

      /*if (num_list >0) {
	 *num_disp = num_list;
	 }
	 else if (num_list == -2) {
	 *num_disp = num_entries;
	 } */
	*num_disp =
	  (num_list >
	   0) ? num_list : ((num_list == -2) ? num_entries : *num_disp);

	print_log(LOGMSG,"T_device:*num_disp = %d num-entries = %d\n", *num_disp,
		num_entries);
	fflush (stdout);

	num_found = -1;
	for (i = 0; i < num_entries; i++) {

	    if (num_list >= 0) {
		found = 0;
		for (tmp = 0; tmp < *num_disp; tmp++) {
		    // print_log(LOGMSG,"loop: dev = %s list = %s\n",p_htxshm_HE->sdev_id,dev_list[tmp]);
		    fflush (stdout);
		    p_htxshm_HE = ECGEXER_ADDR (p_into_table->shm_pos);
		    if (strcmp (p_htxshm_HE->sdev_id, dev_list[tmp]) == 0) {
			print_log(LOGMSG,"found: dev = %s list = %s\n", p_htxshm_HE->sdev_id,
				dev_list[tmp]);
			fflush (stdout);
			found = 1;
			num_found++;
			break;
		    }
		}

		if (found != 1) {
		    p_into_table++;
		    continue;
		}
	    }
	    else {
		num_found++;
		if (i >= *num_disp)
		    break;
		p_htxshm_HE = ECGEXER_ADDR (p_into_table->shm_pos);
	    }


			  /*** Device Status ********************************************/
	    work = get_dst (p_htxshm_HE, p_into_table->shm_pos, &dev_status);
	    workint = strlen (work);       /* length of status string */
	    (void) strncpy (workstr, "        ", 8 - workint);     /* spaces */
	    workstr[8 - workint] = '\0';

	 /* set display attributes based on status */
	    sprintf (INFO_SEND_2 (max_found + num_found).status, work);

	 /* set color for device id based on status */
	    htx_strcpy (INFO_SEND_2 (max_found + num_found).slot_port,
			p_htxshm_HE->slot_port);
	    htx_strcpy (INFO_SEND_2 (max_found + num_found).sdev_id,
			p_htxshm_HE->sdev_id);
	    htx_strcpy (INFO_SEND_2 (max_found + num_found).adapt_desc,
			p_htxshm_HE->adapt_desc);
	    htx_strcpy (INFO_SEND_2 (max_found + num_found).device_desc,
			p_htxshm_HE->device_desc);


	    p_into_table++;
	}                         /* endfor */

	if (num_found == -1) {
	    sprintf (result_msg, "%s", "No valid exerciser list provided");
	    if (p_td_table != NULL) {
		free (p_td_table);  /* release memory for td order tbl */
		p_td_table = NULL;
	    }
	    DBTRACE(DBEXIT,("return/b -1 T_device.c T_device\n"));
	    return -1;
	}
	fflush (stdout);
	num_found = -1;

	switch (msg_rcv.cmd) {
	    default:
		p_into_table = p_td_table;
		num_found = -1;
		for (i = 0; i < num_entries; i++) {
		    p_htxshm_HE = ECGEXER_ADDR (p_into_table->shm_pos);
		    found = 0;
		    if (num_list > 0) {
			//print_log(LOGMSG,"cmp:dev =%s:list = %s\n",p_htxshm_HE->sdev_id,dev_list[tmp]);
			fflush (stdout);
			for (tmp = 0; tmp < *num_disp; tmp++) {
			    if (strcmp (dev_list[tmp], p_htxshm_HE->sdev_id) == 0) {
				print_log(LOGMSG,"found to terminate: dev = %s list = %s\n",
					p_htxshm_HE->sdev_id, dev_list[tmp]);
				fflush (stdout);
				found = 1;
				num_found++;
				break;
			    }
			}

			if (found != 1) {
			    p_into_table++;
			    continue;
			}
			else {
			    if (p_htxshm_HE->PID != 0) {
				rc =
				  terminate_exerciser (p_htxshm_HE,
						       p_into_table->shm_pos, 0,
						       result_msg);
				sleep (2);
				work =
				  get_dst (p_htxshm_HE, p_into_table->shm_pos,
					   &dev_status);
				sprintf (INFO_SEND_2 (max_found + num_found).status,
					 work);
			    }             /* endif */

			    else {
				print_log(LOGMSG,"T_device:The exerciser for %s, %s is already deceased.",
				   p_htxshm_HE->sdev_id, p_htxshm_HE->HE_name);
				sprintf (result_msg,
					 "The exerciser for %s, %s is already deceased.",
					 p_htxshm_HE->sdev_id, p_htxshm_HE->HE_name);
				rc = -1;
			    }             /* else */
			}
		    }
		    else if (num_list == -2) {
			num_found++;
			//p_htxshm_HE = ECGEXER_ADDR(p_into_table->shm_pos);
			if (p_htxshm_HE->PID != 0) {
			    rc =
			      terminate_exerciser (p_htxshm_HE, p_into_table->shm_pos,
						   0, result_msg);
			    sleep (2);
			    work =
			      get_dst (p_htxshm_HE, p_into_table->shm_pos,
				       &dev_status);
			    sprintf (INFO_SEND_2 (max_found + num_found).status, work);
			}                /* endif */

			else {
			    print_log(LOGMSG,"T_device:The exerciser for %s, %s is already deceased.",
			       p_htxshm_HE->sdev_id, p_htxshm_HE->HE_name);
			    sprintf (result_msg,
				     "The exerciser for %s, %s is already deceased.",
				     p_htxshm_HE->sdev_id, p_htxshm_HE->HE_name);
			    rc = -1;
			}                /* else */
		    }
	      /******************** LOOP FOR GUI ************/
		    else if (msg_rcv.subcmd == SUBCMD_w) {
			print_log(LOGMSG,"Loop number 3\n");
			cmp_regexp = regcmp (reg_exp, (char *) 0);       /* compile exp */
			if (cmp_regexp != NULL) {        /* regular expression OK? */
			    for (ii = 1; ii <= *num_disp; ii++) {
				(void) sprintf (workstr, "%-d", ii);
				work = regex (cmp_regexp, workstr);
				if (work != NULL) {
				    (void) strncpy (ret, " ", (size_t) sizeof (ret));
			/* clear return (ret) array */
				    (void) strncpy (ret, __loc1,
						    (size_t) (work - __loc1));
				    if (strcmp (workstr, ret) == 0) {

					p_into_table =
					  p_td_table + (strt_ent - 1) + (ii - 1);
					p_htxshm_HE = ECGEXER_ADDR (p_into_table->shm_pos);
					//num_found = ii-1;
					num_found++;
					print_log(LOGMSG,"In loop... Exer = %s\n",
						p_htxshm_HE->sdev_id);
					if (p_htxshm_HE->PID != 0) {
					    rc =
					      terminate_exerciser (p_htxshm_HE,
								   p_into_table->shm_pos,
								   0, result_msg);
					    sleep (2);
					    work =
					      get_dst (p_htxshm_HE, p_into_table->shm_pos,
						       &dev_status);
					    sprintf (INFO_SEND_2 (ii - 1).status, work);
					}    /* endif */

					else {
					    print_log(LOGMSG,"The exerciser for %s, %s is already deceased.",
					       p_htxshm_HE->sdev_id, p_htxshm_HE->HE_name);
					    sprintf (result_msg,
						     "The exerciser for %s, %s is already deceased.",
						     p_htxshm_HE->sdev_id,
						     p_htxshm_HE->HE_name);
					    rc = -1;
					}    /* else */
					//num_found = msg_rcv.subcmd-1;
				    }
				}
			    }
			}
		    }

	      /******************** LOOP FOR GUI ************/
		    if (num_list == -1)
			break;
		    p_into_table++;
		}
		//if (num_list != -1)
		    //   *num_disp = num_found + 1;
		if (num_list != -1) {
		    max_found = max_found + num_found + 1;
		    *num_disp = max_found;
		}

		break;

	}                         /* endswitch */
    }
    if (p_td_table != NULL) {
	free (p_td_table);        /* release memory for td order tbl */
	p_td_table = NULL;
    }
    DBTRACE(DBEXIT,("return T_device.c T_device\n")); /* fix this to print rc ? */
    return rc;
}                               /* T_device */
