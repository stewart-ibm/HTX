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
#include "cfgclibdef.h"

#ifndef __HTX_LINUX__
#define SEMCTL(x,y,z,a) (void)semctl(x,y,z,a)
#else
#define SEMCTL(x,y,z,a) semctl_arg.val=a, (void)semctl(x,y,z,semctl_arg)
#endif

int LOG_MSG (int , char *, CFG__SFT ** );
int cmprnset_dup_name (struct htxshm_HE *);
int add_to_ecg_all (void);
int rmv_dup_info (char *);
int rmv_ecg_all (char *);
char default_mdt_snz[4096];     /* ptr to default mdt stanza    */

int wof_test_enabled = 0, equaliser_debug_flag = 0;
char cfg_file[30];

extern int cur_ecg_pos;

int all_entries = 0;
int sem_length;

int dup_num;

int
process_mdt_to_shm (char *stanza, int stanza_length,
                    union shm_pointers shm_point, char **default_stanza,
                    CFG__SFT * mdt_fd)
{
    char workstr[128], str_tmp[16];      /* work string                  */
    int rc;                      /* general return code          */
    char *shm_work_ptr;          /* char pointer into shm        */
    char *stanza_ptr;            /* pointer into stanza          */
    int i;
    struct htxshm_HE *p_shm_HE_base;
    DBTRACE(DBENTRY,("enter build_ipc.c process_mdt_to_shm\n"));

    p_shm_HE_base = ((struct htxshm_HE *)
		     ((struct htxshm_hdr *) ECGSHMADDR_HDR + 1));

    if (default_stanza == NULL || *default_stanza == NULL)
      /* read the next stanza */
	rc = cfgcrdsz (mdt_fd, stanza, stanza_length, (char *) NULL);
    else {
	rc = cfgcrdsz (mdt_fd, stanza, stanza_length, "default");
	switch (rc) {
	    case CFG_SUCC:           /* default found, read next stanza */
		rc = cfgcrdsz (mdt_fd, stanza, stanza_length, (char *) NULL);
		break;
	    case CFG_SZNF:           /* not found, rewind, set it, and get next */
		cfgcrwnd (mdt_fd);
		(void) cfgcsvdflt (mdt_fd, *default_stanza);
		rc = cfgcrdsz (mdt_fd, stanza, stanza_length, (char *) NULL);
		break;
	    default:                 /* handle problem later */
		break;
	}                         /* switch */
	*default_stanza = NULL;   /* don't want to see it again */
    }                            /* if */

   /* get next stanza */
    if (rc != CFG_SUCC) {
	DBTRACE(DBEXIT,("return/a build_ipc.c process_mdt_to_shm\n"));
	return (rc);              /* no next stanza or some other problem */
    }

	/**********************************************************/
   /* Now setup the HE entry from the stanza that is in hand */
	/**********************************************************/

   /* set Hardware Exerciser Name */
    cfgcskwd ("HE_name", stanza, workstr);
    (void) strcpy (shm_point.HE_addr->HE_name, unquote (workstr));

   /* set Adapter Description */
    cfgcskwd ("adapt_desc", stanza, workstr);
    (void) strcpy (shm_point.HE_addr->adapt_desc, unquote (workstr));

   /* set Device Description */
    cfgcskwd ("device_desc", stanza, workstr);
    (void) strcpy (shm_point.HE_addr->device_desc, unquote (workstr));

   /* set EMC Rules File Name */
    cfgcskwd ("emc_rules", stanza, workstr);
    (void) strcpy (shm_point.HE_addr->emc_rules, unquote (workstr));

   /* set REG Rules File Name */
    cfgcskwd ("reg_rules", stanza, workstr);
    (void) strcpy (shm_point.HE_addr->reg_rules, unquote (workstr));

    cfgcskwd ("log_vpd", stanza, workstr);
    (void) strcpy (str_tmp, unquote (workstr));
    switch (str_tmp[0]) {

	case 'y':
	case 'Y':
	    shm_point.HE_addr->log_vpd = 1;
	    //print_log(LOGMSG,"%s:Detailed Error log is set\n", shm_point.HE_addr->HE_name);
	    break;
	case 'n':
	case 'N':
	default:
	    shm_point.HE_addr->log_vpd = 0;
	    break;
    }

    cfgcskwd ("dup_device", stanza, workstr);
    (void) strcpy (str_tmp, unquote (workstr));
    switch (str_tmp[0]) {

	case 'y':
	case 'Y':
	    shm_point.HE_addr->dup_device = 1;
	    //print_log(LOGMSG,"%s:dup device is set\n", shm_point.HE_addr->HE_name);
	    break;
	case 'n':
	case 'N':
	default:
	    shm_point.HE_addr->dup_device = 0;
	    break;
    }

    dup_num = 0;
   /* set "/dev/????" Special File Name */
    shm_work_ptr = &(shm_point.HE_addr->sdev_id[0]);
    stanza_ptr = &(stanza[0]);
    do {
	*shm_work_ptr = *stanza_ptr;
	shm_work_ptr++;
	stanza_ptr++;
    }
    while (*stanza_ptr != ':');  /* enddo */
    *shm_work_ptr = '\0';        /* terminating 0 for text string    */

   /* set DMA Channel */
    cfgcskwd ("dma_chan", stanza, workstr);
    shm_point.HE_addr->dma_chan = (unsigned short) atoi (unquote (workstr));

   /* set hft number */
#ifdef HTX_REL_tu320
    cfgcskwd ("hft", stanza, workstr);
    workint = (unsigned short) atoi (unquote (workstr));
    if ((workint < 0) || (workint > 3))
	shm_point.HE_addr->hft_num = 0;
    else
	shm_point.HE_addr->hft_num = workint;
#endif

   /* set Idle Time */
    cfgcskwd ("idle_time", stanza, workstr);
    shm_point.HE_addr->idle_time = (unsigned short) atoi (unquote (workstr));

   /* set Interrupt Level */
    cfgcskwd ("intrpt_lev", stanza, workstr);
    shm_point.HE_addr->intrpt_lev = (unsigned short) atoi (unquote (workstr));

   /* set Load Sequence */
    cfgcskwd ("load_seq", stanza, workstr);
    shm_point.HE_addr->load_seq = (unsigned short) atoi (unquote (workstr));

   /* set Maximum Run Time */
    cfgcskwd ("max_run_tm", stanza, workstr);
    shm_point.HE_addr->max_run_tm = (unsigned short) atoi (unquote (workstr));

   /* set Port Number */
    cfgcskwd ("port", stanza, workstr);
    shm_point.HE_addr->port = (unsigned short) atoi (unquote (workstr));

   /* set Priority */
    cfgcskwd ("priority", stanza, workstr);
    shm_point.HE_addr->priority = (unsigned short) atoi (unquote (workstr));

   /* set Slot Number */
    cfgcskwd ("slot", stanza, workstr);
    shm_point.HE_addr->slot = (unsigned short) atoi (unquote (workstr));
    if (!strcmp (unquote (workstr), "0"))        /* modifications for 354660 */
	strcpy (workstr, "NA          ");
    else
	strcpy (workstr, unquote (workstr));
    for (i = strlen (workstr); i < 13; i++)
	workstr[i] = ' ';
    workstr[13] = 0;

    (void) strcpy (shm_point.HE_addr->slot_port, unquote (workstr));

   /* clear PID Number */
    shm_point.HE_addr->PID = (int) NULL;
    shm_point.HE_addr->is_child = 1;

   /* set Max cycles */
    cfgcskwd ("max_cycles", stanza, workstr);
    shm_point.HE_addr->max_cycles = (unsigned int) atoi (unquote (workstr));

   /* set Continue on Error Flag */
    cfgcskwd ("cont_on_err", stanza, workstr);
    unquote (workstr);
    if ((workstr[0] == 'y') || (workstr[0] == 'Y'))
	shm_point.HE_addr->cont_on_err = 1;
    else {
	shm_point.HE_addr->cont_on_err = 0;
    }                            /* endif */

   /* set Halt on Error Severity Code Level */
    cfgcskwd ("halt_level", stanza, workstr);
    unquote (workstr);
    if (strcmp (workstr, "0") == 0)
	shm_point.HE_addr->halt_sev_level = 0;
    else if (strcmp (workstr, "1") == 0)
	shm_point.HE_addr->halt_sev_level = 1;
    else if (strcmp (workstr, "2") == 0)
	shm_point.HE_addr->halt_sev_level = 2;
    else if (strcmp (workstr, "3") == 0)
	shm_point.HE_addr->halt_sev_level = 3;
    else if (strcmp (workstr, "4") == 0)
	shm_point.HE_addr->halt_sev_level = 4;
    else if (strcmp (workstr, "5") == 0)
	shm_point.HE_addr->halt_sev_level = 5;
    else if (strcmp (workstr, "6") == 0)
	shm_point.HE_addr->halt_sev_level = 6;
    else if (strcmp (workstr, "7") == 0)
	shm_point.HE_addr->halt_sev_level = 7;
    else if (strcmp (workstr, "HTX_HE_HARD_ERROR") == 0)
	shm_point.HE_addr->halt_sev_level = HTX_HE_HARD_ERROR;
    else if (strcmp (workstr, "HTX_HE_SOFT_ERROR") == 0)
	shm_point.HE_addr->halt_sev_level = HTX_HE_SOFT_ERROR;
    else
	shm_point.HE_addr->halt_sev_level = HTX_HE_HARD_ERROR;

   /* set error acknowledged flag */
    shm_point.HE_addr->err_ack = 1;

   /* set start_halted flag */
   /*
    * If HE set to halt on startup, set the appropriate semaphone AFTER
    * the shared memory has been sorted.
    */
    cfgcskwd ("start_halted", stanza, workstr);
    unquote (workstr);
    if ((workstr[0] == 'y') || (workstr[0] == 'Y'))
	shm_point.HE_addr->start_halted = 1;
    else
	shm_point.HE_addr->start_halted = 0;

   /* set virtual terminal number */
#ifdef HTX_REL_tu320
    shm_point.HE_addr->VT_num = 0;
#endif

   /* set Time of Last Error */
    shm_point.HE_addr->tm_last_err = 0;

   /* set Time of Last call to HTX_update() */
    shm_point.HE_addr->tm_last_upd = -1;

   /* set Number of Other Bad Operations */
    shm_point.HE_addr->bad_others = 0;

   /* set Number of Bad Reads */
    shm_point.HE_addr->bad_reads = 0;

   /* set Number of Bad Writes */
    shm_point.HE_addr->bad_writes = 0;

   /* set Number of Bytes Read */
    shm_point.HE_addr->bytes_read1 = 0;
    shm_point.HE_addr->bytes_read2 = 0;

   /* set Number of Bytes Written */
    shm_point.HE_addr->bytes_writ1 = 0;
    shm_point.HE_addr->bytes_writ2 = 0;

   /* set Number of Other Good Operations */
    shm_point.HE_addr->good_others = 0;

   /* set Number of Good Reads */
    shm_point.HE_addr->good_reads = 0;

   /* set Number of Good Writes */
    shm_point.HE_addr->good_writes = 0;
    //print_log(LOGMSG,"assigned?:from process_mdt_to_shm id = %s:\n", shm_point.HE_addr->sdev_id);

    DBTRACE(DBEXIT,("return build_ipc.c process_mdt_to_shm\n"));
    return (rc);                 /* should always be CFG_SUCC here */
}                               /* process_mdt_to_shm */

