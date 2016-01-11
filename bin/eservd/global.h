/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* htxltsbml src/htx/usr/lpp/htx/bin/eservd/global.h 1.21.4.5             */
/*                                                                        */
/* Licensed Materials - Property of IBM                                   */
/*                                                                        */
/* Restricted Materials of IBM                                            */
/*                                                                        */
/* COPYRIGHT International Business Machines Corp. 2003,2012              */
/* All Rights Reserved                                                    */
/*                                                                        */
/* US Government Users Restricted Rights - Use, duplication or            */
/* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.      */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

/* @(#)03	1.21.4.6  src/htx/usr/lpp/htx/bin/eservd/global.h, eserv_daemon, htxubuntu 12/11/12 06:08:50 */

#ifndef GLOBAL_H
#define GLOBAL_H
extern char              ERR_WRAP[8];       /* htxerr wrap keyword               */
extern char              HTXPATH[50];       /* HTX file system path spec.        */
extern char              HTXSCREENS[50];       /* HTX file system path spec.        */
extern char              MAX_ERR_SAVE[16];  /* htxerr.save max file size         */
extern char              MAX_ERR_THRES[16]; /* error log file wrap threshold.    */
extern char              MAX_MSG_SAVE[16];  /* htxmsg.save max file size         */
extern char              MAX_MSG_THRES[16]; /* message log file wrap threshold.  */
extern char		 MSG_ARCHIVE[8];    /* message archive mode              */
extern char              MSG_WRAP[8];       /* htxmsg wrap keyword               */
extern char              level_str[80];     /* HTX and AIX levels */
extern char              obuf[BUFSIZ];      /* buffer for stdout                 */
extern char              *program_name;     /* the name of this program (argv[0]) */
extern char              save_dir[PATH_MAX + 1]; /* original current directory   */
extern char              stress_cycle[32];  /* # of seconds between "heartbeats" */
extern char              stress_dev[64];    /* "heartbeat" device for stress test */
extern int               alarm_sig; /* set to TRUE on receipt of SIGALRM */
extern int               editor_PID;            /* editor process id             */
extern int               HANG_MON_PERIOD; /* Hang monitor period (0=no mon)  */
extern int               hft_flag;  /* set to TRUE if run on hft display */
extern int               MAX_ADDED_DEVICES; /* set from htx_profile          */
extern int               msgqid;       /* Message Log message queue id      */
extern int               profile_emc_mode;       /* for build_ipc()                  */
extern int               semhe_id;     /* HE semaphore id                   */
extern int               semot_id;     /* Order Table semaphore id          */
extern int               shm_id;       /* system shared memory id           */
extern int               rem_shm_id;
extern int               shutd_wait; /* time from SIGTERM to SIGKILL */
extern int               slow_shutd_wait; /* time from SIGTERM to SIGKILL */
extern int               system_call; /* set to TRUE before any system() */
extern int               sig_end_flag;
extern int               sig_handle_done;
extern int               system_started;    /*Flag set when the any ecg first gets started*/
extern int               equaliser_flag;    /* Flag to start equaliser process  */

extern pid_t             hang_mon_PID;  /* hang monitor PID                  */
extern pid_t             equaliser_PID;  /* equaliser PID                  */
extern pid_t             hxsdst_PID;    /* hxsdst PID                        */
extern pid_t             hxsmsg_PID;    /* message handler process id        */
extern pid_t             hxstats_PID;   /* statistics program process id     */
extern pid_t             shell_PID;

extern struct tm         start_time;        /* System startup time.            */

extern unsigned int      max_wait_tm;   /* maximum semop wait time         */

extern tmisc_shm *rem_shm_addr;
extern int start_msg_done;
extern char msg_type[10];
extern int listener;
extern char rsrc_type[20][20],rsrc_name[40][20];
extern char WAS_ip[80][80];
extern int num_hmcs, num_WAS;

extern union shm_pointers shm_addr;         /* shared memory union pointers    */

