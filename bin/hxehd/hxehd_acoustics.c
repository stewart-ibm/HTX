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
/* @(#)09       1.13  src/htx/usr/lpp/htx/bin/hxehd/hxehd_acoustics.c, exer_hd, htxubuntu 8/7/13 02:06:00 */
/******************************************************************************
 
 * CHANGE LOG: programmer  | date     | change
 *            -------------|----------|---------------------------------------
 *             D. Stauffer | 10/01/96 | Added code to be able to submit a
 *                         |          | command asif from the command line.
 *             D. Stauffer | 11/04/96 | Added new keyword ( PAGE_ALIGN ) to
 *                         |          | to be able to align malloced buffers
 *                         |          | on a page boundry.
 *             D. Stauffer | 03/28/97 | Deleted get_rule from HXEHD and moved
 *                         |          | it into its own module. Also, instead
 *                         |          | of reading a stanza and processing it,
 *                         |          | it now reads the entire file and puts
 *                         |          | stanzas into memory.
 *             D. Stauffer | 04/01/97 | Created informational message to be
 *                         |          | put out at beginning of program about
 *                         |          | the drive or logical volume.
 *             D. Stauffer | 04/01/97 | Changed the memset of the buffers from
 *                         |          | all 0's to 0xbb.
 *             D. Stauffer | 04/10/97 | Changed the timed variable from a global
 *                         |          | to a var within the hxehd.h dcl
 *             L. Record   | 04/18/97 | Changed the interface of bldbuf().
 *             D. Stauffer | 08/20/97 | Added new function to exerciser.
 *                         |          | Function will start at the beginning of
 *                         |          | the exerciser and look for threads that
 *                         |          | are hung. When found, it will put out an
 *                         |          | informational message to the message log
 *             D. Stauffer | 01/09/98 | See proc_rule.c for changes - chg 1 &
 *                         |          | chg 2.
 *             D. Stauffer | 02/10/99 | Added a variable hostname to be used in
 *                         |          | building write/read buffers headers.
 *             D. Stauffer | 07/07/99 | Added a three new global variables that
 *                         |          | a user is allowed to set (misc_run_cmd,
 *                         |          | run_on_misc, run_reread).
 *             D. Stauffer | 08/09/99 | Check to make sure background thread has
 *                         |          | not errored out before running fore-
 *                         |          | ground threads.
 *             D. Stauffer | 09/16/99 | Allow user to set maximum block number
 *                         |          | on the background thread.
 *             R. Gebhardt | 07/28/00 | Check to determine if the test disk (or
 *                         |          | logical volume) is a member of a
 *                         |          | group which is known (defined) in the
 *                         |          | in the ODM.
 *             R. Gebhardt | 09/11/00 | Reset lba_fencepost = 0 on subsequent
 *                         |          | exections of BWRC.  This closes a window
 *                         |          | if the thread is delayed in open() and
 *                         |          | causes false "stale data" detection.
 *             R. Gebhardt | 09/11/00 | Change attributes on p_thread_create() so
 *                         |          | that the threads are created in detached
 *                         |          | state to work around a problem in
 *                         |          | AIX 500.
 *             R. Gebhardt | 09/28/00 | Add logic to handle the DVD-RAM drive
 *                         |          | which is a CDROM type in devinfo.
 *             R. Gebhardt | 10/18/00 | Changed stanza start message, tot_blks
 *                         |          | for whole disk was not set yet,
 *                         |          | made it total blocks for stanza.
 *             R. Gebhardt | 12/22/00 | Put the threads to sleep if BSEQWRC
 *                         |          | has not had a chance to init any
 *                         |          | segments of the disk.  This prevents
 *                         |          | a problem where stanzas are skipped
 *                         |          | and fail to save seeds which are needed
 *                         |          | later by stanzas which restore the.
 *                         |          | See defect 324329.
 *             sjayakum    | 10/08/01 | Modification for 64 bit compilation
 *                         |          |
 *             Nitin Gupta | 23/03/05 | Fix the logic for small partitions
 *                         |          | D# 494699 (for details)
 *			   Piyush      | 23/10/12 | Support for 64 Bit LBA number
 ****************************************************************************/
#include <pthread.h>
#include <stdio.h>
#include <strings.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <sys/devinfo.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/scsi.h>
#include <sys/scdisk.h>
#include <cf.h>
#include <sys/cfgodm.h>
#include <odmi.h>
#include <unistd.h>
#include <sys/mdio.h>
#include <sys/cfgdb.h>
#include <lvm.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include "hxehd.h"
#ifdef __64BIT__
#include "hxihtx64.h"
#else
#include "hxihtx.h"
#endif
#include "hxehd_proto.h"
/* changes for accoustics */
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

/* pthread_mutexattr_default extern variable not defined in 64 bit library */
#ifdef __64BIT__
#define DEFAULT_MUTEX_ATTR_PTR        NULL
#define DEFAULT_COND_ATTR_PTR   NULL
#else
#define DEFAULT_MUTEX_ATTR_PTR        (&pthread_mutexattr_default)
#define DEFAULT_COND_ATTR_PTR   (&pthread_condattr_default)
#endif

#define ODMERR odmerrno, errmsg
#define M_NAME_LEN 6
#define MAX_BUFFERS 100

void sig_function(int, int, struct sigcontext *);
void seg_lock();
void seg_unlock();
void run_timer();

unsigned long long   lun_id, scsi_id; 
int          rule_stanza[MAX_STANZA], saved_num_blocks;
unsigned int eeh_enabled;
unsigned int eeh_retries;

#ifndef __HTX43X__
int		scsi_id_parent, lun_id_parent;
int		pass_thru = 0, debug_passthru = 0;
char        disk_parent[20];
#endif

int             crash_on_anyerr = 0, crash_on_miscom = 0, la_on_miscom = 0;
int		        turn_attention_on = 0;
int			    is_bwrc_rule_present = 0;
int             saved_data_len, seeds_were_saved = 0,  bufrem = 0;
int             count_active_cpus, arch, hang_time = 600, threshold = 3,accous_rules = 0;
int             num_of_processors_allowed, cpu_bind = 0, cpustate = 0, tmp_cpustate = 0;
static int      read_rules_file_count;
static int      shift_var = 0xffffffff;
int volatile    cache_cond, threads_created;
int volatile    number_of_threads, percent;
extern int volatile  collisions;
extern unsigned long long volatile  lba_fencepost;
static long long volatile saved_maxlba;
char            msg[MSG_TEXT_SIZE], msg1[MSG_TEXT_SIZE], saved_last_type_len;
char            device_name[10], msg_swtch[4] = "NO", crash_on_hang = 'N';
char            misc_run_cmd[100], run_on_misc = 'N', run_reread = 'Y';
char            pipe_name[15], default_patternid[9] = "#003", signal_flag = 'N', exit_flag = 'N';
char            *getenv(), hostname[10], rules_file_name[100];
char            machine_name[M_NAME_LEN];/* holds machine ID from uname() rtn */
char volatile   backgrnd_thread = 'F';
FILE            *tptr;
time_t          time_mark;
struct          CuDv cusobj;
struct          CuAt *catt;
struct          devinfo info, parent_info;
struct          ruleinfo *first_ruleptr, *prev_ruleptr;
static struct   random_seed_t saved_seed;
static struct   random_seed_t saved_data_seed;
pthread_t       threads[MAX_THREADS];
pthread_attr_t  thread_attrs_detached;    /* threads created detached */
pthread_cond_t  create_thread_cond_var, do_oper_cond_var, segment_do_oper;
pthread_mutex_t arg_mutex, cache_mutex, segment_mutex;

char		lv_devices[10][10]; /* for LA trigger */
char		PCI_parent[10][10];
int		PCI_devfn[10];
int		open_retry_count=0,stats_fd,semcount,semid,semkey,proc_stage=0;
struct htx_data  *datap;

/***************************************************************************
** Function to set the signal flag to yes when a signal 30 is received by
** the exerciser. Used with PIPES.
****************************************************************************/
void sig_function(int sig, int code, struct sigcontext *scp)
{
  signal_flag = 'Y';
}

/* changes for accoustics */
void signal_handler (int sig_type) {

  struct sembuf semops_dec_sem3[1] = {2,-1,0};
  struct sembuf semops_dec[1] = {0,-1,0};
  struct sembuf semops_dec_sem2[1] = {1,-1,0};
  unsigned short sem_val;

  if(accous_rules)
  { 
    semop(semid,&semops_dec_sem3[0],1);
    sem_val = semctl(semid, 2, GETVAL, 0);
    if(sem_val)
    {
      if(proc_stage)
        semop(semid,&semops_dec_sem2[0],1);
      else
        semop(semid,&semops_dec[0],1);
    }
    else
      semctl(semid, 0, IPC_RMID, 0);
  }
  sprintf(msg, "hxehd terminated via signal = %d\n", sig_type);
  user_msg(datap, 0, INFO, msg);
  exit(0);
}
/***************************************************************************
** Function to send the thread to the proper routine to complete
** the rules stanza.
****************************************************************************/
void * zip_thread (void * rule_stats_p)
{
  int             rc;
  cpu_t           cpunum;
  rule_stats_t    *p;
  struct ruleinfo r;
  struct htx_data s;


  p = (rule_stats_t *)rule_stats_p;
  r = *(p->rule);
  s = *(p->stats);

  if ( !tmp_cpustate )
	    tmp_cpustate = cpustate;
  if ( cpu_bind == 1) {
	  if ( tmp_cpustate ) {
		    rc = hxfbindto_a_cpu(&(tmp_cpustate), thread_self(), &cpunum);
		    if ( rc == -2 ) {
		      sprintf(msg, "Invalid cpustate parameter = %x\n", tmp_cpustate);
		      user_msg(&s, 0, INFO, msg);
		    } else if ( rc ) {
		      sprintf(msg, "Bind thread to a processor failed = %d\n", rc);
		      user_msg(&s, 0, INFO, msg);
		    }
	  }
  }


  rc = pthread_mutex_lock(&arg_mutex);
  if ( rc ) {
    sprintf(msg, "First mutex lock failed in process ZIP_THREAD, rc = %d\n",
            rc);
    user_msg(&s, 950, HARD, msg);
    exit(0);
  }
  rc = pthread_cond_broadcast(&create_thread_cond_var);
  if ( rc ) {
    sprintf(msg, "Cond broadcast failed in process ZIP_THREAD, rc = %d\n", rc);
    user_msg(&s, 950, HARD, msg);
    exit(0);
  }
  rc = pthread_cond_wait(&do_oper_cond_var, &arg_mutex);
  if ( rc ) {
    sprintf(msg, "Cond wait failed in process ZIP_THREAD, rc = %d\n", rc);
    user_msg(&s, 950, HARD, msg);
    exit(0);
  }
  rc = pthread_mutex_unlock(&arg_mutex);
  if ( rc ) {
    sprintf(msg, "First mutex unlock failed in process ZIP_THREAD, rc = %d\n",
            rc);
    user_msg(&s, 950, HARD, msg);
    exit(0);
  }
  if ( strcmp(r.oper, "BWRC") == 0 ) {
     number_of_threads--;
     backgrnd_thread = 'R';
  }
  proc_rule(s, r);
  rc = pthread_mutex_lock(&arg_mutex);
  if ( rc ) {
    sprintf(msg, "2nd mutex lock failed in process ZIP_THREAD, rc = %d\n",
            rc);
    user_msg(&s, 950, HARD, msg);
    exit(0);
  }
  if ( strcmp(r.oper, "BWRC") == 0 )
     backgrnd_thread = 'F';
  else
     number_of_threads--;
  rc = pthread_mutex_unlock(&arg_mutex);
  if ( rc ) {
    sprintf(msg, "2nd mutex unlock failed in process ZIP_THREAD, rc = %d\n",
            rc);
    user_msg(&s, 950, HARD, msg);
    exit(0);
  }
}