/*****************************************************************************/
/*****  b u i l d _ i p c ( )  ***********************************************/
/*****************************************************************************/
/* */
/* FUNCTION NAME =     build_ipc()                                           */
/* */
/* DESCRIPTIVE NAME =  Build AIX IPC structures for use by HTX system        */
/* */
/* FUNCTION =          Allocates and initializes AIX IPC structures (message */
/* queue, semaphore, and shared memory) as defined by    */
/* the Master Device Table (normally the attribute file  */
/* "/usr/lpp/htx/ecg/ecg")                               */
/* */
/* INPUT =             The Master Device Table (normally                     */
/* "/usr/lpp/htx/ecg/ecg")                               */
/* */
/* OUTPUT =            The required HTX AIX IPC structures                   */
/* */
/* NORMAL RETURN =     0 to indicate successful completion                   */
/* */
/* ERROR RETURNS =     10 - Unable to open ecg file                          */
/* 11 - No entries in the ecg file                       */
/* 12 - Error reading ecg file                           */
/* 13 - Unable to close ecg file                         */
/* */
/* 20 - Unable to allocate IPC message queue             */
/* */
/* 30 - Unable to allocate IPC semaphore structure       */
/* */
/* 40 - Unable to allocate IPC shared memory segment     */
/* 41 - Unable to attach IPC shared memory segment       */
/* */
/* EXTERNAL REFERENCES                                                       */
/* */
/* OTHER ROUTINES = DRT_compare - used in qsort to order the entries in   */
/* the shared memory segment (misc.c)      */
/* unquote -     strips double quotation marks (") off   */
/* of character strings (misc.c)           */
/* process_mdt_to_shm - interprets mdt attributes and    */
/* constructs the shared memory structure  */
/* for each stanza.                        */
/* DATA AREAS =     HTXPATH - pointer to a string which contains the path */
/* to the top level of the HTX file hierarchy        */
/* (usually "/usr/lpp/htx").                         */
/* MSGLOGKEY message queue (identified by the msgqid     */
/* variable)                                          */
/* SHMKEY shared memory segment (this segment is pointed */
/* to by the shm_addr pointer)                        */
/* SEMHEKEY semaphores (identified by the semhe_id       */
/* variable)                                          */
/* */
/* MACROS =         PRTMSG - Print message CURSES macro                   */
/* (defined in common/hxiconv.h)                */
/* */
/*****************************************************************************/

