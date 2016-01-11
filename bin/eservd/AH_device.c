
/* @(#)18	1.21.5.2  src/htx/usr/lpp/htx/bin/eservd/AH_device.c, eserv_daemon, htxubuntu 12/29/14 23:43:17 */


#include "eservd.h"
#include "global.h"

#ifndef __HTX_LINUX__
#define SEMCTL(x,y,z,a) (void)semctl(x,y,z,a)
#else
#define SEMCTL(x,y,z,a) semctl_arg.val=a, (void)semctl(x,y,z,semctl_arg)
#endif


#if defined(__HTX_LINUX__) || defined(__OS400__)    /* 400 */
char *
regcmp (char *arg1, char *arg2)
{
   if (arg1 == NULL) {
      return NULL;
   }

   if (arg2 == NULL) {
      return arg1;
   }

   strcat (arg1, arg2);
   return arg1;
}

char *
regex (char *arg1, char *arg2)
{
   extern char *__loc1;         /* beginning of reg exp match */
   if (arg1 == NULL || arg2 == NULL) {
      return NULL;
   }
   else {
      while (*arg1++ != *arg2++) ;

      if (arg1 != NULL && arg2 != NULL) {
         __loc1 = (--arg1);
      }
      else {
         __loc1 = NULL;
         return NULL;
      }
      while (*arg1++ == *arg2++) ;

      return (++arg1);
   }

   return NULL;
}
#endif

/* extern thtx_message msg_rcv;   use definition from global.h */
extern tmisc_shm *rem_shm_addr;
extern char reg_exp[10];



/*****************************************************************************/
/*****  A H _ d e v i c e ( )  ***********************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     AH_device.c                                           */
/*                                                                           */
/* DESCRIPTIVE NAME =  Activate/Halt devices                                 */
/*                                                                           */
/* FUNCTION =          Activates/Halts one or more devices.                  */
/*                                                                           */
/* INPUT =             p_loc_thres: pointer to the hft data structure which  */
/*                         contains the current locator (mouse) threshold    */
/*                         values                                            */
/*                                                                           */
/*                     Operator keyboard and/or mouse input                  */
/*                                                                           */
/* OUTPUT =            Device Active/Halted Status screen(s)                 */
/*                                                                           */
/*                     Updated system semaphore structure to activate/halt   */
/*                     individual hardware exerciser programs                */
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
/*                                    devices                                */
/*                     send_message - outputs a message to the screen and    */
/*                                    the message log file                   */
/*                                                                           */
/*    DATA AREAS =     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                     SEMHEKEY semaphores (identified by the semhe_id       */
/*                        variable)                                          */
/*                                                                           */
/*    MACROS =         CLRLIN - Clear line CURSES macro                      */
/*                              (defined in common/hxiconv.h)                */
/*                     PRTMSG - Print message CURSES macro                   */
/*                              (defined in common/hxiconv.h)                */
/*                                                                           */
/*****************************************************************************/

