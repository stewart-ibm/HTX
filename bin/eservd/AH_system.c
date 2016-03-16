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


#if !defined(__OS400__)   /* 400 */
#define  SIGMAX  (SIGRTMAX)
#endif

#if (!defined(__HTX_LINUX__))  && (!defined(__OS400__)) /* 400 */
#define SEMCTL(x,y,z,a) (void)semctl(x,y,z,a)
#else
#define SEMCTL(x,y,z,a) semctl_arg.val=a, (void)semctl(x,y,z,semctl_arg)
#endif

/*
 ***  Externals global to this module **************************************
 */
extern  char  HTXPATH[];             /* HTX file system path         */
extern int load_exerciser(struct htxshm_HE *p_HE, struct sigaction *sigvec);
extern int sem_length;

/*
 ***  Globals ***********************************************************
 */
/*

 ***  The following sembuf structure must be global for correct child  ********
 ***     death processing.  ***************************************************
 */


/*****************************************************************************/
/*****  A H _ s y s t e m ( )  ***********************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     AH_system.c                                           */
/*                                                                           */
/* DESCRIPTIVE NAME =  Activate/Halt the htx system                          */
/*                                                                           */
/* FUNCTION =          Activates/Halts the htx system (all hardware          */
/*                     exercisers defined in the mdt)                        */
/*                                                                           */
/* INPUT =             autoflag - auto startup flag.                         */
/*                     mdt attribute file (/usr/lpp/htx/mdt/mdt).            */
/*                                                                           */
/* OUTPUT =            Loads hardware exerciser programs on start-up         */
/*                                                                           */
/*                     Updated system semaphore structure to activate/halt   */
/*                     all hardware exerciser programs                       */
/*                                                                           */
/* NORMAL RETURN =     N/A - void function                                   */
/*                                                                           */
/* ERROR RETURNS =     N/A - void function                                   */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = load_compar() - compares mdt entries for load         */
/*                                     sequence sort                         */
/*                     send_message() - outputs a message to the screen and  */
/*                                     the message log file                  */
/*                     load_exerciser() - runs setup scripts and spawns      */
/*                                     hardware exerciser.                   */
/*                                                                           */
/*    DATA AREAS =     HTXPATH - pointer to a string which contains the path */
/*                         to the top level of the HTX file hierarchy        */
/*                         (usually "/usr/lpp/htx").                         */
/*                     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                     SEMHEKEY semaphores (identified by the semhe_id       */
/*                        variable)                                          */
/*                     stress_dev - pointer to a string which contains the   */
/*                         name of the "heartbeat" device for stress lab     */
/*                         testing.  A null string means that no "heartbeat" */
/*                         is require -- non-stress test.                    */
/*                     stress_cycle - pointer to a string which contains the */
/*                         number of seconds between "heartbeats".           */
/*                         A null string means that no "heartbeat" is        */
/*                         required.                                         */
/*                                                                           */
/*    MACROS =         PRTMSG - Print message CURSES macro                   */
/*                              (defined in common/hxiconv.h)                */
/*                                                                           */
/*****************************************************************************/


/*
 ***  begining of AH_system definition  ***************************************
 */
