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


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#ifdef			__OS400__
#include "a2e.h"	//400+
#define herror perror
#endif
#include <sys/time.h>	//400+
#define MYPORT 3491 // the port client will be connecting to
#define NOTPORT 3492 // the port client will be connecting to
#define MAXDATASIZE 100 // max number of bytes we can get at once
#define MAXARRAY 1024


struct timeval tval_temp;
int sockfd,lis_fd,listener;
struct sockaddr_in server_addr;
struct sockaddr_in their_addr; // connector’s address information
struct sockaddr_in my_addr; // connector’s address information
char stat_fname[40];
char err_fname[40];
extern char ecg_name[60];
char *hdr;	// *s2[512]; //Not Used
char level[22];
char snd[10*1024];
char file_sut[40];
int cmd;
char *receive_all(int sock, int *numbytes);
int command_return_code = 0;

int main(int argc, char *argv[])
{
  struct hostent *he;
  int yes=1,sin_size;
  char sut_ip[80] , tout_val[5];
  int i,len,rc, *pp, j, k = -1 ;
  char *to;
  int tout, tflag; /* tflag is set to 1 when user has specified the timeout */

#if 0
#ifdef KERNEL64
  struct tim tval;
#else
  struct timeval tval;
#endif
#endif

struct timeval tval;

  len = 0;
  /*printf("argc = %d\n",argc); fflush(stdout);*/
  for (i=1; i<argc; i++) {
      len += strlen(argv[i])+1;
      //printf("argv[%d] = %s\n",i,argv[i]); fflush(stdout);
  }

  memset(snd,0,1024);

    for (i=1; i<argc; i++) {
        if(( to = strstr(argv[i],"-tout" )) == NULL ) {
          strcat(snd,argv[i]);
          strcat(snd," ");
         }
        else {

          /* timeout value specified as arguement (hopefully !!) */

          /* printf( "tout arguement, to = %s\n", to ); */
          /* argv[i+1] will have the timeout value */

             if(argv[i + 1][0] == '-' || argv[i + 1][0] == '\0') {
                  printf("\n Invalid syntax for tout, please specify a timeout value (in seconds)\n");
              }
             else {
                  for(j = 0; argv[i+1][j] != '\0'; j++)
                          if(!(isdigit(argv[i+1][j]))) { /* Invalid timeout calue specified */
                             printf("Timeout value has to be a number\n");
                             exit(1);
                            }
                          else
                            tout_val[++k] = argv[i+1][j];
                  tflag = 1;
                  i++;
             } /* End of second else */

        } /* End of first else */

    }/* End of first for loop */

    tout_val[++k]  = '\0';

    /* printf ( "Timeout = %d\n",atoi(tout_val));
       printf("snd = %s\n",snd); */


  cmd = 9030;
  rc = parse_inp(snd,sut_ip);
  /*printf("rc = %d\n", rc); fflush(stdout);*/
  if (rc == -1) {
      usage(snd);
      exit(1);
  }

  if ((he=gethostbyname(sut_ip)) == NULL) { // get the host info
     herror("gethostbyname");
     exit(1);
  }
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
     perror("socket");
     exit(1);
  }
  their_addr.sin_family = AF_INET; // host byte order
  their_addr.sin_port = htons(MYPORT); // short, network byte order
  their_addr.sin_addr = *((struct in_addr *)(he->h_addr));
  memset(&(their_addr.sin_zero), '\0', 8); // zero the rest of the struct

  if (tflag == 1) {
 	  printf("Setting timeout to %d\n", atoi(tout_val));
	  tval.tv_sec =  atoi(tout_val);
	  tval.tv_usec = 0L;
  }
  else {
	  tval.tv_sec =  5400L;
	  tval.tv_usec = 0L;
  }

#ifndef __OS400__
//400:this code causes execution errors in os/400

#ifdef __HTX_LINUX__
  if ( setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(void*)&tval,pp) == -1 )
#ifdef KERNEL64
     if (setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(void *)&tval,(socklen_t *)(2 *sizeof(struct timeval))) ==
-1) {
#else
    if (setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(void *)&tval,(socklen_t *)sizeof(struct timeval)) == -1) {
#endif
#else
    if (setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(void *)&tval,sizeof(struct timeval)) == -1) {
#endif
    perror("setsockopt");
    printf("Could not set the timeout value. Program will not exit if daemon is not able to respond\n");
    close(sockfd);
    exit(1);
  }

#endif


  if (connect(sockfd, (struct sockaddr *)&their_addr,
     sizeof(struct sockaddr)) == -1) {
     perror("connect");
     close(sockfd);
     exit(1);
  }

#ifdef _a2e_h
  /* convert message to ascii */	//400+
  ebcdic2Ascii(snd, snd, strlen(snd));//400+
#endif
  send_sockmsg(snd);
  
  return command_return_code;
}

