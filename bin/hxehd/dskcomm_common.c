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
/* @(#)36       1.77  src/htx/usr/lpp/htx/bin/hxehd/dskcomm.c, exer_hd, htx61V 6/12/12 04:22:27 */
/*****************************************************************************
 *   FUNCTION: To use these subroutine in the exercising of the disks.
 *
 *****************************************************************************
 *                     INFORMATION
 *****************************************************************************
 *   The following is the definition of BLKNO[0,1,2]. This can get confusing.
 *      BLKNO[0] - this is the current block number (LBA) to be used.
 *      BLKNO[1] - this is the minimum block number (LBA) to be used.
 *      BLKNL[2] - this is the maximum block number (LBA) to be used.
 *   Stating the minimum and maximum does not mean the minimum and maximum on
 *   the disk. These two figures can change such as the case when reading/
 *   writing back and forth across a disk.
 *****************************************************************************
 *   CHANGE LOG:
 *   Date      Name          Description
 *  11/22/96   D. Stauffer   Added a field to crash on miscompare. New field
 *                           will show device that sent the exerciser into the
 *                           debugger.
 *  12/03/96   D. Stauffer   Added a new pattern to build buf.
 *  04/18/97   L. Record    -Modified bldbuf to place a header in all internal
 *                           patterns type 3, 4, and 5.  Header holds info
 *                           about the thread that wrote the data, time, etc,
 *                           a signature, and a checksum.  Also made bldbuf
 *                           more efficient.  NOTE: the interleaved integer
 *                           algorithms give slightly different results than
 *                           before.
 *                          -Modified cmpbuf to look for the header described
 *                           above and skip it during comparisons if the chksum
 *                           and signature were right.
 *                          -Added a PRAGMA with inline code to crash the
 *                           system if a miscompare occurs.  This is much
 *                           faster than the old way of loading a kernel
 *                           extension and calling it so it could call the
 *                           brkpoint() function in the kernel.  Also changed
 *                           the register contents at crash to include the
 *                           htx_data and ruleinfo structures rather than just
 *                           sdev_id in the htx_data structure.
 *  07/15/97   L. Record    -Modify cmpbuf to check for header during re-read
 *                           buffer comparisons also.  Otherwise re-read may
 *                           miscompare when the difference is a time stamp or
 *                           rule name in the header.  (Defect 225312)
 *  08/20/97   D. Stauffer  -Added new function to exerciser to set up a thread
 *                           that will run continuously while the exerciser is
 *                           running. It will look for the condition of a
 *                           thread being hung and it will report this condition
 *                           to the user.
 *  01/09/98   D. Stauffer  -Look in init_blkno and set_blkno for changes made.
 *  02/10/99   D. Stauffer  -Added hostname to the header of the buffers when
 *                           building write/read buffers. Header length is now
 *                           48 bytes.
 *  07/07/99   D. Stauffer  -Added new global variable to cmpbuf to allow a user
 *                           not to reread on a miscompare if desired. Also
 *                           allow a user to run a system command when a mis-
 *                           compare is detected.
 *  08/09/99   D. Stauffer  -Added code to allow a switch (backgrnd_thread) to
 *                           be set to "E" if an error is found.
 *  09/15/99   D. Stauffer  -Corrected code in random_blkno. Allow for cleaner
 *                           generation of random block numbers.
 *  10/15/99   D. Stauffer  -Added new function to do_compare. Check time_stamp
 *                           to make sure not checking stale data.
 *  09/29/00   R. Gebhardt  -Put loop, blkno,  and max_lba on
 *                           messages which pertain to stale data.
 *  03/23/01   R. Gebhardt  -Declare threshold as an external instead of
 *                           a local in run_timer.
 *  10/08/01   sjayakum     -Modification for 64 bit compilation
 *							- uses user thread id for lock in 64 bit (seg_lock & seg_unlock)
 *  09/11/06 deepakcs       - fixed write buf generation bug for pattern #007 in bldbuf routine.
 *  10/21/09 deepthi 	    - fixed  compare routine to report miscompare in the header. Wrong associativity
 *  05/15/10 deepthi        - Added new flag HTX_HE_MISCOMPARE
 *  06/14/10 deepthi 	    - Removing the old IBM copyright statements hardcoded in the files and
 *                             modifying the properties to reflect the new in the form of prolog.
 *  06/16/10 deepthi	  	-  Flexibility to reuse the buffers during the run.  New keyword REUSE_BUFFER added.
 *  14/02/11 Piyush     	-  Performance mode related changes.
 *  24/05/11 Piyush         -  Added new operations in hxehd.
 *  17/10/11 Piyush         - Add Support for new Pattern 8, 9, 10 in hxehd.
 *  18/10/12 Piyush         - Add Support for 64 bit LBA Number in hxehd.
 *  24/07/13 Piyush			-  Code Common AIX/Linux.
 *  01/10/13 Preeti         - Code common for Big Endian and Little Endian
 *  07/11/12 Piyush 		- Support for Dynamic Tier
 *****************************************************************************/
#include "hxehd_common.h"
#include "hxehd_proto_common.h"

int volatile           collisions;
unsigned long long volatile           lba_fencepost = 0;
extern int             crash_on_miscom, hang_time, threshold;
extern int	           turn_attention_on;
static int volatile    seg_table_index = 0;
extern pthread_cond_t  segment_do_oper;
extern pthread_mutex_t cache_mutex;
extern pthread_mutex_t segment_mutex;
extern char            misc_run_cmd[100], run_on_misc, run_reread;
extern char            hostname[], machine_name[], crash_on_hang;
extern char volatile   backgrnd_thread;
extern time_t          time_mark;
extern int 			   la_on_miscom;
extern int 			   threads_created;
extern char 		   exit_flag;
extern int      read_rules_file_count;

#ifndef __HTX_LINUX__
extern char            PCI_parent[10][10]; /* for LA trigger */
extern int             PCI_devfn[10];
#else
void __attn(unsigned int a,
            unsigned int b,
            unsigned int c,
            unsigned int d,
            unsigned int e,
            unsigned int f,
            unsigned int g){
    __asm__ volatile (".long 0x00000200":);
}
#endif

static struct {
	unsigned long long flba;
	unsigned long long llba;
	pthread_t tid;
	time_t thread_time;
	int hang_count;
} volatile seg_table[MAX_THREADS];


/*****************************************************************************
** seg_lock()
** This routine locks a segment described by a starting and ending LBA
** number.  it will sleep waiting for another thread to unlock a segment
** that overlaps the one we want to lock.  The seg_lock function increments
** the number of segment collisions it encountered while trying to lock
** the segment.  This count is zeroed at the start of a rule-file scan and
** printed to the HTX log at the end of a scan.
******************************************************************************/
void seg_lock(struct htx_data *ps, unsigned long long flba, unsigned long long llba)
{
	int       i, locked = 0;
	char      msg[160];
	pthread_t tid;

	tid = pthread_self();

	if ( (i = pthread_mutex_lock(&segment_mutex))) {
		sprintf(msg, "Mutex lock failed in process SEG_LOCK, rc %d (%s)", i, strerror(i));
		user_msg(ps, 950, HARD, msg);
		exit(i);
	}

	/* Repeat until we are able to lock our segment... */
	while ( !locked ) {
		/* Scan the locked segments to see if ours overlaps one already locked */
		for ( i = 0; i < seg_table_index; i++ )
			if ( ((seg_table[i].flba < flba) && (seg_table[i].llba >= flba)) || ((seg_table[i].flba > flba) &&
			    (seg_table[i].flba <= llba)) || (seg_table[i].flba == flba) )
				break;
		if ( i == seg_table_index ) {
			/* Our segment doesn't overlap any locked segment, so lock it. */
			seg_table[i].flba = flba;
			seg_table[i].llba = llba;
			seg_table[i].tid = tid;
			seg_table[i].thread_time = time(0);
			seg_table[i].hang_count = 0;
			seg_table_index++;
			locked = 1;

		} else {
			/* Our segment overlaps one already locked.  Wait for another thread to
           unlock something so we can check again.  We are guaranteed to get a
           wakeup since the fact that another segment is locked means at least
           one other thread is running.   */
			collisions++;

			if ( (i = pthread_cond_wait(&segment_do_oper, &segment_mutex))) {

				sprintf(msg, "Cond_wait failed in process SEG_LOCK, rc %d (%s)", i, strerror(i));
				user_msg(ps, 950, HARD, msg);
				exit(i);
			}
		}
	}

	if ((i = pthread_mutex_unlock(&segment_mutex))) {

		sprintf(msg, "Mutex unlock failed in process SEG_LOCK, rc %d (%s)", i, strerror(i));
		user_msg(ps, 950, HARD, msg);
		exit(i);
	}
}

