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

extern union semun semctl_arg;
extern tmisc_shm *rem_shm_addr;

int
test_summary (char *test_sum)
{

   int prev_ecg_pos=0;
   int num_entries = 0;         /* local number of shm HE entries    */
   int num, cnt;
   long clock;
   int dev_in = 0, dev_dd = 0, dev_tm = 0, dev_st = 0, dev_er = 0, dev_cp =
      0, dev_rn = 0, dev_hg = 0, dev_pr = 0, dev_total = 0;
   int ecg_ina = 0, ecg_act = 0, ecg_par = 0, ecg_sus = 0, ecg_unl = 0;
   int rc;

   struct dst_entry *q_table = NULL;    /* points to begin ahd seq tbl */
   struct dst_entry *p_into_table = NULL;       /* points into ahd sequence table */
   struct htxshm_HE *p_htxshm_HE = NULL;        /* pointer to a htxshm_HE struct */
   struct semid_ds sembuffer;   /* semaphore buffer */
   DBTRACE(DBENTRY,("enter test_summary.c test_summary\n"));

   print_log(LOGMSG,"in test_summary: num_ecgs = %d.\n", num_ecgs);
   fflush (stdout);

   if (do_init_rem_ipc(test_sum) <0) {
      DBTRACE(DBEXIT,("return/a test_summary.c test_summary\n"));
      return;
   }

   //memset (rem_shm_addr->sock_hdr_addr->test_summary, 0, 3024);
   prev_ecg_pos = cur_ecg_pos;
   for (cnt = 1; cnt < num_ecgs; cnt++) {
      cur_ecg_pos = cnt;

      if (strcmp (ECGSTATUS, "UNLOADED") == 0) {
         rc = get_unloaded_entries(test_sum);
         if (rc <0) {
            DBTRACE(DBEXIT,("return/b test_summary.c test_summary\n"));
            return;
         }
         else
            dev_in = dev_in + rc;
         //continue;
      }
      else {
      print_log(LOGMSG,"test_summary: freeing\n");
      fflush (stdout);
      num_entries = init_dst_tbl (&q_table);
      print_log(LOGMSG,"test_summary: num_entries = %d.\n", num_entries);
      fflush (stdout);

      p_into_table = q_table;
      for (num = 0; num < num_entries; num++) {
         //print_log(LOGMSG,"pos = %d\n",p_into_table->shm_pos); fflush(stdout);
         p_htxshm_HE = ECGEXER_ADDR (p_into_table->shm_pos);
         clock = time ((long *) 0);

         if (ECGEXER_HDR (p_into_table->shm_pos)->started == 0) {
            //ecg_ina++;
            dev_in += num_entries;
            break;
         }

         else if (p_htxshm_HE->PID == 0) {
            if (p_htxshm_HE->user_term == 0) {
               dev_dd++;
            }

            else {
               dev_tm++;
            }
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
            dev_st++;
         }

         else
            if (semctl
                (ECGEXER_SEMID (p_into_table->shm_pos),
                 ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL + 1), GETVAL,
                 &sembuffer) != 0) {
            dev_er++;
            if (p_htxshm_HE->tm_last_err >=
                rem_shm_addr->sock_hdr_addr->last_error_time) {
               REM_CUR->start_time = ECGSTARTTIME;
               strcpy (rem_shm_addr->sock_hdr_addr->error_dev,
                       p_htxshm_HE->sdev_id);
            }
         }

         else if ((p_htxshm_HE->max_cycles != 0)
                  && (p_htxshm_HE->cycles >= p_htxshm_HE->max_cycles)) {
            dev_cp++;
         }

         else if ((clock - p_htxshm_HE->tm_last_upd) >
                  ((long)(p_htxshm_HE->max_run_tm + p_htxshm_HE->idle_time))) {
            dev_hg++;
         }
         else if (p_htxshm_HE->no_of_errs > 0) {
            /*strcpy(ECGSTATUS, "PARTIALLY RUNNING");*/
            dev_pr++;
            if (p_htxshm_HE->tm_last_err >=
                rem_shm_addr->sock_hdr_addr->last_error_time) {
               REM_CUR->start_time = ECGSTARTTIME;
               strcpy (rem_shm_addr->sock_hdr_addr->error_dev,
                       p_htxshm_HE->sdev_id);
            }
         }

         else {
            dev_rn++;
         }
         p_into_table++;
      }
      }
   }
   for (cnt = 1; cnt < num_ecgs; cnt++) {
      //print_log(LOGMSG,"test_summary: num_ecgs = %d. max_entries = %d\n",num_ecgs, (ECGSHMADDR_HDR)->max_entries); fflush(stdout);
      cur_ecg_pos = cnt;

      if (strcmp (ECGSTATUS, "ACTIVE") == 0)
         ecg_act++;
      else if (strcmp (ECGSTATUS, "SUSPENDED") == 0)
         ecg_sus++;
      else if (strcmp (ECGSTATUS, "INACTIVE") == 0)
         ecg_ina++;
      else if (strcmp (ECGSTATUS, "UNLOADED") == 0) {
         ecg_unl++;
         //continue;
      }
      else                      //if (strcmp(ECGSTATUS,"Partially Running") == 0)
         ecg_par++;

   }
   if (q_table != NULL)  {
      free(q_table);
      q_table = NULL;
   }
   dev_total =
      dev_rn + dev_st + dev_er + dev_tm + dev_dd + dev_hg + dev_cp + dev_in;


   if (ecg_par > 0)
      strcpy(rem_shm_addr->sock_hdr_addr->system_status, "ERROR");
   else if (ecg_act == (num_ecgs-1))
      strcpy(rem_shm_addr->sock_hdr_addr->system_status, "ACTIVE");
   else if ((ecg_unl+ecg_ina) == (num_ecgs-1))
      strcpy(rem_shm_addr->sock_hdr_addr->system_status, "INACTIVE");
   else if ((ecg_sus+ecg_unl+ecg_ina) == (num_ecgs-1))
      strcpy(rem_shm_addr->sock_hdr_addr->system_status, "SUSPENDED");
   else if ((ecg_act+ecg_unl+ecg_ina+ecg_sus) == (num_ecgs-1))
      strcpy(rem_shm_addr->sock_hdr_addr->system_status, "PARTIALLY ACTIVE");



   if (!is_cmdline)
      sprintf (test_sum,
               "Total ECGs = %d. ACT(%d), PAR(%d), INA(%d), SUS(%d), UNL(%d). Total Devices = %d, RN(%d), ST(%d), ER(%d), PR(%d), TM(%d), DD(%d), HG(%d), CP(%d), IN(%d)",
               num_ecgs-1, ecg_act, ecg_par, ecg_ina, ecg_sus, ecg_unl,
               dev_total, dev_rn, dev_st, dev_er, dev_pr, dev_tm, dev_dd,
               dev_hg, dev_cp, dev_in);
   else
      sprintf (test_sum,
               "Total ECGs          = %d\n"
               " Active             = %d\n"
               " Partially Running  = %d\n"
               " Inactive           = %d\n"
               " Suspended          = %d\n"
               " Unloaded           = %d\n"
               "Total Devices       = %d\n"
               " Running            = %d\n"
               " Suspended          = %d\n"
               " Error              = %d\n"
               " Partialy Running   = %d\n"
               " Terminated         = %d\n"
               " Died               = %d\n"
               " Hung               = %d\n"
               " Completed          = %d\n"
               " Inactive           = %d\n", (num_ecgs-1), ecg_act, ecg_par,
               ecg_ina, ecg_sus, ecg_unl, dev_total, dev_rn, dev_st, dev_er,
               dev_pr, dev_tm, dev_dd, dev_hg, dev_cp, dev_in);

   print_log(LOGMSG,"Done from test_summary...\n");
   fflush (stdout);
   cur_ecg_pos = prev_ecg_pos;
   DBTRACE(DBEXIT,("return 0 test_summary.c test_summary\n"));
   return 0;
}