int
build_ipc (char *result_msg)
{
   /*
    * **  variable/function definitions  ****************************************
    */
    CFG__SFT *mdt_fd;            /* pointer to ECG attribute file */

    char msg[81];                /* message string               */
    char workstr[128];
    char stanza[4096];           /* attribute file stanza        */
    char tag[40];                /* add-on char string           */

    int i, ii, dup_dev = 0; /* general counter              */
    int rc, retcod;                      /* return code                  */
    int workint;                 /* work integer                 */
    int shm_entries;             /* # of entries in the shm      */
    int prev_ecg_pos;


    union shm_pointers shm_point;        /* shared memory address        */

    unsigned int max_entries;    /* max number of entries in shm */
    unsigned int num_entries;    /* current number of entries in shm  */
    DBTRACE(DBENTRY,("enter build_ipc.c build_ipc\n"));

   /* char full_name[40]; */

   /*
    * **  beginning of executable code  **********************************
    */
    //shm_addr.hdr_addr = (struct htxshm_hdr *) 0;  /* no shm yet.         */
    ECGSHMADDR_HDR = (struct htxshm_hdr *) 0;    /* no shm yet.         */

   /* open ECG cfg file ************************************************ */
    PUT_FULL_ECG;
    mdt_fd = cfgcopsf (full_name);
    if (mdt_fd == (CFG__SFT *) NULL) {
	sprintf (result_msg,
		 "######ERROR: Unable to open ECG file.%d (%s)########\n",
		 errno, strerror (errno));
	LOG_MSG (10, result_msg, (CFG__SFT **) NULL);
	DBTRACE(DBEXIT,("return/a 10 build_ipc.c build_ipc\n"));
	return (10);
    }				/* endif */
    for ( (num_entries = (unsigned int) 0);
	  ((rc = cfgcrdsz (mdt_fd, stanza, sizeof (stanza), (char *) NULL)) == CFG_SUCC);
	  (num_entries++) ) { }	/* get number of ECG entries */

    if (rc == CFG_EOF) {	/* normal EOF?                       */
	    max_entries = num_entries + EXTRA_ENTRIES;     /* room for extras   */
      /* endif */
    }
    else {                       /* Problem with ECG.                 */
	(void) sprintf (msg, "ERROR: ECG file read failure CFG_");
	switch (rc) {
	    case CFG_SZNF:           /* req stanza not found */
		(void) sprintf (tag, "SZNF");
		break;
	    case CFG_SZBF:           /* stanza too long      */
		(void) sprintf (tag, "SZBF");
		break;
	    case CFG_UNIO:           /* I/O error            */
		(void) sprintf (tag, "UNIO");
		break;
	    default:                 /* unexpected error     */
		(void) sprintf (tag, "????");
		break;
	}                         /* endswitch */
	(void) strcat (msg, tag);
	(void) sprintf (tag, ", rc = %d.", rc);
	strcat (msg, tag);
	sprintf (result_msg, "%s", msg);
	LOG_MSG (12, result_msg, &mdt_fd);
	DBTRACE(DBEXIT,("return/c 12 build_ipc.c build_ipc\n"));
	return (12);
    }                            /* endif */

   /* close ECG cfg file */

    LOG_MSG (13, "Closing the file...No error message. RET = 13", &mdt_fd);
   /*
    * if (cfgcclsf (mdt_fd) != CFG_SUCC) { sprintf (result_msg,
    * "###########ERROR: Unable to close ECG file.%d (%s)###########\n",
    * errno, strerror (errno)); LOG_MSG (13, result_msg); return (13); }
 *//* endif */

   /* allocate message queue structure  ******************************* */

    if (!MsgQueCreated) {
	msgqid = msgget (MSGLOGKEY, IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);       /* Message Log queue              */
	if (msgqid == -1) {       /* problem?                          */
	    sprintf (result_msg,
		     "###ERROR: Unable to get Message Log message queue. ##Errno = %d ###\n",
		     errno);
	    LOG_MSG (14, result_msg, &mdt_fd);
	    DBTRACE(DBEXIT,("exit 14 build_ipc.c build_ipc\n"));
	    //return (14);
	    exit (14);
	}
      /* endif */
	else
	    MsgQueCreated = TRUE;
    }
   /* allocate semaphore structures ************************************ */
    sem_length = ((int) (max_entries + MAX_ADDED_DEVICES) * SEM_PER_EXER) + SEM_GLOBAL;
    ECGSEMHEID = semget (ECGSEMKEY, sem_length + SEM_GLOBAL, IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);   /* HE semaphores                   */
    if (ECGSEMHEID == -1) {      /* problem?                          */
	sprintf (result_msg,
		 "##ERROR: Unable to get HE semaphore structure for %s/%s.Errno = %d (%s)##\n",
		 ECGPATH, ECGNAME, errno, strerror (errno));
	LOG_MSG (15, result_msg, &mdt_fd);
	DBTRACE(DBEXIT,("return/d 15 build_ipc.c build_ipc\n"));
	return (15);
    }                            /* endif */
   /* allocate shared memory ****************************************** */

    ECGSHMID = shmget (ECGSHMKEY, (int) (sizeof (struct htxshm_hdr) + ((max_entries + PSEUDO_EXTRA_ENTRIES + MAX_ADDED_DEVICES) * sizeof (struct htxshm_HE))), IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);   /* system shared memory              */
    if (ECGSHMID == -1) {        /* problem?                          */
	sprintf (result_msg,
		 "####ERROR: Unable to get shared memory.###ERRNO = %d", errno);
	LOG_MSG (16, result_msg, &mdt_fd);
	DBTRACE(DBEXIT,("return/e 16 build_ipc.c build_ipc\n"));
	return (16);
    }                            /* endif */
   /* attach shared memory */
    ECGSHMADDR_HDR = (struct htxshm_hdr *) shmat (ECGSHMID, (char *) 0, 0);
    print_log(LOGMSG,"BUILDIPC: addr = 0x%x id = %d key=%d\n", (int)ECGSHMADDR_HDR, ECGSHMID,
	    ECGSHMKEY);
    fflush (stdout);
    if ((int) ECGSHMADDR_HDR == -1) {    /* problem?                       */
	sprintf (result_msg,
		 "#######ERROR: Unable to attach shared memory(%d, 0x%x). Errno = %d#######\n",
		 ECGSHMID, ECGSHMID, errno);
	LOG_MSG (17, result_msg, &mdt_fd);
	DBTRACE(DBEXIT,("return/f 17 build_ipc.c build_ipc\n"));
	return (17);
    }                            /* endif */
    if (!sig_handle_done)
	set_sig_handler ();

   /* initialize semaphores ******************************************** */
    for (i = 0; i < (((int) (max_entries + MAX_ADDED_DEVICES) * SEM_PER_EXER) + SEM_GLOBAL); i++) {
	SEMCTL (ECGSEMHEID, i, SETVAL, 0);
    }

   /* initialize shared memory area ********************************* */
    bzero (ECGSHMADDR_HDR,
	   (int) (sizeof (struct htxshm_hdr) +
		  ((max_entries + PSEUDO_EXTRA_ENTRIES + MAX_ADDED_DEVICES)
		   * sizeof (struct htxshm_HE))));

   /* initialize header flags */
    ECGSHMADDR_HDR->emc = profile_emc_mode;      /* set from htx_profile        */

   /* set initial # entries values */
    ECGSHMADDR_HDR->max_entries = max_entries;
    ECGSHMADDR_HDR->num_entries = num_entries;
    ECGSHMADDR_HDR->pseudo_entry_0 = max_entries + MAX_ADDED_DEVICES;
    //print_log(LOGMSG,"Mallocing for exer_list. max_entries = %d\n", max_entries);
    fflush (stdout);

/*    printf("Before setting cur_ecg_pos, cur_ecg_pos = %d\n",cur_ecg_pos);
    cur_ecg_pos = 0;
*/

    EXER_LIST =
      (texer_list *) malloc ((max_entries + MAX_ADDED_DEVICES) *
			     sizeof (texer_list));
    if (EXER_LIST == NULL) {     /* problem?                       */
	sprintf (result_msg,
		 "#######ERROR: Unable to allocate memory for exer_list. Errno = %d (%s)#######\n",
		 errno, strerror (errno));
	LOG_MSG (18, result_msg, &mdt_fd);
	DBTRACE(DBEXIT,("return/g 18 build_ipc.c build_ipc\n"));
	return (18);
    }                            /* endif */
    /*
        printf("addr of EXER_LIST = %p, size = %d, sizeof(texer_list) = %d\n",EXER_LIST,((max_entries + MAX_ADDED_DEVICES) *
                             sizeof (texer_list)),sizeof(texer_list));
    */
    bzero (EXER_LIST,
	   (int) ((max_entries + MAX_ADDED_DEVICES) * sizeof (texer_list)));

    REM_CUR->max_entries = max_entries;
    REM_CUR->num_entries = num_entries;

   /* clear hft_devices  (hft information structure) array */
#ifdef HTX_REL_tu320
    for (i = 0; i < 4; i++) {
	for (j = 0; j < 32; j++) {
	    ECGSHMADDR_HDR->hft_devices[i].fileid[j] = 0;
	    ECGSHMADDR_HDR->hft_devices[i].hf_devid[j] = 0;
	}                         /* endfor */
    }                            /* endfor */
#endif

   /* initialize a section of shared memory for each ECG stanza ************* */

   print_log(LOGMSG,"cur_pos:%d\n", cur_ecg_pos);

   if (num_entries == (unsigned int) 0) {    /* no entries?                  */
        sprintf (result_msg,
                 "########ERROR: No entries in the ECG.########\n");
        LOG_MSG (11, result_msg, &mdt_fd);
        DBTRACE(DBEXIT,("return/b 11 build_ipc.c build_ipc\n"));
        return (11);
    }

    /* re-open ECG cfg file */
    mdt_fd = cfgcopsf (full_name);
    if (mdt_fd == (CFG__SFT *) NULL) {
	sprintf (result_msg, "ERROR: Unable to open ECG file again.n");
	LOG_MSG (19, result_msg, (CFG__SFT **) NULL);
	DBTRACE(DBEXIT,("return/h 19 build_ipc.c build_ipc\n"));
	return (19);
    }                            /* endif */
   /* read the default stanza and save it */
    rc =
      cfgcrdsz (mdt_fd, default_mdt_snz, sizeof (default_mdt_snz), "default");

   if (rc == CFG_SUCC) {
	   /* set Equaliser flag */
	   if (equaliser_flag == 0) {
		   retcod = cfgcskwd("equaliser_flag", default_mdt_snz, workstr);
		   if(retcod ==  CFG_SUCC) {
			   equaliser_flag = atoi(unquote(workstr));
		   }
		   else {
			   equaliser_flag = 0;
		   }
		   if (equaliser_flag == 1) {
			   putenv("EQUALISER_FLAG=1");
			   /* Read config file name */
			   retcod = cfgcskwd("cfg_file", default_mdt_snz, workstr);
			   if(retcod ==  CFG_SUCC) {
				   strcpy(cfg_file, unquote(workstr));
			   }
			   else {
				   strcpy(cfg_file, " ");
			   }
			   /*  Read equaliser_debug_flag  */
			   retcod = cfgcskwd("equaliser_debug_flag", default_mdt_snz, workstr);
			   if(retcod ==  CFG_SUCC) {
				   equaliser_debug_flag = atoi(unquote(workstr));
			   }
			   else {
				   equaliser_debug_flag = 0;
			   }
			   /* Check if WOF testing is enabled */
			   retcod = cfgcskwd("wof_test", default_mdt_snz, workstr);
			   if(retcod ==  CFG_SUCC) {
				   wof_test_enabled = atoi(unquote(workstr));
		       }
		  } else {
			  system_call = TRUE;
			  system("unset EQUALISER_FLAG");
			  system_call = FALSE;
		  }
	 }
  }


			/* read each stanza and set values in shm */
    if (rc != CFG_SUCC) {
	sprintf( result_msg, "ecg doesn't contain default stanza\\n");
	LOG_MSG (20, result_msg, (CFG__SFT **) &mdt_fd);
	DBTRACE(DBEXIT,("return/i 20 build_ipc.c build_ipc\n"));
	return (20);
    }                            /* endif */

    ECG_MAX_ENTRIES = 0;
    shm_entries = 0;
    /*
      printf("Addr of ECGSHMADDR_HDR = %p, ECGSHMADDR_HE = %p, cur_ecg_pos = %d\n",(ECGSHMADDR_HDR), ECGSHMADDR_HE,cur_ecg_pos);
    */
    for ((shm_point.hdr_addr = ECGSHMADDR_HDR + 1);
	 ((rc =
	   process_mdt_to_shm (stanza, sizeof (stanza), shm_point,
			       (char **) NULL, mdt_fd)) == CFG_SUCC);
	 (shm_point.HE_addr++), shm_entries++) {
      /* check/reset maximum time allowed on a semop system wait */

		/******************************************************************************/
	dup_dev = 0;
	dup_num = 0;
	//print_log(LOGMSG,"num_ebgs in build_ipc = %d\n", num_ecgs);
	if (shm_point.HE_addr->dup_device) {
	    dup_num = cmprnset_dup_name (shm_point.HE_addr);
	    if (dup_num < 0){
		sprintf( result_msg, "failure in dupdev\\n");
		LOG_MSG (21, result_msg, (CFG__SFT **) &mdt_fd);
		DBTRACE(DBEXIT,("return/j 21 build_ipc.c build_ipc\n"));
		return (21);
	    }
	}
	else {
	    prev_ecg_pos = cur_ecg_pos;
	    cur_ecg_pos = 0;
		/* printf("NUM_EXERS = %d, shm_point.HE_addr->sdev_id = %s\n",NUM_EXERS,shm_point.HE_addr->sdev_id);*/
	    for (ii = 0; ii < NUM_EXERS; ii++) {
		if (ECGEXER_ADDR (ii)
		    &&
		    (strcmp
		     ((ECGEXER_ADDR (ii))->sdev_id,
		      (shm_point.HE_addr)->sdev_id) == 0)) {

		    print_log(LOGMSG,"%s: its already present in shm, so skip this \n ",
			    (shm_point.HE_addr)->sdev_id);
		    /*
			printf("%s: its already present in shm, so skip this ,ii = %d, ECGEXER_ADDR (ii)->sdev_id = %s ",(shm_point.HE_addr)->sdev_id,ii,ECGEXER_ADDR (ii)->sdev_id);
	            */
		    dup_dev = 1;
		    shm_point.HE_addr->sdev_id[0] = '\0';
		    shm_point.HE_addr--;
		    shm_entries--;
		    cur_ecg_pos = prev_ecg_pos;
		    (ECGSHMADDR_HDR->max_entries)--;
		    (ECGSHMADDR_HDR->num_entries)--;
	       /* slot_index--; */
		    break;
		}
	    }
	    cur_ecg_pos = prev_ecg_pos;
	    //print_log(LOGMSG,"cur_pos:%d\n", cur_ecg_pos);
	}
	if (dup_dev)
	    continue;

	//print_log(LOGMSG,"num_entries is :%d ECG_MAX_ENTRIES:%d\n",
	/*printf("num_entries is :%d ECG_MAX_ENTRIES:%d\n",
		 num_entries, ECG_MAX_ENTRIES);*/
	ECGEXER_ADDR (ECG_MAX_ENTRIES) = shm_point.HE_addr;
	ECGEXER_SEMID (ECG_MAX_ENTRIES) = ECGSEMHEID;
	ECGEXER_POS (ECG_MAX_ENTRIES) = shm_entries;
	ECGEXER_ECGPOS (ECG_MAX_ENTRIES) = cur_ecg_pos;
	ECGEXER_HDR (ECG_MAX_ENTRIES) = ECGSHMADDR_HDR;
	ECGEXER_SHMKEY (ECG_MAX_ENTRIES) = ECGSHMKEY;
	ECGEXER_SEMKEY (ECG_MAX_ENTRIES) = ECGSEMKEY;
	strcpy (EXER_NAME (ECG_MAX_ENTRIES), (shm_point.HE_addr)->sdev_id);
	/*printf("ECG_MAX_ENTRIES = %d, sdev_id = %s",ECG_MAX_ENTRIES, (shm_point.HE_addr)->sdev_id); */
	add_to_ecg_all ();
      /*
       *print_log(LOGMSG,"ecgname:%s, filling up new entry for %s\n"
       * "semid:%d shm_pos:%d ecg_pos:%d exer_hdr:%x\n" "shmkey:%d
       * semkey:%d\n",
       * ECGNAME,(shm_point.HE_addr)->sdev_id,ECGSEMHEID,shm_entries
       * , ECG_MAX_ENTRIES,ECGEXER_HDR(ECG_MAX_ENTRIES),
       * ECGEXER_SHMKEY(ECG_MAX_ENTRIES),ECGEXER_SEMKEY(ECG_MAX_ENTR
       * IES));
       */

		/******************************************************************************/

	workint =
	  (shm_point.HE_addr->max_run_tm * 2) + shm_point.HE_addr->idle_time;
	if (workint > max_wait_tm) {      /* new maximum wait time?          */
	    max_wait_tm = workint;
	}                         /* endif */
	ECG_MAX_ENTRIES++;
    }                            /* endfor */

    if (rc != CFG_EOF) {         /* not normal EOF?                   */
	(void) sprintf (msg, "ERROR: ECG file re-read failure CFG_");
	switch (rc) {
	    case CFG_SZNF:           /* req stanza not found         */
		(void) sprintf (tag, "SZNF");
		break;
	    case CFG_SZBF:           /* stanza too long              */
		(void) sprintf (tag, "SZBF");
		break;
	    case CFG_UNIO:           /* I/O error                    */
		(void) sprintf (tag, "UNIO");
		break;
	    default:                 /* unexpected error             */
		(void) sprintf (tag, "????");
		break;
	}                         /* endswitch */
	sprintf (tag, ", rc = %d.", rc);
	htx_strcat (msg, tag);
	sprintf (result_msg, "ERROR: %s\n", msg);
	LOG_MSG (22, result_msg, &mdt_fd);
	DBTRACE(DBEXIT,("return/k 22 build_ipc.c build_ipc\n"));
	return (22);
    }                            /* endif */
   /* close ECG cfg file */
    LOG_MSG (0, "Closing the file... no error message. RET = 0", &mdt_fd);
   /*
    * if (cfgcclsf (mdt_fd) != CFG_SUCC) { sprintf (result_msg,
    * "###########ERROR: Unable to close ECG file.###########\n", errno,
    * strerror (errno)); LOG_MSG (21, result_msg, mdt_fd); return (21);
    * }
 *//* endif */

   /* sort shared memory **************************************************** */

    shm_point.hdr_addr = ECGSHMADDR_HDR + 1;
    //qsort((char *) shm_point.HE_addr, num_entries,
	    //sizeof(struct htxshm_HE), DRT_compar);

   /*
    * Now that the shared memory has been sorted, we can check for any
    * HE's that are set to be halted on startup and set the appropriate
    * semaphore value to accomplish this.
    */
    shm_point.hdr_addr = ECGSHMADDR_HDR + 1;

    for (i = 0; i < num_entries; i++) {
	//print_log(LOGMSG,"devid[%d] = %s\n", i, (shm_point.HE_addr + i)->sdev_id);
	fflush (stdout);
	if ((shm_point.HE_addr + i)->start_halted == 1) {
	    SEMCTL (ECGSEMHEID, ((i * SEM_PER_EXER) + SEM_GLOBAL), SETVAL, 1);
	}                         /* endif */
    }                            /* endfor */

    print_log(LOGMSG,"num_entries = %d hdr = 0x%x max = %d\n", num_entries,
	    (int)ECGSHMADDR_HDR, ECGSHMADDR_HDR->max_entries);
    /* printf("In build_ipc\n"); */
    /* printf("num_entries = %d hdr = 0x%x max = %d\n", num_entries,
	    (int)ECGSHMADDR_HDR, ECGSHMADDR_HDR->max_entries); */

    strcpy (ECGSTATUS, "INACTIVE");
    print_log(LOGMSG,"Done build_ipc, cur_pos:%d\n", cur_ecg_pos);
    fflush (stdout);

    DBTRACE(DBEXIT,("return 0 build_ipc.c build_ipc\n"));
    return (0);
}                               /* build_IPC() */

