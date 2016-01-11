/*@(#)50	1.6  src/htx/usr/lpp/htx/bin/stxclient/scr_info.h, eserv_gui, htxubuntu 8/21/03 17:11:29*/
#ifndef SCR_INFO
#define SCR_INFO
#include <time.h>

#define STDIN 0

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
    char str[80];
  } thtx_message;

  typedef struct {
    /*struct tm start_time;*/
    long start_time;
    char str_start_time[40];
    char error_flag;
    int max_cycles_done;
    int min_cycles_done;
    int max_entries;
    int num_entries;
  } tcurr_info;

  typedef struct {
    char running_halted[30];                /* started flag of htx_shm_hdr?? */
    char str_curr_time[40];
    struct tm curr_time;
    int bcast_done;                     /* Required for ssm test summary */
    char system_status[32];             /* Required for ssm test summary */
    char error_dev[32];                 /* Required for ssm test summary */
    int  error_count;                   /* Required for ssm test summary */
    long last_error_time;               /* Required for ssm test summary */
    long last_update_time;              /* Required for ssm test summary */
	int  cur_shm_key;                   /* used by hxstats */
	char test_summary[1024];
  } tsys_hdr;


  typedef struct {
    tsys_hdr sys_hdr_info;
    int rc;
    char ecg_name[20];
    char level_str[80];
    char err_msg[80];
  } tscn_1;

  typedef struct{
    tsys_hdr *sock_hdr_addr;
    tcurr_info *cur_shm_addr;
  } tmisc_shm; 

  typedef struct {
    int  exer_num;
    int  exer_cur_shmkey;
    int  exer_cur_semkey;
    char exer_cur_ecg_name[40];
    char exer_id[100][40];
    int  exer_isloaded[100];
    int  exer_pos[100];
    int  exer_shmkey[100];
    int  exer_semkey[100];
    char exer_in_ecg[100][40];
  } texer_ipc;

  typedef struct  {
    char sdev_id[32];
    ushort index;
    char   adapt_desc[12];  /* Adapter Description             */
    char   device_desc[16]; /* Device Description              */
    char   slot_port[16]; /* Device Description              */
    char    status[8];
  } tscn_2_4;

  typedef struct {
    char sdev_id[32];
    char status[3];
    long           tm_last_err;       /* Time of Last Error              */
    long           tm_last_upd;       /* Time of Last Call to htx_update */
    char err_time[13];       /* Time of Last Error              */
    char upd_time[13];       /* Time of Last Error              */
    int  cycles;            /* how many times we finished      */
    int  test_id;            /* how many times we finished      */
    unsigned       err_ack : 1;
  } tscn_5 ;

  typedef struct  {
    char sdev_id[32];
    ushort index;
    char   adapt_desc[12];  /* Adapter Description             */
    char   device_desc[16]; /* Device Description              */
    char   slot_port[16]; /* Device Description              */
    char   status_ah[8];
    char   status_coe[8];
    char   status[3];
    long   tm_last_err;       /* Time of Last Error              */
    long   tm_last_upd;       /* Time of Last Call to htx_update */
    char   err_time[13];    /* Time of Last Error              */
    char   upd_time[13];    /* Time of Last Error              */
    int    cycles;          /* how many times we finished      */
    int    test_id;         /* how many times we finished      */
    unsigned       err_ack : 1;
  } tscn_all;

  typedef struct {
    union shm_pointers ecg_exer_addr;
    int   exer_pos;
	int   ecg_sem_key;
	int   ecg_shm_key;
    int   ecg_semhe_id;
  	union shm_pointers exer_addr;
	char  dev_name[40];
  } texer_list;
  typedef struct {
    int   ecg_pos;
    char  ecg_abs_name[56];
    char  ecg_status[10];
    char  ecg_desc[20];
    int   ecg_shmkey;
    int   ecg_semkey;
    int   ecg_shm_id;
    int   ecg_semhe_id;
    union shm_pointers ecg_shm_addr;
    long  ecg_start_time;
	int   ecg_max_entries;
	texer_list exer_list[1024]; 
  } tecg_struct;

  typedef struct {
    tcurr_info cur_info;
    tsys_hdr sys_hdr_info;
    union {
      tecg_struct scn_0_info[1024];
      tscn_5 scn_5_info[1024];
      tscn_2_4 scn_2_4_info[1024];
      tscn_all scn_all_info[1024];
    } scn_num;
  } tfull_info;



#define ECGNAME          ecg_info[cur_ecg_pos].ecg_abs_name
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
#define ECGEXER_HDR(z)   ecg_info[cur_ecg_pos].exer_list[z].ecg_exer_addr.hdr_addr

#define NUM_EXERS      ecg_info[0].ecg_max_entries
#define EXER_NAME(z)   ecg_info[cur_ecg_pos].exer_list[z].dev_name

#endif
