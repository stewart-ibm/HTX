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

/* @(#)93	1.7.1.9  src/htx/usr/lpp/htx/bin/hxecom/comsock.c, exer_com, htx53A 5/24/04 17:27:35 */

/*
 *   COMPONENT_NAME: exer_com
 *
 *   FUNCTIONS: FixSockAddr
 *		GetConnectStr
 *		GetIP
 *		HostToNetId_t
 *		HostToNetRules
 *		HostToNetSemid_t
 *		InetNtoa
 *		NetToHostId_t
 *		NetToHostRules
 *		NetToHostSemid_t
 *		OpenPort
 *		SetUpConnect
 *		StrHerrno
 *		StreamRead
 *		StreamReadEof
 *		StreamWrite
 *		CantConnect
 */

#    include <stdlib.h>
#    include <string.h>
#    include <unistd.h>
#    include <fcntl.h>
#    include <memory.h>
#    include <sys/types.h>
#    include <sys/socket.h>
#    include <netinet/in.h>
#    include <netdb.h>
#    include <arpa/inet.h>
#    include <sys/time.h>
#    include "hxecomdef.h"

extern int com_hxfmsg(struct htx_data *p, int err, enum sev_code  sev, char *text);

static char * StrHerrno(char * msg, int H_Errno);

void OpenPort(char * InetName, struct id_t * ID, SOCKET * Sock, int layer, struct htx_data * stats)
{
   	struct hostent *    ComInfo=NULL ;  
	struct sockaddr_in sock_in;
	char msg[1024]; 
    socklen_t     length;

/********************************************************************/
/* Open socket.                                                     */
/********************************************************************/
    *Sock = socket(AF_INET, (layer == (int)UDP_LAYER) ? SOCK_DGRAM : SOCK_STREAM, 0);
    if(*Sock == INVALID_SOCKET) {
        sprintf(msg, "OpenPort: Error opening socket - %s. Inet Name = %s \n", STRERROR(errno), InetName);
        hxfmsg(stats, HTXERROR(EX_SOCK8,ERRNO), HTX_HE_SOFT_ERROR, msg);
        HE_exit(EX_SOCK8);
    }

	
/********************************************************************/
/* Setup socket to receive from  (Net addr, Port) pair.             */
/********************************************************************/

    memset(&sock_in, '\0', sizeof(struct sockaddr_in));
	 
	ComInfo = gethostbyname(InetName);
	memcpy(&sock_in.sin_addr, ComInfo->h_addr, ComInfo->h_length);
	/* 	
    if(GetIP(InetName, &sock_in.sin_addr, msg)== -1) {
        hxfmsg(stats, HTXERROR(EX_GETH3,h_errno), HTX_HE_SOFT_ERROR, stats->msg_text);
        closesocket(*Sock);
        HE_exit(EX_GETH3);
    }
	*/ 
    sock_in.sin_family = AF_INET;
#ifndef __HTX_LINUX__
    sock_in.sin_len = sizeof(sock_in);
#endif
    sock_in.sin_port = 0;

    if(bind(*Sock, (struct sockaddr *) &sock_in, sizeof(sock_in)) ) {
        sprintf(msg, "OpenPort: bind Error binding socket - %s.Inet Name =  %s, addrss=%s \n", STRERROR(errno), InetName, inet_ntoa(sock_in.sin_addr));
        hxfmsg(stats, HTXERROR(EX_BIND5,ERRNO), HTX_HE_SOFT_ERROR, msg);
        closesocket(*Sock);
        HE_exit(EX_BIND5);
    }

    length = sizeof(sock_in);
    if(getsockname(*Sock, (struct sockaddr *) &sock_in, &length)) {
        sprintf(msg, "OpenPort: getsockname Error getting socket info - %s. Inet Name =  %s \n", STRERROR(errno), InetName);
        hxfmsg(stats, HTXERROR(EX_GETS1,ERRNO), HTX_HE_SOFT_ERROR,msg);
        closesocket(*Sock);
        HE_exit(EX_GETS1);
    }

    ID->nettype = 1;
    memcpy(&ID->sock, &sock_in, sizeof(sock_in));
}