main (int argc, char *argv[])
{
  int              blksize;
  int              done, i, j, id, rc, stanza_num,len;
  long             num_copies = 0;
  unsigned long long            maxblk;
  char             *dev, bad_rule, work[100],stanza_str[128];
  rule_stats_t     rule_stats;
  struct ruleinfo  r;
  struct htx_data  s;
  struct sigaction sigvector;
  struct sigaction sigdata;
  static sigset_t  sigmask;
  struct utsname machine_info;
  char * d_name;
  char *kdblevel = NULL;
  char fname[20];
  char *ra = "/tmp/stats";
  unsigned short sem_vals[3];
  char str1[250], str2[250];
  FILE *fp2;



  pthread_attr_init(&thread_attrs_detached);
  pthread_attr_setdetachstate(&thread_attrs_detached, PTHREAD_CREATE_DETACHED);

  pthread_mutex_init(&arg_mutex, DEFAULT_MUTEX_ATTR_PTR);
  pthread_mutex_init(&cache_mutex, DEFAULT_MUTEX_ATTR_PTR);
  pthread_mutex_init(&segment_mutex, DEFAULT_MUTEX_ATTR_PTR);

  pthread_cond_init(&create_thread_cond_var, DEFAULT_COND_ATTR_PTR);
  pthread_cond_init(&do_oper_cond_var, DEFAULT_COND_ATTR_PTR);
  pthread_cond_init(&segment_do_oper, DEFAULT_COND_ATTR_PTR);

  sigprocmask(0, NULL, &sigmask);          /* get current signal mask     */
  sigdata.sa_flags = SA_RESTART;           /* restart on signal calls     */
  sigdata.sa_mask = sigmask;
  sigdata.sa_handler = (void(*)(int))sig_function;
  sigaction(SIGUSR1, &sigdata, NULL);      /* call when recv sig 30       */
  sigemptyset(&(sigvector.sa_mask));       /* do not block signals        */
  sigvector.sa_flags = 0;                  /* do not restart system calls */
                                            /* Begin code                   */
  strcpy(s.HE_name, argv[0]);
  strcpy(s.sdev_id, argv[1]);
  strcpy(s.run_type, argv[2]);
  strcpy(rules_file_name, argv[3]);

/* changes for accoustics */
  datap =&s;
  signal(SIGTERM, signal_handler);

  hxfupdate(START, &s);

  kdblevel = getenv("HTXKDBLEVEL");

  sprintf(msg,"argc %d argv[0] %s argv[1] %s argv[2] %s argv[3] %s \n",
	argc,argv[0], argv[1],argv[2],argv[3]);
  user_msg(&s, 0, INFO, msg);

  if ( s.run_type[0] != 'M' ) {
    sprintf(msg, "%s %s %s %s \n", s.HE_name, s.sdev_id, s.run_type,
            rules_file_name);
    user_msg(&s, 0, INFO, msg);
  }
  i = strlen(s.sdev_id);
  done = 0;
  for ( rc = 5; rc < i; rc++ ) {
     pipe_name[done] = s.sdev_id[rc];
     done++;
  }
  strcat(pipe_name, ".pipe");
  done = 0;
  for ( rc = 6; rc < i; rc++ ) {
     device_name[done] = s.sdev_id[rc];
     done++;
  }

  /* get actual number of processors working on this machine */
  rc = hxfinqcpu_state(&cpustate, &count_active_cpus, &arch);
  if ( rc ) {
    count_active_cpus = 1;
    arch = 0;
  }

  rc = gethostname(hostname, 10); /* Get hostname for machine running code */
  if ( rc != 0 ) {
     sprintf(msg, "Unable to get hostname for this run! ");
     user_msg(&s, 0, INFO, msg);
  }
  /*
  ** Convert the machine ID string into a binary array.  uname() returns an
  ** ASCII-encoded hex character string.  This loop converts M_NAME_LEN hex
  ** characters into binary and stores them in the output array.  This array
  ** is used to record the machine ID in patterns that implement a header.
  */
  uname(&machine_info);
  for (i = 0; i < M_NAME_LEN*2; i++) {
    j = (machine_info.machine[i++] & 0x7f) - '0';
    if (j > 9) j -= 7;
    machine_name[i/2] = j << 4;
    j = (machine_info.machine[i] & 0x7f) - '0';
    if (j > 9) j -= 7;
    machine_name[i/2] |= j;
  }

  /*************************************************************************/
  /* Before we open the device, check to see if it is a member of a volume */
  /* group which is currently defined in the ODM.  This check prevents     */
  /* accidentally clobbering a disk which was added to a volume group      */
  /* after logging on as htx.  So it should be impossible to clobber the   */
  /* paging space.  (See defect 305184.)                                   */
  /*                                                                       */
  /* If isVG_memeber returns < 0, then the device is OK to exercise.       */
  /*************************************************************************/

  if ( (rc = isVG_member(s.sdev_id, work, sizeof(work))) == 0) {
     user_msg(&s, 0, HARD, work);
     exit(126);
  } else if ( rc > 0 ) {
     sprintf(msg, "ODM or LVM error in isVG_member, rc = %d\n%s", rc, work);
     user_msg(&s, 0, HARD, msg);
     exit(1);
  }

  for (i=0; i<=open_retry_count; i++) {
     r.fildes = open(s.sdev_id, O_RDONLY);  /* Open device */
     if ( r.fildes == -1 ) {
       if (errno == EINVAL) {
         sprintf(msg, "Open failed in MAIN - ");
         user_msg(&s, errno, INFO, msg);
         sleep(60);
         continue;
       }
       else {
         sprintf(msg, "Open error in MAIN - ");
         user_msg(&s, errno, HARD, msg);
         exit(1);
       }
     }
  }

  rc = ioctl(r.fildes, IOCINFO, &info); /* get device information */
  if ( rc == -1 )
  {
     sprintf(msg, "IOCTL IOCINFO error in MAIN - ");
     user_msg(&s, errno, HARD, msg);
     while ( close(r.fildes) == -1 );
     exit(1);
  }
  if ( info.devtype == DD_SCDISK || info.devtype == DD_SCRWOPT || info.devtype == DD_CDROM)  {

	 /* get device name */
	 d_name = basename(s.sdev_id);
	 if ( strncmp(d_name,"vpath",5) == 0 ) /* Data Path Optimizer Pseudo Device Driver */
		rc = get_lun(&s, d_name); /* D#473903, Changed  get_lun(s.sdev_id) to  get_lun(d_name) */
	 else {
		if ( d_name[0] == 'r')
			rc = get_lun(&s, d_name + 1);
		else
			rc = get_lun(&s, d_name);
		/*rc = get_lun(s.sdev_id+1); to skip first letter from device name*/
	}

     if ( rc  == -1 ) {
        sprintf(msg, "Unable to retreive SCSI ID/LUN from ODM database");
        user_msg(&s, 950, HARD, msg);
        while ( close(r.fildes) == -1 );
        exit(1);
     } else {                                /* print out device information */
		if (info.devtype == DD_CDROM ) {
            maxblk = info.un.sccd.numblks;
  	      blksize = info.un.sccd.blksize;
		}
		else {
            maxblk = (unsigned long)info.un.scdk.numblks;
		blksize = info.un.scdk.blksize;
		}
        sprintf(msg, "Device %s Information:  ID = %d,  LUN = %d\n"
                     "BlockSize = %d (0x%x),  Number of Blocks = %ull (0x%llx)\n"
                     "CPU state = %x,  CPU Count = %d, Arch = %x\n"
                     "Machine ID = %s\n",
                s.sdev_id, scsi_id, lun_id,
                blksize, blksize,
                maxblk, maxblk,
                cpustate, count_active_cpus, arch,
                machine_info.machine);
        user_msg(&s, 0, INFO, msg);
     }
  } else {
     maxblk = info.un.dk.numblks;
     blksize = info.un.dk.bytpsec;
     sprintf(msg, "Device %s Information: \n"
                  "BlockSize = %d (0x%x),  Number of Blocks = %lld (0x%llx)\n"
                  "CPU state = %x,  CPU Count = %d, Arch = %x\n"
                  "Machine ID = %s\n",
             s.sdev_id,
             info.un.dk.bytpsec, info.un.dk.bytpsec,
             info.un.dk.numblks, info.un.dk.numblks,
             cpustate, count_active_cpus, arch,
             machine_info.machine);
     user_msg(&s, 0, INFO, msg);
  }
  if(maxblk == 0)
  {
    maxblk = 0xffffffff;
    sprintf(msg,"It is a 2TB disk maxblk is%ul(0x%x)",maxblk,maxblk);
    user_msg(&s, 0, INFO, msg);
  }

	/* for LA trigger */

	rc = get_dev_fn(s.sdev_id+6);
	if (rc < 0) {
		sprintf(msg,"LA trigger will not be possible\n");
		user_msg(&s, 0, INFO, msg);
	}
	else {
		rc = 0;
		if (strstr(s.sdev_id, "lv0hd")) {
			sprintf(msg, "device %s spans ", s.sdev_id);
			while (strlen(lv_devices[rc])) {
				sprintf(work, "%s ", lv_devices[rc++]);
				strcat(msg, work);
			}
			strcat(msg, "\n");
			user_msg(&s, 0, INFO, msg);
		}
		rc = 0;
		sprintf(msg, "PCI parents are ");
		while (strlen(PCI_parent[rc])) {
			sprintf(work,"%s:%d ", PCI_parent[rc], PCI_devfn[rc++]);
			strcat(msg, work);
		}
		strcat(msg, "\n");
		user_msg(&s, 0, INFO, msg);

	}

	msg[0] =0;
	work[0] = 0;

  while ( close(r.fildes) == -1 );
  if ( strlen(strcpy(work, getenv("HTXPROCESSORS"))) == 0 )
     num_of_processors_allowed = 1;
  else
     sscanf(work, "%d", &num_of_processors_allowed);
  if ( num_of_processors_allowed < 1 )
     num_of_processors_allowed = 1;
                                         /* Begin the mainline of the code  */
  read_rules_file_count = 1;
  rc = 0;
  bad_rule = 'n';
  rc = get_rule(&s, rules_file_name, maxblk, blksize);
  if ( rc != 0 ) {
     sprintf(msg, "Check the HTXMSG log for a listing of errors with the "
                  "rules file! RC = %d\n", rc);
     user_msg(&s, 950, HARD, msg);
     exit(1);
  }

  if (kdblevel != NULL) {
  	if ((atoi(kdblevel) > 0) || (crash_on_miscom == 1) ) crash_on_miscom = 1;
  	else crash_on_miscom = 0;
  }

  sprintf(msg, "crash_on_miscom = %d\n", crash_on_miscom);
  user_msg(&s, 0, INFO, msg);

  time_mark = time(0);                /* set time mark for comparison      */
  current_ruleptr = first_ruleptr;
  pthread_create(&(threads[0]),
                 &thread_attrs_detached,
                 (void *(*)(void *)) run_timer,
                 (void *)(&s));
/* changes for accoustics */
  if(accous_rules)
  {
    sprintf(str1, "cat /usr/lpp/htx/mdt/mdt | grep hxehd | wc -l");
    fp2 = popen(str1,"r");
    fgets(str2,100,fp2);
    sscanf(str2,"%d",&num_copies);
    pclose(fp2);
    num_copies = num_copies/3;
   
    semkey = ftok("/usr/lpp/htx/bin/hxehd",0xacc);
    if(semkey == -1)
    {
      sprintf(msg,"ftok failed with -1\n");
      user_msg(&s, 0, INFO, msg);
    }
    semcount = 3;
    sprintf(msg,"Number of hxehd copies running are %d\n",num_copies);
    user_msg(&s, 0, INFO, msg);
    semid = semget(semkey,semcount,0666);
    if(semid == -1)
    {
      semid = semget(semkey,semcount,IPC_CREAT | 0666);
      if(semid == -1)
      {
        sprintf(msg,"semget failed with -1\n");
        user_msg(&s, 0, INFO, msg);
      }
      else
      {
        sem_vals[0] = num_copies;
        sem_vals[1] = 0;
        sem_vals[2] = num_copies;
        if(semctl(semid,0,SETALL,sem_vals) == -1)
        {
          sprintf(msg,"semctl failed with -1\n");
          user_msg(&s, 0, INFO, msg);
        }
        else
        {
          sprintf(msg,"semval is set\n");
          user_msg(&s, 0, INFO, msg);
        }
      }
    }

    memset (fname,0,sizeof(fname));
    sprintf (fname,"%s%s",ra,&argv[1][5]);
    stats_fd = open(fname,O_RDWR | O_CREAT | O_TRUNC, 004000);
  }
  do {
    collisions = 0;
    stanza_num = 0;
    threads_created = 1;
    number_of_threads = 0;
/* changes for accoustics */
    /*if(accous_rules)
    {
      len = sprintf (stanza_str,"rule file pass no is %d\n",read_rules_file_count);
      write (stats_fd,stanza_str,len);
    }*/
    while ( 1 ) {
      if ( current_ruleptr == NULL ) {
         if ( backgrnd_thread == 'F' )
            break;
         else {
            sprintf(msg, "Pass #%d of rule file %s completed.\n"
                         "Collision count = %d."
                         "  Back_Ground thread is %d percent complete!\n",
                    read_rules_file_count, rules_file_name, collisions,
                    percent);
            user_msg(&s, 0, INFO, msg);
            hxfupdate(FINISH, &s);
            read_rules_file_count++;
            stanza_num = 0;
            current_ruleptr = first_ruleptr;
         }
      }
      s.test_id = rule_stanza[stanza_num];
      stanza_num++;

      rc = 0;
      if ( current_ruleptr->repeat_pos > 0 ) {
        if ( (read_rules_file_count == 1) ||
             ((read_rules_file_count % current_ruleptr->repeat_pos) != 0) ||
             ((current_ruleptr->repeat_pos == 1) &&
              (!(read_rules_file_count & 1))) )
           rc = 1;
      } else if ( current_ruleptr->repeat_neg > 0 ) {
        if ( (read_rules_file_count != 1) &&
             ((read_rules_file_count % current_ruleptr->repeat_neg) != 0) ||
             ((current_ruleptr->repeat_neg == 1) &&
              (!(read_rules_file_count & 1)) &&
              (read_rules_file_count != 1)) )
           rc = 1;
      }
      if ( s.run_type[0] == 'O'                ||
           current_ruleptr->messages[0] == 'Y' ||
           current_ruleptr->messages[0] == 'D' ||
           current_ruleptr->stanza_msg == 1 ) {
         if ( rc == 0 )
            start_msg(&s, current_ruleptr, msg);
         else {
            sprintf(msg, "Rule %s Has Been SKIPPED......\n",
                     current_ruleptr->rule_id);
            user_msg(&s, 0, INFO, msg);
         }
      }
	  hxfupdate(UPDATE, &s);
      if ( rc == 0 ) {
         if ( (strncmp(current_ruleptr->oper, "OM", 2) == 0) ||
              (strcmp(current_ruleptr->oper, "S") == 0)      ||
              (strncmp(current_ruleptr->oper, "X", 1) == 0) ) {
            if ( strcmp(current_ruleptr->oper, "OMINI") == 0 )
               rc = ominit(s, current_ruleptr);
            else if ( strcmp(current_ruleptr->oper, "OMMV") == 0 )
               rc = ommove(s, current_ruleptr);
            else if ( strcmp(current_ruleptr->oper, "OMINF") == 0 )
               rc = ominfo(s, current_ruleptr);
            else if ( strcmp(current_ruleptr->oper, "OMINV") == 0 )
               rc = ominvtry(s, current_ruleptr);
            else if ( strcmp(current_ruleptr->oper, "OMINQ") == 0 )
               rc = ominqry(s, current_ruleptr);
            else if ( strcmp(current_ruleptr->oper, "OMEX") == 0 )
               rc = omexchge(s, current_ruleptr);
            else if ( strcmp(current_ruleptr->oper, "OMPRE") == 0 )
               rc = omprevent(s, current_ruleptr);
            else if ( strcmp(current_ruleptr->oper, "S") == 0 )
               rc = usleep(current_ruleptr->sleep);
            else if ( strcmp(current_ruleptr->oper, "XCMD") == 0 )
               rc = do_cmd(&s, current_ruleptr);
         } else {
            if ( (backgrnd_thread == 'R') &&
                 (strcmp(current_ruleptr->oper, "BWRC") == 0) ) {
               sprintf(msg, "Rule %s Has Been SKIPPED because the previous "
                            "\nBACKGROUND thread has not FINISHED!\n",
                       current_ruleptr->rule_id);
               user_msg(&s, 0, INFO, msg);
            } else {
               rc = pthread_mutex_lock (&arg_mutex);
               if ( rc ) {
                  sprintf(msg, "Mutex lock failed in MAIN, rc = %d\n", rc);
                  user_msg(&s, 950, HARD, msg);
                  exit(0);
               }
               number_of_threads++;
               rule_stats.stats = &s;
               rule_stats.rule = current_ruleptr;
               if ( rc = pthread_create(&(threads[threads_created]),
                                        &thread_attrs_detached,
                                        (void *(*)(void *)) zip_thread,
                                        (void *)(&rule_stats)) ) {
                  sprintf(msg, "rc %d, errno %d from main(): pthread_create",
                          rc, errno);
                  user_msg(&s, 950, HARD, msg);
                  exit(rc);
               } else {  /* detach thread to facilitate thread exit */
                  rc = pthread_detach(threads[threads_created]);
               }

			   /* sprintf(msg, "processing stanza (RULE) %d (%s) tid %x ",
				 stanza_num,current_ruleptr->rule_id,threads[threads_created]);
	           user_msg(&s, 0, INFO, msg);   */
               rc = pthread_cond_wait(&create_thread_cond_var, &arg_mutex);
               if ( rc ) {
                  sprintf(msg, "Cond wait failed in MAIN, rc = %d\n", rc);
                  user_msg(&s, 950, HARD, msg);
                  exit(0);
               }
               threads_created++;
               if ( current_ruleptr->start == 'Y' ) {
                  rc = pthread_cond_broadcast(&do_oper_cond_var);
                  if ( rc ) {
                     sprintf(msg,"Cond broadcast failed in MAIN, rc = %d\n",rc);
                     user_msg(&s, 950, HARD, msg);
                     exit(0);
                  }
                  rc = pthread_mutex_unlock(&arg_mutex);
                  if ( rc ) {
                     sprintf(msg, "Mutex unlock failed in MAIN, rc = %d\n", rc);
                     user_msg(&s, 950, HARD, msg);
                     exit(0);
                  }
                  while ( number_of_threads > 0  &&
                          number_of_threads < MAX_THREADS )
                      sleep(1);
                  if ( number_of_threads != 0 ) {
                     sprintf(msg, "Error with Threads: Number_of_Threads = %d\n"
                                  "Threads_Created = %d, Backgrnd_Thread = %c,"
                                  " Percent = %d\n",
                             number_of_threads, threads_created,
                             backgrnd_thread, percent);
                     user_msg(&s, 950, HARD, msg);
                  }
                  if ( backgrnd_thread == 'R' )
                     threads_created = 2;
                  else
                     threads_created = 1;
               } else {
                  rc = pthread_mutex_unlock(&arg_mutex);
                  if ( rc ) {
                     sprintf(msg, "Mutex unlock failed in MAIN, rc = %d\n", rc);
                     user_msg(&s, 950, HARD, msg);
                     exit(0);
                  }
               }
            }
         }
      } else
         rc = 0;
      current_ruleptr = current_ruleptr->next_ruleptr;
      if ( (s.run_type[0] == 'M' || s.run_type[0] == 'O') && rc != 0 )
         break;
      /************************************************************************/
      /* This code looks to see if it has received a signal 30 from the user. */
      /* This is to see if the user has made a change to the rules file. The  */
      /* program will reread the rules file and start processing again.       */
      /************************************************************************/
      if ( signal_flag == 'Y' ) {
         rc = get_rule(&s, rules_file_name, maxblk, blksize);
         if ( rc != 0 ) {
            sprintf(msg, "Check the HTXMSG log for error information on "
                         "the pipe rules file!\n");
            user_msg(&s, 950, HARD, msg);
            return(1);
         } else
            current_ruleptr = first_ruleptr;
      }
    }
    sprintf(msg, "Pass #%d, rule file %s completed.\nCollision count = %d.",
            read_rules_file_count, rules_file_name, collisions);
    user_msg(&s, 0, INFO, msg);
    hxfupdate(FINISH, &s);
    read_rules_file_count++;
    current_ruleptr = first_ruleptr;
  } while ( strcmp(s.run_type, "OTH") != 0 );
  pthread_attr_destroy(&thread_attrs_detached);
/* changes for accoustics */
  close(stats_fd);

  return(rc);
}            /* end main          */