do_init_rem_ipc(char *res_msg)
{
      DBTRACE(DBENTRY,("enter test_summary.c do_init_rem_ipc\n"));
      if (!ipc_done) {
         if (init_rem_ipc (res_msg) != 0) {
            DBTRACE(DBEXIT,("return -1 test_summary.c do_init_rem_ipc\n"));
            return -1;
         }
         else {
            init_shm ();
            ipc_done = 1;
         }
      }
      DBTRACE(DBEXIT,("return 0 test_summary.c do_init_rem_ipc\n"));
      return 0;
}



/***
   *	returns the active ecg full name
   *
***/
char * get_active_ecg_full_name(char *ecg_name)
{
	int	ecg_count;
	int     previous_ecg_position   = cur_ecg_pos;

	if(ecg_name == NULL) {
		return NULL;
	}

	ecg_name[0] = '\0';

	for(ecg_count = 0; ecg_count < num_ecgs; ecg_count++) {
		cur_ecg_pos = ecg_count;
		if (strcmp (ECGSTATUS, "ACTIVE") == 0) {
			PUT_FULL_ECG;
			strcpy(ecg_name, full_name);
		}
	}
	cur_ecg_pos = previous_ecg_position;
	return ecg_name;
}


int get_fail_status(char *ecg_path_name)
{
	int			ecg_count;
	int			device_count;
	int			previous_ecg_position		= cur_ecg_pos;
	int			is_ecg_name_found		= FALSE;
	struct dst_entry	*device_position_table		= NULL;
	struct dst_entry	*p_device_position_table	= NULL;
	struct htxshm_HE	*p_htxshm_HE			= NULL;
	struct semid_ds		sem_buffer;
	int			num_device_entries;
	int			dev_dd = 0, 
				dev_tm = 0,
				dev_st = 0,
				dev_er = 0,
				dev_cp = 0,
				dev_ld = 0,
				dev_hg = 0,
				dev_pr = 0,
				dev_rn = 0;
	long			run_status_clock;
	char			ecg_full_name[256];

	if( (ecg_path_name[0] == '\0') || (ecg_path_name == NULL) ) {
		get_active_ecg_full_name(ecg_full_name);
		if(ecg_full_name[0] == '\0') {
			return -3;
		}
	} else {
		strcpy(ecg_full_name, ecg_path_name);
	}


	for(ecg_count = 0; ecg_count < num_ecgs; ecg_count++) {
		cur_ecg_pos = ecg_count;
		PUT_FULL_ECG;
		if( (strcmp(full_name, ecg_full_name) ) == 0) {
			is_ecg_name_found = TRUE;	
			num_device_entries = init_dst_tbl (&device_position_table);
			p_device_position_table = device_position_table;
			for (device_count = 0; device_count < num_device_entries; device_count++) {
				p_htxshm_HE = ECGEXER_ADDR (p_device_position_table->shm_pos);
				run_status_clock = time ((long *) 0);
				if (ECGEXER_HDR (p_device_position_table->shm_pos)->started == 0) {
					break;
				}
				else if (p_htxshm_HE->PID == 0) {
					if (p_htxshm_HE->user_term == 0) {
						dev_dd++;
					} else {
						dev_tm++;
					}
				}
				else if ((semctl(ECGEXER_SEMID (p_device_position_table->shm_pos), 0, GETVAL, &sem_buffer) != 0)
					|| (p_htxshm_HE->equaliser_halt == 1) 
					|| (semctl (ECGEXER_SEMID (p_device_position_table->shm_pos), ((ECGEXER_POS (p_device_position_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL), GETVAL, &sem_buffer) != 0)) {
					dev_st++;
				}
				else if (semctl (ECGEXER_SEMID (p_device_position_table->shm_pos), ((ECGEXER_POS (p_device_position_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL + 1), GETVAL, &sem_buffer) != 0) {
					dev_er++;
				}
				else if ((p_htxshm_HE->max_cycles != 0)
					&& (p_htxshm_HE->cycles >= p_htxshm_HE->max_cycles)) {
						dev_cp++;
				}
				else if(p_htxshm_HE->tm_last_upd == -1) {
					dev_ld++;		
				}
				else if ((run_status_clock - p_htxshm_HE->tm_last_upd) > ((long)(p_htxshm_HE->max_run_tm + p_htxshm_HE->idle_time))) {
					dev_hg++;
				}
				else if (p_htxshm_HE->no_of_errs > 0) {
					dev_pr++;
				}
				else {
					dev_rn++;
				} 
				p_device_position_table++;


			}
			if(device_position_table != NULL) {
				free(device_position_table);
				device_position_table = p_device_position_table = NULL;
			}
			break;
		}
	}
	if(is_ecg_name_found == FALSE) {
		cur_ecg_pos = previous_ecg_position;
		return -2;
	}

	cur_ecg_pos = previous_ecg_position;

	if( (dev_er != 0) || (dev_dd != 0) || (dev_tm != 0) || (dev_cp != 0) || (dev_pr != 0) || (dev_hg != 0) ) {
		return 1;
	} else {
		return 0;
	}
}

/***
   *	calculates running time for the specified ecg in seconds
   *	return : error status
***/
int get_ecg_run_time(long *p_ecg_run_time, char *ecg_path_name)
{
	int	ecg_count;
	int	previous_ecg_position	= cur_ecg_pos;
	long	current_time;
	int	is_ecg_name_found	= FALSE;
	char	ecg_full_name[256];

	*p_ecg_run_time	= 0;

	if( (ecg_path_name[0] == '\0') || (ecg_path_name == NULL) ) {
		get_active_ecg_full_name(ecg_full_name);
		if(ecg_full_name[0] == '\0') {
			return -3;
		}
	} else {
		strcpy(ecg_full_name, ecg_path_name);
	}


	for(ecg_count = 0; ecg_count < num_ecgs; ecg_count++) {
		cur_ecg_pos = ecg_count;
		PUT_FULL_ECG;
		if( (strcmp(full_name, ecg_full_name) ) == 0) {
			is_ecg_name_found = TRUE;
			current_time = time((long *) 0);	
			if(ECGSTARTTIME == 0) {
				*p_ecg_run_time = 0;
			} else {
				*p_ecg_run_time = current_time - ECGSTARTTIME;
			}
			break;
		}
	}
        if(is_ecg_name_found == FALSE) {
		cur_ecg_pos = previous_ecg_position;
                return -2;
        }

	if ((strcmp (ECGSTATUS, "ACTIVE") != 0) && (strcmp (ECGSTATUS, "PARTIALLY RUNNING") != 0 )) {
		*p_ecg_run_time = 0;
	}

	cur_ecg_pos = previous_ecg_position;
	return 0;
}



/***
   *	calculates cumulative cycles for all devices matching the device name prefix parameter
   *	return : error status
***/
int get_cumulative_cycles_per_device(long *cumulative_cycle_count, char *device_name_prefix, char *ecg_path_name)
{
	int			ecg_count;
	int			previous_ecg_position		= cur_ecg_pos;
	int			is_ecg_name_found		= FALSE;
	int			is_device_name_found		= FALSE;
	struct dst_entry	*device_position_table		= NULL;
	struct dst_entry	*p_device_position_table	= NULL;
	struct htxshm_HE	*p_htxshm_HE			= NULL;
	int			num_device_entries;
	int			device_count;
	char			ecg_full_name[256];
	int			device_name_search_length;

	*cumulative_cycle_count = 0;


	if( (ecg_path_name[0] == '\0') || (ecg_path_name == NULL) ) {
		get_active_ecg_full_name(ecg_full_name);
		if(ecg_full_name[0] == '\0') {
			return -3;
		}
	} else {
		strcpy(ecg_full_name, ecg_path_name);
	}

	for(ecg_count = 0; ecg_count < num_ecgs; ecg_count++) {
		cur_ecg_pos = ecg_count;
		PUT_FULL_ECG;
		if( (strcmp(full_name, ecg_full_name) ) == 0) {
			is_ecg_name_found = TRUE;
			num_device_entries = init_dst_tbl (&device_position_table);
			p_device_position_table = device_position_table;
			device_name_search_length = strlen(device_name_prefix);
			for (device_count = 0; device_count < num_device_entries; device_count++) {
				p_htxshm_HE = ECGEXER_ADDR (p_device_position_table->shm_pos);
				if (ECGEXER_HDR (p_device_position_table->shm_pos)->started == 0) {
					break;
				}
				if (strncmp(p_htxshm_HE->sdev_id, device_name_prefix, device_name_search_length) == 0) {
					is_device_name_found = TRUE;
					(*cumulative_cycle_count) += p_htxshm_HE->cycles;
					if(p_htxshm_HE->test_id != 0) {
						(*cumulative_cycle_count)++;
					}
				} 
				p_device_position_table++;
			}
			if(device_position_table != NULL) {
				free(device_position_table);
				device_position_table = p_device_position_table = NULL;
			}
			break;
		}

	}
	
        cur_ecg_pos = previous_ecg_position;

	if(is_ecg_name_found == FALSE) {
                return -2;
        }

	if(is_device_name_found == FALSE) {
                return -4;
        }

        return 0;
}



/***
   *	get the latest update from the specified ecg
   *	return : error status
***/
int get_last_update_time(char *last_update_time_string, char *ecg_path_name)
{
	int			previous_ecg_position		= cur_ecg_pos;
	int			is_ecg_name_found		= FALSE;
	int			ecg_count;
	int			device_count;
	int			num_device_entries;
	struct dst_entry	*device_position_table		= NULL;
	struct dst_entry	*p_device_position_table	= NULL;
	struct htxshm_HE	*p_htxshm_HE			= NULL;
	long			last_update_time		= 0;
	char			date_str[32];
	char			day_of_year_str[32];
	char			time_str[32];
	char			ecg_full_name[256];
	char			*label_string			= "Last_update_time";

	if( (ecg_path_name[0] == '\0') || (ecg_path_name == NULL) ) {
		get_active_ecg_full_name(ecg_full_name);
		if(ecg_full_name[0] == '\0') {
			return -3;
		}
	} else {
		strcpy(ecg_full_name, ecg_path_name);
	}
	
	for(ecg_count = 0; ecg_count < num_ecgs; ecg_count++) {
		cur_ecg_pos = ecg_count;
		PUT_FULL_ECG;
		if( (strcmp(full_name, ecg_full_name) ) == 0) {
			is_ecg_name_found	= TRUE;
			num_device_entries	= init_dst_tbl (&device_position_table);
			p_device_position_table	= device_position_table;
			for (device_count = 0; device_count < num_device_entries; device_count++) {
				p_htxshm_HE = ECGEXER_ADDR (p_device_position_table->shm_pos);
				if(p_htxshm_HE->tm_last_upd > last_update_time) {
					last_update_time = p_htxshm_HE->tm_last_upd;
				}
				p_device_position_table++;
			}
			if(device_position_table != NULL) {
				free(device_position_table);
				device_position_table = p_device_position_table = NULL;
			}
			break;
		}
	}
	if(is_ecg_name_found == FALSE) {
		cur_ecg_pos = previous_ecg_position;
		return -2;
	}

	if(last_update_time == 0) {
		sprintf(last_update_time_string,"%s = 00:00:00 00/00/00", label_string);
	} else {
		get_time(last_update_time, date_str, day_of_year_str, time_str);
		sprintf(last_update_time_string, "%s = %s %s", label_string, time_str, date_str);
	}

	cur_ecg_pos = previous_ecg_position;	

	return 0;
}

/****************    END OF test_summary.c     *************************/ 
