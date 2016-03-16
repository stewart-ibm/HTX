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
#if !defined(__OS400__)  /* 400 */
#include <sys/poll.h>
#include <sys/select.h>
#endif

/* extern thtx_message msg_rcv;   use definition from global.h */
int glbl_cycles_flag = 1;

int
disp_dst (char *result_msg, char **dev_list, int num_list, int *num_disp)
{
   /*
    ***  variable definitions  *************************************************
    */
    int error = 0;               /* error flag (1 = not ack'ed)       */
    int i;                       /* loop counter                      */
    int key_input;               /* input from keyboard               */
    int max_page;                /* max dst page number               */
    int num_entries = 0;         /* local number of shm HE entries    */
    int page = 1;                /* current dst page                  */
    unsigned long long min_cycles_done = 0xffffffffffffffff;
    unsigned long long max_cycles_done = 0;

    long clock;                  /* current time in seconds           */


    struct dst_entry *p_dst_table = NULL;        /* points to begin dst seq tbl  */
    struct dst_entry *p_into_table;      /* points into dst sequence table    */

    struct htxshm_HE *p_htxshm_HE;       /* pointer to a htxshm_HE struct     */

    struct semid_ds sembuffer;   /* semaphore buffer                  */

    struct tm *p_tm;             /* pointer to tm structure           */

    int num_found, found, tmp;
    int loop, max_loops, max_found = 0, ecg_cnt, found_ecg;
    DBTRACE(DBENTRY,("enter disp_dst.c disp_dst\n"));

    print_log(LOGMSG,"List = %s num = %d rem_addr = 0x%x\n", dev_list[0], num_list,
	    (int)rem_shm_addr);

    if (num_list == 0) {
	sprintf (result_msg, "%s", "No valid exerciser list provided");
	DBTRACE(DBEXIT,("return/a -1 disp_dst.c disp_dst\n"));
	return -1;
    }

    *num_disp = 0;
    max_found = 0;
    loop = 0;
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

	if (!found_ecg) {
	    continue;
	}
	found_ecg = 0;

	sprintf (result_msg, "disp_dst:No Valid device or ecg name provided");
	REM_CUR->max_entries = ECG_MAX_ENTRIES;
	print_log(LOGMSG,"REM_MAX = %d \n", REM_CUR->max_entries);
	fflush (stdout);
	if (ECG_MAX_ENTRIES == 0) {       /* no HE programs? */
	    REM_CUR->num_entries = 0;
	    print_log(LOGERR,"No hardware programs defined in %s/%s \n", ECGPATH,
		    ECGNAME);
	    sprintf (result_msg, "No hardware programs defined in %s/%s \n",
		     ECGPATH, ECGNAME);
	    continue;
	}

	else {
	    num_entries = init_dst_tbl (&p_dst_table);
	    REM_CUR->num_entries = num_entries;
	    if (num_entries <= 0) {
		print_log(LOGMSG,"Number of entries in %s/%s are %d \n", ECGPATH, ECGNAME,
			num_entries);
		sprintf (result_msg, "Number of entries in %s/%s are %d \n",
			 ECGPATH, ECGNAME, num_entries);
		if (p_dst_table != NULL) {
		    free (p_dst_table);        /* rel mem for dst table */
		    p_dst_table = NULL;
		}
		continue;
	    }
	    else {
		max_page = ((num_entries - 1) / 34) + 1;
	    }                      /* endif */
	}                         /* endif */

      /* Current time ******************************************************* */
	clock = time ((long *) 0);
	p_tm = localtime (&clock);
	cpy_tm(REM_SOCK->curr_time, *p_tm)

	min_cycles_done = 0xffffffffffffffff;
	max_cycles_done = 0;

      /*
       ***  build screen data for the current page  ***************************
       */
	page = msg_rcv.indx;
	if (page < 1)
	    page = 1;
	p_into_table = p_dst_table + ((page - 1) * 34);
	*num_disp = num_entries - ((page - 1) * 34);

	if (*num_disp > 34) {
	    *num_disp = 34;
	}

      /*if (num_list >0) {
	 *num_disp = num_list;
	 }
	 else if (num_list == -2) {
	 *num_disp = num_entries;
	 } */

	*num_disp =
	  (num_list >
	   0) ? num_list : ((num_list == -2) ? num_entries : *num_disp);

	print_log(LOGMSG,
	  "In disp dst entering for loop num_list = %d *num_disp = %d,num_entries = %d\n",
	   num_list, *num_disp, num_entries);
	fflush (stdout);

  /******************** MAIN LOOP STARTS HERE *********************/
	num_found = -1;
	for (i = 0; i < num_entries; i++) {
	    if (num_list >= 0) {
		found = 0;
		for (tmp = 0; tmp < num_list; tmp++) {
		    //print_log(LOGMSG,"dev = %s list = %s\n",p_htxshm_HE->sdev_id,dev_list[tmp]);
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
	 /*
	  * determine HE status
	  */
	    if (((ECGEXER_HDR (p_into_table->shm_pos)->started) == 0)
		&& (ECGEXER_ADDR (p_into_table->shm_pos)->is_child)) {
		htx_strcpy (INFO_SEND_5 (max_found + num_found).status, "  ");      /* System not started. */
	    }

	    else if (p_htxshm_HE->PID == 0) {
		if (p_htxshm_HE->user_term == 0) {
		    htx_strcpy (INFO_SEND_5 (max_found + num_found).status, "DD");   /* HE is dead. */
		}
		else {
		    htx_strcpy (INFO_SEND_5 (max_found + num_found).status, "TM");   /* HE term by user rq */
		}
	    }
	    else if (p_htxshm_HE->no_of_errs > 0) {
		if (semctl
		    (ECGEXER_SEMID (p_into_table->shm_pos),
		     ((p_into_table->shm_pos * SEM_PER_EXER) + SEM_GLOBAL + 1), GETVAL,
		     &sembuffer) != 0)
		    htx_strcpy (INFO_SEND_5 (max_found + num_found).status, "ER");
		else
		    if (semctl
			(ECGEXER_SEMID (p_into_table->shm_pos),
			 ((p_into_table->shm_pos * SEM_PER_EXER) + SEM_GLOBAL + 1), GETVAL,
			 &sembuffer) == 0)
			htx_strcpy (INFO_SEND_5 (max_found + num_found).status, "PR");

	    }

	    else
		if ((semctl
		     (ECGEXER_SEMID (p_into_table->shm_pos), 0, GETVAL,
		      &sembuffer) != 0)
		    || (p_htxshm_HE->equaliser_halt == 1) ||
		    (semctl
		     (ECGEXER_SEMID (p_into_table->shm_pos),
		      ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), GETVAL,
		      &sembuffer) != 0)) {
		    htx_strcpy (INFO_SEND_5 (max_found + num_found).status, "ST");      /* HE is stopped. */
		}

		else
		    if (semctl
			(ECGEXER_SEMID (p_into_table->shm_pos),
			 ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL + 1), GETVAL,
			 &sembuffer) != 0) {
			htx_strcpy (INFO_SEND_5 (max_found + num_found).status, "ER");      /* HE is stopped on an error. */
		    }
		    else if ((p_htxshm_HE->max_cycles != 0)
			     && (p_htxshm_HE->cycles >= p_htxshm_HE->max_cycles)) {
			htx_strcpy (INFO_SEND_5 (max_found + num_found).status, "CP");      /* HE is done. */
		    }

		    else if ((clock - p_htxshm_HE->tm_last_upd) >
			     ((long)(p_htxshm_HE->max_run_tm + p_htxshm_HE->idle_time))) {
			htx_strcpy (INFO_SEND_5 (max_found + num_found).status, "HG");      /* HE is hung. */
		    }

		    else {
			htx_strcpy (INFO_SEND_5 (max_found + num_found).status, "RN");      /* HE is running. */
		    }

	 /*
	  * build output characters for entry
	  */
	    if ((htx_strcmp (INFO_SEND_5 (max_found + num_found).status, "ST") !=
		 0)) {
		if (p_htxshm_HE->cycles < min_cycles_done) {
		    min_cycles_done = p_htxshm_HE->cycles;
		    REM_CUR->min_cycles_done = min_cycles_done;
		}

		if (p_htxshm_HE->cycles >= max_cycles_done) {
		    max_cycles_done = p_htxshm_HE->cycles;
		    REM_CUR->max_cycles_done = max_cycles_done;
		}
	    }

	 /*
	  * HE status
	  */

	    htx_strcpy (INFO_SEND_5 (max_found + num_found).sdev_id,
			p_htxshm_HE->sdev_id);

	    INFO_SEND_5 (max_found + num_found).tm_last_upd =
	      p_htxshm_HE->tm_last_upd;
	    INFO_SEND_5 (max_found + num_found).cycles = p_htxshm_HE->cycles;
	    INFO_SEND_5 (max_found + num_found).test_id = p_htxshm_HE->test_id;
	    INFO_SEND_5 (max_found + num_found).err_ack = p_htxshm_HE->err_ack;
	    INFO_SEND_5 (max_found + num_found).tm_last_err =
	      p_htxshm_HE->tm_last_err;

	    p_into_table++;
	}                         /* endfor */
	// *num_disp = num_found +1;
	max_found = max_found + num_found + 1;
	*num_disp = max_found;
	print_log(LOGMSG,"max_found = %d num_found = %d\n", max_found, num_found);

	if (num_found == -1) {
	    sprintf (result_msg, "%s", "No valid exerciser list provided");
	    if (p_dst_table != NULL) {
		free (p_dst_table);        /* rel mem for dst table */
		p_dst_table = NULL;
	    }
	    continue;
	    //return -1;
	}

	if (msg_rcv.subcmd == SUBCMD_a) {
	    for (i = 0; i < num_entries; i++) {
		(ECGEXER_ADDR (i))->err_ack = 1;
	    }                      /* endfor */
	}
      /*
       * check for unack'ed errors
       */

	REM_CUR->error_flag = 0;
	if (error == 0) {
	    for (i = 0; i < num_entries; i++) {
		if ((ECGEXER_ADDR (i))->err_ack == 0) {
		    error = 1;
		    print_log(LOGERR,"Setting error flag = %d\n", error);
		    REM_CUR->error_flag = 1;
		    strcpy (ECGSTATUS, "PARTIALLY RUNNING");
		    strcpy (active_ecg_name, ECGNAME);
		    i = num_entries;
		}                   /* endif */
		if (p_htxshm_HE->no_of_errs > 0) {
		    strcpy (ECGSTATUS, "PARTIALLY RUNNING");
		    strcpy (active_ecg_name, ECGNAME);
                }
	    }                      /* endfor */
	}                         /* endif */


	key_input = msg_rcv.subcmd;

	switch (key_input) {
	    case SUBCMD_f:
	    case SUBCMD_F:
		if (page < max_page) {
		    page++;
		}
		break;

	    case SUBCMD_b:
	    case SUBCMD_B:
		if (page > 1) {
		    page--;
		}
		break;

	    case SUBCMD_q:
	    case SUBCMD_Q:
		if (p_dst_table != NULL) {
		    free (p_dst_table);        /* rel mem for dst table */
		    p_dst_table = NULL;
		}
		DBTRACE(DBEXIT,("return/b 0 disp_dst.c disp_dst\n"));
		return (0);
	    case SUBCMD_a:
	    case SUBCMD_A:
		error = 0;             /* clear error flag           */
		REM_CUR->error_flag = 0;
		for (i = 0; i < num_entries; i++) {
		    (ECGEXER_ADDR (i))->err_ack = 1;
		}                      /* endfor */
		strcpy (ECGSTATUS, "ACTIVE");
		strcpy (active_ecg_name, ECGNAME);
		break;
	    case SUBCMD_c:
	    case SUBCMD_C:
		if (glbl_cycles_flag)
		    glbl_cycles_flag = 0;
		else
		    glbl_cycles_flag = 1;
		break;
#if !defined(__HTX_LINUX__) && !defined(__OS400__)
	    case KEY_SLL:            /* Locator select (mouse buttons) */
	    case KEY_LOCESC:         /* Locator report following...    */
		(void) fflush (stdin);
	 /* Discard following locator data in the input buffer. */
		break;
#endif

	    default:
		break;
	}                         /* endswitch */
	if (p_dst_table != NULL) {
	    free (p_dst_table);        /* rel mem for dst table */
	    p_dst_table = NULL;
	}
    }
   print_log(LOGMSG,"p_dst_table = 0x%x\n",(int)p_dst_table); fflush(stdout);
    DBTRACE(DBEXIT,("return 0 disp_dst.c disp_dst\n"));
    return 0;
}                               /* disp_dst() */