/**************************************************************************/
/* put out rule started message                                           */
/**************************************************************************/
void
start_msg(struct htx_data *ps, struct ruleinfo *pr, char *msg_text)
{
  if ( strcmp(pr->addr_type,"SEQ") == 0 )
     sprintf(msg_text, "Started %s pattern_id = %s sleep = %d\n"
                       "addr_type = %s num_oper = %d oper = %s\n"
                       "starting_block = %#llx direction = %s increment = %d\n"
                       "type_length = %s num_blks = %d total blks = %#llx",
             pr->rule_id, pr->pattern_id, pr->sleep, pr->addr_type,
             pr->num_oper, pr->oper, pr->starting_block, pr->direction,
             pr->increment, pr->type_length, pr->num_blks,
             pr->num_blks * pr->num_oper);
  else
     sprintf(msg_text, "Started %s pattern_id = %s sleep = %d\n"
                       "addr_type = %s num_oper = %d oper = %s\n"
                       "type_length = %s num_blks = %d tot_blks < %#llx",
             pr->rule_id, pr->pattern_id, pr->sleep, pr->addr_type,
             pr->num_oper, pr->oper, pr->type_length, pr->num_blks,
             pr->num_blks * pr->num_oper);
  user_msg(ps, 0, INFO, msg_text);
}

/******************************************************************************
* getsysvol(buf,bufsize)                                                      *
*                                                                             *
* get the list of physical volumes for rootvg from /tmp/sysdisk,              *
* the rootvg may span multiple physical volumes.                              *
*                                                                             *
* /tmp/sysdisk should be formated as:                                         *
*     vol1:vol2:vol3:                                                         *
*                                                                             *
* with this command:                                                          *
*     lsvg -p rootvg|awk 'NR>2 {printf("%s:",$1);}'                           *
*                                                                             *
* returns pointer to buf containing the system volume name vector.            *
* returns 0 on error                                                          *
******************************************************************************/
char * getsysvol(char *buf, int bufsize)
{
   FILE *fd;

   if ( (fd = fopen("/tmp/sysdisk", "r")) == NULL )
       return(NULL);
   if ( fgets(buf, bufsize, fd) == NULL )
       return(NULL);                                  /* error reading pipe */
   fclose(fd);                                        /* check for SIGCHLD  */
                                            /* eliminate '\n' at end of buf */
   { char *p; for (p = buf; *p; p++) if (*p == '\n') { *p = '\0'; break; } }
   return (buf);
}