int
AH_device (char *result_msg, char **dev_list, int num_list, int *num_disp)
{
  /***************************************************************************/
  /**  variable definitions  *************************************************/
  /***************************************************************************/
    boolean_t run_device;        /* TRUE = activate, FALSE = halt    */

    char *cmp_regexp;            /* ptr to compiled reg expression    */
    char input[32];              /* input string                      */
    char ret[3];                 /* return array for regex()    */
    char *work;                  /* work int */
    char workstr[128];           /* work string                       */

    extern char *__loc1;         /* beginning of reg exp match */

    int err_semval;              /* error semaphore value */
    int i;                       /* loop counter                      */
    int max_strt_ent;            /* max starting entry                */
    int num_entries = 0;         /* local number of shm HE entries    */
    int semval;                  /* semaphore values                  */
    int strt_ent = 1;            /* first entry on screen             */

    struct ahd_entry *p_ahd_table = NULL;        /* points to begin ahd seq tbl  */
    struct ahd_entry *p_into_table;      /* points into ahd sequence table    */

    struct htxshm_HE *p_htxshm_HE;       /* pointer to a htxshm_HE struct     */

    struct semid_ds sembuffer;   /* semaphore buffer                  */

    int num_found, found, tmp, rc = 0;
    int loop, max_loops, max_found = 0, ecg_cnt, found_ecg;

    DBTRACE(DBENTRY,("enter AH_device.c AH_device\n"));


  /***************************************************************************/
  /**  beginning of executable code  *****************************************/
  /***************************************************************************/


  /***************************************************************************/
  /**  loop until operator says to quit  *************************************/
  /***************************************************************************/
    memset (result_msg, 0, 80);

    sprintf (result_msg, "AH_device:No Valid device or ecg name provided");
    strt_ent = (msg_rcv.indx > 0) ? msg_rcv.indx : 1;

    *num_disp = 0;
    max_found = 0;
    loop = 0;

    if (num_list == 0) {
	sprintf (result_msg, "%s", "No valid exerciser list or ecg name provided");
	DBTRACE(DBEXIT,("return/a -1 AH_device.c AH_device\n"));
	return -1;
    }
    //max_loops = (num_list==-1)?1:((num_ecg==0)?num_ecgs:num_ecg);
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
	      print_log(LOGMSG,"num_list = %d ECGNAME = :%s/%s: ecgname = :%s:\n",
		      num_list, ECGPATH, ECGNAME, ecg[loop]);
	      PUT_FULL_ECG;
	      if (strcmp (full_name, ecg[loop]) == 0) {
		  print_log(LOGMSG,"found ECGNAME = :%s/%s: ecgname = :%s:\n", ECGPATH,
			  ECGNAME, ecg[loop]);
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


	if (ECG_MAX_ENTRIES == 0) {       /* no HE programs?        */
	 /*print_log(LOGMSG,"AH_device:There are no Hardware Exerciser programs currently defined.");*/
	    sprintf (result_msg,
		     "AH_device:There are no Hardware Exerciser programs currently defined.");
	    REM_CUR->num_entries = 0;
	    continue;
	}
	else {                    /* some HE's defined.                */
	    REM_CUR->max_entries = ECG_MAX_ENTRIES;
	    if (ECG_MAX_ENTRIES != num_entries) {  /* # HE change? */
		print_log(LOGMSG,"AH_Device: creating table\n");
		num_entries = init_ahd_tbl (&p_ahd_table);
		REM_CUR->num_entries = num_entries;
		if (num_entries <= 0) {
		    if (p_ahd_table != NULL) {
			free ((char *) p_ahd_table);        /* release memory for ahd order tbl */
			p_ahd_table = NULL;
		    }
		    continue;        //return(num_entries);
		}
		else {              /* init fcn ran ok.                */
		    if (num_entries <= 14) { /* 15 or fewer entries?          */
			max_strt_ent = 1;
		    }
		    else {           /* more than 15 entries          */
			max_strt_ent = num_entries;
		    }                /* endif */
		}                   /* endif */
	    }                      /* endif */
	}                         /* endif */

      /***********************************************************************/
      /**  build screen data for the current strt_ent  ***********************/
      /***********************************************************************/
	print_log(LOGMSG,"num_entries:%d str_ent:%d\n", num_entries, strt_ent);
	p_into_table = p_ahd_table + strt_ent - 1;
	*num_disp = num_entries - strt_ent + 1;
	if (*num_disp > 14)
	    *num_disp = 14;

      /*if (num_list >= 0)
	 *num_disp = num_list;
	 else if (num_list == -2)
	 *num_disp = num_entries; */

	*num_disp =
	  (num_list >
	   0) ? num_list : ((num_list == -2) ? num_entries : *num_disp);
	print_log(LOGMSG,"*num_disp = %d num-entries = %d\n", *num_disp, num_entries);
	fflush (stdout);

	num_found = -1;

	for (i = 0; i < num_entries; i++) {

	    //print_log(LOGMSG,"AH_device: num_list: %d\n", num_list);
	    if (num_list >= 0) {
		found = 0;
		for (tmp = 0; tmp < *num_disp; tmp++) {
		    //print_log(LOGMSG,"loop: dev = %s list = %s\n",p_htxshm_HE->sdev_id,dev_list[tmp]);
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
	    print_log(LOGMSG,"NUM_FOUND = %d\n", num_found);


	      /*** Regular Active/Halted Status ******************************/
	    semval = semctl (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), GETVAL, &sembuffer);  /* get sem value of entry */

	    print_log(LOGMSG,"AH_Device:getting the status\n");
	    if (semval == 0) {     /* set to run? */
		sprintf (INFO_SEND_2 (max_found + num_found).status_ah, "ACTIVE");
	    }

	    else {                 /* halted. */
		sprintf (INFO_SEND_2 (max_found + num_found).status_ah,
			 "Suspend");
	    }                      /* endif */

	    if (p_htxshm_HE->cont_on_err == 0) {
		semval = semctl (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL + 1), GETVAL, &sembuffer);   /* get sem value of entry */

		if (semval != 0) {
		    sprintf (INFO_SEND_2 (max_found + num_found).status_ah,
			     "ERROR");
		}                   /* endif */
		else {
		    if (p_htxshm_HE->no_of_errs > 0)

			sprintf (INFO_SEND_2 (max_found + num_found).status_ah,
				 "PRTLYRN");
		}
	    }
	 /* endif */
	    else {
		if (p_htxshm_HE->no_of_errs > 0)
		    sprintf (INFO_SEND_2 (max_found + num_found).status_ah,
			     "PRTLYRN");
	    }

	    strcpy (INFO_SEND_2 (max_found + num_found).slot_port,
		    p_htxshm_HE->slot_port);
	    strcpy (INFO_SEND_2 (max_found + num_found).sdev_id,
		    p_htxshm_HE->sdev_id);
	    strcpy (INFO_SEND_2 (max_found + num_found).adapt_desc,
		    p_htxshm_HE->adapt_desc);
	    strcpy (INFO_SEND_2 (max_found + num_found).device_desc,
		    p_htxshm_HE->device_desc);
	    print_log(LOGMSG,"st = %s id = %s dd = %s ad = %s pt = %s\n",
		    INFO_SEND_2 (max_found + num_found).status_ah,
		    INFO_SEND_2 (max_found + num_found).sdev_id,
		    INFO_SEND_2 (max_found + num_found).device_desc,
		    INFO_SEND_2 (max_found + num_found).adapt_desc,
		    INFO_SEND_2 (max_found + num_found).slot_port);

	    p_into_table++;

	}                         /* endfor */
	if (num_found == -1) {
	    sprintf (result_msg, "%s", "No valid exerciser list provided");
	    if (p_ahd_table != NULL) {
		free ((char *) p_ahd_table);        /* release memory for ahd order tbl */
		p_ahd_table = NULL;
	    }
	    DBTRACE(DBEXIT,("return/b -1 AH_device.c AH_device\n"));
	    return -1;
	}

	input[0] = msg_rcv.subcmd;
	switch (input[0]) {
	    case SUBCMD_a:		/* activate all devices */
	    case SUBCMD_A:
		for (i = 0; i < *num_disp; i++) {
		    run_device = FALSE;
	    /* compute pointers for this device */
		    p_into_table = p_ahd_table + (strt_ent - 1) + (i);
		    p_htxshm_HE = ECGEXER_ADDR (p_into_table->shm_pos);

	    /* get semaphore value of entry  */
		    semval = semctl (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), GETVAL, &sembuffer);       /* get sem value of entry */
	    /* get semaphore value of entry */
		    err_semval = semctl (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL + 1), GETVAL, &sembuffer);       /* get sem value of entry */

	    /* not COE and halted on error now */
		    if (p_htxshm_HE->cont_on_err == 0 && err_semval == 1) {
			run_device = TRUE;
			(void) sprintf (workstr,
					"Request to BYPASS  HALT ON ERROR CONDITION for %s device issued by operator.",
					p_htxshm_HE->sdev_id);
			print_log(LOGMSG,"Request to BYPASS HALT ON ERROR CONDITION for %s device issued by operator.",
			   p_htxshm_HE->sdev_id);
		    }                   /* if */

	    /*       halted                 */
	    /*                      AND                                     */
	    /* HOE & not halted on error OR COE */
		    if (semval == 1) {
	       /* HOE & halted on error OR COE */
			if ((p_htxshm_HE->cont_on_err == 0 && err_semval == 0))
			    run_device = TRUE;
			else if (p_htxshm_HE->cont_on_err == 1 || err_semval == 1) {
			    run_device = TRUE;
			    sprintf (INFO_SEND_2 (i).status_ah, "PRTLYRN");
			    print_log(LOGMSG,"\nStatus of the device is Partially Running");
			    fflush (stdout);
			}

			(void) sprintf (workstr,
					"Request to ACTIVATE %s device issued by operator.",
					p_htxshm_HE->sdev_id);
			print_log(LOGMSG,workstr,
				"Request to ACTIVATE %s device issued by operator.",
				p_htxshm_HE->sdev_id);
		    }                   /* if  */

		    if (run_device) {
	       /* semaphores to run */
			run_device = TRUE;
			//(void)semctl(ECGEXER_SEMID(p_into_table->shm_pos), ((ECGEXER_POS(p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), SETVAL, 0); /* get sem value of entry */
			//(void)semctl(ECGEXER_SEMID(p_into_table->shm_pos), ((ECGEXER_POS(p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL + 1), SETVAL, 0); /* get sem value of entry */
			SEMCTL (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), SETVAL, 0);      /* get sem value of entry */
			SEMCTL (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL + 1), SETVAL, 0);  /* get sem value of entry */
			sprintf (INFO_SEND_2 (i).status_ah, "ACTIVE");
			(void) send_message (workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
		    }                   /* if  */
		}                      /* for */
		break;
	    case SUBCMD_s:                /* halt all devices */
	    case SUBCMD_S:
		for (i = 0; i < *num_disp; i++) {
	    /* compute pointers for this device */
		    p_into_table = p_ahd_table + (strt_ent - 1) + i;
		    p_htxshm_HE = ECGEXER_ADDR (p_into_table->shm_pos);

	    /* get semaphore value of entry  */
		    semval = semctl (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), GETVAL, &sembuffer);       /* get sem value of entry */
	    /* get semaphore value of entry */
		    err_semval = semctl (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL + 1), GETVAL, &sembuffer);       /* get sem value of entry */

	    /* running and not stopped on error */
		    if (semval == 0 && err_semval == 0) {

	       /* semaphore to halt */
			SEMCTL (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), SETVAL, 1);      /* get sem value of entry */

			sprintf (INFO_SEND_2 (i).status_ah, "Suspend");
			(void) sprintf (workstr,
					"Request to SUSPEND %s device issued by operator.",
					p_htxshm_HE->sdev_id);
			print_log(LOGMSG,"Request to SUSPEND %s device issued by operator.",
				p_htxshm_HE->sdev_id);
			(void) send_message (workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
		    }                   /* if  */
		}                      /* for */
		break;
	    default:
		if ((msg_rcv.cmd == 2012) && (msg_rcv.subcmd == SUBCMD_w)) {
		    rc = 0;
		    //cmp_regexp = regcmp(msg_rcv.str, (char *) 0);  /* compile exp */
		    cmp_regexp = regcmp (reg_exp, (char *) 0);  /* compile exp */

		    if (cmp_regexp != NULL) {   /* regular expression OK? */
			for (i = 1; i <= *num_disp; i++) {
			    (void) sprintf (workstr, "%-d", i);
			    work = regex (cmp_regexp, workstr);
			    if (work != NULL) {
				(void) strncpy (ret, " ", (size_t) sizeof (ret));
		     /* clear return (ret) array */
				(void) strncpy (ret, __loc1, (size_t) (work - __loc1));
				if (strcmp (workstr, ret) == 0) {

				    p_into_table = p_ahd_table + (strt_ent - 1) + (i - 1);
				    p_htxshm_HE = ECGEXER_ADDR (p_into_table->shm_pos);
				    semval = semctl (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), GETVAL, &sembuffer);   /* get sem value of entry */
			/* get semaphore value of entry */
				    err_semval = semctl (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL + 1), GETVAL, &sembuffer);   /* get sem value of entry */
				    if ((p_htxshm_HE->cont_on_err == 1)
					|| (err_semval == 0)) {
					if (semval == 0) {   /* set to run?                 */
					    SEMCTL (ECGEXER_SEMID
						    (p_into_table->shm_pos),
						    ((ECGEXER_POS (p_into_table->shm_pos)
						      * SEM_PER_EXER) + SEM_GLOBAL), SETVAL, 1);
					    semval = semctl (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), GETVAL, &sembuffer);     /* get sem value of entry */
					    sprintf (workstr,
						     "Request to SUSPEND %s device issued by operator.semid:%d,exer_pos: %d, ecg_name: %s/%s",
						     p_htxshm_HE->sdev_id,
						     ECGEXER_SEMID (p_into_table->shm_pos),
						     ECGEXER_POS (p_into_table->shm_pos),
						     ECGPATH, ECGNAME);
					}
					else {       /* halted. */
					    SEMCTL (ECGEXER_SEMID (p_into_table->shm_pos),
						    ((ECGEXER_POS (p_into_table->shm_pos) *
						      SEM_PER_EXER) + SEM_GLOBAL), SETVAL, 0);
					    semval = semctl (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), GETVAL, &sembuffer);     /* get sem value of entry */
					    sprintf (workstr,
						     "Request to ACTIVATE \"%s\" device issued by operator.",
						     p_htxshm_HE->sdev_id);
					}
				    }
				    else {  /* release error semaphore wait */
					SEMCTL (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL + 1), SETVAL, 0);      /* clear error semaphore */
					semval = semctl (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), GETVAL, &sembuffer);        /* get sem value of entry */
					sprintf (workstr,
						 "Request to BYPASS SOE CONDITION for %s device issued by operator.",
						 p_htxshm_HE->sdev_id);
				    }       /* endif */
				    if (semval == 0)
					sprintf (INFO_SEND_2 (i - 1).status_ah, "ACTIVE");
				    else if (semval == 1)
					sprintf (INFO_SEND_2 (i - 1).status_ah, "Suspend");
				    send_message (workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
				}
			    }
			}
		    }
		}

		if (msg_rcv.cmd == 2022) {
		    p_into_table = p_ahd_table;
		    print_log(LOGMSG,"activate %s\n", dev_list[0]);
		    fflush (stdout);
		    num_found = -1;
		    for (i = 0; i < num_entries; i++) {
			p_htxshm_HE = ECGEXER_ADDR (p_into_table->shm_pos);
			//print_log(LOGMSG,"searching dev = %s in 2022. \n",p_htxshm_HE->sdev_id);
			fflush (stdout);
			found = 0;
			if (num_list > 0) {
			    for (tmp = 0; tmp < *num_disp; tmp++) {
				if (strcmp (p_htxshm_HE->sdev_id, dev_list[tmp]) == 0) {
				    print_log(LOGMSG,"Found %s in 2022. \n", dev_list[tmp]);
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
			}
			semval = semctl (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), GETVAL, &sembuffer);    /* get sem value of entry */
			if (semval == 1) {
			    SEMCTL (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), SETVAL, 0);   /* clear AH_DEVICE semaphore */
			    semval = semctl (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), GETVAL, &sembuffer); /* get sem value of entry */
			}
			else
			    print_log(LOGMSG,"Exer %s is already in running state\n",
				    p_htxshm_HE->sdev_id);

			semval = semctl (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), GETVAL, &sembuffer);    /* get sem value of entry */
			if (semval == 0)
			    sprintf (INFO_SEND_2 (max_found + num_found).status_ah,
				     "ACTIVE");
			else if (semval == 1)
			    sprintf (INFO_SEND_2 (max_found + num_found).status_ah,
				     "Suspend");
			p_into_table++;
			if ((num_list > 0)
			    && ((max_found + num_found + 1) >= num_list))
			    break;
		    }                   // for num_entries
			max_found = max_found + num_found + 1;
		    *num_disp = max_found;
		}

		if (msg_rcv.cmd == 2032) {
		    p_into_table = p_ahd_table;
		    num_found = -1;
		    for (i = 0; i < num_entries; i++) {
			p_htxshm_HE = ECGEXER_ADDR (p_into_table->shm_pos);
			found = 0;
			if (num_list > 0) {
			    for (tmp = 0; tmp < *num_disp; tmp++) {
				//print_log(LOGMSG,"cmp: dev = %s list[%d] = %s\n",p_htxshm_HE->sdev_id, i, dev_list[tmp]);
				fflush (stdout);
				if (strcmp (p_htxshm_HE->sdev_id, dev_list[tmp]) == 0) {
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
			}

			semval = semctl (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), GETVAL, &sembuffer);    /* get sem value of entry */
			if (semval == 0) {
			    SEMCTL (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), SETVAL, 1);   /* set the  AH_DEVICE semaphore */
			    semval = semctl (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), GETVAL, &sembuffer); /* get sem value of entry */
			}
			else
			    print_log(LOGMSG,"Exer %s is already in Suspended state\n",
				    p_htxshm_HE->sdev_id);
			semval = semctl (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), GETVAL, &sembuffer);    /* get sem value of entry */
			if (semval == 0) {
			    sprintf (INFO_SEND_2 (max_found + num_found).status_ah,
				     "ACTIVE");
		  /*print_log(LOGMSG,"i =%d status = %s\n", i,
			  INFO_SEND_2 (max_found + num_found).status_ah);*/
			}
			else if (semval == 1) {
			    sprintf (INFO_SEND_2 (max_found + num_found).status_ah,
				     "Suspend");
		  /*print_log(LOGMSG,"i = %d status_ah = %s\n", i,
			  INFO_SEND_2 (max_found + num_found).status_ah);*/
			}
			print_log(LOGMSG,"semval = %d\n", semval);
			fflush (stdout);

			p_into_table++;
			if ((num_list > 0)
			    && ((max_found + num_found + 1) >= num_list))
			    break;
		    }                   // for num_entries
			max_found = max_found + num_found + 1;
		    *num_disp = max_found;


		}                      // if 2032
		    break;
	}                         /* endswitch */
	if (p_ahd_table != NULL) {
	    free ((char *) p_ahd_table);        /* release memory for ahd order tbl */
	    p_ahd_table = NULL;
	}
    }
    DBTRACE(DBEXIT,("return AH_device.c AH_device\n"));
    return rc;
}                               /* AH_device() */

