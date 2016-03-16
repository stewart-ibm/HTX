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
/* @(#)92       1.5.1.2  src/htx/usr/lpp/htx/bin/hxecom/comrw.h, exer_com, htx53A 6/10/02 04:18:37 */
/*
 *   COMPONENT_NAME: exer_com
 *
 *   FUNCTIONS: addw
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

#ifndef HXECOMRW_H
#define HXECOMRW_H

#ifdef __HTX_LINUX__
# include <sys/time.h>
#endif


struct stat_rw {
    unsigned int   bad_others;
    unsigned int   bad_rw;
    unsigned long  bad_bytes_rw1;
    unsigned long  bad_bytes_rw2;
    unsigned long  good_bytes_rw1;
    unsigned long  good_bytes_rw2;
    unsigned int   good_others;
    unsigned int   good_rw;
};


struct cum_rw {
    struct stat_rw totals;
    struct stat_rw current;
    unsigned int   rw_size;
};



struct rw_t {
    struct timeval timeout;
    int SigAlarmFlag;
    struct htx_data *stats;
};



#define addw(msw, lsw, num) \
{ \
    lsw += num; \
    if (lsw > 999999999) /* split the addition over two words? */\
      {\
        msw++; \
        lsw -= 1000000000; \
      } /* endif */ \
}



void bad_other(struct cum_rw * Lstats, struct htx_data * pstats, struct htx_data * stats);
void good_other(struct cum_rw * Lstats, struct htx_data * pstats, struct htx_data * stats);
int  GetPattern(struct sockaddr_in ServerID, char *patternbuf, int pattern_max, struct htx_data * stats);
int  GetRules(struct sockaddr_in ServerID, struct rule_format rule[], int max_rules, struct htx_data * stats);
void GetPseudoDeviceStr(char * PseudoDeviceStr, int StrLen, const char *Str,
                                           struct sockaddr_in sock, struct htx_data * stats);
void ShutdownTest(struct sockaddr_in ServerID, struct htx_data * stats);
void Free(void * arg);
void ShutdownSocket(void *arg);
void SetAlarm(int sec, struct rw_t * rwParms);

#endif /* HXECOMRW_H */
