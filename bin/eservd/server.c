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



#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <dirent.h>
#include <hxiipc.h>
#include "eservd.h"
#include "global.h"
#ifdef __OS400__
#include "a2e.h"
#include <qshell.h>
#define system QzshSystem
#endif
#define MYPORT 3491             // the port users will be connecting to
#define NOTPORT 3492            // the port users will be connecting to
#define MAXBUFLEN 100
#define BACKLOG 10

#ifdef __OS400__               /* 400 */
extern int truncate( const char *, off_t);
extern int mkstemp( const char * );
#endif

#define MAX_DISP 34
#define MAX_DISP_2 14
#define MAX_DISP_9 13
#define MAX_ENTRIES (rem_shm_addr->cur_shm_addr)->max_entries
#define NUM_ENTRIES (rem_shm_addr->cur_shm_addr)->num_entries
#define STX_RETURN(z) stx_free(); return(z)

void send_scn_5_data (int);
void send_scn_2_data (int);
void send_scn_9_data (int);
void init_shm (void);
void header_scn (char *, int, int, char *);
void send_data (int, int, int, int, char *);
int send_broadcast(char *, char *, char *, int );
int get_time(long, char *, char *, char *);
int receive_alld (int sock, char *numbytes);
int send_err_msg(int , char *, int );
int process_msg (int fd_num);
int parsefile (int , int *, char data[][80], int , char *, int , int );
int transfer_file (int , char *, char *, char *, char *, int );
int init_ecg_info(void);
void stxgui_getecginfo_hdr(char *file_xfer, char *tmpstr);
extern int ecg_summary (char *, int );
extern int H_device(struct htxshm_HE, char *);
extern int query_all(char *, char **, int , int *);
extern int ecg_get(void);
extern int user_ecg_get(char *, char *);
extern int parse_cmd ( char **, int *num);
extern int find_nap_enable();
int init_ecg (char *);
int is_statready();
//char** parsefile(int *);
extern int init_rem_ipc (char *);
extern int orig_list (void);
extern tmisc_shm *rem_shm_addr;
int good_cmd = 1;
char reg_exp[10], in_ecg_name[56];
char curr_client_ip[20];
int is_ssm = FALSE;
char *commands[512];

#define SIZE_SCN  100
#define SIZE_HDR  1024

char result_msg[10240];
int limit_exceed = 0;
int stx_error_no = 0;

void
sigterm_handler (int signal_number, int code, struct sigcontext *scp)
{
  DBTRACE(DBENTRY,("enter server.c sigterm_handler\n"));
  print_log (LOGMSG,"sigternm received \n");
  fflush (stdout);
  shutdown_flag = TRUE;
  DBTRACE(DBEXIT,("return server.c sigterm_handler\n"));
  return;
}

