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

/* @(#)39	1.28.4.2  src/htx/usr/lpp/htx/bin/stxclient/client.c, eserv_gui, htxubuntu 3/14/12 06:41:45 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "hxssup.h"
#include "cfgclibdef.h"
#define MYPORT 3491 // the port client will be connecting to
#define NOTPORT 3492 // the port client will be connecting to
#define MAXDATASIZE 100 // max number of bytes we can get at once
#define MAXARRAY 1024

/*
struct tim {
unsigned long long tv_sec;
unsigned long long tv_usec;
};
*/

int sockfd,lis_fd,listener;
int is_local_host=0;
struct sockaddr_in server_addr;
thtx_message msg_send;
tprobe_msg probe_rcv;
tsys_hdr sys_hdr_in;
//tscn_2_4 scn_2_4_in[MAXARRAY];
//tscn_5   scn_5_in[MAXARRAY];
tscn_1   scn_1_in;
tfull_info   info_rcv;
struct sockaddr_in their_addr; // connector’s address information
struct sockaddr_in my_addr; // connector’s address information
char stat_fname[40];
char err_fname[40];
char sum_fname[40];
char sysdata_fname[40];

extern int init_screen_done;
extern char ecg_name[20];
extern char *server_ip;
extern int system_call;
char *hdr,*s2[512];
char level[22];
char workstr[128];
long STX_SOCKET_TIMEOUT;
char *receive_allc(int, int *);
extern void parse_input_9(char*, int*, char*, int);
extern void parse_input_6(char*, int*, char*,char*, int);
extern void parse_input_5(char*, int*, char*, int);
extern void parse_input_2(char*, int*, char*, int);
extern void parse_input_1(char*, int*, char*, int, char*);

int setup_client()
{
  struct hostent *he;
  char my_host_name[256];
  struct hostent *my_he;
  int yes=1,sin_size;

#if 0
#ifdef KERNEL64
  struct tim tval;
#else
  struct timeval tval;
#endif
#endif

  struct timeval tval;
  int *pp;
  int hostid, my_hostid, temp_hostid;
  CFG__SFT *pro_fd;                    /* pointer to MDT attribute file     */


  char     stanza[4096];               /* stanza area for attribute file    */
  /*FILE *fd = fopen("/tmp/timeout","w");*/


  if(gethostname(my_host_name, sizeof(my_host_name)) == -1) {
    perror("gethostname");
    exit(1);
  }

  if ((my_he=gethostbyname(my_host_name)) == NULL) { // get the host info
    herror("gethostent");
    exit(1);
  }
  my_hostid = ntohl(((struct in_addr *)my_he->h_addr)->s_addr);
  /*printf("my_addr:%x\n",my_hostid);*/

  if ((he=gethostbyname(server_ip)) == NULL) { // get the host info
       herror("gethostbyname");
       exit(1);
  }
  temp_hostid = (((struct in_addr *)he->h_addr)->s_addr);
  hostid = ntohl(temp_hostid);
  /*printf("server_addr:%x\n",hostid);*/
  if (hostid == 0x7f000001) {
    /*printf("going thru loop back\n");fflush(stdout);
     sleep(5);*/
     is_local_host = 1;
  } else {
    if ( hostid == my_hostid) {
       /*printf("local system is same as the destination\n");fflush(stdout);
       sleep(5);*/
       is_local_host = 1;
    }else {
       /*printf("local system is different from the destination\n");fflush(stdout);
       sleep(5);*/
       is_local_host = 0;
    }
  }
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(1);
  }