extern char sep_at,sep_nl;
extern char str_rcv[1024], bcast_msg[20];
extern char daemon_start_time[20],daemon_upd_time[20];
extern char dt[20],dy[4],tmm[20];
extern char ecg_list[1024][20];

extern int shutdown_gui, shutdown_flag, shutdown_ecg;
extern int is_cmdline, running_status;
extern int ipc_done,bcast_done, num_ecgs;
extern int cur_ecg_pos;
extern int autostart;
extern char start_ecg_name[];
extern int MsgQueCreated;
extern int DevToSlotInitialized;
extern int MAX_EXER_ENTRIES;
extern int MAX_ECG_ENTRIES;
extern int DAEMON_LOG_WRAP;



/* ascii code definitions for characters sent in msg_rcv.subcmd */
#define SUBCMD_A 0x41
#define SUBCMD_B 0x42
#define SUBCMD_C 0x43
#define SUBCMD_F 0x46
#define SUBCMD_Q 0x51
#define SUBCMD_R 0x52
#define SUBCMD_S 0x53
#define SUBCMD_W 0x57

#define SUBCMD_a 0x61
#define SUBCMD_b 0x62
#define SUBCMD_c 0x63
#define SUBCMD_f 0x66
#define SUBCMD_q 0x71
#define SUBCMD_r 0x72
#define SUBCMD_s 0x73
#define SUBCMD_w 0x77

extern thtx_message msg_rcv;
extern tecg_struct *ecg_info;
extern tdup_struct *dup_info;
extern tfull_info  info_send;
extern tprobe_msg probe_send;
extern tnotify_msg notify_send;
//extern texer_ipc *exer_ipcaddr;

extern int num_ecg, num_run, num_stop, num_actv, num_suspend, num_coe, num_soe, num_shtd, num_stts, num_f_stat, num_f_err, num_f_sum, num_rstrt, num_term, num_vpd, num_sysdata, num_mda, num_cbl, num_query, num_none, num_addexer, num_add_ah, num_add_coe, num_add_force, num_add_dup, num_dblevel, num_get_actecg, num_nonblk, num_failstatus, num_runtime, num_devcycles, num_last_update_time; /*sampan */

extern char active_ecg_name[50], *ecg[80], *run, *stop, *actv[80], *suspend[80], *coe[80], *soe[80], *shtd, *stts[80], *f_stat, *f_err, *f_sum, *rstrt[80], *term[80], *f_vpd, *f_sysdata, *f_mda, *f_cbl, *query[500], *none, *add_exer[80], *add_ah, *add_coe, *add_force, *add_dup, *dblevel, *get_actecg, *nonblk, *devcycles[80]; /* sampan */
extern char path_for_ecgmdt[200];
extern char *full_list[512], lst[80][80];
extern char full_name[];

extern char * malloc_array[];
extern int malloc_index;
extern int listener, fd_num;

extern char * stx_malloc(int size );
extern void stx_free(void);
#if defined(__HTX_LINUX__) || defined(__OS400__)
  extern union semun semctl_arg;
  extern char         *__loc1;
#endif

#if defined(__OS400__)
#define bzero(dst,len) memset(dst,0,len) /* 400 */
#define bcopy(src, dst, len) memcpy(dst, src, len) /* 400 */
char * tempnam(char *, char *);
#endif

/* Debug/Trace facility */
#define DBENTRY	0x00000001
#define DBEXIT	0x00000002
#define DBTRACE(flag,printparms) { \
  if ((flag)&dbflags) { \
    print_log(LOGDB1,"DB:%04x:%08x:",++dbcounter,((flag)&dbflags)); \
    print_log(LOGDB1,printparms); fflush(stdout); \
    } \
  }
extern int dbflags;
extern int dbcounter;


#define LOGERR -1
#define LOGMSG 0
#define LOGDB1 1
#define LOGDB2 2

void print_log (int a, const char *fmt, ...) ;


extern int LEVEL;
#endif
