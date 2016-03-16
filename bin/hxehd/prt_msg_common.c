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
/* @(#)40       1.12  src/htx/usr/lpp/htx/bin/hxehd/prt_msg.c, exer_hd, htx61V 6/15/10 00:55:17 */
/******************************************************************************

 * MODULE NAME: prt_msg.c
 *
 * FUNCTION: Formats messages and sends them to HTX
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "hxehd_common.h"
#include "hxehd_proto_common.h"

extern int crash_on_anyerr;
extern unsigned long long volatile  lba_fencepost;    /* just for error messages */
extern int    is_bwrc_rule_present;

/**************************************************************************/
/* format information message                                             */
/**************************************************************************/
info_msg(struct htx_data *ps, struct ruleinfo *pr, int loop,
         unsigned long long *blkno, char *msg_text)
{
  if ( (strcmp(pr->addr_type, "RANDOM") == 0)  ||
       (strcmp(pr->type_length, "RANDOM") == 0) )
	if(is_bwrc_rule_present) {
     	sprintf(msg_text,
             "%s  numopers=%10d  loop=%10d  blk=%#llx \n"
             "len=%10d   offset=%d   Seed Values= %d, %d, %d \n"
             "Data Pattern Seed Values = %d, %d, %d    LBA Fencepost = %#llx\n",
             pr->rule_id, pr->num_oper, loop, blkno[0], pr->dlen, pr->offset,
             pr->seed[0], pr->seed[1], pr->seed[2], pr->seed[3], pr->seed[4],
             pr->seed[5], lba_fencepost);
	} else {
		sprintf(msg_text,
             "%s  numopers=%10d  loop=%10d  blk=%#llx \n"
             "len=%10d   offset=%d   Seed Values= %d, %d, %d \n"
             "Data Pattern Seed Values = %d, %d, %d    \n",
             pr->rule_id, pr->num_oper, loop, blkno[0], pr->dlen, pr->offset,
             pr->seed[0], pr->seed[1], pr->seed[2], pr->seed[3], pr->seed[4],
             pr->seed[5] );
	}
  else
     sprintf(msg_text,
             "%s numopers=%10d loop=%10d blk=%#llx len=%10d offset=%10d\n",
             pr->rule_id, pr->num_oper, loop, blkno[0], pr->dlen, pr->offset);
  return;
}

/**************************************************************************/
/* send user defined message to HTX with more information                 */
/**************************************************************************/
prt_msg(struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long *blkno,
        int err, int sev, char *text)
{
  char msg[MSG_TEXT_SIZE];

  if ( crash_on_anyerr ) {
	#ifdef __HTX_LINUX__
		do_trap_htx64( 0xDEADDEED, loop, blkno, ps, pr);
	#else
     	trap( 0xDEADDEED, loop, blkno, ps, pr);
	#endif
	}
  info_msg(ps, pr, loop, blkno, msg);
  strcat(msg, text);
  if ( err > 0 && err <= sys_nerr ) {
     strcat(msg, sys_errlist[errno]);
     strcat(msg, "\n");
  }
  hxfmsg(ps, err, sev, msg);
  return;
}

/**************************************************************************/
/* send a user defined message to HTX                                     */
/**************************************************************************/
user_msg(struct htx_data *ps, int err, int sev, char *text)
{
  char msg[MSG_TEXT_SIZE];

  /*if ( (err != 0) && (crash_on_anyerr) )*/
  if ( (sev == HARD) && (crash_on_anyerr) ) {
	#ifdef __HTX_LINUX__
		do_trap_htx64(0xDEADDEED, ps);
	#else
     trap(0xDEADDEED, ps);
	#endif
  }
  strcpy(msg, text);
  if ( (err > 0) && ( err <= sys_nerr) ) {
     strcat(msg, sys_errlist[err]);
     strcat(msg, "\n");
  }

  hxfmsg(ps, err, sev, msg);
  return;
}