(void) strcpy(workstr, "/usr/lpp/htx/");     /* copy HTX file system path         */
  pro_fd = cfgcopsf(strcat(workstr,"/.htx_profile"));
  if ((pro_fd != (CFG__SFT *) NULL) &&
      (cfgcrdsz(pro_fd, stanza, sizeof(stanza), (char *) NULL) == CFG_SUCC))
    {

 	 strcpy(workstr,"-1");
         (void) cfgcskwd("stx_socket_timeout", stanza, workstr);
       /*  printf("workstr = %s\n",workstr);*/

  	 if(strcmp(workstr,"-1")) {
     	 	 (void) unquote(workstr);               /* no quotes */
      		 STX_SOCKET_TIMEOUT = atol(workstr);
		if(STX_SOCKET_TIMEOUT < 0 || STX_SOCKET_TIMEOUT > 2147483647)
			STX_SOCKET_TIMEOUT = 300L;
		/*fprintf(fd,"%ld",STX_SOCKET_TIMEOUT);*/
    	 }
  	else
   	{
        	STX_SOCKET_TIMEOUT = 300L;
		/*fprintf(fd,"%ld",STX_SOCKET_TIMEOUT);*/
        	/*printf("STX_SOCKET_TIMEOUT = %ld\n",STX_SOCKET_TIMEOUT);*/
   	}
  }
   /*fclose(fd); */
   tval.tv_sec =  STX_SOCKET_TIMEOUT;
   tval.tv_usec = 0L;

   printf("test message\n"); fflush(stdout);

#ifdef __HTX_LINUX__
#if 0
   if (setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&tval,pp) == -1)
#ifdef KERNEL64
     if (setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(void *)&tval,(socklen_t *)(2 *sizeof(struct timeval))) == -1) {
#else
     if (setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(void *)&tval,(socklen_t *)sizeof(struct timeval)) == -1) {
#endif
#endif
        if (setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(void *)&tval,(socklen_t )sizeof(struct timeval)) == -1) {
#else
if (setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(void *)&tval,(socklen_t *)sizeof(struct timeval)) == -1) {
#endif
    perror("setsockopt");
    printf("Could not set the timeout value. Program will not exit if daemon is not able to respond\n");
    exit(1);
  }


  their_addr.sin_family = AF_INET; // host byte order
  their_addr.sin_port = htons(MYPORT); // short, network byte order
  their_addr.sin_addr = *((struct in_addr *)(he->h_addr));
  memset(&(their_addr.sin_zero), '\0', 8); // zero the rest of the struct

  sprintf(stat_fname,"mkdir -p /tmp/%s",server_ip);
  system_call = TRUE;
  system(stat_fname);
  system_call = FALSE;

  memset(err_fname, '\0', 40); // zero the rest of the struct
  memset(stat_fname, '\0', 40); // zero the rest of the struct
  memset(sum_fname, '\0', 40); // zero the rest of the struct
  memset(sysdata_fname, '\0', 40); // zero the rest of the struct

  sprintf(stat_fname,"/tmp/%s/htxstats",server_ip);
  //strcat(stat_fname,"/htxstats");

  sprintf(err_fname,"/tmp/%s/htxerr",server_ip);
  //strcat(err_fname,"/htxerr");

  sprintf(sum_fname,"/tmp/%s/htxsum",server_ip);
  //strcat(sum_fname,"/htxsum");

  sprintf(sysdata_fname,"/tmp/%s/htx_sysdata",server_ip);
  //strcat(sysdata_fname,"/htx_sysdata");

  if (connect(sockfd, (struct sockaddr *)&their_addr,
     sizeof(struct sockaddr)) == -1) {
     perror("connect");
     exit(1);
  }
  else {
     //sleep(2);
     /*if ((he=gethostbyname(server_ip)) == NULL) { // get the host info
     herror("gethostbyname");
     exit(1);
     }
     if ((lis_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
         perror("socket");
         exit(1);
     }
     their_addr.sin_family = AF_INET; // host byte order
     their_addr.sin_port = htons(NOTPORT); // short, network byte order
     their_addr.sin_addr = *((struct in_addr *)(he->h_addr));
     memset(&(their_addr.sin_zero), '\0', 8); // zero the rest of the struct
     if (connect(lis_fd, (struct sockaddr *)&their_addr,
         sizeof(struct sockaddr)) == -1) {
         perror("connect");
         exit(1);
     }*/

    init_screen();
    init_screen_done = 1;
    htx_scn();
  }
  return 0;
}