int send_sockmsg()  // char *snd)
{
  char *str_rcv;
  char *str_snd;
  //int cmd;
  char *tmpstr;

  int numbytes,fileid,rc_stat;

  str_snd = (char *)malloc(1024);
  //printf("file_sut = %s\n",file_sut);
  if (cmd == 9023) {  //file on sut
    file_sut[strlen(file_sut) -1] = '\0';
    sprintf(str_snd,"%04X%04X%04X%s",cmd, 0, 0, file_sut);//hex:leading 0s and UpperCase, no spaces
  }
  else {             // esrv command
    sprintf(str_snd,"%04X%04X%04X%d",cmd, 0, 0, strlen(snd));//hex:leading 0s and UpperCase, no spaces
  }

  tmpstr = (char *)malloc(strlen(str_snd)+8);
  memset(tmpstr,0,(strlen(str_snd)+8));
  sprintf(tmpstr,"%08X%s",strlen(str_snd), str_snd);//hex:leading 0s and UpperCase
  //printf("tmpstr = %s..\n",tmpstr);
#ifdef _a2e_h
  /* convert message to ascii */	//400+
  ebcdic2Ascii(tmpstr, tmpstr, strlen(tmpstr));//400+
#endif
  if ((numbytes=send(sockfd, tmpstr, strlen(tmpstr), 0)) == -1) {
    close(sockfd);
    exit(1);
  }

  if ((cmd == 9030) || (cmd == 9022)) {
     if ((numbytes = recv(sockfd, str_snd, 1024, 0)) == -1) {
       close(sockfd);
       exit(1);
     }
     //printf("snd = :%s:\n",snd);
     if ((numbytes=send(sockfd, snd, strlen(snd), 0)) == -1) {
       close(sockfd);
       exit(1);
     }
  }
  str_rcv = receive_all(sockfd,&numbytes);
  //system("clear");
  //printf("\n########################## Result Starts Here ##################################\n");
  //printf("\n%s\n",str_rcv);
  //printf("########################### Result Ends Here ###################################\n");
  //fflush(stdout);
  close(sockfd);

  return 0;
}

char *receive_all(int sock, int *numbytes)
{
int  i, size_rcv, num_cmds, temp_buf_size;
char str_size[10],*str_rcv;

  if ((*numbytes=recv(sock,&str_size[0], 8, MSG_WAITALL)) <= 0) { //400:2nd parm must be char *
     printf("Received %d Bytes. errno = %d. Exiting \n",*numbytes,errno);
     fflush(stdout);
     perror("recvfrom");
     close(sockfd);
     exit(1);
  }
  str_size[8] = '\0';
#ifdef _a2e_h
  /* convert message to ebcdic */	//400+
  ascii2Ebcdic(&str_size[0], &str_size[0], 8);//400+
#endif
  sscanf(str_size,"%8x",&size_rcv);

  if (size_rcv <0) {
     num_cmds = size_rcv;
     for (i=num_cmds; i<0; i++) {
        if ((*numbytes=recv(sock,&str_size[0],8,MSG_WAITALL)) <= 0) { //400:2nd parm must be char *
           printf("Received %d Bytes. Exiting \n",*numbytes);
           fflush(stdout);
           perror("recvfrom");
           close(sockfd);
           exit(1); 
        }
        str_size[8] = '\0';
#ifdef _a2e_h
        /* convert message to ebcdic */	//400+
        ascii2Ebcdic(&str_size[0], &str_size[0], 8);//400+
#endif
        sscanf(str_size,"%8x",&size_rcv);
        if ((*numbytes=recv(sock,&str_size[0],8,MSG_WAITALL)) <= 0) { //400:2nd parm must be char *
           printf("Received %d Bytes. Exiting \n",*numbytes);
           fflush(stdout);
           perror("recvfrom");
           close(sockfd);
           exit(1); 
        }
        str_size[8] = '\0';
#ifdef _a2e_h
//        / * convert message to ebcdic * /	//400+
        ascii2Ebcdic(&str_size[0], &str_size[0], 8);//400+
#endif
        sscanf(str_size,"%8x",&command_return_code);
        str_rcv = (char *)malloc(size_rcv+10);
        memset(str_rcv,0,(size_rcv+10));
		temp_buf_size = sizeof(struct timeval);
		if ( getsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (void *)&tval_temp,(unsigned long *)&temp_buf_size) == -1) {
			//printf("Error: getsockopt: %d:%s\n",errno, strerror(errno));
		}
		else {
			//printf("Receive socket timeout = %d:%d\n", tval_temp.tv_sec,tval_temp.tv_usec);
		}

		*numbytes = 0;
		while ( *numbytes < size_rcv ) {
			int rcv_byte;
			//printf("ERROR: Received : %d(%d)\nReceiving again...\n", *numbytes, size_rcv);
			//printf("Error: %d:%s\n",errno, strerror(errno));

        	rcv_byte = recv(sock, (str_rcv + *numbytes), (size_rcv - *numbytes), MSG_WAITALL);
        	if ( rcv_byte == -1 ) {
				printf("Received -1 bytes. \n");
				fflush(stdout);
				perror("recvfrom");
				close(sockfd);
				exit(1);
       		}
       		//printf("Received %d more bytes\n",rcv_byte);
			*numbytes += rcv_byte;
		}

		//printf("Byte Received = %d\n", *numbytes);

#ifdef _a2e_h
        /* convert message to ebcdic */	//400+
        *numbytes = ascii2Ebcdic(str_rcv, str_rcv, *numbytes);	//400+
#endif
        printf("\n######################## Result Starts Here ################################");//400-limit width
        printf("\n%s\n",str_rcv);
        printf("######################### Result Ends Here #################################\n");//400
        fflush(stdout);
    }
  }
  else {
        if ((*numbytes=recv(sock,&str_size[0],8,MSG_WAITALL)) <= 0) { //400:2nd parm must be char *
           printf("Received %d Bytes. Exiting \n",*numbytes);
           fflush(stdout);
           perror("recvfrom");
           close(sockfd);
           exit(1); 
        }
        str_size[8] = '\0';
#ifdef _a2e_h
//        / * convert message to ebcdic * /	//400+
        ascii2Ebcdic(&str_size[0], &str_size[0], 8);//400+
#endif
        sscanf(str_size,"%8x",&command_return_code);
    str_rcv = (char *)malloc(size_rcv+10);
    memset(str_rcv,0,(size_rcv+10));
	*numbytes = 0;
	while ( *numbytes < size_rcv ) {
		int rcv_byte;
		//printf("ERROR: Received : %d(%d)\nReceiving again...\n", *numbytes, size_rcv);
		//printf("Error: %d:%s\n",errno, strerror(errno));

       	rcv_byte = recv(sock, (str_rcv + *numbytes), (size_rcv - *numbytes), MSG_WAITALL);
       	if ( rcv_byte == -1 ) {
			printf("Received -1 bytes. \n");
			fflush(stdout);
			perror("recvfrom");
			close(sockfd);
			exit(1);
		}
		//printf("Received %d more bytes\n",rcv_byte);
		*numbytes += rcv_byte;
	}

#ifdef _a2e_h
    /* convert message to ebcdic */	//400+
    *numbytes = ascii2Ebcdic(str_rcv, str_rcv, *numbytes);//400+
#endif
    printf("\n######################## Result Starts Here ################################");//400
    printf("\n%s\n",str_rcv);
    printf("######################### Result Ends Here #################################\n");//400
    fflush(stdout);
  }
  return str_rcv;
}