/*****************************************************************************
* proc_rule.c                                                                *
* Description: Process a rules stanza                                        *
* 1/9/98 chg 1 - make sure calculation for number of operations using        *
*                max_blkno - min_blkno when direction is "OUT" or when       *
*                starting from the "TOP".                                    *
* 1/9/98 chg 2 - changed sign in if statement form <= to <.                  *
******************************************************************************/
proc_rule(struct htx_data s, struct ruleinfo r)
{
  unsigned long long      blkno[3]; /* block number pointers - 0 - current, 1 - up, 2 - down */
  int      loop, rc, i, do_partial, wrap_rc,j;
  int      id, lun, lockodm, count, err_no,len;
  int      mallocount = 0,mallocount_temp = 0;
  unsigned long long   saved_num_blks;
  int      saved_dir_flag = 0, prcent_check = 20;
  unsigned int      saved_dlen;
  int      write_stanza,read_stanza,semval = 0;
  unsigned long long total_bytes_transfered,total_writes,total_reads,transfer_rate;
  double	prcnta, prcntb;
  char     tim[20], tot[20], tmpmsg[200], msg[MSG_TEXT_SIZE];
  char     errmsg[100], parent[80], work[200], path[100];
  char     *wptr[MAX_BUFFERS], *rptr[MAX_BUFFERS], seedswtch = 'N';
  char     *wbuf, *rbuf,stanza_str[128],operation[6];
  double   total_bytes, timer3;
  time_t   timer1, timer2,start_time,current_time,end_time,time_spent,sleep_time;
/* changes for accoustics */
  struct sembuf semops_dec[1] = {0,-1,1};
  struct sembuf semops_cmp[1] = {0,0,1};
  struct sembuf semops_inc[1] = {0,1,1};
  struct sembuf semops_inc_sem2[1] = {1,1,1};
  struct sembuf semops_dec_sem2[1] = {1,-1,1};
  struct sembuf semops_cmp_sem2[1] = {1,0,1};
  timebasestruct_t rdt_start, rdt_present, rdt_spent, rdt_oprate; /* RDT changes */
  struct timestruc_t sleep_timer;
  int secs, nsecs;
  struct   htx_data *ps;
  struct   ruleinfo *pr;
  struct   devinfo b;
  struct   random_seed_t seed;
  struct   random_seed_t data_seed;
  unsigned psize = getpagesize() - 1;

  ps = &s;
  pr = &r;
  *msg = NULL;
  bzero(wptr,MAX_BUFFERS);
  bzero(rptr,MAX_BUFFERS);

  /***************************************************************************
  *  prevent other thread from starting if background thread gets delayed by *
  *  the call to open().                                                     *
  ***************************************************************************/
  if ( strcmp(pr->oper, "BWRC") == 0 )
    lba_fencepost = 0;

  /***************************************************************************
  * Open disk to be exercised                                                *
  ***************************************************************************/
 for(j=0; j<=open_retry_count; j++) {
  if( strchr(pr->oper, 'W') != NULL )
  {
    pr->fildes = open(ps->sdev_id, O_RDWR);
  }
  else
  {
    pr->fildes = open(ps->sdev_id, O_RDONLY);
  }
  if ( pr->fildes == -1 ) {
     if ( errno == EMEDIA ) {
        if ( odm_initialize() < 0 ) {
           odm_err_msg(odmerrno, (unsigned char**)&errmsg);
           sprintf(msg, "ID = %s, Initializing ODM operations \n"
                        "ODMerrno = %d (%s)\n", pr->rule_id, ODMERR);
           user_msg(ps, errno, HARD, msg);
           return(errno);
        }
        rc = 0;
        i = 0;
        while ( rc == 0 ) {
           if ( (lockodm = odm_lock("/etc/objrepos/config_lock", 0)) == -1 ) {
              if ( i > 5 ) {
                 odm_err_msg(odmerrno, (unsigned char**)&errmsg);
                 sprintf(msg, "ID = %s, Locking config database - \n"
                              "ODMerrno = %d (%s)\n", pr->rule_id, ODMERR);
                 user_msg(ps, errno, HARD, msg);
                 return(errno);
              } else {
                 sleep(2);
                 i++;
              }
            } else
              rc = 1;
        }
        if ( (catt = getattr(device_name,"block_size",FALSE,&count)) == NULL ) {
           sprintf(msg, "ID = %s, Unable to retrieve blocksize from the ODM",
                   pr->rule_id);
           user_msg(ps, 950, HARD, msg);
           return(-1);
        } else if ( strcmp(catt->value, "512") == 0 )
             sprintf(catt->value, "%s", "1024");
          else
             sprintf(catt->value, "%s", "512");
        if ( putattr(catt) < 0 ) {
           odm_err_msg(odmerrno, (unsigned char**)&errmsg);
           sprintf(msg, "ID = %s, PutAttr setting blocksize - \n"
                        "ODMerrno = %d (%s)\n", pr->rule_id, ODMERR);
           user_msg(ps, errno, HARD, msg);
           return(errno);
        }
        sprintf(work, "rmdev -l %s >/tmp/odmmsg 2>/tmp/odmmsg", device_name);
        if ( (rc = system(work)) != 0 ) {
           sprintf(work, "%s", "/tmp/odmmsg");
           if ( (tptr = fopen(work, "r")) == NULL ) {
               sprintf(msg, "ID = %s, error opening tmp ODM error msg file - ",
                       pr->rule_id);
               if (errno <= sys_nerr)
                  strcat(msg, sys_errlist[errno]);
               strcat(msg, "\n");
               user_msg(ps, errno, HARD, msg);
               return(1);
           }
           i = 10;
           strcpy(msg, " ");
           tmpmsg[0] = '\0';
           while ( i > 1 ) {
              if ( fgets(tmpmsg, 200, tptr) != NULL ) {
                 strcat(msg, tmpmsg);
                 i = strlen(work);
              } else
                 i = 0;
           }
           strcat(msg, "\n");
           user_msg(ps, 950, HARD, msg);
           return(rc);
        }
        sprintf(work, "mkdev -l %s >/tmp/odmmsg 2>/tmp/odmmsg", device_name);
        if ( (rc = system(work)) != 0 ) {
           sprintf(work, "%s", "/tmp/odmmsg");
           if ( (tptr = fopen(work, "r")) == NULL ) {
               sprintf(msg, "ID = %s, Error opening tmp ODM error msg file - ",
                       pr->rule_id);
               if ( errno <= sys_nerr )
                  strcat(msg, sys_errlist[errno]);
               strcat(msg, "\n");
               user_msg(ps, errno, HARD, msg);
               return(1);
           }
           i = 10;
           strcpy(msg, " ");
           tmpmsg[0] = '\0';
           while ( i > 1 ) {
              if ( fgets(tmpmsg, 200, tptr) != NULL ) {
                 strcat(msg, tmpmsg);
                 i = strlen(work);
              } else
                 i = 0;
           }
           strcat(msg, "\n");
           user_msg(ps, 950, HARD, msg);
           return(rc);
        }
        if ( odm_unlock(lockodm) < 0 ) {
           odm_err_msg(odmerrno, (unsigned char**)&errmsg);
           sprintf(msg, "ID = %s, Unlocking config database - \n"
                        "ODMerrno = %d (%s)\n", pr->rule_id, ODMERR);
           user_msg(ps, errno, HARD, msg);
           return(errno);
        }
        odm_terminate();
	if( strchr(pr->oper, 'W') != NULL )
          pr->fildes = open(ps->sdev_id, O_RDWR);
        else
          pr->fildes = open(ps->sdev_id, O_RDONLY);
        if ( pr->fildes == -1 ) {
           sprintf(msg, "ID = %s, Open error in procrule - ", pr->rule_id);
           if ( errno <= sys_nerr ) strcat(msg, sys_errlist[errno]);
           strcat(msg, "\n");
           user_msg(ps, errno, HARD, msg);
           return(1);
        }
        break;
     } else if (errno == EINVAL) {
        sprintf(msg, "ID = %s, Open failed in proc rule - ", pr->rule_id);
        strcat(msg, sys_errlist[errno]);
        strcat(msg, "\n");
        user_msg(ps, errno, INFO, msg);
        sleep(60);
        continue;
     } else {
        sprintf(msg, "ID = %s, Open error in proc rule - ", pr->rule_id);
        if ( errno <= sys_nerr ) strcat(msg, sys_errlist[errno]);
        strcat(msg, "\n");
        user_msg(ps, errno, HARD, msg);
        return(1);
     }
  } else break;
 }

  /**************************************************************************
  * Set disk parameters                                                     *
  **************************************************************************/
  switch (info.devtype) {
     case DD_SCDISK:
     case DD_SCRWOPT:
		 pr->bytpsec   = info.un.scdk.blksize;
		 pr->tot_blks  = info.un.scdk.numblks;
		 break;
	 case DD_CDROM:
		 pr->bytpsec   = info.un.sccd.blksize;
		 pr->tot_blks  = info.un.sccd.numblks;
		 break;
	 default:
		 pr->bytpsec   = info.un.dk.bytpsec;
		 pr->tot_blks  = info.un.dk.numblks;
  } /* end switch */
  if(pr->tot_blks == 0)
  {
    pr->tot_blks = 0xffffffff;
    sprintf(msg,"It is a 2TB disk, tot_blks is%ul(0x%x)",pr->tot_blks,pr->tot_blks);
    user_msg(&s, 0, INFO, msg);
  }

  /*
   * 494699, If disk contains < num_blks (specified in the stanza)
   * Take the min of tot_blks and max_blkno.
   */

    if( (pr->num_blks > pr->tot_blks) || (pr->num_blks > pr->max_blkno) )
    {
       pr->num_blks = (pr->max_blkno - pr->tot_blks)? pr->tot_blks: pr->max_blkno;
    }


  if ( strcmp(pr->oper, "BWRC") == 0 ) {
     time_mark = time(0);
     if ( pr->om_prevent == 0 ) {      /* if user did not specify max blkno */
        pr->max_blkno = pr->tot_blks;
        prcnta = pr->tot_blks;
     } else                            /* else use the user specified blkno */
        prcnta = pr->max_blkno;
  }
  /*
   * 494699, For small disks, background thread would have already finished
   * even before other threads are created. So, other threads might cross the
   * fencepost boundary, thereby carrying a potential risk of false miscompares.
   */
/*  if ( ((backgrnd_thread == 'R') || (backgrnd_thread == 'E')) &&
       (strcmp(pr->oper, "BWRC") != 0) ) { */
/*
 * D502299 changes This was breaking the rules file which do not
 * had any BWRC stanza, like default.dvd. So, lba_fencepost variable
 * was always set to 0 there and hence when the threads came out of
 * the sleep below, they reset tot_blks and max_blkno to 0, thereby
 * preventing any further execution of these stanzas
 */
  if ( (strcmp(pr->oper, "BWRC") != 0) && (is_bwrc_rule_present == 1))
  {
     while ( (backgrnd_thread == 'R') && (lba_fencepost == 0) )
        sleep(60);      /* take a nice long nap */
     if ( (pr->max_blkno == 1) || (pr->max_blkno > lba_fencepost) ) {
        pr->tot_blks  = lba_fencepost;
        pr->max_blkno = pr->tot_blks;
     }
  } else if ( pr->max_blkno == 1 )
     pr->max_blkno = pr->tot_blks;
  if ( pr->min_blkno >= pr->max_blkno ) {
     if ( backgrnd_thread == 'E' ) {
        sprintf(msg, "This stanza %s will not be run as MIN LBA %d "
                     "is greater than the MAX LBA %d.\nThe cause "
                     "of this error is due to a HARD error in the "
                     "background thread.\n",
                pr->rule_id, pr->min_blkno, pr->max_blkno);
        user_msg(ps, 0, HARD, msg);
        rc = -1;
     } else {
        sprintf(msg, "This stanza %s will be skipped as MIN LBA %d"
                     "\nis greater than the MAX LBA %d",
                pr->rule_id, pr->min_blkno, pr->max_blkno);
        user_msg(ps, 0, INFO, msg);
        rc = 0;
     }
     while ( close(pr->fildes) == -1 );
     return(rc);
  }
  if ( pr->min_blkno > pr->tot_blks ) {
     sprintf(msg, "This stanza %s will not be run as the MINIMUM LBA %#llx"
                  "\nis greater than the TOTAL NUMBER of BLOCKS %#llx on disk",
             pr->rule_id, pr->min_blkno, pr->tot_blks);
     user_msg(ps, 950, HARD, msg);
     while ( close(pr->fildes) == -1 );
     return(-1);
  } else if ( pr->max_blkno > pr->tot_blks ) {
     sprintf(msg, "In this stanza %s: MAXIMUM LBA %#llx exceeded TOTAL BLOCKS %#llx "
                 "\nMAXIMUM LBA will be reset to TOTAL BLOCKS!",
             pr->rule_id, pr->max_blkno, pr->tot_blks);
     user_msg(ps, 0, INFO, msg);
     pr->max_blkno = pr->tot_blks;
  } else if ( pr->first_block < pr->min_blkno ) {
     sprintf(msg, "In this stanza %s: STARTING LBA %#llx was less than MINIMUM "
                  "LBA %#llx\nSTARTING LBA will be reset to MINIMUM LBA!",
             pr->rule_id, pr->first_block, pr->min_blkno);
     user_msg(ps, 0, INFO, msg);
     pr->first_block = pr->min_blkno;
  } else if ( pr->first_block >= pr->max_blkno ) {
     sprintf(msg, "This stanza %s will not be run as the STARTING LBA %#llx"
                  "\nis greater than the MAX LBA %#llx",
             pr->rule_id, pr->first_block, pr->max_blkno);
     user_msg(ps, 950, HARD, msg);
     while ( close(pr->fildes) == -1 );
     return(-1);
  }

  /* initialize seed for random number generator */
  if ( pr->rule_options & RESTORE_SEEDS_FLAG ) {
    seed = saved_seed;
    data_seed = saved_data_seed;
    pr->max_blkno = saved_maxlba;
    seedswtch = 'Y';
  } else if ( pr->rule_options & USER_SEEDS_FLAG ) {
    for ( i = 0; i < 3; i++ )
       seed.xsubi[i] = pr->useed[i];
    for ( i = 0; i < 3; i++ )
       data_seed.xsubi[i] = pr->useed[i+3];
    pr->max_blkno = pr->useed_lba;
    seedswtch = 'Y';
  } else {
    init_seed(&seed);
    init_seed(&data_seed);
    if ( pr->rule_options & SAVE_SEEDS_FLAG ) {
      seedswtch = 'Y';
      saved_seed = seed;
      saved_data_seed = data_seed;
      saved_maxlba = pr->max_blkno;
      if ( strcmp(pr->type_length, "FIXED") == 0 )
         saved_data_len = pr->num_blks * pr->bytpsec;
      else
         saved_data_len = -1;
    }
  }
  for ( i = 0; i < 3; i++ )
     pr->seed[i] = seed.xsubi[i];
  for ( i = 0; i < 3; i++ )
     pr->seed[i+3] = data_seed.xsubi[i];
  pr->seed_lba = saved_maxlba;

  /* initialize length of data to be transmitted */
  if (strcmp(pr->type_length, "FIXED") == 0)
     pr->dlen = pr->num_blks * pr->bytpsec;
  else
     random_dlen(pr->bytpsec, pr->max_blkno, &seed, pr);

  /* initialize current block number */
  if ( strcmp(pr->addr_type, "SEQ") == 0 )
     init_blkno(ps,pr, blkno);
  else {
    /************************************************************************
    ** If we are using a restored seed and fixed data lengths, generate the
    ** random block number based on the length of the data when the original
    ** writes occured.  This is necessary because the data length used to
    ** write the random addresses could be different than the length used to
    ** read them back.  See the random_blkno() routine for details.
    *************************************************************************/
     if ( (pr->rule_options & RESTORE_SEEDS_FLAG) &&
          (strcmp(pr->type_length, "FIXED") == 0) )
        random_blkno(blkno, saved_data_len, pr->bytpsec, pr->max_blkno,
                     &seed, pr->min_blkno, pr->lba_align);
     else    /* We're not restoring seeds, so use the current data length */
        random_blkno(blkno, pr->dlen, pr->bytpsec, pr->max_blkno,
                     &seed, pr->min_blkno, pr->lba_align);
  }

  /* Check if user specified a seed value for this rule.  Indicated by pattern
     ID of !001xxx or !002xxx. */
  if ( pr->pattern_id[0] == '!' ) {
    if ( (strncmp(pr->pattern_id + 1, "001", 3) != 0 &&
          strncmp(pr->pattern_id + 1, "002", 3) != 0) ||
          strlen(pr->pattern_id) != 7 ) {
       sprintf(msg,
                "pattern error - Fixed seed (\"%s\") patterns must have form:\n"
                "!001xyz or !002xyz where xyz is used as the seed.\n",
               pr->pattern_id);
       user_msg(ps, 950, HARD, msg);
       while ( close(pr->fildes) == -1 );
       return(1);
    }
    data_seed.xsubi[0] = pr->pattern_id[4];
    data_seed.xsubi[1] = pr->pattern_id[5];
    data_seed.xsubi[2] = pr->pattern_id[6];
  }

/*****************************************************************************
** Fix the logic so that if user specs num_oper = 0, we reset it to the
** number of operations required to do the whole disk.  Note - last loop may
** not have full num_blks left on disk so a partial xfer may be required.
** A partial xfer will ONLY be allowed if the num_oper = 0.  The "do_partial"
** flag is set so that if a wrap is detected then the partial transfer will
** be done.  Otherwise, the wrap will be done and the partial segment skipped.
** By "whole disk" we mean the disk area specified from first_block to
** max_blkno.
******************************************************************************/
/* 494699 */
do_partial = 0;
  if ( pr->num_oper == 0 || (strncmp(pr->rule_id, "CLEAN", 5) == 0)) {
     if ( (strcmp(pr->direction, "OUT") == 0) ||        /* chg 1 */
          (strcmp(pr->starting_block, "TOP") == 0) )
        rc = pr->max_blkno - pr->min_blkno; /* 494699 */
     else
        rc = pr->max_blkno - pr->first_block; /* 494699 */
     pr->num_oper = rc / (pr->num_blks + pr->increment);
     if ( rc % (pr->num_blks + pr->increment) ) {
        pr->num_oper++;
        do_partial = 1;
     }
  } else {
     /* do_partial = 0; */ /* 494699 */
     if ( pr->percent > 0.0 ) {
       /**********************************************************************
       ** correct the calculation for SCALE_OPER function...
       ** The following calculation scales the number of operations as a
       ** percentage of the range between max_blkno and min_blkno.  The size
       ** of the range in blocks is divided by the number of blocks per
       ** operation in the rule.  The result is the number of operations it
       ** takes to do the whole range.  Then a percentage of that number is
       ** used to set the number of operations to perform.
       ***********************************************************************/
        pr->num_oper = ( (pr->max_blkno - pr->min_blkno) / pr->num_blks ) *
                        pr->percent;
        if ( strcmp(pr->type_length, "RANDOM") == 0 )
           pr->num_oper = pr->num_oper / 2;
     }
  }

    /*
     * 494699, Need to check wrap before first iteration also as
     * first iteration itself might require partial transfer logic.
     * This piece of code is taken from the hxehur exerciser.
     */

    if (wrap (pr, blkno) ) {
                if ( do_partial ) {
                    if ( *pr->direction == 'U') /* UP direction */
                    {
                      if ((pr->max_blkno - blkno[1]) < (pr->num_blks + pr->increment))
                      pr->num_blks = (int)(pr->max_blkno - blkno[1]); /* calc partial blks */
                    }
                    else if ( *pr->direction == 'D') /* DOWN direction */
                    {
                      if ((pr->num_blks + blkno[2]) > pr->min_blkno)
                      pr->num_blks = pr->num_blks - (int)(pr->min_blkno - blkno[2]); /* calc partial blks */
                      blkno[0] = blkno[2] = pr->min_blkno; /* adjust start LBA for partial transfer */
                    }
                    else
                      init_blkno(ps,pr, blkno); /* unexpected direction - init blk to be safe */

                    do_partial = 0; /* clear do_partial flag, now we've used it for this stanza */

                    pr->dlen = pr->num_blks * pr->bytpsec;
                        sprintf(msg,"Partial xfer required on loop %d starting at\n"
                                "block number %lld (0x%llx).  Transfer length will be\n"
                                "%d (0x%x) blocks (%d bytes).",
                                loop+1, blkno[0], blkno[0], pr->num_blks,
                                pr->num_blks, pr->dlen);
                        user_msg(ps, 0, INFO, msg);
                } else { /* Dont do this for random addressing... DANGEROUS!!! */
                	if (strcmp (pr->addr_type, "RANDOM") != 0) /* 494699 */
                    init_blkno(ps,pr, blkno);
			   }
            }

  saved_num_blks = pr->num_blks;
  saved_dlen = pr->dlen;
  if ( pr->timed == 'Y' )
  {
     total_bytes = 0.0;
     (void) time(&timer1);         /* start timer */
  }
  total_bytes_transfered = 0ull;
  total_writes = 0ull;
  total_reads = 0ull;

  if ( strstr(pr->rule_id, "RDT")) {  /* RDT changes */
      read_real_time(&rdt_start, TIMEBASE_SZ);
      time_base_to_time(&rdt_start, TIMEBASE_SZ);
      read_real_time(&rdt_oprate, TIMEBASE_SZ);
      time_base_to_time(&rdt_oprate, TIMEBASE_SZ);
	}

/* changes for accoustics */
    /*sprintf(msg,"before semop_dec %d\n",errno);
    user_msg(&s, 0, INFO, msg);*/
  if(accous_rules)
  {
    printf("accoustics: inside semop if\n");
    if (semop(semid,&semops_dec[0],1) == -1)
    {
      sprintf(msg,"semops_dec failed with -1 errnois %d\n",errno);
      user_msg(&s, 0, INFO, msg);
    }
    /*else 
    {
      sprintf(msg,"decrimented\n");
      user_msg(&s, 0, INFO, msg);
    }*/ 
    if (semop(semid,&semops_cmp[0],1) == -1)
    {
      sprintf(msg,"semop failed with -1 errno is %d\n",errno);
      user_msg(&s, 0, INFO, msg);
    } 
    /*else 
    {
      sprintf(msg,"starting stanza\n");
      user_msg(&s, 0, INFO, msg);
    }*/ 
    if (semop(semid,&semops_inc_sem2[0],1) == -1)
    {
      sprintf(msg,"semop failed with -1 errno is %d\n",errno);
      user_msg(&s, 0, INFO, msg);
    } 
    proc_stage = 1;
    start_time = time(0);
  }

  for ( loop = 1; loop <= pr->num_oper; loop++ ) {

	if(exit_flag == 'Y') { 
		break; 
	}

  if (pr->pattern_id[3] == '7') { /* Algorithm 7 needed */
    if (pr->align > 0)
      {
       wptr[mallocount] = (char *) malloc(pr->dlen + pr->align + 128);
       rptr[mallocount] = (char *) malloc(pr->dlen + pr->align + 128);
       if ( wptr[mallocount] == NULL || rptr[mallocount] == NULL ) {
          sprintf(msg, "ID = %s, Malloc error - %s mallocount = %d\n",
                  pr->rule_id, strerror(errno), mallocount);
          user_msg(ps, errno, HARD, msg);
          freebufs(&mallocount, wptr, rptr);
          while ( close(pr->fildes) == -1 );
          return(1);
       }
     }
     else {

        sprintf(msg, "ID = %s, Align value not given. Taking default value as 2 \n", pr->rule_id);
        user_msg(ps, errno, INFO, msg);
        pr->align = 2;

       wptr[mallocount] = (char *) malloc(pr->dlen + pr->align + 128);
       rptr[mallocount] = (char *) malloc(pr->dlen + pr->align + 128);
       if ( wptr[mallocount] == NULL || rptr[mallocount] == NULL ) {
          sprintf(msg, "ID = %s, Malloc error - %s mallocount = %d\n", pr->rule_id, strerror(errno), mallocount);
        user_msg(ps, errno, HARD, msg);
        freebufs(&mallocount, wptr, rptr);
        while ( close(pr->fildes) == -1 );
        return(1);
         }
        }

      wbuf = wptr[mallocount] + pr->offset;
      rbuf = rptr[mallocount] + pr->offset;

      bufrem = ( (unsigned) wbuf )  % (pr->align);
      if ( bufrem != 0 )
       wbuf = wbuf + ( pr->align - bufrem );

      bufrem = ( (unsigned) rbuf ) % (pr->align);

      if ( bufrem != 0 )
       rbuf = rbuf + ( pr->align - bufrem );
    } else {
       wptr[mallocount] = (char *) malloc(pr->dlen + 128);
       rptr[mallocount] = (char *) malloc(pr->dlen + 128);
       if ( wptr[mallocount] == NULL || rptr[mallocount] == NULL ) {
          sprintf(msg, "ID = %s, Malloc error - %s mallocount = %d\n",
                  pr->rule_id, strerror(errno), mallocount);
          user_msg(ps, errno, HARD, msg);
          freebufs(&mallocount, wptr, rptr);
          while ( close(pr->fildes) == -1 );
          return(1);
       }
       wbuf = wptr[mallocount] + pr->offset;
       rbuf = rptr[mallocount] + pr->offset;
    }
                    /* initialize write buffer if static pattern specified */
    if ( pr->pattern_id[0] != '#' && pr->pattern_id[0] != '!' ) {
       strcpy (path, PATLIB_PATH);
       strcat (path, pr->pattern_id);
       rc = hxfpat(path, wbuf, pr->dlen);
       if ( rc == 1 ) {
          sprintf(msg, "cannot open pattern file - %s\n", path);
          user_msg(ps, 950, HARD, msg);
          freebufs(&mallocount,wptr,rptr);
          while ( close(pr->fildes) == -1 );
          return(1);
       }
       if ( rc == 2 ) {
          sprintf(msg, "cannot read pattern file - %s\n", path);
          user_msg(ps, 950, HARD, msg);
          freebufs(&mallocount, wptr, rptr);
          while ( close(pr->fildes) == -1 );
          return(1);
       }
    } else {
        /* Only re-init the pattern seed if type-1 dynamic pattern specified.*/
        /* Type-2 dynamic patterns build on existing pseudo-random sequence. */
       if ( (pr->pattern_id[0] == '#') && (loop > 1) )
          init_seed(&data_seed);
       if ( (strcmp(pr->oper, "R") != 0) && (strcmp(pr->oper, "RS") != 0) )
          bldbuf((unsigned short *)wbuf, &data_seed, blkno, ps, pr, pr->pattern_id[3]);
    }
    if ( ps->run_type[0] == 'O' || pr->messages[0] == 'D' ) {
       info_msg(ps, pr, loop, blkno, msg);
       user_msg(ps, 0, INFO, msg);
    }
    seg_lock(ps, *blkno, *blkno + (pr->dlen / pr->bytpsec));

/*  wb_prot(wbuf);       DEBUG: Write-protect the wbuf */
    if ( strcmp(pr->oper, "RWRC") == 0 ) {
       rc = read_disk(ps, pr, loop, blkno, rbuf);
       if ( rc == 0 )
          rc = write_disk(ps, pr, loop, blkno, wbuf);
       if ( rc == 0 )
          rc = read_disk(ps, pr, loop, blkno, rbuf);
       if ( rc == 0 )
          rc = cmpbuf(ps, pr, loop, blkno, wbuf, rbuf);
    } else if ( strcmp(pr->oper, "WRCW") == 0 ) {
       rc = write_disk(ps, pr, loop, blkno, wbuf);
       if ( rc == 0 )
          rc = read_disk(ps, pr, loop, blkno, rbuf);
       if ( rc == 0 )
          rc = cmpbuf(ps, pr, loop, blkno, wbuf, rbuf);
       /* Re-write the default pattern if the one we just wrote is different */
       if ( rc == 0 && strcmp(default_patternid, pr->pattern_id) ) {
          clrbuf(wbuf, pr->dlen);
          if ( default_patternid[0] != '#' && default_patternid[0] != '!' ) {
             strcpy (path, PATLIB_PATH);
             strcat (path, pr->pattern_id);
             rc = hxfpat(path, wbuf, pr->dlen);
             if ( rc != 0) {
                sprintf(msg, "cannot work pattern file in WRCW - %s\n", path);
                user_msg(ps, 950, HARD, msg);
                freebufs(&mallocount, wptr, rptr);
                while ( close(pr->fildes) == -1 );
                return(1);
             }
          } else
             bldbuf((unsigned short *)wbuf, &data_seed, blkno, ps, pr, default_patternid[3]);
          rc = write_disk(ps, pr, loop, blkno, wbuf);
       }
    } else if ( strcmp(pr->oper, "WRC") == 0 ) {
       rc = write_disk(ps, pr, loop, blkno, wbuf);

       if ( rc == 0 )
          rc = read_disk(ps, pr, loop, blkno, rbuf);
       if ( rc == 0 )
          rc = cmpbuf(ps, pr, loop, blkno, wbuf, rbuf);
    } else if ( strcmp(pr->oper, "BWRC") == 0 ) {
       rc = write_disk(ps, pr, loop, blkno, wbuf);
       if ( rc == 0 )
          rc = read_disk(ps, pr, loop, blkno, rbuf);
       if ( rc == 0 )
          rc = cmpbuf(ps, pr, loop, blkno, wbuf, rbuf);
    } else if ( strcmp(pr->oper, "WR") == 0 ) {
      if (strstr(pr->rule_id, "RDT")) {  /* RDT changes */
       if (((*blkno) & 0x8) >> 3) {
        rc = write_disk(ps, pr, loop, blkno, wbuf);
       }
       else {
        rc = read_disk(ps, pr, loop, blkno, rbuf);
       }
      }
			else {
       rc = write_disk(ps, pr, loop, blkno, wbuf);
       if ( rc == 0 )
          rc = read_disk(ps, pr, loop, blkno, rbuf);
			}
    } else if ( strcmp(pr->oper, "RW") == 0 ) {
       rc = read_disk(ps, pr, loop, blkno, rbuf);
       if ( rc == 0 )
          rc = write_disk(ps, pr, loop, blkno, wbuf);
    } else if ( strcmp(pr->oper, "R") == 0 ) {
       rc = read_disk(ps, pr, loop, blkno, rbuf);
    }
#ifndef __HTX43X__
      else if ( strcmp(pr->oper, "V") == 0 ) {
        if(pass_thru == 1)
               rc = verify_disk_wrapper(ps, pr, loop, blkno, wbuf);
        else{
          sprintf(msg, "Operation type V *NOT* permitted.\n"
                       "See the keyword pass_thru in hxehd.doc file.\n");
          user_msg(ps, 0, INFO, msg);
        }
     }
#endif
     else if ( strcmp(pr->oper, "W") == 0 ) {
       rc = write_disk(ps, pr, loop, blkno, wbuf);
    } else if ( strcmp(pr->oper, "RS") == 0 ) {
       rc = read_disk(ps, pr, loop, blkno, rbuf);
       if ( rc == 0 )
          rc = usleep(pr->sleep);
    } else if ( strcmp(pr->oper, "WS") == 0 ) {
       rc = write_disk(ps, pr, loop, blkno, wbuf);
       if ( rc == 0 )
          rc = usleep(pr->sleep);
    } else if ( strcmp(pr->oper, "CARW") == 0 ) {
       rc = write_disk(ps, pr, loop, blkno, wbuf);
       if ( rc == 0 )
          rc = read_disk(ps, pr, loop, blkno, rbuf);
       if ( rc == 0 )
          rc = cmpbuf(ps, pr, loop, blkno, wbuf, rbuf);
       if ( rc == 0 )
          rc = read_cache_disk(ps, pr, loop, blkno, rbuf, wbuf);
       if ( rc == 0 )
          rc = compare_cache(ps, pr, loop, rbuf, wbuf, blkno);
    } else if ( strcmp(pr->oper,"CAWW") == 0 ) {
       rc = write_cache_disk(ps, pr, loop, blkno, wbuf, rbuf);
       if ( rc == 0 )
          rc = compare_cache(ps, pr, loop, rbuf, wbuf, blkno);
    } else if ( strcmp(pr->oper, "CARR") == 0 ) {
       rc = write_disk(ps, pr, loop, blkno, wbuf);
       if ( rc == 0 )
          rc = read_disk(ps, pr, loop, blkno, rbuf);
       if ( rc == 0 )
          rc = cmpbuf(ps, pr, loop, blkno, wbuf, rbuf);
       if ( rc == 0 )
          rc = read_cache_disk(ps, pr, loop, blkno, rbuf, wbuf);
       if ( rc == 0 )
          rc = compare_cache(ps, pr, loop, rbuf, wbuf, blkno);
    } else if ( strcmp(pr->oper,"CAWR") == 0 ) {
       rc = write_cache_disk(ps, pr, loop, blkno, wbuf, rbuf);
       if ( rc == 0 )
          rc = compare_cache(ps, pr, loop, rbuf, wbuf, blkno);
    } else if ( strcmp(pr->oper, "RC") == 0 ) {
       rc = read_disk(ps, pr, loop, blkno, rbuf);
       if ( rc == 0 )
          rc = cmpbuf(ps, pr, loop, blkno, wbuf, rbuf);
    } else {
      ;
    } /* end if (strcmp(pr->oper,"") == 0  */
/*  wb_unprot(wbuf);   DEBUG: Set buffer back to read/write */
    seg_unlock(ps, pr, *blkno, *blkno + (pr->dlen / pr->bytpsec) );

    if ( (rc != 0) && (ps->run_type[0] == 'M' || ps->run_type[0] == 'O') ) {
       freebufs(&mallocount, wptr, rptr);    /* free write and read buffers  */
       while ( close(pr->fildes) == -1 );
       return(rc);
    }
/* changes for accoustics */
    if(accous_rules)
    {
      strcpy(operation,pr->oper);
      write_stanza = 0;
      read_stanza = 0;
      for(i=0;(i<sizeof(operation)) && (operation[i] != '\0');i++)
      {
        if(operation[i] == 'W')
          write_stanza++;
        else if(operation[i] == 'R')
          read_stanza++;
      }
      /*if(((write_stanza >0) && (read_stanza >0)) || (write_stanza > 1) || (read_stanza >1))
      {
          sprintf(msg, "write_stanza is %d read_stanza is %d operation is %s\n",write_stanza,read_stanza,operation);
          user_msg(ps, 0, INFO, msg);
      }*/
      total_bytes_transfered = total_bytes_transfered + (unsigned long long)(pr->dlen * (write_stanza+read_stanza));
      total_writes = total_writes + (unsigned long long)(pr->dlen * write_stanza);
      total_reads = total_reads + (unsigned long long)(pr->dlen * read_stanza);
    printf("accoustics: inside data count if \n");
    }
    if ( pr->timed == 'Y' )
       total_bytes = total_bytes + (double) pr->dlen;
    if ( (loop % 15) == 0 )
       hxfupdate(UPDATE, ps);                /* update htx statistics        */
    /* set length of data to be transmitted if random data lengths specified */
    if ( strcmp(pr->type_length, "RANDOM") == 0 )
        random_dlen(pr->bytpsec, pr->max_blkno, &seed, pr);
 /*****************************************************************************
 ** Advance to next block in preparation for next loop through the
 ** stanza.  If a partial xfer is required and the do_partial flag is set
 ** then adjust the parameters for the partial xfer, which will only be true
 ** if the num_oper was originally 0 meaning we wanted to walk through the
 ** entire disk.
 ******************************************************************************/
    if ( (backgrnd_thread == 'R') && (strcmp(pr->oper, "BWRC") == 0) ) {
       prcntb = blkno[0] + pr->num_blks; /* We need to add num_blks because blkno is the current blkno, prcnta is equated to total blks. Logically 100% finish comes with prcntb is at total blks - num_blks. */
       percent = (prcntb / prcnta) * 100;
       if ( percent >= prcent_check ) {
          sprintf(msg, "BackGround thread is %d percent finished.\n", percent);
          user_msg(ps, 0, INFO, msg);
          prcent_check += 20;
       }
    } else if ( (backgrnd_thread == 'R') && (strcmp(pr->oper, "BWRC") != 0) ) {
       if ( seedswtch == 'N' )
          pr->max_blkno = lba_fencepost;
       if ( (blkno[0] + pr->num_blks + pr->increment) > lba_fencepost &&
            strcmp(pr->addr_type, "SEQ") == 0 ) {
          sprintf(msg, "Stanza %s has caught the background thread!\n"
                       "LBA will be reset - Could cause miscompares!\n"
                       "Fencepost = %#llx, LBA before reset = %#llx\n",
                  pr->rule_id, lba_fencepost, (blkno[0] + pr->num_blks +
                  pr->increment));
          user_msg(ps, 0, INFO, msg);
       }
    }
    if ( strcmp(pr->addr_type, "RANDOM") == 0 ) {
       if ( (pr->rule_options & RESTORE_SEEDS_FLAG) &&
            (strcmp(pr->type_length, "FIXED") == 0) )
          random_blkno(blkno, saved_data_len, pr->bytpsec,
                       pr->max_blkno, &seed, pr->min_blkno, pr->lba_align);
       else
          random_blkno(blkno, pr->dlen, pr->bytpsec, pr->max_blkno,
                       &seed, pr->min_blkno, pr->lba_align);
    } else {
        if ( saved_dir_flag > 0 ) {        /* this flag is only valid       */
           pr->num_blks = saved_num_blks;  /* if the direction is IN or OUT */
           pr->dlen = saved_dlen;          /* this makes sure we don't lose */
           saved_dir_flag = 0;             /* the original num blks and dlen*/
        }
        set_blkno(blkno, pr->direction, pr->increment, pr->num_blks, pr->lba_align);
          /* If the direction is "IN" then we want to make sure that we don't */
          /* run off the end of the disk, either at the top or the bottom of  */
          /* disk. If we set the lba to an unusable lba then we just reset it */
          /* back to the original starting point and begin again.             */
        if ( *pr->direction == 'I' ) {
           if ( (blkno[0] < pr->min_blkno) ||                     /* chg 2 */
                ((blkno[0] + pr->num_blks + pr->increment) > pr->max_blkno) ) {
              init_blkno(ps,pr,blkno);
              saved_dir_flag = 0;
           }
          /* If the direction is "OUT" then we want to make sure that we do   */
          /* then entire disk but that we don't try to access a bad lba.      */
        } else if ( *pr->direction == 'O' ) {
           if ( blkno[0] > pr->max_blkno )
              init_blkno(ps,pr, blkno);
           else if ( (blkno[0] + pr->num_blks) >= pr->max_blkno ) {
                   pr->num_blks = pr->max_blkno - blkno[0];
                   pr->dlen = pr->num_blks * pr->bytpsec;
                   saved_dir_flag = 2;
                } else if ( blkno[0] < pr->min_blkno ) {
                   if ( (blkno[0] + pr->num_blks) > pr->min_blkno ) {
                      pr->num_blks = (blkno[0] + pr->num_blks) - pr->min_blkno;
                      pr->dlen = pr->num_blks * pr->bytpsec;
                      blkno[0] = pr->min_blkno;
                      saved_dir_flag = 2;
                   } else
                      init_blkno(ps,pr, blkno);
               }
          /* On a sequential transfer going from the bottom of the disk       */
          /* to the top or from the top to the bottom of the disk and there   */
          /* is less than num_blks left & partial xfer is required to finish. */
          /* Then re-calc the residual and adjust the data length, if         */
          /* less than num_blks left and partial xfer not required, then      */
          /* reset back to the starting block number.                         */
        } else if ( wrap(pr, blkno) )
        {
            if ( do_partial )
            {
				/* 494699
                pr->num_blks = (pr->max_blkno - pr->first_block) %
                               (pr->num_blks + pr->increment);
                pr->dlen = pr->num_blks * pr->bytpsec;
                */
                /* 494699 */

		      if ( *pr->direction == 'U') /* UP direction */
		      {
			      if ((pr->max_blkno - blkno[1]) < (pr->num_blks + pr->increment))
				      pr->num_blks = (int)(pr->max_blkno - blkno[1]); /* calc partial blks */
		      }
		      else if ( *pr->direction == 'D') /* DOWN direction */
		      {
			      if ((pr->num_blks + blkno[2]) > pr->min_blkno)
				      pr->num_blks = pr->num_blks - (int)(pr->min_blkno - blkno[2]); /* calc partial blks */
				      blkno[0] = blkno[2] = pr->min_blkno; /* adjust start LBA for partial transfer */
		      }
		      else
		        init_blkno(ps,pr, blkno); /* unexpected direction - init blk to be safe */

		      do_partial = 0; /* clear do_partial flag, now we've used it for this stanza */

		      pr->dlen = pr->num_blks * pr->bytpsec;

                if ( pr->messages[0] != 'N' )
                {
                    sprintf(msg,"Partial xfer required on loop %d starting at\n"
                            "block number %d (0x%x).  Transfer length will be\n"
                            "%d (0x%x) blocks (%d bytes).",
                            loop+1, blkno[0], blkno[0], pr->num_blks,
                            pr->num_blks, pr->dlen);
                    user_msg(ps, 0, INFO, msg);
                }
            } else { /* Dont do this for random addressing.. DANGEROUS!!! */
            	if (strcmp (pr->addr_type, "RANDOM") != 0) /* 494699 */
                init_blkno(ps,pr, blkno);
			}
        }
    }
    if ( mallocount >= pr->no_mallocs - 1 )
      freebufs(&mallocount, wptr, rptr);
    else
      mallocount++;
    if ( pr->loop_on_offset == 1 )
       pr->offset = (pr->offset +1) & 63;

    if (pr->op_rate && !(loop % pr->op_rate)) { /* RDT changes */
      read_real_time(&rdt_spent, TIMEBASE_SZ);
      time_base_to_time(&rdt_spent, TIMEBASE_SZ);
      secs = rdt_spent.tb_high - rdt_oprate.tb_high;
      nsecs = rdt_spent.tb_low - rdt_oprate.tb_low;
      if (nsecs < 0) {
        secs--;
        nsecs += 1000000000;
      }
      if (secs < 1) {
        nsecs = 1000000000 - nsecs;
        sleep_timer.tv_sec = 0;
        sleep_timer.tv_nsec = nsecs;
        nsleep(&sleep_timer, 0);
        read_real_time(&rdt_spent, TIMEBASE_SZ);
        time_base_to_time(&rdt_spent, TIMEBASE_SZ);
      }
      else {
        sprintf(msg, "op rate for rule %s is slow...\n", pr->rule_id);
        user_msg(ps, 0, INFO, msg);
      }
      read_real_time(&rdt_oprate, TIMEBASE_SZ);
      time_base_to_time(&rdt_oprate, TIMEBASE_SZ);
    }

    if (pr->rule_time) {
      read_real_time(&rdt_present, TIMEBASE_SZ);
      time_base_to_time(&rdt_present, TIMEBASE_SZ);
      secs = rdt_present.tb_high - rdt_start.tb_high;
      nsecs = rdt_present.tb_low - rdt_start.tb_low;
      if (nsecs < 0) {
        secs--;
        nsecs += 1000000000;
      }
      if (secs >=  (pr->rule_time)) {
        sprintf(msg,"Time spent(%d) in rule %s and rule time(%d), present(%u, %u) \n", secs, pr->rule_id, pr->rule_time, rdt_present.tb_high, rdt_present.tb_low);
        strcat(msg, "Rule finished....\n");
        user_msg(ps, 0, INFO, msg);
        break;
      }
    }
/* changes for accoustics */
    if(accous_rules)
    {
      current_time = time(0);
      if((start_time + pr->stanza_time) <= current_time)
      {
        sprintf(msg,"Time out... current time is %d rule start time is %d stanza time is %d",current_time,start_time,pr->stanza_time);
        user_msg(ps, 0, INFO, msg);
        break;
      }
    printf("accoustics: inside timeout if\n");
    }

  } /* end for (loop=1 ; loop <= pr->num_oper ; loop++) */
/* changes for accoustics */
  if(accous_rules)
  {
    if (semop(semid,&semops_inc[0],1) == -1)
    {
      sprintf(msg,"semop failed with -1 errno is %d\n",errno);
      user_msg(&s, 0, INFO, msg);
    } 
    if (semop(semid,&semops_dec_sem2[0],1) == -1)
    {
      sprintf(msg,"semop failed with -1 errno is %d\n",errno);
      user_msg(&s, 0, INFO, msg);
    } 
    if (semop(semid,&semops_cmp_sem2[0],1) == -1)
    {
      sprintf(msg,"semop failed with -1 errno is %d\n",errno);
      user_msg(&s, 0, INFO, msg);
    } 
    /*else 
    {
      sprintf(msg,"semval incremented\n");
      user_msg(&s, 0, INFO, msg);
    }*/ 
    /*sprintf(msg,"stanza completed.\n");
    user_msg(&s, 0, INFO, msg);*/
    proc_stage = 0;
  }
  if ( pr->timed == 'Y' ) {
     (void) time(&timer2);
     timer3 = (double) timer2 - (double) timer1;
     sprintf(tmpmsg, "\nStanza %s: ", pr->rule_id);
     strcpy(msg, tmpmsg);
     if ( e_notation(timer3, tim) ) {
        sprintf(tmpmsg, "Unable to convert Time (%.10e) into engineering "
                        "notation\n", timer3);
        strcat(msg, tmpmsg);
        if ( e_notation(total_bytes, tot) )
           sprintf(tmpmsg, "\nUnable to convert Total Bytes (%.10e) into "
                           "engineering notation\n", total_bytes);
        else
           sprintf(tmpmsg, "Total Bytes = %s\n", tot);
     } else {
        sprintf(tmpmsg, "Total Time = %s;  ", tim);
        strcat(msg, tmpmsg);
        if ( e_notation(total_bytes, tot) )
           sprintf(tmpmsg, "\nUnable to convert Total Bytes (%.10e) into "
                           "engineering notation\n", total_bytes);
        else
           sprintf(tmpmsg, "Total Bytes = %s\n", tot);
     }
     strcat(msg, tmpmsg);
     if ( timer3 >= 1.0 ) {
        timer3 = total_bytes / timer3;
        if ( e_notation(timer3, tim) )
           sprintf(tmpmsg, "\nUnable to convert Average Bytes (%.10e) into "
                           "engineering notation\n", timer3);
        else
           sprintf(tmpmsg, "Stanza averaged %s bytes per second transfer\n",
                   tim);
        strcat(msg, tmpmsg);
     }
     user_msg(ps, 0, INFO, msg);
  }
/* changes for accoustics */
  if(accous_rules)
  {
    end_time = time(0);
    time_spent = end_time-start_time;
    transfer_rate = total_bytes_transfered/(unsigned long long)time_spent;
    len = sprintf (stanza_str,"Pass_no %d stanza_id %s transfer_rates(B/sec): W %llu R %llu Total %llu Stanza_time(sec) %d\n",read_rules_file_count,pr->rule_id,total_writes/(unsigned long long)time_spent,total_reads/(unsigned long long)time_spent,transfer_rate,pr->stanza_time);
    write (stats_fd,stanza_str,len);
  }
  hxfupdate(UPDATE, ps);                    /* update htx statistics        */
  mallocount_temp = mallocount-1;
  freebufs(&mallocount_temp, wptr, rptr);
  if ( (pr->num_oper != 0) && (strcmp(pr->oper, "BWRC") == 0) ) {
     sprintf(msg, "BackGround Write/Read Compare has FINISHED.\n");
     user_msg(ps, 0, INFO, msg);
  }

  /**************************************************************************
  * close disk,                                                             *
  **************************************************************************/
  while ( close(pr->fildes) == -1 );

  return(rc);
}
void
freebufs(int *pmallo, char *wptr[], char *rptr[])
{
  int i;

  for ( i = 0; i < *pmallo + 1; i++ ) {
    if ( wptr[i] != NULL ) {
       free(wptr[i]);
       wptr[i] = NULL;
    }
    if ( rptr[i] != NULL ) {
       free(rptr[i]);
       rptr[i] = NULL;
    }
  }
  *pmallo = 0;
}