int StreamRead(SOCKET fd, register char *ptr, register int nbytes)
{
   int  nleft;
   int  nread;

   nleft = nbytes;
   while(nleft > 0) {
      nread = recv(fd, ptr, nleft, 0);
      if(nread == SOCKET_ERROR)
         return -1;
      else if(nread == 0)
         break;
      nleft -= nread;
      ptr += nread;
   }
   return (nbytes -nleft);
}



int StreamReadEof(SOCKET fd, register char *ptr, register int nbytes)
{
   size_t nleft;
   int nread;
   char  c;

   nleft = nbytes;
   while(nleft > (size_t)0) {
      nread = recv(fd, ptr, nleft, 0);
      if(nread == SOCKET_ERROR)
         return -1;
      else if(nread == 0)
         break;
      nleft -= nread;
      ptr += nread;
   }
/****************************************************************************/
/* Read to end-of-file. Throwing away remainder.                            */              
/****************************************************************************/
   if(nread != 0)
      while(nread)
         nread = recv(fd, &c, 1, 0);
         
   return (nbytes -nleft);
}

int CantConnect(char *ConnectAddr,struct htx_data *stats)
{
    char that_addr[32];
    char this_addr[32];
    int index;
    char *that_token;
    char *this_token;

	strcpy(that_addr,ConnectAddr);
	that_token = strtok(that_addr,".");
	that_token = strtok(NULL,".");
	that_token = strtok(NULL,".");
	that_token = strtok(NULL,".");

	/*If the last octet of the host trying to connect matches my last octet*/
	/* allow connect */
	if(strcmp(shm_HXECOM->mynetid,that_token)==0) {
		return(0);
	}

	/*If the last octet of the host trying to connect matches the last octet*/
	/* of any ip address in the /tmp/other_ids file :  allow connect */
    for(index=0; index < shm_HXECOM->HostCnt; index++) {
        InetNtoa(shm_HXECOM->Other_ips[index], this_addr, stats);
        this_token = strtok(this_addr,".");
        this_token = strtok(NULL,".");
        this_token = strtok(NULL,".");
        this_token = strtok(NULL,".");
		if(strcmp(this_token,that_token)==0) {
			return(0);
		}
    }

	/* no matches so don't allow connect. */
	/* You get here when some old version of hxecom does broadcast on the
		comnet sub net. Bad old version!!!*/
	return(1);

}


int StreamWrite(SOCKET fd, register char *ptr, register int nbytes)
{
   size_t  nleft;
   int nwritten;

   nleft = nbytes;
   while(nleft > 0) {
      nwritten = send(fd, ptr, nleft, 0);
      if(nwritten == SOCKET_ERROR)
         return -1;
      nleft -= nwritten;
      ptr += nwritten;
   }
   return (nbytes - nleft);
}


SOCKET SetUpConnect(struct sockaddr_in * dest, struct htx_data * stats, uint32_t state)
{
    SOCKET Sock = -1;
    int i, rc;
    char addrStr[30];
    long   num;
    struct timeval tv;
    struct timezone tz;
	char msg_text[1024]; 

#define NORETRIES	5
    for(i=0; i<NORETRIES; i++) {
        Sock = socket(AF_INET, SOCK_STREAM, 0);
        if(Sock == INVALID_SOCKET) {
			if(state & SH_FORCE) {
				HE_exit(0);
			} else {
				sprintf(msg_text, "SetUpConnect: Error opening socket for (%s,%d)- %s.\n", 
						   InetNtoa(dest->sin_addr, addrStr, stats) , (int) ntohs(dest->sin_port), STRERROR(errno));
				hxfmsg(stats, HTXERROR(EX_SOCK9,ERRNO), HTX_HE_SOFT_ERROR, msg_text);
				HE_exit(EX_SOCK9);
			}
        }

        rc = connect(Sock, (struct sockaddr *) dest, sizeof(struct sockaddr_in));

        if(rc == 0 || (ERRNO != ECONNREFUSED && ERRNO != ETIMEDOUT))
            break;
        closesocket(Sock);
        if(i < NORETRIES-1) {
			if(state & SH_FORCE) {
				HE_exit(0);
			} else {
				gettimeofday(&tv, &tz);
				num = (tv.tv_usec % 300 + 100) * 10000;
				sprintf(msg_text, "SetUpConnect: Error Connecting to (%s,%d).\nRetrying... (#%d) ERRNO=%d\n", 
							InetNtoa(dest->sin_addr, addrStr, stats) , (int) ntohs(dest->sin_port), i+1,ERRNO);
				hxfmsg(stats, HTXERROR(EX_SOCK10,ERRNO), HTX_HE_INFO, msg_text);
				usleep(num);
			}
        }
    }
    if(rc == SOCKET_ERROR) {
		if(state & SH_FORCE) {
			HE_exit(0);
		} else {
			sprintf(msg_text, 
				  "SetUpConnect: Error Connecting to (%s,%d) - %s.\n", 
							InetNtoa(dest->sin_addr, addrStr, stats), (int) ntohs(dest->sin_port), STRERROR(errno));
			hxfmsg(stats, HTXERROR(EX_GETH4,ERRNO), HTX_HE_SOFT_ERROR, msg_text);
			closesocket(Sock);
			HE_exit(EX_GETH4);
		}
    }

    return Sock;
}




