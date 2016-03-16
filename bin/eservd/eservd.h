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

#ifndef ESERVD_H
#define ESERVD_H

/*
 ******************************************************************************
 ***  eservd.h -- "eservd" HTX Supervisor Include File  ***********************
 ******************************************************************************
 */                                                          

#ifdef  __HTX_LINUX__
 #include <sys/param.h>
#endif
#include <htx_local.h>
#include <hxiconv.h>
#include <hxiipc.h>
#include <hxihtx.h>

/* undefine TRUE and FALSE to prevent lint errors -- redefined in cfgdb.h */
//#undef TRUE
//#undef FALSE
#ifdef __OS400__    /* 400*/
#define TRUE  1
#define FALSE 0
#endif
#include <scr_info.h>

extern tmisc_shm *rem_shm_addr;
extern tfull_info info_send;
#define REM_SOCK       (rem_shm_addr->sock_hdr_addr)
#define REM_CUR        (rem_shm_addr->cur_shm_addr)
//#define INFO_SEND_5(x) info_send.scn_num.scn_5_info[x]
//#define INFO_SEND_2(x) info_send.scn_num.scn_2_4_info[x]
//#define INFO_SEND_ALL(x) info_send.scn_num.scn_all_info[x]

#ifndef __HTX_LINUX__
#define htx_strcpy strcpy
#define htx_strlen strlen
#define htx_strcat strcat
#define htx_strcmp strcmp
#define htx_strncmp strncmp
#define htx_strncpy strncpy

#ifndef __OS400__
#include <sys/cfgdb.h>
#include <sys/cfgodm.h>
#endif
#else
  #define  SIGMAX  (SIGRTMAX)
  #define  SIGEMT  (SIGSEGV)
  #define  SIGMSG  (SIGUNUSED)
  #define  SIGDANGER  (SIGUNUSED)
  #define  SIGMIGRATE  (SIGUNUSED)
  #define  SIGGRANT  (SIGUNUSED)
  #define  SIGRETRACT  (SIGUNUSED)
  #define  SIGSOUND  (SIGUNUSED)
  #define  SIGSAK  (SIGUNUSED)
#endif

#include "defines.h"

#endif  /* HXSSUP_H */