int
init_rem_ipc (char *result_msg)
{
    extern int rem_shm_id;       /* system shared memory id      */
    extern tmisc_shm *rem_shm_addr;
    DBTRACE(DBENTRY,("enter build_ipc.c init_rem_ipc\n"));

    rem_shm_addr = (tmisc_shm *) malloc (sizeof (tmisc_shm));
    rem_shm_addr->sock_hdr_addr = (tsys_hdr *) 0;        /* no shm yet. */

    rem_shm_id =
      shmget (REMSHMKEY, 4096,
	      IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH |
	      S_IWOTH);
    if (rem_shm_id == -1) {      /* problem?                          */
	sprintf (result_msg,
		 "init_rem_ipc:Error creating shared memory Errno = %d \n",
		 errno);
	print_log(LOGERR,"Error creating shared memory Errno = %d \n", errno);
	DBTRACE(DBEXIT,("return/a 40 build_ipc.c init_rem_ipc\n"));
	return (40);
    }                            /* endif */
    rem_shm_addr->sock_hdr_addr =
      (tsys_hdr *) shmat (rem_shm_id, (char *) 0, 0);
    if ((int) rem_shm_addr->sock_hdr_addr == -1) {       /* problem? */
	sprintf (result_msg,
		 "init_rem_ipc:Error Attaching to shared memory errno=%d\n",
		 errno);
	print_log(LOGERR,"Error Attaching to shared memory(%d, 0x%x). errno=%d\n",
		rem_shm_id, rem_shm_id, errno);
	DBTRACE(DBEXIT,("return/b 41 build_ipc.c init_rem_ipc\n"));
	return (41);
    }                            /* endif */
    rem_shm_addr->sock_hdr_addr->max_exer_entries = MAX_EXER_ENTRIES;
    print_log(LOGMSG,"sock addr = 0x%x id = %d key = %d ***",
	    (int)rem_shm_addr->sock_hdr_addr, rem_shm_id, REMSHMKEY);
    fflush (stdout);
    rem_shm_addr->cur_shm_addr = (tcurr_info *) malloc (sizeof (tcurr_info));
    if (rem_shm_addr->cur_shm_addr == NULL) {
	sprintf (result_msg,
		 "init_rem_ipc:Error mallocing for rem_shm_addr. Errno = %d (%s)\n",
		 errno, strerror (errno));
	print_log
	  (LOGERR,"init_rem_ipc:Error mallocing for rem_shm_addr. Errno = %d (%s)\n",
	   errno, strerror (errno));
	exit (99);
    }                            /* endif */
    print_log(LOGMSG,"cur addr = 0x%x sizeof(tsys_hdr) = %d\n",
	    (int)rem_shm_addr->cur_shm_addr, sizeof (tsys_hdr));
    fflush (stdout);
    print_log(LOGMSG,"Done init_rem_ip****\n");
    fflush (stdout);

    DBTRACE(DBEXIT,("return 0 build_ipc.c init_rem_ipc\n"));
    return 0;
}

