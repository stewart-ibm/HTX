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

/*
 * Define this so that the rest of the code fails.
 */
extern union semun semctl_arg;
/* extern thtx_message msg_rcv;   use definition from global.h */
extern tmisc_shm *rem_shm_addr;
extern tfull_info info_send;

int query_all(char *result_msg, char **dev_list, int num_list, int *num_disp)
{
    int i = 0;                   /* loop counter                      */
    int num_entries = 0;         /* local number of shm HE entries    */
    int semval = 0;              /* semaphore values                  */
    long clock;

    struct dst_entry *q_table = NULL;    /* points to begin ahd seq tbl */
    struct dst_entry *p_into_table = NULL;       /* points into ahd sequence table */
    struct htxshm_HE *p_htxshm_HE = NULL;        /* pointer to a htxshm_HE struct */
    struct semid_ds sembuffer;   /* semaphore buffer */
    struct tm *p_tm;
    unsigned long long min_cycles_done = 0xffffffffffffffff;
    unsigned long long max_cycles_done = 0;

    int num_found, found, tmp, rc = 0;
    int loop, max_loops, max_found = 0, ecg_cnt, found_ecg;
    DBTRACE(DBENTRY,("enter query.c query_all\n"));

/*************************************************************************/

    /* printf("\n In query_all funstion\n"); */
    /* printf("List[0] = %s num = %d rem_addr = 0x%x\n", dev_list[0], num_list,
           (int)rem_shm_addr); */
    print_log(LOGMSG,"List[0] = %s num = %d rem_addr = 0x%x\n", dev_list[0], num_list,
	   (int)rem_shm_addr);

    if (num_list == 0) {
	sprintf(result_msg, "%s", "No valid exerciser list provided");
	DBTRACE(DBEXIT,("return/a -1 query.c query_all\n"));
	return -1;
    }

    *num_disp = 0;
    max_found = 0;
    loop = 0;
    if (num_list == -1)          //GUI Client. Not to be true for this file
	max_loops = 1;
    else {
	if (num_ecg == 0) {
	    max_loops = 1;
	    strcpy(ecg[0], "/ecg.all");
	}
	else
	    max_loops = num_ecg;   // One loop per ECG
    }

    /* printf("max_loop = %d\n", max_loops); */
    print_log(LOGMSG,"max_loop = %d\n", max_loops);

    for (loop = 0; loop < max_loops; loop++) {
      /********** for command line client *********/
	if (num_list != -1) {     //If selected ecgs have to be searched
	  for (ecg_cnt = 0; ecg_cnt < num_ecgs; ecg_cnt++) {
	      found_ecg = 0;
	      cur_ecg_pos = ecg_cnt;
	      PUT_FULL_ECG;
	      if (strcmp(full_name, ecg[loop]) == 0) {
		  /* printf("found ECGNAME = :%s/%s: ecgname = :%s:\n", ECGPATH,
                         ECGNAME, ecg[loop]); */
		  print_log(LOGMSG,"found ECGNAME = :%s/%s: ecgname = :%s:\n", ECGPATH,
			 ECGNAME, ecg[loop]);
		  cur_ecg_pos = ecg_cnt;
		  found_ecg = 1;
		  break;
	      }
	  }
	}
      /********** Done: for command line client *********/

	else if (num_list == -1) {        // for GUI client, ecg will be definitely be present
	    found_ecg = 1;
	}

	if (!found_ecg)
	    continue;
	found_ecg = 0;

	sprintf(result_msg, "query:No Valid device or ecg name provided");
	REM_CUR->max_entries = ECG_MAX_ENTRIES;
	/* printf("REM_MAX = %d \n", REM_CUR->max_entries); */
	print_log(LOGMSG,"REM_MAX = %d \n", REM_CUR->max_entries);
	fflush(stdout);
	if (ECG_MAX_ENTRIES == 0) {       /* no HE programs? */
	    printf("No hardware programs defined in %s/%s \n",
                    ECGPATH, ECGNAME);
	    sprintf(result_msg, "No hardware programs defined in %s/%s \n",
		    ECGPATH, ECGNAME);
	    continue;
	}

	else {
	    /* printf("\n Before calling init_dst_tbl\n"); */
	    num_entries = init_dst_tbl(&q_table);
	    /* printf("\n After calling init_dst_tbl, num_entries = %d\n",num_entries); */
	    REM_CUR->num_entries = num_entries;
	    if (num_entries <= 0) {
		/* printf("Number of entries in %s/%s are %d \n",
                        ECGPATH, ECGNAME, num_entries); */
		sprintf(result_msg, "Number of entries in %s/%s are %d \n",
			ECGPATH, ECGNAME, num_entries);
		continue;
	    }
	 /*else {
	    max_page = ((num_entries - 1) / 34) + 1;
   } *//* endif */
	}                         /* endif */

      /* Current time ******************************************************* */
	clock = time((long *) 0);
	p_tm = localtime(&clock);
	cpy_tm(REM_SOCK->curr_time ,*p_tm)

	min_cycles_done = 0xffffffffffffffff;
	max_cycles_done = 0;

      /*
       ***  build screen data for the current page  ***************************
       */
	p_into_table = q_table;   // + ((page - 1) * 34);

	*num_disp = (num_list > 0) ? num_list : num_entries;

/*************************************************************************/

	/* printf("*num_disp = %d num-entries = %d, num_list = %d\n", *num_disp,
	       num_entries, num_list); */
	print_log(LOGMSG,"*num_disp = %d num-entries = %d, num_list = %d\n", *num_disp,
	       num_entries, num_list);
	fflush(stdout);

	num_found = -1;
	for (i = 0; i < num_entries; i++) {

	    if (num_list >= 0) {
		found = 0;
		for (tmp = 0; tmp < *num_disp; tmp++) {
		    p_htxshm_HE = ECGEXER_ADDR(p_into_table->shm_pos);
		    //print_log(LOGMSG,"loop: tmp = %d. dev = %s list = %s\n",tmp, p_htxshm_HE->sdev_id,dev_list[tmp]); fflush(stdout);
		    if (strcmp(p_htxshm_HE->sdev_id, dev_list[tmp]) == 0) {
			/* printf("found: dev = %s list = %s\n", p_htxshm_HE->sdev_id,
			       dev_list[tmp]); */
			print_log(LOGMSG,"found: dev = %s list = %s\n", p_htxshm_HE->sdev_id,
			       dev_list[tmp]);
			fflush(stdout);
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
		//print_log(LOGMSG,"num_list is less than 0. ECG = %d.:%s/%s: pos = %d\n",cur_ecg_pos,ECGPATH,ECGNAME, p_into_table->shm_pos); fflush(stdout);
		num_found++;
		if (i >= *num_disp)
		    break;
		p_htxshm_HE = ECGEXER_ADDR(p_into_table->shm_pos);
	    }
	    //clock = time((long *) 0);
	    print_log(LOGMSG,"query all, i = %d, cur_ecg_pos = %d, p_into_table->shm_pos = %d, num_found = %d, max_found = %d\n",i, cur_ecg_pos, p_into_table->shm_pos, num_found, max_found);

	    semval =
	      semctl(ECGEXER_SEMID(p_into_table->shm_pos),
		     ((ECGEXER_POS(p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), GETVAL,
		     &sembuffer);

	    if (semval == 0 && p_htxshm_HE->equaliser_halt == 0) {     /* set to run? */
		sprintf(INFO_SEND_5(max_found + num_found).status_ah, "ACTIVE");
	    }

	    else {                 /* halted. */
		sprintf(INFO_SEND_5(max_found + num_found).status_ah, "Suspend");
	    }                      /* endif */

	    if (p_htxshm_HE->cont_on_err == 0) {
		semval =
		  semctl(ECGEXER_SEMID(p_into_table->shm_pos),
			 ((ECGEXER_POS(p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL + 1),
			 GETVAL, &sembuffer);

		if (semval != 0) {
		    sprintf(INFO_SEND_5(max_found + num_found).status_ah, "ERROR");
		}                   /* endif */
	    }                      /* endif */

	    print_log(LOGMSG,"query all, i = %d, cur_ecg_pos = %d, slot_port = %s, sdev_id = %s, adapt_desc = %s, device_desc = %s\n", i, cur_ecg_pos, p_htxshm_HE->slot_port, p_htxshm_HE->sdev_id, p_htxshm_HE->adapt_desc, p_htxshm_HE->device_desc);

	    htx_strcpy(INFO_SEND_5(max_found + num_found).slot_port,
		       p_htxshm_HE->slot_port);
	    htx_strcpy(INFO_SEND_5(max_found + num_found).sdev_id,
		       p_htxshm_HE->sdev_id);
	    htx_strcpy(INFO_SEND_5(max_found + num_found).adapt_desc,
		       p_htxshm_HE->adapt_desc);
	    htx_strcpy(INFO_SEND_5(max_found + num_found).device_desc,
		       p_htxshm_HE->device_desc);
	    if (p_htxshm_HE->cont_on_err == 1)
		htx_strcpy(INFO_SEND_5(max_found + num_found).status_coe, "COE");
	    else
		htx_strcpy(INFO_SEND_5(max_found + num_found).status_coe, "SOE");

	    if (ECGEXER_HDR(p_into_table->shm_pos)->started == 0) {
		htx_strcpy(INFO_SEND_5(max_found + num_found).status, "__");        /* System not started. */
	    }

	    else if (p_htxshm_HE->PID == 0) {
		if (p_htxshm_HE->user_term == 0) {
		    htx_strcpy(INFO_SEND_5(max_found + num_found).status, "DD");     /* HE is dead. */
		}

		else {
		    htx_strcpy(INFO_SEND_5(max_found + num_found).status, "TM");     /* HE term by user rq */
		}
	    }

	    else
		if ((semctl
		     (ECGEXER_SEMID(p_into_table->shm_pos), ECGEXER_POS(0),
		      GETVAL, &sembuffer) != 0)
		    || (p_htxshm_HE->equaliser_halt == 1) ||
		    (semctl
		     (ECGEXER_SEMID(p_into_table->shm_pos),
		      ((ECGEXER_POS(p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), GETVAL,
		      &sembuffer) != 0)) {
		    htx_strcpy(INFO_SEND_5(max_found + num_found).status, "ST");        /* HE is stopped. */
		}

		else
		    if (semctl
			(ECGEXER_SEMID(p_into_table->shm_pos),
			 ((ECGEXER_POS(p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL + 1), GETVAL,
			 &sembuffer) != 0) {
			htx_strcpy(INFO_SEND_5(max_found + num_found).status, "ER");        /* HE is stopped on an error. */
		    }

		    else if (p_htxshm_HE->no_of_errs > 0) {
			if (semctl
			    (ECGEXER_SEMID (p_into_table->shm_pos),
			     ((ECGEXER_POS(p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL + 1), GETVAL,
			     &sembuffer) == 0)
			    htx_strcpy (INFO_SEND_5 (max_found + num_found).status, "PR");

		    }

		    else if ((p_htxshm_HE->max_cycles != 0)
			     && (p_htxshm_HE->cycles >= p_htxshm_HE->max_cycles)) {
			htx_strcpy(INFO_SEND_5(max_found + num_found).status, "CP");        /* HE is done. */
		    }

		    else if ((clock - p_htxshm_HE->tm_last_upd) >
			     ((long)(p_htxshm_HE->max_run_tm + p_htxshm_HE->idle_time))) {
			htx_strcpy(INFO_SEND_5(max_found + num_found).status, "HG");        /* HE is hung. */
		    }

		    else {
			htx_strcpy(INFO_SEND_5(max_found + num_found).status, "RN");        /* HE is running. */
		    }

	    if ((htx_strcmp(INFO_SEND_5(max_found + num_found).status, "ST") !=
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

	    INFO_SEND_5(max_found + num_found).tm_last_upd =
	      p_htxshm_HE->tm_last_upd;
	    INFO_SEND_5(max_found + num_found).cycles = p_htxshm_HE->cycles;
	    INFO_SEND_5(max_found + num_found).test_id = p_htxshm_HE->test_id;
	    INFO_SEND_5(max_found + num_found).num_errs = p_htxshm_HE->no_of_errs;
	    INFO_SEND_5(max_found + num_found).err_ack = p_htxshm_HE->err_ack;
	    INFO_SEND_5(max_found + num_found).tm_last_err =
	      p_htxshm_HE->tm_last_err;

	    p_into_table++;

	}                         /* endfor */
	max_found = max_found + num_found + 1;
	*num_disp = max_found;

      /*if (max_found == 1) {
	 strcat(result_msg,"No valid exerciser list provided");
	 continue;
	 } */

	if (q_table != NULL) {
	    free(q_table);
	    q_table = NULL;
	}
    }
    DBTRACE(DBEXIT,("return query.c query_all\n"));
    /* printf("\n rc from query_all = %d\n",rc); */
    return rc;
}

int init_q_tbl(struct ahd_entry **p_query_tbl_ptr)
{
    char workstr[128];           /* work string */
    extern union shm_pointers shm_addr;  /* shared memory union pointers */
    int i;                       /* loop counter */
    int num_entries;             /* local number of entries */
    union shm_pointers shm_addr_wk;      /* work ptr into shm */
    DBTRACE(DBENTRY,("enter query.c init_q_tbl\n"));

   /*
    * allocate space for ahd order table
    */
    if (*p_query_tbl_ptr != NULL) {
	free((char *) *p_query_tbl_ptr);
	*p_query_tbl_ptr = NULL;
    }

    if ((num_entries = (shm_addr.hdr_addr)->max_entries) == 0) {
	print_log
	  (LOGMSG,"query:There are no Hardware Exerciser programs currently executing.\n");
	DBTRACE(DBEXIT,("return/a -1 query.c init_q_tbl\n"));
	return (-1);
    }                            /* endif */

    *p_query_tbl_ptr =
      (struct ahd_entry *) calloc((unsigned) num_entries,
				  sizeof(struct ahd_entry));
    if (*p_query_tbl_ptr == NULL) {
	sprintf(workstr, "Unable to allocate memory for ahd sort.  errno = %d.",
		errno);
	print_log(LOGERR,"Unable to allocate memory for ahd sort.  errno = %d.", errno);
	send_message(workstr, errno, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
	DBTRACE(DBEXIT,("return/b -1 query.c init_q_tbl\n"));
	return (-1);
    }                            /* endif */

   /*
    * build ahd display order table
    */
    shm_addr_wk.hdr_addr = shm_addr.hdr_addr;    /* copy addr to work space */
    (shm_addr_wk.hdr_addr)++;    /* skip over header */

    for (i = 0; i < num_entries; i++) {
	(*p_query_tbl_ptr + i)->shm_pos = i;
	(*p_query_tbl_ptr + i)->slot = (shm_addr_wk.HE_addr + i)->slot;
	(*p_query_tbl_ptr + i)->port = (shm_addr_wk.HE_addr + i)->port;
	strcpy(((*p_query_tbl_ptr + i)->sdev_id),
	       ((shm_addr_wk.HE_addr + i)->sdev_id));
    }                            /* endfor */

    DBTRACE(DBEXIT,("return query.c init_q_tbl\n"));
    return (num_entries);
}                               /* init_q_tbl() */