/****************************************************************************
** seg_unlock()
** This routine unlocks a previously locked segment.
*****************************************************************************/
void seg_unlock(struct htx_data *ps, struct ruleinfo *pr, unsigned long long flba, unsigned long long llba)
{
	int       i;
	char      msg[(MAX_THREADS + 2) * 80];
	pthread_t tid;

	tid = pthread_self(); /* changed fro kernel thread id to user thread id */

	/* sprintf(msg, "seg_unlock request : tid %x flbsa %x llba %x \n",pthread_self(),flba,llba);
  	user_msg(ps, 0, INFO, msg); */

	if ((i = pthread_mutex_lock(&segment_mutex))) {
		sprintf(msg, "Mutex lock failed in process SEG_UNLOCK, rc %d (%s)", i, strerror(i));
		user_msg(ps, 950, HARD, msg);
		exit(i);
	}
	/* Scan the table looking for our segment... */
	for ( i = 0; i < seg_table_index; i++ )
		if ( (seg_table[i].flba == flba) && (seg_table[i].llba == llba) && (seg_table[i].tid == tid) ) break;
	if ( i == seg_table_index ) {
		/* Something's gone seriously wrong - couldn't find the segment */
		sprintf(msg, "seg_unlock(): can't find segment! i at exit = %d\n" "flba=0x%x, llba=0x%x, tid=0x%x, seg_table_index = %d\n"
		    "Table Dump:\n", i, flba, llba, (unsigned int)tid, seg_table_index);
		for (i = 0; i < seg_table_index; i++)
			sprintf(msg+strlen(msg), "  seg_table[%d]: %lld %lld %x\n", i, seg_table[i].flba, seg_table[i].llba,
			    (unsigned)seg_table[i].tid);
		user_msg(ps, 950, HARD, msg);
		exit(-1);
	} else {
		/* Found the segment - now delete it */
		seg_table_index--;
		for ( ; i < seg_table_index; i++) {
			seg_table[i].flba = seg_table[i+1].flba;
			seg_table[i].llba = seg_table[i+1].llba;
			seg_table[i].tid = seg_table[i+1].tid;
			seg_table[i].thread_time = seg_table[i+1].thread_time;
			seg_table[i].hang_count = seg_table[i+1].hang_count;
		}
		if ( strcmp(pr->oper,"BWRC") == 0 ) { /* bwrc */
			lba_fencepost = llba; /* 494699 */
			/* sprintf(msg,"fencepost %d\n",lba_fencepost);
		user_msg(ps,0,INFO,msg); */
		} /* of bwrc */
	}
	if ( (i = pthread_cond_broadcast(&segment_do_oper)) ) {
		sprintf(msg, "Broadcast failed in process SEG_UNLOCK, rc = %d (%s)", i, strerror(i));
		user_msg(ps, 950, HARD, msg);
		exit(i);
	}
	if (( i = pthread_mutex_unlock(&segment_mutex)) ) {
		sprintf(msg, "Mutex unlock failed in process SEG_UNLOCK, rc %d (%s)", i, strerror(i));
		user_msg(ps, 950, HARD, msg);
		exit(i);
	}
}


/**************************************************************************
** Initialize seed for random number generator (nrand48)
** This routine initializes the random number seed from a combination of
** the process id, a count of invocations, and the current second.
***************************************************************************/
void init_seed(struct random_seed_t *seed)
{
	char     msg[80];
	time_t   time_v;
	unsigned rc, pid;
	static unsigned short invocation_count;

	(void) time(&time_v);
	pid = (unsigned) getpid();
	seed->xsubi[0] = (unsigned short) pid;
	seed->xsubi[2] = (unsigned short) time_v;
	rc = pthread_mutex_lock(&cache_mutex);
	if ( rc ) {
		sprintf(msg, "Mutex lock failed in process INIT_SEED, rc = %d (%s)\n", rc, strerror(rc));
		user_msg(NULL, 950, HARD, msg);
		exit(rc);
	}
	seed->xsubi[1] = invocation_count++;
	rc = pthread_mutex_unlock(&cache_mutex);
	if ( rc ) {
		sprintf(msg, "Mutex unlock failed in process INIT_SEED, rc = %d(%s)\n", rc, strerror(rc));
		user_msg(NULL, 950, HARD, msg);
		exit(rc);
	}
	memset(&(seed->drand_struct), 0, sizeof(struct drand48_data));
}

/*************************************************************************
 ** returns random number of 4 bytes
 ************************************************************************/
unsigned int
get_random_number(struct random_seed_t *seed) {
	long rand_no;
	(void) nrand48_r(seed->xsubi, &(seed->drand_struct), &rand_no);
	return((rand_no & UINT_MAX));
}

/**************************************************************************
** returns random data length
***************************************************************************/
void
random_dlen(short bytpsec, unsigned long long max_blkno, struct random_seed_t *seed,
			struct ruleinfo *pr)
{
	unsigned int dlen;
	long rand_no;

	(void) nrand48_r(seed->xsubi, &(seed->drand_struct), &rand_no);
	dlen = rand_no % (pr->num_blks + 1);            /* this dlen is in blks */
	if ( dlen == 0 )
		pr->dlen = pr->bytpsec;
	else if (dlen < pr->min_blklen)                 /* RDT changes */
		pr->dlen = pr->bytpsec * pr->min_blklen;
	else
		pr->dlen = dlen * pr->bytpsec;               /* convert dlen to bytes */
}

/**************************************************************************
** sets random block number
***************************************************************************/
void
random_blkno(unsigned long long * blkno, unsigned int len, short bytpsec, unsigned long long max_blkno,
				struct random_seed_t *seed, unsigned long long min_blkno, unsigned int blk_align)
{
	int nrand48_r();
	long tmp1, tmp2;
	unsigned long long aligned_blkno, rand_no;

	(void) nrand48_r(seed->xsubi, &(seed->drand_struct), &tmp1);
	(void) nrand48_r(seed->xsubi, &(seed->drand_struct), &tmp2);
	rand_no = (unsigned long long)((unsigned long long)tmp1 << 32 | ((unsigned long long)tmp2 & 0xffffffffULL));
	aligned_blkno = min_blkno + ((unsigned long long)rand_no % (max_blkno - blk_align - min_blkno - len / bytpsec + 1));
    if(blk_align) {
        if(aligned_blkno % blk_align) {
            aligned_blkno = aligned_blkno + (blk_align - (aligned_blkno % blk_align));
        }
    }
    *blkno = aligned_blkno ;
}

/**************************************************************************
** initialize block number
** 1/9/98 chg 1 - set minimum block number to negative number
** 1/9/98 chg 2 - set maximum block number to the max_blkno
***************************************************************************/
void
init_blkno(struct htx_data *ps,struct ruleinfo *pr, unsigned long long *blkno)
{
	long long over;
	unsigned long long blk;

	pr->first_block = set_first_blk(pr);
	over = (long long)pr->first_block + (long long)pr->num_blks - (long long)pr->max_blkno;
	/*
	 *	sprintf(ps->msg_text, "first block is %lx num_blks is %lx max_blkno is %lx over is %llx",pr->first_block,pr->num_blks,pr->max_blkno,over);
  	 *	user_msg(ps, 0, INFO, ps->msg_text);
	 */
	if ( over > (long long)0 )
		blk = (unsigned long long)((long long)pr->first_block - over);
	else
		blk = pr->first_block;
	/*	sprintf(msg, "blk is %lx\n",blk);
  	 *	user_msg(ps, 0, INFO, msg);
	 */
	if ( strcmp(pr->direction,"UP") == 0 )
		blkno[0] = blkno[1] = blk;
	else if ( strcmp(pr->direction,"DOWN") == 0 )
		blkno[0] = blkno[2] = blk;
	else if ( strcmp(pr->direction,"IN") == 0 ) {
		if ( blk > (pr->min_blkno + (pr->max_blkno - pr->min_blkno) / 2) ) {
			blkno[0] = blkno[2] = blk;
			blkno[1] = pr->min_blkno - pr->num_blks - pr->increment; /* chg 1 */
		} else {
			blkno[0] = blkno[1] = blk;
			blkno[2] = pr->max_blkno;  /* chg 2 */
		}
	} else
		blkno[0] = blkno[1] = blkno[2] = blk;
	return;
}

/**************************************************************************
** set first_block number for sequential operations
***************************************************************************/
unsigned long long
set_first_blk(struct ruleinfo *pr)
{
	unsigned long long   blk;

	if(pr->hotness > 1) {
		/*
		 * If hotness is specified then we would rather make starting_block more
		 * intelligent such that it increments first_block each time as per
		 * rule file passes, not just reset it back.
		 */
		unsigned long long blks_written = 0;
		blks_written = pr->num_oper * pr->num_blks;
		if(strchr(pr->oper, '[') != NULL) {
			if(pr->opertn.compare_enabled) {
                blks_written = blks_written * pr->opertn.num_writes;
            } else {
                blks_written = blks_written * (((pr->opertn.num_writes > pr->opertn.num_reads) ? pr->opertn.num_writes : pr->opertn.num_reads));
            }
		}
		if ( (strcmp(pr->starting_block,"BOT")) == 0 )
			blk = pr->min_blkno + (read_rules_file_count -1) * blks_written ;
		else if ( (strcmp(pr->starting_block,"MID")) == 0 ) {
			if ( strcmp(pr->direction,"DOWN") == 0 ) {
				blk = pr->min_blkno + ((pr->max_blkno - pr->min_blkno) / 2) - ((read_rules_file_count - 1) * blks_written);
			} else if ( strcmp(pr->direction,"UP") == 0 ) {
				blk = pr->min_blkno + ((pr->max_blkno - pr->min_blkno) / 2) + ((read_rules_file_count - 1) * blks_written);
			} else {
				blk = pr->min_blkno + (pr->max_blkno - pr->min_blkno) / 2;
			}
		} else if ( (strcmp(pr->starting_block,"TOP")) == 0 )
			blk = pr->max_blkno - (read_rules_file_count * blks_written) - 1;
		else {
			blk = atoi(pr->starting_block);
			blk = blk + pr->min_blkno;
		}
	} else {
	    if ( (strcmp(pr->starting_block,"BOT")) == 0 )
            blk = pr->min_blkno;
        else if ( (strcmp(pr->starting_block,"MID")) == 0 )
            blk = pr->min_blkno + (pr->max_blkno - pr->min_blkno) / 2;
        else if ( (strcmp(pr->starting_block,"TOP")) == 0 )
            blk = pr->max_blkno - 1;
        else {
            blk = atoi(pr->starting_block);
            blk = blk + pr->min_blkno;
        }
	}
	if(pr->lba_align) {
		if(blk % pr->lba_align) {
			if ( (strcmp(pr->starting_block,"BOT")) == 0 )
				blk = blk + (pr->lba_align - (blk % pr->lba_align));
			else if ( (strcmp(pr->starting_block,"TOP")) == 0)
				blk = blk - (blk % pr->lba_align);
			else if ((strcmp(pr->starting_block,"MID")) == 0 ) {
				if ( ( strcmp(pr->direction,"DOWN")) == 0 )
					blk = blk - (blk % pr->lba_align);
				else
					blk = blk + (pr->lba_align - (blk % pr->lba_align));
			}
		}
	}
	return(blk);
}