int set_sig_handler ()
{
    int i;
    struct sigaction sigvector;  /* structure for signals    */
    struct sigaction old_SIGHUP_vector;  /* old structure for signals    */
    DBTRACE(DBENTRY,("enter build_ipc.c set_sig_handler\n"));

    for (i = 1; i <= SIGMAX; i++) {
	sigemptyset (&(sigvector.sa_mask));       /* do not block signals       */
	sigvector.sa_flags = 0;   /* do not restart system calls on
				 * sigs */
	switch (i) {
	    case SIGHUP:             /* hangup              */
		(void) sigaction (i, (struct sigaction *) NULL, &old_SIGHUP_vector);
	 /*if (old_SIGHUP_vector.sa_handler == SIG_IGN) */  /* nohup in effect? */
		sigvector.sa_handler = (void (*)(int)) SIG_IGN;
	 /*else
	    sigvector.sa_handler = (void (*)(int)) sig_end;*/
		break;
	    case SIGINT:             /* interrupt (rubout)  */
		sigvector.sa_handler = (void (*)(int)) sig_end;
		break;
	    case SIGQUIT:            /* quit (ASCII FS)     */
		sigvector.sa_handler = SIG_IGN;
		break;
	    case SIGILL:             /* illegal instruction */
		sigvector.sa_handler = (void (*)(int)) sig_end;
		break;
	    case SIGTRAP:            /* trace trap          */
		sigvector.sa_handler = SIG_DFL;
		break;
	    case SIGABRT:            /* abort process       */
		sigvector.sa_handler = SIG_DFL;
		break;
#if !defined(__HTX_LINUX__) && !defined(__OS400__)	/* 400 */
	    case SIGEMT:             /* EMT instruction     */
		sigvector.sa_handler = SIG_DFL;
		break;
#endif
	    case SIGFPE:             /* floating point exception    */
		sigvector.sa_handler = (void (*)(int)) sig_end;
		break;
	    case SIGKILL:            /* kill (cannot be caught)     */
		sigvector.sa_handler = SIG_DFL;
		break;
	    case SIGBUS:             /* bus error                   */
		sigvector.sa_handler = (void (*)(int)) sig_end;
		break;
		//case SIGSEGV:         /* segmentation violation      */
		    //  sigvector.sa_handler = (void (*)(int)) sig_end;
		    //  break;
		case SIGSYS:             /* bad argument to system call */
		    sigvector.sa_handler = (void (*)(int)) sig_end;
		    break;
		case SIGPIPE:            /* write on a pipe with no one t */
		    sigvector.sa_handler = (void (*)(int)) sig_end;
		    break;
		case SIGALRM:            /* alarm clock                 */
		    sigvector.sa_handler = (void (*)(int)) alarm_signal;
		    break;
		case SIGTERM:            /* software termination signal */
		    sigvector.sa_handler = (void (*)(int)) sig_end;
		    break;
		case SIGURG:             /* urgent condition on I/O channel */
		    sigvector.sa_handler = SIG_DFL;
		    break;
		case SIGSTOP:            /* stop (cannot be caught or ignored) */
		    sigvector.sa_handler = SIG_DFL;
		    break;
		case SIGTSTP:            /* interactive stop */
		    sigvector.sa_handler = SIG_DFL;
		    break;
		case SIGCONT:            /* continue (cannot be caught or ignored) */
		    sigvector.sa_handler = SIG_DFL;
		    break;
		case SIGCHLD:            /* sent to parent on child stop or exit */
		    sigvector.sa_handler = (void (*)(int)) child_death;
#ifndef __OS400__                           /* 400 */
		    sigvector.sa_flags = SA_RESTART;
#endif
		    break;
		case SIGTTIN:            /* background read attempted from control
				 * terminal */
		    sigvector.sa_handler = SIG_DFL;
		    break;
		case SIGTTOU:            /* background write attempted to control
				 * terminal */
		    sigvector.sa_handler = SIG_DFL;
		    break;
		case SIGIO:              /* I/O possible or completed */
		    sigvector.sa_handler = SIG_IGN;
		    break;
		case SIGXCPU:            /* cpu time limit exceeded (see setrlimit())  */
		    sigvector.sa_handler = SIG_DFL;
		    break;
		case SIGXFSZ:            /* file size limit exceeded (see setrlimit()) */
		    sigvector.sa_handler = SIG_DFL;
		    break;
#if !defined(__HTX_LINUX__) && !defined(__OS400__)	/* 400 */
		case SIGMSG:             /* input data is in the hft ring buffer */
		    sigvector.sa_handler = SIG_IGN;
		    break;
#endif
		case SIGWINCH:           /* window size change */
		    sigvector.sa_handler = SIG_IGN;
		    break;
#ifndef __OS400__               /* 400 */
		case SIGPWR:             /* power-fail restart          */
		    sigvector.sa_handler = (void (*)(int)) sig_end;
		    break;
#endif
		case SIGUSR1:            /* user defined signal         */
		    sigvector.sa_handler = (void (*)(int)) sig_end;
		    break;
		case SIGUSR2:            /* user defined signal         */
		    sigvector.sa_handler = (void (*)(int)) sig_restart_HE;
		    break;
#ifdef _DR_HTX_
		case SIGRECONFIG:        /* user defined signal         */
		    sigvector.sa_handler = (void (*)(int)) DR_handler;
		    break;
#endif
		case SIGPROF:            /* profiling time alarm (see setitimer) */
		    sigvector.sa_handler = SIG_DFL;
		    break;
#if !defined(__HTX_LINUX__) && !defined(__OS400__)	/* 400 */
		case SIGDANGER:          /* system crash imminent (maybe) */
		    sigvector.sa_handler = (void (*)(int)) sig_end;
		    break;
#endif
		case SIGVTALRM:          /* virtual time alarm (see setitimer) */
		    sigvector.sa_handler = SIG_DFL;
		    break;
#if !defined(__HTX_LINUX__) && !defined(__OS400__)	/* 400 */
		case SIGMIGRATE:         /* migrate process (see TCF) */
		    sigvector.sa_handler = SIG_DFL;
		    break;
		case SIGGRANT:           /* monitor mode granted        */
		    sigvector.sa_handler = SIG_IGN;
		    break;
		case SIGRETRACT:         /* monitor mode retracted      */
		    sigvector.sa_handler = SIG_IGN;
		    break;
		case SIGSOUND:           /* sound ack                   */
		    sigvector.sa_handler = SIG_IGN;
		    break;
		case SIGSAK:             /* secure attention key */
		    sigvector.sa_handler = SIG_IGN;
		    break;
#endif
		default:
		    sigvector.sa_handler = SIG_DFL;
		    ;
	}                         /* endswitch */
	(void) sigaction (i, &sigvector, (struct sigaction *) NULL);
    }

    DBTRACE(DBEXIT,("return 0 build_ipc.c set_sig_handler\n"));
    return 0;
}