/*****************************************************************************/
/* isVG_member() - checks to see if diskdev is a member of a volume group    */
/*                 currently defined in the ODM.                             */
/*                                                                           */
/* This is a little trickier than it might appear.  The following conditions */
/* must occur for it to pass the test:                                       */
/*     1) The ODM class CuAt contains a record with the keywords             */
/*        name = to the logical disk name and attribute = "pvid".            */
/*     2) The diskdev appears in the ODM class CuAt with the keywords        */
/*        attribute = "pv" and value = to the physical volume id (PVID)      */
/*        for that disk.                                                     */
/*     3) The same PVID must be found on the disk.                           */
/* This method accounts for many unusual situations which may occur in the   */
/* lab, like cloning disks and moving disks around without properly deleting */
/* them from the ODM.                                                        */
/*                                                                           */
/* Parameter:  diskdev - pathname to test device.                            */
/*             sysmsg - name of volume group that diskdev is in, or, a       */
/*             error message, or null, if the diskdev is not in a VG.        */
/*             msg_length - size in bytes of sysmsg.                         */
/* Returns: 0 - The disk is a member of a defined VG.                        */
/*          >0 - system error.                                               */
/*          <0 - not a member of a VG.                                       */
/*          -1 - No PV id found for disk in ODM.                             */
/*          -2 - No pv attribute found for disk in ODM.                      */
/*        6000 - LVM system error.                                           */
/*                                                                           */
/*****************************************************************************/
int isVG_member(unsigned char *diskdev, char *sysmsg, int msg_length)
{
  char   pvname[32], search_crit[80], *errmsg, pvid_str[33];
  int    term_rc, rc = 0;
  struct CuAt CuAt_obj, *p_CuAt;
  struct querypv *p_qpv;	/* querypv structure */
  struct unique_id pv_id, vg_id;

  *sysmsg = '\0';
  strcpy( pvname, &(diskdev[6]) );

  /* init odm */
  rc = (int) odm_initialize();
  if (rc < 0) {
    odm_err_msg(odmerrno, (char **) &errmsg);
    strncpy(sysmsg, errmsg, msg_length);
    sysmsg[msg_length-1] = '\0';
      return(odmerrno);
  }

  /* build the query */
  /* Find the CuAt object corresponding to the pvid for that disk */
  sprintf(search_crit, "name = '%s' and attribute = 'pvid'", pvname);
  p_CuAt = odm_get_first(CuAt_CLASS, search_crit,&CuAt_obj);
  if ( (int)p_CuAt == -1 ) {
    odm_err_msg(odmerrno, (char **) &errmsg);
    strncpy(sysmsg, errmsg, msg_length);
    sysmsg[msg_length-1] = '\0';
    rc = odmerrno;
  } else if (p_CuAt == NULL)    /* no pvid found for disk device */
    rc = -1;

  if ( rc == 0 ) {
      /* convert pvid to lvm structure */
      strncpy(pvid_str, CuAt_obj.value, 16); /* only use first 16 digits */
      pvid_str[16]='\0';
      bzero(&pv_id, sizeof(pv_id));
      get_uniq_id(pvid_str, &pv_id);

      /* Find the CuAt object for that disk in a volume group */
      sprintf(search_crit, "value = '%s' and attribute = 'pv'", CuAt_obj.value);
      p_CuAt = odm_get_first(CuAt_CLASS, search_crit,&CuAt_obj);
      if ( (int)p_CuAt == -1 ) {
        odm_err_msg(odmerrno, (char **) &errmsg);
        strncpy(sysmsg, errmsg, msg_length);
        sysmsg[msg_length-1] = '\0';
        rc = odmerrno;
      } else if (p_CuAt == NULL)    /* no pv attribute with disk's pv_id */
        rc = -2;
  }

  term_rc = odm_terminate();
  if ( term_rc != 0 ) {
      strncat(sysmsg, "odm_terminate() Failed. ", msg_length - strlen(sysmsg)-1 );
      rc = rc == 0 ? odmerrno : rc;    /* don't clobber former bad rc */
  }

  if (rc != 0) return(rc);

  /* now check to see if the LVM agrees with what we found, this will */
  /* read the PVID off the disk.                                      */
  bzero(&vg_id, sizeof(vg_id));
  rc = lvm_querypv(&vg_id, &pv_id, &p_qpv, pvname);
  switch (rc) {
      case LVM_SUCCESS:    /* disk or could be a member of a VG */
      case LVM_BADBBDIR:
      case LVM_NOPVVGDA:
    free(p_qpv->pp_map);
    free(p_qpv);
    sprintf(sysmsg, "Device appears to be a member of the %s volume group.",
          CuAt_obj.name);
    rc = 0;
    break;
      case LVM_NOTVGMEM:   /* disk is not actually a member, ODM is screwed up  */
    break;
      default:             /* unexpected error */
    sprintf(sysmsg, "lvm_querypv() returned %d.", rc);
    rc = 6000;
    break;
  } /* switch */

  return(rc);

} /* isVG_member */

