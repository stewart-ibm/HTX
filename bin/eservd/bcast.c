/* @(#)24	1.26  src/htx/usr/lpp/htx/bin/eservd/bcast.c, eserv_daemon, htxubuntu 5/21/04 18:53:57 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "eservd.h"
#include "global.h"
#ifdef __OS400__
#include "a2e.h"
#endif
#define MYPORT 1200 // the port users will be connecting to
#define MAXBUFLEN 500

#ifdef __OS400__            /* 400 */
#include "popen.h"
#endif


//main()
//extern char msg_type[10];
extern char daemon_start_time[20], daemon_upd_time[20];
extern int get_time(long, char *, char *, char *);
extern tmisc_shm *rem_shm_addr;
extern  union shm_pointers shm_addr;
extern int ipc_done;
extern int num_hmcs;
extern char rsrc_type[20][20], rsrc_name[40][20];
extern char curr_client_ip[20];

int getfullhostname(char *);

int send_broadcast(char *msg_status, char *start_time, char *upd_time, int new_fd)
{
    int bcast_fd;
    struct sockaddr_in my_addr; /* my address information */
    int addr_len, numbytes;
    char buf[MAXBUFLEN],hostname[80],tmpstr[500],test_sum[3*1024];
    int yes=1, i=0;
    char dt[20], dy[4], tmm[20];
    DBTRACE(DBENTRY,("enter bcast.c send_broadcast\n"));

    if ((bcast_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
	print_log(LOGERR,"socket");
	DBTRACE(DBEXIT,("return/a 1 bcast.c send_broadcast\n"));
	return 1;
    }

    if (setsockopt(bcast_fd,SOL_SOCKET,SO_BROADCAST,(char *) &yes,sizeof(int)) == -1) {
	print_log(LOGERR,"setsockopt");
	close(bcast_fd);
	DBTRACE(DBEXIT,("return/b 1 bcast.c send_broadcast\n"));
	return 1;
    }

    my_addr.sin_family = AF_INET; // host byte order
      my_addr.sin_port = htons(MYPORT); // short, network byte order
      my_addr.sin_addr.s_addr = inet_addr("255.255.255.255"); // automatically fill with my IP
	memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

	  if (getfullhostname(hostname) != 0 ){
	     print_log(LOGERR,"Error getting hostname. errorno = %d\n", errno);
	      sprintf(hostname," ");
	      hostname[1] = '\0';
	  }

  /*if (gethostbyname(hostname) == NULL) {
     print_log(LOGERR,"Error getting gethostbyname. errorno = %d\n", errno);
      sprintf(hostname," ");
      hostname[1] = '\0';
  }*/
      /*sprintf(hostname," ");
      hostname[1] = '\0';*/

	print_log(LOGMSG,"in send_broadcast\n"); fflush(stdout);
	sprintf(buf,"%s@%s@",msg_type, hostname);

	if (ipc_done) {
	    test_summary(test_sum);
	   print_log(LOGMSG,"Called test_summary %s/%s\n", ECGPATH,ECGNAME); fflush(stdout);
	    /* sprintf(tmpstr,"%s/ecg.all@", ECGPATH,ECGNAME); */
	    sprintf(tmpstr,"/usr/lpp/htx/ecg/ecg.bu@");
	    strcat(buf,tmpstr);

	    sprintf(tmpstr,"%s@", rem_shm_addr->sock_hdr_addr->system_status);
	    strcat(buf,tmpstr);

	    get_time(rem_shm_addr->cur_shm_addr->start_time,dt,dy,tmm);
	    sprintf(tmpstr,"%s  %s@", dt,tmm);
	    strcat(buf,tmpstr);


	    get_time(rem_shm_addr->sock_hdr_addr->last_update_time,dt,dy,tmm);
	    sprintf(tmpstr,"%s  %s@", dt,tmm);
	    strcat(buf,tmpstr);

	    sprintf(tmpstr,"%d@", rem_shm_addr->sock_hdr_addr->error_count);
	    strcat(buf,tmpstr);

	    if (rem_shm_addr->sock_hdr_addr->error_count == 0) {
		sprintf(tmpstr,"  @  @");
		strcat(buf,tmpstr);
	    }
	    else {
		get_time(rem_shm_addr->sock_hdr_addr->last_error_time,dt,dy,tmm);
		sprintf(tmpstr,"%s  %s@", dt,tmm);
		strcat(buf,tmpstr);

		sprintf(tmpstr,"%s @",rem_shm_addr->sock_hdr_addr->error_dev);
		strcat(buf,tmpstr);
	    }

	    //test_summary();
	    sprintf(tmpstr,"%s@", test_sum);
	    strcat(buf,tmpstr);
	}
	else {
	  strcat(buf,"/usr/lpp/htx/ecg/ecg.bu@Inactive@  @  @0@  @  @  @"); //name,type,status,start,update,err_count,err_time,err_dev
	    //type,hostname,ecg_name,status,start,update,err_count,err_time,err_dev,summary
	}

	sprintf(tmpstr,"%d@",num_hmcs);
	strcat(buf,tmpstr);
	for (i=0; i<num_hmcs; i++) {
	    sprintf(tmpstr,"%s@%s@",(char *)rsrc_type,(char *)rsrc_name);
	    strcat(buf,tmpstr);
	}

	addr_len = sizeof(struct sockaddr);

	memset(tmpstr,0,500);
	sprintf(tmpstr,"%8x@%s",strlen(buf),buf);
	print_log(LOGMSG,"Sending broadcast message: %s\n",tmpstr); fflush(stdout);	/* print before conversion */
#ifdef _a2e_h	/*400-EBCDIC*/
	ebcdic2Ascii(&tmpstr[0],&tmpstr[0],strlen(tmpstr));
#endif
	if (new_fd == 0) {
	    if ((numbytes=sendto(bcast_fd,tmpstr, strlen(tmpstr), 0,
				 (struct sockaddr *)&my_addr, addr_len)) == -1) {
		print_log(LOGERR,"sendto");
		//exit(1);
	    }
	   print_log(LOGMSG,"Broadcast message sent, len = \n",numbytes); fflush(stdout);
	    //close(bcast_fd);
	}

	if (new_fd > 0) {
	    send(new_fd,tmpstr, strlen(tmpstr),0);
	    //print_log(LOGMSG,"Broadcast message: strlen = %d: %s\n",strlen(tmpstr),tmpstr); fflush(stdout);
	}
	else if (new_fd == 0){
	    for (i=0; i< num_WAS; i++) {
		my_addr.sin_addr.s_addr = inet_addr(WAS_ip[i]);
		print_log(LOGMSG,"Sending unicast message to %s\n", WAS_ip[i]); fflush(stdout);
		memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct
		  if ((numbytes=sendto(bcast_fd,tmpstr, strlen(tmpstr), 0,
				       (struct sockaddr *)&my_addr, addr_len)) == -1) {
		      print_log(LOGERR,"sendto");
		      //exit(1);
		  }
	    }
	}
	else {
	    if ((numbytes = send ((0-new_fd), tmpstr, strlen (tmpstr), 0)) == -1) {
		print_log(LOGERR,"send");
	    }
	}


	close(bcast_fd);
	DBTRACE(DBEXIT,("return 0 bcast.c send_broadcast\n"));
	return 0;
}


int
get_WAS_info(void)
{
    int rc;
    FILE *fileid;
    struct stat WAS_LIST_stat;
    DBTRACE(DBENTRY,("enter bcast.c get_WAS_info\n"));


 /* fileid = fopen("/tmp/WAS_LIST","r"); */
if(stat("/usr/lpp/htx/etc/scripts/WAS_LIST",&WAS_LIST_stat) == -1 && errno == ENOENT)
 {
  fileid = fopen("/usr/lpp/htx/etc/scripts/WAS_LIST","a");
 }
else
 {
    fileid = fopen("/usr/lpp/htx/etc/scripts/WAS_LIST","r");
    if (fileid == NULL) {
    print_log(LOGERR,"Could not open the WAS_LIST file. errono = %d (%s)\n",errno, strerror(errno));
     DBTRACE(DBEXIT,("return/a 0 bcast.c get_WAS_info\n"));
     return 0;
    }
 }
    num_WAS = 0;
    do {
	rc = fscanf(fileid,"%s",WAS_ip[num_WAS]);
	if (rc == EOF)
	    break;
	print_log(LOGMSG,"WAS Info: %s ",WAS_ip[num_WAS]);
	num_WAS++;
    } while (rc!=EOF);
    fclose(fileid);

    DBTRACE(DBEXIT,("return 0 bcast.c get_WAS_info\n"));
    return 0;
}


int getfullhostname(char * host_fullname )
{
    char hostname[40]="";
    char hostip[40]="";
    struct hostent *myhostent;
    char workstr[256], *tmpptr1;
    FILE *pfd;
    int found_ip=1;
    int timed_out=0;
    DBTRACE(DBENTRY,("enter bcast.c getfullhostname\n"));

    strcpy(host_fullname,"unknown");

    if(gethostname(hostname, sizeof(hostname)) != 0 ) {
	print_log(LOGERR,"gethostname");
	DBTRACE(DBEXIT,("return/a -1 bcast.c getfullhostname\n"));
	return -1;
    }
   print_log(LOGMSG,"host:%s\n",hostname);
    if((myhostent = gethostbyname(hostname))== NULL ) {
	print_log(LOGERR,"gethostbyname");
	DBTRACE(DBEXIT,("return/b -1 bcast.c getfullhostname\n"));
	return -1;
    }
#ifdef __OS400__
    strcpy(host_fullname,hostname);	/* os400 does not provide host command, just use the hostname */
#else	/* NOT __OS400__ */
    strcpy(hostip,(char *)inet_ntoa(*((struct in_addr *)myhostent->h_addr)));
   print_log(LOGMSG,"host:%s\n",hostip);
    sprintf(workstr,"host %s",hostip);
    pfd = popen(workstr,"r");
    if (pfd == NULL ) {
	print_log(LOGERR,"popen");
	DBTRACE(DBEXIT,("return/c -1 bcast.c getfullhostname\n"));
	return -1;
    }
    if(fgets(workstr,256, pfd)== NULL ) {
	print_log(LOGERR,"fgets");
	pclose(pfd);
	DBTRACE(DBEXIT,("return/d -1 bcast.c getfullhostname\n"));
	return -1;
    }
	/*print_log(LOGMSG,"workstr:%s\n",workstr);*/
    tmpptr1 = (char *)strtok(workstr," ");
    if( tmpptr1 == NULL ) {
	print_log(LOGERR,"error in getting the full hostname\n");
	pclose(pfd);
	DBTRACE(DBEXIT,("return/e -1 bcast.c getfullhostname\n"));
	return -1;
    }
	/*print_log(LOGERR,"tmpptr:%s\n",tmpptr1);*/
#ifndef __HTX_LINUX__
    strcpy(host_fullname,tmpptr1);
#endif
    while(tmpptr1 != NULL ) {
	if ( (strcmp(tmpptr1,"NXDOMIAN")== 0 )
	     || (strcmp(tmpptr1,"NOT")== 0 )
	     || (strcmp(tmpptr1,"cannot")== 0 )) {
	    found_ip = 0;
	   print_log(LOGERR,"wrong ip\n");
	}
	if (( (strcmp(tmpptr1,"timed"))== 0 )|| ((strcmp(tmpptr1,"Timed"))== 0 )) {
		    /*print_log(LOGERR,"got timed\n");*/
	    timed_out=1;
	}
	if (timed_out && ((strncmp(tmpptr1,"out",3))== 0 )) {
	    found_ip = 0;
	    timed_out=0;
	   print_log(LOGERR,"couldn't reach DNS\n");
	}
#ifdef __HTX_LINUX__
	strcpy(host_fullname,tmpptr1);
#endif
	tmpptr1 = (char *)strtok(NULL," ");
    }
    if (!found_ip){
	print_log(LOGMSG,"will use ip address\n");
	strcpy(host_fullname,hostip);
    } else {
	if (host_fullname[strlen(host_fullname)-1] == '\n')
	    host_fullname[strlen(host_fullname)-1] = '\0';
	if (host_fullname[strlen(host_fullname)-1] == '.')
	    host_fullname[strlen(host_fullname)-1] = '\0';
	print_log(LOGMSG,"host:%s\n",host_fullname);
    }

    pclose(pfd);
#endif	/*__OS400__*/
    DBTRACE(DBEXIT,("return 0 bcast.c getfullhostname\n"));
    return 0;
}

