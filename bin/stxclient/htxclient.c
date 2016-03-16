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

/* @(#)43	1.9  src/htx/usr/lpp/htx/bin/stxclient/htxclient.c, eserv_gui, htxubuntu 5/24/04 17:06:31 */

#include "hxssup.h"
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

char ERR_WRAP[8];		/* htxerr wrap keyword               */
char HTXPATH[50];		/* HTX file system path spec.        */
char HTXSCREENS[50];		/* HTX file system path spec.        */
char MAX_ERR_SAVE[16];		/* htxerr.save max file size         */
char MAX_ERR_THRES[16];		/* error log file wrap threshold.    */
char MAX_MSG_SAVE[16];		/* htxmsg.save max file size         */
char MAX_MSG_THRES[16];		/* message log file wrap threshold.  */
char MSG_WRAP[8];		/* htxmsg wrap keyword               */
char level_str[80];		/* HTX and LINUX levels */
char obuf[BUFSIZ];		/* buffer for stdout                 */
char *program_name;		/* the name of this program (argv[0]) */
char save_dir[PATH_MAX + 1];	/* original current directory   */
char stress_cycle[32];		/* # of seconds between "heartbeats" */
char stress_dev[64];		/* "heartbeat" device for stress test */

int alarm_sig = FALSE;		/* set to TRUE on receipt of SIGALRM */
int editor_PID;			/* editor process id             */
int HANG_MON_PERIOD = 0;	/* Hang monitor period (0=no mon)  */
int hft_flag = FALSE;		/* set to TRUE if run on hft display */
int MAX_ADDED_DEVICES = 0;	/* set from htx_profile          */
int msgqid = -1;		/* Message Log message queue id      */
int profile_emc_mode;		/* for build_ipc()                  */
int semhe_id = -1;		/* HE semaphore id                   */
int semot_id = -1;		/* Order Table semaphore id          */
int shm_id = -1;		/* system shared memory id           */
int shutd_wait = SD_DEFAULT;	/* time from SIGTERM to SIGKILL */
int slow_shutd_wait = SLOW_SD_DEFAULT;	/* time from SIGTERM to SIGKILL */
int system_call = FALSE;	/* set to TRUE before any system() */
int sig_end_flag = 0;
int run_type_disp = 1 ;
int init_screen_done =0;

struct tm start_time;		/* System startup time.            */

unsigned int max_wait_tm = 0;	/* maximum semop wait time         */
char *server_ip;

void sigint_handler(int sig, int code, struct sigcontext *scp);

int	main(int argc, char *argv[])
{
	extern int mmenu(int);

	char work_dir[PATH_MAX + 1];	/* string for chdir command      */
	int autostart;		/* auto-startup flag */
	int rc = 0;		/* return code                       */

   struct sigaction sigvector;

   sigemptyset(&(sigvector.sa_mask)); /* do not block signals       */
   sigvector.sa_flags = 0;            /* do not restart system calls on sigs */
   sigvector.sa_handler = (void (*)(int)) sigint_handler;
   (void) sigaction(SIGINT, &sigvector, (struct sigaction *) NULL);

   if (argc != 2) {
      printf("Usage: htxclient <Server IP>\n");
      exit(0);
   }
	setpriority(PRIO_PROCESS, 0, -1);

#ifdef	SUPERUSER_ONLY
	if(getuid() != (uid_t) 0)  {
		fprintf(stderr, "HTX: %s process being executed by non-superuser, quitting ... have a nice day !\n", argv[0]);
		/*
		 * Process not being executed by superuser ... retval = 2
		 */
		exit(2);
	}
#endif

	/*
	 * Make sure that we're the only copy of hxssup running...
	 * */

	program_name = argv[0];	/*  setup program_name variable.     */
	setbuf(stdout, obuf);	/*  dedicate static buffer to stdout */


#ifdef  __HTX_LINUX__
        strcpy(HTXPATH, "/usr/lpp/htx");
        /*if(strlen(strcpy(HTXPATH, getenv("HTXPATH"))) == 0)  {
                 strcpy(HTXPATH, "/usr/lpp/htx");
        }*/
#else
        if(strlen(strcpy(HTXPATH, getenv("HTXPATH"))) == 0)  {
                 strcpy(HTXPATH, "/usr/lpp/htx");
        }
#endif

#ifndef __HTX_LINUX__
        getwd(save_dir);        /*  get current directory                    */
#else
        strcpy(save_dir,"/usr/lpp/htx");
#endif

        strncpy(work_dir, HTXPATH, PATH_MAX + 1);
        strncat(work_dir, "/bin", PATH_MAX + 1);
        chdir(work_dir);        /*  change current directory                 */

        //htx_scn();                      /* display htx logo screen */
	/*
	 * get HTXPATH environment variable
	 */
  server_ip = (char *)malloc(strlen(argv[1])+1);
  //server_ip = (char *)malloc(20);
  strcpy(server_ip,argv[1]);
  setup_client();
  rc = mmenu(0);

  CLRLIN(MSGLINE, 0);
  clear();                /*  clean up screen               */
  refresh();              /*  update screen                 */
  endwin();               /*  end CURSES */
  exit(0);
}				/* main() */



void sigint_handler(int sig, int code, struct sigcontext *scp)
{

   printf("Interrupt Received... Client will be shutting down\n");
   fflush(stdout);
   //getch();

   if (init_screen_done) {
    CLRLIN(MSGLINE, 0);   
    clear();                /*  clean up screen               */
    refresh();              /*  update screen                 */
    endwin();               /*  end CURSES */
   }
    exit(0);

}