int get_dev_fn(uchar *dev_name)
{
   FILE *fp;
   char str[250];
   char pvname[50];
   char cmd[50];
   int  i, j , rc = 0;
   char sstring[256];              /* search criteria pointer      */
   struct Class *cusdev;           /* customized devices class ptr */

	for (i=0; i<10; i++)
	{
		lv_devices[i][0] = 0;
		PCI_parent[i][0] = 0;
		PCI_devfn[i] = 0;
	}

   if ( odm_initialize() == -1 ) {
      return(-1);
   }

   if ( (int)(cusdev = odm_open_class(CuDv_CLASS)) == -1 ) {
      odm_close_class(CuDv_CLASS);
      odm_terminate();
      return(-1);
   }

   sprintf(sstring, "name = %s", (ulong)dev_name);
   rc = (int)odm_get_first(cusdev, sstring, &cusobj);
   if ( rc == 0 ) {
      odm_close_class(CuDv_CLASS);
      odm_terminate();
      return(-1);
   } else if ( rc == -1 ) {
      odm_close_class(CuDv_CLASS);
      odm_terminate();
      return(-1);
   }

   if (strstr(cusobj.PdDvLn_Lvalue, "logical_volume"))
   {
      sprintf(cmd,"lslv -l %s",dev_name);
      fp = popen(cmd, "r");
      if (fp == NULL)
      {
         printf("popen fail...\n");
         exit(0);
      }


      i = 0;
      while (fgets(str, 250, fp) != NULL)
      {
         sscanf(str,"%s %*s",pvname);
         if (strstr(str,"hdisk"))
         {
            strcpy(lv_devices[i++],pvname);
         }
      }
   }
   else if (strstr(cusobj.PdDvLn_Lvalue, "disk"))
   {
      strcpy(lv_devices[0], dev_name);
   }
   else return -1;

   for (i=0; strlen(lv_devices[i]); i++)
   {
      sprintf(sstring, "name = %s", (ulong)lv_devices[i]);
      rc = (int)odm_get_first(cusdev, sstring, &cusobj);
      if ( rc == 0 ) {
         odm_close_class(CuDv_CLASS);
         odm_terminate();
         return(-1);
      } else if ( rc == -1 ) {
         odm_close_class(CuDv_CLASS);
         odm_terminate();
         return(-1);
      }
      sprintf(sstring, "name = %s", (ulong)cusobj.parent);
      rc = (int)odm_get_first(cusdev, sstring, &cusobj);
      if ( rc == 0 ) {
         odm_close_class(CuDv_CLASS);
         odm_terminate();
         return(-1);
      } else if ( rc == -1 ) {
         odm_close_class(CuDv_CLASS);
         odm_terminate();
         return(-1);
      }

			for (j=0; j<i; j++) {
				if ((!strcmp(PCI_parent[i], PCI_parent[j])) && (PCI_devfn[j] == PCI_devfn[i]))
					break;
			}
			strcpy(PCI_parent[j], cusobj.parent);
			PCI_devfn[j] = atoi(cusobj.connwhere);
   }

   if ( odm_close_class(cusdev) == -1 ) {
      odm_terminate();
      return(-1);
   }
   odm_terminate();

   return 0;
}