/**************************************************************************
** set next block number
** chg 1 - swapped the sign from + increment to - increment
** chg 2 - swapped the sign from - increment to + increment
***************************************************************************/
void
set_blkno(unsigned long long *blkno, char *direction, int increment, unsigned int num_blks, unsigned int blk_align)
{
	unsigned long long temp;

	/*
	 * printf("%s:%d, blkno[0]=%#llx, blkno[1]=%#llx, blkno[2]=%#llx, direction=%s, increment=%#x, num_blks=%#x, blk_align=%#x \n",
	 *	__FUNCTION__,__LINE__, blkno[0], blkno[1], blkno[2], direction, increment, num_blks, blk_align);
	 */
    if ( strcmp(direction,"UP") == 0 ) {
        temp = blkno[1] + num_blks + increment;
        if(blk_align) {
            if(temp % blk_align) {
                temp = temp + (blk_align - (temp % blk_align));
            }
        }
        blkno[0] = blkno[1] = temp;
    } else if ( strcmp(direction,"DOWN") == 0 ) {
        temp = blkno[2] - num_blks - increment ;
        if(blk_align) {
            if(temp % blk_align) {
                temp = temp - (temp % blk_align);
            }
        }
        blkno[0] = blkno[2] = temp;
    } else {
        if ( blkno[0] == blkno[1] ) {
            /* chg 1 */
            temp = blkno[2] - num_blks - increment;
            if(blk_align) {
                if(temp % blk_align) {
                    temp = temp - (temp % blk_align);
                }
            }
            blkno[0] = blkno[2] = temp;
       } else {
            /* chg 2 */
            temp = blkno[1] + num_blks + increment;
            if(blk_align) {
                if(temp % blk_align) {
                    temp = temp + (blk_align - (temp % blk_align));
                }
            }
            blkno[0] = blkno[1] = temp;
        }
    }
}

/**************************************************************************
** check for file wrap around on sequential operations
***************************************************************************/
int wrap(struct ruleinfo *pr, unsigned long long blkno[])
{
	int rc = 0;

	if ( *pr->direction == 'U' ) {                    /* Direction is UP    */
		if ( blkno[1] + pr->num_blks + pr->increment > pr->max_blkno )
			rc = 1;
	} else if ( *pr->direction == 'D' ) {             /* Direction is DOWN  */
		if ( blkno[2] < pr->min_blkno )
			rc = 1;
	} else if ( *pr->direction == 'I' ) {             /* Direction is IN    */
		if ( (blkno[1] > (pr->min_blkno + (pr->max_blkno - pr->min_blkno) / 2)) || (blkno[2] < (pr->min_blkno +
		    (pr->max_blkno - pr->min_blkno) / 2)) )
			rc = 1;
	} else {                                          /* Direction is OUT   */
		if ( ((blkno[1] + pr->num_blks + pr->increment) > pr->max_blkno) || (blkno[2] < pr->min_blkno) )
			rc = 1;
	}
	return(rc);
}

/***************************************************************************
** Do boundary checkings.
****************************************************************************/
void
do_boundary_checks(struct ruleinfo *pr, struct htx_data *ps, unsigned long long *blkno,
				int * saved_dir_flag, int * do_partial) {

	char msg[MSG_TEXT_SIZE];

	if ( *pr->direction == 'I' ) {
    	/* If the direction is "IN" then we want to make sure that we don't */
        /* run off the end of the disk, either at the top or the bottom of  */
        /* disk. If we set the lba to an unusable lba then we just reset it */
        /* back to the original starting point and begin again.             */
        if ( (blkno[0] < pr->min_blkno) ||                     /* chg 2 */
        	((blkno[0] + pr->num_blks + pr->increment) > pr->max_blkno) ) {
             init_blkno(ps,pr,blkno);
             *saved_dir_flag = 0;
        }
    } else if ( *pr->direction == 'O' ) {
    	/* If the direction is "OUT" then we want to make sure that we do   */
    	/* then entire disk but that we don't try to access a bad lba.      */
    	if ( blkno[0] > pr->max_blkno )
    		init_blkno(ps,pr, blkno);
    	else if ( (blkno[0] + pr->num_blks) >= pr->max_blkno ) {
    		pr->num_blks = pr->max_blkno - blkno[0];
    		pr->dlen = pr->num_blks * pr->bytpsec;
    		*saved_dir_flag = 2;
    	} else if ( blkno[0] < pr->min_blkno ) {
    		if ( (blkno[0] + pr->num_blks) > pr->min_blkno ) {
    			pr->num_blks = (blkno[0] + pr->num_blks) - pr->min_blkno;
    			pr->dlen = pr->num_blks * pr->bytpsec;
            	blkno[0] = pr->min_blkno;
            	*saved_dir_flag = 2;
        	} else
            	init_blkno(ps,pr, blkno);
        }
    } else if(wrap(pr, blkno)) {
    	/* On a sequential transfer going from the bottom of the disk       */
        /* to the top or from the top to the bottom of the disk and there   */
        /* is less than num_blks left & partial xfer is required to finish. */
        /* Then re-calc the residual and adjust the data length, if         */
        /* less than num_blks left and partial xfer not required, then      */
        /* reset back to the starting block number.                         */
        if ( *do_partial ) {
        	/* 494699 */
            if ( *pr->direction == 'U') { /* UP direction */
           		if ((pr->max_blkno - blkno[1]) < (pr->num_blks + pr->increment))
                	pr->num_blks = (int)(pr->max_blkno - blkno[1]); /* calc partial blks */

                } else if ( *pr->direction == 'D') { /* DOWN direction */
                    if ((pr->num_blks + blkno[2]) > pr->min_blkno)
                        pr->num_blks = pr->num_blks - (int)(pr->min_blkno - blkno[2]); /* calc partial blks */
					/* Not sure why we need to change the LBA # ??? -- Piyush
                    blkno[0] = blkno[2] = pr->min_blkno;
					*/
                } else {
                    init_blkno(ps,pr, blkno); /* unexpected direction - init blk to be safe */
                }
                *do_partial = 0; /* clear do_partial flag, now we've used it for this stanza */
                pr->dlen = pr->num_blks * pr->bytpsec;
                if ( pr->messages[0] != 'N' ) {
                    sprintf(msg,"Partial xfer required starting at\n" "block number %lld (0x%llx).  Transfer length will be\n"
                        "%d (0x%x) blocks (%d bytes).", blkno[0], blkno[0], pr->num_blks,
                         pr->num_blks, pr->dlen);
                  	user_msg(ps, 0, INFO, msg);
                }
            } else { /* Dont do this for random addressing.. DANGEROUS!!! */
                if (strcmp (pr->addr_type, "RANDOM") != 0) /* 494699 */
                    init_blkno(ps,pr, blkno);
            }
    }
}


/***************************************************************************
** set read buffer to binary 0xbb
****************************************************************************/
void
clrbuf(char buf[], unsigned int dlen)
{
	memset(buf, 0xbb, dlen);
}

/**************************************************************************
** build write buffer sequences
** Add header stuff and improve algorithm.
** Added new algorithm '7' for performance.
***************************************************************************/
int
bldbuf(unsigned short *wbuf, struct random_seed_t *seed, unsigned long long * blkno,
		struct htx_data *ps, struct ruleinfo *pr, char algorithm, unsigned short write_stamping)
{
	unsigned lba_low16;
	unsigned long long lba;
	int i, j, k, cksum = 0, lba_count, zz;
	unsigned int gen_time ;
	unsigned char * char_ptr;
	unsigned long long * ull_ptr;
	long tmp;
	int step=0, bufsize;
	char *msg;
	extern int num_sub_blks_psect;
	char diskname[9], diskname2[15],new_diskname[9];
	strcpy(diskname2, basename(ps->sdev_id));
	step = strlen(diskname2);