int usage(char *bad_cmd)
{

printf("\n%s\n",bad_cmd);
printf("Usage: esrv -sut <IP> OPTIONS\n"
"               Following are the options:\n"
"               -run [-ecg] <ecg_name> [-nonblock]\n"
"               -shutdown [-ecg] <ecg_name>\n"
"               -getactecg\n"
"               -addecg -ecg <ecg_name1> [<ecg_name2> ...]\n"
"               -query [all] [<device_name1> <device_name2> ...] [-ecg <ecg_name1> [<ecg_name2> ...]]\n"
"               -activate [all] [<device_name1> <device_name2> ...] [-ecg <ecg_name1> [<ecg_name2> ...]]\n"
"               -suspend [all] [<device_name1> <device_name2> ...] [-ecg <ecg_name1> [<ecg_name2> ...]] \n"
"               -terminate [all] [<device_name1> <device_name2> ...] [-ecg <ecg_name1> [<ecg_name2> ...]]\n"
"               -restart [all] [<device_name1> <device_name2> ...] [-ecg <ecg_name1> [<ecg_name2> ...]] \n"
"               -coe [all] [<device_name1> <device_name2> ...] [-ecg <ecg_name1> [<ecg_name2> ...]]\n"
"               -soe [all] [<device_name1> <device_name2> ...] [-ecg <ecg_name1> [<ecg_name2> ...]] \n"
"               -status [all] [<device_name1> <device_name2> ...] [-ecg <ecg_name1> [<ecg_name2> ...]] \n"
"               -getstats [-ecg <ecg_name1> [<ecg_name2> ...]] \n"
"               -get_fail_status [-ecg <ecg_name>]\n"
"               -get_run_time [-ecg <ecg_name>]\n"
"               -get_dev_cycles <device_name_prefix> [-ecg <ecg_name>]\n"
"               -get_last_update_time [-ecg <ecg_name>]\n"
"               -getecglist\n"
"               -gettestsum\n"
"               -getecgsum [-ecg <ecg_name1> [<ecg_name2> ...]] \n"
"               -geterrlog\n"
"               -clrerrlog\n"
"               -getvpd\n"
"               -exersetupinfo -ecg <ecg_name>\n"
"               -cmd <shell command>\n\n"
"      'run <esrv> to see this usage'\n\n"
       );


}