int GetIP(char * InetName, struct in_addr *Addr, char * msg_text)
{
    char  ErrMsg[80];
    struct hostent *    ComInfo=NULL;
#ifndef __HTX_LINUX__
    struct hostent * gethostbyname(char *);
#endif



#ifdef __DEBUG__
	printf("GetIP::Getting the hostent structure for id:%s and size:%d\n",InetName,strlen(InetName));	
#endif

	ComInfo	= gethostbyname(InetName);

#ifdef __DEBUG__
	printf("getIP:: ComInfo pointing to :%llx\n",ComInfo);
#endif
    if(ComInfo == NULL) {
    	printf("%d: %s\n",getpid(),(char *)InetName);
        sprintf(msg_text, "Error getting Net '%s' info - %s.\n", InetName, StrHerrno(ErrMsg, h_errno));
        return -1;
    }
#ifdef __DEBUG__
    printf("%d : finished gethostbyname\n",getpid());
#endif
    memcpy(Addr, ComInfo->h_addr, ComInfo->h_length);
    return 0;   
}


static char * StrHerrno(char * msg, int H_Errno)
{
    switch (H_Errno) {
        case HOST_NOT_FOUND:
            sprintf(msg, "Unacceptable host name or alias.");
            break;
        case TRY_AGAIN:
            sprintf(msg, "No response from authoritative server, try again.");
            break;
        case NO_RECOVERY:
            sprintf(msg, "Unrecoverable error.");
            break;
        case NO_ADDRESS:
            sprintf(msg, "Requested name is valid but does not have an IP address at name server.");
            break;
        case EINVAL:
            sprintf(msg, "Invalid parameter.");
            break;
        default:
            sprintf(msg, "Unknown h_errno - %d", H_Errno);
            break;
    }
    return msg;
}


void GetConnectStr(char * ConnectStr, int StrLen, const char *Str1, 
              struct sockaddr_in sock1, const char *Str2, struct sockaddr_in sock2,
                  struct htx_data * stats)
{
    char msg[1000];
    char addrStr1[30], addrStr2[30];
    u_short port1;

    port1 = ntohs(sock1.sin_port);
    sprintf(msg, "%s(%s.%hu)%s(%s.%hu)", Str1, InetNtoa(sock1.sin_addr, addrStr1, stats), port1, Str2, 
                   InetNtoa(sock2.sin_addr, addrStr2, stats), ntohs(sock2.sin_port));
    strncpy(ConnectStr, msg, StrLen -1);
}



char * InetNtoa(struct in_addr addr, char *ReturnStr, struct htx_data * stats)
{
    strcpy(ReturnStr, inet_ntoa(addr));
    return ReturnStr;
}