/*****************************************************************************/
/*****  i n i t _ a h d _ t b l ( )  *****************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME  =    init_ahd_tbl()                                        */
/*                                                                           */
/* DESCRIPTIVE NAME =  Initialize Activate/Halt Device Table                 */
/*                                                                           */
/* FUNCTION =          Initializes and sorts the activate/halt device table  */
/*                                                                           */
/* INPUT =             p_ahd_tbl_ptr: pointer to a pointer which will (after */
/*                         the function completes) point to the activate/    */
/*                         halt device table                                 */
/*                                                                           */
/* OUTPUT =            The sorted activate/halt device status table          */
/*                                                                           */
/* NORMAL RETURN =     The number of entries in the activate/halt device     */
/*                     table                                                 */
/*                                                                           */
/* ERROR RETURN =      -1 to indicate no hardware exercisers currently in    */
/*                         memory.                                           */
/*                     -2 to indicate an unsuccessful attempt to allocate    */
/*                         memory for the activate/halt device table.        */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = AHD_compar() - compares hardware exerciser entries    */
/*                                    for sequence sort                      */
/*                                                                           */
/*    DATA AREAS =     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                                                                           */
/*    MACROS =         PRTMSG - Print message CURSES macro                   */
/*                              (defined in common/hxiconv.h)                */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

