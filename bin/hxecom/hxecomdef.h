/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* htx71F src/htx/usr/lpp/htx/bin/hxecom/hxecomdef.h 1.4.5.3              */
/*                                                                        */
/* Licensed Materials - Property of IBM                                   */
/*                                                                        */
/* COPYRIGHT International Business Machines Corp. 2010                   */
/* All Rights Reserved                                                    */
/*                                                                        */
/* US Government Users Restricted Rights - Use, duplication or            */
/* disclosure restricted by GSA ADP Schedule Contract with IBM Corp.      */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
/* @(#)99       1.4.1.4  src/htx/usr/lpp/htx/bin/hxecom/hxecomdef.h, exer_com, htx53A 6/10/02 02:12:13 */
/*
 *   COMPONENT_NAME: exer_com
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *   OBJECT CODE ONLY SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef HXECOMDEF_H
#define HXECOMDEF_H

#ifndef NTDEF
#include <sys/param.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/socketvar.h>

#ifdef __HTX_LINUX__
#include <sys/stat.h>
#endif

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#ifdef HXECOM_THREADED
#include <pthread.h>
#endif
#ifdef _64BIT_
#include "hxihtx64.h"
#else
#include "hxihtx.h"
#endif

#include "global.h"
#include "exit.h"
#endif

int  main(int argc, char *argv[]);
void OthMsg(struct htx_data * stats, char msg[]);
void HE_exit(int exit_no);
int  get_rule(FILE *fd, int *line, struct rule_format * r_ptr);
int  DefineMySems(int ExerNo);
void GlobalWait(int SemNo, struct htx_data * stats);
void GlobalSignal(int SemNo, struct htx_data * stats);
void GetSockID(struct id_t * ID);
struct id_t * SaveSockID(struct id_t ID);
SOCKET SetUpConnect(struct sockaddr_in * dest, struct htx_data * stats, uint32_t state);
int StreamWrite(SOCKET fd, register char *ptr, register int nbytes);
int CantConnect(char *ConnectAddr,struct htx_data * stats);
int StreamReadEof(SOCKET fd, register char *ptr, register int nbytes);
int StreamRead(SOCKET fd, register char *ptr, register int nbytes);
void OpenPort(char * InetName, struct id_t * ID, SOCKET * Sock, int layer, struct htx_data * stats);
void SaveSemServID(struct id_t SemServID);
int  DefineShm(char * hostname);
#ifdef NTDEF
void UpSem(HANDLE  hSem[], int offset, struct htx_data * stats);
void DownSem(HANDLE hSem[], int offset, struct htx_data * stats);
void GateSem(HANDLE hSem[], int offset, struct htx_data * stats);
void IncSem(HANDLE hSem[], int offset, int inc, struct htx_data * stats);
void SetSem(HANDLE hSem[], int offset, int value, struct htx_data * stats);
#else
void UpSem(int SemId, int SemNo, struct htx_data * stats);
void DownSem(int SemId, int SemNo, struct htx_data * stats);
void DecSem(int SemId, int SemNo, struct htx_data * stats);  /*twm*/
void GateSem(int SemId, int SemNo, struct htx_data * stats);
void IncSem(int SemId, int SemNo, int inc, struct htx_data * stats);
void SetSem(int SemId, int sem_type, int value,struct htx_data * stats);
#endif
void GetHost(struct in_addr addr, char * netname, int namelen);
void * comwrite(void * Targ);
void * comwrite_rdma(void * Targ, int exer_idx);
void coord(int argc, char *argv[]);
void * comread(void * arg);
void * comread_rdma(void * arg, int exer_idx);
void * comsem(void * arg);
void Detach_HXECOM(struct shm_hxecom_t * ptr);
void Detach_Private(struct shm_Private_t * ptr);
void Detach_COMSEM(void);
void IncDeadCnt(void);
void InitWriteVars(struct htx_data *stats);
int  GetIP(char * Inetname, struct in_addr *Addr, char * msg_text);
void GetConnectStr(char * ConnectStr, int StrLen, const char *Str1,
              struct sockaddr_in sock1, const char *Str2, struct sockaddr_in sock2,
                  struct htx_data * stats);
char * InetNtoa(struct in_addr addr, char *ReturnStr, struct htx_data * stats);
void HostToNetSemid_t(struct semid_t * SemID);
void NetToHostSemid_t(struct semid_t * SemID);
void NetToHostId_t(struct id_t * ID);
void HostToNetId_t(struct id_t * ID);
void NetToHostRules(struct rule_format * rules);
void HostToNetRules(struct rule_format * rules);
void FixSockAddr(struct sockaddr_in * sock);
void AttachGLOBALSems(struct htx_data * stats);
void AttachShmHxecom(void);

#ifndef NTDEF
/* These aren't in the headers ??                                       */
pid_t setpgrp(void);
int  statx(char * Path, struct stat * Buffer, int Length, int Command);
#else
char * GetErrorStr(char * ErrStr);
#endif

#endif/* HXECOMDEF_H */