int get_lun(struct htx_data *ps, uchar *dev_name)
{
   int  i, rc;
   char sstring[256];              /* search criteria pointer      */
   struct Class *cusdev;           /* customized devices class ptr */
   char tmp[20];

   if ( odm_initialize() == -1 )
      return(-1);
   if ( (int)(cusdev = odm_open_class(CuDv_CLASS)) == -1 ) {
      odm_close_class(CuDv_CLASS);
      odm_terminate();
      return(-1);
   }
   sprintf(sstring, "name = %s", (ulong)dev_name);
   rc = (int)odm_get_first(cusdev, sstring, &cusobj);
   if ( rc == 0 ) {
      odm_close_class(CuDv_CLASS);
      odm_terminate();
      return(-1);
   } else if ( rc == -1 ) {
      odm_close_class(CuDv_CLASS);
      odm_terminate();
      return(-1);
   }
   if ( odm_close_class(cusdev) == -1 ) {
      odm_terminate();
      return(-1);
   }
   odm_terminate();

   strcpy(tmp, cusobj.connwhere );
   tmp[strlen(cusobj.connwhere)] = '\0';

        /*
         * The tmp string contains a string of the form 10,0
         * So, the code snippet below tries to get the scsi id
         * and lun id from connwhere and then get the scsi id
         * and lun id of its parent. The parent id's will be used
         * only in case of pass-thru commands implemented in
         * read_verify_disk and write_verify_disk functions in
         * io_oper.c file. See defect 388686 for details.
         */

   i = 0;

   do{
        i++;
     }while( isdigit(tmp[i-1]) );

   lun_id = atoi(&tmp[i]);
     i = -1;

   do{
        i++;
        tmp[i] = cusobj.connwhere[i];
     }while(  isdigit(cusobj.connwhere[i+1])  );
        tmp[i+1] = '\0';
   scsi_id = atoi(tmp);

#ifndef __HTX43X__
   strcpy(disk_parent, cusobj.parent);
   disk_parent[strlen(cusobj.parent) ] = '\0';

   get_parent_lun(cusobj.parent);
#endif

   return(rc);
}