	diskname[0] = diskname2[1];
	diskname[1] = diskname2[2];
	do{
		--step;
	}while( isdigit (diskname2[step]));
	strcpy(diskname+2, diskname2+step+1);
	tmp = strlen(diskname);
	diskname[tmp] = ' ';
	diskname[tmp+1] = 0;
	for (j=tmp+2;j<9;j++)
		diskname[j] = 0;
	/* Retrieve the number of seconds since Epoch (00:00:00 GMT, 1/1/1970) */
	gen_time = (unsigned int)(time(0) & 0xFFFFFFFFu);
	lba = *blkno;
	lba_count = pr->dlen / pr->bytpsec;
	switch ( algorithm ) {
	case '1':
	case '2':
		for ( i = 0, j = 1; lba_count > 0; i += 128, ++j, ++lba, --lba_count ) {
			lba_low16 =  lba & 0xffff ;
			ull_ptr = (unsigned long long *) wbuf; /* get d-word ptr */ 			/* wbuf + 0 */
			*(unsigned long long *)ull_ptr = lba;									/* wbuf =lba  */
			wbuf += (sizeof(unsigned long long) / sizeof(unsigned short));			/* wbuf += 4 */
			step = (sizeof(unsigned long long) / sizeof(unsigned short));
			*(unsigned short * )wbuf = (unsigned short)seed->xsubi[0];
			wbuf++;																	/* wbuf += 2 */
			step ++;
			*(unsigned short * )wbuf = (unsigned short)seed->xsubi[1];
			wbuf++;
			step++;
			*(unsigned short *)wbuf = (unsigned short)seed->xsubi[2];
			wbuf ++;
			step++;
			nrand48_r(seed->xsubi, &( seed->drand_struct ), &tmp);
			*(unsigned short *)wbuf = (unsigned short)tmp;
			wbuf++;
			step++;
			for ( k = step; k < pr->bytpsec/sizeof(unsigned int) ; k++ ) {  /* We are writing one word at a time */
				*(unsigned short *)wbuf = i + k;
				wbuf ++;
				if ( algorithm == '1' )
					*wbuf = j * 0x101;
				else {
					(void) nrand48_r( seed->xsubi, &( seed->drand_struct ), &tmp );
					*wbuf = tmp;
				}
				wbuf ++;
			}
		}
		break;
	case '3':
	case '4':
	case '5':
	case '7':
	case '8':
	case '9':
	case 'A':
		for ( ; lba_count > 0; ++lba, --lba_count ) {
			/*  Format of the 48-byte header (first 48 bytes of every pr->bytpsec
   	     	**  bytes of the buffer):
       	 	**  8 bytes = LBA number
        	**  4 bytes = Time stamp (seconds since hxehd started)
        	**  8 bytes = ASCII device name
        	**  8 bytes = Stanza ID generating this buffer
        	** 12 bytes = ASCII host name
        	**  2 bytes = Pattern algorithm in binary/if Hotness then WriteStamping.
        	**  4 bytes = Buffer Signature (BUFSIG)
        	**  2 bytes = Checksum
        	**  Total Overhead = 48 bytes (24 shorts)
        	**  The modulo-16 sum of all bytes should be 0.
        	*/
			#define OVERHEAD (48/2)
			#define BUFSIG "MDHF"
			for(k = 1,zz = 0; zz <= num_sub_blks_psect; k++, zz++) {
				lba_low16 =  lba & 0xffff ;
				char_ptr = (unsigned char *) wbuf;
				memcpy(char_ptr, &lba, 8); 											/* 8 byte LBA number */
				char_ptr += 8;
				memcpy(char_ptr, &gen_time, 4); 									/* 4 byte Timestamp  */
				char_ptr += 4;
				/* in 64 bit time is 8 bytes info */
				if(num_sub_blks_psect)
					strncpy( (char *)(char_ptr), diskname, 8 ); 					/* 8 byte Device name */
				else
				{
					strcpy( new_diskname, basename( ps->sdev_id ));
					if(new_diskname[0] == 'r')
						strncpy( (char *)(char_ptr), &new_diskname[1], 8 ); 		/* 8 byte Device name */
					else
						strncpy( (char *)(char_ptr), new_diskname, 8 ); 			/* 8 byte Device name */
				}
				char_ptr+= 8;
				memcpy((char *)(char_ptr), pr->rule_id, 8 );             			/* 8 byte Stanza Name */
				char_ptr+= 8;
				memcpy((char *)(char_ptr), hostname, 12 );              			/* 12 byte Host name */
				char_ptr +=12;
				if(num_sub_blks_psect) {
					*(unsigned int *)char_ptr = (unsigned int) wbuf; 				/* 4 byte wbuf base address */
					char_ptr += 4;
					*(unsigned short *)char_ptr = 0xa5a5;							/* 2 byte 0xa5a5 */
					char_ptr += 2;
					if(pr->hotness > 1) {
						*(unsigned short *)char_ptr = write_stamping;				/* Identify wach write on same LBA sets uniquely */
					} else {
						*(unsigned short *)char_ptr = (algorithm - '0'); 			/* 2 byte Algorithm */
					}
					char_ptr += 2;
					memcpy(char_ptr, BUFSIG, 4 );									/* 4 byte Buffer Signature */
					/* Calculate the checksum value by adding up the above values */
					/* and calculating the two's complement so that overall sum is 0. */
					for ( i = 0, cksum = 0; i < OVERHEAD - 1; i++ )
						cksum += *(wbuf+i);
					*(wbuf+(OVERHEAD-1)) = ~( cksum & 0xffff ) + 1; 				/* 2 byte Check-Sum  */
					/****************************************************************/
					/* Done with header population, adjust my input buffer and		*/
					/* begin the pattern generation 								*/
					/****************************************************************/
					wbuf += OVERHEAD;
					/*
					if(wbuf != (unsigned short *)(char_ptr + 2))
						printf("Wuff Wuff !! Code bug, wbuf = %#x, char_ptr = %#x \n", wbuf, char_ptr);
					*/
					/***************************************************************
					 * UNIQUE_PATTERNS :
					 * bytes  description
					 * -----  ------------------------------------------
					 * 0 - 1  algorithm
					 * 2 - 3  Counter (Example 0x0001, 0x0002 ...).
				     * 4 - 7  LBA  Counter. (recreatable pattern)
					 * 8 - 15 Device name.
					 **************************************************************/

					for(j=((pr->bytpsec /(num_sub_blks_psect +1) - OVERHEAD*2)/16); j; j--) {
						unsigned int *uint_ptr;
						char *char_ptr;
						int z;

						uint_ptr = (unsigned int *)wbuf;
						*(wbuf)=(algorithm - '0');
						*(wbuf+1)=k;
						*(uint_ptr + 1) = lba+k;
						k++;
						char_ptr = (char *) (((unsigned int *)uint_ptr)+2);
						for (z=0; z<8; z++)
							*(char_ptr+z) = diskname[z];
						wbuf += 8;
					}
				}
				else {
                    if(pr->hotness > 1) {
                        *(unsigned short *)char_ptr = write_stamping;               /* Identify wach write on same LBA sets uniquely */
                    } else {
                        *(unsigned short *)char_ptr = (algorithm - '0');            /* 2 byte Algorithm */
                    }
					char_ptr+= 2;
					memcpy(char_ptr, BUFSIG, 4); 									/* 4 byte Buffer Signature */
					char_ptr += 4;
					/* Calculate the checksum value by adding up the above values     */
					/* and calculating the two's complement so that overall sum is 0. */
					for ( i = 0, cksum = 0; i < OVERHEAD - 1; i++ )
						cksum += *(wbuf+i);
					*(wbuf+(OVERHEAD-1)) = ~( cksum & 0xffff ) + 1;          		/* 2 byte Check-Sum */

					/****************************************************************/
					/* adjust for the header and begin the pattern generation 		*/
					/****************************************************************/
					wbuf += OVERHEAD;
					/*
					if(wbuf != (unsigned short *)(char_ptr + 2))
						printf("Wuff Wuff !! Code bug, wbuf = %#x, char_ptr = %#x \n", wbuf, char_ptr);
					*/
					if ( algorithm == '3' ) {
						/*************************************************************
						 * PATTERN_ID = 3
						 * bytes  description
						 * -----  ------------------------------------------
						 * 0 - 1  relative word number (ex. 0000, 0001, 0002, ...)
					     * 2 - 3  recreateable pattern (ex. add the one to the starting LBA to get your
					     *					 pattern.
						 ************************************************************/
						for ( k = 1, j = pr->bytpsec / 2 - OVERHEAD; j; j -= 2 ) {
							*wbuf++ = k++;
							*wbuf++ = lba_low16++;
						}
					} else if ( algorithm == '4' ) {
						/***********************************************************
						* PATTERN_ID #004
						* bytes  description
						* -----  ------------------------------------------
					  	* 0 - 1  fixed pattern of A5A5.
						***********************************************************/
						for ( j = pr->bytpsec / 2 - OVERHEAD; j; j-- )
							*wbuf++ = 0xa5a5;
					} else if (algorithm == '5' ) {
						/**********************************************************
						 * PATTERN_ID #005
						 * bytes  description
						 * -----  ------------------------------------------
						 * 0 - 1  relative word number (ex. 0000, 0001, 0002, ...)
					 	 * 2 - 3  fixed pattern of A5A5.
						 **********************************************************/
						for ( k = 1, j = pr->bytpsec / 2 - OVERHEAD; j; j -= 2 ) {
							*wbuf++ = k++;
							*wbuf++ = 0xa5a5;
						}
					} else {
						/* Come here for Pattern 7, pattern 8, pattern9, pattern A */
					    /**************************************************************************
    					 * Input Format :
    					 * ARGUMENT 1 : Pattern  Buffer
    					 * ARGUMANT 2 : Buffer Size
    					 * ARGUMANT 3 : Pattern Type
    					 * ARGUMENT 4 : Line Length
    				 	 * ARGUMENT 5 : HTX DS
						 * ARGUMENT 6 : Rules data Structure.
    					 *************************************************************************/
						bufsize = pr->bytpsec - (OVERHEAD * 2); /* need buffer size in bytes */

						if(generate_pattern((char *)wbuf, bufsize, algorithm, pr->align, seed, ps, pr) == -1) {
							strcpy( msg, "Exerciser internal error - Not able to generate patterns \n");
							sprintf( msg + strlen( msg ), "bldbuf: pattern_id = \'%c\' (0x%x).", pr->pattern_id, pr->pattern_id);
							user_msg(ps, 950, HARD, msg);
							memset( wbuf, '?', pr->dlen );
    				        free( msg );
						}
						wbuf += (bufsize/sizeof(unsigned short));
					}
				}
			}
		}
		break;
	case '6':
		for ( ; lba_count > 0; ++lba, --lba_count )
			for ( j = pr->bytpsec / 2; j; j -= pr->pat_cnt )
				for ( k = 0; k < pr->pat_cnt; k++ )
					*wbuf++ = pr->form[k];
		break;
	default:
		msg = malloc( 256 );
		if ( msg ) {
			strcpy( msg, "Exerciser internal error - default switch taken in\n" );
			sprintf( msg + strlen( msg ), "bldbuf: pattern_id = \'%c\' (0x%x).", pr->pattern_id, pr->pattern_id
			    );
			user_msg(ps, 950, HARD, msg);
			memset( wbuf, '?', pr->dlen );
			free( msg );
		} else
			exit(0);
		break;
	} /* end switch */

}

