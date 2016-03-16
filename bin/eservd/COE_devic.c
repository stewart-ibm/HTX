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


#if !defined(__HTX_LINUX__)   /* 400 */
#define SEMCTL(x,y,z,a) (void)semctl(x,y,z,a)
#else
#define SEMCTL(x,y,z,a) semctl_arg.val=a, (void)semctl(x,y,z,semctl_arg)
#endif

/* extern thtx_message msg_rcv;   use definition from global.h */

extern char reg_exp[10];


/*****************************************************************************/
/*****  C O E _ d e v i c e ( )  *********************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME =     COE_device.c                                          */
/*                                                                           */
/* DESCRIPTIVE NAME =  Show/Set/Clear "Continue on Error" Flag(s) for        */
/*                     Device(s)                                             */
/*                                                                           */
/* FUNCTION =          Displays all the devices' current COE (Continue On    */
/*                     Error) flag states and allows the operator to change  */
/*                     the COE flags.                                        */
/*                                                                           */
/* INPUT =             Operator keyboard input.                              */
/*                                                                           */
/* OUTPUT =            Device "Continue on Error" Flag Screen(s)             */
/*                                                                           */
/*                     Updated COE flags in shared memory.                   */
/*                                                                           */
/* NORMAL RETURN =     0 to indicate successful completion                   */
/*                                                                           */
/* ERROR RETURNS =     -1 to indicate no hardware exerciser programs         */
/*                         currently in memory.                              */
/*                     -2 to indicate an unsuccessful attempt to allocate    */
/*                         memory for the COE device table.                  */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = display_scn -  displays screen image                  */
/*                     init_coe_tbl - initializes and sorts the list of      */
/*                                    devices                                */
/*                     send_message - outputs a message to the screen and    */
/*                                    the message log file                   */
/*                                                                           */
/*    DATA AREAS =     SHMKEY shared memory segment (this segment is pointed */
/*                        to by the shm_addr pointer)                        */
/*                                                                           */
/*    MACROS =         CLRLIN - Clear line CURSES macro                      */
/*                              (defined in common/hxiconv.h)                */
/*                     PRTMSG - Print message CURSES macro                   */
/*                              (defined in common/hxiconv.h)                */
/*                                                                           */
/*****************************************************************************/

int init_coe_tbl (struct ahd_entry **);
#if defined(__HTX_LINUX__) || defined(__OS400__)
extern char * regcmp (char *, char *);
extern char * regex (char *, char *);
#endif