int add_to_ecg_all (void)
{
    int i;
    DBTRACE(DBENTRY,("enter build_ipc.c add_to_ecg_all\n"));

    if (NUM_EXERS == MAX_EXER_ENTRIES) {
	print_log(LOGERR,"no space in ecg_info structure\n");
	DBTRACE(DBEXIT,("return/a -1 build_ipc.c add_to_ecg_all\n"));
	return -1;
    }
    for (i = 0; i < NUM_EXERS; i++) {
	if (strcmp
	    ((ECGEXER_ADDR (ECG_MAX_ENTRIES))->sdev_id,
	     ecg_info[0].exer_list[i].dev_name) == 0) {
	    DBTRACE(DBEXIT,("return/b build_ipc.c add_to_ecg_all\n"));
	    return i;
	}
    }
    for (i = 0; i < NUM_EXERS; i++) {
	if (ecg_info[0].exer_list[i].exer_addr.HE_addr == 0) {
	   strcpy (ecg_info[0].exer_list[i].dev_name, EXER_NAME (ECG_MAX_ENTRIES));
           ecg_info[0].exer_list[i].exer_pos = ECGEXER_POS (ECG_MAX_ENTRIES);
           ecg_info[0].exer_list[i].parent_ecg_pos = ECGEXER_ECGPOS (ECG_MAX_ENTRIES);
           ecg_info[0].exer_list[i].ecg_exer_addr.hdr_addr = ECGEXER_HDR (ECG_MAX_ENTRIES);
           ecg_info[0].exer_list[i].ecg_shm_key = ECGEXER_SHMKEY (ECG_MAX_ENTRIES);
           ecg_info[0].exer_list[i].ecg_semhe_id = ECGEXER_SEMID (ECG_MAX_ENTRIES);
           ecg_info[0].exer_list[i].ecg_sem_key = ECGEXER_SEMKEY (ECG_MAX_ENTRIES);
           ecg_info[0].exer_list[i].exer_addr.HE_addr = ECGEXER_ADDR (ECG_MAX_ENTRIES);

	   print_log(LOGMSG,"i:%d dev: %s exer_hdr:0x%x, exer_he_addr: 0x%x \n",
           i,ecg_info[0].exer_list[i].dev_name, ecg_info[0].exer_list[i].ecg_exer_addr.hdr_addr,
           ecg_info[0].exer_list[i].exer_addr.HE_addr);


	    return (i);
	}
    }
    if (i == NUM_EXERS) {
	strcpy (ecg_info[0].exer_list[i].dev_name, EXER_NAME (ECG_MAX_ENTRIES));
        ecg_info[0].exer_list[i].ecg_semhe_id = ECGEXER_SEMID (ECG_MAX_ENTRIES);
        ecg_info[0].exer_list[i].exer_addr.HE_addr = ECGEXER_ADDR (ECG_MAX_ENTRIES);
        ecg_info[0].exer_list[i].parent_ecg_pos = ECGEXER_ECGPOS (ECG_MAX_ENTRIES);
        ecg_info[0].exer_list[i].ecg_shm_key = ECGEXER_SHMKEY (ECG_MAX_ENTRIES);
        ecg_info[0].exer_list[i].exer_pos = ECGEXER_POS (ECG_MAX_ENTRIES);
        ecg_info[0].exer_list[i].ecg_sem_key = ECGEXER_SEMKEY (ECG_MAX_ENTRIES);
        ecg_info[0].exer_list[i].ecg_exer_addr.hdr_addr = ECGEXER_HDR (ECG_MAX_ENTRIES);
        //printf("adding at the end\n");
        NUM_EXERS++;
           print_log(LOGMSG,"i:%d dev: %s exer_hdr:0x%x, exer_he_addr: 0x%x \n",
           i,ecg_info[0].exer_list[i].dev_name, ecg_info[0].exer_list[i].ecg_exer_addr.hdr_addr,
           ecg_info[0].exer_list[i].exer_addr.HE_addr);
           /* printf("i:%d dev: %s exer_hdr:0x%x, exer_he_addr: 0x%x \n",
           i,ecg_info[0].exer_list[i].dev_name, ecg_info[0].exer_list[i].ecg_exer_addr.hdr_addr,
           ecg_info[0].exer_list[i].exer_addr.HE_addr); */
	return (i);
    }
    return 0;
}