int
init_ahd_tbl (struct ahd_entry **p_ahd_tbl_ptr)
{
    char workstr[128];           /* work string                       */

    int i;                       /* loop counter                      */
    int num_entries;             /* local number of entries           */
    DBTRACE(DBENTRY,("enter AH_device.c init_ahd_tbl\n"));

  /***************************************************************************/
  /**  allocate space for ahd order table  ***********************************/
  /***************************************************************************/
    if (*p_ahd_tbl_ptr != NULL)
	free ((char *) *p_ahd_tbl_ptr);
    if ((num_entries = ECG_MAX_ENTRIES) == 0) {
	print_log(LOGMSG,"AH_device:There are no Hardware Exerciser programs currently executing.\n");
	DBTRACE(DBEXIT,("return/a -1 AH_device.c init_ahd_tbl\n"));
	return (-1);
    }                            /* endif */
    *p_ahd_tbl_ptr = (struct ahd_entry *) calloc ((unsigned) num_entries,
						  sizeof (struct ahd_entry));
    if (*p_ahd_tbl_ptr == NULL) {
	(void) sprintf (workstr,
			"Unable to allocate memory for ahd sort.  errno = %d.",
			errno);
	(void) send_message (workstr, errno, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
	DBTRACE(DBEXIT,("return/b -1 AH_device.c init_ahd_tbl\n"));
	return (-1);
    }                            /* endif */

  /***************************************************************************/
  /**  build ahd display order table  ****************************************/
  /***************************************************************************/

    for (i = 0; i < num_entries; i++) {
	(*p_ahd_tbl_ptr + i)->shm_pos = i;
	(*p_ahd_tbl_ptr + i)->slot = (ECGEXER_ADDR (i))->slot;
	(*p_ahd_tbl_ptr + i)->port = (ECGEXER_ADDR (i))->port;
	(void) strcpy (((*p_ahd_tbl_ptr + i)->sdev_id),
		       ((ECGEXER_ADDR (i))->sdev_id));
    }                            /* endfor */

    DBTRACE(DBEXIT,("return AH_device.c init_ahd_tbl\n"));
    return (num_entries);
}                               /* init_ahd_tbl() */