int
get_line( char s[], int lim, FILE *fp)
{
    int c=0,i;

    i=0;
    while (--lim > 0 && ((c = fgetc(fp)) != EOF)) {
        s[i++] = c;
    }

   	s[i] = '\0';
    return(i);
}


/************************************************************************
** cmpbuf - Compare read buffer to write buffer. If a miscompare is de-
** tected and crash_on_mis = YES, the system will crash and go into the
** the kernel debugger. The parameters to trap() will be loaded in
** the CPU registers. The flag value 0xBEFFDEAD will in the first reg;
** pointers to the wbuf and rbuf will be in the 2nd and 3rd regs. The
** offset into the miscompare will be in the 4th reg and the 5th reg
** will hold a pointer to the device causing the exerciser to go into
** the debugger.
** 04/18/97: modified to look for header stored in bldbuf above.
** 07/07/97: pulled comparison routine into independent subroutine so
**           it could be used both for read and re-read compare.
*************************************************************************/
#define DUMP_PATH "/tmp/"
#define MAX_MISCOMPARES 11
#define MAX_MSG_DUMP 20
#define MAX_DUMP_DATA_LEN MAX_TEXT_MSG

char cmpbuf( struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long  * blkno,
											char * wbuf, char * rbuf, unsigned int nwrites )
{
	unsigned int offs;   				/* Offset of miscompare if found                */
	int badsig = 0, cksum = 0;

  	offs = do_compare(ps, pr, wbuf, rbuf, crash_on_miscom, &badsig, &cksum, loop, *blkno, nwrites);
  	if ( offs != -1 ) {     							/* problem with the compare?                 */

		char * reread = NULL, * save ;        			/* Pointer to a reread-buffer (if necessary)    */
		int offs_reread = -1, alignment = 0;    		/* Offset of error in re-read if non-zero       */
		char stanza_save[16];	     					/* This string holds the original stanza name   */
	    char s[3];                      		        /* string segment used when building error msg  */
    	char work_str[512], cmd[256];           		/* work string                                  */
    	int rc, bufrem = 0;                        		/* return code flag                             */
    	int cnt;                         				/* internal miscompare count and checksum accum */
    	char msg1[MAX_TEXT_MSG];                		/* message text character array (string)        */
    	static ushort miscompare_count = 0;     		/* miscompare count                             */
    	extern int num_sub_blks_psect;
    	char path[128], path_w[128], path_r[128];     /* dump files path                              */
    	char *wbuffer, *buff;
		unsigned int temp;
    	FILE *fp;

    	ps->bad_others++;
    	if ( lba_fencepost < pr->max_blkno )
      		sprintf(msg1, "Miscompare at buffer offset %d (0x%x)  "
                    "LBA FencePost = %#llx", offs, offs, lba_fencepost);
   	 	else
      		sprintf(msg1, "Miscompare at buffer offset %d (0x%x)  "
                    "Maximum LBA = %#llx", offs, offs, pr->max_blkno);
    		sprintf(work_str, "\n(Flags: badsig=%d; cksum=0x%x)", badsig, cksum);
    		strcat(msg1, work_str);
    		sprintf(work_str, "\nwbuf (baseaddr 0x%x) ",wbuf);
    		strcat(msg1, work_str);
    		if ( offs > 0 )
       			--offs;
    		for ( temp=offs; (offs-temp < MAX_MSG_DUMP) && (offs < pr->dlen); offs++ ) {
      			sprintf(s, "%0.2x", wbuf[offs]);
      			strcat(msg1, s);
    		}
    		sprintf(work_str, "\nrbuf (baseaddr 0x%x) ", rbuf);
    		strcat(msg1, work_str);
    		for ( offs=temp; (offs-temp < MAX_MSG_DUMP) && (offs < pr->dlen); offs++ ) {
      			sprintf(s, "%0.2x", rbuf[offs]);
      			strcat(msg1, s);
    		}
    		strcat(msg1, "\n");
    		if ( run_reread == 'Y' ) {

	            if( pr->align > 0 ) { /* If align is specified then IO buffers are aligned to this */
   	             	alignment = pr->align;
            	} else if(pr->lba_align > 0) { /* If not specified, Check if LBA align is specfied */
                	alignment = pr->align *  pr->bytpsec ;
            	} else if(pr->pattern_id[0] == '#' &&       /* These algorighms needs a min alignment */
   			                  (	pr->pattern_id[3] == '7' ||
            		           	pr->pattern_id[3] == '8' ||
                      			pr->pattern_id[3] == '9' ||
                      			pr->pattern_id[3] == 'A'  )) {
                	alignment = 16 ;
                	pr->align = 16 ;
                	pr->lba_align = 0;
            	} else { /* If nothing specified then use default as - dword */
                	pr->align = pr->lba_align = alignment = 0;
            	}

       			if ( (reread = malloc( pr->dlen + alignment + 128)) == NULL ) {
         				sprintf(work_str, "**> Can't malloc re-read buf: "
                           "errno %d (%s) - re-read not done!\n",
                  		errno, strerror(errno));
         			strcat(msg1, work_str);
       			} else {
					save = reread;
					/* Align re-read bufer if required */
		            if(alignment) {
                		bufrem = ((unsigned)reread % (alignment));
                		if ( bufrem != 0 )
                   			reread = reread + (alignment - bufrem);
    		        }
         			/* Save the original stanza name and replace it with "!Re-Read"
          			 * so that if read_disk gets an error and HTX halts it before
          			 * it returns control to us, it's obvious that the error occurred
          			 * during a re-read operation.  After read_disk returns, we'll
          			 * copy the original stanza name back.
          			 */
         			strcpy(stanza_save, pr->rule_id);
        	 		strcpy(pr->rule_id, "!Re-Read");
         			rc = read_disk(ps, pr, loop, blkno, reread);
         			strcpy(pr->rule_id, stanza_save);
         			if ( rc ) {
           				sprintf(work_str, "***> Error trying to re-read disk bfr!\n");
           				strcat(msg1, work_str);
           				free(save);
           				reread = (char *) 0;
         			} else {
           				offs_reread = do_compare(ps, pr, wbuf, reread, 0, &badsig, &cksum,
                                    loop, *blkno, nwrites);
						if(offs_reread != -1) {
							/* If reread fails then we treat it as a real miscompae */
							 rc = pthread_mutex_lock(&cache_mutex);
				            if ( rc ) {
                				sprintf(msg1, "Mutex lock failed in process CMPBUF, rc = %d\n", rc);
                				user_msg(NULL, 950, HARD, msg1);
                				free(save);
								exit(0);
            				}
            				miscompare_count++;
            				cnt = miscompare_count;
    						rc = pthread_mutex_unlock(&cache_mutex);
    						if ( rc ) {
      							sprintf(msg1, "Mutex unlock failed in process CMPBUF, rc = %d\n", rc);
      							user_msg(NULL, 950, HARD, msg1);
								free(save);
      							exit(0);
    						}
						}
					}
				}
			}
    		if ( cnt < MAX_MISCOMPARES ) {
				/* Save the wbuf */
      			strcpy(path_w, DUMP_PATH);
      			strcat(path_w, "htx");
      			strcat(path_w, &(ps->sdev_id[5]));
      			strcat(path_w, ".wbuf");
      			sprintf(work_str, "%-d", cnt);
      			strcat(path_w, work_str);
				hxfsbuf(wbuf, pr->dlen, path_w, ps);
				sprintf(work_str, "Write buffer saved in %s\n", path_w);
                strcat(msg1, work_str);
				/* Save the rbuf */
                strcpy(path_r, DUMP_PATH);
                strcat(path_r, "htx");
                strcat(path_r, &(ps->sdev_id[5]));
                strcat(path_r, ".rbuf");
                sprintf(work_str, "%-d", cnt);
                strcat(path_r, work_str);
                hxfsbuf(rbuf, pr->dlen, path_r, ps);
                sprintf(work_str, "Read buffer saved in %s\n", path_r);
                strcat(msg1, work_str);
				/* Save re-read buffer */
                if ( run_reread == 'Y' ) {
                    if ( reread != (char *) 0 ) {
                        if ( offs_reread != -1 ) {
                            strcpy(path, DUMP_PATH);
                            strcat(path, "htx");
                            strcat(path, &(ps->sdev_id[5]));
                            strcat(path, ".rerd");
                            sprintf(work_str, "%-d", cnt);
                            strcat(path, work_str);
                            hxfsbuf(reread, pr->dlen, path, ps);
                            strcat( msg1, "Re-read fails compare at offset " );
                            sprintf(work_str, "%d; buffer saved in %s\n",
                                                offs_reread, path);
                            strcat(msg1, work_str);
                        } else
                            strcat(msg1, "Re-read compares OK; buffer not saved.\n");
                    }
					if ( reread != (char *) 0 )
                    	free(save);
                }

      			prt_msg(ps, pr, loop, blkno, 950, MISCOMPARE, msg1);

		      	/* Following has been added for SMART tool debug */
            	if ( system("ls /usr/lpp/htx/bin/smart 2> /dev/null") == 0 )  { /* SMART tool is present */
      				wbuffer = (char*) malloc(pr->dlen + 512);  /* Initial 512 bytes are used to store data required by smart tool in wbuf */
      				memcpy(wbuffer, current_ruleptr, 510);
      				strcat(wbuffer, "\0 \n");

      				/* time stamp @0x100 */
      				buff = (char*)wbuffer + 256;
      				memcpy(buff, &time_mark, sizeof(time_mark));

      				/* Address of read buffer @0x110 */
      				buff = wbuffer + 272;
      				memcpy(buff, &rbuf, sizeof(rbuf));

      				/* Address of write buffer @0x120 */
      				buff = wbuffer + 288;
      				memcpy(buff, &wbuf, sizeof(wbuf));

      				buff = (char*)wbuffer + 512;
      				memcpy(buff, wbuf, pr->dlen);
					/* Adjust path_w to point to wbuffer used for smart */
					strcpy(path_w, DUMP_PATH);
                	strcat(path_w, "htx");
                	strcat(path_w, &(ps->sdev_id[5]));
                	strcat(path_w, ".wbuf");
                	sprintf(work_str, "%-d", cnt);
                	strcat(path_w, work_str);
                	hxfsbuf(wbuffer, pr->dlen, path_w, ps);

			    	/* Miscompare reported, invoke the smart tool */
                	sprintf(cmd, "/usr/lpp/htx/bin/smart r hxehd %s %s ", path_r, path_w);
                	fp  = popen(cmd, "r");
                	if (fp == NULL) {
                    	/* Error messg */
                    	sprintf(work_str, "Popen failure in invoking the smart tool errno:%d", errno);
                    	user_msg(ps, errno, SOFT, work_str);
                    	return(1);
            		} else {
                		char * msg = NULL;
                		msg = (char *)malloc(MAX_DUMP_DATA_LEN);
                		if(msg == NULL) {
                    		sprintf(work_str, "malloc failed for msg buffer for SMART errno = %d\n",errno);
                    		user_msg(ps, errno, SOFT, work_str);
                    		return(1);
                		}
                		rc = get_line(msg, MAX_DUMP_DATA_LEN, fp);
                		if (rc == 0) {
                    		/* Error  */
                    		sprintf(work_str, "failure in retrieving miscompare data from SMART tool, rc = %d \n", rc);
                    		user_msg(ps, errno, SOFT, work_str);
                    		free(msg);
                    		return(1);
                		}
                		prt_msg(ps, pr, loop, blkno, 950, MISCOMPARE, msg);
                		pclose(fp);
                		free(msg);
            		}
					free(wbuffer);

				}
      			if ( strcmp(pr->oper, "BWRC") == 0 )
         			backgrnd_thread = 'E';
    		} else {
      			if ( strcmp(pr->oper, "BWRC") == 0 )
       				backgrnd_thread = 'E';
   				sprintf(work_str, "The maximum number of saved miscompare "
                       		"buffers (%d) have already\nbeen saved. "
                       		"The read and write buffers for this miscompare "
                       		"will\nnot be saved to disk.\n", MAX_MISCOMPARES);
   				strcat(msg1, work_str);
   				prt_msg(ps, pr, loop, blkno, 950, MISCOMPARE, msg1);
   				if ( run_reread == 'Y' ) {
       				if ( reread != (char *) 0 )
      					free(save);
   				}
   				return(1);
   			}
	}
  	return(0);
}
#ifndef __HTX_LINUX__

