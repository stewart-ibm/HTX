/*@(#)45  1.14.3.3  src/htx/usr/lpp/htx/inc/scr_info.h, htx_libhtx, htxfedora 3/28/13 01:29:57*/
#ifndef __HTX_INC_SCR_INFO__
#define __HTX_INC_SCR_INFO__
#include <time.h>

#define STDIN 0
#define MAX_DUP_DEVICES 1024
extern int cur_ecg_pos;

struct  stx_tm {    /* this is defined here to maintain consistency n 32 and
                          64 bit application */
        int     tm_sec;
        int     tm_min;
        int     tm_hour;
        int     tm_mday;
        int     tm_mon;
        int     tm_year;
        int     tm_wday;
        int     tm_yday;
        int     tm_isdst;
};

typedef struct {
  int cmd;
  char msg[80];
} tnotify_msg;

typedef struct {
int err;   /* if err */
int errno_size; /* size or errno */
char msg_text[80];
} tprobe_msg;

  typedef struct{
    ushort cmd;
    ushort subcmd;
    int indx;
    char str[1024];
  } thtx_message;

  typedef struct {
    int start_time;
    char str_start_time[40];
    char error_flag;
    unsigned long long max_cycles_done;
    unsigned long long min_cycles_done;
    int max_entries;
    int num_entries;
  } tcurr_info;

  typedef struct {
    char str_curr_time[40];
    struct stx_tm curr_time;
    int bcast_done;                     /* Required for ssm test summary */
    char system_status[32];             /* Required for ssm test summary */
    char error_dev[DEV_ID_MAX_LENGTH];                 /* Required for ssm test summary */
    int  error_count;                   /* Required for ssm test summary */
    int last_error_time;               /* Required for ssm test summary */
    int last_update_time;              /* Required for ssm test summary */
    int  max_exer_entries;
    int  cur_shm_key;                   /* used by hxstats */
    char test_summary[3024];
  } tsys_hdr;


  typedef struct{
    tsys_hdr *sock_hdr_addr;
    tcurr_info *cur_shm_addr;
  } tmisc_shm;

  typedef struct  {
    char sdev_id[DEV_ID_MAX_LENGTH];
    ushort index;
    char   adapt_desc[12];  /* Adapter Description             */
    char   device_desc[16]; /* Device Description              */
    char   slot_port[16]; /* Device Description              */
    char   status_ah[8];
    char   status_coe[8];
    unsigned long long    cycles;          /* how many times we finished      */
    char   status[3];
    int   tm_last_err;       /* Time of Last Error              */
    int   tm_last_upd;       /* Time of Last Call to htx_update */
    char   err_time[13];    /* Time of Last Error              */
    char   upd_time[13];    /* Time of Last Error              */
    int    test_id;         /* how many times we finished      */
    int    num_errs;        /* Number of Errors */
    unsigned       err_ack : 1;
  } tscn_all;

  typedef struct {
    union shm_pointers ecg_exer_addr;
    int   exer_pos;
    int   parent_ecg_pos;
    int   ecg_sem_key;
    int   ecg_shm_key;
    int   ecg_semhe_id;
    union shm_pointers exer_addr;
    char  dev_name[DEV_ID_MAX_LENGTH];
    int   exer_pid;
  } texer_list;

  typedef struct {
    int   ecg_pos;
	char  ecg_path[40];
    char  ecg_name[50];
    char  ecg_status[20];
    char  ecg_desc[20];
    int   ecg_shmkey;
    int   ecg_semkey;
    int   ecg_shm_id;
    int   ecg_semhe_id;
    union shm_pointers ecg_shm_addr;
    int  ecg_start_time;
    int   ecg_max_entries;
    texer_list *exer_list;
  } tecg_struct;

  typedef struct {
    tcurr_info cur_info;
    tsys_hdr sys_hdr_info;
    union {
      tscn_all *scn_all_info;
    } scn_num;
  } tfull_info;

  typedef struct {
     char sdev_id[DEV_ID_MAX_LENGTH];
	 int  dup_mask[MAX_DUP_DEVICES/32];
  } tdup_struct;


