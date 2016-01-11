
/* @(#)32	1.4  src/htx/usr/lpp/htx/bin/hxecd/rnd_oper.c, exer_cd, htxubuntu 5/24/04 17:25:14 */

/******************************************************************************
 *   COMPONENT_NAME: exer_cd
 * 
 *   MODULE NAME: rnd_oper.c
 *
 *   FUNCTIONS: init_seed
 *              random_blkno
 *              random_dlen
 *
 *   DESCRIPTION: Random operation functions.
 ******************************************************************************/
#include <time.h>
#include "hxecd.h"

/**************************************************************************/
/* Initialize seed for random number generator (erand48)                  */
/**************************************************************************/
init_seed(seed)
unsigned short seed[];
{
  long   clk, time();
  struct tm *tp;

  clk = time((long *) 0);
  tp = localtime(&clk);
  seed[0]  = (*tp).tm_sec;
  seed[1]  = (*tp).tm_min * (*tp).tm_sec;
  seed[2]  = (*tp).tm_hour * (*tp).tm_sec;
  return;
} 

/**************************************************************************/
/* returns random data length                                             */
/**************************************************************************/
random_dlen(bytpsec,max_blkno,seed)
short bytpsec;
long max_blkno;
unsigned short seed[];
{
  int  dlen, i;
  long nrand48();

  i = 0;
  do {
     dlen = nrand48(seed) % ((BUF_SIZE / bytpsec) + 1);
     if ( dlen == 0 )
        dlen = bytpsec;
     else
        dlen = dlen * bytpsec;
     i++;
     if ( i > 100 )
        dlen = bytpsec;
  } while ( dlen > BUF_SIZE || (dlen / bytpsec) > max_blkno );
  return(dlen);
} 

/**************************************************************************/
/* sets random block number                                               */
/**************************************************************************/
random_blkno(blkno,dlen,bytpsec,max_blkno,seed,min_blkno)
int *blkno;
unsigned int dlen;
short bytpsec;
long max_blkno;
unsigned short seed[];
long min_blkno;
{
  int  i;
  long nrand48();

  i = 0;
  do {
     blkno[0] = nrand48(seed) % (max_blkno - min_blkno);
     i++;
     if ( i > 100 )
        blkno[0] = 0;
  } while( (blkno[0] + (dlen / bytpsec)) >= (max_blkno - min_blkno) );  
  blkno[0] = blkno[0] + min_blkno;
  return;
} 
