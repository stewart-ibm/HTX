
/* @(#)36	1.6  src/htx/usr/lpp/htx/bin/hxecd/prt_msg.c, exer_cd, htxubuntu 5/24/04 17:24:55 */

/******************************************************************************
 *   COMPONENT_NAME: exer_cd
 * 
 *   MODULE NAME: prt_msg.c
 *
 *   FUNCTIONS: info_msg
 *              prt_msg
 *              prt_msg_asis
 *
 *   DESCRIPTION: Functions to format messages and send to HTX.
 ******************************************************************************/
#include <string.h>
#include "hxecd.h"

/**************************************************************************/
/* format information message                                             */
/**************************************************************************/
info_msg(ps,pr,loop,blkno,msg_text)
struct htx_data *ps;
struct ruleinfo *pr;
int  loop;
int  *blkno;
char *msg_text;
{
  sprintf(msg_text, "%s: loop = %5d,  blk = %5d, len = %4d\n",
          pr->rule_id, loop, blkno[0], pr->dlen);
  return;
} 

/**************************************************************************/
/* send message to HTX                                                    */
/**************************************************************************/
prt_msg(ps,pr,loop,blkno,err,sev,text)
struct htx_data *ps;
struct ruleinfo *pr;
int  loop;
int  *blkno;
int  err;
int  sev;
char *text;
{
  char msg[550];

  info_msg(ps, pr, loop, blkno, msg);
  strcat(msg, text);
  if ( err <= sys_nerr )
     strcat(msg, sys_errlist[err]);
  hxfmsg(ps, err, sev, msg);
  return;
} 

/****************************************************************/
/* Send message to HTX as-is to bypass system error messages    */
/****************************************************************/
prt_msg_asis(phtx_info,prule_info,loop,pblk_num,err,sev,text)
struct htx_data *phtx_info;
struct ruleinfo *prule_info;
struct blk_num_typ *pblk_num;
int  loop;
int  err;
int  sev;
char *text;
{
  char msg[550];

  info_msg(phtx_info, prule_info, loop, pblk_num, msg);
  strncat(msg, text, (sizeof(msg) - strlen(msg)) );
  hxfmsg(phtx_info, err, sev, msg);
  return;
} 