int generate_pci_config(struct htx_data *ps, char *devname)
{
	int filedes;
	struct  mdio    md_io;
	int rc_ioctl;
	char val_ch[100], devicename[20], debugfile[30];
	FILE *fp;
	int i;

	sprintf(debugfile,"/tmp/pci.%s",devname);
	fp = fopen(debugfile,"w");
	if (fp == NULL)
	{
		return -1;
	}
	fprintf(fp, "==============LOG Started==============\n");

	for (i=0; strlen(PCI_parent[i]); i++)
	{
		sprintf(devicename,"/dev/%s",PCI_parent[i]);
		filedes = eeh_open(ps,devicename,O_RDONLY);
		strcpy(val_ch,"HELL");
		if (filedes < 0)
		{
			return -1;
		}

		md_io.md_addr = 0;
		md_io.md_incr = MV_WORD;
		md_io.md_size = 1;
		md_io.md_sla = PCI_devfn[i];
		md_io.md_data   = (char *) &val_ch;
		rc_ioctl      = eeh_ioctl(ps,filedes,MIOPCFGET,&md_io);

		for (i=0; i<4; i++)
			fprintf(fp, "rc_ioctl = %d, value = %x\n", rc_ioctl, val_ch[i]);

		eeh_close(ps,filedes);
	}
	fprintf(fp, "==============LOG Stoped==============\n\n");
	fclose(fp);
}
#endif
/******************************************************************************
** do_compare - actually does the compare of two buffers.  Checks for a valid
** header at the start of each sector as is found in pattern types 3, 4, and
** 5.  Returns -1 if compare is OK, offset of error if not.
*******************************************************************************/
int do_compare( struct htx_data *ps, struct ruleinfo *pr, char wbuf[],
						char rbuf[], int crash_flag, int *badsig, int *cksum,
						int loop, unsigned long long blkno, /* last two needed for error messages */
						unsigned int nwrites) /* If write was done before RC */
{
	int            error_found, offset;
	char           msg[MAX_TEXT_MSG];   /* message text character array       */
	time_t         time_buffer;
	register unsigned int  i, j;
	unsigned short *sptr;
	extern int num_sub_blks_psect;
	int cmp_bytes = (pr->bytpsec/(num_sub_blks_psect +1));

	error_found = 0;
	for ( i = 0; i < pr->dlen && !error_found; i += cmp_bytes) {
		/* Check for a valid algorithm 3, 4, or 5 HTX pattern in this lba */
		/* Both the wbuf and the rbuf have to have the signature before   */
		/* we will ignore it in the comparison. Check if the oper         */
        /* is a write followed by a read/compare. If so,                  */
        /* then check all bytes including the header.                     */
		*badsig = 1;
		if ( memcmp( rbuf + i + 42, BUFSIG, strlen(BUFSIG)) ||
			 memcmp( wbuf + i + 42, BUFSIG, strlen(BUFSIG)) ||
			 memcmp( rbuf + i + 12, wbuf + i + 12, 8) ||
             (nwrites > 0 ))
			offset = 0;                      /* Bad signature - start compare at 0 */
		else {
			/* Check time stamp to make sure we aren't doing   */
			/* a compare against stale data.                   */
			#ifdef  __HTX_LE__
				time_buffer = rbuf[11] << 24 | rbuf[10] << 16 | rbuf[9] << 8 | rbuf[8];
			#else
				time_buffer = rbuf[8] << 24 | rbuf[9] << 16 | rbuf[10] << 8 | rbuf[11];
			#endif
			if ( time_buffer < time_mark ) {
				sprintf(msg, "The buffer time stamp  (0x%x) is earlier than\n" "the control time stamp (0x%x).  Possible stale data"
				    " was written to\n" "the disk or disk was not initalized during this run.\n", time_buffer,
				    time_mark);
				prt_msg(ps, pr, loop, &blkno, pr->max_blkno, SOFT, msg);
				error_found = 1;
			}
			/* Calc the sum of 1st OVERHEAD shorts. Note: the  */
			/* sum was gen'd from an array of shorts so use a  */
			/* (short *) to index it for calculating the sum.  */
			*badsig = 0;
			*cksum = 0;
			sptr = (unsigned short *) &rbuf[i];
			for ( offset = 0; offset < OVERHEAD; offset++ )
				*cksum += sptr[offset];
			/* If sum is 0 then we don't want to compare the   */
			/* 1st OVERHEAD*2 bytes, else we want to compare   */
			/* the whole block. Note that wbuf is an array of  */
			/* chars so we have to double the OVERHEAD value   */
			/* (it's in shorts.) Also, if sum is 0 then the    */
			/* LBA fields should be compared as a final check  */
			/* to makie sure the header can be ignored.        */
			if ( ( *cksum & 0xffff ) || ( *((unsigned long long *)(rbuf+i)) != *((unsigned long long *)(wbuf+i)) ) ) {
				offset = 0;
				/*sprintf(msg_jk,"do compare: check sum %x unsigned rbuf %x unsigned wbuf %x\n",
		  		(*cksum & 0xffff), *(unsigned *)rbuf,*(unsigned *)wbuf);
           		prt_msg(ps, pr, loop, &blkno, pr->max_blkno, SOFT, msg_jk);*/
			} else
				offset = OVERHEAD * 2;
		}
		/* To prevent failure reporting at the begining of next block */
		/* set offset to time stamp  */
		if ( error_found ) {
			user_msg(ps,0,INFO,"error found step1 time ");
			j = i + 4; /* time stamp */
			break;
		}

		j = i + offset;
		/* Perf : First compare using memcmp, if mismatch found then
		 * return offset on char boundary
		 */
		if(memcmp(&wbuf[j], &rbuf[j], (cmp_bytes - offset))) {
			while (offset++ < cmp_bytes) {
				if ( wbuf[j] != rbuf[j] ) {
					#ifndef __HTX_LINUX__
					if (la_on_miscom) {
						if (generate_pci_config(ps,(ps->sdev_id)+5) < 0) {
							user_msg(ps,0,INFO,"LA trigger failed on miscompare\n");
						}
					}
					#endif
					if (turn_attention_on == 1) {
					#ifndef __HTX_LINUX__
						attn( 0xBEEFDEAD, (unsigned int)wbuf, (unsigned int)rbuf, j, (unsigned int)ps, (unsigned int)pr, pr->dlen );
					#else
						__attn(0xBEEFDEAD, (unsigned int)wbuf, (unsigned int)rbuf, j, (unsigned int)ps, (unsigned int)pr, pr->dlen );
					#endif
					}
					if ( run_on_misc == 'Y' )
						system(misc_run_cmd);

					if ( crash_flag ) {
						#ifndef __HTX_LINUX__
							trap( 0xBEEFDEAD, wbuf, rbuf, j, ps, pr, pr->dlen );
						#else
							do_trap_htx64( 0xBEEFDEAD, wbuf, rbuf, j, ps, pr, pr->dlen );
						#endif
					}
					error_found = 1;
					break;
				}
				++j;
			}
		}
	}
	return( error_found ? j : -1 );
}