int rmv_ecg_all (char *mydev_id)
{
    int i;
    DBTRACE(DBENTRY,("enter build_ipc.c rmv_ecg_all\n"));

    for (i = 0; i < NUM_EXERS; i++) {
      /*
       *print_log(LOGMSG,"exer_name:%s\n",ecg_info[0].exer_list[i].dev_name);
       *
       */
	if ((strcmp (mydev_id, ecg_info[0].exer_list[i].dev_name) == 0)
	    && (strcmp (ecg_info[0].exer_list[i].dev_name, "") != 0)) {
	    //print_log(LOGMSG," got the device(:%s:,dup=%d), let me check if he is a duplicate\n", mydev_id, ecg_info[0].exer_list[i].exer_addr.HE_addr->dup_device);
	    fflush (stdout);
	    if (ecg_info[0].exer_list[i].exer_addr.HE_addr->dup_device)
		rmv_dup_info (mydev_id);
	    ecg_info[0].exer_list[i].exer_addr.HE_addr = 0;
	    ecg_info[0].exer_list[i].ecg_semhe_id = 0;
	    ecg_info[0].exer_list[i].exer_pos = 0;
	    ecg_info[0].exer_list[i].ecg_exer_addr.hdr_addr = 0;
	    ecg_info[0].exer_list[i].ecg_shm_key = 0;
	    ecg_info[0].exer_list[i].ecg_sem_key = 0;
	    ecg_info[0].exer_list[i].exer_pid = 0;
	    ecg_info[0].exer_list[i].parent_ecg_pos = 0;
	    sprintf (ecg_info[0].exer_list[i].dev_name, "");
	 /*
	  *print_log(LOGMSG,"removed the exer %s, i:%d heptr:0x%x\n",
	  * mydev_id, i,
	  * ecg_info[0].exer_list[i].exer_addr.HE_addr );
	  * fflush(stdout);
	  */
	    if (i == (NUM_EXERS - 1))
		NUM_EXERS--;
	    DBTRACE(DBEXIT,("return 0 build_ipc.c rmv_ecg_all\n"));
	    return (0);
	}
    }
    //print_log(LOGMSG,"exerciser %s not found in ecg.all, cant del\n", mydev_id);
    DBTRACE(DBEXIT,("return -1 build_ipc.c rmv_ecg_all\n"));
    return -1;
}