void
setup_server (void)
{
    //int lis_fd;
  int new_fd;                  // listen on sock_fd, new connection on new_fd
  struct sockaddr_in my_addr;  // my address information
  struct sockaddr_in their_addr;       // connector’s address information
    //struct sockaddr_in clt_addr; // connector’s address information
  int sin_size;
  int yes = 1, numbytes;
  int server_pid;
  fd_set master;
  fd_set read_fds;
  int fdmax, rc_select;
  struct timeval tval;
    //int fd_notify[100],fd_n;
    //struct hostent *he_clt;
  DBTRACE(DBENTRY,("enter server.c setup_server\n"));

  print_log (LOGMSG,"setup *** ");
  fflush (stdout);
  server_pid = getpid ();

  FD_ZERO (&master);
  FD_ZERO (&read_fds);

  print_log (LOGMSG,"socket ***");
  fflush (stdout);
  if ((listener = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
	print_log (LOGERR,"socket");
    DBTRACE(DBEXIT,("exit(1)/a server.c setup_server\n"));
    exit (1);
  }
  if (setsockopt (listener, SOL_SOCKET, SO_REUSEADDR, (void *) &yes, sizeof (int)) ==	-1) {
	print_log (LOGERR,"setsockopt");
    DBTRACE(DBEXIT,("exit(1)/b server.c setup_server\n"));
    exit (1);
  }
  print_log (LOGMSG,"done socket: %d ***\n", listener);
  fflush (stdout);
  my_addr.sin_family = AF_INET;        // host byte order
  my_addr.sin_port = htons (MYPORT);   // short, network byte order
  my_addr.sin_addr.s_addr = INADDR_ANY;        // automatically fill with my IP
  memset (&(my_addr.sin_zero), '\0', 8);       // zero the rest of the struct

  if (bind (listener, (struct sockaddr *) &my_addr, sizeof (struct sockaddr)) == -1) {
	print_log (LOGERR,"bind");
    DBTRACE(DBEXIT,("exit(1)/c server.c setup_server\n"));
    exit (1);
  }

  if (listen (listener, BACKLOG) == -1) {
	print_log (LOGERR,"listen");
    DBTRACE(DBEXIT,("exit(1)/d server.c setup_server\n"));
    exit(1);
  }

   /*if ((lis_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	  print_log(LOGERR,"socket");
      exit(1);
      }
      if (setsockopt(lis_fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
	  print_log(LOGERR,"setsockopt");
      exit(1);
      }
	  print_log(LOGMSG,"done socket \n");
      fflush(stdout);
      my_addr.sin_family = AF_INET; // host byte order
      my_addr.sin_port = htons(NOTPORT); // short, network byte order
      my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
      memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

      if (bind(lis_fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
	  print_log(LOGERR,"bind");
      exit(1);
      }

      if (listen(lis_fd, BACKLOG) == -1) {
	  print_log(LOGERR,"listen");
      exit(1);
      } */

/****************************************************/

  FD_SET (listener, &master);
  fdmax = listener;
  fd_num = 0;
  rc_select = 0;
  /***************************************/
  strcpy (bcast_msg, "INACTIVE");
  sprintf (msg_type, "START");
  bcast_done = FALSE;

  get_time (-1, dt, dy, tmm);
  sprintf (daemon_start_time, "%s  %s", dt, tmm);
  /*if (!sig_handle_done) {
     set_sig_handler();
     //sig_handle_done = 1;
  }*/


  /***************************************/

  print_log
	  (LOGMSG,"**************** Starting STX Daemon **%s*******************\n\n\n",
       daemon_start_time);
  do {
    read_fds = master;

    if (autostart) {
	  print_log (LOGMSG,"****************  autostart set *********************\n");
	  print_log (LOGMSG,"**** starting ecg.all by default ********************\n");
      fflush (stdout);
      process_msg (0);
      autostart = 0;
      continue;
    }

	print_log
	(LOGMSG,"**************** in select %d bcast = %d*********************\n",
	 fd_num, bcast_done);
    fflush (stdout);
    do {
      errno = 0;

      tval.tv_sec = 600;
      tval.tv_usec = 0;

      if (ipc_done)
	bcast_done = bcast_done & rem_shm_addr->sock_hdr_addr->bcast_done;

	  print_log (LOGMSG,"bcast_done = %d***", bcast_done);
      fflush (stdout);
      if ((bcast_done == TRUE) || (good_cmd == 0))
	rc_select = select (fdmax + 1, &read_fds, NULL, NULL, NULL);
      else {
	if (fd_num != listener) {
	  print_log (LOGMSG,"Calling send_broadcast\n");
	  fflush (stdout);
	  //send_broadcast (bcast_msg, "some time", "some time", 0);

	  /* The send_broadcast function was causing the stxclient to time out. Since this function was used in SSM and as SSM is currently
	     not being used, the call to this function is commented */

	  print_log (LOGMSG,"Called send_broadcast\n");
	  fflush (stdout);
	}
		//send_unicast(0);
	rc_select = select (fdmax + 1, &read_fds, NULL, NULL, &tval);
      }

    } while ((rc_select == -1) && (errno == EINTR));

    for (fd_num = 0; fd_num <= fdmax; fd_num++) {
      if (FD_ISSET (fd_num, &read_fds)) {
		//bcast_done = TRUE;
	print_log (LOGMSG,"out of select %d***", fd_num);
	fflush (stdout);

	print_log (LOGMSG,"Received %d bytes from select %d\n", rc_select, fd_num);
	fflush (stdout);
	if (fd_num == listener) {
	  sin_size = sizeof(struct sockaddr);
	  memset((void*)&their_addr, 0, sizeof(struct sockaddr));
	  if ((new_fd = accept (listener,
				(struct sockaddr *) &their_addr,
				&sin_size)) == -1) {
		print_log (LOGERR,"accept");
	    continue;
	  }
	  else {
	    FD_SET (new_fd, &master);
	    if (new_fd > fdmax)
	      fdmax = new_fd;
	  }
	  print_log (LOGMSG,"server: got connection from %s\n",
		  inet_ntoa (their_addr.sin_addr));
	  fflush (stdout);
  	  strcpy(curr_client_ip, inet_ntoa(their_addr.sin_addr));
  	  print_log(LOGMSG,"curr_client_ip = %s. conn from = %s new = %d old = %d\n", curr_client_ip, inet_ntoa(their_addr.sin_addr), fd_num, listener);

		 /***********************************************/
	       /*if ((fd_notify[new_fd] = accept(lis_fd, (struct sockaddr *)&their_addr,
		  &sin_size)) == -1) {
		  print_log(LOGERR,"accept");
		  }

		  print_log(LOGMSG,"server: got another connection from %s\n",
		  inet_ntoa(their_addr.sin_addr));
		  fflush(stdout); */

		 /***********************************************/
	}
	else {
	  memset ((void *)&msg_rcv, 0, sizeof (thtx_message));
	  if ((numbytes = receive_alld (fd_num, (char*)&msg_rcv)) == -1) {
		print_log
		(LOGERR," Received -1. Closing socket %d. errno = %d (%s)\n",
		 fd_num, errno, strerror (errno));
	    fflush (stdout);
		print_log (LOGERR,"recv");
	    FD_CLR (fd_num, &master);
	    close (fd_num);
	  }
	  else if (numbytes == 0) {
		print_log (LOGERR,"Received Zero bytes: Closing the connection\n");
		fflush (stdout);
		close (fd_num);
		FD_CLR (fd_num, &master);
	  }
	  else {
			//print_log(LOGMSG,"processing %d \n",numbytes); fflush(stdout);
	    process_msg (fd_num);
		print_log
		(LOGMSG,"********************processed******************** \n\n\n");
		  /*if (notify_send.cmd != 0) {
		     for (fd_n = 0; fd_n <= fdmax; fd_n++) {
		     if (FD_ISSET(fd_n, &master)) {
		     if ((fd_n != fd_num) && (fd_n != listener) && (strcmp(msg_rcv.str,"java") != 0)) {
			 print_log(LOGMSG,"sending notification to %d\n",fd_notify[fd_n]);
		     fflush(stdout);
		     if (send(fd_notify[fd_n],&notify_send,100, 0) == -1)
			 print_log(LOGERR,"send");
		     }
		     }
		     }
		     } */
	  }                //else
	}
	break;
      }
	 /*else {
		print_log(LOGMSG,"Timed out? Broadcast again\n");
	    bcast_done = FALSE;
	    break;
	    } */
    }

  } while (shutdown_flag == FALSE);    // outer while

   /* if SIGTERM was received, kill parent, hangmon and the suicide */
  if (shutdown_flag == TRUE) {
	print_log (LOGMSG,"Received shutdown request. hang_mon = %d\n", hang_mon_PID);
    fflush (stdout);
    kill (server_pid, SIGTERM);
    kill (hang_mon_PID, SIGKILL);
    end_it (0);
    shutdown_flag = FALSE;
  }
  DBTRACE(DBEXIT,("exit(0) server.c setup_server\n"));
  exit(0);

	//print_log(LOGMSG,"sflag= %d\n",shutdown_flag); fflush(stdout);

  print_log (LOGMSG,"returning from server\n");
  fflush (stdout);
  DBTRACE(DBEXIT,("return server.c setup_server\n"));
  return;
}

int
process_msg (int new_fd)
{
  int numbytes, bad_input, main_loop_count, nap_enabled=0;
  int a, b, c, size, fileid, rc, max_disp;
  int rc_stat, num_cmds, i, xfer_size, temp_var;
  int num_cmddata, cnt, tmp, num_entries;
  struct sigaction sigvec;
  char buf1[80], str[512], res_msg[80],strlog[80];
  char *file_xfer = NULL, *tmpstr = NULL, xfer_path[40], get_put[4], msgptr[512];
  char run_cmd[512], *msg_tmp;
  char cmd_data[80][80], *cmd_host = NULL, *ecg_ptr1 = NULL;
  struct htxshm_HE add_HE_struct;
  struct stat daemon_log_stat;
  char *tmp1,*tmp2,tmp_ecgname[64];
  char xfer_name[80], *xfer_mode, *stat_workstr;
  static int chk;
  char mycommand[512];
  char mydate[25];
  FILE *fp;
  int isRunningECGOnBackground = FALSE;
  int commandListIndex;
  char currentlyRunningECGName[80];
  char *newECGFileName; 
  long ecg_run_time;
  long device_cycle_count;
  char active_ecg_full_name[128];
  struct htxshm_HE  *p_shm_HE_temp;
  int HE_count;
   

  DBTRACE(DBENTRY,("enter server.c process_msg\n"));

 /*
  msgptr = (char *)malloc(512);
  if(msgptr == NULL) {
	print_log (LOGERR,"malloc to msgptr failed\n");
	return (-1);
  }
i*/
  stx_error_no = 0;

  print_log (LOGMSG,"String = %s msgpointer = 0x%x\n", (char *) &msg_rcv, (int)&msg_rcv);
  fflush (stdout);

  a = 0;
  b = 0;
  c = 0;
  if (autostart) {
    a = SCREEN_1;
    b = 1;
    c = 1;
    if (strcmp (start_ecg_name, "")) {
	  print_log (LOGMSG,"ecg %s specified, lets use it\n", start_ecg_name);
      strcpy (buf1, start_ecg_name);
    }
    else {
	  print_log (LOGMSG,"ecg not specified, lets use ecg.all\n");
	  sprintf (buf1, "/usr/lpp/htx/mdt/mdt.all");
    }
	sprintf ((char *) &msg_rcv, "%04x%04x%04x%s", a, b, c, buf1);
  }
  else {
    memset (buf1, 0, 80);
  }
  sscanf ((char *) &msg_rcv, "%04x%04x%04x%s", &a, &b, &c, buf1);
  if (!((a == CMDLINE) || (a == FILEONHOST) || (a == FILEONSSM)))
    strcpy (buf1, (char *) (((char *) &msg_rcv) + 12));
   /*print_log (LOGMSG,"buf=%s:cm=%s: 1=%c: :len = %d\n", buf1,
	   (char *) ((&msg_rcv) + 12), ((char *) ((&msg_rcv) + 12))[0],
	   strlen (buf1));*/
   /* print_log above may crash the daemon */

  if (strncmp ((char *) &msg_rcv, "WASACKMESG", 10) == 0) {
    bcast_done = TRUE;
    if (ipc_done)
      rem_shm_addr->sock_hdr_addr->bcast_done = TRUE;
    DBTRACE(DBEXIT,("return/a server.c process_msg\n"));
    STX_RETURN (-1);
  }
  else {
	print_log (LOGMSG,"NOT ACKNOWLEDGEMENT\n");
    fflush (stdout);
  }

  print_log (LOGMSG,"command = %d\n", a);
  fflush (stdout);

  is_ssm = FALSE;
  is_cmdline = FALSE;
  num_cmddata = 1;             /* By default only one command has to be run */
  if ((a == CMDLINE) || (a == FILEONHOST) || (a == FILEONSSM)) {
    if (a != FILEONSSM) {
      is_ssm = FALSE;
      is_cmdline = TRUE;
      sep_at = ' ';
      sep_nl = '\n';
    }
    else {
      is_ssm = TRUE;
      is_cmdline = TRUE;
	    //is_cmdline = FALSE;
      sep_at = '@';
      sep_nl = '*';
    }
	//print_log(LOGMSG,"Message from Commandline Client\n"); fflush(stdout);
	sprintf (xfer_name, "Send more\n");
#ifdef _a2e_h	/*400-EBCDIC*/
    ebcdic2Ascii(&xfer_name[0],&xfer_name[0],strlen(xfer_name));
#endif
    if (send (new_fd, xfer_name, strlen (xfer_name), 0) == -1) {      //send acknowledgment
	  print_log (LOGERR,"Error in send\n");
      DBTRACE(DBEXIT,("return/b server.c process_msg\n"));
      STX_RETURN (-1);
    }
    xfer_size = atoi (buf1);  //size of the command that is going to come in the next message
	print_log (LOGMSG,"To be received = %d bytes. cmd=0x%x\n", xfer_size, (int)cmd_host);
    fflush (stdout);
    cmd_host = (char *) malloc (xfer_size + 1);       //get the memory to keep the incoming command
    if (cmd_host == NULL) {
	  print_log (LOGERR,"Error in malloc for cmd_host. errno =%d (%s).", errno,
	      strerror (errno));
      fflush (stdout);
	  sprintf (result_msg, "Error in malloc for cmd_host. errno =%d (%s).",
	       errno, strerror (errno));
      send_err_msg (new_fd, result_msg, -1);
      DBTRACE(DBEXIT,("return/c server.c process_msg\n"));
      STX_RETURN (-1);
    }
    memset (&msg_rcv, 0, sizeof (thtx_message));
    memset (cmd_host, 0, xfer_size);
    do {
      numbytes = recv (new_fd, cmd_host, xfer_size, MSG_WAITALL);
    } while ((numbytes <= 0) && (errno == EINTR));
    if (numbytes <= 0) {
	  print_log
	  (LOGERR,"Error in receiving message from the client. Received = %d bytes. Errno = %d\n",
	   numbytes, errno);
	  print_log (LOGERR,result_msg,
		  "Error in receiving message from the client. Received = %d bytes. Errno = %d\n",
	      numbytes, errno);
      DBTRACE(DBEXIT,("return/d server.c process_msg\n"));
      STX_RETURN (-1);
    }

    cmd_host[xfer_size] = '\0';
#ifdef _a2e_h	/*400-EBCDIC*/
    numbytes = ascii2Ebcdic(&cmd_host[0],&cmd_host[0],numbytes);
#endif
	print_log (LOGMSG,"Received commmand is %s. size = %d\n", cmd_host, numbytes);
    fflush (stdout);
    strcpy ((char *) &msg_rcv, cmd_host);
	print_log (LOGMSG,"recieved string is:%s,\n", (char *)&msg_rcv);

    memset (cmd_data, 0, 80 * 80);
    parsefile (new_fd, &num_cmddata, cmd_data, a, cmd_host, xfer_size,
	       fileid);
	print_log (LOGMSG,"Done parsing file\n");
    fflush (stdout);
    if (cmd_host != NULL) {
	  print_log (LOGMSG,"Freeing cmd_host = 0x%x\n", (int)cmd_host);
      fflush (stdout);
      free (cmd_host);
      cmd_host = NULL;
	}
	print_log (LOGMSG,"num_cmddata = %d\n", num_cmddata);
	fflush (stdout);
	if (num_cmddata == -1) {  //If there was problem reading the file, just quit
	  print_log (LOGERR,"there was problem reading the file\n");
	  fflush (stdout);
	  DBTRACE(DBEXIT,("return/e server.c process_msg\n"));
	  STX_RETURN (-1);
    }
	sprintf (xfer_name, "%8x", (0 - num_cmddata));    //send the number of commands.
#ifdef _a2e_h	/*400-EBCDIC*/
    ebcdic2Ascii(&xfer_name[0],&xfer_name[0],strlen(xfer_name));
#endif
    if (!(a == FILEONSSM))
      if (send (new_fd, xfer_name, strlen (xfer_name), 0) == -1) {   // client will receive that many messages.
	print_log (LOGERR,"Error in send\n");
	fflush (stdout);
	DBTRACE(DBEXIT,("return/f server.c process_msg\n"));
	STX_RETURN (-1);
      }
  }
  else if (a == FILEONSUT) {
    is_cmdline = TRUE;
    sep_at = ' ';
    sep_nl = '\n';
    memset (cmd_data, 0, 80 * 80);
    parsefile (new_fd, &num_cmddata, cmd_data, a, buf1, -1, -1);      // Parse the file and read the commands in cmd_data
    if (num_cmddata == -1) {  //If there was problem reading the file, just quit
	  print_log (LOGERR,"there was problem reading the file\n");
      fflush (stdout);
      DBTRACE(DBEXIT,("return/g server.c process_msg\n"));
      STX_RETURN (-1);
    }
	sprintf (xfer_name, "%8x", (0 - num_cmddata));    //send the number of commands.
#ifdef _a2e_h	/*400-EBCDIC*/
    ebcdic2Ascii(&xfer_name[0],&xfer_name[0],strlen(xfer_name));
#endif
    if (send (new_fd, xfer_name, strlen (xfer_name), 0) == -1) {      // client will receive that many messages.
	  print_log (LOGERR,"Error in send\n");
      fflush (stdout);
      DBTRACE(DBEXIT,("return/h server.c process_msg\n"));
      STX_RETURN (-1);
    }
  }


   /* Run a loop for every command in the file. This loop will send the message to the client for each time it runs. */

  for (main_loop_count = 0; main_loop_count < num_cmddata; main_loop_count++) {        // MAIN FOR LOOP
    if ((a == FILEONSUT) || (a == FILEONHOST) || (a == FILEONSSM)) {
	  print_log (LOGMSG,"num_cmddata = %d cmd_data[%d] = %s\n", num_cmddata, main_loop_count,
	      cmd_data[main_loop_count]);
      fflush (stdout);
      strcpy ((char *) &msg_rcv, cmd_data[main_loop_count]);
    }
    if ((a == FILEONHOST) || (a == FILEONSUT) || (a == FILEONSSM)) {  // If <a single command from web> or <command file on SUT>
      if (strncmp ((char *) &msg_rcv, "esrv ", 5) != 0) {    //not an ESRV command
	//print_log(LOGMSG,"Command = %s\n", run_cmd); fflush(stdout);

	   /****************execute the command*******************/
	sigvec.sa_handler = SIG_IGN;
	sigaction (SIGCHLD, &sigvec, (struct sigaction *) NULL);
#ifndef __OS400__                           /* 400 */
	sigvec.sa_flags = SA_RESTART;
#endif

	sprintf (run_cmd, "echo  \"$ %s \" > /tmp/command_result",
		 ((char *) &msg_rcv));
	system_call = TRUE;
	system (run_cmd);
	system_call = FALSE;
	sprintf (run_cmd,
		 "%s 1>>/tmp/command_result 2>>/tmp/command_result",
		 ((char *) &msg_rcv));
	system_call = TRUE;
	system (run_cmd);
	system_call = FALSE;

	sigvec.sa_handler = (void (*)(int)) child_death;
	sigaction (SIGCHLD, &sigvec, (struct sigaction *) NULL);
#ifndef __OS400__                           /* 400 */
	sigvec.sa_flags = SA_RESTART;
#endif
	   /****************Done: executing the comamnd **********/

	   /****************Transfer the result file    **********/
	sprintf (get_put, "get");
	//xfer_mode = malloc;
	sprintf (xfer_path, "/tmp/");
	sprintf (xfer_name, "command_result");
	transfer_file (new_fd, get_put, "ascii", xfer_path, xfer_name, 0);
	   /**********Done: Transfer the result file    **********/
	print_log (LOGMSG,"WebCMDLINE result = %s\n", str);
	continue;
      }
      else {                 //it's an esrv command
	msg_tmp = strchr ((char *) &msg_rcv, '-');  //Remove esrv from the command string
	print_log (LOGMSG,"tmp Command : %s: fcmd = %s\n", msg_tmp,
		(char *) &msg_rcv);
	fflush (stdout);
	memset (str, 0, 512);
	strcpy (str, msg_tmp);
//print_log(LOGMSG,"str = %s len = %d\n", str, strlen(str));
	memset ((char *) &msg_rcv, 0, sizeof (thtx_message));
	strcpy ((char *) &msg_rcv, str);
	((char *) &msg_rcv)[strlen (str)] = '\0';
//print_log(LOGMSG,"str = %s len = %d\n", str, strlen(str)); fflush(stdout);
	print_log (LOGMSG,"Command parsed as: %s\n", (char *) &msg_rcv);
	fflush (stdout);
      }
    }

/*******************************/
    num_cmds = 0;
    num_stts = -1;
    num_actv = -1;
    num_suspend = -1;
    num_term = -1;
    num_rstrt = -1;
    num_query = -1;
    num_devcycles = 0;
    if (((char *) &msg_rcv)[0] == '-') {      //if it's a command line client
	//print_log(LOGMSG,"from command line\n"); fflush (stdout);
	 /*print_log (LOGMSG,"ecg[0] = %p\n", ecg[0]);
	 fflush (stdout);*/
      if (num_ecgs == 0)
	max_disp = init_ecg_info ();
      parse_cmd ( commands, &num_cmds);

	  print_log
	  (LOGMSG,"num_ecg = %d. num_cmds = %d msgcmd = %d num_ah = %d num_coe = %d num_dup = %d\n",
	   num_ecg, num_cmds, msg_rcv.cmd, num_add_ah, num_add_coe,
	   num_add_dup);
	  /* printf
	  ("num_ecg = %d. num_cmds = %d msgcmd = %d num_ah = %d num_coe = %d num_dup = %d\n",
	   num_ecg, num_cmds, msg_rcv.cmd, num_add_ah, num_add_coe,
	   num_add_dup); */
      fflush (stdout);
      if (msg_rcv.cmd == 2011){
	for (commandListIndex = 0; commandListIndex < num_cmds; ++commandListIndex){
	  if (strncmp (commands[commandListIndex], "nonblock", 8 ) == 0) {
		isRunningECGOnBackground = TRUE;   
		break;
	  }
	}
      }
      msg_rcv.subcmd = 0;

      ((thtx_message *) & msg_rcv)->indx = 0;

    }
    else {                    //if GUI client.
      sep_at = '@';
      sep_nl = '*';
      is_cmdline = FALSE;
      msg_rcv.cmd = a;
      msg_rcv.subcmd = b;
      msg_rcv.indx = c;
      strcpy (msg_rcv.str, buf1);
    }
/*******************************/
	print_log (LOGMSG,"CMD = %d SUBCMD = %d INDX= %d STRING = %s\n cmd_line = %d",
	    msg_rcv.cmd, msg_rcv.subcmd, msg_rcv.indx, msg_rcv.str,
	    is_cmdline);
    fflush (stdout);

    notify_send.cmd = 0;
	print_log (LOGMSG,"IPC_DONE = %d  ", ipc_done);

    switch (msg_rcv.cmd) {
      case SCREEN_4:
	    //case SHUTDOWN:
	if ((!is_cmdline) && (a != FILEONSSM) &&
	    (strlen (msg_rcv.str) != 0)) {  /* it is from gui */
	  num_ecg = 1;
	  ecg[0] = &(msg_rcv.str[0]);
	  print_log (LOGMSG," for shutdown case copied the ecg name to ecg[0]: %s\n", ecg[0]);
	  fflush (stdout);
	}
	break;
      case 2029:
      case 2012:
      case 2013:
      case SCREEN_9_A:
      case SCREEN_9_R_D:
      case SCREEN_9_T_D:
	tmpstr = strtok (msg_rcv.str, "@");
	print_log (LOGMSG,"input ecg_name = :%s:\n", tmpstr);
	fflush (stdout);
	strcpy (in_ecg_name, tmpstr);

	tmpstr = strtok (NULL, "@");
	print_log (LOGMSG,"Regex = :%s:\n", reg_exp);
	fflush (stdout);
	if (tmpstr != NULL)
	  strcpy (reg_exp, tmpstr);

	memset (msg_rcv.str, 0, 80);
	strcpy (msg_rcv.str, in_ecg_name);
	print_log (LOGMSG,"ecg_name = :%s: exp = :%s:\n", msg_rcv.str, reg_exp);
	fflush (stdout);

      case 2000:
      case 2009:
      case 2010:
      case 2020:
      case SCREEN_1:
      case 2011:
      case 2021:
      case SCREEN_2:
      case 2022:
      case 2032:
      case SCREEN_3:
      case 2023:
      case 2033:
		//case SCREEN_4:
      case SHUTDOWN:
      case SCREEN_5:
      case SCREEN_6:
      case SCREEN_9_R:
      case SCREEN_9_T:
      case 4000:
      case 9996:
      case 9997:
      case 9998:
      case 9999:
	      /**** ecg_info is not initialized initialize it ****/
	if (num_ecgs == 0)
	  max_disp = init_ecg_info ();

	 /*  getting the command and ecg specified */

	print_log (LOGMSG,"Num_ECGS = %d\n", num_ecgs);
	fflush (stdout);
	cur_ecg_pos = -1;
	rc = 0;
	if ((is_cmdline) && (num_ecg == 0)) {  /* user hasn't given any ecg from command line, use ecg.all */
	  print_log(LOGMSG,"Command line = %d num_ecg = %d\n",is_cmdline, num_ecg); fflush(stdout);
	  ecg[0] = (char *) stx_malloc (80);
	  if (ecg[0] == NULL) {       /* problem?                          */
		print_log
		(LOGERR,"####ERROR: Unable to malloc for ecg-0.###ERRNO = %d (%s)",
		 errno, strerror (errno));
		sprintf (result_msg,
		     "####ERROR: Unable to malloc for ecg-0.###ERRNO = %d (%s)",
		     errno, strerror (errno));
	    send_err_msg (new_fd, result_msg, -1);
	    DBTRACE(DBEXIT,("return/i server.c process_msg\n"));
	    STX_RETURN (-1);
	  }                   /* endif */
	  if ((msg_rcv.cmd == 2020) || (msg_rcv.cmd == 9996))
	    strcpy(ecg[0], " ");
	  else
	    strcpy (ecg[0], "/ecg.all");
	  num_ecg = 1;
	}
	else {                 /* user has given an ecg from command line or it is a gui */
	  print_log (LOGMSG,"is_cmdline:%d rcvd_cmd:%d str:%s,\n", is_cmdline,
		  msg_rcv.cmd, msg_rcv.str);
	  fflush (stdout);
	  if ((!is_cmdline) && (a != FILEONSSM) && (strlen (msg_rcv.str) != 0)) {  /* it is from gui */
	    if (msg_rcv.cmd == 2010) {       // Addecg command
	      num_ecg = 0;
	      ecg_ptr1 = strtok (msg_rcv.str, " ");
	      ecg[num_ecg] = malloc (60);
	      if (ecg[num_ecg] == NULL) {   /* problem?                          */
		print_log
			(LOGERR,"####ERROR: Unable to malloc for ecg[%d].###ERRNO = %d (%s)",
		     num_ecg, errno, strerror (errno));
		sprintf (result_msg,
			 "####ERROR: Unable to malloc for ecg[%d].###ERRNO = %d (%s)",
			 num_ecg, errno, strerror (errno));
		send_err_msg (new_fd, result_msg, -1);
		DBTRACE(DBEXIT,("return/j server.c process_msg\n"));
		STX_RETURN (-1);
	      }             /* endif */
	      strcpy (ecg[num_ecg++], ecg_ptr1);
	      while ((ecg_ptr1 = strtok (NULL, " ")) != NULL) {
		ecg[num_ecg] = malloc (60);
		if (ecg[num_ecg] == NULL) {        /* problem?                          */
		  print_log
			  (LOGERR,"####ERROR: Unable to malloc for ecg[%d].###ERRNO = %d (%s)",
		       num_ecg, errno, strerror (errno));
		  sprintf (result_msg,
			   "####ERROR: In While, Unable to malloc for ecg[%d].###ERRNO = %d (%s)",
			   num_ecg, errno, strerror (errno));
		  send_err_msg (new_fd, result_msg, -1);
		  DBTRACE(DBEXIT,("return/k server.c process_msg\n"));
		  STX_RETURN (-1);
		}          /* endif */
		strcpy (ecg[num_ecg++], ecg_ptr1);
	      }
	    }                // Done AddECG
	    else {           /* for others, copy msg_rcv.str to ecg[0] */
              if ( strlen(msg_rcv.str) != 0 ) {
	          num_ecg = 1;
				  //ecg[0] = (char *)malloc(strlen(msg_rcv.str));
	          ecg[0] = &(msg_rcv.str[0]);
				  //strcpy(ecg[0], msg_rcv.str);
			  print_log (LOGMSG,"copied the ecg name to ecg[0]: %s\n", ecg[0]);
	          fflush (stdout);
              } else {
	          num_ecg = 0;
	          ecg[0] = &(msg_rcv.str[0]);
			  print_log(LOGERR,"no ecg was specified with the command\n");
	          fflush (stdout);
              }
		active_ecg_full_name[0] = '\0';
		get_active_ecg_full_name(active_ecg_full_name);
		if(active_ecg_full_name[0] != '\0') {
			ecg[0] = active_ecg_full_name;
		}
	    }
	  }                   /* done for command line */
	}
	 /* got the command and ecg, let us load ecgs if not loaded */
    if(msg_rcv.cmd==2011){       /* Is this run command */

	   print_log(LOGMSG,"in if 2011 n unloaded :\n");
	   strcpy(path_for_ecgmdt,ecg[0]);    /* Copying full name to path_for_ecgmdt string */

	   max_disp = ecg_get();
	}
    if (strcmp (ecg[0], "/ecg.all") == 0) {        /* if ecg.all, load all the ecgs */
	  print_log (LOGMSG,"3rd loop is_cmdline:%d rcvd_cmd:%d :%s:,\n", is_cmdline,
		  msg_rcv.cmd, ecg[0]);
	  fflush (stdout);
	  cur_ecg_pos = 0;
	  for (cnt = 1; cnt < num_ecgs; cnt++) {
	    cur_ecg_pos = cnt;
	    /* below 2 mdts are used by equaliser to test thread transitioning" feature of P8.
	     * For testing this, nap should be enabled. i.e. value in file /proc/sys/kernel/powersave-nap
	     * should be non-zero( 1 - nap, 2 - sleep). Send error msg. if not set.        */
	     if ((msg_rcv.cmd == 2011) && ((strcmp(ECGNAME, "mdt.eq_th_trans_switch") == 0) || (strcmp(ECGNAME, "mdt.eq_th_trans_mix") == 0))) {
			 nap_enabled = find_nap_enable(result_msg);
			 if (nap_enabled == 0 || nap_enabled == -1) {
				 send_err_msg(new_fd, result_msg, -1);
				 STX_RETURN(-1);
			 }
		 }
		 if ( (msg_rcv.cmd == 2001  &&  msg_rcv.subcmd == 1) ||
		 (msg_rcv.cmd == 2002) || (msg_rcv.cmd == 2003) ||
		 (msg_rcv.cmd == 2011) || (msg_rcv.cmd == 2012) ||
		 (msg_rcv.cmd == 2022) || (msg_rcv.cmd == 2032) ||
		 (msg_rcv.cmd == 2013) || (msg_rcv.cmd == 2023) ||
		 (msg_rcv.cmd == 2033) || (msg_rcv.cmd == 2005) ||
		 (msg_rcv.cmd == 9999) ) {
	      if ((strcmp (ECGSTATUS, "UNLOADED") == 0)) {
				  //print_log(LOGMSG,"Calling init_ecg for %s/%s\n",ECGPATH,ECGNAME); fflush(stdout);
				  /* printf("In process_msg, Calling init_ecg for %s/%s\n",ECGPATH,ECGNAME); fflush(stdout); */
		rc = init_ecg (result_msg);
		if (rc == -1) {
		  sprintf(result_msg, "No ECG Directory");
		  send_err_msg (new_fd, result_msg, -1);
		  DBTRACE(DBEXIT,("return/l server.c process_msg\n"));
		  return -1;
		}
	      } // UNLOADED
	    } //msg_rcv.cmd
	  } //for num_ecgs
	  cur_ecg_pos = 0;
	} // if ecg.all
	else {                 /* not ecg.all, so load only specified ecg */
          cur_ecg_pos = -1;
	  for (tmp = 0; tmp < num_ecg; tmp++) {
		print_log (LOGMSG,"comparing ecg with ecg[%d]:%s, num_ecg in cmd : %d \n", tmp, ecg[tmp], num_ecg);
		print_log (LOGMSG,"path_for_ecgmdt in loop:%s,full_name:%s",path_for_ecgmdt,full_name);
	    if (msg_rcv.cmd == 2001 || msg_rcv.cmd == 2011) {
            sprintf(mycommand, "cp %s /usr/lpp/htx/mdt/mdt", ecg[tmp]);
            system_call = TRUE;
            system(mycommand);
            system_call = FALSE;
        }
	    for (cnt = 1; cnt < num_ecgs; cnt++) {
	      cur_ecg_pos = cnt;
	      PUT_FULL_ECG;
	      if (strcmp (ecg[tmp], full_name) == 0) {      // if any ecg is specified and matches with existing ecgs
	          print_log(LOGMSG,"IN test");
	          /* below 2 mdts are used by equaliser to test thread transitioning" feature of P8.
	           * For testing this, nap should be enabled. i.e. value in file /proc/sys/kernel/powersave-nap
	           * should be non-zero( 1 - nap, 2 - sleep). Send error msg. if not set.        */
	           if ((msg_rcv.cmd == 2011) && ((strcmp(ECGNAME, "mdt.eq_th_trans_switch") == 0) || (strcmp(ECGNAME, "mdt.eq_th_trans_mix") == 0))) {
				    nap_enabled = find_nap_enable(result_msg);
				    if (nap_enabled == 0 || nap_enabled == -1) {
						send_err_msg(new_fd, result_msg, -1);
						STX_RETURN(-1);
				 	}
		       }

		if ( (strcmp (ECGSTATUS, "UNLOADED") == 0)) {
		  if ( (msg_rcv.cmd == 2001  &&  msg_rcv.subcmd == 1) ||
		       (msg_rcv.cmd == 2002) || (msg_rcv.cmd == 2003) ||
		       (msg_rcv.cmd == 2011) || (msg_rcv.cmd == 2012) ||
		       (msg_rcv.cmd == 2022) || (msg_rcv.cmd == 2032) ||
		       (msg_rcv.cmd == 2013) || (msg_rcv.cmd == 2023) ||
		       (msg_rcv.cmd == 2033) || (msg_rcv.cmd == 2005) ||
		       (msg_rcv.cmd == 9999) ) {
			print_log(LOGMSG,"In if unloaded:");
			print_log (LOGMSG,"Calling init_ecg for %s/%s\n", ECGPATH,
			    ECGNAME);
		    fflush (stdout);
		    rc = init_ecg (result_msg);
		    if (rc == -1) {
		      sprintf(result_msg, "No ECG Directory");
		      send_err_msg (new_fd, result_msg, -1);
		      DBTRACE(DBEXIT,("return/m server.c process_msg\n"));
		      return -1;
		    }
		    else if (rc != -2) {
		      shm_addr.hdr_addr = ECGSHMADDR_HDR;
		      shm_addr.HE_addr = ECGSHMADDR_HE;
		      semhe_id = ECGSEMHEID;
		      break;
		    }
		  } //msg_rcv.cmd
		} // UNLOADED
		else if (strcmp (ECGSTATUS, "UNLOADED") != 0) {    // already loaded
		  shm_addr.hdr_addr = ECGSHMADDR_HDR;
		  shm_addr.HE_addr = ECGSHMADDR_HE;
		  semhe_id = ECGSEMHEID;
		}
		break;
	      }
	      else          // ecgname did not match
		cur_ecg_pos = -1;
	    }
	  }
	}

	if (cur_ecg_pos == -1) {       /* this from gui and ecg is not specified */
	    strcpy(tmp_ecgname, ecg[0]);
	    tmp1=dirname(tmp_ecgname);      /* Parsing fullname into ecg name and path of ecg (eg.ecg[0]will hv fullname) */
        strcpy(tmp_ecgname, ecg[0]);
        tmp2=basename(tmp_ecgname);     /* temp2 contains name of the file */
        print_log (LOGMSG,"In dirname loop");
          if ( !(msg_rcv.cmd == 2000  &&  msg_rcv.subcmd == 0)) {
	       for (cnt = 1; cnt < num_ecgs; cnt++) {
	          cur_ecg_pos = cnt;
	          if ((strcmp (tmp1, ECGPATH) == 0)
		      && (strcmp (tmp2, ECGNAME) == 0)) {
	              if (strcmp (ECGSTATUS, "UNLOADED") == 0) {
				   print_log (LOGMSG,"FOR:Calling init_ecg for  %s/%s\n", ECGPATH,
			           ECGNAME);
		           fflush (stdout);
		           rc = init_ecg (result_msg);
	              }
	              if (rc == -1) {
		          	sprintf(result_msg, "No ECG Directory");
		          	send_err_msg (new_fd, result_msg, -1);
	              } else if (rc == -2)
		          rc = -2;
	              else {
		          shm_addr.hdr_addr = ECGSHMADDR_HDR;
		          shm_addr.HE_addr = ECGSHMADDR_HE;
		          semhe_id = ECGSEMHEID;
		          break;
	              }
	          }
	      }
         } else {
			 print_log(LOGMSG,"since its displaying ecg, don't load any ecg\n");
             fflush(stdout);
         }
	}

	break;


      default:
	break;

    }


	print_log(LOGMSG,"Command to be processed is. %d shut = %d n4 = %d\n", msg_rcv.cmd, SHUTDOWN, SCREEN_4);

    switch (msg_rcv.cmd) {

      case 2000:

	rc = 0;
		//print_log(LOGMSG,"Option 0 selected: ipc_done = %d \n", ipc_done);
	memset (result_msg, 0, 80);
	bad_input = FALSE;
	sprintf (result_msg, "Command Completed Successfully");        // Initialize the message
	max_disp = ecg_get ();
	print_log (LOGMSG,"num_ecgs = %d\n", num_ecgs);
	fflush (stdout);
	if (max_disp == -1) {  // If no ecg directory found.
	  rc = -1;
	  print_log (LOGMSG,"no ecg directory\n");
	  fflush (stdout);
	  sprintf (result_msg, "/usr/lpp/htx/ecg directory not found \n");
	}
	else if (max_disp == -2) {     // if no new ecg file, send the old structure.
	  print_log (LOGMSG,"no new ecg file, send the old structure\n");
	  fflush (stdout);
	  rc = 0;
	  max_disp = num_ecgs;
	}
	else if (max_disp == -3) {     // if no new ecg file, send the old structure.
	  print_log (LOGMSG,"Error opening pipe in ecg_get...\n");
	  fflush (stdout);
	  rc = -3;
	  sprintf(result_msg, "Error opening pipe in ecg_get...");
	  send_err_msg (new_fd, result_msg, -1);
	  DBTRACE(DBEXIT,("return/n server.c process_msg\n"));
	  return -3;
	}

	if (msg_rcv.indx <= 0) {       // index is less than 0, display the first page
	  msg_rcv.indx = 0;
	}
	else if (msg_rcv.indx > num_ecgs) {    //index goes beyond the number of ecgs, display the first page
	  msg_rcv.indx = 0;
	}
	else if ((num_ecgs - msg_rcv.indx + 1) > 14) { // good case, there are plentyu of entries, display on 14
	  msg_rcv.indx = msg_rcv.indx - 1;
	  max_disp = 14;
	  sprintf (result_msg, "Send another page\n");
	}
	else if ((num_ecgs - msg_rcv.indx + 1) < 14) { // Only some entries are left, send them all
	  msg_rcv.indx = msg_rcv.indx - 1;
	  max_disp = num_ecgs - msg_rcv.indx + 1;
	}
	max_disp = (max_disp > 14) ? 14 : max_disp;
	print_log (LOGMSG,"preparing message for screen 0****");
	fflush (stdout);
	send_data (new_fd, msg_rcv.cmd, max_disp, rc, result_msg);
	break;

      case 2010:
	memset (result_msg, 0, 80);
	bad_input = FALSE;
	sprintf (result_msg, "Command Completed Successfully");        // Initialize the message
	print_log (LOGMSG,"adding %d num_ecgs\n", num_ecg);
	for (tmp = 0; tmp < num_ecg; tmp++) {
	  print_log (LOGMSG,"adding the  ecg[%d]:%s\n", tmp, ecg[tmp]);
	  max_disp = user_ecg_get (ecg[tmp], result_msg);
	  if (max_disp == -1) {       // If no ecg directory found.
	    rc = -1;
		print_log (LOGERR,"no ecg directory\n");
	    fflush (stdout);
		sprintf (result_msg,
		     "/usr/lpp/htx/ecg directory not found \n");
	  }
	}
	if (!is_cmdline) {
	  msg_rcv.indx = (num_ecgs / 14) * 14 + 1;
	  if (msg_rcv.indx <= 0) {    ////index is less than 0, display the first page
	    msg_rcv.indx = 0;
	  }
	  else if (msg_rcv.indx > num_ecgs) { //index goes beyond the number of ecgs, display the first page
	    msg_rcv.indx = 0;
	  }
	  else if ((num_ecgs - msg_rcv.indx + 1) > 14) {      // good case, there are plentyu of entries, display on 14
	    msg_rcv.indx = msg_rcv.indx - 1;
	    max_disp = 14;
		sprintf (result_msg, "Send another page\n");
	  }
	  else if ((num_ecgs - msg_rcv.indx + 1) < 14) {      // Only some entries are left, send them all
	    msg_rcv.indx = msg_rcv.indx - 1;
	    max_disp = num_ecgs - msg_rcv.indx + 1;
	  }
	  max_disp = (max_disp > 14) ? 14 : max_disp;
	}
	else
	  msg_rcv.indx = 0;
	print_log (LOGMSG,"preparing message for screen 0****");
	fflush (stdout);
	send_data (new_fd, msg_rcv.cmd, max_disp, rc, result_msg);
	if (!is_cmdline) {
	  for (tmp = 0; tmp < num_ecg; tmp++) {
	    free (ecg[tmp]);
	    ecg[tmp] = NULL;
	  }
	}
	break;

      case SCREEN_1:
	sprintf (result_msg, " ");

	if (msg_rcv.subcmd != 0) {
		//print_log(LOGMSG,"calling AH_system ECG = %s/%s\n",ECGPATH,ECGNAME);fflush(stdout);
	  sprintf (res_msg, " ");
	  if (strcmp (msg_rcv.str, "/ecg.all") == 0) {
	    for (cnt = 1; cnt < num_ecgs; cnt++) {
	      cur_ecg_pos = cnt;
	      if (ECG_MAX_ENTRIES > 0) {
		shm_addr.hdr_addr = ECGSHMADDR_HDR;
		shm_addr.HE_addr = ECGSHMADDR_HE;
		semhe_id = ECGSEMHEID;
		print_log
			(LOGMSG,"calling AH_system for %s/%s:\n shmkey:%d semkey:%d\n",
		     ECGPATH, ECGNAME, ECGSHMKEY, ECGSEMKEY);
		rc = AH_system (0, res_msg);
	      }
	    }
		print_log (LOGMSG,"Retruned from AH_system. rc = %d Msg = %s\n", rc,
		    res_msg);
	    strcat (result_msg, res_msg);
	    notify_send.cmd = msg_rcv.cmd;
	    strcpy (notify_send.msg,
		    "Action was taken to Active or Halt the system.");
	    cur_ecg_pos = 0;
	  }
	  else {
	    if (ECG_MAX_ENTRIES > 0) {
	      rc = AH_system (0, res_msg);
	    }
	    else {
		  sprintf (res_msg,"ECG Doesn't contain any exerciser"
		       " or it has \nexercisers that have already been"
		       " defined in other ECG(s)\n");
	    }
		print_log (LOGMSG,"Retruned from AH_system. rc = %d Msg = %s\n", rc,
		    res_msg);
	    strcat (result_msg, res_msg);
	    notify_send.cmd = msg_rcv.cmd;
	    strcpy (notify_send.msg,
		    "Action was taken to Active or Halt the system.");
			//sprintf(scn_1_send.err_msg,"Command completed Successfully");
	  }
	}
		//print_log(LOGMSG,"done ah_sys\n"); fflush(stdout);
	    //}
    //else
	//      strcat(result_msg,"ERROR");

	if (!autostart)
	  send_data (new_fd, msg_rcv.cmd, 0, rc, result_msg);
	break;
      case 2011:
      case 2021:
	/* run command with -nonblock, client no need to wait for completion of 'run' command, so sends response back */
	if ( (isRunningECGOnBackground == TRUE) && (msg_rcv.cmd == 2011) && (is_cmdline == TRUE) ){
		newECGFileName = strrchr(full_name, '/');
		if ( newECGFileName != NULL)
		{ 
			newECGFileName++;
			if (*newECGFileName != NULL) {
				strcpy (currentlyRunningECGName, get_running_ecg_name(0));
				if ( (strlen(currentlyRunningECGName) !=0) && 
					(strncmp (currentlyRunningECGName, newECGFileName, strlen(currentlyRunningECGName)) == 0 ) ) {
					sprintf(result_msg," ECG (%s) already Active", full_name);
				}
				else {
					sprintf(result_msg," STX daemon is processing the run command, please wait before executing next command...");
				}

				send_data (new_fd, msg_rcv.cmd, 0, rc, result_msg);
			}
		}
	}

	print_log (LOGMSG,"rc = %d num_ecg = %d num_ecgs =%d\n", rc, num_ecg,
		num_ecgs);
    //if (rc >= -1) {
	sprintf (result_msg, " ");
	if (strcmp (ecg[0], "/ecg.all") == 0) {
	  for (cnt = 1; cnt < num_ecgs; cnt++) {
	    cur_ecg_pos = cnt;
		print_log (LOGMSG,"ECG_MAX_ENTRIES: %d ECGSTATUS: %s \n",
		    ECG_MAX_ENTRIES, ECGSTATUS);
	    if (ECG_MAX_ENTRIES > 0) {
	      shm_addr.hdr_addr = ECGSHMADDR_HDR;
	      shm_addr.HE_addr = ECGSHMADDR_HE;
	      semhe_id = ECGSEMHEID;
		  print_log
		  (LOGMSG,"calling AH_system for %s/%s:\n shmkey:%d semkey:%d\n",
		   ECGPATH, ECGNAME, ECGSHMKEY, ECGSEMKEY);
	      rc = AH_system (0, res_msg);
	    }
	  }
	  if ( msg_rcv.cmd == 2011 )
		sprintf(res_msg,"Activated /ecg.all (SYSTEM ECG)");
	  else
		sprintf(res_msg,"Suspended /ecg.all (SYSTEM ECG)");
	  print_log (LOGMSG,"Retruned from AH_system. rc = %d Msg = %s\n", rc,
		  res_msg);
	  strcat (result_msg, res_msg);
	  notify_send.cmd = msg_rcv.cmd;
	  strcpy (notify_send.msg,
		  "Action was taken to Active or Halt the system.");
	  cur_ecg_pos = 0;
	}
	else {
	  for (cnt = 0; cnt < num_ecg; cnt++) {
		print_log (LOGMSG,"To start_stop is %s cur_ecg_pos = %d\n", ecg[cnt],
		    cur_ecg_pos);
	    bad_input = TRUE;
	    for (tmp = 0; tmp < num_ecgs; tmp++) {
	      cur_ecg_pos = tmp;
	      PUT_FULL_ECG;
	      if ((strcmp (full_name, ecg[cnt]) == 0)
		  && (ECG_MAX_ENTRIES > 0)) {
		shm_addr.hdr_addr = ECGSHMADDR_HDR;
		shm_addr.HE_addr = ECGSHMADDR_HE;
		semhe_id = ECGSEMHEID;
		sprintf (res_msg, " ");
		bad_input = FALSE;
		print_log (LOGMSG,"20n1:calling AH_system ECG = %s\n", full_name);
		fflush (stdout);
		rc = AH_system (0, res_msg);

		print_log (LOGMSG,"Retruned from AH_system. rc = %d Msg = %s\n",
			rc, res_msg);
		strcat (result_msg, res_msg);
		notify_send.cmd = msg_rcv.cmd;
		strcpy (notify_send.msg,
			"Action was taken to Active or Halt the system.");
		break;
	      }
	      else if ((strcmp (full_name, ecg[cnt]) == 0)
		       && (ECG_MAX_ENTRIES <= 0)) {
		bad_input = FALSE;
		sprintf (result_msg,"ECG(%s) Doesn't contain any exerciser"
			 " or it has \nexercisers that have already been"
			 " defined in other ECG(s)\n", ecg[cnt]);
	      }
	    }

	    if (bad_input) {
		  stx_error_no = 1;
		  sprintf (result_msg, "Incorrect ECG Name(%s) provided.\n",
		       ecg[cnt]);
	    }
	  }
	}
	if (isRunningECGOnBackground != TRUE) { /* skipping response for nonblock run command, since response was already send */
		send_data (new_fd, msg_rcv.cmd, 0, rc, result_msg);
	}
	break;


      case SCREEN_2:
      case 2012:
      case 2022:
      case 2032:
	print_log (LOGMSG,"calling AH_device %d: %s\n", num_actv, lst[0]);
	fflush (stdout);
	if (msg_rcv.cmd == 2022)
	  rc = AH_device (result_msg, actv, num_actv, &max_disp);
	else if (msg_rcv.cmd == 2032)
	  rc = AH_device (result_msg, suspend, num_suspend, &max_disp);
	else
	  rc = AH_device (result_msg, (char **) 0, -1, &max_disp);

	print_log (LOGMSG,"called AH_device \n");

	send_data (new_fd, msg_rcv.cmd, max_disp, rc, result_msg);
	if (((msg_rcv.cmd == SCREEN_2)
	     && ((msg_rcv.subcmd == SUBCMD_a) || (msg_rcv.subcmd == SUBCMD_s)))
	    || (msg_rcv.cmd == 2012)) {
	  notify_send.cmd = msg_rcv.cmd;
	  strcpy (notify_send.msg,
		  "Action was taken to Active or Halt the Device.");
	}
	break;

      case SCREEN_3:
      case 2013:
      case 2023:
      case 2033:
	if (msg_rcv.cmd == 2023)
	  rc = COE_device (result_msg, coe, num_coe, &max_disp);
	else if (msg_rcv.cmd == 2033)
	  rc = COE_device (result_msg, soe, num_soe, &max_disp);
	else
	  rc = COE_device (result_msg, (char **) 0, -1, &max_disp);

	//print_log(LOGMSG,"sending\n"); fflush(stdout);
	send_data (new_fd, msg_rcv.cmd, max_disp, rc, result_msg);
	if (((msg_rcv.cmd == SCREEN_3)
	     && ((msg_rcv.subcmd == SUBCMD_c) || (msg_rcv.subcmd == SUBCMD_s)))
	    || (msg_rcv.cmd == 2013)) {
	  notify_send.cmd = msg_rcv.cmd;
	  strcpy (notify_send.msg,
		  "Action was taken to COE or SOE Device.");
	}
	break;


      case SCREEN_5:
	print_log(LOGMSG,"Calling disp dst\n"); fflush(stdout);
	fflush (stdout);
	rc = disp_dst (result_msg, stts, num_stts, &max_disp);
	send_data (new_fd, msg_rcv.cmd, max_disp, rc, result_msg);
	break;

      case SCREEN_6:

	print_log (LOGMSG,"STATS... pid = %d ecg=:%s:\n", hxstats_PID, ecg[0]);
	fflush (stdout);
	temp_var = hxstats_PID;
	if (hxstats_PID != 0) {
	  if (strcmp (ecg[0], "/ecg.all") == 0) {
	    stat_workstr = malloc (512);
	    if (stat_workstr == NULL) {      /* problem?                          */
		  print_log
		  (LOGERR,"####ERROR: Unable to malloc for stat_workstr ###ERRNO = %d (%s)",
		   errno, strerror (errno));
		  sprintf (result_msg,
		       "####ERROR: Unable to malloc for stat_workstr ###ERRNO = %d (%s)",
		       errno, strerror (errno));
	      send_err_msg (new_fd, result_msg, -1);
	      break;
	    }                /* endif */
	    if (truncate ("/tmp/htxstats.ecg.all", 0) == -1) {
	      if ( errno == ENOENT )  {
		if ( creat("/tmp/htxstats.ecg.all", 0) >= 0 ) {
		  print_log (LOGMSG,"created the file /tmp/htxstats.ecg.all\n");
		} else {
		  print_log
			  (LOGERR,"Could not truncate the htxstats file.errno=%d (%s)\n",
		       errno, strerror (errno));;
		       sprintf(result_msg, "Error truncting the htxstats file.");
		       send_err_msg (new_fd, result_msg,
				     -1);
		}
	      } else {
		print_log
			(LOGERR,"Could not truncate the htxstats file.errno=%d (%s)\n",
		     errno, strerror (errno));;
		     sprintf(result_msg, "Error truncting the htxstats file.");
		     send_err_msg (new_fd, result_msg,
				   -1);
	      }
	    }
		print_log (LOGMSG,"truncated the file /tmp/htxstats.ecg.all\n");
		sprintf (stat_workstr,
		     "cat /tmp/htxstats >>/tmp/htxstats.ecg.all");
		sprintf (xfer_name, "htxstats.ecg.all");
	    for (i = 1; i < num_ecgs; i++) {
	      cur_ecg_pos = i;
		  print_log
		  (LOGMSG,"################ ECG.all Sending sigusr1 ################\n");
	      fflush (stdout);
	      print_log(LOGMSG,"ECGNAME: %s, ECGSTATUS: %s\n",ECGNAME,ECGSTATUS);
	      if (strcmp(ECGSTATUS, "UNLOADED") != 0){
		rem_shm_addr->sock_hdr_addr->cur_shm_key = ECGSHMKEY;
		system_call = TRUE;
		system("rm /tmp/htxstats_done");
		system_call = FALSE;
 		kill (temp_var, SIGUSR1);  /* tell stats prog to upd file */
		if ( is_statready() == 0 )
	 	   print_log(LOGMSG,"htxstats for %s not ready\n",ECGNAME);
		else
		   print_log(LOGMSG,"htxstats for %s ready\n",ECGNAME);

	      }
	      else
		continue;
		  print_log (LOGMSG,"Updating STX statistics file...");
	      /*sleep (5); */   /* give stats prog time to upd */
	      system_call = TRUE;
	      system("ls -l /tmp/htxstats >> /tmp/statsize");
	      system_call = FALSE;
	      system_call = TRUE;
	      system (stat_workstr);
	      system_call = FALSE;
	    }
	    free (stat_workstr);
	    stat_workstr = NULL;
	  }
	  else if (is_cmdline) {
	    stat_workstr = malloc (512);
		print_log (LOGMSG,"getting hxstats for cmd_line\n");
	    if (truncate ("/tmp/htxstats.cmd_ln", 0) == -1) {
	      if ( errno == ENOENT )  {
		if ( creat("/tmp/htxstats.ecg.all", 0) >= 0 ) {
		  print_log (LOGMSG,"created the file /tmp/htxstats.cmd_ln\n");
		} else {
		  print_log
			  (LOGERR,"Could not truncate the htxstats file.errno=%d (%s)\n",
		       errno, strerror (errno));;
		       sprintf(result_msg, "Error truncting the htxstats file.");
		       send_err_msg (new_fd, result_msg,
				     -1);
		}
	      } else {
		print_log
			(LOGERR,"Could not truncate the htxstats file.errno=%d (%s)\n",
		     errno, strerror (errno));;
		     sprintf(result_msg, "Error truncting the htxstats file.");
		     send_err_msg (new_fd, result_msg,
				   -1);
	      }
	    }
		print_log (LOGMSG,"truncated the file /tmp/htxstats.cmd_ln\n");
		sprintf (stat_workstr,
		     "cat /tmp/htxstats >>/tmp/htxstats.cmd_ln");
		sprintf (xfer_name, "htxstats.cmd_ln");
	    for (i = 0; i < num_ecg; i++) {
	      for (cnt = 1; cnt < num_ecgs; cnt++) {
		cur_ecg_pos = cnt;
		PUT_FULL_ECG;
		print_log (LOGMSG,"comparing ecg %s with full_name %s\n", ecg[tmp],
			full_name);
		if (strcmp (ecg[i], full_name) == 0) {
		  print_log
			  (LOGMSG,"################ Sending sigusr1 ################\n");
		  fflush (stdout);
		  if (strcmp(ECGSTATUS, "UNLOADED") != 0)
		  	rem_shm_addr->sock_hdr_addr->cur_shm_key = ECGSHMKEY;
			//print_log(LOGMSG,"the current shared memory is: %d\n",rem_shm_addr->sock_hdr_addr->cur_shm_key );
			//sleep(2);
		  kill (temp_var, SIGUSR1);    /* tell stats prog to upd file */
		  print_log (LOGMSG,"Updating STX statistics file...");
		  /* sleep (1); */     /* give stats prog time to upd */
		  system_call = TRUE;
		  system (stat_workstr);
		  system_call = FALSE;
		  break;
		}
	      }
	    }
	    free (stat_workstr);
	    stat_workstr = NULL;
	  }
	  else {
	    stat_workstr = malloc (512);
	    if (stat_workstr == NULL) {      /* problem?                          */
		  print_log
		  (LOGERR,"####ERROR: In else, Unable to malloc for stat_workstr ###ERRNO = %d (%s)",
		   errno, strerror (errno));
		  sprintf (result_msg,
			   "####ERROR: In else, Unable to malloc for stat_workstr ###ERRNO = %d (%s)",
			   errno, strerror (errno));
		  send_err_msg (new_fd, result_msg, -1);
		  break;
		}                /* endif */
		print_log (LOGMSG,"################ Sending sigusr1 ################\n");
	    fflush (stdout);
	    if (strcmp(ECGSTATUS, "UNLOADED") != 0)
	    	rem_shm_addr->sock_hdr_addr->cur_shm_key = ECGSHMKEY;
	    //sleep(2);
	    kill (temp_var, SIGUSR1);     /* tell stats prog to upd file */
		print_log (LOGMSG,"Updating STX statistics file...");
	    /* sleep (1);  */     /* give stats prog time to upd */
		sprintf (stat_workstr, "cp /tmp/htxstats /tmp/htxstats.%s",
		     ECGNAME);
		sprintf (xfer_name, "htxstats.%s", ECGNAME);
	    system_call = TRUE;
	    system (stat_workstr);
	    system_call = FALSE;
	    free (stat_workstr);
	    stat_workstr = NULL;
	  }
	  sprintf (get_put, "get");
	  sprintf (xfer_path, "/tmp/");
	  transfer_file (new_fd, get_put, "ascii", xfer_path, xfer_name, 0);
	  print_log (LOGMSG,"Done the transfer of file\n");
	  fflush (stdout);
	  break;
	}
	else {
	  rc = -1;
	  strcpy(result_msg, "echo \"System must be started before statistics can be collected .\"> /tmp/command_result");
	  system_call = TRUE;
	  system(result_msg);
	  system_call = FALSE;

#if 0


       /******* Allocate memory **********/
	  file_xfer = (char *) malloc (512);
	  if (file_xfer == NULL) {
		print_log
		(LOGERR,"Error in malloc for file_xfer. errno =%d (%s). htxstats()",
		 errno, strerror (errno));
		sprintf (result_msg,
		     "Error in malloc for file_xfer. errno =%d (%s). htxstats()",
		     errno, strerror (errno));
	    send_err_msg (new_fd, result_msg, -1);
	    break;
	  }
	  tmpstr = (char *) malloc (512);
	  if (tmpstr == NULL) {
		print_log
		(LOGERR,"System not started. Error in malloc for tmpstr. errno =%d (%s). htxstats()",
		 errno, strerror (errno));
		sprintf (result_msg,
		     "System not started. Error in malloc for tmpstr. errno =%d (%s). htxstats()",
		     errno, strerror (errno));
	    send_err_msg (new_fd, result_msg, -1);
	    break;
	  }
	  memset (tmpstr, 0, 512);
	  memset (file_xfer, 0, 512);
       /******* Done:  Allocate memory **********/
	  sprintf (file_xfer, "%d%c%s ", rc, sep_at, result_msg);
	  sprintf (msgptr, "%8x%c%s", (strlen (file_xfer) + 1), sep_at,
		   file_xfer);
	  print_log
		  (LOGERR,"System must be started before statistics can be collected.");
#ifdef _a2e_h	/*400-EBCDIC*/
	  ebcdic2Ascii(&msgptr[0],&msgptr[0],strlen(msgptr));
#endif
	  if ((numbytes = send (new_fd, msgptr, strlen (msgptr), 0)) == -1) {
		print_log (LOGERR,"send");
	  }
	  free (tmpstr);
	  free (file_xfer);
	  tmpstr = NULL;
	  file_xfer = NULL;
#endif
	  sprintf(get_put, "get");
	  sprintf(xfer_path, "/tmp/");
	  transfer_file(new_fd, get_put, "ascii", xfer_path, "command_result", 0) ;
	  break;
	}                      /* endif */

      case SCREEN_8:
	sigvec.sa_handler = SIG_IGN;
	sigaction (SIGCHLD, &sigvec, (struct sigaction *) NULL);
#ifndef __OS400__                           /* 400 */
	sigvec.sa_flags = SA_RESTART;
#endif

	system_call = TRUE;
	system ("perl /usr/lpp/htx/etc/scripts/eMerge.pl > /tmp/htxsum");
	system_call = FALSE;

	sigvec.sa_handler = (void (*)(int)) child_death;
	sigaction (SIGCHLD, &sigvec, (struct sigaction *) NULL);
#ifndef __OS400__                           /* 400 */
	sigvec.sa_flags = SA_RESTART;
#endif
	sprintf (get_put, "get");
	//sprintf(xfer_mode,"ascii");
	sprintf (xfer_path, "/tmp/");
	sprintf (xfer_name, "htxsum");
	transfer_file (new_fd, get_put, "ascii", xfer_path, xfer_name, 0);
	break;

      case 2018:
	if (truncate ("/tmp/htxsum", 0) == -1) {
	  print_log (LOGERR,"Could not truncate the htxsum file.errno=%d (%s)\n",
		  errno, strerror (errno));;
		  sprintf(result_msg, "Error truncting the htxerr file.");
		  send_err_msg (new_fd, result_msg, -1);
	}
	else
	  sprintf(result_msg, "Error summary(htxsum) cleared.");
	  send_err_msg (new_fd, result_msg, 0);
	break;

      case SCREEN_7:
	sprintf (get_put, "get");
	sprintf (xfer_path, "/tmp/");
	sprintf (xfer_name, "htxerr");
	transfer_file (new_fd, get_put, "ascii", xfer_path, xfer_name, 0);
	break;

    //case CLRERRLOG:
      case 2017:
	if (truncate ("/tmp/htxerr", 0) == -1) {
	  print_log (LOGERR,"Could not truncate the htxerr file.errno=%d (%s)\n",
		  errno, strerror (errno));;
		  sprintf(result_msg, "Error truncting the htxerr file.");
		  send_err_msg (new_fd, result_msg, -1);
	}
	else
	  sprintf(result_msg, "Error log(htxerr) cleared.");
	  send_err_msg (new_fd, result_msg, 0);
	break;


      case SCREEN_A:      /* 3000 */
	sigvec.sa_handler = SIG_IGN;
	sigaction (SIGCHLD, &sigvec, (struct sigaction *) NULL);
#ifndef __OS400__                           /* 400 */
	sigvec.sa_flags = SA_RESTART;
#endif

	system_call = TRUE;
	system ("/usr/lpp/htx/etc/scripts/gen_sysdata");
	system_call = FALSE;

	sigvec.sa_handler = (void (*)(int)) child_death;
	sigaction (SIGCHLD, &sigvec, (struct sigaction *) NULL);
#ifndef __OS400__                           /* 400 */
	sigvec.sa_flags = SA_RESTART;
#endif
	sprintf (get_put, "get");
	sprintf (xfer_path, "/tmp/");
	sprintf (xfer_name, "htx_sysdata");
	transfer_file (new_fd, get_put, "ascii", xfer_path, xfer_name, 0);
	break;

      case 3001:
	sigvec.sa_handler = SIG_IGN;
	sigaction (SIGCHLD, &sigvec, (struct sigaction *) NULL);
#ifndef __OS400__                           /* 400 */
	sigvec.sa_flags = SA_RESTART;
#endif

	system_call = TRUE;
	system ("/usr/lpp/htx/etc/scripts/gen_vpd >/tmp/htx_vpd");
	system_call = FALSE;

	sigvec.sa_handler = (void (*)(int)) child_death;
	sigaction (SIGCHLD, &sigvec, (struct sigaction *) NULL);
#ifndef __OS400__                           /* 400 */
	sigvec.sa_flags = SA_RESTART;
#endif
	sprintf (get_put, "get");
	sprintf (xfer_path, "/tmp/");
	sprintf (xfer_name, "htx_vpd");
	transfer_file (new_fd, get_put, "ascii", xfer_path, xfer_name, 0);
	break;

      case 2029:
	if (num_addexer > 0)
	  strcpy (reg_exp, add_exer[0]);
	else {
	  sprintf (result_msg, "Please enter a valid device name\n");
	  rc = -1;
	  send_data (new_fd, msg_rcv.cmd, max_disp, rc, result_msg);
	  break;
	}

      case 9995:
        strcpy (tmp_ecgname, get_running_ecg_name (0));
	if (strlen (tmp_ecgname) > 0) {
	  sprintf (result_msg, "Currently running ECG/MDT : %s", tmp_ecgname);
	}
	else {
	  strcpy (result_msg, "Currently running ECG/MDT : No ECG/MDT is running currently");
	}
	send_data (new_fd, msg_rcv.cmd, 0, 0, result_msg);
	break;

      case 2042:
	rc = get_ecg_run_time(&ecg_run_time, ecg[0]);
	switch(rc)
        {
        case 0:
		sprintf (result_msg,"Run_time = %ld minutes %ld seconds", ecg_run_time / 60, ecg_run_time % 60);
		break;
	case -2:
		stx_error_no = 1;
                sprintf (result_msg, "Incorrect ECG Name(%s) provided", ecg[0]);
                break;

        case -3:
		stx_error_no = 1;
		sprintf (result_msg, "No ECG/MDT is running currently");
		break;

        default:
		stx_error_no = 1;
                sprintf (result_msg, "Unknown Error");
                break;
        }

	send_data (new_fd, msg_rcv.cmd, 0, 0, result_msg);
	break;

      case 2044:
	rc = get_last_update_time(result_msg,ecg[0]);
	switch(rc)
	{
	case 0:
		break; /* no error to handle */
	case -2:
		stx_error_no = 1;
		sprintf (result_msg, "Incorrect ECG Name(%s) provided", ecg[0]);
		break;
        case -3:
		stx_error_no = 1;
		sprintf (result_msg, "No ECG/MDT is running currently");
		break;
	default:
		stx_error_no = 1;
		sprintf (result_msg, "Unknown error (%d)", rc);
		break;
	}
	send_data (new_fd, msg_rcv.cmd, 0, 0, result_msg);
	break;

      case 2045:
	rc = get_fail_status(ecg[0]);
	switch(rc)
	{
	case 1:
	case 0:
		sprintf (result_msg,"Fail_status = %d", rc);
		break;
	case -2:
		stx_error_no = 1;
		sprintf (result_msg, "Incorrect ECG Name(%s) provided", ecg[0]);
		break;
        case -3:
		stx_error_no = 1;
		sprintf (result_msg, "No ECG/MDT is running currently");
		break;
	default:
		stx_error_no = 1;
		sprintf (result_msg, "Unknown error (%d)", rc);
		break;
	}
	send_data (new_fd, msg_rcv.cmd, 0, 0, result_msg);
	break;

      case 2046:
	if(num_devcycles == 0) {
		stx_error_no = 1;
		sprintf (result_msg, "No device name prefix is provided");
	} else {

		rc = get_cumulative_cycles_per_device(&device_cycle_count, devcycles[0], ecg[0]);
		switch(rc)
		{
		case 0:
			sprintf (result_msg,"%s = %ld", devcycles[0], device_cycle_count);
			break;
		case -2:
			stx_error_no = 1;
			sprintf (result_msg, "Incorrect ECG Name(%s) provided", ecg[0]);
			break;
		case -3:
			stx_error_no = 1;
			sprintf (result_msg, "No ECG/MDT is running currently");
			break;
		case -4:
			stx_error_no = 1;
			sprintf (result_msg, "Device name prefix(%s) is NOT found", devcycles[0]);
			break;
		default:
			stx_error_no = 1;
			sprintf (result_msg, "Unknown error (%d)", rc);
			break;
		}
	}

	send_data (new_fd, msg_rcv.cmd, 0, 0, result_msg);
	break;
      case SCREEN_9_A:
	PUT_FULL_ECG;
	if (strcmp (full_name, "/ecg.all") == 0) {
	  memset (result_msg, 0, 80);
	  sprintf (result_msg,
		   "You selected the SYSTEM ECG. Select Another ECG\n");
	  rc = -1;
	  send_data (new_fd, msg_rcv.cmd, max_disp, rc, result_msg);
	  break;
	}
	rc = A_device (result_msg);
	print_log (LOGMSG,"Retruned from A_device. rc = %d \n", rc);
	info_send.cur_info = *(rem_shm_addr->cur_shm_addr);
	info_send.sys_hdr_info = *(rem_shm_addr->sock_hdr_addr);
	 /*if (msg_rcv.cmd == SCREEN_9_A) {
	    notify_send.cmd = msg_rcv.cmd;
	    strcpy(notify_send.msg, "Action was taken to Add Device.");
	    } */

	if (rc == 0) {
	  memset (result_msg, 0, 80);
	  sprintf (result_msg, "Command completed Successfully\n");
	}

	print_log (LOGMSG,"Sending data for 9_A\n");
	send_data (new_fd, msg_rcv.cmd, max_disp, rc, result_msg);
	print_log (LOGMSG,"Sent data for 9_A\n");
	break;

      case SCREEN_9_R:
      case SCREEN_9_R_D:
	rc = R_device (result_msg, rstrt, num_rstrt, &max_disp);
	if (msg_rcv.cmd == SCREEN_9_R_D) {
	  notify_send.cmd = msg_rcv.cmd;
	  strcpy (notify_send.msg, "Action was taken to Restart Device.");
	}

	send_data (new_fd, msg_rcv.cmd, max_disp, rc, result_msg);
	break;

      case SCREEN_9_T:
      case SCREEN_9_T_D:
	rc = T_device (result_msg, term, num_term, &max_disp);
	if (msg_rcv.cmd == SCREEN_9_T_D) {
	  notify_send.cmd = msg_rcv.cmd;
	  strcpy (notify_send.msg, "Action was taken to Terminate Device.");
	}
	send_data (new_fd, msg_rcv.cmd, max_disp, rc, result_msg);
	break;

      case SCREEN_4:
	print_log(LOGMSG,"ECG To be shutdown is. num_ecgs = %d \n", num_ecgs); fflush(stdout);
	shutdown_ecg = TRUE;
	bad_input = TRUE;
	strcpy(res_msg," ");
	if (is_cmdline) {
	  get_running_ecg_name (result_msg);
	  strcat (result_msg, "\n ");
	}
	else {
	  strcpy(result_msg," ");
	}
	notify_send.cmd = msg_rcv.cmd;
	strcpy (notify_send.msg, "Action was taken to Shutdown the ECG.");
	print_log (LOGMSG,"Shutting down ECG as per request\n");
	fflush (stdout);
	running_status = 99;
	if (num_ecgs == 0) {
	  sprintf(result_msg, "No ECG File in use. Nothing to shutdown\n");
	  send_err_msg (new_fd, result_msg , 0);
	  break;
	}
	sprintf (res_msg, " ");
	limit_exceed = 0;
	if (strcmp (ecg[0], "/ecg.all") == 0) {
	  for (cnt=1; cnt<num_ecgs; cnt++) {
	    cur_ecg_pos=cnt;
	    PUT_FULL_ECG;
	    if ( strcmp(ECGSTATUS,"UNLOADED")!=0) {
	      shm_addr.hdr_addr = ECGSHMADDR_HDR;
	      shm_addr.HE_addr = ECGSHMADDR_HE;
	      semhe_id = ECGSEMHEID;
	      end_ecg ();
	      if ((strcmp (ECGSTATUS, "ACTIVE") == 0) || (strcmp (ECGSTATUS, "PARTIALLY RUNNING") == 0 )) {
		active_ecg_name[0] = '\0';
	      }
	      strcpy (ECGSTATUS, "UNLOADED");
	      if (is_cmdline) {
		  sprintf(res_msg,"ECG(%s) shutdown successful",full_name);
	      }
	      else {
		  sprintf(res_msg,"ECG(%s) shutdown successful\n",full_name);
	      }

                  /* Log the time of system shutdown */
                 fp = popen("date","r");
                 fgets(mydate,25,fp);

                 sprintf(mycommand,"echo \"%s shutdown on %s \" >> /tmp/stx.start.stop.time",ECGNAME,mydate);
                 system_call = TRUE;
                 system(mycommand);
                 system_call = FALSE;
                 pclose(fp);

	      if ((strlen(result_msg) + strlen(res_msg))<=10240)
		strcat(result_msg,res_msg);
	      else if (limit_exceed == 0) {
		strcat(result_msg,"...Message too long");
	    limit_exceed = 1;
	      }
	    }
	  }
	} else {
	  for (i=0; i<num_ecg; i++) {
	    bad_input = TRUE;
	    for (cnt=1; cnt<num_ecgs; cnt++) {
	      cur_ecg_pos=cnt;
	      PUT_FULL_ECG;
	      if ((strcmp(full_name, ecg[i]) ==0) &&
		  (strcmp(ECGSTATUS,"UNLOADED")!=0)) {
		shm_addr.hdr_addr = ECGSHMADDR_HDR;
		shm_addr.HE_addr = ECGSHMADDR_HE;
		semhe_id = ECGSEMHEID;
		end_ecg ();
	        if ((strcmp (ECGSTATUS, "ACTIVE") == 0) || (strcmp (ECGSTATUS, "PARTIALLY RUNNING") == 0 )) {
	          active_ecg_name[0] = '\0';
	        }
		strcpy (ECGSTATUS, "UNLOADED");
	      if (is_cmdline) {
		  sprintf(res_msg,"ECG(%s) shutdown successful",full_name);
	      }
	      else {
		sprintf(res_msg,"ECG(%s) shutdown successful\n",full_name);
	      }

                /* Log the time of ecg shutdown */
                  fp = popen("date","r");
                  fgets(mydate,25,fp);

                  sprintf(mycommand,"echo \"%s shutdown on %s \" >> /tmp/stx.start.stop.time",ECGNAME,mydate);
                  system_call = TRUE;
                  system(mycommand);
                  system_call = FALSE;
                  pclose(fp);

		if ((strlen(result_msg) + strlen(res_msg))<=10240)
		  strcat(result_msg,res_msg);
		else if (limit_exceed == 0) {
		  strcat(result_msg,"...Message too long");
		  limit_exceed = 1;
		}
		bad_input = FALSE;
	      }
	      else if ((strcmp(full_name, ecg[i]) ==0) &&
		       (strcmp(ECGSTATUS,"UNLOADED")==0)) {
		sprintf(res_msg,"ECG(%s) already Inactive\n",full_name);
		if ((strlen(result_msg) + strlen(res_msg))<=10240)
		  strcat(result_msg,res_msg);
		else if (limit_exceed == 0) {
		  strcat(result_msg,"...Message too long");
		  limit_exceed = 1;
		}
		bad_input = FALSE;
	      }
	    }
	    if (bad_input) {
		  stx_error_no = 1;
		  sprintf(res_msg,"Incorrect ECG(%s) name provided\n",ecg[i]);
	      if ((strlen(result_msg) + strlen(res_msg))<=10240)
		strcat(result_msg,res_msg);
	      else if (limit_exceed == 0) {
		strcat(result_msg,"...Message too long");
	    limit_exceed = 1;
	      }
	    }

	  }
	}
	send_err_msg (new_fd, result_msg, 0);
	bcast_done = FALSE;
	if (ipc_done) {
	  rem_shm_addr->sock_hdr_addr->bcast_done = FALSE;
	}
	sprintf (msg_type, "UPDATE");
	break;

      case SHUTDOWN:
	 /********** save the htxstats first **************************/
	temp_var = hxstats_PID;
	if (hxstats_PID != 0) {
	  stat_workstr = malloc (512);
	  if (stat_workstr == NULL) {      /* problem?                */
		print_log
		(LOGERR,"####ERROR: Unable to malloc for stat_workstr ###ERRNO = %d (%s)",
		 errno, strerror (errno));
		sprintf (result_msg,
		     "####ERROR: Unable to malloc for stat_workstr ###ERRNO = %d (%s)",
		     errno, strerror (errno));
	    send_err_msg (new_fd, result_msg, -1);
	    break;
	  }                /* endif */
	  if (truncate ("/tmp/htxstats.ecg.all", 0) == -1) {
	    if ( errno == ENOENT )  {
	      if ( creat("/tmp/htxstats.ecg.all", 0) >= 0 ) {
		print_log (LOGMSG,"created the file /tmp/htxstats.ecg.all\n");
	      } else {
		print_log
			(LOGERR,"Could not truncate the htxstats file.errno=%d (%s)\n",
		     errno, strerror (errno));;
		     sprintf(result_msg, "Error truncting the htxstats file.");
		     send_err_msg (new_fd, result_msg,
				   -1);
	      }
	    } else {
		  print_log
		  (LOGERR,"Could not truncate the htxstats file.errno=%d (%s)\n",
		   errno, strerror (errno));;
		   sprintf(result_msg, "Error truncting the htxstats file.");
		   send_err_msg (new_fd, result_msg,
				 -1);
	    }
	  }
	  print_log (LOGMSG,"truncated the file /tmp/htxstats.ecg.all\n");
	  sprintf (stat_workstr,
		   "cat /tmp/htxstats >>/tmp/htxstats.ecg.all");
	  sprintf (xfer_name, "htxstats.ecg.all");
	  for (i = 1; i < num_ecgs; i++) {
	    cur_ecg_pos = i;
		print_log
		(LOGMSG,"################ Sending sigusr1 ################\n");
	    fflush (stdout);
	    rem_shm_addr->sock_hdr_addr->cur_shm_key = ECGSHMKEY;
		//sleep(2);
	    if (strcmp(ECGSTATUS, "UNLOADED") != 0)
	      kill (temp_var, SIGUSR1);  /* tell stats prog to upd file */
	    else
	      continue;
		print_log (LOGMSG,"Updating STX statistics file...");
	     sleep (5);    /* give stats prog time to upd */
	    system_call = TRUE;
	    system (stat_workstr);
	    system_call = FALSE;
	  }
	  free (stat_workstr);
	  stat_workstr = NULL;
	}
	 /************ shutdown the current test **************************/
	shutdown_ecg = TRUE;
	notify_send.cmd = msg_rcv.cmd;
	strcpy (notify_send.msg, "Action was taken to Shutdown the SYSTEM.");
	print_log (LOGMSG,"Shutting down SYSTEM as per request\n");
	fflush (stdout);
	running_status = 99;
	for (i = 1; i < num_ecgs; i++) {
	  cur_ecg_pos = i;
	  if (strcmp (ECGSTATUS, "UNLOADED") != 0) {
	    shm_addr.hdr_addr = ECGSHMADDR_HDR;
	    shm_addr.HE_addr = ECGSHMADDR_HE;
	    semhe_id = ECGSEMHEID;
		//shm_addr.hdr_addr->started = 0;
	    end_ecg ();
	    if ((strcmp (ECGSTATUS, "ACTIVE") == 0) || (strcmp (ECGSTATUS, "PARTIALLY RUNNING") == 0 )) {
	      active_ecg_name[0] = '\0';
	    }
	    strcpy (ECGSTATUS, "UNLOADED");

            /* Log the time of ECG shutdown */
               fp = popen("date","r");
               fgets(mydate,25,fp);

               sprintf(mycommand,"echo \"%s shutdown on %s \" >> /tmp/stx.start.stop.time",ECGNAME,mydate);
               system_call = TRUE;
               system(mycommand);
               system_call = FALSE;
               pclose(fp);

	  }
	  else
	    continue;

	}
	print_log (LOGMSG,"Done shutting down the ECGs... Will call end_it now...\n");
	fflush (stdout);
	end_it ((0 - new_fd));
	num_ecgs = 0;
	ipc_done = 0;
	//sig_handle_done = 0;
	system_started = 0;
	sprintf(result_msg, "STX Daemon Shutdown Successful");
	send_err_msg (new_fd, result_msg, -99);
	 //bzero((int *)ecg_info, (sizeof(tecg_struct) * MAX_ECG_ENTRIES));
	bcast_done = FALSE;
	 /*MsgQueCreated = FALSE; */
	DevToSlotInitialized = FALSE;
	sprintf (msg_type, "UPDATE");
	break;

      case 9999:
	rc = query_all (result_msg, query, num_query, &max_disp);
	send_data (new_fd, msg_rcv.cmd, max_disp, rc, result_msg);
	break;

      case 9998:
	send_data (new_fd, msg_rcv.cmd, num_ecgs, 0, "GETECGLIST");
	break;

      case 9997:
	send_data (new_fd, msg_rcv.cmd, 1, 0, "GETECGSUMMARY");
	break;

      case 9996:
	send_data (new_fd, msg_rcv.cmd, 1, 0, "GETTESTSUMMARY");
	break;

      case 2015:
	send_data (new_fd, msg_rcv.cmd, 1, 0, "GETSSMSUM");
	break;

      case 2020:
	send_data (new_fd, msg_rcv.cmd, 1, 0, "GETECGINFO");
	break;

      case 4000:
	sprintf (xfer_name, "SENDMORE");
	tmpstr = (char *) malloc (100);
	if (tmpstr == NULL) {
	  print_log
		  (LOGERR,"Command 4000. Error in malloc for tmpstr. errno =%d (%s).",
	       errno, strerror (errno));
	  sprintf (result_msg,
		   "Command 4000. Error in malloc for tmpstr. errno =%d (%s).",
		   errno, strerror (errno));
	  send_err_msg (new_fd, result_msg, -1);
	  break;
	}
	sprintf (msgptr, "%8x@%s", (strlen (xfer_name) + 1), xfer_name);
	size = msg_rcv.subcmd;
#ifdef _a2e_h	/*400-EBCDIC*/
	ebcdic2Ascii(&msgptr[0],&msgptr[0],strlen(msgptr));
#endif
	if (send (new_fd, msgptr, strlen (msgptr), 0) == -1) {
	  print_log (LOGERR,"Could not send the data to the client. errno = %d\n",
		  errno);
	  DBTRACE(DBEXIT,("return/n server.c process_msg\n"));
	  STX_RETURN (-1);
	}
	free (tmpstr);
	tmpstr = NULL;

	if ((numbytes =
	     recv (new_fd, (void *)&add_HE_struct, size,
		   MSG_WAITALL)) <= 0) {
	  print_log
		  (LOGERR," Received -1. Closing socket %d. errno = %d (%s).Hot_Plug()\n",
	       new_fd, errno, strerror (errno));
	  fflush (stdout);
	  print_log (LOGERR,"recv");
	  sprintf(result_msg, "transfer_file: Error in recv call for FILE_TRANSFER");
	  send_err_msg (new_fd,
			result_msg,
			-1);
	  DBTRACE(DBEXIT,("return/o server.c process_msg\n"));
	  STX_RETURN (-1);
	}
#ifdef _a2e_h	/*400-EBCDIC*/
	numbytes = ascii2Ebcdic((char *)&add_HE_struct,(char *)&add_HE_struct,numbytes);
#endif
	print_log (LOGMSG,"Received the data to add the device. name = %s\n",
		add_HE_struct.sdev_id);
	H_device (add_HE_struct, result_msg);
	send_data (new_fd, msg_rcv.cmd, 1, 0, "HOTPLUG");
	break;



      case FILE_TRANSFER:
	print_log (LOGMSG,"command = %d. subcmd = %d. Format = %d.\n", msg_rcv.cmd,
		msg_rcv.subcmd, msg_rcv.indx);


	tmpstr = strtok (msg_rcv.str, "@");
	print_log (LOGMSG,"get_put = :%s:\n", tmpstr);
	fflush (stdout);
	strcpy (get_put, tmpstr);
	print_log (LOGMSG,"get_put = :%s:\n", get_put);
	fflush (stdout);

	tmpstr = strtok (NULL, "@");
	print_log (LOGMSG,"size = %s\n", tmpstr);
	fflush (stdout);
	xfer_size = atoi (tmpstr);

	if ((xfer_size > 0) && (strcmp (get_put, "put") == 0)) {       //daemon to get the file.
	  sprintf (xfer_name, "0@FILETRANSFERTOSUT");
	  tmpstr = (char *) malloc (100);
	  if (tmpstr == NULL) {
		print_log
		(LOGMSG,"Command FILE_TRANSFER. Error in malloc for tmpstr. errno =%d (%s).",
		 errno, strerror (errno));
		sprintf (result_msg,
		     "Command FILE_TRANSFER. Error in malloc for tmpstr. errno =%d (%s).",
		     errno, strerror (errno));
	    send_err_msg (new_fd, result_msg, -1);
	    break;
	  }
	  sprintf (msgptr, "%8x@%s", (strlen (xfer_name) + 1), xfer_name);
#ifdef _a2e_h	/*400-EBCDIC*/
	  ebcdic2Ascii(&msgptr[0],&msgptr[0],strlen(msgptr));
#endif
	  if (send (new_fd, msgptr, strlen (msgptr), 0) == -1) {
		print_log (LOGERR,"send failed\n");
	    DBTRACE(DBEXIT,("return/p server.c process_msg\n"));
	    STX_RETURN (-1);
	  }
	  print_log (LOGMSG,"send more: %s\n", msgptr);
	  fflush (stdout);
	  free (tmpstr);
	  tmpstr = NULL;
	}
	else if ((xfer_size == 0) && ((strcmp (get_put, "get") == 0))) {
	  sprintf (xfer_name, "0@FILETRANSFERFROMSUT");
	  tmpstr = (char *) malloc (100);
	  if (tmpstr == NULL) {
		print_log
		(LOGERR,"Command FILE_TRANSFER(1). Error in malloc for tmpstr. errno =%d (%s).",
		 errno, strerror (errno));
		sprintf (result_msg,
		     "Command FILE_TRANSFER(1). Error in malloc for tmpstr. errno =%d (%s).",
		     errno, strerror (errno));
	    send_err_msg (new_fd, result_msg, -1);
	    break;
	  }
	  sprintf (msgptr, "%8x@%s", (strlen (xfer_name) + 1), xfer_name);
#ifdef _a2e_h	/*400-EBCDIC*/
	    //	  ebcdic2Ascii(&msgptr[0],&msgptr[0],strlen(msgptr));
#endif
	    //send(new_fd, tmpstr, strlen(tmpstr), 0);
	  print_log (LOGMSG,"send more: %s\n", msgptr);
	  fflush (stdout);
	  free (tmpstr);
	  tmpstr = NULL;
	}
	else {
	  sprintf (xfer_name, "-1@Invalid message sent by client");
	  tmpstr = (char *) malloc (100);
	  if (tmpstr == NULL) {
		print_log
		(LOGERR,"Command FILE_TRANSFER(2). Error in malloc for tmpstr. errno =%d (%s).",
		 errno, strerror (errno));
		sprintf (result_msg,
		     "Command FILE_TRANSFER(2). Error in malloc for tmpstr. errno =%d (%s).",
		     errno, strerror (errno));
	    send_err_msg (new_fd, result_msg, -1);
	    break;
	  }
	  sprintf (msgptr, "%8x@%s", (strlen (xfer_name) + 1), xfer_name);
#ifdef _a2e_h	/*400-EBCDIC*/
	  ebcdic2Ascii(&msgptr[0],&msgptr[0],strlen(msgptr));
#endif
	  if (send (new_fd, msgptr, strlen (msgptr), 0) == -1) {
		print_log (LOGERR,"send failed\n");
	    DBTRACE(DBEXIT,("return/q server.c process_msg\n"));
	    STX_RETURN (-1);
	  }
	  print_log (LOGMSG,"send more: %s\n", msgptr);
	  fflush (stdout);
	  free (tmpstr);
	  tmpstr = NULL;
	  break;
	}

	tmpstr = strtok (NULL, "@");
	print_log (LOGMSG,"name = %s\n", tmpstr);
	fflush (stdout);
	strcpy (xfer_name, tmpstr);


	tmpstr = strtok (NULL, "@");
	//print_log(LOGMSG,"path = %s\n",tmpstr); fflush(stdout);
	strcpy (xfer_path, tmpstr);

	//xfer_mode = strtok(NULL,"@");
	//print_log(LOGMSG,"mode = %s\n",xfer_mode); fflush(stdout);



	transfer_file (new_fd, get_put, xfer_mode, xfer_path, xfer_name,
		       xfer_size);
	//transfer_file(new_fd, "put", xfer_mode, xfer_path, xfer_name, xfer_size);


	break;

	//case ADDSUT:
      case 9040:
	print_log (LOGMSG,"Adding ip to file\n");
	strcpy (curr_client_ip, msg_rcv.str);
	//send_broadcast (bcast_msg, "some time", "some time", new_fd);
	    //send_unicast(new_fd);
	break;

    case 2009:
    	print_log(LOGMSG, "Getting exer setup info\n");
    	sprintf (result_msg, " ");
    	if (shm_addr.hdr_addr == NULL) {
			sprintf(result_msg, "Ecg must be running to collect the exerciser setup info.");
		}
		else {
			num_entries = (shm_addr.hdr_addr)->max_entries;
			p_shm_HE_temp = (struct htxshm_HE  *)((shm_addr.hdr_addr) + 1);
			sprintf(result_msg, "setup_of_all_exers_done = 1");
		 	for(HE_count = 0; HE_count < num_entries; HE_count++) {
				if(p_shm_HE_temp->upd_test_id == 0 ) {
					sprintf(result_msg, "setup_of_all_exers_done = 0");
					break;
				}
				p_shm_HE_temp++;
			}	
		}
		send_data(new_fd, msg_rcv.cmd, 1, 0, result_msg);  
		break;

    default:
	send_err_msg (new_fd,
		      result_msg,
		      -1);
	break;
    }
	print_log (LOGMSG,"Done for loop. i = %d\n", i);
    fflush (stdout);
  }

      /*********** Truncate the daemon_log file *******************************/
      /* rc_stat = stat ("/tmp/daemon_log", &daemon_log_stat);

      if (rc_stat == -1) {
	 print_log (LOGERR,"Error in stat for daemon_log file. errno = %d(%s).\n", errno, strerror(errno));
	 fflush (stdout);
      }
      else if (daemon_log_stat.st_size >= DAEMON_LOG_WRAP) {
	 system_call = TRUE;
	 system("rm /tmp/daemon_log");
	 system_call = FALSE;
	sprintf(strlog,"echo \"\" > /tmp/daemon_log");
	system_call = TRUE;
	system(strlog);
	system_call = FALSE;

	else if (daemon_log_stat.st_size >= 1024*1024)
	   if (truncate("/tmp/daemon_log", 1024) == -1)
		print_log (LOGERR,"Error truncating daemon_log file. errno = %d(%s).\n", errno, strerror(errno));
	 else {
	 print_log (LOGMSG,"Truncating daemon_log file\n");
	 fflush(stdout); }
	  } */
	  /*********** Done: Truncate the daemon_log file *************************/



  print_log (LOGMSG,"ecg[0] = %p\n", ecg[0]);
  fflush (stdout);
   /*if (ecg[0] != NULL) {
      free(ecg[0]);
	  print_log(LOGMSG,"Freed ecg\n"); fflush(stdout);
      } */
  DBTRACE(DBEXIT,("return 0 server.c process_msg\n"));
  STX_RETURN (0);
}


void
send_data (int new_fd, int cmd, int num_disp, int result_rc, char *result_msg)
{
  char *tmpstr, tmpstr1[1024];
  char *str_5;
  int i, numbytes, siz_to_malloc;
  int len;
  char dt[20], dy[4], tmm[20], msgptr[1024];
  DBTRACE(DBENTRY,("enter server.c send_data\n"));

  if (is_ssm)
    is_cmdline = FALSE;
  if (is_ssm && cmd != 2020 && cmd != 2029) {
    if (query[0] == NULL) {
      query[0] = (char *) stx_malloc (60);
      if (query[0] == NULL) {
	print_log
		(LOGERR,"Command is_ssm. Error in malloc for query. errno =%d (%s).",
	     errno, strerror (errno));
	sprintf (result_msg,
		 "Command is_ssm. Error in malloc for query. errno =%d (%s).",
		 errno, strerror (errno));
	send_err_msg (new_fd, result_msg, -1);
	DBTRACE(DBEXIT,("return/a server.c send_data\n"));
	return;
      }
    }

    strcpy (query[0], "all");
    result_rc = query_all (result_msg, query, -2, &num_disp);
      /*query[0] will be freed in stx_free call */
    cmd = 9999;
  }

  siz_to_malloc = (num_disp * SIZE_SCN) + SIZE_HDR;
  if (siz_to_malloc < 30 * 1024)
    siz_to_malloc = 30 * 1024;
  print_log (LOGMSG,"Size to malloc = %d\n", siz_to_malloc);

  tmpstr = (char *) malloc (siz_to_malloc);
  if (tmpstr == NULL) {
	print_log
	(LOGERR,"Command send_data. Error in malloc for tmpstr. errno =%d (%s).",
	 errno, strerror (errno));
    fflush (stdout);
	sprintf (result_msg,
	     "Command send_data. Error in malloc for tmpstr. errno =%d (%s).",
	     errno, strerror (errno));
    send_err_msg (new_fd, result_msg, -1);
    DBTRACE(DBEXIT,("return/b server.c send_data\n"));
    return;
  }
  print_log (LOGMSG,"location of tmpstr = %x\n", tmpstr);
  memset (tmpstr, 0, siz_to_malloc);

  str_5 = (char *) malloc (siz_to_malloc);
  if (str_5 == NULL) {
	print_log (LOGERR,"Command send_data. Error in malloc for str_5. errno =%d (%s).",
	    errno, strerror (errno));
    fflush (stdout);
	sprintf (result_msg,
	     "Command send_data. Error in malloc for str_5. errno =%d (%s).",
	     errno, strerror (errno));
    send_err_msg (new_fd, result_msg, -1);
    if (tmpstr != NULL) {
      free (tmpstr);
      tmpstr = NULL;
    }
    DBTRACE(DBEXIT,("return/c server.c send_data\n"));
    return;
  }
  print_log (LOGMSG,"location of str_5 = %x\n", str_5);
  memset (str_5, 0, siz_to_malloc);

  print_log (LOGMSG,"in send_data for cmd = %d, before switch\n", cmd); fflush(stdout);
  if (ipc_done) {
    info_send.cur_info = *(rem_shm_addr->cur_shm_addr);
    info_send.sys_hdr_info = *(rem_shm_addr->sock_hdr_addr);
  }

  if (cmd != SCREEN_9_A) {
    if (!is_cmdline) {
	  print_log (LOGMSG,"Calling header scan\n");
      fflush (stdout);
      header_scn (str_5, cmd, result_rc, result_msg);
    }
  }

    /*** Add the additional header including the ECG information for SSM *****/
  if (is_ssm && (cmd != 2020)) {
	sprintf (tmpstr, "%c%d", sep_at, num_ecgs);
    strcat (str_5, tmpstr);
    for (i = 0; i < num_ecgs; i++) {
	/*if ( strcmp(full_name,"/ecg.all") == 0)*/
      if (i==0)
      {
	ecg_summary (tmpstr, is_ssm);
	sprintf (tmpstr, "%c%s%c%s/%s%c%s", sep_nl,
		 rem_shm_addr->sock_hdr_addr->system_status, sep_at, ecg_info[i].ecg_path,
		 ecg_info[i].ecg_name, sep_at, ecg_info[i].ecg_desc, sep_at);
	strcat (str_5, tmpstr);
      }
      else
      {
	sprintf (tmpstr, "%c%s%c%s/%s%c%s", sep_nl, ecg_info[i].ecg_status,
		 sep_at, ecg_info[i].ecg_path, ecg_info[i].ecg_name, sep_at,
		 ecg_info[i].ecg_desc);
	strcat (str_5, tmpstr);
      }
	}
  }
   /* Done: Add the additional header including the ECG information for SSM * */

/************************************************************************************/
  switch (cmd) {


    case 9999:
		//print_log(LOGMSG,"Preparing data for query. num_disp = %d\n", num_disp); fflush(stdout);
	    //sep_at = ' ';

      if (is_cmdline) {
	if ((result_rc == -1) || (num_disp == 0)) {
	  sprintf (tmpstr, "query:No Valid device or ecg name provided");
	  strcat (str_5, tmpstr);
	  print_log (LOGMSG,"for query cmdline: rc = %d disp = %d\n", result_rc,
		  num_disp);
	  fflush (stdout);
	  num_disp = 0;
	}

	else if (num_disp > 0) {
	  sprintf (tmpstr,
		   "--------------------------------------------------------------------------------\n");
	  strcat (str_5, tmpstr);
	  sprintf (tmpstr,
		   " Device   ST   ACTIVE COE Last Update  Count Stanza Last Error   Slot/  Num_errs\n");
	  strcat (str_5, tmpstr);
	  sprintf (tmpstr,
		   "              SUSPEND SOE day Time                  Day  Time    Port\n");
	  strcat (str_5, tmpstr);
	  sprintf (tmpstr,
		   "--------------------------------------------------------------------------------\n");
	  strcat (str_5, tmpstr);
	}
      }
      for (i = 0; i < num_disp; i++) {

	sprintf (tmpstr, "%c%-8s%c%3s%c%8s%c%3s", sep_nl,
		 info_send.scn_num.scn_all_info[i].sdev_id, sep_at,
		 info_send.scn_num.scn_all_info[i].status, sep_at,
		 info_send.scn_num.scn_all_info[i].status_ah, sep_at,
		 info_send.scn_num.scn_all_info[i].status_coe);
	strcat (str_5, tmpstr);

	get_time (info_send.scn_num.scn_all_info[i].tm_last_upd, dt, dy,
		  tmm);

	sprintf (tmpstr, "%c%s%c%s%c%-8llu%c%-3d", sep_at, dy, sep_at, tmm,
		 sep_at, info_send.scn_num.scn_all_info[i].cycles, sep_at,
		 info_send.scn_num.scn_all_info[i].test_id);
	strcat (str_5, tmpstr);

	get_time (info_send.scn_num.scn_all_info[i].tm_last_err, dt, dy,
		  tmm);
	if (info_send.scn_num.scn_all_info[i].tm_last_err != 0)
	  sprintf (tmpstr, "%c%s%c%s", sep_at, dy, sep_at, tmm);
	else
	  sprintf (tmpstr, "%c NA%c NA     ", sep_at, sep_at);
	strcat (str_5, tmpstr);

	sprintf (tmpstr, "%c%s%c%d",
		 sep_at, info_send.scn_num.scn_all_info[i].slot_port,
		 sep_at, info_send.scn_num.scn_all_info[i].num_errs);
	strcat (str_5, tmpstr);

	 /*sprintf(tmpstr,"%c%s",sep_at, info_send.scn_num.scn_all_info[i].device_desc);
	    strcat(str_5,tmpstr);

		sprintf(tmpstr,"%c%s",sep_at, info_send.scn_num.scn_all_info[i].adapt_desc);
	    strcat(str_5,tmpstr);

		sprintf(tmpstr,"%c%d-%d",sep_at,info_send.scn_num.scn_all_info[i].slot_port,info_send.scn_num.scn_all_info[i].port);
	    strcat(str_5,tmpstr);

		sprintf(tmpstr,"%c%d",sep_at, info_send.scn_num.scn_all_info[i].port);
	    strcat(str_5,tmpstr); */

      }
		//sep_at = ' ';
      break;

    case 9998:
	  print_log (LOGMSG,"in send_data for cmd = %d num_disp = %d\n", cmd, num_disp);
      fflush (stdout);
	  sprintf (tmpstr, "Total ECGS=%d%c", num_disp, sep_nl);
      strcpy (str_5, tmpstr);
      for (i = 0; i < num_disp; i++) {
	sprintf (tmpstr, "%s/%s%c", ecg_info[i].ecg_path,
		 ecg_info[i].ecg_name, sep_at);
	strcat (str_5, tmpstr);
      }
	  print_log (LOGMSG,"Sent for 9998 \n");
      fflush (stdout);
      break;

    case 9997:
	  print_log (LOGMSG,"in send_data for cmd = %d num_disp = %d\n", cmd, num_disp);
      fflush (stdout);
	  ecg_summary (str_5, 0);
		//sprintf(str_5,"%s",rem_shm_addr->sock_hdr_addr->test_summary);
	  print_log (LOGMSG,"Sent for 9997 \n");
      fflush (stdout);
      break;

    case 2015:
	  print_log (LOGMSG,"in send_data for cmd = %d num_disp = %d\n", cmd, num_disp);
      fflush (stdout);
      //send_broadcast (bcast_msg, "some time", "some time", (0-new_fd));
	    //test_summary ();
		//sprintf (str_5, "@UPDATE@mete5.austin.ibm.com@ecg.bu@ACTIVE@start_time@uptdate_time@0@err_time@err_dev@sut@0");
	  print_log (LOGMSG,"Sent for 2015 \n");
      fflush (stdout);
      break;

    case 9996:
	  print_log (LOGMSG,"in send_data for cmd = %d num_disp = %d\n", cmd, num_disp);
      fflush (stdout);
      test_summary (str_5);
		//sprintf (str_5, "%s", rem_shm_addr->sock_hdr_addr->test_summary);
	  print_log (LOGMSG,"Sent for 9996 \n");
      fflush (stdout);
      break;

    case 4000:
	  print_log (LOGMSG,"in send_data for cmd = %d num_disp = %d\n", cmd, num_disp);
      fflush (stdout);
	    //test_summary ();
	  sprintf (str_5, "BEEFDEAD");
	  print_log (LOGMSG,"Sent for 4000 \n");
      fflush (stdout);
      break;


    case 2000:
	  print_log (LOGMSG,"in send_data for cmd = %d num_disp = %d\n", cmd, num_disp);
      fflush (stdout);
	  sprintf (tmpstr, "%c%d", sep_at, num_ecgs);
      strcat (str_5, tmpstr);
	  for (i = msg_rcv.indx; i < msg_rcv.indx + num_disp; i++) {
		//print_log(LOGMSG,"path = :%s: name = :%s:\n",ecg_info[i].ecg_path, ecg_info[i].ecg_name);
	if ( strcmp(full_name,"/ecg.all") == 0)
	{
	  sprintf (tmpstr, "%c%s%c%s/%s%c%s %c ", sep_nl,
		   rem_shm_addr->sock_hdr_addr->system_status, sep_at, ecg_info[i].ecg_path,
		   ecg_info[i].ecg_name, sep_at, ecg_info[i].ecg_desc, sep_at);
	  strcat (str_5, tmpstr);
	  ecg_summary (tmpstr, is_ssm);
	  strcat (str_5, tmpstr);

	}
	else
	{
	  sprintf (tmpstr, "%c%s%c%s/%s%c%s", sep_nl, ecg_info[i].ecg_status,
		   sep_at, ecg_info[i].ecg_path, ecg_info[i].ecg_name, sep_at,
		   ecg_info[i].ecg_desc);
		//print_log(LOGMSG,"tmpstr = %s\n", tmpstr); fflush(stdout);
	  strcat (str_5, tmpstr);
	}
	  }
	  print_log (LOGMSG,"Sent for 2000 \n");
      fflush (stdout);
      break;

    case 2009:
    	print_log (LOGMSG,"in send_data for cmd = %d num_disp = %d\n", cmd, num_disp);
    	fflush (stdout);
    	len = sprintf (str_5, "%c%s\n", sep_at, result_msg);
    	str_5[len] = '\0';
    	break;

    case 2010:
	  print_log (LOGMSG,"in send_data for cmd = %d num_disp = %d\n", cmd, num_disp);
      fflush (stdout);
	  sprintf (tmpstr, "%c%d", sep_at, num_ecgs);
      strcat (str_5, tmpstr);
      for (i = msg_rcv.indx; i < msg_rcv.indx + num_disp; i++) {
	if ( strcmp(full_name,"/ecg.all") == 0)
	{
	  sprintf (tmpstr, "%c%s%c%s/%s%c%s %c ", sep_nl,
		   rem_shm_addr->sock_hdr_addr->system_status, sep_at, ecg_info[i].ecg_path,
		   ecg_info[i].ecg_name, sep_at, ecg_info[i].ecg_desc, sep_at);
	  strcat (str_5, tmpstr);
	  ecg_summary (tmpstr, is_ssm);
	  strcat (str_5, tmpstr);

	}
	else
	{
	  sprintf (tmpstr, "%c%s%c%s/%s%c%s", sep_nl, ecg_info[i].ecg_status,
		   sep_at, ecg_info[i].ecg_path, ecg_info[i].ecg_name, sep_at,
		   ecg_info[i].ecg_desc);
		//print_log(LOGMSG,"tmpstr = %s\n", tmpstr); fflush(stdout);
	  strcat (str_5, tmpstr);
	}
      }
	  print_log (LOGMSG,"Sent for 2010 \n");
      fflush (stdout);
      break;

    case 2020:
	  print_log (LOGMSG,"in send_data for cmd = %d num_disp = %d\n", cmd, num_disp);
      fflush (stdout);
	  sprintf (tmpstr, "%c%d", sep_at, num_ecgs);
      strcat (str_5, tmpstr);
      for (i = 0; i < num_ecgs; i++) {
	cur_ecg_pos = i;
	print_log (LOGMSG,"from summary...i = %d name = %s\n", cur_ecg_pos, ECGNAME);
	fflush (stdout);
	PUT_FULL_ECG;
	strcpy (ecg[0], full_name);
	num_ecg = 1;
	if ( strcmp(full_name,"/ecg.all") == 0) {
	  ecg_summary (tmpstr, is_ssm);
	  sprintf (tmpstr, "%c%s%c%s/%s%c%s ", sep_nl,
		   rem_shm_addr->sock_hdr_addr->system_status, sep_at, ecg_info[i].ecg_path,
		   ecg_info[i].ecg_name, sep_at, ecg_info[i].ecg_desc, sep_at);
	  strcat (str_5, tmpstr);
		    //ecg_summary (tmpstr, is_ssm);
	  strcat (str_5, tmpstr);
	}
	else {
	  sprintf (tmpstr, "%c%s%c%s/%s%c%s ", sep_nl,
		   ecg_info[i].ecg_status, sep_at, ecg_info[i].ecg_path,
		   ecg_info[i].ecg_name, sep_at, ecg_info[i].ecg_desc, sep_at);
	  strcat (str_5, tmpstr);
	  ecg_summary (tmpstr, is_ssm);
	  strcat (str_5, tmpstr);
	}
      }
      break;

/************************************************************************************/
    case 2001:
    case 2011:
    case 2021:
      get_time (-1, dt, dy, tmm);
      if (is_cmdline) {
	sprintf (str_5, "\n%s\n", result_msg);
      }
      else {
		//print_log(LOGMSG,"name = :%s:, path = :%s:\n",ECGNAME,ECGPATH);
	if ( strcmp(ECGNAME,"ecg.all") == 0)
	{
	  num_ecg=1;
	  ecg_summary (str_5, is_ssm);
	  sprintf (str_5, "%c%d%c%s%c%s%c0%c%s%c%s%c%s/%s%c%s%c\n", sep_at,
		   result_rc, sep_at, result_msg, sep_at, level_str, sep_at,
		   sep_at, dt, sep_at, tmm, sep_at, ECGPATH, ECGNAME, sep_at,
		   rem_shm_addr->sock_hdr_addr->system_status, sep_at);
	}
	else
	{
	  sprintf (str_5, "%c%d%c%s%c%s%c0%c%s%c%s%c%s/%s%c%s%c\n", sep_at,
		   result_rc, sep_at, result_msg, sep_at, level_str, sep_at,
		   sep_at, dt, sep_at, tmm, sep_at, ECGPATH, ECGNAME, sep_at,
		   ECGSTATUS, sep_at);
	}
      }
	  print_log (LOGMSG,"FOR 0 and 1. %s\n", str_5);
      fflush (stdout);
      break;
/************************************************************************************/

    case SCREEN_5:
		//print_log(LOGMSG,"Preparing data for Screen 5\n"); fflush(stdout);
	  if (is_cmdline) {
	if ((result_rc == -1) || (num_disp == 0)) {
	  print_log (LOGMSG,"for scn2 cmdline: rc = %d disp = %d\n", result_rc,
		  num_disp);
	  fflush (stdout);
	  sprintf (tmpstr, "%s\n", result_msg);
	  strcat (str_5, tmpstr);
	  num_disp = 0;
	}
	else {
	  sprintf (tmpstr,
		   "            Last Update     Cycle  Curr  Last Error\n");
	  strcat (str_5, tmpstr);
	  sprintf (tmpstr,
		   "ST Device   Day  Time       Count Stanza Day  Time\n");
	  strcat (str_5, tmpstr);
	}
      }
      for (i = 0; i < num_disp; i++) {

	get_time (INFO_SEND_5 (i).tm_last_upd, dt, dy, tmm);
	sprintf (tmpstr, "%c%-2s%c%-8s%c%s%c%s%c%7llu%c%7d", sep_nl,
		 INFO_SEND_5 (i).status, sep_at, INFO_SEND_5 (i).sdev_id,
		 sep_at, dy, sep_at, tmm, sep_at, INFO_SEND_5 (i).cycles,
		 sep_at, INFO_SEND_5 (i).test_id);
	strcat (str_5, tmpstr);


	if (!is_cmdline) {
	  sprintf (tmpstr, "%c%d", sep_at, INFO_SEND_5 (i).err_ack);
	  strcat (str_5, tmpstr);
	}

	get_time (INFO_SEND_5 (i).tm_last_err, dt, dy, tmm);
		//print_log(LOGMSG,"For scn 5: tm_err = %d:  %s \n",INFO_SEND_5(i).tm_last_err,INFO_SEND_5(i).sdev_id); fflush(stdout);
	if (INFO_SEND_5 (i).tm_last_err != 0)
	  sprintf (tmpstr, "%c%s%c%s", sep_at, dy, sep_at, tmm);
	else
	  sprintf (tmpstr, "%c   %c        ", sep_at, sep_at);

	strcat (str_5, tmpstr);
      }

      str_5[strlen (str_5)] = '\n';
      break;
/************************************************************************************/

    case 2029:
	  len = sprintf (str_5, "%c%s", sep_at, result_msg);
      str_5[len] = '\0';
      break;
    case SCREEN_9_A:
		//print_log(LOGMSG,"Preparing data for Screen 9_A\n");
      len =
	  sprintf (str_5, "%c%d%c%s", sep_at, result_rc, sep_at, result_msg);
      str_5[len] = '\n';
      break;
/************************************************************************************/

    case 9995:
      print_log (LOGMSG,"in send_data for cmd = %d result_msg = %s\n", cmd, result_msg);
      fflush (stdout);
      sprintf (str_5, "\n %s\n", result_msg);
      break;

    default:
	  print_log (LOGMSG,"in send_data for cmd = %d num_disp = %d\n", cmd, num_disp);
		//print_log(LOGMSG,"Preparing data for Screen 2 rc = %d disp = %d\n",result_rc,num_disp); fflush(stdout);
      if (is_cmdline) {
	if ((result_rc == -1) || (num_disp == 0)) {
	  sprintf (tmpstr, "%s\n", result_msg);
	  strcat (str_5, tmpstr);
	  num_disp = 0;
	}
	else {
	  sprintf (tmpstr,
		   "State   Dev     Adapt Desc   Device Desc     Slot   Port\n");
	  strcat (str_5, tmpstr);
	  sprintf (tmpstr,
		   "-----------------------------------------------------------\n");
	  strcat (str_5, tmpstr);
	}
      }
      for (i = 0; i < num_disp; i++) {

	if ((msg_rcv.cmd == SCREEN_2) || (msg_rcv.cmd == 2012)
		|| (msg_rcv.cmd == 2022) || (msg_rcv.cmd == 2032)) {
	  sprintf (tmpstr, "%c%s", sep_nl, INFO_SEND_2 (i).status_ah);
	  strcat (str_5, tmpstr);
	}
	else if ((msg_rcv.cmd == SCREEN_3) || (msg_rcv.cmd == 2013)
		 || (msg_rcv.cmd == 2023) || (msg_rcv.cmd == 2033)) {
	  sprintf (tmpstr, "%c%s", sep_nl, INFO_SEND_2 (i).status_coe);
	  strcat (str_5, tmpstr);
	}
	else if ((msg_rcv.cmd == SCREEN_9) || (msg_rcv.cmd == SCREEN_9_R)
		 || (msg_rcv.cmd == SCREEN_9_R_D)
		 || (msg_rcv.cmd == SCREEN_9_T)
		 || (msg_rcv.cmd == SCREEN_9_T_D)) {
	  sprintf (tmpstr, "%c%s", sep_nl, INFO_SEND_2 (i).status);
	  strcat (str_5, tmpstr);
	}

	sprintf (tmpstr, "%c%-8s%c%-11s%c%-15s%c%s", sep_at,
		 INFO_SEND_2 (i).sdev_id, sep_at, INFO_SEND_2 (i).adapt_desc,
		 sep_at, INFO_SEND_2 (i).device_desc, sep_at,
		 INFO_SEND_2 (i).slot_port);
	strcat (str_5, tmpstr);

      }
      str_5[strlen (str_5)] = '\n';
      break;
  }
/************************************************************************************/

  if (is_cmdline && (msg_rcv.cmd != 9995) && (msg_rcv.cmd != 2011) ) {   /* for these commands we do not want to attach active ecg name on output */
	attach_ecg_name_front (str_5);
  }
  sprintf (tmpstr, "%8x", strlen (str_5));
  print_log (LOGMSG,"str_5 size = %d, tmpstr size = %d\n",
             strlen(str_5), strlen(tmpstr));
  print_log (LOGMSG,"location of str_5 = %x tmpstr = %x\n", str_5,tmpstr);
  /*printf ("tmpstr = %s,str_5 = %s\n", tmpstr, str_5);*/
  if(is_cmdline) {
    sprintf (tmpstr, "%8x%8x%s", strlen (str_5), stx_error_no, str_5);
  } else {
    sprintf (tmpstr, "%8x%s", strlen (str_5), str_5);
  }
  print_log (LOGMSG,"size tmpstr = %d, str:%s:\n", strlen (tmpstr), tmpstr);

#ifdef _a2e_h	/*400-EBCDIC*/
  ebcdic2Ascii(&tmpstr[0],&tmpstr[0],strlen(tmpstr));
#endif
  numbytes = 0;
  while ( numbytes < strlen(tmpstr)) {
  	int send_bytes;

  	send_bytes = send (new_fd, (tmpstr + numbytes), (strlen(tmpstr) - numbytes), 0);
  	if ( send_bytes == -1 ) {
  		print_log (LOGERR,"Error in send. send_bytes = %d", send_bytes);
	}

	numbytes += send_bytes;
  }

  //print_log(LOGMSG,"sending java\n");
  print_log(LOGMSG,"sent bytes = %d  string = %s strlen = %d\n",numbytes, tmpstr,strlen(tmpstr));

  print_log (LOGMSG,"################## Sent %d bytes ######################\n\n\n",
	  numbytes);
  fflush (stdout);
  if (tmpstr != NULL) {
    free (tmpstr);
    tmpstr = NULL;
  }
  if (str_5 != NULL) {
    free (str_5);
    str_5 = NULL;
  }
  DBTRACE(DBEXIT,("return server.c send_data\n"));
  return;
}

int
sendall (int tmp_fd, char *buf, int *len)
{
  int sent, total = 0;
  int bytes_left = *len;
  DBTRACE(DBENTRY,("enter server.c sendall\n"));

#ifdef _a2e_h	/*400-EBCDIC*/
  ebcdic2Ascii(buf,buf,bytes_left);	/* This assumes *buf is char data not binary */
#endif
  while (total < *len) {
    sent = send (tmp_fd, (buf + total), bytes_left, 0);
    if (sent == -1)
      break;
    total += sent;
    bytes_left -= sent;
  }

  *len = total;
  DBTRACE(DBEXIT,("return server.c sendall\n"));
  return (sent == -1) ? -1 : 0;
}

void
header_scn (char *str_5, int cmd, int result_rc, char *result_msg)
{
  char tmpstr[512];
  char dt[20], dy[4], tmm[20];
  char version_11[] = "STX-V1.1";
  DBTRACE(DBENTRY,("enter server.c header_scn\n"));

  switch (cmd) {

    case 2000:
    case 2010:
	  sprintf (str_5, "%c%d", sep_at, result_rc);
      get_time (-1, dt, dy, tmm);
	  sprintf (tmpstr, "%c%s%c%s%c0%c%s%c%s", sep_at, result_msg, sep_at,
	       level_str, sep_at, sep_at, dt, sep_at, tmm);
      strcat (str_5, tmpstr);
      break;
    case 2020:
		//print_log(LOGMSG,"Preparing header for screen 0\n"); fflush(stdout);
	  sprintf (str_5, "%c%d", sep_at, result_rc);
      get_time (-1, dt, dy, tmm);
	  sprintf (tmpstr, "%c%s%c%s%c%s%c0%c%s%c%s", sep_at, result_msg, sep_at,
	       version_11, sep_at, level_str, sep_at, sep_at, dt, sep_at, tmm);
      strcat (str_5, tmpstr);
      break;
    case SCREEN_2:
    case 2012:
    case 2022:
    case 2032:
    case SCREEN_3:
    case 2013:

	  // print_log(LOGMSG,"Preparing header for screen 2\n"); fflush(stdout);

	  sprintf (str_5, "%c%d", sep_at, result_rc);
      get_time (-1, dt, dy, tmm);
	  sprintf (tmpstr, "%c%s%c%s%c0%c%s%c%s%c%d", sep_at, result_msg, sep_at,
	       level_str, sep_at, sep_at, dt, sep_at, tmm, sep_at,
		   NUM_ENTRIES);
      strcat (str_5, tmpstr);
      break;

    case 9999:
    case SCREEN_5:

		//print_log(LOGMSG,"Preparing header for screen 5\n"); fflush(stdout);
	      /****************************** Prepare Message ******************************/
	  sprintf (str_5, "%c%d", sep_at, result_rc);
      get_time (-1, dt, dy, tmm);
	  sprintf (tmpstr, "%c%s%c%s%c%s%c%s%c%s%c", sep_at, result_msg, sep_at,
	       level_str, sep_at, dy, sep_at, dt, sep_at, tmm, sep_at);
      strcat (str_5, tmpstr);

	    //get_time((rem_shm_addr->cur_shm_addr)->start_time,dt,dy,tmm);
      get_time (ECGSTARTTIME, dt, dy, tmm);
	  sprintf (tmpstr, "%s%c%s%c%s%c", dy, sep_at, dt, sep_at, tmm, sep_at);
      strcat (str_5, tmpstr);

	  sprintf (tmpstr, "%d%c%llu%c%llu%c%d%c%d",
	       (rem_shm_addr->cur_shm_addr)->error_flag, sep_at,
	       (rem_shm_addr->cur_shm_addr)->max_cycles_done, sep_at,
	       (rem_shm_addr->cur_shm_addr)->min_cycles_done, sep_at,
	       MAX_ENTRIES, sep_at, NUM_ENTRIES);
      strcat (str_5, tmpstr);
	      /****************************** Done: Prepare Message ******************************/

      break;

    case SCREEN_9:
    case SCREEN_9_R:
    case SCREEN_9_R_D:
    case SCREEN_9_T:
    case SCREEN_9_T_D:
		//print_log(LOGMSG,"Preparing header for screen 9\n"); fflush(stdout);
	  sprintf (str_5, "%c%d", sep_at, result_rc);
		//sprintf(str_5, "@%d",result_rc);

      get_time (-1, dt, dy, tmm);
	  sprintf (tmpstr, "%c%s%c%s%c0%c%s%c%s%c%d", sep_at, result_msg, sep_at,
	       level_str, sep_at, sep_at, dt, sep_at, tmm, sep_at,
	       NUM_ENTRIES);
      strcat (str_5, tmpstr);

		//print_log(LOGMSG,"From command 9: header = %s\n",str_5);
      break;
  }
	//print_log(LOGMSG,"return devid = %s\n",INFO_SEND_2(1).sdev_id);
  DBTRACE(DBEXIT,("return server.c header_scn\n"));
  return;
}

void
init_shm ()
{
  struct tm *p_tm;
  long clock;
  DBTRACE(DBENTRY,("enter server.c init_shm\n"));

  print_log (LOGMSG,"in init_shm****");
  fflush (stdout);

    //(rem_shm_addr->sock_hdr_addr)->running_halted = 99;
  clock = time ((long *) 0);
  p_tm = localtime (&clock);
  cpy_tm((rem_shm_addr->sock_hdr_addr)->curr_time , *p_tm)

      (rem_shm_addr->cur_shm_addr)->start_time = clock;    // *p_tm;
  (rem_shm_addr->cur_shm_addr)->error_flag = 0;
  (rem_shm_addr->cur_shm_addr)->max_cycles_done = 0;
  (rem_shm_addr->cur_shm_addr)->min_cycles_done = 0;
  rem_shm_addr->sock_hdr_addr->error_count = 0;
  hang_mon_PID = 0;
  hxstats_PID = 0;
  shutdown_flag = FALSE;
  print_log (LOGMSG,"done init_shm\n");
  fflush (stdout);

  DBTRACE(DBEXIT,("leave server.c init_shm\n"));
}

/*int send_blank_msg(int new_fd)
{
char str_blank[512],tmpstr[512];
char result_msg[200];
int rc,numbytes;
char dt[20],dy[4],tmm[20];
long clock;

	print_log(LOGMSG,"newfd = %d\n",new_fd);
    fflush(stdout);
    memset(result_msg,0,80);
    memset(str_blank,0,512);
    if (msg_rcv.cmd != SCREEN_1) {
       rc = -99;
       strcpy(result_msg, "No ECG File loaded.\nUse \"option 0 in Main Menu\" or \"esrv -ecg <ecgname>\" to specify ecg file.\n");
    }
    else {
       rc = -99;
	   strcpy(result_msg, " ");
    }

    get_time(-1, dt, dy, tmm);


	print_log(LOGMSG,"in sending blank\n");
    fflush(stdout);
    // ****************************** Prepare Message ******************************
	//print_log(LOGMSG,"newfd = %d: level = %s: rc = %d\n",new_fd,level_str,rc); fflush(stdout);
    if (is_cmdline) {
	   sprintf(str_blank,"%s",result_msg);
    }
    else {
	sprintf(str_blank,"%c%d%c%s%c",sep_at,rc,sep_at,result_msg,sep_at);
	sprintf(tmpstr, "%s%c",level_str,sep_at);
    strcat(str_blank,tmpstr);
	sprintf(tmpstr, "%s%c%s%c%s%c", dy, sep_at, dt, sep_at, tmm, sep_at);
    strcat(str_blank,tmpstr);
	sprintf(tmpstr, "000%c00/00/00%c00:00:00%cecg.bu%c99\n$$\n",sep_at,sep_at,sep_at,sep_at);
	strcat(str_blank,tmpstr);
    // ***************************** Done: Prepare Message *************************
    }
	//print_log(LOGMSG,"newfd = %d\n",new_fd); fflush(stdout);
    memset(tmpstr,0,512);
	sprintf(tmpstr,"%8x",(strlen(str_blank)+0));
	print_log(LOGMSG,"str_blank size = %d\n",(strlen(str_blank)+0));
    strcat(tmpstr,str_blank);
	print_log(LOGMSG,"size of blank message = %d\n",strlen(tmpstr)); fflush(stdout);
	//print_log(LOGMSG,"String = %s\n",tmpstr);

    if ((numbytes=send(new_fd,tmpstr, strlen(tmpstr),0)) <= 0) {
	print_log(LOGMSG,"Error sending data for blank message. numbytes = %d\n", numbytes); fflush(stdout);
	print_log(LOGERR,"send");
    }
	print_log(LOGMSG,"Sent %d bytes for blank message\n", numbytes); fflush(stdout);
    return 0;
}*/

int
init_ecg (char *result_msg)	/* //, int cur_ecg_pos)//, char *ecg_name) */
{
  char run_cmd[100], str[80], res_msg[200];
  struct stat ecgbu_stat;
  int rc_stat, is_ok, rc;
  struct sigaction sigvec;
  char *tmp1,*tmp2,tmp_ecgname[64];
  DBTRACE(DBENTRY,("enter server.c init_ecg\n"));

  is_ok = 0;
	//sprintf(result_msg,"%s will be used as the ecg file for this run.",ecg_name);
  sprintf (result_msg, "ECG = %s/%s .", ECGPATH, ECGNAME);
  if (strlen (ECGNAME) == 0) {
    rc = -1;
    is_ok = -1;
	print_log (LOGERR,"No ecg file provided. Will use ecg.bu as default file\n");
	sprintf (result_msg, "No ecg provided. Using ecg.bu file.");
    fflush (stdout);
    /* Changes for feature select ecg as well as mdt */
	strcpy(tmp_ecgname, ecg[0]);
	tmp1=dirname(tmp_ecgname);
    strcpy(tmp_ecgname, ecg[0]);
    tmp2=basename(tmp_ecgname);
    strcpy (ECGNAME, tmp2);
    strcpy (ECGPATH, tmp1);
  }

  sprintf (run_cmd, "%s/%s", ECGPATH, ECGNAME);
  print_log (LOGMSG,"init_Ecg: ECG name = %s/%s addr = 0x%x\n", ECGPATH, ECGNAME,
	  (int )&ECGNAME);
  rc_stat = stat (run_cmd, &ecgbu_stat);
  if (rc_stat == -1) {
    is_ok = -1;
	sprintf (str, "Error reading %s.errno = %d (%s).init_ecg()", run_cmd,
	     errno, strerror (errno));
	print_log (LOGERR,str, "Error reading %s.errno = %d (%s).init_ecg()", run_cmd,
	    errno, strerror (errno));
    fflush (stdout);
    memset (result_msg, 0, 80);
    strcpy (result_msg, str);
	//strcat(result_msg,str);
    if (stat ("/usr/lpp/htx/ecg/ecg.bu", &ecgbu_stat) == -1) {
      strcat (result_msg, "ecg.bu not found");
      running_status = 99;
      DBTRACE(DBEXIT,("return/a -2 server.c init_ecg\n"));
      return -2;
	}
    else {
	  print_log
	  (LOGERR,"Error reading ecg file.errno = %d (%s). Will use ecg.bu as default file\n",
	   errno, strerror (errno));
      strcat (result_msg, "Will use ecg.bu as default file");
      fflush (stdout);
      /* Changes for feature select ecg as well as mdt */
	  strcpy(tmp_ecgname, ecg[0]);
	  tmp1=dirname(tmp_ecgname);
      strcpy(tmp_ecgname, ecg[0]);
      tmp2=basename(tmp_ecgname);
      strcpy (ECGNAME, tmp2);
      strcpy (ECGPATH, tmp1);
    }
  }

  while (1) {
	  /*sprintf(run_cmd,"cp %s/%s /usr/lpp/htx/ecg/ecg",ECGPATH,ECGNAME);

	 sigvec.sa_handler = SIG_IGN;
	 sigaction(SIGCHLD, &sigvec, (struct sigaction *) NULL);
	 sigvec.sa_flags = SA_RESTART;

	 system_call = TRUE;
	 system(run_cmd);
	 system_call = FALSE;

	 sigvec.sa_handler = (void (*)(int)) child_death;
	 sigaction(SIGCHLD, &sigvec, (struct sigaction *) NULL);
	 sigvec.sa_flags = SA_RESTART; */

	//print_log(LOGMSG,"cmd = %s ::: ECG = %s/%s\n",run_cmd,ECGPATH,ECGNAME); fflush(stdout);

    if (!ipc_done) {
      if (init_rem_ipc (res_msg) != 0) {
	strcat (result_msg, res_msg);
	is_ok = -2;
	break;
      }
      else {
	init_shm ();
	    /*sigvec.sa_handler = SIG_IGN;
	    sigaction (SIGCHLD, &sigvec, (struct sigaction *) NULL);
	    sigvec.sa_flags = SA_RESTART;
		system_call = TRUE;
	    if(msg_rcv.cmd != 9999)
	    system ("/usr/lpp/htx/etc/scripts/exer_setup 2>/tmp/res_setup");
	    system_call = FALSE;
	    sigvec.sa_handler = (void (*)(int)) child_death;
	    sigaction (SIGCHLD, &sigvec, (struct sigaction *) NULL);
	    sigvec.sa_flags = SA_RESTART;
		print_log (LOGMSG,"Setting ipc flag ****\n");
	    fflush (stdout);*/
	ipc_done = 1;

      }
    }

	if(msg_rcv.cmd != 9999) {
	    sprintf(run_cmd,"/usr/lpp/htx/etc/scripts/exer_setup %s/%s 2>/tmp/res_setup", ECGPATH, ECGNAME);
        system_call = TRUE;
        system(run_cmd);
        system_call = FALSE;
    }
	//if (!sig_handle_done) {
		//print_log(LOGMSG,"CAlling build_ipc. cur_pos = %d\n",cur_ecg_pos); fflush(stdout);
	if (build_ipc (res_msg) != 0) {        /*  build IPC data structures */
      strcat (result_msg, res_msg);
      if ((strcmp (ECGSTATUS, "ACTIVE") == 0) || (strcmp (ECGSTATUS, "PARTIALLY RUNNING") == 0)) {
	active_ecg_name[0] = '\0';
      }
      strcpy(ECGSTATUS, "INACTIVE");
      is_ok = -2;
		//end_it(0);
      break;
    }
	    //}
      /*if (!sig_handle_done) {
	 sigvec.sa_handler = (void (*)(int)) child_death;
	 sigaction (SIGCHLD, &sigvec, (struct sigaction *) NULL);
	 sigvec.sa_flags = SA_RESTART;
	 sig_handle_done = TRUE;
      }*/

    if (!start_msg_done) {
	  print_log (LOGMSG,"Stating message handler\n");
      fflush (stdout);
      if (start_msg_hdl (0) != 0) {  /* start the msg handler program */
	is_ok = -2;
	break;
      }
      start_msg_done = 1;
	  print_log (LOGMSG,"Done start_msg_hdl ****");
      fflush (stdout);
    }

	//init_shm();
	print_log (LOGMSG,"Setting ipc flag ****\n");
    fflush (stdout);
	//ipc_done = 1;
    is_ok = TRUE;
    break;
  }
  DBTRACE(DBEXIT,("return server.c init_ecg\n"));
  /* printf("num_ecgs = %d\n",num_ecgs); */
  return is_ok;
}

int
send_err_msg (int fd, char *msg, int err_code)
{
  char errmsg[4096], str[4096];
  char dt[20], dy[4], tmm[20];
  int rc;
  DBTRACE(DBENTRY,("enter server.c send_err_msg\n"));

  rc = err_code;
  if (is_ssm)
    is_cmdline=FALSE;
  get_time (-1, dt, dy, tmm);
  if (is_cmdline) {
    if (msg_rcv.cmd != SCREEN_4) {
      get_running_ecg_name (errmsg);
      msg[4096-250-strlen (errmsg)] = '\0';
      sprintf (str, "%s\n%s", errmsg, msg);
    }
    else {
      msg[4096-250] = '\0';                           /* Just a precautionary check. Cant afford msg longer that 4096-250 */
      sprintf (str, "%s\n", msg);
    }
    if(err_code != 0) {
	stx_error_no = err_code;
    }
    sprintf (errmsg, "%8x%8x%s", strlen (str), abs(stx_error_no), str);
  }
  else {
    msg[4096-250] = '\0';                           /* Just a precautionary check. Cant afford msg longer that 4096-250 */
	print_log (LOGMSG," string may not be enough num_ecgs:%d\n", num_ecgs);
    if (num_ecgs > 0) {
	  sprintf (str, "%d%c%s%c%s%c0%c%s%c%s%c%s/%s%c%s%c\n", rc, sep_at,
	       msg, sep_at, level_str, sep_at, sep_at, tmm, sep_at, dt,
	       sep_at, ECGPATH, ECGNAME, sep_at, ECGSTATUS, sep_at);
	}
    else {
	  sprintf (str, "%d%c%s%c%s%c0%c%s%c%s%c%s%c%s%c\n", rc, sep_at, msg,
	       sep_at, level_str, sep_at, sep_at, tmm, sep_at, dt, sep_at,
	       "None", sep_at, "None", sep_at);

    }
    sprintf (errmsg, "%8x%c%s", (strlen (str) + 1), sep_at, str);
  }
  print_log (LOGMSG,"sending the error message. Date = %s Time = %s\n", dt,tmm);

#ifdef _a2e_h	/*400-EBCDIC*/
  ebcdic2Ascii(&errmsg[0],&errmsg[0],strlen(errmsg));
#endif
  if (send (fd, errmsg, strlen (errmsg), 0) == -1) {
	print_log (LOGERR,"send");
  }
  print_log (LOGMSG,"Sent the error message.\n");
  fflush (stdout);
  DBTRACE(DBEXIT,("return 0 server.c send_err_msg\n"));
  return 0;
}

int
get_time (long clk, char *dt, char *dy, char *tm)
{
  struct tm *p_tm;
  int yr2000;
  DBTRACE(DBENTRY,("enter server.c get_time\n"));

  if (clk == -1)
    clk = time ((long *) 0);

  p_tm = localtime (&clk);

  if (p_tm->tm_year > 99) {
    yr2000 = p_tm->tm_year - 100;
  }
  else {
    yr2000 = p_tm->tm_year;
  }

  sprintf (dt, "%.2d/%.2d/%.2d", (p_tm->tm_mon + 1), p_tm->tm_mday, yr2000);
  sprintf (dy, "%.3d", (p_tm->tm_yday + 1));
  sprintf (tm, "%.2d:%.2d:%.2d", p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);
  DBTRACE(DBEXIT,("return 0 server.c get_time\n"));
  return 0;
}


int
transfer_file (int new_fd, char *get_put, char *xfer_mode, char *tmp_path,
	       char *tmp_filename, int size)
{
  int rc, rc_stat, numbytes, i;
  int fileid, file_size;
  struct stat file_stat;
  char *file_xfer, *tmpstr;
  char *result_msg[80], filename[40];
  char err_msg[80];
  char dt[20],dy[4],tmm[20];
  char version_11[]="STX-V1.1";
  char ecg_name_str[512];
  int ecg_name_len;

  DBTRACE(DBENTRY,("enter server.c transfer_file\n"));

  print_log(LOGMSG,"In transfer file. File to %s is %s%s\n", get_put, tmp_path, tmp_filename); fflush(stdout);
  if (is_ssm)
    is_cmdline = FALSE;

  rc = 0;
  memset (filename, 0, 40);
  sprintf (filename, "%s/%s", tmp_path, tmp_filename);
  filename[strlen (tmp_filename) + strlen (tmp_path) + 1] = '\0';
  print_log (LOGMSG,"FILENAME = :%s: get_put=:%s:\n", filename, get_put);
  fflush (stdout);

  if (strcmp (get_put, "get") == 0) {  //daemon to put the file on the client machine.
      /*** Open the file ***********/
    fileid = open (filename, O_RDONLY);
    rc_stat = stat (filename, &file_stat);
    if (rc_stat != 0) {
	  sprintf (result_msg,
		   "Error in stat() for filename = %s. errno = %d (%s).transfer_file()",
	       filename, errno, strerror (errno));
	  print_log
	  (LOGERR,"transfer_file:Error in stat() for filename = %s. errno = %d.transfer_file()",
	   filename, errno);
      send_err_msg (new_fd, result_msg, -1);
      close (fileid);
      DBTRACE(DBEXIT,("return/a -1 server.c transfer_file\n"));
      return -1;
    }
    /*** Done: Open the file ***********/

    /************ Malloc the size. ***********/
    size = file_stat.st_size + 1024;

    if (is_ssm)
      size = size + num_ecgs*(128+64+20); //(name=128,desc=64,status=20)

	print_log (LOGMSG,"Size of file %s = %d\n", filename, size);
    fflush (stdout);

    file_xfer = (char *) malloc (size);
    if (file_xfer == NULL) {
	  print_log
	  (LOGERR,"transfer_file: Malloc error for file_xfer. errno = %d (%s).transfer_file()",
	   errno, strerror (errno));
	  sprintf (result_msg,
	      	 "Malloc error for file_xfer. errno = %d (%s).transfer_file()",
	       errno, strerror (errno));
      send_err_msg (new_fd, result_msg, -1);
      close (fileid);
      DBTRACE(DBEXIT,("return/b -1 server.c transfer_file\n"));
      return -1;
    }

    if (is_cmdline) {
    	get_running_ecg_name (ecg_name_str);
    	strcat (ecg_name_str, "\n");
    	ecg_name_len = strlen (ecg_name_str);
    	tmpstr = (char *) malloc (ecg_name_len + size);
    }
    else {
    	tmpstr = (char *) malloc (size);
    }

    if (tmpstr == NULL) {
	  print_log
	  (LOGERR,"transfer_file: Malloc error for tmpstr. errno = %d (%s).transfer_file()",
	   errno, strerror (errno));
	  sprintf (result_msg,
		   "Malloc error for tmpstr. errno = %d (%s).transfer_file()",
	       errno, strerror (errno));
      send_err_msg (new_fd, result_msg, -1);
      close (fileid);
      DBTRACE(DBEXIT,("return/c -1 server.c transfer_file\n"));
      return -1;
    }
    if (is_cmdline) {
    	memset (tmpstr, 0, ecg_name_len + size);
    }
    else {
    	memset (tmpstr, 0, size);
    }
    memset (file_xfer, 0, size);
    /***************** Done: Malloc the size. ***********/
    if (!is_cmdline)
	  sprintf (file_xfer, "%d%c%s%c", rc, sep_at, "No Message", sep_at);   // Initialize message

    /********This piece of code only executes for stxgui **************/
    (void)stxgui_getecginfo_hdr(file_xfer, tmpstr);
    /*if (is_ssm) {
       get_time(-1, dt, dy, tmm);
	   sprintf (tmpstr, "%s%c%s%c%s%c%s%c", version_11,sep_at,level_str, sep_at, dy, sep_at, tmm, sep_at);
       strcat (file_xfer, tmpstr);

	   ecg_summary(tmpstr, 0);
	   sprintf (tmpstr, "%d", num_ecgs);
       strcat (file_xfer, tmpstr);
       for (i = 0; i < num_ecgs; i++) {
	   if (i==0) {
		  sprintf (tmpstr, "%c%s%c%s/%s%c%s", sep_nl,
		      rem_shm_addr->sock_hdr_addr->system_status, sep_at, ecg_info[i].ecg_path,
		      ecg_info[i].ecg_name, sep_at, ecg_info[i].ecg_desc);
	      strcat (file_xfer, tmpstr);
	   }
	   else {
		  sprintf (tmpstr, "%c%s%c%s/%s%c%s", sep_nl, ecg_info[i].ecg_status,
		      sep_at, ecg_info[i].ecg_path, ecg_info[i].ecg_name, sep_at,
		      ecg_info[i].ecg_desc);
	      strcat (file_xfer, tmpstr);
	   }
       }
	   sprintf (tmpstr, "%c", sep_nl);
       strcat (file_xfer, tmpstr);
    }*/
	/********Done: This piece of code only executes for stxgui **************/

    if ((file_size =
	 read (fileid, (file_xfer + strlen (file_xfer)), (size-1024))) == -1) {
		  /* sprintf ((char *)result_msg,
		       "Error reading %s file. errno = %d (%s).transfer_file()\n",
		       filename, errno, strerror (errno));*/
	  sprintf ((char *)result_msg, "Error reading %s file. errno = %d (%s).transfer_file()\n",
	       strstr (filename, "htx"), errno, strerror (errno));
	  sprintf (file_xfer, "%d%c%s%c", rc, sep_at, (char *)result_msg, sep_at);
	 //sprintf (tmpstr, "%d%c%s%c", rc, sep_at, (char *)result_msg, sep_at);
	 //strcat(file_xfer, tmpstr);
      if (is_ssm) {
	(void)stxgui_getecginfo_hdr(file_xfer, tmpstr);
      }

      strcat (file_xfer, "\n");
	  print_log (LOGERR,"Error reading %s file. errno = %d (%s).transfer_file()\n",
	      filename, errno, strerror (errno));
      fflush (stdout);
	}

	print_log(LOGMSG,"file_size = %d path = %s\n",file_size,filename); fflush(stdout);
    if (file_size == 0) {
	  sprintf ((char *)result_msg, "%s file is empty\n", strstr (filename, "htx"));
	  sprintf (file_xfer, "%d%c%s%c", rc, sep_at, (char *)result_msg, sep_at);
      if (is_ssm) {
	(void)stxgui_getecginfo_hdr(file_xfer, tmpstr);
      }
      strcat (file_xfer, "\n");
    }

    if (is_cmdline) {
	sprintf (tmpstr, "%8x%8x%s%s", ecg_name_len + strlen (file_xfer), stx_error_no,
	     ecg_name_str, file_xfer);
    }
    else {
	sprintf (tmpstr, "%8x%c%s", (strlen (file_xfer) + 1), sep_at,
	     file_xfer);
    }
	print_log (LOGMSG,"Sending File of %d length...", strlen (tmpstr));
    fflush (stdout);
#ifdef _a2e_h	/*400-EBCDIC*/
    ebcdic2Ascii(&tmpstr[0],&tmpstr[0],strlen(tmpstr));
#endif

    numbytes = 0;
    while ( numbytes < strlen(tmpstr)) {
		int send_bytes;

        send_bytes = send (new_fd, (tmpstr + numbytes), (strlen(tmpstr) - numbytes), 0);
        if ( send_bytes == -1 ) {
	        print_log (LOGERR,"Error in send. send_bytes = %d", send_bytes);
		}

		numbytes += send_bytes;
	}
	print_log (LOGMSG,"Sent %d bytes\n", numbytes);
    fflush (stdout);
    free (tmpstr);
    tmpstr = NULL;
    close (fileid);
    free (file_xfer);
    file_xfer = NULL;
  }

  else if (strcmp (get_put, "put") == 0) {
    rc = 0;
	print_log (LOGMSG,"Getting file of size = %d\n", size);
    fflush (stdout);
    file_xfer = (char *) malloc (size);
    if (file_xfer == NULL) {
	  print_log
	  (LOGERR,"transfer_file: Malloc error for file_xfer. errno = %d (%s). transfer_file",
	   errno, strerror (errno));
	  sprintf (result_msg,
	       "Malloc error for file_xfer. errno = %d (%s). transfer_file",
	       errno, strerror (errno));
      send_err_msg (new_fd, result_msg, -1);
      DBTRACE(DBEXIT,("return/d 0 server.c transfer_file\n"));
      return 0;
    }

	if ((numbytes = recv (new_fd, file_xfer, size, MSG_WAITALL)) == -1) {
	  print_log
	  (LOGERR," Received -1. Closing socket %d. errno = %d (%s).transfer_file()\n",
	   new_fd, errno, strerror (errno));
	  fflush (stdout);
	  print_log (LOGERR,"recv");
	  sprintf(result_msg, "transfer_file: Error in recv call for FILE_TRANSFER");
	  send_err_msg (new_fd,
			result_msg,
			-1);
	  DBTRACE(DBEXIT,("return/e 0 server.c transfer_file\n"));
	  return 0;
	}

#ifdef _a2e_h	/*400-EBCDIC*/
	numbytes = ascii2Ebcdic(file_xfer,file_xfer,numbytes);
#endif
	print_log (LOGMSG,"Size = %d. numbytes = %d\n", size, numbytes);
    fflush (stdout);
    fileid = open (filename, O_WRONLY | O_CREAT);
    if ((file_size = write (fileid, file_xfer, size)) == -1) {
	  print_log (LOGERR,"transfer_file: Error writing to %s file. errno = %d (%s)\n",
	      filename, errno, strerror (errno));
	  sprintf ((char *)result_msg,
	       "Error Writing to %s file. errno = %d (%s).transfer_file\n",
	       filename, errno, strerror (errno));
      send_err_msg (new_fd, (char *)result_msg, -1);
      fflush (stdout);
    }

       /******* Allocate memory **********/
	sprintf ((char *)result_msg, "%d%cWrote %d bytes to File %s.", rc, sep_at,
	     file_size, filename);
	tmpstr = (char *) malloc (100);
    if (tmpstr == NULL) {
	  print_log (LOGERR,"Error in Malloc. errno = %d (%s). transfer_file",
	      errno, strerror (errno));
	  sprintf ((char *)result_msg,
	       "-1%cfile_transfer: Error in Malloc. errno = %d (%s). transfer_file",
	       sep_at, errno, strerror (errno));
      send_err_msg (new_fd, (char *)result_msg, -1);
      close (fileid);
      DBTRACE(DBEXIT,("return/f 0 server.c transfer_file\n"));
      return 0;
    }
    memset (tmpstr, 0, 100);
       /******* Done:  Allocate memory **********/
	sprintf (tmpstr, "%8x%c%s", (strlen ((char *)result_msg) + 1), sep_at,
	     (char *)result_msg);
#ifdef _a2e_h	/*400-EBCDIC*/
    ebcdic2Ascii(&tmpstr[0],&tmpstr[0],strlen(tmpstr));
#endif
    if ((numbytes = send (new_fd, tmpstr, strlen (tmpstr), 0)) == -1) {
	  print_log (LOGERR,"send");
    }
    free (tmpstr);
    tmpstr = NULL;
    close (fileid);
    free (file_xfer);
    file_xfer = NULL;
  }
  DBTRACE(DBEXIT,("return 0 server.c transfer_file\n"));
  return 0;
}

int
parsefile (int new_fd, int *num, char data[][80], int cmd, char *filename,
	   int size, int fileid)
{
  char *str[80];
  char *cmd_data;
  char result_msg[80];
  int i;
  int rc_stat;
  struct stat file_stat;
  DBTRACE(DBENTRY,("enter server.c parsefile\n"));

  if (cmd == FILEONSUT) {
	print_log (LOGMSG,"Command file Name is %s\n", filename);
    fflush (stdout);
    rc_stat = stat (filename, &file_stat);
	//print_log(LOGMSG,"rc_stat = %d\n",rc_stat); fflush(stdout);
    if (rc_stat != 0) {
	  print_log (LOGERR,"stat error. filename = %s. errno = %d (%s). parsefile()",
	      filename, errno, strerror (errno));
      fflush (stdout);
	  sprintf (result_msg,
	       "Error in stat call for filename = %s. errno = %d (%s).parsefile()",
	       filename, errno, strerror (errno));
      send_err_msg (new_fd, result_msg, -1);
      *num = -1;
      DBTRACE(DBEXIT,("return/a 0 server.c parsefile\n"));
      return 0;
	}

    fileid = open (filename, O_RDONLY);
    size = file_stat.st_size;
    cmd_data = (char *) malloc (size + 2);
    if (cmd_data == NULL) {
	  print_log (LOGERR,"Error in Malloc for cmd_data. errno = %d (%s). parsefile",
	      errno, strerror (errno));
	  sprintf (result_msg,
	       "Error in Malloc for cmd_data. errno = %d (%s). parsefile",
	       errno, strerror (errno));
      send_err_msg (new_fd, result_msg, -1);
      close (fileid);
      DBTRACE(DBEXIT,("return/b 0 server.c parsefile\n"));
      return 0;
    }

	print_log (LOGMSG,"Size of file %s = %d\n", filename, size);
    fflush (stdout);
    if ((read (fileid, cmd_data, size)) == -1) {
	  print_log (LOGERR,"Error reading %s file. errno = %d (%s). parsefile()\n",
	      filename, errno, strerror (errno));
	  sprintf (result_msg,
	       "Error reading %s file. errno = %d (%s). parsefile()\n",
	       filename, errno, strerror (errno));
      fflush (stdout);
    }
    size--;
    close (fileid);
  }
  else {
	print_log (LOGMSG,"Size = %d. filename = :%s: cmd_data = 0x%x msgrcv = 0x%x\n",
	    size, filename, (int)cmd_data, (int)&msg_rcv);
    fflush (stdout);
    cmd_data = (char *) malloc (size + 2);
    if (cmd_data == NULL) {
	  print_log (LOGERR,"Error in Malloc for cmd_data. errno = %d (%s). parsefile()",
	      errno, strerror (errno));
      fflush (stdout);
	  sprintf (result_msg,
		   "Error in Malloc for cmd_data. errno = %d (%s). parsefile()",
	       errno, strerror (errno));
      send_err_msg (new_fd, result_msg, -1);
      *num = -1;
      DBTRACE(DBEXIT,("return/c 0 server.c parsefile\n"));
      return 0;
    }
	print_log (LOGMSG,"Malloc for cmd_data successfule\n");
    fflush (stdout);
    strncpy (cmd_data, filename, size);
    cmd_data[size] = '\0';
  }

  print_log (LOGMSG,"cmd = %s:::\n", cmd_data);
  fflush (stdout);
	//print_log(LOGMSG,"last char is  = %c\n", cmd_data[size]); fflush(stdout);
  if (cmd_data[size] != ';') {
	//print_log(LOGMSG,"last char is not ;, = %c\n", cmd_data[size]); fflush(stdout);
    cmd_data[size] = ';';
    cmd_data[size + 1] = '\0';
  }
	//print_log(LOGMSG,"last char now is  = %c\n", cmd_data[size]); fflush(stdout);

  i = 0;
  str[i] = strtok (cmd_data, ";");
  strncpy (data[i], str[i], strlen (str[i]));
  print_log (LOGMSG,"i = %d, str = %s, len = %d, data = %s\n", i, str[i],
	  strlen (str[i]), (char *)(data + i));
  fflush (stdout);

  do {
    i++;
    str[i] = strtok ((char *) 0, ";");
    if (str[i] == (char *) 0) {
      i--;
      break;
    }
    strncpy ((char *)(data + i), str[i], strlen (str[i]));
	print_log (LOGMSG,"i = %d, str = %s, len = %d, data = %s\n", i, str[i],
	    strlen (str[i]), (char*)(data + i));
	fflush (stdout);
  } while (str[i] != (char *) 0);

  if (cmd_data != NULL) {
    free (cmd_data);
    cmd_data = NULL;
  }

  *num = (i + 1);
  DBTRACE(DBEXIT,("return 0 server.c parsefile\n"));
  return 0;
}

int
create_tempfile (int new_fd)
{
  int fl;
  char result_msg[80];
  char template[] = "stx_XXXXXX";
  DBTRACE(DBENTRY,("enter server.c create_tempfile\n"));

  fl = mkstemp (template);
  print_log (LOGMSG,"fl = %d\n", fl);

  if (write (fl, (char *) &msg_rcv, strlen ((char *) &msg_rcv)) == -1) {
	print_log
	(LOGERR,"Error writing to temporary file. errno = %d (%s). create_tempfile\n",
	 errno, strerror (errno));
    fflush (stdout);
	sprintf (result_msg,
	     ": Error Writing to temporary file. errno = %d (%s). create_tempfile()\n",
	     errno, strerror (errno));
    send_err_msg (new_fd, result_msg, -1);
    fflush (stdout);
  }
  DBTRACE(DBEXIT,("return server.c create_tempfile\n"));
  return fl;
}

int
receive_alld (int sock, char *str_rcv)
{
  int size_rcv, numbytes;
  char str_size[10];
  DBTRACE(DBENTRY,("enter server.c receive_alld\n"));

	//print_log(LOGMSG,"Receiving 8 bytes\n"); fflush(stdout);

  if ((numbytes = recv (sock, &str_size[0], 8, 0)) <= 0) {
	print_log (LOGMSG,"Received %d Bytes. errno = %d. Not a good command.\n", numbytes, errno);
    good_cmd = 0;
	print_log (LOGERR,"recvfrom");
    if (errno == EAGAIN) {
	  print_log (LOGMSG,"\nstxdaemon timeout:No data received from stx client.\n");
      DBTRACE(DBEXIT,("return/a -1 server.c receive_alld\n"));
      return (-1);
    }
	print_log (LOGMSG,"Received %d Bytes.\n", numbytes);
	print_log (LOGMSG,"Shutting Down the client\n");
    DBTRACE(DBEXIT,("return/b server.c receive_alld\n"));
	return numbytes;
  }
#ifdef _a2e_h	/*400-EBCDIC*/
  numbytes = ascii2Ebcdic(&str_size[0],&str_size[0],numbytes);
#endif
  print_log (LOGMSG,"Receiving %d bytes. size = %s\n", numbytes, str_size);
  fflush (stdout);
  if (numbytes < 8) {          // the other side has closed the connection,
    good_cmd = 0;
    DBTRACE(DBEXIT,("return/c 0 server.c receive_alld\n"));
    return 0;                 // skip second receive and retrun 0
  }
  good_cmd = 1;
  str_size[8] = '\0';
  sscanf (str_size, "%x", &size_rcv);
  print_log (LOGMSG,"Receiving ... %d\n", size_rcv);
  memset (str_rcv, 0, size_rcv);
  if ((numbytes = recv (sock, str_rcv, (size_rcv), 0)) == -1) {
	print_log (LOGMSG,"Received -1 bytes. \n");
	print_log (LOGERR,"recvfrom");
	DBTRACE(DBEXIT,("return/d server.c receive_alld\n"));
    return numbytes;
  }
#ifdef _a2e_h	/*400-EBCDIC*/
  numbytes = ascii2Ebcdic(&str_rcv[0],&str_rcv[0],numbytes);
#endif
  print_log (LOGMSG,"Received ... %s\n", str_rcv);
  DBTRACE(DBEXIT,("return server.c receive_alld\n"));
  return numbytes;
}


/* This function counts the total number of entries in a directory. */
int get_directory_entries(char *dir_path)
{
	int 		dir_entry_count = 0;
	DIR *           dir_ptr;
	struct dirent * dir_entry_ptr;


	dir_ptr = opendir(dir_path);
	if(dir_ptr != NULL) {
		while( (dir_entry_ptr = readdir(dir_ptr) ) ) {
			dir_entry_count++;
		}
		close(dir_ptr);
	}

	return dir_entry_count;
}



/* This function calcualtes and return number of ecg_info entries  
   to be allocated. The number is sum of ecgs and mdts + 25 percent
   the sum.
*/
int get_number_of_ecg_info_to_be_created()
{
	int	total_ecg_count;
	double	temp_count;
	int	ecg_info_count;


	total_ecg_count = get_directory_entries("/usr/lpp/htx/mdt") + get_directory_entries("/usr/lpp/htx/ecg");
	temp_count = total_ecg_count + (.25 * total_ecg_count);
	ecg_info_count = (int) temp_count;

	return ecg_info_count;
}

int
init_ecg_info ()
{
  int max_disp;
  int ecg_info_count;


  DBTRACE(DBENTRY,("enter server.c init_ecg_info\n"));

  /* printf("Entering init_ecg_info, num_ecgs = %d\n",num_ecgs); */

  if (num_ecgs == 0) {
    ecg_info_count = get_number_of_ecg_info_to_be_created();

    if(ecg_info_count > MAX_ECG_ENTRIES) {
       MAX_ECG_ENTRIES = ecg_info_count;
    }
      /********** initialize the main shared memory ***********************/

    ecg_info = (tecg_struct *) malloc (MAX_ECG_ENTRIES * sizeof (tecg_struct));
    if (ecg_info == NULL) {   /* problem?                          */
	  sprintf (result_msg,
	       "####ERROR: Unable to Malloc for ecg_info.###ERRNO = %d (%s)",
	       errno, strerror (errno));
	  print_log
	  (LOGERR,"####ERROR: Unable to Malloc for ecg_info.###ERRNO = %d (%s)",
	   errno, strerror (errno));
      exit (40);
    }                         /* endif */
    bzero (ecg_info, (sizeof (tecg_struct) * MAX_ECG_ENTRIES));
	print_log(LOGMSG,"MAX_EXER_ENTRIES:%d\n MAX_ECG_ENTRIES:%d\n DAEMON_LOG_WRAP:%d\n",
	   MAX_EXER_ENTRIES,MAX_ECG_ENTRIES,DAEMON_LOG_WRAP);
    shm_id = shmget (SHMKEY, MAX_EXER_ENTRIES * sizeof (texer_list), IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);  /* system shared memory              */
	if (shm_id == -1) {       /* problem?                          */
	  sprintf (result_msg,
	       "####ERROR: Unable to get shared memory for exers.###ERRNO = %d (%s)",
	       errno, strerror (errno));
	  print_log
	  (LOGERR,"#######ERROR: Unable to get shared memory.#######Errno = %d (%s)",
	   errno, strerror (errno));
      fflush (stdout);
      exit (42);
    }                         /* endif */

      /* attach shared memory */
    ecg_info[0].exer_list = (texer_list *) shmat (shm_id, (char *) 0, 0);
	print_log (LOGMSG,"ecg_info: addr = 0x%x id = %d\n", (int)ecg_info, (int)shm_id);
    fflush (stdout);
    if ((int) ecg_info[0].exer_list == -1) {  /* problem? */
	  sprintf (result_msg,
	       "Unable to attach shared memory(%d, 0x%x). Errno = %d (%s)\n",
	       shm_id, shm_id, errno, strerror (errno));
	  print_log
	  (LOGERR,"Unable to attach shared memory(%d, 0x%x). Errno = %d (%s)\n",
	   shm_id, shm_id, errno, strerror (errno));
      fflush (stdout);
      exit (43);
    }                         /* endif */
    bzero (ecg_info[0].exer_list, (sizeof (texer_list) * MAX_EXER_ENTRIES));
    cur_ecg_pos = 0;
    ECG_MAX_ENTRIES = 0;

    dup_info = malloc (sizeof (tdup_struct) * MAX_EXER_ENTRIES);
    if (dup_info == NULL) {   /* problem?                          */
	  sprintf (result_msg,
	       "####ERROR: Unable to malloc for dup_info .###ERRNO = %d (%s)",
	       errno, strerror (errno));
	  print_log
	  (LOGERR,"####ERROR: Unable to malloc for dup_info .###ERRNO = %d (%s)",
	   errno, strerror (errno));
      exit (44);
    }                         /* endif */

	print_log (LOGMSG,"got the main shared memory ecg_info[0].exer_list: addr:0x%x\n",
	    (int)ecg_info[0].exer_list);
      /*****************************************************/

    info_send.scn_num.scn_all_info =
	(tscn_all *) malloc (MAX_EXER_ENTRIES * sizeof (tscn_all));
	print_log (LOGMSG,"Callinf ecg_get\n");
    fflush (stdout);
    max_disp = ecg_get ();

  }
  DBTRACE(DBEXIT,("return server.c init_ecg_info\n"));
  /* printf("max_disp = %d\n",max_disp); */
  return(max_disp);
}

char *
stx_malloc (int size)
{
  char *ptr;
  DBTRACE(DBENTRY,("enter server.c stx_malloc\n"));

  ptr = malloc (size);
  if (ptr == NULL) {
	print_log (LOGERR,"malloc failed. size = %d errno =%d\n", size, errno);
    DBTRACE(DBEXIT,("return/a server.c stx_malloc\n"));
    return ptr;
  }
  malloc_array[malloc_index++] = ptr;
  DBTRACE(DBEXIT,("return server.c stx_malloc\n"));
  return ptr;
}

void
stx_free ()
{
  int i;
  DBTRACE(DBENTRY,("enter server.c stx_free\n"));

  print_log (LOGMSG,"freeing up the malloc_array\n");
  fflush (stdout);

  for (i = 0; i < malloc_index; i++) {
    if (malloc_array[i] != NULL) {
      free (malloc_array[i]);
      malloc_array[i] = 0;
    }
  }
  malloc_index = 0;
  DBTRACE(DBEXIT,("leave server.c stx_free\n"));
}

void stxgui_getecginfo_hdr(char *file_xfer, char *tmpstr)
{
  int i;
  char dt[20],dy[4],tmm[20];
  char version_11[]="STX-V1.1";


  if (is_ssm) {
    get_time(-1, dt, dy, tmm);
	sprintf (tmpstr, "%s%c%s%c%s%c", level_str, sep_at, dy, sep_at, tmm, sep_at);
	   //sprintf (tmpstr, "%s%c%s%c%s%c%s%c", version_11,sep_at,level_str, sep_at, dy, sep_at, tmm, sep_at);
    strcat (file_xfer, tmpstr);

    ecg_summary(tmpstr, 0);
	sprintf (tmpstr, "%d", num_ecgs);
    strcat (file_xfer, tmpstr);
    for (i = 0; i < num_ecgs; i++) {
      if (i==0) {
	sprintf (tmpstr, "%c%s%c%s/%s%c%s", sep_nl,
		 rem_shm_addr->sock_hdr_addr->system_status, sep_at, ecg_info[i].ecg_path,
		 ecg_info[i].ecg_name, sep_at, ecg_info[i].ecg_desc);
	strcat (file_xfer, tmpstr);
      }
      else {
	sprintf (tmpstr, "%c%s%c%s/%s%c%s", sep_nl, ecg_info[i].ecg_status,
		 sep_at, ecg_info[i].ecg_path, ecg_info[i].ecg_name, sep_at,
		 ecg_info[i].ecg_desc);
	strcat (file_xfer, tmpstr);
      }
    }
	sprintf (tmpstr, "%c", sep_nl);
    strcat (file_xfer, tmpstr);
  }
}

int is_statready()
 {

  int rc_stat,rc = 0;
  struct stat file_stat;
  int i=0;

   while(i < 60) {
    rc_stat = stat ("/tmp/htxstats_done", &file_stat);
    /* printf("is_statready, rc_stat = %d, errno = %d\n",rc_stat,errno); */
    if (rc_stat == -1) {
       if ( errno == ENOENT)
	   sleep(1);
       else
          break;
    } else {
      rc = 1;
      break;
     }
    i++;
   }
   /* printf("is_statready, i = %d, rc = %d\n",i,rc); */
   return rc;
 }