/*****************************************************************************
** run_timer()
**  - routine to watch for hung I/O threads.
**    This routine scans through the segment lock table, checking the start
**    time of each pending I/O.  When an I/O in the table exceeds an integral
**    multiple of hang_time seconds, the table is printed in an HTX message
**    and it's associated hang counter variable is incremented.  If none of
**    the hang counters exceed a threshold, then the HTX message is constructed
**    with an error status of SOFT.  If any I/O exceeds the threshold for hang
**    count, then the error status is set to HARD.
**
**    This routine is started as an independent thread by main() and runs for
**    the duration of the exerciser.
******************************************************************************/
void run_timer(struct htx_data *ps, struct ruleinfo *pr)
{
	#define MSG1_LIMIT (80 * 15)                 /* Allow for 15 80-char lines */

	int     i, err_level, hung_switch;
	int     threshold_exceeded = 0;
	char    msg[80 * 2], msg1[MSG1_LIMIT + 25];
	pid_t   pid;
	time_t  current_time, oldest_time, temp_time;

	pid = getpid();                                      /* Get our process ID */
	while ( 1 ) {
		if(exit_flag == 'Y') {
			/* Exit ...... */
			break;
		}
		/* Acquire the segment table mutex so table won't change on us... */
		if ( (err_level = pthread_mutex_lock(&segment_mutex)) ) {
			sprintf(msg, "Mutex lock failed in run_timer(), rc %d (%s)\n" "Hung I/O monitor for disk exerciser terminating.",
			    err_level, strerror(err_level));
			user_msg(ps, 950, HARD, msg);
			return;
		}
		/************************************************************************
      	** After each scan of the table, we will go to sleep until it's time to
      	** scan the table again.  However we want to adjust the amount of time we
      	** sleep so that we wake up in time to check the oldest thread in the
      	** table for the hang condition.  Thus, if the hang_time var is set for
      	** 5 seconds, and the oldest thread in the table started 1 second ago, we
      	** only want to sleep for 5-1=4 seconds so that we check that thread as
      	** close to the 5 second mark as possible.
      	*************************************************************************/
		oldest_time = current_time = time(0);
		hung_switch = 0;
		/* seg_table_index always points to the next empty table entry. */
		for ( i = 0; i < seg_table_index; i++ ) {
			/************************************************************************
      		** Check if I/O time exceeds a multiple of the hang_time var.  The object
      		** is to only issue a new alert when a multiple of hang_time has been
      		** exceeded.  Thus if hang_time is 5 seconds, we'll issue an alert
      		** message at 5, 10, 15, ... seconds.  The multiplier is provided by the
      		** hang_count var for that I/O, which initially is 0 when the I/O is
      		** started and is incremented each time a hang is detected.
      		*************************************************************************/
			if ( (current_time - seg_table[i].thread_time) >= (hang_time * (seg_table[i].hang_count + 1)) ) {
				/* Found a hung I/O!  Incr it's hang_count and check the threshold. */
				if ( ++seg_table[i].hang_count > threshold )
					threshold_exceeded = 1;
				++hung_switch;
			}
			/*******************************************************************
           	** Record the oldest I/O in this period.  Adjust I/Os that have been
           	** already marked hung by their hang count for purposes of figuring
           	** the oldest I/O.  This will be used in calculating the length of
           	** time to sleep between table scans.
           	********************************************************************/
			if ( (temp_time = seg_table[i].thread_time + (seg_table[i].hang_count * hang_time)) < oldest_time
			    )
				oldest_time = temp_time;
		}
		if ( hung_switch ) {
			sprintf(msg1, "Hung I/O alert!  Detected %d I/O(s) hung.\nCurrent time: " "%d; hang criteria: %d secs, Hard hang threshold: %d\n"
			    "Process ID: 0x%x\n" "1st lba   Blocks    Kernel   Hang  Duration\n" "(Hex)     (Hex)     Thread    Cnt  (Secs)\n",
			    hung_switch, current_time, hang_time, threshold, pid);
			if ( threshold_exceeded ) {
				sprintf(msg, "** Threshold of %d secs on one or more I/Os exceeded!\n", threshold * hang_time);
				strcat(msg1, msg);
				err_level = HARD;
			} else
				err_level = SOFT;
			for ( i = 0; i < seg_table_index; i++ ) {
				sprintf(msg, "%#llx %llx %llx %d %d \n", seg_table[i].flba, seg_table[i].llba - seg_table[i].flba,
				    seg_table[i].tid, seg_table[i].hang_count, current_time - seg_table[i].thread_time);
				if ( strlen(msg1) + strlen(msg) < MSG1_LIMIT )
					strcat(msg1, msg);
				else {
					strcat(msg1, "(size limit reached)");
					break;
				}
			}
			if ( crash_on_hang == 'Y' ) {
			#ifdef __HTX_LINUX__
				do_trap_htx64(0xBEEFDEAD, seg_table, seg_table_index, pid, msg1, ps );
			#else
				trap( 0xBEEFDEAD, seg_table, seg_table_index, pid, msg1, ps );
			#endif
			}
			user_msg(ps, 950, err_level, msg1);
		}
		/* Release the segment table mutex. */
		if ( (err_level = pthread_mutex_unlock(&segment_mutex)) ) {
			sprintf(msg, "Mutex unlock failed run_timer(), rc %d (%s)", "Hung I/O monitor for disk exerciser terminating.",
			    err_level, strerror(err_level));
			user_msg(ps, 950, HARD, msg);
			return;
		}
		/* Sleep until time to check again... */
		temp_time = hang_time - (current_time - oldest_time);
		sleep(temp_time <= 0 ? hang_time : temp_time);
	}
	threads_created --;
}

int
generate_pattern( char * buf, unsigned int bufsize, char pattern_id, unsigned int line_length, struct random_seed_t *seed, struct htx_data * htx, struct ruleinfo * pr) {

    int rc = 0;
    if(pattern_id == '7') {
        rc = generate_pattern7(buf, bufsize, line_length, htx);
    } else if(pattern_id == '8') {
        rc = generate_pattern8(buf, bufsize, line_length, pr, htx);
    } else if (pattern_id == '9') {
		(void) nrand48_r(seed->xsubi, &(seed->drand_struct), (signed long *)&pr->rand_index);
		pr->rand_index = (pr->rand_index % pr->size_rand_buf);
        rc = generate_pattern9(buf, bufsize, line_length, htx, pr);
    } else if (pattern_id == 'A') {
		(void) nrand48_r(seed->xsubi, &(seed->drand_struct), (signed long *)&pr->rand_index);
        pr->rand_index = (pr->rand_index % pr->size_rand_buf);
        rc = generate_patternA(buf, bufsize, line_length, htx, pr);
    } else {
        return(-1);
    }
    return(rc);
}

int
generate_pattern7( char * buf, unsigned int bufsize, unsigned int line_length, struct htx_data * htx) {

    /*********************************************************
    * algorithm 7 choosen. This algorithm will generate the buffer depending
    * on the width of the scsi bus. Say the width is 16 bits so the data
    * transfer should be ffff 0000 ffff 0000 ... This will generate the
    * highest electrical stress on the scsi bus.
    * Pattern Generated .......
    * <----------- line_length -------------->
    * FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
    * 0000000000000000000000000000000000000000
    * FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
    * 0000000000000000000000000000000000000000
    *********************************************************/
    int i;
    unsigned int byte_left = bufsize;
    char * wbuf = buf;


    while(byte_left) {
        for(i = 0; ((i < line_length) && (byte_left)); i++) {
            *((char*)wbuf) = 0xff;
            wbuf = ((char *)wbuf) + 1;
            byte_left--;
        }
        for(i = 0; ((i < line_length) && (byte_left)); i++) {
            *((char*)wbuf) = 0x00;
            wbuf = ((char *)wbuf) + 1;
            byte_left--;
        }
    }
    return(0);
}