void HostToNetRules(struct rule_format * rules)
{
    rules->ComNetMask    = htonl(rules->ComNetMask);
    rules->ComPortStream = htons(rules->ComPortStream);
    rules->ComPortDgram  = htons(rules->ComPortDgram);
    rules->nettype       = htons(rules->nettype);
    rules->write_ahead   = htons(rules->write_ahead);
    rules->ack_trig      = htons(rules->ack_trig);
    rules->num_oper      = htons(rules->num_oper);
    rules->bufmin        = htons(rules->bufmin);
    rules->bufmax        = htons(rules->bufmax);
    rules->bufinc        = htons(rules->bufinc);
    rules->replicates    = htons(rules->replicates);
    rules->write_sleep   = htons(rules->write_sleep);
    rules->alarm         = htons(rules->alarm);
    rules->idle_time     = htons(rules->idle_time);
    rules->shutdown      = htons(rules->shutdown);
    rules->transact      = htons(rules->transact);
}



void NetToHostRules(struct rule_format * rules)
{
    rules->ComNetMask    = ntohl(rules->ComNetMask);
    rules->ComPortStream = ntohs(rules->ComPortStream);
    rules->ComPortDgram  = ntohs(rules->ComPortDgram);
    rules->nettype       = ntohs(rules->nettype);
    rules->write_ahead   = ntohs(rules->write_ahead);
    rules->ack_trig      = ntohs(rules->ack_trig);
    rules->num_oper      = ntohs(rules->num_oper);
    rules->bufmin        = ntohs(rules->bufmin);
    rules->bufmax        = ntohs(rules->bufmax);
    rules->bufinc        = ntohs(rules->bufinc);
    rules->replicates    = ntohs(rules->replicates);
    rules->write_sleep   = ntohs(rules->write_sleep);
    rules->alarm         = ntohs(rules->alarm);
    rules->idle_time     = ntohs(rules->idle_time);
    rules->shutdown      = ntohs(rules->shutdown);
    rules->transact      = ntohs(rules->transact);
}



void HostToNetId_t(struct id_t * ID)
{
    ID->nettype     = htons(ID->nettype);
    ID->replicates  = htons(ID->replicates);
    ID->transact    = htons(ID->transact);
    ID->ExerVersion = htons(ID->ExerVersion);
}



void NetToHostId_t(struct id_t * ID)
{
    ID->nettype     = ntohs(ID->nettype);
    ID->replicates  = ntohs(ID->replicates);
    ID->transact    = ntohs(ID->transact);
    ID->ExerVersion = ntohs(ID->ExerVersion);
}



void NetToHostSemid_t(struct semid_t * SemID)
{
    SemID->WriteIdx = ntohs(SemID->WriteIdx);
    SemID->ackNo    = ntohs(SemID->ackNo);
    SemID->NumOper  = ntohs(SemID->NumOper);
    SemID->bsize    = ntohs(SemID->bsize);
    SemID->Iloop    = ntohs(SemID->Iloop);
}

void HostToNetSemid_t(struct semid_t * SemID)
{
    SemID->WriteIdx = htons(SemID->WriteIdx);
    SemID->ackNo    = htons(SemID->ackNo);
    SemID->NumOper  = htons(SemID->NumOper);
    SemID->bsize    = htons(SemID->bsize);
    SemID->Iloop    = htons(SemID->Iloop);
}


void NetToHostBsize_t(struct bufsize_t * Bufsize) { 
	Bufsize->WriterIdx = ntohs(Bufsize->WriterIdx); 
	Bufsize->bufmin   = ntohs(Bufsize->bufmin); 
	Bufsize->bufmax   = ntohs(Bufsize->bufmax); 
	Bufsize->num_oper = ntohs(Bufsize->num_oper); 
	Bufsize->num_pktsizes = ntohs(Bufsize->num_pktsizes); 
}  



void FixSockAddr(struct sockaddr_in * sock)
{
    struct sockaddr_in temp;

/********************************************************************/
/*  If we hadn't setup originally to use memcpy and memcmp in all   */
/*  functions, this wouldn't be necessary.  However, this structure */
/*  isn't the same in NT or Linux and by making a local copy when we receive */
/*  a remote message handles the problem.                           */
/********************************************************************/
    memset(&temp, '\0', sizeof(temp));
    temp.sin_addr.s_addr = sock->sin_addr.s_addr;
    temp.sin_family      = AF_INET;
#ifndef __HTX_LINUX__
    temp.sin_len         = sizeof(temp);
#endif
    temp.sin_port        = sock->sin_port;
    memcpy(sock, &temp, sizeof(temp));
}