int send_sockmsg(ushort short1, ushort byt2, int byt3, char *str4, char *result_msg)
{

  int numbytes,fileid,rc_stat;
  char *file_rcv;
  char fname[40],workstr[80];
  int rc,len;
  char *str_rcv,run_cmd[80],*str_msg,*tmp_str;
  struct stat fname_stat;

  msg_send.cmd = (ushort) short1;
  msg_send.subcmd = (ushort) byt2;
  msg_send.indx = (int) byt3;
  strcpy(msg_send.str,str4);

  //str_msg = (char *)malloc(1024);
  //memset(str_msg,0,1024);
  str_msg = (char *)malloc(sizeof(msg_send));
  memset(str_msg,0,sizeof(msg_send));

  sprintf(str_msg,"%04x%04x%04x%s", msg_send.cmd,msg_send.subcmd,msg_send.indx,msg_send.str);
  tmp_str = malloc( (strlen(str_msg)+9));
  sprintf(tmp_str,"%8x",strlen(str_msg));
  strcat(tmp_str,str_msg);
  tmp_str[strlen(str_msg)+8] = '\0';
  if ((numbytes=send(sockfd, tmp_str, strlen(tmp_str), 0)) == -1) {
  	free(tmp_str);
    printf("Send error. errno = %d\n",errno);
    fflush(stdout);
    getch();
    perror("send");
    exit(1);
  }
  free(tmp_str);
  free(str_msg);

  switch (short1) {
    case 2000:
    case 2010:
      //printf("Receiving ... \n"); fflush(stdout); getch();
      str_rcv = receive_allc(sockfd,&numbytes);
      if (numbytes == 0)
         rc = -99;
      else
         (void)parse_input_0(str_rcv,&rc,result_msg,numbytes);

      break;

    case SCREEN_1:
    case 2011:
    case 2021:
      str_rcv = receive_allc(sockfd,&numbytes);
      if (numbytes == 0)
         rc = -99;
      else
         (void)parse_input_1(str_rcv,&rc,result_msg,(numbytes-1),ecg_name);

      break;

    case SCREEN_2:
    case 2012:
      str_rcv = receive_allc(sockfd,&numbytes);
      if (numbytes == 0)
         rc = -99;
      else
         (void)parse_input_2(str_rcv,&rc,result_msg,(numbytes-1));

      free(str_rcv);
      break;

    case SCREEN_3:
    case 2013:
      str_rcv = receive_allc(sockfd,&numbytes);
      //printf("received %d bytes\n",numbytes);
      //fflush(stdout);
      //getch();
      if (numbytes == 0)
         rc = -99;
      else
         (void)parse_input_2(str_rcv,&rc,result_msg,(numbytes-1));

      free(str_rcv);
      break;

    case SCREEN_4:
    case SHUTDOWN:
      str_rcv = receive_allc(sockfd,&numbytes);
      if (numbytes == 0)
         rc = -99;
      else
         do {
            (void)parse_input_1(str_rcv,&rc,result_msg,(numbytes-1), ecg_name);
            CLRLIN(MSGLINE,0);
            PRTMSG(MSGLINE, 0, ("%s", result_msg));
            if (rc !=0)
               str_rcv = receive_allc(sockfd,&numbytes);
         } while (rc != 0);

      //clear();                /*  clean up screen               */
      //refresh();              /*  update screen                 */
      //endwin();               /*  end CURSES */
      //rc = 0;
      //exit(0);

      break;

    case SCREEN_5:
      str_rcv = receive_allc(sockfd,&numbytes);
      if (numbytes == 0)
         rc = -99;
      else
         (void)parse_input_5(str_rcv,&rc,result_msg,(numbytes-1));

      free(str_rcv);
      break;

    case SCREEN_6:
    case SCREEN_7:
    case SCREEN_8:
    case SCREEN_A:

      if (short1 == SCREEN_6)
         strcpy(fname,stat_fname);
      else if (short1 == SCREEN_8)
         strcpy(fname,sum_fname);
      else if (short1 == SCREEN_7)
         strcpy(fname,err_fname);
      else
         strcpy(fname,sysdata_fname);

      str_rcv = receive_allc(sockfd,&numbytes);
      if (numbytes == 0)
         rc = -99;
      else {

      /************* Delete the existing file ****************/
      sprintf(run_cmd,"%s",fname);
      rc_stat = stat(run_cmd,&fname_stat);
      if (rc_stat == 0) {
         sprintf(run_cmd,"rm %s",fname);
         system_call = TRUE;
	 system(run_cmd);
	 system_call = FALSE;
      }
      /************* Done: Delete the existing file **********/

      /************* Open the file ****************/
      fileid = open(fname, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
      if (fileid == -1) {
        sprintf(workstr, "Error opening the file %s: errno = %d\n",fname, errno);
        PRTMSG(MSGLINE, 0, ("%s", workstr));
        getch();
      }
      /************* Done: Open the file ****************/

      if (numbytes != 0) {
         file_rcv = (char *)malloc(numbytes+1);
         memset(file_rcv,0,(numbytes+1));
         (void)parse_input_6(str_rcv,&rc,result_msg,file_rcv,numbytes);
      }
      else {
        sprintf(file_rcv," ");
        numbytes = 1;
      }
      file_rcv[strlen(file_rcv)] = '\0';
      len = (strlen(file_rcv)>3)?(strlen(file_rcv) -1):1;
      //if (write(fileid, file_rcv,(strlen(file_rcv) -3)) == -1) {
      if (write(fileid, file_rcv,len) == -1) {
         sprintf(workstr, "Error writing to the file %s:  errno = %d\n",fname, errno);
         PRTMSG(MSGLINE, 0, ("%s", workstr));
         getch();
      }

      //free(file_rcv);
      //free(str_rcv);
      close(fileid);
      //rc = 0;
      }
      break;

    case SCREEN_9_A:
      str_rcv = receive_allc(sockfd,&numbytes);
      if (numbytes == 0)
         rc = -99;
      else {

      (void)parse_input_9(str_rcv,&rc,result_msg,numbytes);
      probe_rcv.err = rc;
      memset(probe_rcv.msg_text,0,80);
      strcpy(probe_rcv.msg_text,result_msg);
      }
      break;
    case SCREEN_9_R:
    case SCREEN_9_R_D:
    case SCREEN_9_T:
    case SCREEN_9_T_D:
      str_rcv = receive_allc(sockfd,&numbytes);
      if (numbytes == 0)
         rc = -99;
      else
         (void)parse_input_2(str_rcv,&rc,result_msg,(numbytes-1));

      free(str_rcv);
      break;
  }
  return rc;

}

char *receive_allc(int sock, int *numbytes)
{
int size_rcv;
char str_size[10],*str_rcv;

  if ((*numbytes=recv(sock,&str_size, 8, MSG_WAITALL)) <= 0) {
       printf("Received %d Bytes. errno = %d Exiting \n",*numbytes, errno);
       fflush(stdout);
       perror("recvfrom");
       clear();                /*  clean up screen               */
       refresh();              /*  update screen                 */
       endwin();               /*  end CURSES */
       if (errno == EAGAIN) {
           printf("\n\n\n\t*****************************************************\n");
           printf("\tSTX daemon is busy, please try connecting to daemon \n");
           printf("\t again after sometime\n");
           printf("\t*****************************************************\n");
           fflush(stdout);
           sleep(1);
           exit(0);
       }
       printf("\n\n\n\t*****************************************************\n");
       printf("\tDaemon has  closed the connection\n");
       printf("\tShutting down the client\n");
       printf("\t*****************************************************\n");
       fflush(stdout);
       sleep(1);
       exit(0);


       return 0;
       //exit(1);
  }
  str_size[8] = '\0';
  sscanf(str_size,"%x",&size_rcv);
  str_rcv = (char *)malloc(size_rcv);
  //printf("Receiving ... %d\n",size_rcv); fflush(stdout); getch();
  if ((*numbytes=recv(sock,str_rcv, (size_rcv), MSG_WAITALL)) == -1) {
       printf("Received -1 bytes. \n");
       fflush(stdout);
       perror("recvfrom");
       exit(1);
    }
      //printf("Received ... %d\n",*numbytes); fflush(stdout); getch();
    return str_rcv;
}