#ifndef __HTX43X__
int get_parent_lun(uchar *dev_name)
{
   int  i, rc;
   char sstring[256];              /* search criteria pointer      */
   struct Class *cusdev;           /* customized devices class ptr */
   char tmp[20];

   if ( odm_initialize() == -1 )
      return(-1);
   if ( (int)(cusdev = odm_open_class(CuDv_CLASS)) == -1 ) {
      odm_close_class(CuDv_CLASS);
      odm_terminate();
      return(-1);
   }
   sprintf(sstring, "name = %s", (ulong)dev_name);
   rc = (int)odm_get_first(cusdev, sstring, &cusobj);
   if ( rc == 0 ) {
      odm_close_class(CuDv_CLASS);
      odm_terminate();
      return(-1);
   } else if ( rc == -1 ) {
      odm_close_class(CuDv_CLASS);
      odm_terminate();
      return(-1);
   }
   if ( odm_close_class(cusdev) == -1 ) {
      odm_terminate();
      return(-1);
   }
   odm_terminate();

   strcpy(tmp, cusobj.location );
   tmp[strlen(cusobj.location)] = '\0';

   i = 0;

   do{
        i++;
     }while( isdigit(tmp[i-1]) );

   lun_id_parent = atoi(&tmp[i]);

     i = -1;

   do{
        i++;
        tmp[i] = cusobj.location[i];
     }while(  isdigit(cusobj.location[i+1])  );
        tmp[i+1] = '\0';
   scsi_id_parent = atoi(tmp);
   return(0);
}
#endif

/******************************************************************************
 * Routine to generate a string representation of a number in a format similar
 * to engineering notation where the exponent is always a power of 3.  This
 * routine is limited to positive numbers.  It does not round the number based
 * on precision; instead it truncates digits past the precision.  It doesn't
 * require logarithms so runs pretty fast.  Best suited for numbers greater
 * than 999.
 ******************************************************************************/
int e_notation(double d, char *s)
{
  #define DIGITS 4                                     /* Sets the precision */
  int i, j, k, dig;

  if ( d < 0.0 ) {
     *s = '\0';
     return(-1);
  }
  if ( d < 1000.0 )
     sprintf(s, "%.3f", d);
  else {
    /**************************************************************************
     * The number is >= 1000 so we need to convert to engineering notation.
     * First, convert the number to a string, figure out how many digits it
     * has, put the decimal place in the right place after shifting rightmost
     * digits to the right one place, add the proper exponent, and we're done.
     **************************************************************************/
    sprintf(s, "%.0lf", d);
    i = strlen(s);
    dig = (i < DIGITS) ? i : DIGITS;
    j = ((i - 4) % 3) + 1;
    for ( k = dig - 1; k >= j; k-- )
       s[k+1] = s[k];
    s[j]='.';
    s[dig+1]='e';
    sprintf(s+dig+2, "%d", ((i - dig) / 3 + 1) * 3);
  }
  return(0);
}