int AH_system(int autoflag, char *result_msg)
{
  /*
   ***  variable/function declarations  **************************************
   */
    char          display_str[64];       /* display device name               */
    char          workstr[512];          /* work string                       */

    int           frkpid;                /* fork PID                          */
    int           hf_devid;              /* hf device id */
    int           hft;                   /* hft number */
    int           hft_fileid;            /* hft file descriptor. */
    int           i;                     /* loop counter                      */
    int           j;                     /* loop counter                      */
    int           num_entries;           /* local number of entries variable  */
    int           rc;                    /* general return code               */
    int           semval;                /* semaphore value                   */
    int           semval_add;            /* semaphore value                   */
    int           tcseta_rc;             /* return code for TCSETA ioctl()    */
    int           VT[4];                 /* virtual terminal array of indexes */

    int           *prc_ah,rc_ah = 0;
    long          clock;                 /* current time in seconds.          */
    char          mycommand[512];        /* command string used in system() function */
    char          mydate[25];
    FILE          *fp;

    int delay_counter;
    int delay_slice = 2;
    int exer_system_halted;
    int exer_error_halted;
    int exer_operation_halted;
    int exer_exited;
    int exer_count;
    int exer_shm_entries;
    int all_exer_stopped;


#ifdef HTX_REL_tu320
    static struct hfchgdsp hfchgdsp =
    {
	HFINTROESC, HFINTROLBR, HFINTROEX, 0, 0, 0, sizeof(struct hfchgdsp)-3,
	HFCHGDSPCH, HFCHGDSPCL, 2, 0,
	0,0,  0,0,0,0,0,0,0,0,  0,0,0,0,  0,0,0,0,0,0,0,0,0,0
    };
#endif

    static struct sembuf halt_sops[1] =
    { /* halt system semaphore operation */
	{ 0, 1, SEM_UNDO}
    };

#ifdef HTX_REL_tu320
    struct hftgetid hfid;                /* get id structure */

    struct hftsmgrcmd mgrcmd;            /* command struct to screen manager. */
#endif

    struct htxshm_HE *p_HE;              /* pointer to HE struct in shm       */

    struct load_tbl *load_tbl_ptr;       /* pointer to load sequence table    */

    struct semid_ds sembuffer;           /* semaphore buffer                  */

    struct sigaction sigvector;          /* structure for signal specs */

#if defined(__HTX_LINUX__)   /* 400*/
#include <termios.h>
#include <sys/ioctl.h>
    struct termios get_io, put_io;  /* termio structures.                */
#elif !defined(__OS400__)   /* 400 */
    struct termio get_io, put_io;  /* termio structures.                */
#endif

#if defined(__OS400__)         /* 400 */
  /* os400 */
    char * spw_path = "/usr/lpp/htx/bin/hxstats";
    char * spw_path2 = "hxstats";
    char * spw_argv[4];
    char * spw_env[1];
    struct inheritance spw_inherit;
    char * stats_dir = "/tmp/htxstats";
    char * stats_num = "30";
#endif

    union shm_pointers shm_addr_wk;      /* shared memory union pointers      */
    DBTRACE(DBENTRY,("enter AH_system.c AH_system\n"));

  /*
   ***  beginning of executable code  ****************************************
   */
    print_log(LOGMSG,"started = %d\n",(shm_addr.hdr_addr)->started);
    fflush(stdout);
    prc_ah = &rc_ah;


    if (((shm_addr.hdr_addr)->started == 0) && ((msg_rcv.cmd == 2001) || (msg_rcv.cmd == 2011))) { /* first time system started?  */
	 print_log(LOGMSG,"Starting ECG = %s/%s first time\n",ECGPATH,ECGNAME);

        /* Log the start time of system activation */
         fp = popen("date","r");
         fgets(mydate,25,fp);

         sprintf(mycommand,"echo \"%s started on %s \" >> /tmp/stx.start.stop.time",ECGNAME,mydate);
         system_call = TRUE;
         system(mycommand);
         system_call = FALSE;
         pclose(fp);


      /* get system startup time  ********************************************/
	strcpy(ECGSTATUS,"ACTIVE");
	strcpy(active_ecg_name, ECGNAME);
	clock = time((long *) 0);
	start_time = *(localtime(&clock));
	if(system_started==0)
	{
	    system_started=1;
	    ecg_info[0].ecg_start_time=clock;
	}

	ECGSTARTTIME = clock;
	//(rem_shm_addr->cur_shm_addr)->start_time = clock;

      /*
       *** Send "System started" message *************************************
       */
	if (autoflag == 0)
	{
	    (void) send_message("System started by Operator request.", 0,
				HTX_SYS_INFO, HTX_SYS_MSG);
	}
	else
	{
	    (void) send_message("System started by AUTOSTART option.", 0,
				HTX_SYS_INFO, HTX_SYS_MSG);
	}

      /*** Look for display devices and open virtual terminals  ***/

	num_entries = shm_addr.hdr_addr->num_entries;

#ifdef HTX_REL_tu320

	for (hft = 0; hft < 4; hft++)
	    VT[hft] = 0;                 /* # of vir terms = 0 */
	p_HE = (struct htxshm_HE *) (shm_addr.hdr_addr + 1); /* skip header */
	for (i = 0; i < num_entries; i++)
	{
	    display_str[0] = '\0';
	    if (strncmp(p_HE->sdev_id, "gem", (size_t) 3) == 0)
		(void) strcpy(display_str, "hispd3d"); /* Gemini */
	    else if (strncmp(p_HE->sdev_id, "ppr", (size_t) 3) == 0)
		(void) strcpy(display_str, "ppr"); /* Pedenales / Lega Family */
	    else if (strncmp(p_HE->sdev_id, "sab", (size_t) 3) == 0)
		(void) strcpy(display_str, "hiprfmge"); /* Sabine */
	    else if (strncmp(p_HE->sdev_id, "sga", (size_t) 3) == 0)
		(void) strcpy(display_str, "sga"); /* Salmon Graphics Adapter */
	    else if (strncmp(p_HE->sdev_id, "skyc", (size_t) 4) == 0)
		(void) strcpy(display_str, "gda"); /* Skyway -- Color */
	    else if (strncmp(p_HE->sdev_id, "skym", (size_t) 4) == 0)
		(void) strcpy(display_str, "gda"); /* Skyway -- Monochrome */
	    else if (strncmp(p_HE->sdev_id, "wga", (size_t) 3) == 0)
		(void) strcpy(display_str, "wga"); /* Skyway -- Monochrome */
	    else if (strncmp(p_HE->sdev_id, "bbl", (size_t) 3) == 0)
		(void) strcpy(display_str, "bbl"); /* baby blue */
	    else if (strncmp(p_HE->sdev_id, "rby", (size_t) 3) == 0)
		(void) strcpy(display_str, "rby"); /* baby blue */
	    else if (strncmp(p_HE->sdev_id, "nep", (size_t) 3) == 0)
		(void) strcpy(display_str, "nep"); /* baby blue */
	    else if (strncmp(p_HE->sdev_id, "mag", (size_t) 3) == 0)
		(void) strcpy(display_str, "mag"); /* Magenta */

	    if (display_str[0] != '\0')  /* display device ? */
	    {
		(void) strcat(display_str, strpbrk(p_HE->sdev_id, "0123456789"));
		if ((hf_devid = get_dispid(display_str)) < 0)
		{
		    (void) sprintf(workstr, "Unable to get display_id for %s "
				   "(mdt entry:%s).\nget_dispid() rc = %d.",
				   display_str, p_HE->sdev_id, hf_devid);
		    (void) send_message(workstr, 0, HTX_SYS_SOFT_ERROR,
					HTX_SYS_MSG);
		    (void) sprintf(workstr, "Unable to get display_id for %s "
				   "(mdt entry:%s).", display_str,
				   p_HE->sdev_id);
		    *prc_ah = -1;
		    sprintf(result_msg,"%s", workstr);
		}
		else if ((hft_fileid = open("/dev/hft", O_RDWR)) == -1)
		{
		    (void) sprintf(workstr, "Unable to open hft for %s.  "
				   "errno = %d.", p_HE->sdev_id, errno);
		    (void) send_message(workstr, errno, HTX_SYS_SOFT_ERROR,
					HTX_SYS_MSG);
		    *prc_ah = -1;
		    sprintf(result_msg,"%s", workstr);
		}
		else
		{
		    (void) sprintf(workstr, "display_id for %s (mdt entry:%s) "
				   "= %x.", display_str, p_HE->sdev_id,
				   hf_devid);
		    (void) send_message(workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);

		    hft = p_HE->hft_num;
		    tcseta_rc = -1;
		  /* save line discipline settings */
		    if (ioctl(hft_fileid, TCGETA, &get_io) != 0)
		    {
			(void) sprintf(workstr, "Unable to get termio "
				       "structure for %s.  errno = %d.",
				       p_HE->sdev_id, errno);
			(void) send_message(workstr, errno, HTX_SYS_SOFT_ERROR,
					    HTX_SYS_MSG);
			*prc_ah = -1;
			sprintf(result_msg,"%s", workstr);
		    }
		    else
		    {
			put_io = get_io;
			put_io.c_oflag &= ~OPOST;
		      /* turn off OPOST */
			if ((tcseta_rc = ioctl(hft_fileid, TCSETA, &put_io))
			    != 0)
			{
			    (void) sprintf(workstr, "Unable to set termio "
					   "structure for %s.  errno = %d.",
					   p_HE->sdev_id, errno);
			    (void) send_message(workstr, errno,
						HTX_SYS_SOFT_ERROR,
						HTX_SYS_MSG);
			    *prc_ah = -1;
			    sprintf(result_msg,"%s", workstr);
			}
		    }

		    shm_addr.hdr_addr->hft_devices[hft].fileid[VT[hft]]
		      = hft_fileid;
		    shm_addr.hdr_addr->hft_devices[hft].hf_devid[VT[hft]]
		      = hf_devid;
		    p_HE->VT_num = VT[hft];
		    VT[hft]++;
		  /* Assign virtual terminal to proper hf_devid */
		    hfchgdsp.hf_mode[0] = HFNONDEF;
		    hfchgdsp.hf_devid[0] = hf_devid >> 24;
		    hfchgdsp.hf_devid[1] = hf_devid >> 16;
		    hfchgdsp.hf_devid[2] = hf_devid >> 8;
		    hfchgdsp.hf_devid[3] = hf_devid;
		    if (write(hft_fileid, (char *) &hfchgdsp, sizeof(hfchgdsp))
			== -1)
		    {
			(void) sprintf(workstr, "Unable assign virtual "
				       "terminal to 0x%x for %s.  errno = %d.",
				       hf_devid, p_HE->sdev_id, errno);
			(void) send_message(workstr, errno, HTX_SYS_SOFT_ERROR,
					    HTX_SYS_MSG);
			*prc_ah = -1;
			sprintf(result_msg,"%s", workstr);

		    } /* endif */
		    if (tcseta_rc == 0)  /*successful TCSETA ioctl call before?*/
		    {
			if (ioctl(hft_fileid, TCSETA, &get_io) != 0)
			{
			    (void) sprintf(workstr, "Unable to restore "
					   "termio structure for %s.  "
					   "errno = %d.",
					   p_HE->sdev_id, errno);
			    (void) send_message(workstr, errno,
						HTX_SYS_SOFT_ERROR,
						HTX_SYS_MSG);
			    *prc_ah = -1;
			    sprintf(result_msg,"%s", workstr);
			} /* endif */
		    } /* endif */
		} /* endif */
	    } /* endif */
	    p_HE++;

	} /* endfor */

      /*** Assign virtual terminal file id to ktsm/gio devices ***/

	p_HE = (struct htxshm_HE *) (shm_addr.hdr_addr + 1); /* skip header */
	for (i = 0; i < num_entries; i++)
	{
	    if ((strncmp(p_HE->sdev_id, "gio", (size_t) 3) == 0) ||
		(strncmp(p_HE->sdev_id, "lpf", (size_t) 3) == 0) ||
		(strncmp(p_HE->sdev_id, "kbd", (size_t) 3) == 0) ||
		(strncmp(p_HE->sdev_id, "dials", (size_t) 5) == 0) ||
		(strncmp(p_HE->sdev_id, "mous", (size_t) 4) == 0) ||
		(strncmp(p_HE->sdev_id, "tab", (size_t) 3) == 0) ||
		(strncmp(p_HE->sdev_id, "sal_kbd", (size_t) 7) == 0) ||
		(strncmp(p_HE->sdev_id, "sal_mous", (size_t) 8) == 0) ||
		(strncmp(p_HE->sdev_id, "sal_tab", (size_t) 7) == 0))
	    {
		hft = p_HE->hft_num;
		if (shm_addr.hdr_addr->hft_devices[hft].fileid[0] == 0)
		{
		    rc = open("/dev/hft", O_RDWR);
		    if (rc == -1)
		    {
			(void) sprintf(workstr, "Unable to open hft for %s.  "
				       "errno = %d.", p_HE->sdev_id, errno);
			(void) send_message(workstr, errno,
					    HTX_SYS_SOFT_ERROR,
					    HTX_SYS_MSG);
			*prc_ah = -1;
			sprintf(result_msg,"%s", workstr);
		    }
		    else
		    {
			shm_addr.hdr_addr->hft_devices[hft].fileid[0] = rc;
			p_HE->VT_num = 0;
		    } /* endif */
		} /* endif */
	    } /* endif */
	    p_HE++;

	} /* endfor */

      /* if this program is running on an hft, make sure that
	 this program has the active vertual terminal.           */

	rc=ioctl(0, HFTGETID, &hfid);
	if ((long) hfid.hf_chan < 0)
	    rc = -1;


	if (rc == 0)                          /* hft display? */
	{
	  /* Make this program's virtual terminal the ACTIVE terminal. */
	    mgrcmd.hf_cmd = SMACT;
	    mgrcmd.hf_vsid = 0;
	    mgrcmd.hf_vtid = hfid.hf_chan;
	    if (ioctl(0, HFTCSMGR, &mgrcmd) != 0)
	    {
		(void) sprintf(workstr, "ioctl(0, HF(T)CSMGR , &mgrcmd) call "
			       "failed.  errno = %d.", errno);
		(void) send_message(workstr, errno, HTX_SYS_SOFT_ERROR,
				    HTX_SYS_MSG);
		*prc_ah = -1;
		sprintf(result_msg,"%s", workstr);
	    } /* endif */
	} /* endif */

#endif


      /*
       ***  allocate space for load sequence table  **************************
       */

	if ( ECG_MAX_ENTRIES <= 0 ) {
	    (void) sprintf(workstr, " mdt doesn't contain any exerciser");
	    (void) send_message(workstr, errno, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
	    *prc_ah = -1;
	    sprintf(result_msg,"%s", workstr);
	    DBTRACE(DBEXIT,("return/a -1 AH_system.c AH_system\n"));
	    return -1 ;
	}
	shm_addr_wk.hdr_addr = shm_addr.hdr_addr;  /* copy addr to work space */
	(shm_addr_wk.hdr_addr)++;         /* skip over header                  */
	if ( num_entries != 0 ) {

	    load_tbl_ptr = (struct load_tbl *) calloc((unsigned)num_entries,
						      sizeof(struct load_tbl));
	    if (load_tbl_ptr == NULL) {
		(void) sprintf(workstr, "Unable to allocate memory for load "
			       "sequence table.  errno = %d.", errno);
		(void) send_message(workstr, errno, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
		*prc_ah = -1;
		sprintf(result_msg,"%s", workstr);
		DBTRACE(DBEXIT,("return/b -1 AH_system.c AH_system\n"));
		return -1;
	    } /* endif */

		/*
		***  build load sequence table  ***************************************
		*/
	    for (i = 0; i < num_entries; i++) { /* build tbl                       */
		(load_tbl_ptr + i)->shm_pos = i;
		(load_tbl_ptr + i)->load_seq = (shm_addr_wk.HE_addr + i)->load_seq;
	    } /* endfor */

#if defined(__HTX_LINUX__) || defined(__OS400__)  /* 400 */
#include <stdlib.h>
	    qsort((void *) load_tbl_ptr, (size_t) num_entries, (size_t) sizeof(struct load_tbl), (int (*)(const void *, const void *)) load_compar);
#else

	    qsort((char *) load_tbl_ptr, (size_t) num_entries,
		  (size_t) sizeof(struct load_tbl), load_compar);
#endif
	}


	(void) fcntl(fileno(stdin), F_SETFD, 1);  /* stdin to close on exec  */
	(void) fcntl(fileno(stdout), F_SETFD, 1); /* stdout to close on exec */
	(void) fcntl(fileno(stderr), F_SETFD, 1); /* stderr to close on exec */

      /*
       ***  set up sigvector structure  **************************************
       */
	sigvector.sa_handler = SIG_DFL; /* set default action                  */
	sigemptyset(&(sigvector.sa_mask));  /* do not block other signals      */
	sigvector.sa_flags = 0;         /* No special flags                    */

      /*
       ***  load HE's  *******************************************************
       */
	for (i = 0; i < num_entries; i++)
	{
	    p_HE = shm_addr_wk.HE_addr + (load_tbl_ptr + i)->shm_pos;
	    rc = load_exerciser(p_HE, &sigvector);
	    // print_log(LOGMSG,"loaded %s exerciser ecg_name: %s\n",p_HE->HE_name, ECGNAME );
	} /* endfor */

	if (load_tbl_ptr != NULL) {
	    free( (char *) load_tbl_ptr);      /* free load table memory           */
	    load_tbl_ptr = NULL;
	}
	(shm_addr.hdr_addr)->started = 1;  /* show system started              */

      /***  load program to periodically record system statistics  ***/
#if defined(__OS400__)
    /* os400 */
        if (hxstats_PID == 0) {
	   spw_argv[0] = spw_path2;
	   spw_argv[1] = stats_dir;
	   spw_argv[2] = stats_num;
	   spw_argv[3] = NULL;

	   spw_env[0] = NULL;

	   memset(&spw_inherit.flags,0x00,sizeof(spw_inherit));
	   spw_inherit.flags = SPAWN_SETSIGMASK | SPAWN_SETSIGDEF;
	   spw_inherit.pgroup = SPAWN_NEWPGROUP;
	   sigemptyset(&(spw_inherit.sigmask));
	   sigfillset(&(spw_inherit.sigdefault));

	   frkpid = spawn(spw_path, 0, NULL, &spw_inherit, spw_argv, spw_env);

	   switch (frkpid) {
	       case -1:  /* problem with fork() call          */
		   sprintf(workstr, "HTXD: Unable to fork for hxstats program.  errno = %d", errno);
		   send_message(workstr, errno, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
	           *prc_ah = -1;
		   print_log(LOGMSG,"%s\n", workstr);
		   sprintf(result_msg,"%s", workstr);
		   break;

	       default:  /* parent process                  */
		   hxstats_PID = frkpid;
                   print_log(LOGMSG,"default HTXPATH = %s ,statspid = %d\n",HTXPATH, hxstats_PID); fflush(stdout);
		   //rem_shm_addr->misc_data_addr->hxstats_PID = frkpid;
		   break;

	   }    /* endswitch */
	}  /* if (hxstats_PID == 0) */

#else
	if (hxstats_PID == 0) {
	    frkpid = fork();

	    switch (frkpid)
	    {
		case 0:                        /* child process                     */

		    if ( listener != 0 ) {
			close(listener);
		    }
		    if ( fd_num != 0 ) {
			close(fd_num);
		    }



		    for (j = 1; j <= SIGMAX; j++)   /* set default signal processing   */
			(void) sigaction(j, &sigvector, (struct sigaction *) NULL);
		    (void) setpgid(getpid(), getpid()); /* change the process group so
						 we don't get signals meant
						 for the supervisor program */

		    (void) strcpy(workstr, HTXPATH);
		    (void) strcat(workstr,"/bin/hxstats");
		    print_log(LOGMSG,"HTXPATH = %s\n",HTXPATH);

		    if ( (execl(workstr, "hxstats", "/tmp/htxstats", "30", (char *) 0) )
			 == -1)
		    {
			(void) sprintf(workstr,
				       "Unable to load hxstats program.  errno = %d",
				       errno);
			(void) send_message(workstr, errno, HTX_SYS_HARD_ERROR,
					    HTX_SYS_MSG);
			*prc_ah = -1;
			sprintf(result_msg,"%s", workstr);
			exit(errno);
		    } /* endif */
		    break;
		case -1:                       /* problem with fork() call          */

		    (void) sprintf(workstr, "Unable to fork for hxstats program.  "
				   "errno = %d", errno);
		    (void) send_message(workstr, errno, HTX_SYS_HARD_ERROR,
					HTX_SYS_MSG);
		    *prc_ah = -1;
		    sprintf(result_msg,"%s", workstr);
		    break;
		default :                      /* parent process                  */
		    hxstats_PID = frkpid;
		    print_log(LOGMSG,"default HTXPATH = %s ,statspid = %d\n",HTXPATH, hxstats_PID); fflush(stdout);
		    break;
	    } /* endswitch */
	}
      /***  fork process to watch for hung exercisers ***/
	if ((HANG_MON_PERIOD > 0) && (hang_mon_PID == 0))
	{
	    frkpid = fork();

	    switch (frkpid)
	    {
		case 0:                      /* child process                     */
		    if ( listener != 0 ) {
			close(listener);
		    }
		    if ( fd_num != 0 ) {
			close(fd_num);
		    }

	    /* Let HE's do 1st hxfupdate...    */
		    (void) sleep((HANG_MON_PERIOD>5) ? HANG_MON_PERIOD : 5);
		    hang_monitor();
		    exit(0);
		    break;
		case -1:                     /* problem with fork() call          */

		    (void) sprintf(workstr, "Unable to fork for hang_monitor process.  "
				   "errno = %d", errno);
		    (void) send_message(workstr, errno, HTX_SYS_HARD_ERROR,
					HTX_SYS_MSG);
		    *prc_ah = -1;
		    sprintf(result_msg,"%s", workstr);
		    break;
		default :                      /* parent process                  */
		    hang_mon_PID = frkpid;
		    break;
	    } /* endswitch */
	} /* endif */
	if ((equaliser_flag != 0) && (equaliser_PID == 0)) {
		frkpid = fork();
		switch (frkpid)  {

			case 0:  /* child process       */
				/*  let exercisers do 1st hxfupdate   */
				equaliser();
				exit(0);
				break;
			case -1:     /* problem with fork()   */
				sprintf(workstr, "Unable to fork for equaliser process.  errno = %d", errno);
				send_message(workstr, errno, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
				break;
			default:        /* parent process */
				equaliser_PID = frkpid;
				break;
		} /* end switch */
	} /* endif */

#endif

      /*** set semaphore to show all HE's exec'd *************************/
	errno = 0;                          /* clear errno. */
	//SEMARG = 1;
	SEMCTL(semhe_id, 3, SETVAL, 1);
	//semval = SEMCTL(semhe_id, 3, SETVAL, 1);
	//if (semval != 0)
	    //  {
	    //    errno_save = errno;
	    //    (void) sprintf(workstr, "Error trying to set \"All HE's started\" "
				 //                   "semaphore to 1.\nerrno = %d (%s).",
				 //                   errno_save, strerror(errno_save));
	    //    (void) send_message(workstr, errno_save, HTX_SYS_HARD_ERROR,
				      //                        HTX_SYS_MSG);
	    //    *prc_ah = -1;
	    //    sprintf(result_msg,"%s", workstr);
	    //  } /* endif */

	(void) fcntl(fileno(stdin), F_SETFD, 0); /* stdin NOT to close on exec */
	(void) fcntl(fileno(stdout), F_SETFD, 0); /*stdout NOT to clse on exec */
	(void) fcntl(fileno(stderr), F_SETFD, 0); /*stderr NOT to clse on exec */

	rem_shm_addr->sock_hdr_addr->bcast_done = FALSE;
	//strcpy(rem_shm_addr->sock_hdr_addr->system_status,"ACTIVE");
	rem_shm_addr->sock_hdr_addr->last_update_time = time((long *) 0);
	sprintf(msg_type,"UPDATE");
	sprintf(result_msg,"ECG(%s/%s) Activated.",ECGPATH,ECGNAME);

    }
    else if (((shm_addr.hdr_addr)->started == 0) && (msg_rcv.cmd == 2021))
	sprintf(result_msg,"ECG(%s/%s) is in Inactive state.",ECGPATH,ECGNAME);
    else /* system already started            */
    {

	num_entries = shm_addr.hdr_addr->num_entries;
	if ( num_entries != 0 ) {
	    print_log(LOGMSG,"num_entries to halt:%d\n",num_entries);
	    semval = semctl(semhe_id, 0, GETVAL, &sembuffer);
	    print_log(LOGMSG,"num_entries = %d in AH_system for ECG = %s/%s,semid=%d,semval=%d\n",num_entries,ECGPATH,ECGNAME, semhe_id,semval);

	    switch (semval)
	    {
		case -1:                         /* error getting sem value          */
		    (void) sprintf(workstr, "Unable to get Global Stop/Start "
				   "semaphore value.  errno = %d.", errno);
		    (void) send_message(workstr, errno, HTX_SYS_HARD_ERROR,
					HTX_SYS_MSG);
		    *prc_ah = -1;
		    sprintf(result_msg,"%s", workstr);
		    DBTRACE(DBEXIT,("return/c -1 AH_system.c AH_system\n"));
		    return -1;
		case 0:                          /* system active                  */
		    if (((msg_rcv.cmd == 2001) || (msg_rcv.cmd == 2021))) {
			 print_log(LOGMSG,"Waiting for all Hardware Exercisors to stop");
			sprintf(result_msg,"Waiting for all Hardware Exercisors to stop");
			semop(semhe_id, halt_sops, (unsigned int) 1);
			send_message("HTXD: Operator has requested a system halt.", 0, HTX_SYS_INFO, HTX_SYS_MSG);
			exer_shm_entries = shm_addr.hdr_addr->max_entries;

			semctl_arg.array = (ushort*) malloc(sem_length * sizeof(ushort) );
			if(semctl_arg.array == NULL) {
				sprintf(workstr, "Unable to allocate memory for semctl GETVAL array.  errno = %d.", errno);
				send_message(workstr, errno, HTX_SYS_HARD_ERROR, HTX_SYS_MSG);
				return;
			}

			p_HE = (struct htxshm_HE *) (shm_addr.hdr_addr + 1);
			all_exer_stopped = delay_counter = 0;
			while(delay_counter < max_wait_tm) {
				memset(semctl_arg.array, 0, (sem_length * sizeof(ushort) ) );
				semctl(semhe_id, 0, GETALL, semctl_arg);
				
				exer_count = exer_system_halted = exer_error_halted = exer_operation_halted = exer_exited = 0;
				while(exer_count < exer_shm_entries) {
					if(semctl_arg.array[(exer_count * SEM_PER_EXER + SEM_GLOBAL + 2)] == 1) {
						exer_system_halted++;
					} else if(semctl_arg.array[(exer_count * SEM_PER_EXER + SEM_GLOBAL + 1)] == 1) {
						exer_error_halted++;
					} else if(semctl_arg.array[(exer_count * SEM_PER_EXER + SEM_GLOBAL )] == 1) {
						exer_operation_halted++;
					} else if( (p_HE + exer_count) -> PID == 0) {
						exer_exited++;
					} else {
						break;
					}
					exer_count++;
				}

				if(exer_shm_entries  <= (exer_system_halted + exer_error_halted + exer_operation_halted + exer_exited) ) {
					all_exer_stopped = 1;
					break;
				}

				sleep(delay_slice);
				delay_counter += delay_slice;
			}

			if ( all_exer_stopped == 0) {
				send_message("Warning: Not all of the HE's stopped within the allowed time.\nSystem will continue...", 0, HTX_SYS_INFO, HTX_SYS_MSG);
			} else {
				send_message("System halted by Operator request.", 0, HTX_SYS_INFO, HTX_SYS_MSG);
			}

			free(semctl_arg.array);
			semctl_arg.array = NULL;
			strcpy(ECGSTATUS,"SUSPENDED");
			strcpy(active_ecg_name, ECGNAME);
			(void) send_message("System halted by Operator request.", 0,
						HTX_SYS_INFO, HTX_SYS_MSG);
			rem_shm_addr->sock_hdr_addr->bcast_done = FALSE;
			rem_shm_addr->sock_hdr_addr->last_update_time = time((long *) 0);
			sprintf(msg_type,"UPDATE");
		    }
		    else if (msg_rcv.cmd == 2011) {
			 print_log(LOGMSG,"ECG(%s/%s) already Active.",ECGPATH,ECGNAME);
			sprintf(result_msg,"ECG(%s/%s) already Active.",ECGPATH,ECGNAME);
		    }
		    break;
		case 1:                          /* system halted                    */
		    if (((msg_rcv.cmd == 2001) || (msg_rcv.cmd == 2011))) {
			 print_log(LOGMSG,"System is Suspended\n");
			fflush(stdout);
			semctl_arg.val = 0;
			semctl(semhe_id, 0, SETVAL, semctl_arg);
			send_message("HTXD: System re-activated by Operator request.", 0, HTX_SYS_INFO, HTX_SYS_MSG);
			 print_log(LOGMSG,"ECG(%s/%s) activated.",ECGPATH,ECGNAME);

			sprintf(result_msg,"ECG(%s/%s) activated.",ECGPATH,ECGNAME);
			//strcpy(rem_shm_addr->sock_hdr_addr->curr_ecg_name, ECGNAME);
			strcpy(ECGSTATUS,"ACTIVE");
			strcpy(active_ecg_name, ECGNAME);
			rem_shm_addr->sock_hdr_addr->bcast_done = FALSE;
			//strcpy(rem_shm_addr->sock_hdr_addr->system_status,"ACTIVE");
			rem_shm_addr->sock_hdr_addr->last_update_time = time((long *) 0);
			sprintf(msg_type,"UPDATE");
		    }
		    else if (msg_rcv.cmd == 2021) {
			 print_log(LOGMSG,"ECG(%s/%s) already Suspended.",ECGPATH,ECGNAME);
			sprintf(result_msg,"ECG(%s/%s) already Suspended.",ECGPATH,ECGNAME);
		    }
		    break;
	    } /* endswitch */

	}
    } /* endif */

    DBTRACE(DBEXIT,("return AH_system.c AH_system\n"));
    return *prc_ah;
} /* AH_system() */