int
COE_device (char *result_msg, char **dev_list, int num_list, int *num_disp)
{
  /***************************************************************************/
  /**  variable definitions  *************************************************/
  /***************************************************************************/
    char *cmp_regexp;            /* ptr to compiled reg expression    */
    char input[32];              /* input string                      */
    char ret[3];                 /* return array for regex()    */
    char *work;                  /* work int */
    char workstr[128];           /* work string                       */

    extern char *__loc1;         /* beginning of reg exp match */

    extern union shm_pointers shm_addr;  /* shared memory union pointers      */

    int i;                       /* loop counter                      */
    int max_strt_ent;            /* max starting entry                */
    //int                 num_disp;        /* number of coe entries to show     */
    int num_entries = 0;         /* local number of shm HE entries    */
    int strt_ent = 1;            /* first entry on screen             */

    struct ahd_entry *p_coe_table = NULL;        /* points to begin coe seq tbl  */
    struct ahd_entry *p_into_table;      /* points into coe sequence table    */

    struct htxshm_HE *p_htxshm_HE;       /* pointer to a htxshm_HE struct     */


    unsigned short err_semnum;   /* the HE error semaphore number.    */
    int num_found, found, tmp, rc = 0;
    int max_loops, loop, max_found, found_ecg, ecg_cnt;
    DBTRACE(DBENTRY,("enter COE_device.c COE_device\n"));

  /***************************************************************************/
  /**  loop until operator says to quit  *************************************/
  /***************************************************************************/
    memset (result_msg, 0, 80);

    sprintf (result_msg, "COE_device:No Valid device or ecg name provided");
    strt_ent = (msg_rcv.indx > 0) ? msg_rcv.indx : 1;

    *num_disp = 0;
    max_found = 0;
    loop = 0;
    print_log(LOGMSG,"COE_device:No Message");
    if (num_list == 0) {
	sprintf (result_msg, "%s", "No valid exerciser list provided");
	DBTRACE(DBEXIT,("return/a -1 COE_device.c COE_device\n"));
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
		  shm_addr.hdr_addr = ECGSHMADDR_HDR;
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
	    print_log(LOGERR,
	      "There are no Hardware Exerciser programs currently defined.");
	    sprintf (result_msg,
		     "COE_device:There are no Hardware Exerciser programs currently defined in %s/%s.\n",
		     ECGPATH, ECGNAME);
	    REM_CUR->num_entries = 0;
	    continue;
	}
	else {                    /* some HE's defined.                */
	    if (ECG_MAX_ENTRIES != num_entries) {  /* # HE change? */
		num_entries = init_coe_tbl (&p_coe_table);
		REM_CUR->num_entries = num_entries;
		if (num_entries <= 0) {
		    if (p_coe_table != NULL) {
			free (p_coe_table);  /* release memory for coe order tbl */
			p_coe_table = NULL;
		    }
		    sprintf (result_msg,
			     "COE_device:Number of entries in %s/%s is %d\n",
			     ECGPATH, ECGNAME, num_entries);
		    continue;        //return(num_entries);
		}
	    /* problem in init fcn - bye!      */
		else {              /* init fcn ran ok.                */
		    if (num_entries <= 14) { /* 14 or fewer entries?          */
			max_strt_ent = 1;
		    }
		    else {           /* more than 14 entries          */
			max_strt_ent = num_entries;
		    }                /* endif */
		}                   /* endif */
	    }                      /* endif */
	}                         /* endif */
	print_log(LOGMSG,"Entries = %d\n", num_entries);
      /***********************************************************************/
      /**  build screen data for the current strt_ent  ***********************/
      /***********************************************************************/

	p_into_table = p_coe_table + strt_ent - 1;
	*num_disp = num_entries - strt_ent + 1;
	if (*num_disp > 14)
	    *num_disp = 14;

	*num_disp =
	  (num_list >
	   0) ? num_list : ((num_list == -2) ? num_entries : *num_disp);

	print_log(LOGMSG,"num_disp = %d num_list = %d\n", *num_disp, num_list);
	fflush (stdout);

	num_found = -1;

	for (i = 0; i < num_entries; i++) {

      /****************************************************************************/
	    if (num_list >= 0) {
		found = 0;
		for (tmp = 0; tmp < *num_disp; tmp++) {
		    //print_log(LOGMSG,"loop: dev = :%s: list = :%s:\n",p_htxshm_HE->sdev_id,dev_list[tmp]);
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
      /****************************************************************************/
	    if (p_htxshm_HE->cont_on_err == 1) {   /* continue on error flag set?   */
		sprintf (INFO_SEND_2 (max_found + num_found).status_coe, "COE");
	    }

	    else {                 /* halted. */
		sprintf (INFO_SEND_2 (max_found + num_found).status_coe, "SOE");
	    }                      /* endif */

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
	print_log(LOGMSG,"processing the subcmd now,num_found:%d\n", num_found);

	if (num_found == -1) {
	    sprintf (result_msg, "%s", "No valid exerciser list provided");
	    if (p_coe_table != NULL) {
		free (p_coe_table);  /* release memory for coe order tbl */
		p_coe_table = NULL;
	    }
	    DBTRACE(DBEXIT,("return/b -1 COE_device.c COE_device\n"));
	    return -1;
	}

	input[0] = msg_rcv.subcmd;

	switch (input[0]) {
	    case SUBCMD_c:
	    case SUBCMD_C:		/* mark all to COE */
		for (i = 0; i < *num_disp; i++) {
		    p_into_table = p_coe_table + (strt_ent - 1) + i;
		    p_htxshm_HE = ECGEXER_ADDR (p_into_table->shm_pos);

		    if (p_htxshm_HE->cont_on_err == 0) {        /* COE flag clear */
			p_htxshm_HE->cont_on_err = 1;
			sprintf (INFO_SEND_2 (i).status_coe, "COE");
	       /* set COE flag */
			(void) sprintf (workstr,
					"Request to SET \"%s\" continue on error flag issued by operator.",
					p_htxshm_HE->sdev_id);
			err_semnum =
			  (unsigned short) ((p_into_table->shm_pos * SEM_PER_EXER) + SEM_GLOBAL + 1);
			SEMCTL (ECGEXER_SEMID (p_into_table->shm_pos),
				(int) err_semnum, SETVAL, 0);
			(void) send_message (workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
		    }                   /* endif */
		}                      /* for */
		break;
	    case SUBCMD_s:
	    case SUBCMD_S:		/* mark all to SOE */
		for (i = 0; i < *num_disp; i++) {
		    p_into_table = p_coe_table + (strt_ent - 1) + i;
		    p_htxshm_HE = ECGEXER_ADDR (p_into_table->shm_pos);

		    if (p_htxshm_HE->cont_on_err == 1) {        /* COE flag set? */
			p_htxshm_HE->cont_on_err = 0;    /* clear COE flag */
			sprintf (INFO_SEND_2 (i).status_coe, "SOE");
			(void) sprintf (workstr,
					"Request to  CLEAR \"%s\" continue on error flag issued by operator.",
					p_htxshm_HE->sdev_id);
			(void) send_message (workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
		    }                   /* if */
		}                      /* for */
		break;
	    default:
		if ((msg_rcv.cmd == 2013) && (msg_rcv.subcmd == SUBCMD_w)) {
		    rc = 0;
		    cmp_regexp = regcmp (reg_exp, (char *) 0);  /* compile exp */
		    if (cmp_regexp != NULL) {   /* regular expression OK? */
			for (i = 1; i <= *num_disp; i++) {
			    (void) sprintf (workstr, "%-d", i);
			    work = regex (cmp_regexp, workstr);
			    if (work != NULL) {
				(void) strncpy (ret, " ", (size_t) sizeof (ret));
		     /* clear return (ret) array */
				(void) strncpy (ret, __loc1, (size_t) (work - __loc1));

				if (strcmp (workstr, ret) == 0) {  /* matched reg exp? */
				    sprintf (result_msg, "No Message");
				    p_into_table = p_coe_table + (strt_ent - 1) + (i - 1);
				    p_htxshm_HE = ECGEXER_ADDR (p_into_table->shm_pos);

				    if (p_htxshm_HE->cont_on_err == 1) {    /* COE flag set?               */
					p_htxshm_HE->cont_on_err = 0;        /* clear COE flag */
					sprintf (INFO_SEND_2 (i - 1).status_coe, "SOE");
					sprintf (workstr,
						 "Request to CLEAR \"%s\" continue on error flag issued by operator.",
						 p_htxshm_HE->sdev_id);
				    }
				    else {  /* COE flag clear */
					p_htxshm_HE->cont_on_err = 1;        /* set COE flag */
					sprintf (INFO_SEND_2 (i - 1).status_coe, "COE");
					sprintf (workstr,
						 "Request to SET %s continue on error flag issued by operator.",
						 p_htxshm_HE->sdev_id);
					err_semnum =
					  (unsigned short) ((p_into_table->shm_pos * SEM_PER_EXER) +
							    SEM_GLOBAL + 1);
					SEMCTL (ECGEXER_SEMID (p_into_table->shm_pos),
						(int) err_semnum, SETVAL, 0);
				    }       /* endif */
				    send_message (workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
				}
			    }
			}
		    }
		}
		else if (msg_rcv.cmd == 2023) {
		    rc = 0;
		    sprintf (result_msg, "No Message");
		    p_into_table = p_coe_table;
		    num_found = -1;
		    print_log(LOGMSG,"COE num_entries = %d num_disp = %d.\n", num_entries,
			    *num_disp);
		    fflush (stdout);
		    for (i = 0; i < num_entries; i++) {
			p_htxshm_HE = ECGEXER_ADDR (p_into_table->shm_pos);
			found = 0;
			if (num_list > 0) {
			    for (tmp = 0; tmp < *num_disp; tmp++) {
				//print_log(LOGMSG,"Comparing for 2023. sdev_id = :%s:, dev_list = :%s:\n",p_htxshm_HE->sdev_id,dev_list[tmp]);
				if (strcmp (p_htxshm_HE->sdev_id, dev_list[tmp]) == 0) {
				    found = 1;
				    num_found++;
				    break;
				}
			    }
			    if (found != 1) {
				//i--;
				p_into_table++;
				//sprintf(INFO_SEND_2(max_found+i).status_coe,"COECOE");
				continue;
			    }
			}
			else {
			    num_found++;
			    if (i >= *num_disp)
				break;
			}

			if (p_htxshm_HE->cont_on_err == 1) {     /* COE flag set?               */
			    print_log(LOGERR,"COE flag is already set for %s \n",
				    p_htxshm_HE->sdev_id);
			    sprintf (workstr,
				     "Request to SET %s continue on error flag issued by operator. Already set",
				     p_htxshm_HE->sdev_id);
			}
			else {           /* COE flag clear */
			    p_htxshm_HE->cont_on_err = 1; /* set COE flag */
			    sprintf (INFO_SEND_2 (max_found + num_found).status_coe,
				     "COE");
			    sprintf (workstr,
				     "Request to SET \"%s\" continue on error flag issued by operator.",
				     p_htxshm_HE->sdev_id);
			    SEMCTL (ECGEXER_SEMID (p_into_table->shm_pos), ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL + 1), SETVAL, 0);       /* get sem value of entry */
			}                /* endif */
			send_message (workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
			p_into_table++;
			if ((num_list > 0)
			    && ((max_found + num_found + 1) >= num_list))
			    break;
		    }
		    max_found = max_found + num_found + 1;
		    *num_disp = max_found;
		}
		else if (msg_rcv.cmd == 2033) {
		    rc = 0;
		    sprintf (result_msg, "No Message");
		    p_into_table = p_coe_table;
		    print_log(LOGMSG,"SOE num_entries = %d num_disp = %d.\n", num_entries,
			    *num_disp);
		    fflush (stdout);
		    num_found = -1;
		    for (i = 0; i < num_entries; i++) {
			p_htxshm_HE = ECGEXER_ADDR (p_into_table->shm_pos);
			found = 0;
			if (num_list > 0) {
			    for (tmp = 0; tmp < *num_disp; tmp++) {
				//print_log(LOGMSG,"Comparing for 2033. sdev_id = :%s:, dev_list = :%s:\n",p_htxshm_HE->sdev_id,dev_list[tmp]);
				if (strcmp (p_htxshm_HE->sdev_id, dev_list[tmp]) == 0) {
				    found = 1;
				    num_found++;
				    break;
				}
			    }
			    if (found != 1) {
				//i--;
				p_into_table++;
				//sprintf(INFO_SEND_2(max_found+i).status,"SOECOE");
				continue;
			    }
			}
			else {
			    num_found++;
			    if (i >= *num_disp)
				break;
			}

			if (p_htxshm_HE->cont_on_err == 1) {     /* COE flag set?               */
			    p_htxshm_HE->cont_on_err = 0; /* clear COE flag */
			    sprintf (INFO_SEND_2 (max_found + num_found).status_coe,
				     "SOE");
			    sprintf (workstr,
				     "Request to CLEAR continue on error flag issued by operator for %s",
				     p_htxshm_HE->sdev_id);
			    print_log(LOGMSG,
			      "Request to CLEAR continue on error flag issued by operator for %s",
			       p_htxshm_HE->sdev_id);
			}

			else {           /* COE flag clear */
			    sprintf (workstr,
				     "Request to CLEAR \"%s\" continue on error flag issued by operator.Already clear",
				     p_htxshm_HE->sdev_id);
			    print_log(LOGMSG,
			      "Request to CLEAR \"%s\" continue on error flag issued by operator.Already clear",
			       p_htxshm_HE->sdev_id);
			    err_semnum =
			      (unsigned short) ((p_into_table->shm_pos * SEM_PER_EXER) + SEM_GLOBAL + 1);
			}                /* endif */
			send_message (workstr, 0, HTX_SYS_INFO, HTX_SYS_MSG);
			p_into_table++;
			if ((num_list > 0)
			    && ((max_found + num_found + 1) >= num_list))
			    break;

		    }
		    max_found = max_found + num_found + 1;
		    *num_disp = max_found;
		}
		break;
	}                         /* endswitch */
	if (p_coe_table != NULL) {
	    free (p_coe_table);  /* release memory for coe order tbl */
	    p_coe_table = NULL;
	}
    }

    DBTRACE(DBEXIT,("return COE_device.c COE_device\n"));
    return rc;
}                               /* COE_device() */


/*****************************************************************************/
/*****  i n i t _ c o e _ t b l ( )  *****************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME  =    init_coe_tbl()                                        */
/*                                                                           */
/* DESCRIPTIVE NAME =  Initialize Continue on Error Flag Table.              */
/*                                                                           */
/* FUNCTION =          Initializes and sorts the COE flag device table       */
/*                                                                           */
/* INPUT =             p_coe_tbl_ptr: pointer to a pointer which will (after */
/*                         the function completes) point to the COE flag     */
/*                         device table.                                     */
/*                                                                           */
/* OUTPUT =            The sorted COE flag device status table               */
/*                                                                           */
/* NORMAL RETURN =     The number of entries in the COE flag device table.   */
/*                                                                           */
/* ERROR RETURN =      -1 to indicate no hardware exercisers currently in    */
/*                         memory.                                           */
/*                     -2 to indicate an unsuccessful attempt to allocate    */
/*                         memory for the COE flag device table.             */
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
init_coe_tbl (struct ahd_entry **p_coe_tbl_ptr)
{
    char workstr[128];           /* work string                       */

    int i;                       /* loop counter                      */
    int num_entries;             /* local number of entries           */
    DBTRACE(DBENTRY,("enter COE_device.c init_coe_tbl\n"));

   /*
    ***  allocate space for coe order table  ***********************************
    */
    if (*p_coe_tbl_ptr != NULL) {
	free ((char *) *p_coe_tbl_ptr);
	*p_coe_tbl_ptr = NULL;
    }
    if ((num_entries = ECG_MAX_ENTRIES) == 0) {
	print_log(LOGMSG,
	  "There are no Hardware Exerciser programs currently executing.");
	DBTRACE(DBEXIT,("return/a -1 COE_device.c init_coe_tbl\n"));
	return (-1);
    }                            /* endif */
    *p_coe_tbl_ptr = (struct ahd_entry *) calloc ((unsigned) num_entries,
						  sizeof (struct ahd_entry));
    if (*p_coe_tbl_ptr == NULL) {
	(void) sprintf (workstr,
			"Unable to allocate memory for coe sort.  errno = %d.",
			errno);
	print_log(LOGMSG,"%s", workstr);
	(void) send_message (workstr, errno, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
	DBTRACE(DBEXIT,("return/b -1 COE_device.c init_coe_tbl\n"));
	return (-1);
    }                            /* endif */

   /*
    ***  build coe display order table  ****************************************
    */

    for (i = 0; i < num_entries; i++) {
	(*p_coe_tbl_ptr + i)->shm_pos = i;
	(*p_coe_tbl_ptr + i)->slot = (ECGEXER_ADDR (i))->slot;
	(*p_coe_tbl_ptr + i)->port = (ECGEXER_ADDR (i))->port;
	(void) strcpy (((*p_coe_tbl_ptr + i)->sdev_id),
		       ((ECGEXER_ADDR (i))->sdev_id));
    }                            /* endfor */

    DBTRACE(DBEXIT,("return COE_device.c init_coe_tbl\n"));
    return (num_entries);
}                               /* init_coe_tbl() */
