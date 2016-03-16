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
#include "cfgclibdef.h"
#include <sys/resource.h>

char              ERR_WRAP[8];       /* htxerr wrap keyword               */
char              HTXPATH[50];       /* HTX file system path spec.        */
char              HTXSCREENS[50];       /* HTX file system path spec.        */
char              MAX_ERR_SAVE[16];  /* htxerr.save max file size         */
char              MAX_ERR_THRES[16]; /* error log file wrap threshold.    */
char              MAX_MSG_SAVE[16];  /* htxmsg.save max file size         */
char              MAX_MSG_THRES[16]; /* message log file wrap threshold.  */
char		  MSG_ARCHIVE[8];    /* message archive mode              */
char              MSG_WRAP[8];       /* htxmsg wrap keyword               */
char              level_str[80];     /* HTX and AIX levels */
char              obuf[BUFSIZ];      /* buffer for stdout                 */
char              *program_name;     /* the name of this program (argv[0]) */
char              save_dir[PATH_MAX + 1]; /* original current directory   */
char              stress_cycle[32];  /* # of seconds between "heartbeats" */
char              stress_dev[64];    /* "heartbeat" device for stress test */
int               alarm_sig = FALSE; /* set to TRUE on receipt of SIGALRM */
int               editor_PID;            /* editor process id             */
int               HANG_MON_PERIOD = 0; /* Hang monitor period (0=no mon)  */
int               hft_flag = FALSE;  /* set to TRUE if run on hft display */
int               MAX_ADDED_DEVICES = 0; /* set from htx_profile          */
int               msgqid = -1;       /* Message Log message queue id      */
int               profile_emc_mode;       /* for build_ipc()                  */
int               semhe_id = -1;     /* HE semaphore id                   */
int               semot_id = -1;     /* Order Table semaphore id          */
int               shm_id = -1;       /* system shared memory id           */
int               rem_shm_id = -1;
int               shutd_wait = SD_DEFAULT; /* time from SIGTERM to SIGKILL */
int               slow_shutd_wait = SLOW_SD_DEFAULT; /* time from SIGTERM to SIGKILL */
int               system_call = FALSE; /* set to TRUE before any system() */
int               sig_end_flag = 0;
int               sig_handle_done=0;
int               system_started=0; /* flag that gets set when any ecg in the system first gets started */
int               equaliser_flag = 0;
pid_t             equaliser_PID = 0;   /* equaliser PID  */
pid_t             hang_mon_PID = 0;  /* hang monitor PID                  */
pid_t             hxsdst_PID = 0;    /* hxsdst PID                        */
pid_t             hxsmsg_PID = 0;    /* message handler process id        */
pid_t             hxstats_PID = 0;   /* statistics program process id     */
pid_t             shell_PID;

struct tm         start_time;        /* System startup time.            */

unsigned int      max_wait_tm = 0;   /* maximum semop wait time         */

tmisc_shm *rem_shm_addr;
int start_msg_done = 0;
char msg_type[10];
char rsrc_type[20][20],rsrc_name[40][20];
char WAS_ip[80][80];
int num_hmcs=0, num_WAS=0;
union shm_pointers shm_addr;         /* shared memory union pointers    */

char sep_at,sep_nl;
char str_rcv[1024], bcast_msg[20];
char daemon_start_time[20],daemon_upd_time[20];
char dt[20],dy[4],tmm[20];
char ecg_list[1024][20];

int shutdown_gui = FALSE, shutdown_flag = FALSE, shutdown_ecg = FALSE;
int is_cmdline, running_status = 99;
int ipc_done=0,bcast_done=FALSE, num_ecgs;
//volatile int cur_ecg_pos;
int autostart;
char start_ecg_name[40] = "";
int MsgQueCreated = FALSE;
int DevToSlotInitialized = FALSE;
int MAX_EXER_ENTRIES;
int MAX_ECG_ENTRIES;
int DAEMON_LOG_WRAP;

thtx_message msg_rcv;
tecg_struct *ecg_info;
tdup_struct *dup_info;
tfull_info  info_send;
tprobe_msg probe_send;
tnotify_msg notify_send;

int num_ecg, num_run, num_stop, num_actv, num_suspend, num_coe, num_soe, num_shtd, num_stts, num_f_stat, num_f_err, num_f_sum, num_rstrt, num_term, num_vpd,num_sysdata, num_mda, num_cbl, num_query, num_none, num_addexer, num_add_ah, num_add_coe, num_add_force, num_add_dup, num_dblevel, num_get_actecg, num_nonblk, num_failstatus, num_runtime, num_devcycles, num_last_update_time; /* sampan */

char active_ecg_name[50]="", *ecg[80], *run, *stop, *actv[80], *suspend[80], *coe[80], *soe[80], *shtd, *stts[80], *f_stat, *f_err, *f_sum, *rstrt[80], *term[80], *f_vpd,*f_sysdata, *f_mda, *f_cbl,  *query[500], *none, *add_exer[80], *add_ah, *add_coe, *add_force, *add_dup, *dblevel, *get_actecg, *nonblk, *devcycles[80]; /* sampan */
char *full_list[512], lst[80][80];
char full_name[56];
char path_for_ecgmdt[200];
char * malloc_array[80];
int malloc_index=0;
int listener=0, fd_num=0;

#if defined(__HTX_LINUX__) || defined(__OS400__)
  union semun semctl_arg;
  char         *__loc1;
#endif

/* Debug/Trace facility */
/*int dbflags = 0xFFFFFFFF;*/
int dbflags = 0x0;
int dbcounter = 0;

int LEVEL = 0;