/*****************************************************************************/
/*****  i n i t _ d s t _ t b l ( )  *****************************************/
/*****************************************************************************/
/*                                                                           */
/* FUNCTION NAME  =    init_dst_tbl()                                        */
/*                                                                           */
/* DESCRIPTIVE NAME =  Initialize Device Status Table                        */
/*                                                                           */
/* FUNCTION =          Initializes and sorts the devices status table        */
/*                                                                           */
/* INPUT =             p_dst_tbl_ptr: pointer to a pointer which will (after */
/*                         the function completes) point to the device       */
/*                         status table                                      */
/*                                                                           */
/* OUTPUT =            The sorted device status table                        */
/*                                                                           */
/* NORMAL RETURN =     The number of entries in the device status table      */
/*                                                                           */
/* ERROR RETURN =      -1 to indicate no hardware exercisers currently in    */
/*                         memory.                                           */
/*                     -2 to indicate an unsuccessful attempt to allocate    */
/*                         memory for the device status table.               */
/*                                                                           */
/* EXTERNAL REFERENCES                                                       */
/*                                                                           */
/*    OTHER ROUTINES = DST_compar() - compares hardware exerciser entries    */
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
init_dst_tbl (struct dst_entry **p_dst_tbl_ptr)
{
    char workstr[128];           /* work string                       */

    int i;                       /* loop counter                      */
    int j;                       /* loop counter                      */
    int num_entries;             /* local number of entries           */
    DBTRACE(DBENTRY,("enter disp_dst.c init_dst_tbl\n"));

   /*
    ***  allocate space for dst order table  ***********************************
    */
    print_log(LOGMSG,"Entering init_dst_tbl\n");
    /* printf("Entering init_dst_tbl\n"); */
    fflush (stdout);
    if (*p_dst_tbl_ptr != NULL) {
	free (*p_dst_tbl_ptr);
	*p_dst_tbl_ptr = NULL;
    }
    /* printf("\n init_dst_tbl, pid = %d\n",getpid()); */
    print_log(LOGMSG,"ECG_MAX_ENTRIES = %d\n", ECG_MAX_ENTRIES);
    /* printf("ECG_MAX_ENTRIES = %d\n", ECG_MAX_ENTRIES); */
    fflush (stdout);
    if ((num_entries = ECG_MAX_ENTRIES) == 0) {
	 printf(
	  "There are no Hardware Exerciser programs currently executing.");
	fflush(stdout);
	print_log(LOGMSG,
	  "There are no Hardware Exerciser programs currently executing.");
	DBTRACE(DBEXIT,("return/a -1 disp_dst.c init_dst_tbl\n"));
	return (-1);
    }                            /* endif */

    /* printf("sizeof(struct dst_entry) = %d\n",sizeof (struct dst_entry)); */
    fflush(stdout);

    *p_dst_tbl_ptr =
      (struct dst_entry *) calloc ((size_t) num_entries,
				   (size_t) sizeof (struct dst_entry));

    if (*p_dst_tbl_ptr == NULL) {
	(void) sprintf (workstr,
			"Unable to allocate memory for DST sort.  errno = %d.",
			errno);
	/* printf("%s", workstr);
	fflush(stdout); */
	print_log(LOGMSG,"%s", workstr);
	(void) send_message (workstr, errno, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
	DBTRACE(DBEXIT,("return/b -2 disp_dst.c init_dst_tbl\n"));
	return (-2);
    }                            /* endif */
    else{
	/* printf("calloc successful\n"); */
	fflush(stdout);
	}
   /*
    ***  build dst display order table  ****************************************
    */

    /* printf("In init_dst_tbl(), before for loop\n"); */
	fflush(stdout);

    /* printf("addr of ecg_info[%d].exer_list[0] = %p\n",cur_ecg_pos,&(ecg_info[cur_ecg_pos].exer_list[0]));
     printf("addr of ecg_info[%d].exer_list[0].exer_addr.HE_addr = %p\n",cur_ecg_pos,&(ecg_info[cur_ecg_pos].exer_list[0].exer_addr.HE_addr));
    printf("addr of ecg_info[%d].exer_list[0].exer_addr.HE_addr->sdev_id = %p\n",cur_ecg_pos,&(ecg_info[cur_ecg_pos].exer_list[0].exer_addr.HE_addr->sdev_id));
    printf("addr of ecg_info[%d].exer_list[164].exer_addr.HE_addr = %p\n",cur_ecg_pos,&(ecg_info[cur_ecg_pos].exer_list[164].exer_addr.HE_addr));
    printf("ecg_info[%d].exer_list[164].exer_addr.HE_addr->HE_name = %s\n",cur_ecg_pos,ecg_info[cur_ecg_pos].exer_list[164].exer_addr.HE_addr->HE_name);
    printf("ecg_info[%d].exer_list[164].exer_addr.HE_addr->PID = %d\n",cur_ecg_pos,ecg_info[cur_ecg_pos].exer_list[164].exer_addr.HE_addr->PID);
    printf("addr of ecg_info[%d].exer_list[164].exer_addr.HE_addr->sdev_id = %p\n",cur_ecg_pos,&(ecg_info[cur_ecg_pos].exer_list[164].exer_addr.HE_addr->sdev_id));
   */

    i = 0;
    for (j = 0; j < num_entries; j++) {
	    /* printf("In for loop, i = %d, j = %d, sdev_id = %s\n",i, j, (ECGEXER_ADDR (i))->sdev_id); */
	if ((ECGEXER_ADDR (i))->sdev_id[0] == '\0')       /* blank entry?   */
	 {
	    /*printf("In if part, j = %d,sdev_id = %s\n",j,(ECGEXER_ADDR (i))->sdev_id);*/
	    j--;
	 }
	else {
	    /*printf("In else part, i = %d, sdev_id = %s\n",i,(ECGEXER_ADDR (i))->sdev_id);*/
	    (*p_dst_tbl_ptr + j)->shm_pos = i;
	    /* printf("(*p_dst_tbl_ptr + j)->shm_pos = %d\n",(*p_dst_tbl_ptr + j)->shm_pos); */
	    /* printf("(ECGEXER_ADDR (i))->sdev_id = %s\n",((ECGEXER_ADDR (i))->sdev_id)); */
	    (void) strcpy (((*p_dst_tbl_ptr + j)->sdev_id),
			   ((ECGEXER_ADDR (i))->sdev_id));
	}                         /* endif */

	/*if((i - j) > 2 ) {
	  num_entries = j;
	  break;
	}*/

	 i++;
    }                            /* endfor */
    /* printf("In init_dst_tbl(), after for loop\n"); */
	fflush(stdout);

    /* printf("\n In init_dst_tbl(), before qsort \n"); */
	fflush(stdout);
    qsort ((void *) *p_dst_tbl_ptr, (size_t) num_entries,
	   (size_t) sizeof (struct dst_entry),
	   (int (*)(const void *, const void *)) DST_compar);
    /* printf("\n In init_dst_tbl(), after qsort \n"); */
	fflush(stdout);

   /* printf("*p_dst_tbl_ptr = 0x%x\n",(int)*p_dst_tbl_ptr ); fflush(stdout); */
	fflush(stdout);
   print_log(LOGMSG,"*p_dst_tbl_ptr = 0x%x\n",(int)*p_dst_tbl_ptr ); fflush(stdout);
    /* printf("returning from init_dst_tbl\n"); */
	fflush(stdout);
    print_log(LOGMSG,"returning from init_dst_tbl\n");
    fflush (stdout);
    DBTRACE(DBEXIT,("return disp_dst.c init_dst_tbl\n"));
    /* printf("\n In init_dst_tbl, num_entries = %d\n",num_entries); */
	fflush(stdout);
    return (num_entries);
}                               /* init_dst_tbl() */