int
generate_pattern8( char * wbuf, unsigned int bufsize, unsigned int line_length, struct ruleinfo * pr, struct htx_data * htx ) {

    /*******************************************************
    * Even line marches 1 from left to right
    * Odd line marches 0 from right to left
    * Pattern Generated ........
    * <--line_length->
    * fffffffffffffffe
    * 4000000000000000
    * fffffffffffffffd
    * 2000000000000000
    * fffffffffffffffb
    *******************************************************/

    unsigned long long HEXFF = 0xFFFFffffFFFFffffULL;
    unsigned long long HEXZERO = 0x0000000000000000ULL;
    unsigned long long LEAD_ONE = 0x8000000000000000ULL;
    unsigned long long TRAIL_ZERO = 0x1 ;
    unsigned int num_lines, left_line, line;
    unsigned int dword_size, num_dwords;
    unsigned long long * pattern, * strt_pattern;
    unsigned long long pattern_8;

    /* We are going to write line_length data in one shot
     * make sure bufsize is multiple of line_length, if not
     * take care of line_left separately.
     */

    num_lines = bufsize / line_length;
    left_line = bufsize % line_length;
    /* Check how many dwords each line spans */
    dword_size = sizeof(unsigned long long);
    num_dwords = line_length / dword_size;
    if(line_length % dword_size) {
        sprintf(htx->msg_text, "generate_pattern8: Error : line_length = %d not multiple of dwords=%d \n", line_length, dword_size);
        hxfmsg(htx, 0, HTX_HE_SOFT_ERROR, htx->msg_text);
        return(-1);
    }
    strt_pattern = (unsigned long long *) malloc(dword_size * num_dwords);
    if(strt_pattern == (unsigned long long *)NULL) {
        sprintf(htx->msg_text, "unable to get strt_pattern, errno = %d \n", errno);
        hxfmsg(htx, 0, HTX_HE_SOFT_ERROR, htx->msg_text);
        return(-1);
    }
    if(num_lines) {
        for(line = 0; line < num_lines; line++ ) {
            if( (line % 2) ==0) { /* Even Lines */
                pattern = strt_pattern;
                memset(pattern, 0x00, dword_size * num_dwords);
                pattern_8 = (HEXZERO | pr->or_mask );
                pattern[pr->beging_dword] = pattern_8;
                pr->or_mask = pr->or_mask >> 1;
               	if(pr->or_mask == 0 ) {
                    /* One dwork finished, reset mask and move to next dword */
                    pr->or_mask = LEAD_ONE;
                    pr->beging_dword = (pr->beging_dword + 1) % num_dwords;
                }
            } else { /* Odd lines */
                pattern = strt_pattern;
                memset(pattern, 0xFF, dword_size * num_dwords);
                pattern_8 = (HEXFF & ~(pr->and_mask));
                pattern[pr->trailing_dword]= pattern_8;
                pr->and_mask = pr->and_mask << 1;
                if(pr->and_mask == 0) {
                    /* One dwork finished, reset mask and move to next dword */
                    pr->and_mask = TRAIL_ZERO;
                    if(pr->trailing_dword > 0) {
                        pr->trailing_dword --;
                    } else {
                        pr->trailing_dword = num_dwords - 1;
                    }
                }
            }
            memcpy(wbuf, pattern, dword_size * num_dwords);
            wbuf += (dword_size * num_dwords);
        }
    }
    if(left_line) {
        num_dwords = left_line / dword_size;
        /* This is the even line left */
		pattern = strt_pattern;
        memset(pattern, 0x00, dword_size * num_dwords);
        pattern_8 = (HEXZERO | pr->or_mask );
        pattern[(pr->beging_dword % num_dwords)] = pattern_8;
		pr->or_mask = pr->or_mask >> 1;
        if(pr->or_mask == 0 ) {
        	/* One dwork finished, reset mask and move to next dword */
            pr->or_mask = LEAD_ONE;
            pr->beging_dword = (pr->beging_dword + 1) % num_dwords;
        }
        memcpy(wbuf, pattern, dword_size * num_dwords);
        wbuf += (dword_size * num_dwords);
    }
	free(strt_pattern);
    return(0);
}

int
generate_pattern9( char * wbuf, unsigned int bufsize, unsigned int line_length, struct htx_data * htx, struct ruleinfo * pr) {

    /*******************************************************
     * Even line has high entropy random data
     * odd line marches 1 from right to left
     * Pattern Generated ........
     * <--line_length->
     * 3c3c3c3c3c3c3c3c
     * 0000000000000001
     * 55555555aa555a5a
     * 0000000000000002
     *****************************************************/
    unsigned int num_lines, left_line, line;
    unsigned int dword_size, num_dwords;
    unsigned long long * pattern, * strt_pattern;
    unsigned long long trail_zero = 0x1;
    unsigned long long pattern_8;


    /* We are going to write line_length data in one shot
     * make sure bufsize is multiple of line_length, if not
     * take care of line_left separately.
     */
    num_lines = bufsize / line_length;
    left_line = bufsize % line_length;
    /* Check how many dwords each line spans */
    dword_size = sizeof(unsigned long long);
    num_dwords = line_length / dword_size;
    if(line_length % dword_size) {
        sprintf(htx->msg_text, "generate_pattern8: Error : line_length = %d not multiple of dwords=%d \n", line_length, dword_size);
        hxfmsg(htx, 0, HTX_HE_SOFT_ERROR, htx->msg_text);
        return(-1);
    }
    strt_pattern = (unsigned long long *) malloc(dword_size * num_dwords);
    if(strt_pattern == (unsigned long long *)NULL) {
        sprintf(htx->msg_text, "unable to get strt_pattern, errno = %d \n", errno);
        hxfmsg(htx, 0, HTX_HE_SOFT_ERROR, htx->msg_text);
        return(-1);
    }
    if(num_lines) {
        for(line = 0; line < num_lines; line++) {
            if((line%2) ==0) { /* Even Lines */
				if((pr->rand_index + dword_size * num_dwords) < pr->size_rand_buf) {
					pattern =(unsigned long long *)(pr->rand_buf + pr->rand_index);
					pr->rand_index += dword_size * num_dwords;
				} else {
					pr->rand_index = 0;
					pattern =(unsigned long long *)(pr->rand_buf + pr->rand_index);
				}
            } else { /* Odd line */
           		pattern = strt_pattern;
           		memset(pattern, 0x00, dword_size * num_dwords);
                pattern_8 = pr->and_mask;
                pattern[pr->trailing_dword] = pattern_8;
                pr->and_mask = pr->and_mask << 1;
                if(pr->and_mask == 0) {
                    pr->and_mask = trail_zero ;
                    if(pr->trailing_dword > 0) {
                        pr->trailing_dword --;
                    } else {
                        pr->trailing_dword = num_dwords - 1;
                    }
                }
            }
            memcpy(wbuf, pattern, dword_size * num_dwords);
            wbuf += dword_size * num_dwords;
        }
    }
    if(left_line) {
        num_dwords = left_line / dword_size;
        /* This is the even line left */
        pattern = strt_pattern;
        memset(pattern, 0x00, dword_size * num_dwords);
       	if((pr->rand_index + dword_size * num_dwords) < pr->size_rand_buf) {
        	pattern =(unsigned long long *)(pr->rand_buf + pr->rand_index);
            pr->rand_index += dword_size * num_dwords;
        } else {
            pr->rand_index = 0;
            pattern =(unsigned long long *)(pr->rand_buf + pr->rand_index);
        }
		/*
        printf("left_line=%d, num_dwords=%d, pattern_8=%llx, trailing_dword =%d \n", left_line, num_dwords, pattern_8, pr->trailing_dword);
		*/
        memcpy(wbuf, pattern, dword_size * num_dwords);
        wbuf += (dword_size * num_dwords);
    }
	free(strt_pattern);
    return(0);
}

int
generate_patternA( char * wbuf, unsigned int bufsize, unsigned int line_length, struct htx_data * htx, struct ruleinfo * pr) {

    /*******************************************************
     * Even line has high entropy random data
     * Odd line marches 0 from right to left
     *****************************************************/
    unsigned long long HEXFF = 0xFFFFffffFFFFffffULL;
    unsigned int num_lines, left_line, line;
    unsigned int dword_size, num_dwords;
    unsigned long long * pattern, * strt_pattern;
    unsigned long long trail_zero = 0x1;
    unsigned long long pattern_8;

    /* We are going to write line_length data in one shot
     * make sure bufsize is multiple of line_length, if not
     * take care of line_left separately.
     */
    num_lines = bufsize / line_length;
    left_line = bufsize % line_length;
    /* Check how many dwords each line spans */
    dword_size = sizeof(unsigned long long);
    num_dwords = line_length / dword_size;
    if(line_length % dword_size) {
        sprintf(htx->msg_text, "generate_pattern8: Error : line_length = %d not multiple of dwords=%d \n", line_length, dword_size);
        hxfmsg(htx, 0, HTX_HE_SOFT_ERROR, htx->msg_text);
        return(-1);
    }
    strt_pattern = (unsigned long long *) malloc(dword_size * num_dwords);
    if(strt_pattern == (unsigned long long *)NULL) {
        sprintf(htx->msg_text, "unable to get strt_pattern, errno = %d \n", errno);
        hxfmsg(htx, 0, HTX_HE_SOFT_ERROR, htx->msg_text);
        return(-1);
    }
    if(num_lines) {
        for(line = 0; line < num_lines; line++) {
            pattern = strt_pattern;
            if((line%2) ==0) { /* Even Lines */
                if((pr->rand_index + dword_size * num_dwords) < pr->size_rand_buf) {
                    pattern =(unsigned long long *)(pr->rand_buf + pr->rand_index);
                    pr->rand_index += dword_size * num_dwords;
                } else {
                    pr->rand_index = 0;
                    pattern =(unsigned long long *)(pr->rand_buf + pr->rand_index);
                }
            } else {
                pattern_8 = (HEXFF & ~(pr->and_mask));
                memset(pattern, 0xFF, dword_size * num_dwords);
                pattern[pr->trailing_dword]= pattern_8;
                pr->and_mask = pr->and_mask << 1;
                if(pr->and_mask == 0) {
                    /* One dwork finished, reset mask and move to next dword */
                    pr->and_mask = trail_zero;
                    if(pr->trailing_dword > 0) {
                        pr->trailing_dword --;
                    } else {
                        pr->trailing_dword = num_dwords - 1;
                    }
                }
            }
            memcpy(wbuf, pattern, dword_size * num_dwords);
            wbuf += (dword_size * num_dwords);
        }
    }
    if(left_line) {
        num_dwords = left_line / dword_size;
        /* This is the even line left */
       	if((pr->rand_index + dword_size * num_dwords) < pr->size_rand_buf) {
        	pattern =(unsigned long long *)(pr->rand_buf + pr->rand_index);
            pr->rand_index += dword_size * num_dwords;
        } else {
            pr->rand_index = 0;
            pattern =(unsigned long long *)(pr->rand_buf + pr->rand_index);
        }
        memcpy(wbuf, pattern, dword_size * num_dwords);
        wbuf += (dword_size * num_dwords);
    }
	free(strt_pattern);
    return(0);
}



