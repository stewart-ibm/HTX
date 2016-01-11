/* @(#)10       1.8  src/htx/usr/lpp/htx/bin/hxehd/prt_msg_acoustics.c, exer_hd, htxubuntu 8/7/13 02:06:05 */
/******************************************************************************
 
 * MODULE NAME: prt_msg.c
 *
 * FUNCTION: Formats messages and sends them to HTX
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "hxehd.h"
#include "hxehd_proto.h"

extern int crash_on_anyerr;
extern int volatile  lba_fencepost,proc_stage,accous_rules,semid;    /* just for error messages */

/**************************************************************************/
/* format information message                                             */
/**************************************************************************/
info_msg(struct htx_data *ps, struct ruleinfo *pr, int loop,
         unsigned long long *blkno, char *msg_text)
{
  if ( (strcmp(pr->addr_type, "RANDOM") == 0)  ||
       (strcmp(pr->type_length, "RANDOM") == 0) )
     sprintf(msg_text,
             "%s  numopers=%10d  loop=%10d  blk=%#llx\n"
             "len=%10d   offset=%d   Seed Values= %d, %d, %d \n"
             "Data Pattern Seed Values = %d, %d, %d    LBA Fencepost = %#llx\n",
             pr->rule_id, pr->num_oper, loop, blkno[0], pr->dlen, pr->offset,
             pr->seed[0], pr->seed[1], pr->seed[2], pr->seed[3], pr->seed[4],
             pr->seed[5], lba_fencepost);
  else
     sprintf(msg_text,
             "%s numopers=%10d loop=%10d blk=%10d len=%10d offset=%10d\n",
             pr->rule_id, pr->num_oper, loop, blkno[0], pr->dlen, pr->offset);
  return;
}

/**************************************************************************/
/* send user defined message to HTX with more information                 */
/**************************************************************************/
prt_msg(struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long * blkno,
        int err, int sev, char *text)
{
  char msg[MSG_TEXT_SIZE];
  struct sembuf semops_dec[1] = {0,-1,1};
  struct sembuf semops_dec_sem2[1] = {1,-1,1};

  if ( crash_on_anyerr )
     trap( 0xDEADDEED, loop, blkno, ps, pr);
  info_msg(ps, pr, loop, blkno, msg);
  strcat(msg, text);
  if ( err <= sys_nerr ) {
     strcat(msg, sys_errlist[errno]);
     strcat(msg, "\n");
  }
  /* changes for accoustics */
  if(sev == HARD)
  if(accous_rules)
  {
    if(proc_stage)
      semop(semid,&semops_dec_sem2[0],1);
    else
      semop(semid,&semops_dec[0],1);
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
  if ( (sev == HARD) && (crash_on_anyerr) )
     trap(0xDEADDEED, ps);
  strcpy(msg, text);
  if ( (err != 0) && (err <= sys_nerr) ) {
     strcat(msg, sys_errlist[err]);
     strcat(msg, "\n");
  }

  hxfmsg(ps, err, sev, msg);
  return;
}