int cmprnset_dup_name (struct htxshm_HE *my_exer)
{
    char tmpdev_id2[48];

   /* char *tmp_ptr; */
    int i, ii;
    int found = 0;
    int dup_index = 0;
    int tmp_mask;
    int original = 0;
    int prev_ecg_pos = 0;
    DBTRACE(DBENTRY,("enter build_ipc.c cmprnset_dup_name\n"));

   /* search for any entry present with my name */
    for (i = 0; i < MAX_EXER_ENTRIES; i++) {
	if (strcmp (dup_info[i].sdev_id, "") == 0)
	    continue;
	if (strcmp (my_exer->sdev_id, dup_info[i].sdev_id) == 0) {
	    found = 1;
	    break;
	}
    }
   /* if entry is found, find my index from mask */
    if (found) {
	dup_index = 0;
	//print_log(LOGMSG,"getting my dup_index\n");
	for (ii = 0; ii < MAX_DUP_DEVICES / 32; ii++, dup_index += 32) {
	    if (dup_info[i].dup_mask[ii] != 0xffffffff) {
		tmp_mask = dup_info[i].dup_mask[ii];
		while (tmp_mask & 0x80000000) {
		    tmp_mask = tmp_mask << 1;
		    dup_index++;
		}
		sprintf (tmpdev_id2, "%s(%d)", my_exer->sdev_id, dup_index);
		strcpy (my_exer->sdev_id, tmpdev_id2);
	    /*
	     *print_log(LOGMSG,"added the identifier to
	     * exer,exer_name:%s, index:%d\n",
	     * my_exer->sdev_id,dup_index);
	     */
		dup_info[i].dup_mask[ii] =
		  dup_info[i].dup_mask[ii] | (1 << (31 - (dup_index % 32)));
		DBTRACE(DBEXIT,("return/a build_ipc.c cmprnset_dup_name\n"));
		return (dup_index);
	    }
	}
	print_log
	  (LOGERR,"number of duplicate devices of this name has exceeded the limit\n");
	DBTRACE(DBEXIT,("return/b -1 build_ipc.c cmprnset_dup_name\n"));
	return (-1);
    }
    else {
	//print_log(LOGMSG,"creating entry in dup_struct\n");
      /*
       * if entry is not found, let me creat it for me, i may not
       * be first device with this name, so let me have an index of
       * 1
       */

      /* let me check if i am the first device with this name */
	prev_ecg_pos = cur_ecg_pos;
	cur_ecg_pos = 0;
	for (i = 0; i < NUM_EXERS; i++) {
	    if (strcmp (my_exer->sdev_id, EXER_NAME (i)) == 0) {
		//print_log(LOGMSG,"found a original for this guy %s\n", EXER_NAME(i));
		original = 1;       /* found a device who had
				 * this name */
		ECGEXER_ADDR (i)->dup_device = 1;
	    }
	}
	cur_ecg_pos = prev_ecg_pos;
	for (i = 0; i < MAX_EXER_ENTRIES; i++) {
	    if (strcmp (dup_info[i].sdev_id, "") == 0) {
		strcpy (dup_info[i].sdev_id, my_exer->sdev_id);
		if (original) {
		    dup_index = 1;
		    dup_info[i].dup_mask[0] = 0xc0000000;
		    //print_log(LOGMSG,"the mask is set for both\n");
		    for (ii = 1; ii < MAX_DUP_DEVICES / 32; ii++)
			dup_info[i].dup_mask[ii] = 0;
		    sprintf (tmpdev_id2, "%s(%d)", my_exer->sdev_id, dup_index);
		    strcpy (my_exer->sdev_id, tmpdev_id2);
		    //print_log(LOGMSG,"added the identifier to exer,exer_name:%s, index:%d\n", my_exer->sdev_id, dup_index);
		    DBTRACE(DBEXIT,("return/c build_ipc.c cmprnset_dup_name\n"));
		    return (dup_index);
		}
		else {
		    dup_info[i].dup_mask[0] = 0x80000000;
		    //print_log(LOGMSG,"the mask is set for only one guy\n");
		    for (ii = 1; ii < MAX_DUP_DEVICES / 32; ii++)
			dup_info[i].dup_mask[ii] = 0;
		    //print_log(LOGMSG,"added the identifier to exer,exer_name:%s\n", my_exer->sdev_id);
		    DBTRACE(DBEXIT,("return/d 0 build_ipc.c cmprnset_dup_name\n"));
		    return (0);
		}
	    }
	}
	if (i == MAX_EXER_ENTRIES) {
	    print_log
	      (LOGERR,"the number of exercisers has exceeded the MAX_EXER_ENTRIES\n");
	    DBTRACE(DBEXIT,("return/e -1 build_ipc.c cmprnset_dup_name\n"));
	    return (-1);
	}
    }

    DBTRACE(DBEXIT,("return 0 build_ipc.c cmprnset_dup_name\n"));
    return (0);
}

int rmv_dup_info (char *mydev_id)
{
    char tmpdev_id[DEV_ID_MAX_LENGTH];
    char fulldev_id[DEV_ID_MAX_LENGTH];
    char *tmp_ptr1, *tmp_ptr2;
    int i = 0, ii = 0, dup_index;
    int found = 0;
    int all_mask = 0;
    DBTRACE(DBENTRY,("enter build_ipc.c rmv_dup_info\n"));

    strcpy (fulldev_id, mydev_id);
    tmp_ptr1 = strtok (fulldev_id, "(");
    strcpy (tmpdev_id, tmp_ptr1);
    tmp_ptr2 = strtok (NULL, ")");
    if (tmp_ptr2 != NULL)
	dup_index = atoi (tmp_ptr2);
    else
	dup_index = 0;

    for (i = 0; i < MAX_EXER_ENTRIES; i++) {
	if (strcmp (dup_info[i].sdev_id, "") == 0)
	    continue;
	if (strcmp (tmpdev_id, dup_info[i].sdev_id) == 0) {
	    found = 1;
	    break;
	}
    }

    if (found) {
      /*
       *print_log(LOGMSG,"found the device %s, index : %d clearing the
       * mask\n", tmpdev_id, dup_index);
       */
	dup_info[i].dup_mask[(dup_index) / 32] =
	  dup_info[i].dup_mask[(dup_index) /
			       32] & ~(1 << (31 - (dup_index) % 32));
	//print_log(LOGMSG,"mask:0x%x\n", dup_info[i].dup_mask[(dup_index) / 32]);

	for (ii = 0; ii < MAX_DUP_DEVICES / 32; ii++) {
	    all_mask |= dup_info[i].dup_mask[ii];
	}
	if (!all_mask) {
	    sprintf (dup_info[i].sdev_id, "");
	    //print_log(LOGMSG,"cleared the dup_struct: %s\n", dup_info[i].sdev_id);
	}
	DBTRACE(DBEXIT,("return 0 build_ipc.c rmv_dup_info\n"));
	return 0;
    }
    else {
	print_log(LOGERR," couldn't find the device %s, something wrong\n", tmpdev_id);
	DBTRACE(DBEXIT,("return -1 build_ipc.c rmv_dup_info\n"));
	return (-1);
    }

    DBTRACE(DBEXIT,("leave build_ipc.c rmv_dup_info\n"));
}

int LOG_MSG (int ret, char *res_msg, CFG__SFT ** p_mdt_fd)
{
   /*print_log(LOGMSG,"mdt_fd: 0x%x return code: %d\n", *p_mdt_fd, ret); fflush(stdout);*/
   if (*p_mdt_fd != NULL) {
      if (cfgcclsf (*p_mdt_fd) != CFG_SUCC) {
         sprintf (res_msg,
                  "###########ERROR: Unable to close ECG file.%d (%s)###########\n",
                  errno, strerror (errno));
         print_log(LOGMSG,"res_msg, Result = %s. Returning from LOG_MSG.", res_msg);
         return (9);
      }                         /* endif */
      *p_mdt_fd = (CFG__SFT *)NULL;
   }
   print_log(LOGMSG,"Result = %s. Returning from build_ipc. mdt_fd 0x%x Retcod = %d.\n",
          res_msg,*p_mdt_fd, ret);
      fflush (stdout);
   return 0;
}