#define INFO_SEND_ALL(x)        info_send.scn_num.scn_all_info[x]
#define INFO_SEND_2(x)          INFO_SEND_ALL(x)
#define INFO_SEND_2_STATUS(x)   INFO_SEND_ALL(x).status_ah
#define INFO_SEND_3_STATUS(x)   INFO_SEND_ALL(x).status_coe
#define INFO_SEND_5(x)          INFO_SEND_ALL(x)

#define ECGNAME          ecg_info[cur_ecg_pos].ecg_name
#define ECGPATH          ecg_info[cur_ecg_pos].ecg_path
#define ECGSTATUS        ecg_info[cur_ecg_pos].ecg_status
#define ECGSHMKEY        ecg_info[cur_ecg_pos].ecg_shmkey
#define ECGSEMKEY        ecg_info[cur_ecg_pos].ecg_semkey
#define ECGSHMID         ecg_info[cur_ecg_pos].ecg_shm_id
#define ECGSEMHEID       ecg_info[cur_ecg_pos].ecg_semhe_id
#define ECGSHMADDR_HDR   ecg_info[cur_ecg_pos].ecg_shm_addr.hdr_addr
#define ECGSHMADDR_HE    ecg_info[cur_ecg_pos].ecg_shm_addr.HE_addr
#define ECGSTARTTIME     ecg_info[cur_ecg_pos].ecg_start_time
#define ECGEXER_ADDR(z)  ecg_info[cur_ecg_pos].exer_list[z].exer_addr.HE_addr
#define ECG_MAX_ENTRIES  ecg_info[cur_ecg_pos].ecg_max_entries
#define ECGEXER_SEMID(z) ecg_info[cur_ecg_pos].exer_list[z].ecg_semhe_id
#define ECGEXER_SHMKEY(z) ecg_info[cur_ecg_pos].exer_list[z].ecg_shm_key
#define ECGEXER_SEMKEY(z) ecg_info[cur_ecg_pos].exer_list[z].ecg_sem_key
#define ECGEXER_POS(z)   ecg_info[cur_ecg_pos].exer_list[z].exer_pos
#define ECGEXER_ECGPOS(z)   ecg_info[cur_ecg_pos].exer_list[z].parent_ecg_pos
#define ECGEXER_HDR(z)   ecg_info[cur_ecg_pos].exer_list[z].ecg_exer_addr.hdr_addr

#define NUM_EXERS      ecg_info[0].ecg_max_entries
#define EXER_LIST      ecg_info[cur_ecg_pos].exer_list
#define EXER_NAME(z)   ecg_info[cur_ecg_pos].exer_list[z].dev_name
#define LIB_ECGEXER_SHMKEY(z) exer_info[z].ecg_shm_key
#define LIB_ECGEXER_SEMKEY(z) exer_info[z].ecg_sem_key
#define LIB_EXER_NAME(z)   exer_info[z].dev_name
#define LIB_EXER_PID(z)    exer_info[z].exer_pid
#define EXER_PID(z)    ecg_info[cur_ecg_pos].exer_list[z].exer_pid

#define PUT_FULL_ECG   sprintf(full_name,"%s/%s",ECGPATH,ECGNAME)


#define  cpy_tm(q,z)  \
            (q).tm_sec = (z).tm_sec; \
            (q).tm_min = (z).tm_min; \
            (q).tm_hour = (z).tm_hour; \
            (q).tm_mday = (z).tm_mday; \
            (q).tm_mon = (z).tm_mon;  \
            (q).tm_year = (z).tm_year; \
            (q).tm_wday = (z).tm_wday; \
            (q).tm_yday = (z).tm_yday; \
            (q).tm_isdst = (z).tm_isdst; \



#endif  /* __HTX_INC_SCR_INFO__ */
