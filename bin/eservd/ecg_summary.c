
/* @(#)99	1.24.4.1  src/htx/usr/lpp/htx/bin/eservd/ecg_summary.c, eserv_daemon, htxubuntu 12/16/14 03:58:51 */

#include "eservd.h"
#include "global.h"

#if defined(__OS400__)   /* 400 */
#include "popen.h"
#endif

extern union semun semctl_arg;
int get_unloaded_entries(char *);
extern tmisc_shm *rem_shm_addr;

int
ecg_summary (char *sum_data, int for_ssm)
{
   int num_entries = 0;         /* local number of shm HE entries    */
   int num, cnt, dont_cmp = 0;
   long clock;
   int dev_in = 0, dev_dd = 0, dev_tm = 0, dev_st = 0, dev_er = 0, dev_cp =
      0, dev_rn = 0, dev_hg = 0, dev_pr = 0, dev_total = 0;
   int ssm_dev_in = 0, ssm_dev_dd = 0, ssm_dev_tm = 0, ssm_dev_st =
      0, ssm_dev_er = 0, ssm_dev_cp = 0, ssm_dev_rn = 0, ssm_dev_hg =
      0, ssm_dev_pr = 0, ssm_dev_total = 0;
   int ecg_ina = 0, ecg_act = 0, ecg_par = 0, ecg_sus = 0, ecg_unl = 0;
   int prev_ecg_pos;
   struct dst_entry *q_table = NULL;    /* points to begin ahd seq tbl */
   struct dst_entry *p_into_table = NULL;       /* points into ahd sequence table */
   struct htxshm_HE *p_htxshm_HE = NULL;        /* pointer to a htxshm_HE struct */
   struct semid_ds sembuffer;   /* semaphore buffer */
   int found_ecg = 0, found_good = 0, tmp;
   char temp_stat[512];
   DBTRACE(DBENTRY,("enter ecg_summary.c ecg_summary\n"));

   //memset(rem_shm_addr->sock_hdr_addr->test_summary,0,3024);

   if (do_init_rem_ipc(sum_data) <0) {
      DBTRACE(DBEXIT,("return/a ecg_summary.c ecg_summary\n"));
      return;
   }

   strcpy (sum_data, " ");

   dont_cmp = 0;
   prev_ecg_pos = cur_ecg_pos;
   for (tmp = 0; tmp < num_ecg; tmp++) {
      if (strcmp (ecg[tmp], "/ecg.all") == 0) {
         dont_cmp = 1;
         num_ecg = num_ecgs-1;
	      for (cnt = 1; cnt < num_ecgs; cnt++) {
             cur_ecg_pos = cnt;
             if (strcmp (ECGSTATUS, "ACTIVE") == 0)
                ecg_act++;
             else if (strcmp (ECGSTATUS, "SUSPENDED") == 0)
                ecg_sus++;
             else if (strcmp (ECGSTATUS, "INACTIVE") == 0)
                ecg_ina++;
             else if (strcmp (ECGSTATUS, "UNLOADED") == 0)
                ecg_unl++;
             else
                ecg_par++;
         }

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
         break;
      }
   }

   print_log(LOGMSG,"ECG_SUMMARY. num_ecg = %d\n", num_ecg);
   //sprintf(rem_shm_addr->sock_hdr_addr->test_summary," ");

  /************* MAIN LOOP **********************/
   ssm_dev_in = ssm_dev_dd = ssm_dev_tm = ssm_dev_st = ssm_dev_er =
      ssm_dev_cp = ssm_dev_rn = ssm_dev_hg = ssm_dev_pr = ssm_dev_total = 0;
   for (tmp = 0; tmp < num_ecg; tmp++) {
      found_ecg = 0;
      found_good = 0;
      dev_in = dev_dd = dev_tm = dev_st = dev_er = dev_cp = dev_rn = dev_hg =
         dev_pr = dev_total = 0;

      /*******local for loop **********/
      for (cnt = 1; cnt < num_ecgs; cnt++) {
         cur_ecg_pos = cnt;

         PUT_FULL_ECG;
         if (dont_cmp) {
            found_ecg = 1;
            cur_ecg_pos =
               ((tmp + 1) > (num_ecgs - 1)) ? (num_ecgs - 1) : (tmp + 1);
            /*print_log(LOGMSG,"Not comparing for ecg. cur_pos = %d ECGNAME=:%s/%s: ecg=:%s:\n",
                cur_ecg_pos, ECGPATH, ECGNAME, ecg[tmp]);*/
            fflush (stdout);
            if (strcmp (ECGSTATUS, "UNLOADED") == 0) {
               break;
            }
            else {
               found_good = 1;
               break;
            }
         }
         else if ((strcmp (full_name, ecg[tmp]) == 0)) {
            print_log(LOGMSG,"Comparing for ecg. ECGNAME=:%s: ecg=:%s:\n", full_name,
                    ecg[tmp]);
            fflush (stdout);
            found_ecg = 1;
            if (strcmp (ECGSTATUS, "UNLOADED") == 0) {
               break;
            }
            else {
               found_good = 1;
               break;
            }
         }
      }
      /*******local for loop **********/

      if (!found_ecg) {
         sprintf (temp_stat, "ECG %s Not Found\n", ecg[tmp]);
         strcat (sum_data, temp_stat);
         //strcat(rem_shm_addr->sock_hdr_addr->test_summary, temp_stat);
         continue;
      }

      if (!found_good) {
         dev_total = get_unloaded_entries(sum_data);
         if (dev_total <0) {
            DBTRACE(DBEXIT,("return/b ecg_summary.c ecg_summary\n"));
            return;
            //continue;
         }
      }
      else {

      if (ECGSHMADDR_HDR->max_entries == 0) {
         if (q_table != NULL) {
            free ((char *) q_table);
            q_table = NULL;
         }
         //continue;
      }
      num_entries = init_dst_tbl (&q_table);
      /*print_log(LOGMSG,"ecg_summary: cur_ecg_pos = %d: num_entries = %d.\n",
              cur_ecg_pos, num_entries);
      fflush (stdout);*/

      p_into_table = q_table;
      for (num = 0; num < num_entries; num++) {
         //print_log(LOGMSG,"ecg_summary: shm_pos = %d\n", p_into_table->shm_pos);
         p_htxshm_HE = ECGEXER_ADDR (p_into_table->shm_pos);
         //print_log(LOGMSG,"name = %s\n", p_htxshm_HE->sdev_id);
         fflush (stdout);
         clock = time ((long *) 0);

         if (ECGEXER_HDR (p_into_table->shm_pos)->started == 0) {
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
                ||
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
            //strcpy(ECGSTATUS, "PARTIALLY RUNNING");
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
            if (semctl
                (ECGEXER_SEMID (p_into_table->shm_pos),
                 ((ECGEXER_POS (p_into_table->shm_pos) * SEM_PER_EXER) + SEM_GLOBAL + 1), GETVAL,
                 &sembuffer) == 0)
            dev_pr++;
            //strcpy(ECGSTATUS, "PARTIALLY RUNNING");
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

         if (p_htxshm_HE->no_of_errs > 0) {
            if (strcmp(ECGSTATUS, "ACTIVE")==0) {
               strcpy(ECGSTATUS, "PARTIALLY RUNNING");
            }
            strcpy(rem_shm_addr->sock_hdr_addr->system_status, "ERROR");
         }
         p_into_table++;
      }
      dev_total =
         dev_rn + dev_st + dev_er + dev_tm + dev_dd + dev_hg + dev_cp +
         dev_in;
      }

      print_log(LOGMSG,"Doing a loop \n");
      fflush (stdout);
      if (!for_ssm) {
         sprintf (temp_stat,
                  "%s/%s is in %s state\n"
                  "Total Devices       = %d\n"
                  " Running            = %d\n"
                  " Suspended          = %d\n"
                  " Error              = %d\n"
                  " Partialy Running   = %d\n"
                  " Terminated         = %d\n"
                  " Died               = %d\n"
                  " Hung               = %d\n"
                  " Completed          = %d\n"
                  " Inactive           = %d\n", ECGPATH, ECGNAME, ECGSTATUS,
                  dev_total, dev_rn, dev_st, dev_er, dev_pr, dev_tm, dev_dd,
                  dev_hg, dev_cp, dev_in);
         strcat (sum_data, temp_stat);
      }
      else {
         ssm_dev_total = ssm_dev_total + dev_total;
         ssm_dev_rn = ssm_dev_rn + dev_rn;
         ssm_dev_er = ssm_dev_er + dev_er;
         ssm_dev_pr = ssm_dev_pr + dev_pr;

      }
      //strcat(rem_shm_addr->sock_hdr_addr->test_summary, temp_stat);
      print_log(LOGMSG,"Done a loop \n");
      fflush (stdout);
      if (q_table != NULL) {
         free (q_table);
         q_table = NULL;
      }
   }
   if (for_ssm) {
      sprintf (temp_stat,
               " @%d@%d@%d@%d@%d",
               ssm_dev_total, ssm_dev_rn, ssm_dev_er, ssm_dev_pr,
               (ssm_dev_total - ssm_dev_rn - ssm_dev_er - ssm_dev_pr));
      strcat (sum_data, temp_stat);
   }
   cur_ecg_pos = prev_ecg_pos;
   DBTRACE(DBEXIT,("return/c ecg_summary.c ecg_summary\n"));
   return 0;
}


int get_unloaded_entries(char *res_msg)
{
char str[80], buf[80];
FILE *pipe_id;
int entries;
DBTRACE(DBENTRY,("enter ecg_summary.c get_unloaded_entries\n"));

   sprintf(str, "grep : %s/%s|wc -l",ECGPATH,ECGNAME);
   print_log(LOGMSG,"str = %s\n", str); fflush(stdout);

   pipe_id = popen(str, "r");
   if (pipe_id == NULL) {
      print_log(LOGERR,"Error opening pipe for get_unloaded_entries. errno = %d (%s).\n",errno, strerror(errno));
      sprintf(res_msg, "Error opening pipe for get_unloaded_entries. errno = %d (%s).\n",errno, strerror(errno));
      DBTRACE(DBEXIT,("return/a -1 ecg_summary.c get_unloaded_entries\n"));
      return -1;
   }
   print_log(LOGMSG,"piped = 0x%x\n", pipe_id); fflush(stdout);

   if (fgets(buf,80,pipe_id) == NULL) {
        print_log(LOGERR,"Error getting ps detailes from fgets\n");
        sprintf(res_msg, "Error getting get_unloaded_entries. Error in fgets. errno = %d (%s)\n",errno, strerror(errno));
        pclose(pipe_id);
        DBTRACE(DBEXIT,("return/b -1 ecg_summary.c get_unloaded_entries\n"));
        return -1;
   }      /* endif */
   print_log(LOGMSG,"buf = %s\n", buf); fflush(stdout);
   entries = atoi(buf) - 1;
   if (entries <0)
      entries = 0;
   pclose(pipe_id);
   DBTRACE(DBEXIT,("return ecg_summary.c get_unloaded_entries\n"));
   return entries;
}
