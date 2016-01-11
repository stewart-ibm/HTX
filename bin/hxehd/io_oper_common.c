/* @(#)39       1.40  src/htx/usr/lpp/htx/bin/hxehd/io_oper.c, exer_hd, htx61V 2/27/12 06:35:18 */
/******************************************************************************

 * FUNCTION: Different subroutines used in the exercising of hard disks and
 *           read/write optical devices.
 *
 * CHANGE LOG: programmer  | date     | change
 *            -------------|----------|---------------------------------------
 *             D. Stauffer | 10/01/96 | Added code to be able to submit a
 *                         |          | command asif from the command line.
 *             D. Stauffer | 04/10/97 | Changed rule argument in do_cmd from
 *                         |          | a struct to a pointer to a struct.
 *             L. Record   | 04/18/97 | Modified interface of do_cmd to be
 *                         |          | more efficient.  Added trap_sys inline
 *                         |          | code to replace crash_sys.  Fixed error
 *                         |          | in read_cace, write_cache, read_mem, &
 *                         |          | write_mem to prevent race condition
 *             R. Gebhardt | 09/11/00 | Changed lba_fencepost to an extern.
 *                         |          | The definition is in dskcomm.c and it
 *                         |          | is also extern in hxehd.c.
 *             R. Gebhardt | 09/11/00 | Create threads in detached state as a
 *                         |          | workaround for a AIX 500 threads
 *                         |          | termination problem.
 *             sjayakum    | 10/08/01 | Modification for 64 bit compilation
 *             Piyush      | 24/05/11 | Clear rbuf nly if we are going to compare.
 *			   Piyush      | 23/10/12 | 64 Bit LBA number in hxehd.
 *			   Piyush      | 24/07/13 | Code Common AIX/Linux.
 ******************************************************************************/
#include "hxehd_common.h"
#include "hxehd_proto_common.h"


char                 msg[MSG_TEXT_SIZE];
ushort               misc_count = 0;
pthread_t            cache_threads[8];
int volatile         DMA_running;
pthread_mutex_t      cache_mutex;
extern pthread_attr_t thread_attrs_detached;
extern unsigned long long volatile  lba_fencepost;
extern int           crash_on_miscom;

#ifndef __HTX_LINUX__
extern int	     pass_thru, debug_passthru;
extern unsigned long long       scsi_id_parent, lun_id_parent;
extern struct    devinfo parent_info;
extern unsigned long long	     scsi_id, lun_id;
extern char	     disk_parent[20];
/* AIX specific function -Passthru funtion defintion */
void dump_iocmd(struct htx_data *ps, struct sc_passthru *s);
int write_verify_disk(struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long *blkno, char *wbuf);
int read_verify_disk(struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long *blkno, char *rbuf);
int verify_disk(struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long *blkno, char *rbuf, int fildes);
#else
extern char fsync_flag;
typedef loff_t  offset_t;
#endif

extern int volatile  threads_created;
extern int volatile  cache_cond;
extern char volatile backgrnd_thread;
extern unsigned int eeh_enabled;
extern unsigned int eeh_retries;

/**************************************************************************/
/* set file pointer                                                       */
/**************************************************************************/
int
set_addr(struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long *blkno)
{
	offset_t rcode;
	offset_t addr;


	addr = (offset_t)blkno[0] * (offset_t)pr->bytpsec;

	#ifdef __HTX_LINUX__
	rcode = lseek64(pr->fildes, addr, SEEK_SET);
	#else
	rcode = llseek(pr->fildes, addr, 0);
	#endif
	/* EEH retry */
	if ( (rcode == -1) && eeh_enabled )
	{
		unsigned int temp_retries = eeh_retries;

		while( (rcode == -1) && temp_retries )
		{
			sleep(5);
			/* retry */
			sprintf(msg, "EEH retry by hxehd exerciser, on llseek system call failure retry number =  %d \n",(eeh_retries - temp_retries - 1));
			user_msg(ps, errno, SOFT, msg);
			rcode = llseek(pr->fildes, addr, 0);
            temp_retries--;
		}

		if( rcode >= 0 )
		{
			sprintf(msg, "EEH retry succesfull by hxehd exerciser, for llseek call retry number  %d \n",(eeh_retries - temp_retries));
			user_msg(ps, 0, SOFT, msg);
		}

		if ( (rcode == -1) && !temp_retries )
		{
			sprintf(msg, "\n Exerciser failed on max retries for EEH, llseek call failed \n\n");
			user_msg(ps, errno, HARD, msg);
		}
	}

	if ( addr != rcode ) {
		sprintf(msg,"Error in setaddr: addr %llx rcode %llx \n",addr,rcode);
		user_msg(ps, 0, HARD, msg);
	}
	if ( rcode == -1 ) {
		sprintf(msg, "errno %d (%s) from llseek: addr = %llu (0x%llx)", errno, strerror(errno), addr, addr);
		user_msg(ps, 0, INFO, msg);
		return(rcode);
	} else
		return(0);
}

/**************************************************************************/
/* write to disk                                                          */
/**************************************************************************/
int
write_disk(struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long * blkno,
	char *wbuf)
{
	int  rc;
    char msg[12000];
#ifndef __HTX_LINUX__
	char parent[256], tmp[256];
	unsigned long long dev_scsi_id = scsi_id;
	int adapter;
	unsigned long long dev_lun_id = lun_id;
	/*
     * SUPPORT FOR PASS-THRU COMANDS STARTS. AS OF NOW,
     * ONLY SCIOLCMD INTERFACE IS SUPPORTED. THIS IS AN
     * ADAPTER IOCTL, SO THE IOCTL WILL BE ISSUED TO THE
     * PARENT DEVICE, THAT IS, ADAPTER. SEE D#388686 FOR
     * DETAILS.
     */
	if (pass_thru == 1){
		/* send the passthru command */
		if (debug_passthru == 1){
			sprintf(msg, "Attempting to send the passthru command-Write_Verify \n", rc);
			user_msg(ps, 0, INFO, msg);
		}

		if(( rc = write_verify_disk(ps, pr, loop, blkno, wbuf)) != 0 ) {
			return rc;
		} else {
			if (debug_passthru == 1){
				sprintf(msg, "Sent passthru command-Write_Verify successfully \n", rc);
				user_msg(ps, 0, INFO, msg);
			}
		} /* end else */
		return 0;
	}

	/*
     * SUPPORT FOR PASS-THRU COMANDS ENDS
     */
#endif
	rc = set_addr(ps, pr, loop, blkno);
	if ( rc != 0 )
		return(rc);
	rc = eeh_write(ps, pr->fildes, wbuf, pr->dlen);
	if ( rc == -1 ) {
		if ( strcmp(pr->oper, "BWRC") == 0 )
			backgrnd_thread = 'E';
		ps->bad_writes = ps->bad_writes + 1;
		prt_msg(ps, pr, loop, blkno, errno, HARD, "Write error - ");
		return(errno);
	} else if ( rc != pr->dlen ) {
		if ( strcmp(pr->oper, "BWRC") == 0 )
			backgrnd_thread = 'E';
		sprintf(msg, "Attempted bytes written %d (0x%x) not equal to actual bytes\n" "written %d (0x%x).  Rule_id: %s, max_blkno: %#llx\n"
		    "first_block: %#llx, tot_blks: %#llx, current blk: %#llx", pr->dlen, pr->dlen, rc, rc, pr->rule_id, pr->max_blkno,
		    pr->first_block, pr->tot_blks, blkno[0]);
		user_msg(ps, 950, HARD, msg);
		return(1);
	} else {
#ifdef __HTX_LINUX__
        if(fsync_flag=='Y') {
        	int ret=0;
            ret=fsync(pr->fildes);
        	if( ret == -1 ) {
            	if ( strcmp(pr->oper, "BWRC") == 0 )
                	backgrnd_thread = 'E';
			}
        	ps->bad_writes = ps->bad_writes + 1;
        	prt_msg(ps, pr, loop, blkno, errno, HARD, "Write error, fsync failed - ");
        	return(errno);
   		}
#endif
		ps->good_writes = ps->good_writes + 1;
		ps->bytes_writ = ps->bytes_writ + pr->dlen;
		return(0);
	}
}

/**************************************************************************/
/* read from disk                                                         */
/**************************************************************************/
int
read_disk(struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long *blkno,
				char *rbuf)
{
	int  rc;
	char msg[256];

	if((strchr(pr->oper, 'C') != NULL) || ((strchr(pr->oper, '[') != NULL) && pr->opertn.compare_enabled)) {
		/* Only do this if compare is enabled */
		clrbuf(rbuf, pr->dlen);
	}
#ifndef __HTX_LINUX__
	/*
     * SUPPORT FOR PASS-THRU COMMANDS STARTS
     */
	if (pass_thru == 1){

		if (debug_passthru == 1){
			sprintf(msg, "Device Name = %s  Parent = %s \n",ps->sdev_id, disk_parent);
			user_msg(ps, 0, INFO, msg);
			/* send the passthru command */
			if (debug_passthru == 1)
			sprintf(msg, "Attempting to send the passthru command Read-Verify \n", rc);
			user_msg(ps, 0, INFO, msg);
		}

		if( ( rc = read_verify_disk(ps, pr, loop, blkno, rbuf)) != 0 ) {
			return(rc);
		} else {
			if (debug_passthru == 1){
				sprintf(msg, "Sent passthru command-read_Verify successfully \n", rc);
				user_msg(ps, 0, INFO, msg);
			}
		} /* End else */

		return 0;
	}

	/*
     * SUPPORT FOR PASS-THRU COMANDS ENDS
     */
#endif

	rc = set_addr(ps, pr, loop, blkno);
	if ( rc != 0 )
		return(rc);
	rc = eeh_read(ps, pr->fildes, rbuf, pr->dlen);
	if ( rc == -1 ) {
		if ( strcmp(pr->oper, "BWRC") == 0 )
			backgrnd_thread = 'E';
		ps->bad_reads = ps->bad_reads + 1;
		prt_msg(ps, pr, loop, blkno, errno, HARD, "read error - ");
		return(errno);
	} else if ( rc != pr->dlen ) {
		if ( strcmp(pr->oper, "BWRC") == 0 )
			backgrnd_thread = 'E';
		sprintf(msg, "Attempted bytes read %d (0x%x) not equal to actual bytes\n" "read %d (0x%x).  Rule_id: %s, max_blkno: %#llx\n"
		    "first_block: %#llx, tot_blks: %#llx, current blk: %#llx", pr->dlen, pr->dlen, rc, rc, pr->rule_id, pr->max_blkno,
		    pr->first_block, pr->tot_blks, blkno[0]);
		user_msg(ps, 950, HARD, msg);
		return(1);
	} else {
		ps->good_reads = ps->good_reads + 1;
		ps->bytes_read = ps->bytes_read + pr->dlen;
		return(0);
	}
}

/****************************************************************************/
/* Process to issue the read command to the system.                         */
/****************************************************************************/
int
read_cache(struct cache_chk *ck)
{
	int rc;
	struct htx_data *local_ts;

	/* Keep the address of our htx_data structure in a variable on our stack so
     that when the parent thread terminates we still have a valid pointer to
     the chunk of memory we need to free.
    */
	local_ts = ck->ts;

	rc = eeh_read(local_ts, ck->filedes, ck->buff1, ck->dlen);
	DMA_running = 0;
	if ( rc == -1 )
		cache_cond = errno;
	else if ( rc != ck->dlen )
		cache_cond = 2;
	else
		cache_cond = 0;
	rc = pthread_mutex_lock(&cache_mutex);
	if ( rc ) {
		sprintf(msg, "Mutex lock failed in process READ_CACHE  rc = %d\n", rc);
		user_msg(local_ts, 950, HARD, msg);
		exit(0);
	}
	threads_created--;
	rc = pthread_mutex_unlock(&cache_mutex);
	if ( rc ) {
		sprintf(msg, "Mutex unlock failed in process READ_CACHE rc = %d\n", rc);
		user_msg(local_ts, 950, HARD, msg);
		exit(0);
	}
	free(local_ts);
}

/*****************************************************************************/
/* Process to write directly to memory.                                      */
/*****************************************************************************/
int
write_mem(struct c_thread *ctab)
{
	int  i, rc;
	struct htx_data *local_ts;
	char pattern;

	/* Keep the address of our htx_data structure in a variable on our stack so
     that when the parent thread terminates we still have a valid pointer to
     the chunk of memory we need to free.
    */
	local_ts = ctab->ts;

	if ( ctab->count == 0 )
		pattern = 0x11;
	else if ( ctab->count == 1 )
		pattern = 0x22;
	else if ( ctab->count == 2 )
		pattern = 0x33;
	else if ( ctab->count == 3 )
		pattern = 0x44;
	else if ( ctab->count == 4 )
		pattern = 0x66;
	else if ( ctab->count == 5 )
		pattern = 0x77;
	else
		pattern = 0x88;
	i = ctab->count;
	while ( DMA_running ) {
		ctab->buff[i] = pattern;
		i += 10;
		if ( i >= ctab->dlen )
			i = ctab->count;
	}
	rc = pthread_mutex_lock(&cache_mutex);
	if ( rc ) {
		sprintf(msg, "Mutex lock failed in process WRITE_MEM  rc = %d\n", rc);
		user_msg(local_ts, 950, HARD, msg);
		exit(0);
	}
	threads_created--;
	rc = pthread_mutex_unlock(&cache_mutex);
	if ( rc ) {
		sprintf(msg, "Mutex unlock failed in process WRITE_MEM  rc = %d\n", rc);
		user_msg(local_ts, 950, HARD, msg);
		exit(0);
	}
	free(local_ts);
}

/*****************************************************************************/
/* Process to read directly from memory.                                     */
/*****************************************************************************/
int
read_mem(struct c_thread *ctab)
{
	int  i, rc;
	struct htx_data *local_ts;
	char hold_pattern, msg[MSG_TEXT_SIZE];

	/* Keep the address of our htx_data structure in a variable on our stack so
     that when the parent thread terminates we still have a valid pointer to
     the chunk of memory we need to free.
    */
	local_ts = ctab->ts;

	i = ctab->count;
	while ( DMA_running ) {
		hold_pattern = ctab->buff[i];
		i += 10;
		if ( i >= ctab->dlen )
			i = ctab->count;
	}
	rc = pthread_mutex_lock(&cache_mutex);
	if ( rc ) {
		sprintf(msg, "Mutex lock failed in process READ_MEM  rc = %d\n", rc);
		user_msg(local_ts, 950, HARD, msg);
		exit(0);
	}
	threads_created--;
	rc = pthread_mutex_unlock(&cache_mutex);
	if ( rc ) {
		sprintf(msg, "Mutex unlock failed in process READ_MEM  rc = %d\n", rc);
		user_msg(local_ts, 950, HARD, msg);
		exit(0);
	}
	free(local_ts);
}

/*****************************************************************************/
/* New Function to Read Disk and Write to Memory at the same time. Checks    */
/* cache processing.                                                         */
/*****************************************************************************/
int
read_cache_disk(struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long *blkno,
char *rbuf, char *wbuf)
{
	int     t /* local thread arg index */, rc;
	char   msg[MSG_TEXT_SIZE];
	struct cache_chk ck;
	struct c_thread ctab[6];

	memset(rbuf, 0xbb, pr->dlen);
	cache_cond = 10;
	threads_created = 0;
	DMA_running = 1;
	t = 0;
	if ( strcmp(pr->oper, "CARW") == 0 ) {
		while ( t < pr->max_number_of_threads -1 ) {
			threads_created++;
			ctab[t].dlen = pr->dlen;
			ctab[t].buff = rbuf;
			ctab[t].count = t;
			ctab[t].ts = (struct htx_data *)malloc(sizeof(struct htx_data));
			memcpy(ctab[t].ts, ps, sizeof(struct htx_data));
			rc = pthread_create(&(cache_threads[t]), &thread_attrs_detached, (void *(*)(void *))write_mem, (void
			    *)(&ctab[t]));
			t++;
		}
	} else {
		while ( t < pr->max_number_of_threads -1 ) {
			threads_created++;
			ctab[t].dlen = pr->dlen;
			ctab[t].buff = rbuf;
			ctab[t].count = t;
			ctab[t].ts = (struct htx_data *)malloc(sizeof(struct htx_data));
			memcpy(ctab[t].ts, ps, sizeof(struct htx_data));
			rc = pthread_create(&(cache_threads[t]), &thread_attrs_detached, (void *(*)(void *))read_mem, (void
			    *)(&ctab[t]));
			t++;
		}
	}
	ck.filedes = pr->fildes;
	ck.dlen    = pr->dlen;
	ck.buff1   = rbuf;
	ck.buff2   = wbuf;
	ck.ts = (struct htx_data *)malloc(sizeof(struct htx_data));
	memcpy(ck.ts, ps, sizeof(struct htx_data));
	threads_created++;
	rc = set_addr(ps, pr, loop, blkno);
	if ( rc != 0 ) {
		/* Something bad happened in set_addr - signal the running threads and
    ** wait for them to stop, then return.  */
		DMA_running = 0;
		while (threads_created > 0) usleep(10000);
		return(rc);
	}
	rc = pthread_create(&(cache_threads[t]), &thread_attrs_detached, (void *(*)(void *))read_cache, (void *)(&ck));
	t++;

	while ( threads_created > 0 )
		usleep(10000);      /* wait till all threads are finished */
	if ( cache_cond == 0 ) {
		ps->good_reads = ps->good_reads + 1;
		ps->bytes_read = ps->bytes_read + pr->dlen;
		return(0);
	} else if ( cache_cond == 2) {
		sprintf(msg, "CACHE - attempted bytes read not equal actual bytes read");
		user_msg(ps, 950, HARD, msg);
		return(1);
	} else {
		ps->bad_reads = ps->bad_reads + 1;
		prt_msg(ps, pr, loop, blkno, cache_cond, HARD, "read error - ");
		return(cache_cond);
	}
}

/****************************************************************************/
/* Process to issue the write command to the system.                        */
/****************************************************************************/
int
write_cache(struct cache_chk *ck)
{
	int rc;
	struct htx_data *local_ts;
	char msg[MSG_TEXT_SIZE];

	/* Keep the address of our htx_data structure in a variable on our stack so
     that when the parent thread terminates we still have a valid pointer to
     the chunk of memory we need to free.
    */
	local_ts = ck->ts;

	rc = eeh_write(local_ts, ck->filedes, ck->buff2, ck->dlen);
	DMA_running = 0;
	if ( rc == -1 )
		cache_cond = errno;
	else if ( rc != ck->dlen )
		cache_cond = 2;
	else
		cache_cond = 0;
#ifdef __HTX_LINUX__
	int ret;
   	if(fsync_flag=='Y')
   		ret=fsync(ck->filedes);

    if( ret == -1 )
       	cache_cond = 50;                      /* to denote fsync failure */
#endif
	rc = pthread_mutex_lock(&cache_mutex);
	if ( rc ) {
		sprintf(msg, "Mutex lock failed in process WRITE_CACHE rc = %d\n", rc);
		user_msg(local_ts, 950, HARD, msg);
		exit(0);
	}
	threads_created--;
	rc = pthread_mutex_unlock(&cache_mutex);
	if ( rc ) {
		sprintf(msg, "Mutex unlock failed in process WRITE_CACHE rc = %d\n", rc);
		user_msg(local_ts, 950, HARD, msg);
		exit(0);
	}
	free(local_ts);
}

/****************************************************************************/
/* Process to write to disk and at the same time write from the CPU to      */
/* memory. To check cache processing.                                       */
/****************************************************************************/
int
write_cache_disk(struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long * blkno,
					char *wbuf, char *rbuf)
{
	int       i, t /* local threads arg index */, rc;
	char      msg[MSG_TEXT_SIZE];
	struct    cache_chk ck;
	struct    c_thread ctab[6];
	pthread_t cache_threads[8];

	memcpy(rbuf, wbuf, pr->dlen);
	t = 0;
	threads_created = 0;
	cache_cond = 10;
	DMA_running = 1;
	if ( strcmp(pr->oper, "CAWW") == 0 ) {
		while ( t < pr->max_number_of_threads - 1) {
			threads_created++;
			ctab[t].dlen = pr->dlen;
			ctab[t].buff = wbuf;
			ctab[t].count = t;
			ctab[t].ts = (struct htx_data *)malloc(sizeof(struct htx_data));
			memcpy(ctab[t].ts, ps, sizeof(struct htx_data));
			rc = pthread_create(&(cache_threads[t]), &thread_attrs_detached, (void *(*)(void *))write_mem, (void
			    *)(&ctab[t]));
			t++;
		}
	} else {
		while ( t < pr->max_number_of_threads - 1) {
			threads_created++;
			ctab[t].dlen = pr->dlen;
			ctab[t].buff = wbuf;
			ctab[t].count = t;
			ctab[t].ts = (struct htx_data *)malloc(sizeof(struct htx_data));
			memcpy(ctab[t].ts, ps, sizeof(struct htx_data));
			rc = pthread_create(&(cache_threads[t]), &thread_attrs_detached, (void *(*)(void *))read_mem, (void
			    *)(&ctab[t]));
			t++;
		}
	}
	rc = set_addr(ps, pr, loop, blkno);
	if ( rc != 0 ) {
		/* Something bad happened in set_addr - signal the running threads and
    ** wait for them to stop, then return.  */
		DMA_running = 0;
		while (threads_created > 0) usleep(10000);
		return(rc);
	}
	ck.filedes = pr->fildes;
	ck.dlen    = pr->dlen;
	ck.buff1   = rbuf;
	ck.buff2   = wbuf;
	ck.ts = (struct htx_data *)malloc(sizeof(struct htx_data));
	memcpy(ck.ts, ps, sizeof(struct htx_data));
	threads_created++;
	rc = pthread_create(&(cache_threads[t]), &thread_attrs_detached, (void *(*)(void *))write_cache, (void *)(&ck));
	t++;

	while ( threads_created > 0 )
		usleep(10000);      /* wait till all threads are finished */

	if ( cache_cond == 0 ) {
		for ( i = 0; i <= pr->dlen; i++ )
			wbuf[i] = rbuf[i];
		clrbuf(rbuf,pr->dlen);
		rc = set_addr(ps, pr, loop, blkno);
		if ( rc != 0 )
			return(rc);
		rc = eeh_read(ps, pr->fildes, rbuf, pr->dlen);
		if ( rc == -1 ) {
			ps->bad_reads = ps->bad_reads + 1;
			prt_msg(ps, pr, loop, blkno, errno, HARD, "CACHE read error - ");
			return(errno);
		} else if ( rc != pr->dlen ) {
			sprintf(msg, "Attempted bytes read %d not equal to actual bytes read %d\n", pr->dlen,pr->dlen);
			user_msg(ps, 950, HARD, msg);
			return(1);
		} else {
			ps->good_writes = ps->good_writes + 1;
			ps->bytes_writ = ps->bytes_writ + pr->dlen;
			return(0);
		}
	} else if ( cache_cond == 2 ) {
		sprintf(msg, "Attempted bytes written did not equal actual bytes written\n");
		user_msg(ps, 950, HARD, msg);
		return(1);
#ifdef __HTX_LINUX__
    } else if ( cache_cond == 50 ) {
        sprintf(msg, "\nfsync failed...\n");
        user_msg(ps, 950, HARD, msg);
        return(1);
#endif
	} else {
		ps->bad_writes = ps->bad_writes + 1;
		prt_msg(ps, pr, loop, blkno, cache_cond, HARD, "CACHE write error - ");
		return(cache_cond);
	}
}

/******************************************************************************
** Process to compare the buffers and make sure that what was done is what   **
** was expected.                                                             **
*******************************************************************************
** Notes on this routine:                                                    **
** The normal buffer comparison routine checks the header of each block of   **
** data to see if it's a pattern 3, 4, or 5 header.  In this routine we      **
** don't do that because we are always comparing the disk blocks against     **
** themselves, whether we just put them there or they were there already.    **
** The only thing we are concerned about is the appearance of the thread     **
** byte in the read buffer, and it's behavior is well defined.               **
******************************************************************************/
int
compare_cache(struct htx_data *ps, struct ruleinfo *pr, int loop,
				char rbuf[], char wbuf[], unsigned long long *blkno)
#define CMP_PATH "/tmp/"
#define MAX_MISC 10
#define MAX_MSG 20
{
	int  i, j, cmp_cnt, save_reread;
	char s[3], path[128], cmp_str[512], msg[MAX_TEXT_MSG];
	char *reread, * save, stanza_save[9];

    unsigned blk_shift = (pr->align * pr->bytpsec) -1;
	reread = (char *) 0;	     /* This buffer will hold the re-read block      */
	save_reread = 0;
	cache_cond = 0;
	for ( i = 0; i < pr->dlen; i++ ) {
        if ( (rbuf[i] == wbuf[i])                 ||
            ((rbuf[i] == 0x11) && (i % 10 == 0)) ||
            ((rbuf[i] == 0x22) && (i % 10 == 1)) ||
            ((rbuf[i] == 0x33) && (i % 10 == 2)) ||
            ((rbuf[i] == 0x44) && (i % 10 == 3)) ||
			/* Note 0x55 skipped because it occurs frequently naturally. */
            ((rbuf[i] == 0x66) && (i % 10 == 4)) ||
            ((rbuf[i] == 0x77) && (i % 10 == 5)) ||
            ((rbuf[i] == 0x88) && (i % 10 == 6)) );
		else {
			cmp_cnt = i;
			cache_cond = 1;
			if ( crash_on_miscom ) {
			#ifdef __HTX_LINUX__
                do_trap_htx64( 0xBEEFDEAD, wbuf, rbuf, i, ps, pr );
			#else
				trap(0xBEEFDEAD, wbuf, rbuf, i, ps, pr );
			#endif
			}
			i = pr->dlen;
		}
	}
	if ( cache_cond == 0 )
		return(0);
	else {
		if ( lba_fencepost < pr->max_blkno )
			sprintf(msg, "\nMiscompare in oper %s at displacement %d (0x%x)\n" "Block Number = %lld (%#llx), LBA Fencepost = %lld (%llx)",
			    pr->oper, cmp_cnt, cmp_cnt, blkno[0], blkno[0], lba_fencepost, lba_fencepost);
		else
			sprintf(msg, "\nMiscompare in oper %s at displacement %d (0x%x)\n" "Block Number = %lld (%llx), Maximum LBA = %lld (%llx)",
			    pr->oper, cmp_cnt, cmp_cnt, blkno[0], blkno[0], pr->max_blkno, pr->max_blkno);
		sprintf(cmp_str,"\nCompare had to match 0x11, 0x22, 0x33, 0x44," " 0x66, 0x77, 0x88,\nor the pattern that was  "
		    "read from the disk.");
		strcat(msg, cmp_str);
		sprintf(cmp_str, "\n<>");
		strcat(msg, cmp_str);
		sprintf(cmp_str, "\nwbuf(hex %x)", wbuf);
		strcat(msg, cmp_str);
		for ( j = cmp_cnt; ((j - cmp_cnt) < MAX_MSG) && (j < pr->dlen); j++ ) {
			sprintf(s, "%0.2x", wbuf[j]);
			strcat(msg, s);
		}
		sprintf(cmp_str, "\nrbuf(hex %x)", rbuf);
		strcat(msg, cmp_str);
		for ( j = cmp_cnt; ((j - cmp_cnt) < MAX_MSG) && (j < pr->dlen); j++ ) {
			sprintf(s, "%0.2x", rbuf[j]);
			strcat(msg, s);
		}
		strcat(msg, "\n");
		/*********************************************************
 	 	 *                                                       *
 	 	 * Block align the re-read buffer here. Needed in Linux  *
 	 	 *                                                       *
 	 	 *********************************************************/

        if(pr->align > 0)
        {
            reread = (char *) malloc(pr->dlen + blk_shift + 128);
            save = reread;
            if(reread == NULL)
            {
                sprintf(cmp_str,"** Can't malloc re-read buf: "
                    "errno %d (%s) - re-read not done!\n",
                    errno,strerror(errno));
                strcat(msg, cmp_str);
            }
            else
                reread = (char *) ((((blk_shift + (unsigned) reread) & ~blk_shift)) + pr->offset);

        }
        else
        {
            reread = (char *) malloc(pr->dlen + 128);
            save = reread;
            if(reread == NULL)
            {
                sprintf(cmp_str,"** Can't malloc re-read buf: "
                    "errno %d (%s) - re-read not done!\n",
                    errno,strerror(errno));
                strcat(msg, cmp_str);
            }
            else
                reread = reread + pr->offset;
        }

        if (reread == NULL )
        {
            sprintf(cmp_str, "**> Can't malloc re-read buf: "
                "errno %d (%s) - re-read not done!\n",
                errno, strerror(errno));
            strcat(msg, cmp_str);
        }
        else
        {
			/* Save the original stanza name and replace it with "!Re-Read"
              * so that if read_disk gets an error and HTX halts it before
              * it returns control to us, it's obvious that the error occurred
              * during a re-read operation.  After read_disk returns, we'll
              * copy the original stanza name back.
              */
			strcpy(stanza_save, pr->rule_id);
			strcpy(pr->rule_id, "#Re-Read");
			cache_cond = read_disk(ps, pr, loop, blkno, reread);
			strcpy(pr->rule_id, stanza_save);
			if ( cache_cond ) {
				sprintf(cmp_str, "CACHE > Error trying to re-read disk buffer!\n");
				strcat(msg, cmp_str);
				free(reread);
				reread = (char *) 0;
			} else {
				cache_cond = 0;
				for ( i = 0; i < pr->dlen; i++ ) {
                    if ( (reread+i == (char *)wbuf[i])                 ||
                        ((reread+i == ( char *)0x11) && (i % 10 == 0)) ||
                        ((reread+i == ( char *)0x22) && (i % 10 == 1)) ||
                        ((reread+i == ( char *)0x33) && (i % 10 == 2)) ||
                        ((reread+i == ( char *)0x44) && (i % 10 == 3)) ||
                        ((reread+i == ( char *)0x66) && (i % 10 == 4)) ||
                        ((reread+i == ( char *)0x77) && (i % 10 == 5)) ||
                        ((reread+i == ( char *)0x88) && (i % 10 == 6)) );
					else {
						cache_cond = 1;
						i = pr->dlen;
					}
				}
				save_reread = cache_cond;
			}
		}
		misc_count++;
		if ( misc_count < MAX_MISC ) {
			strcpy(path, CMP_PATH);           /* set up path for the read buffer */
			strcat(path, "htx");
			strcat(path, &(ps->sdev_id[5]));
			strcat(path, ".crbuf");
			sprintf(cmp_str, "%-d", misc_count);
			strcat(path, cmp_str);
			hxfsbuf(rbuf,pr->dlen,path,ps);
			strcpy(path, CMP_PATH);          /* set up path for the write buffer */
			strcat(path, "htx");
			strcat(path, &(ps->sdev_id[5]));
			strcat(path, ".cwbuf");
			sprintf(cmp_str, "%-d", misc_count);
			strcat(path, cmp_str);
			hxfsbuf(wbuf,pr->dlen,path,ps);
			if ( reread != (char *) 0 ) {
				if ( save_reread ) {
					strcpy(path, CMP_PATH);
					strcat(path, "htx");
					strcat(path, &(ps->sdev_id[5]));
					strcat(path, ".crerd");
					sprintf(cmp_str, "%-d", misc_count);
					strcat(path, cmp_str);
					hxfsbuf(reread, pr->dlen, path, ps);
					strcat(msg, "Re-read fails to compare; ");
					sprintf(cmp_str, "buffer saved in %s\n", path);
					strcat(msg, cmp_str);
				} else
					strcat(msg, "Re-read compares OK; buffer not saved.\n");
				free(reread);
			}
			user_msg(ps, 950, HARD, msg);
		} else {
			sprintf(cmp_str, "The maximum number of saved miscompare buffers " "in the CACHE rules (%d) have already\nbeen saved."
			    " The read (CRBUF) and write (CWBUF) buffers for this" "\nmiscompare will not be saved to the disk.\n",
			    MAX_MISC);
			strcat(msg, cmp_str);
			user_msg(ps, 950, HARD, msg);
			if ( reread != (char *) 0 )
				free(reread);
		}
		return(1);
	}
}

#ifndef __HTX_LINUX__   /* AIX only !! Functions */

#include "oscjb.h"

/*****************************************************************************/
/* Get information on the optical changer device - PIRANAHA                  */
/*****************************************************************************/
ominfo(struct htx_data s, struct ruleinfo * r)
{
	int        rc = 0;
	char       msg[150], msg1[150];
	struct     htx_data *ps;
	struct     ruleinfo *pr;
	jb_DevInfo dev_info;

	ps = &s;
	pr = r;
	pr->fildes = openx(pr->dev_name, O_RDWR, NULL, 0);
	if ( pr->fildes < 1 ) {
		sprintf(msg, "ID = %s, Open error in changer process OMINFO - ", pr->rule_id);
		user_msg(ps, errno, HARD, msg);
		return(1);
	}
	rc = ioctl(pr->fildes, IOCINFO, &dev_info);
	if ( rc ) {
		sprintf(msg, "ID = %s, Unable to retreive optical changer info - ", pr->rule_id);
		user_msg(ps, errno, HARD, msg);
		return(1);
	} else {
		sprintf(msg, "\nOptical Medium Changer %s Information:\n", pr->dev_name);
		strcpy(msg1, msg);
		sprintf(msg, "   Device Type    > %c\n", dev_info.devtype);
		strcat(msg1, msg);
		sprintf(msg, "   Flags          > %x\n", dev_info.flags);
		strcat(msg1, msg);
		sprintf(msg, "   Device SubType > %x\n", dev_info.devsubtype);
		strcat(msg1, msg);
		user_msg(ps, 0, INFO, msg);
		while(close(pr->fildes) == -1);
		return(rc);
	}
}

/*****************************************************************************/
/* Get inquiry information on the optical changer device                     */
/*****************************************************************************/
ominqry(struct htx_data s, struct ruleinfo *r)
{
	int      rc = 0;
	char     msg[200], msg1[200];
	struct   htx_data *ps;
	struct   ruleinfo *pr;
	jb_IOCTL jb_ioctl;

	ps = &s;
	pr = r;
	pr->fildes = openx(pr->dev_name, O_RDWR, NULL, 0);
	if ( pr->fildes < 1 ) {
		sprintf(msg, "ID = %s, Open error in changer process OMINQRY - ", pr->rule_id);
		user_msg(ps, errno, HARD, msg);
		return(1);
	}
	rc = ioctl(pr->fildes, JBIOC_INQUIRY, &jb_ioctl);
	if ( rc ) {
		sprintf(msg, "ID = %s, Unable to get optical changer inquiry data - ", pr->rule_id);
		user_msg(ps, errno, HARD, msg);
		return(1);
	} else {
		sprintf(msg, "\nOptical Medium Changer %s Inquiry Information:\n", pr->dev_name);
		strcpy(msg1, msg);
		sprintf(msg, "   Device Type    > %x\n", jb_ioctl.Cmd.Inquiry.DeviceType);
		strcat(msg1, msg);
		sprintf(msg, "   Removeable     > %d\n", jb_ioctl.Cmd.Inquiry.Removeable);
		strcat(msg1, msg);
		sprintf(msg, "   Flipable       > %x\n", jb_ioctl.Cmd.Inquiry.Flipable);
		strcat(msg1, msg);
		sprintf(msg, "   Product Type   > %s\n", jb_ioctl.Cmd.Inquiry.ProductType);
		strcat(msg1, msg);
		sprintf(msg, "   Product Model  > %s\n", jb_ioctl.Cmd.Inquiry.ProductModel);
		strcat(msg1, msg);
		user_msg(ps, 0, INFO, msg1);
		while(close(pr->fildes) == -1);
		return(rc);
	}
}

/*****************************************************************************/
/* To allow/not allow the media to be removed                                */
/*****************************************************************************/
omprevent(struct htx_data s, struct ruleinfo *r)
{
	int      rc = 0;
	char     msg[100];
	struct   htx_data *ps;
	struct   ruleinfo *pr;
	jb_IOCTL jb_ioctl;

	ps = &s;
	pr = r;
	pr->fildes = openx(pr->dev_name, O_RDWR, NULL, 0);
	if ( pr->fildes < 1 ) {
		sprintf(msg, "ID = %s, Open error in changer process OMPREVENT - ", pr->rule_id);
		user_msg(ps, errno, HARD, msg);
		return(1);
	}
	jb_ioctl.Cmd.PreventAllow.Prevent = pr->om_prevent;
	rc = ioctl(pr->fildes, JBIOC_PREVENTALLOW, &jb_ioctl);
	if ( rc ) {
		sprintf(msg, "ID = %s, Command to allow/prevent media failed - ", pr->rule_id);
		user_msg(ps, errno, HARD, msg);
		return(1);
	} else {
		while(close(pr->fildes) == -1);
		return(rc);
	}
}

/*****************************************************************************/
/* To be used to initialize the media changer                                */
/*****************************************************************************/
ominit(struct htx_data s, struct ruleinfo * r)
{
	int      rc = 0;
	char     msg[100];
	struct   htx_data *ps;
	struct   ruleinfo *pr;
	jb_IOCTL jb_ioctl;

	ps = &s;
	pr = r;
	pr->fildes = openx(pr->dev_name, O_RDWR, NULL, 0);
	if ( pr->fildes < 1 ) {
		sprintf(msg, "ID = %s, Open error in changer process OMINIT - ", pr->rule_id);
		user_msg(ps, errno, HARD, msg);
		return(1);
	}
	rc = ioctl(pr->fildes, JBIOC_IES, &jb_ioctl);
	if ( rc ) {
		sprintf(msg, "ID = %s, Command to initialize optical media failed - ", pr->rule_id);
		user_msg(ps, errno, HARD, msg);
		return(1);
	} else {
		while(close(pr->fildes) == -1);
		return(rc);
	}
}

/*****************************************************************************/
/* To be used to move a media from one slot to another slot                  */
/*****************************************************************************/
ommove(struct htx_data s, struct ruleinfo * r)
{
	int      rc = 0;
	char     msg[100];
	struct   htx_data *ps;
	struct   ruleinfo *pr;
	jb_IOCTL jb_ioctl;

	ps = &s;
	pr = r;

	pr->fildes = openx(pr->dev_name, O_RDWR, NULL, 0);
	if ( pr->fildes < 1 ) {
		sprintf(msg, "ID = %s, Open error in changer process OMMOVE - ", pr->rule_id);
		user_msg(ps, errno, HARD, msg);
		return(1);
	}
	jb_ioctl.Cmd.Move.TransportElement   = 0;
	jb_ioctl.Cmd.Move.SourceElement      = pr->om_source;
	jb_ioctl.Cmd.Move.DestinationElement = pr->om_target;
	jb_ioctl.Cmd.Move.Invert             = pr->om_invert;
	rc = ioctl(pr->fildes, JBIOC_MOVE, &jb_ioctl);
	if ( rc ) {
		sprintf(msg, "ID = %s, Command to move optical media failed - ", pr->rule_id);
		user_msg(ps, errno, HARD, msg);
		return(1);
	} else {
		while(close(pr->fildes) == -1);
		return(rc);
	}
}

/*****************************************************************************/
/* To be used to exchange media from one slot to another slot                */
/*****************************************************************************/
omexchge(struct htx_data s, struct ruleinfo * r)
{
	int      rc = 0;
	char     msg[100];
	struct   htx_data *ps;
	struct   ruleinfo *pr;
	jb_IOCTL jb_ioctl;

	ps = &s;
	pr = r;
	pr->fildes = openx(pr->dev_name, O_RDWR, NULL, 0);
	if ( pr->fildes < 1 ) {
		sprintf(msg, "ID = %s, Open error in changer process OMEXCHGE - ", pr->rule_id);
		user_msg(ps, errno, HARD, msg);
		return(1);
	}
	jb_ioctl.Cmd.Exchange.TransportElement    = 0;
	jb_ioctl.Cmd.Exchange.SourceElement       = pr->om_source;
	jb_ioctl.Cmd.Exchange.DestinationElement1 = pr->om_target;
	jb_ioctl.Cmd.Exchange.DestinationElement2 = pr->om_target1;
	jb_ioctl.Cmd.Exchange.Invert1             = pr->om_invert;
	jb_ioctl.Cmd.Exchange.Invert2             = pr->om_invert1;
	rc = ioctl(pr->fildes, JBIOC_EXCHANGE, &jb_ioctl);
	if ( rc ) {
		sprintf(msg, "ID = %s, Command to exchange optical media failed - ", pr->rule_id);
		user_msg(ps, errno, HARD, msg);
		return(1);
	} else {
		while(close(pr->fildes) == -1);
		return(rc);
	}
}

/*****************************************************************************/
/* To be used to make an inventory of what is on the optical machine         */
/*****************************************************************************/
ominvtry(struct htx_data s, struct ruleinfo * r)
{
	int      i, rc = 0;
	char     msg[150], msg1[2500];
	struct   htx_data *ps;
	struct   ruleinfo *pr;
	jb_IOCTL jb_ioctl;
	jb_TransportElemStat    * Transports = NULL;
	jb_StorageElemStat      * Slots      = NULL;
	jb_ImportExportElemStat * ImpExps    = NULL;
	jb_DataTransferElemStat * Datas      = NULL;

	ps = &s;
	pr = r;
	pr->fildes = openx(pr->dev_name, O_RDWR, NULL, 0);
	if ( pr->fildes < 1 ) {
		sprintf(msg, "ID = %s, Open error in changer process OMINVTRY - ", pr->rule_id);
		user_msg(ps, errno, HARD, msg);
		return(1);
	}
	memset(&jb_ioctl, 0, sizeof(jb_ioctl));
	rc = ioctl(pr->fildes, JBIOC_INVENTORY, &jb_ioctl);
	if ( rc ) {
		sprintf(msg, "ID = %s, Command to get optical media inventory failed - ", pr->rule_id);
		user_msg(ps, errno, HARD, msg);
		return(1);
	} else {
		sprintf(msg, "\nOptical Medium Changer %s Inventory Information:\n", pr->dev_name);
		strcpy(msg1, msg);
		sprintf(msg, "   1st Transport  > %d\n", jb_ioctl.Cmd.Inventory.FirstTransportElem);
		strcat(msg1, msg);
		sprintf(msg, "   # of Transport > %d\n", jb_ioctl.Cmd.Inventory.TransportElements);
		strcat(msg1, msg);
		sprintf(msg, "   1st Storage    > %d\n", jb_ioctl.Cmd.Inventory.FirstStorageElem);
		strcat(msg1, msg);
		sprintf(msg, "   # of Storage   > %d\n", jb_ioctl.Cmd.Inventory.StorageElements);
		strcat(msg1, msg);
		sprintf(msg, "   1st I/O        > %d\n", jb_ioctl.Cmd.Inventory.FirstImportExportElem);
		strcat(msg1, msg);
		sprintf(msg, "   # of I/Os      > %d\n", jb_ioctl.Cmd.Inventory.ImportExportElements);
		strcat(msg1, msg);
		sprintf(msg, "   1st Drive      > %d\n", jb_ioctl.Cmd.Inventory.FirstDataTransferElem);
		strcat(msg1, msg);
		sprintf(msg, "   # of Drives    > %d\n", jb_ioctl.Cmd.Inventory.DataTransferElements);
		strcat(msg1, msg);
		user_msg(ps, 0, INFO, msg1);
		jb_ioctl.Cmd.Inventory.TransportElemStat = malloc( jb_ioctl.Cmd.Inventory.TransportElements * sizeof(jb_TransportElemStat) );
		if ( jb_ioctl.Cmd.Inventory.TransportElemStat == NULL ) {
			sprintf(msg, "ERROR: Malloc of Transport buffer FAILED!\n");
			user_msg(ps, 950, HARD, msg);
		}
		memset(jb_ioctl.Cmd.Inventory.TransportElemStat, 0, jb_ioctl.Cmd.Inventory.TransportElements * sizeof(jb_TransportElemStat));
		jb_ioctl.Cmd.Inventory.DataTransferElemStat = malloc( jb_ioctl.Cmd.Inventory.DataTransferElements * sizeof(jb_DataTransferElemStat) );
		if ( jb_ioctl.Cmd.Inventory.DataTransferElemStat == NULL ) {
			sprintf(msg, "ERROR: Malloc of DataTransfer buffer FAILED!\n");
			user_msg(ps, 950, HARD, msg);
		}
		memset(jb_ioctl.Cmd.Inventory.DataTransferElemStat, 0, jb_ioctl.Cmd.Inventory.DataTransferElements * sizeof(jb_DataTransferElemStat));
		jb_ioctl.Cmd.Inventory.ImportExportElemStat = malloc( jb_ioctl.Cmd.Inventory.ImportExportElements * sizeof(jb_ImportExportElemStat) );
		if ( jb_ioctl.Cmd.Inventory.ImportExportElemStat == NULL ) {
			sprintf(msg, "ERROR: Malloc of ImportExport buffer FAILED!\n");
			user_msg(ps, 950, HARD, msg);
		}
		memset(jb_ioctl.Cmd.Inventory.ImportExportElemStat, 0, jb_ioctl.Cmd.Inventory.ImportExportElements * sizeof(jb_ImportExportElemStat));
		jb_ioctl.Cmd.Inventory.StorageElemStat = malloc( jb_ioctl.Cmd.Inventory.StorageElements * sizeof(jb_StorageElemStat) );
		if ( jb_ioctl.Cmd.Inventory.StorageElemStat == NULL ) {
			sprintf(msg, "ERROR: Malloc of Storage buffer FAILED!\n");
			user_msg(ps, 950, HARD, msg);
		}
		memset(jb_ioctl.Cmd.Inventory.StorageElemStat, 0, jb_ioctl.Cmd.Inventory.StorageElements * sizeof(jb_StorageElemStat));
		rc = ioctl(pr->fildes, JBIOC_INVENTORY, &jb_ioctl);
		if ( rc ) {
			strcpy(msg, "Command to get inventory failed - ");
			if ( errno > 0 && errno <= sys_nerr )
				strcat(msg, sys_errlist[errno]);
			strcat(msg, "\n");
			free(jb_ioctl.Cmd.Inventory.TransportElemStat);
			free(jb_ioctl.Cmd.Inventory.StorageElemStat);
			free(jb_ioctl.Cmd.Inventory.ImportExportElemStat);
			free(jb_ioctl.Cmd.Inventory.DataTransferElemStat);
			user_msg(ps, 950, HARD, msg);
			return(rc);
		} else {
			strcpy(msg1, "  ");
			Transports = jb_ioctl.Cmd.Inventory.TransportElemStat;
			sprintf(msg, "\nTransport Element Information:\n");
			strcat(msg1, msg);
			sprintf(msg, "------------------------------\n");
			strcat(msg1, msg);
			for ( i = 0; i < jb_ioctl.Cmd.Inventory.TransportElements; i++ ) {
				sprintf(msg, "  Element Address: %3d,  State: %8s,  Status: %5s\n", Transports->ElementAddress,
				    (Transports->Exception ? "ABNORMAL" : "NORMAL"), (Transports->Full ? "FULL" : "EMPTY") );
				strcat(msg1, msg);
				Transports++;
			}
			user_msg(ps, 0, INFO, msg1);
			strcpy(msg1, "  ");
			Datas = jb_ioctl.Cmd.Inventory.DataTransferElemStat;
			sprintf(msg, "\nData Transfer Element Information:\n");
			strcat(msg1, msg);
			sprintf(msg, "----------------------------------\n");
			strcat(msg1, msg);
			for ( i = 0; i < jb_ioctl.Cmd.Inventory.DataTransferElements; i++ ) {
				sprintf(msg, "  Element Address: %3d,  State: %8s, ", Datas->ElementAddress, (Datas->Exception
				    ? "ABNORMAL" : "NORMAL") );
				strcat(msg1, msg);
				sprintf(msg, " Accessible: %3s,  Status: %5s\n", (Datas->Access ? "YES" : "NO"), (Datas->
				    Full ? "FULL" : "EMPTY") );
				strcat(msg1, msg);
				Datas++;
			}
			user_msg(ps, 0, INFO, msg1);
			strcpy(msg1, "  ");
			ImpExps = jb_ioctl.Cmd.Inventory.ImportExportElemStat;
			sprintf(msg, "\nImport / Export Element Information:\n");
			strcat(msg1, msg);
			sprintf(msg, "------------------------------------\n");
			strcat(msg1, msg);
			for ( i = 0; i < jb_ioctl.Cmd.Inventory.ImportExportElements; i++ ) {
				sprintf(msg, "  Element Address: %3d,  State: %8s, ", ImpExps->ElementAddress, (ImpExps->
				    Exception ? "ABNORMAL" : "NORMAL") );
				strcat(msg1, msg);
				sprintf(msg, " Accessible: %3s,  Import: %8s, ", (ImpExps->Access ? "YES" : "NO"), (ImpExps->
				    ImportEnabled ? "ENABLED" : "DISABLED") );
				strcat(msg1, msg);
				sprintf(msg, " Export: %8s,  Status: %5s\n", (ImpExps->ExportEnabled ? "ENABLED" : "DISABLED"),
				    (ImpExps->Full ? "FULL" : "EMPTY") );
				strcat(msg1, msg);
				ImpExps++;
			}
			user_msg(ps, 0, INFO, msg1);
			strcpy(msg1, "  ");
			Slots = jb_ioctl.Cmd.Inventory.StorageElemStat;
			sprintf(msg, "\nStorage Element Information:\n");
			strcat(msg1, msg);
			sprintf(msg, "------------------------------");
			strcat(msg1, msg);
			for ( i = 0; i < jb_ioctl.Cmd.Inventory.StorageElements; i++ ) {
				sprintf(msg, "\n  Element Address: %3d,  State: %8s, ", Slots->ElementAddress, (Slots->Exception
				    ? "ABNORMAL" : "NORMAL") );
				strcat(msg1, msg);
				sprintf(msg, " Accessible: %3s,  Status: %5s", (Slots->Access ? "YES" : "NO"), (Slots->Full
				    ? "FULL" : "EMPTY") );
				strcat(msg1, msg);
				Slots++;
				if ( i == 25 ) {
					user_msg(ps, 0, INFO, msg1);
					strcpy(msg1, " ");
				}
			}
			user_msg(ps, 0, INFO, msg1);
			free(jb_ioctl.Cmd.Inventory.TransportElemStat);
			free(jb_ioctl.Cmd.Inventory.StorageElemStat);
			free(jb_ioctl.Cmd.Inventory.ImportExportElemStat);
			free(jb_ioctl.Cmd.Inventory.DataTransferElemStat);
			while(close(pr->fildes) == -1);
			return(0);
		}
	}
}
/**************************************************************************/
/* write_verify ( using scsi passthru commands ) to disk                  */
/**************************************************************************/
int write_verify_disk(struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long *blkno,
									   char *wbuf)
{
	struct scsi_iocmd iocmd;
	/*extern int lun; */
	int  rc;
	char msg[256];

	if (debug_passthru == 1){
		sprintf( msg, " Inside write_verify_disk \n " );
		user_msg(ps, 0, INFO, msg);
	}

	/*initialise scsi iocmd structure */

	memset(&iocmd, 0, sizeof(struct scsi_iocmd));
	iocmd.version = (ushort)SCSI_VERSION_2;
	iocmd.timeout_value = 30; /* default to 30 secs */
	iocmd.status_validity = 0;
	iocmd.scsi_id = (unsigned long long)scsi_id; /* Device scsi_id */
	iocmd.lun_id = (unsigned long long)lun_id;   /* Device lun_id  */
	iocmd.flags |= B_WRITE;
	iocmd.data_length =  pr->dlen;
	iocmd.command_length = 10;
	/* Make the iocmd buffer point to the data we need to write */
	iocmd.buffer = wbuf;

	/* Initialise iocmd structure with the reqd data */
	iocmd.scsi_cdb[0] = 0x2e;        						/* set operation code */
	iocmd.scsi_cdb[1] = 0x02;  								/* BYTE CHECK */
	iocmd.scsi_cdb[2] = (uchar) (blkno[0] >> 24) & 0xff;    /* LBA MSB    */
	iocmd.scsi_cdb[3] = (uchar) (blkno[0] >> 16) & 0xff;    /* LBA MSB    */
	iocmd.scsi_cdb[4] = (uchar) (blkno[0] >> 8) & 0xff;     /* LBA        */
	iocmd.scsi_cdb[5] = (uchar) (blkno[0] & 0xff);          /* LBA LSB    */
	iocmd.scsi_cdb[7] = (pr->num_blks >> 8) & 0xff;  		/* transfer lgnth MSB */
	iocmd.scsi_cdb[8] = (pr->num_blks & 0xff);       		/* transfer lgnth LSB */
	iocmd.scsi_cdb[9] = 0;									/* Control 			*/

	/* Do the ioctl */
	if (debug_passthru == 1){
		sprintf( msg, " Before sending the CDB \n " );
		user_msg(ps, 0, INFO, msg);
	}


	rc = eeh_ioctl(ps, pr->adapter, SCIOLCMD, &iocmd);

	if (debug_passthru == 1){
		sprintf( msg, " After sending the CDB \n " );
		user_msg(ps, 0, INFO, msg);
	}

	/* check return code of cmd  */
	if ( rc != 0 ) {
		sprintf( msg, " einval_arg = %d, errno = %d \n ", iocmd.adap_set_flags, errno );
		user_msg(ps, 0, INFO, msg);

		if ( strcmp(pr->oper, "BWRC") == 0 )
			backgrnd_thread = 'E';

		iocmd.buffer = 0;

		ps->bad_writes = ps->bad_writes + 1;
		prt_msg(ps, pr, loop, blkno, errno, HARD, "Write error (pass_thru) - ");
		return(errno);
	} else {
		/* command successfull */
		if(debug_passthru == 1){
			sprintf(msg, "IOCTL return status = %d\n", rc);
			user_msg(ps, 0, INFO, msg);
		}

		ps->good_writes = ps->good_writes + 1;
		ps->bytes_writ = ps->bytes_writ + pr->dlen;

		iocmd.buffer = 0;
		return(0);
	}
}

int read_verify_disk(struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long *blkno,
				char *rbuf)
{
	struct  scsi_iocmd iocmd      ;
	int  rc;
	char msg[256];

	if (debug_passthru == 1){
		sprintf( msg, " Inside read_verify_disk \n " );
		user_msg(ps, 0, INFO, msg);
	}
	/*initialise sc_passthru struct*/
	memset(&iocmd, 0, sizeof(struct scsi_iocmd));
	iocmd.version = SCSI_VERSION_2;
	iocmd.timeout_value = 30; /* default to 30 secs */
	iocmd.status_validity = 0;
	iocmd.scsi_id = scsi_id; /* Device scsi_id */
	iocmd.lun_id = lun_id;   /* Device lun_id  */
	iocmd.flags |= B_READ;
	iocmd.data_length =  pr->dlen;
	iocmd.command_length = 10;

	/* Make the iocmd buffer point to the data we need to read from */
	iocmd.buffer = rbuf;

	/* Initialise iocmd structure with the reqd data */
	iocmd.scsi_cdb[0] = 0x28;            /* set operation code */
	iocmd.scsi_cdb[1] = (lun_id << 5);   /* obsolete field now */
	iocmd.scsi_cdb[2] = (uchar) (blkno[0] >> 24) & 0xff;     /* LBA MSB    */
	iocmd.scsi_cdb[3] = (uchar) (blkno[0] >> 16) & 0xff;     /* LBA MSB    */
	iocmd.scsi_cdb[4] = (uchar) (blkno[0] >> 8) & 0xff;      /* LBA        */
	iocmd.scsi_cdb[5] = (uchar) (blkno[0] & 0xff);           /* LBA LSB    */
	iocmd.scsi_cdb[7] = (pr->num_blks >> 8) & 0xff;  /* transfer lgnth MSB */
	iocmd.scsi_cdb[8] = (pr->num_blks & 0xff);       /* transfer lgnth LSB */
	iocmd.scsi_cdb[9] = 0;


	/* Do the ioctl */
	if (debug_passthru == 1){
		sprintf( msg, " Before sending the CDB \n " );
		user_msg(ps, 0, INFO, msg);
	}


	rc = eeh_ioctl(ps, pr->adapter, SCIOLCMD, (void *)&iocmd);
		/* check return code of cmd  */
	if ( rc != 0 ) {
		sprintf( msg, " einval_arg = %d, errno = %d \n ", iocmd.adap_set_flags, errno );
		user_msg(ps, 0, INFO, msg);
		if ( strcmp(pr->oper, "BWRC") == 0 )
			backgrnd_thread = 'E';

		iocmd.buffer = 0;

		ps->bad_reads = ps->bad_reads + 1;
		prt_msg(ps, pr, loop, blkno, errno, HARD, "read error(pass_thru) - ");
		return(errno);
	} else {
		/* command successfull */
		if (debug_passthru == 1 ){
			sprintf(msg, "IOCTL return status = %d\n", rc);
			user_msg(ps, 0, INFO, msg);
		}
		ps->good_reads = ps->good_reads + 1;
		ps->bytes_read = ps->bytes_read + pr->dlen;

		iocmd.buffer = 0;
		return(0);
	}
}

verify_disk_wrapper(struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long *blkno,
char *wbuf)
{
	int  rc, adapter;
	char msg[256], parent[256], tmp[256];


	/*
     * SUPPORT FOR PASS-THRU COMMANDS STARTS. AS OF NOW,
     * ONLY SCIOCMD INTERFACE IS SUPPORTED. THIS IS AN
     * ADAPTER IOCTL, SO THE IOCTL WILL BE ISSUED TO THE
     * PARENT DEVICE, THAT IS, ADAPTER. SEE D#388686 FOR
     * DETAILS.
     */
	if (pass_thru == 1){

		if (debug_passthru == 1)
		{
			sprintf(msg, "Device Name = %s  Parent = %s \n",ps->sdev_id, disk_parent);
			user_msg(ps, 0, INFO, msg);
		}

		/* open the device's parent */

		strcpy(tmp,"/dev/");
		strcat(tmp, disk_parent);

		if( ( adapter = eeh_open(ps, tmp,  O_RDWR) ) == -1 ) {
			sprintf(msg, "Open error in write_disk (pass_thru) - ");
			if ( errno > 0 && errno <= sys_nerr ) strcat(msg, sys_errlist[errno]);
			strcat(msg, "\n");
			user_msg(ps, errno, HARD, msg);
			return(-1);
		}

		/* First issue SCIOSTOP, just in case the prev START
         * has not been STOPed yet
         */

		rc = eeh_ioctl(ps, adapter, SCIOSTOP, (void *)IDLUN(scsi_id, lun_id));
		/* Issue SCIOSTART now */

		rc = eeh_ioctl(ps, adapter, SCIOSTART, (void *)IDLUN(scsi_id, lun_id));
		if( rc !=0 )
		{
			close(adapter);
			sprintf(msg, "SCIOSTART failed: %s\n",sys_errlist[errno]);
			user_msg(ps, errno, HARD, msg);
			return(-1);
		}

		/* send the passthru command */

		if (debug_passthru == 1){
			sprintf(msg, "Attempting to send the passthru command- Verify \n");
			user_msg(ps, 0, INFO, msg);
		}

		if( ( rc = verify_disk(ps, pr, loop, blkno, wbuf, adapter)) != 0 ) {
			/*while ( close(pr->fildes) == -1 ); */

			/* Issue SCIOSTOP now...*/

			rc = eeh_ioctl(ps, adapter, SCIOSTOP, (void *)IDLUN(scsi_id, lun_id));
			if( rc !=0 )
			{
				close(adapter);
				sprintf(msg, "SCIOSTOP failed: %s\n",sys_errlist[errno]);
				user_msg(ps, errno, HARD, msg);
				return(-1);
			}
			eeh_close(ps, adapter);
			return rc;
		} else {
			if (debug_passthru == 1){
				sprintf(msg, "Sent passthru command - Verify successfully \n");
				user_msg(ps, 0, INFO, msg);
			}
		}

		/*
         * Issue SCIOSTOP
         */

		rc = eeh_ioctl(ps, adapter, SCIOSTOP, (void *)IDLUN(scsi_id, lun_id));
		if( rc !=0 )
		{
			close(adapter);
			sprintf(msg, "SCIOSTOP failed: %s\n",sys_errlist[errno]);
			user_msg(ps, errno, HARD, msg);
			return(-1);
		}
		eeh_close(ps, adapter);

		return 0;
	}

}

int verify_disk(struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long *blkno,
char *wbuf, int fildes)
{
	struct        sc_passthru scpt;
	/*extern int lun; */
	int  rc, err, retries = 0;
	char msg[256];

	if (debug_passthru == 1){
		sprintf( msg, " Inside write_verify_disk \n " );
		user_msg(ps, 0, INFO, msg);
	}

	/*initialise sc_passthru struct*/

	memset(&scpt, 0, sizeof(struct sc_passthru));
	scpt.version = 0x02;
	scpt.timeout_value = 60; /* default to 30 secs */
	/*scpt.status_validity = SC_SCSI_ERROR; */
	scpt.scsi_id = scsi_id; /* Device scsi_id */
	scpt.lun_id = lun_id;   /* Device lun_id  */
	scpt.q_tag_msg = SC_NO_Q;

	/* Initialise iocmd structure with the reqd data */
	scpt.scsi_cdb[0] = 0x2f;        /* set operation code */
	scpt.scsi_cdb[1] = (lun_id << 5);  /* Obsolete field now */
	scpt.scsi_cdb[2] = (uchar) (blkno[0] >> 24) & 0xff;     /* LBA MSB    */
	scpt.scsi_cdb[3] = (uchar) (blkno[0] >> 16) & 0xff;     /* LBA MSB    */
	scpt.scsi_cdb[4] = (uchar) (blkno[0] >> 8) & 0xff;      /* LBA        */
	scpt.scsi_cdb[5] = (uchar) (blkno[0] & 0xff);           /* LBA LSB    */
	scpt.scsi_cdb[6] = 0;
	scpt.scsi_cdb[7] = (pr->num_blks >> 8) & 0xff;  /* transfer lgnth MSB */
	scpt.scsi_cdb[8] = (pr->num_blks & 0xff);       /* transfer lgnth LSB */
	scpt.scsi_cdb[9] = 0;
	scpt.data_length =  0;
	scpt.command_length = 10;
	scpt.buffer = NULL;
	/* Do the ioctl */
	if (debug_passthru == 1){
		sprintf( msg, " Before sending the CDB \n " );
		user_msg(ps, 0, INFO, msg);
		dump_iocmd( ps, &scpt );
	}

	for (retries = 0; retries <3; retries++){
		if (rc = eeh_ioctl(ps, fildes, SCIOCMD, &scpt) !=0 ){
			#if 0
			if (debug_passthru == 1){
				sprintf( msg, " After sending the CDB \n " );
				user_msg(ps, 0, INFO, msg);
			}
			#endif

			/* sprintf( msg, " einval_arg = %d \n status_validity = %d scsi_bus_status = %d",
                scpt.einval_arg, scpt.status_validity, scpt.scsi_bus_status ); */
			sprintf( msg, "error sending ioctl command = errno %d  einval_arg = %d \n" "status_validity = %d scsi_bus_status = %d",
			    errno, scpt.einval_arg, scpt.status_validity, scpt.scsi_bus_status );
			user_msg(ps, 950, HARD, msg);

			if ((scpt.status_validity & 0x03) == SC_SCSI_ERROR)
			{
				if (scpt.scsi_bus_status == SC_CHECK_CONDITION)
				{
					sprintf(msg, "\nSCSI status valid\n");
					user_msg(ps, 950, HARD, msg);
				}
			}
			else
			{
				if((scpt.status_validity & 0x03) == SC_ADAPTER_ERROR)
				{
					sprintf(msg, "\nSCSI adapter status valid\n");
					user_msg(ps, 950, HARD, msg);
				}
			}
		} /* End if rc != 0 */
	} /* End for retries */
}


/*************************************************************************/
/* dump_iocmd is a debug routine that dumps the sc_iocmd structure       */
/*************************************************************************/
void dump_iocmd(struct htx_data *ps, struct sc_passthru *s)
{
	int i;
	char msg[512];

	sprintf(msg, "\nDump of the struct sc_passthru about to be sent:\n\n");
	user_msg(ps, 0, INFO, msg);

	sprintf(msg, "Version     = %d", s->version);
	user_msg(ps, 0, INFO, msg);

	sprintf(msg, "data_length    = %llu (0x%llx); ", s->data_length, s->data_length);
	user_msg(ps, 0, INFO, msg);

	sprintf(msg, "buffer = 0x%lx\n", s->buffer);
	user_msg(ps, 0, INFO, msg);

	sprintf(msg, "timeout_value = %u (0x%x); ", s->timeout_value, s->timeout_value);
	user_msg(ps, 0, INFO, msg);

	sprintf(msg, "status_validity = %u (0x%02x)\n", s->status_validity, s->status_validity);
	user_msg(ps, 0, INFO, msg);

	sprintf(msg, "scsi_bus_status= %u (0x%02x); ", s->scsi_bus_status, s->scsi_bus_status);
	user_msg(ps, 0, INFO, msg);

	sprintf(msg, "adapter_status = %u (0x%02x)\n", s->adapter_status, s->adapter_status);
	user_msg(ps, 0, INFO, msg);

	sprintf(msg, "adap_q_status  = %u (0x%02x); ", s->adap_q_status, s->adap_q_status);
	user_msg(ps, 0, INFO, msg);

	sprintf(msg, "q_tag_msg = %u (0x%02x)\n", s->q_tag_msg, s->q_tag_msg);
	user_msg(ps, 0, INFO, msg);

	sprintf(msg, "flags          = %u (0x%02x); ", s->flags, s->flags);
	user_msg(ps, 0, INFO, msg);

	sprintf(msg, "command_length = %u (0x%02x)\n", s->command_length, s->command_length);
	user_msg(ps, 0, INFO, msg);

	for (i = 0; i < 10; i++) {
		sprintf(msg, "scsi_cdb[%2d] = 0x%02x\n", i, s->scsi_cdb[i]);
		user_msg(ps, 0, INFO, msg);
	}

}
#endif /* End of AIX specific function */

/**************************************************************************/
/* Execute a system command from a psuedo command line                    */
/**************************************************************************/
int
do_cmd(struct htx_data *ps, struct ruleinfo *pr)
{
    int    a, b, c, d, flag = 0, rc = 0, filedes;
    char   tmsg[600], cmd_line[300], msg[650];
    char   filenam[30] = "/tmp/cmdout.";

    b = 0;
    if ( strncmp(ps->sdev_id, "/dev/rlv", 8) == 0 ) {
        sprintf(msg, "Command %s\nCANNOT be run against a logical volume!", pr->cmd_list);
        user_msg(ps, 0, INFO, msg);
        return(0);
    }
    for ( a = 0; a <= strlen(pr->cmd_list); a++ ) {
        cmd_line[b] = pr->cmd_list[a];
        if ( cmd_line[b] == '$' ) {
            a++;
            if ( pr->cmd_list[a] == 'd' || pr->cmd_list[a] == 'D' || pr->cmd_list[a] == 'p' || pr->cmd_list[a]
                == 'P' || pr->cmd_list[a] == 'o' ) {
                switch ( pr->cmd_list[a] ) {
                case 'd' :
                    {
                        for ( c = 6; c < strlen(ps->sdev_id); c++ ) {
                            cmd_line[b] = ps->sdev_id[c];
                            b++;
                        }
                        break;
                    }
                case 'D' :
                    {
                        for ( c = 5; c < strlen(ps->sdev_id); c++ ) {
                            cmd_line[b] = ps->sdev_id[c];
                            b++;
                        }
                        break;
                    }
                case 'p' :
                    {
                        for ( c = 0; c < strlen(ps->sdev_id); c++ ) {
                            cmd_line[b] = ps->sdev_id[c];
                            if ( cmd_line[b] == 'r' && cmd_line[b-1] == '/' ) ;
                            else
                                b++;
                        }
                        break;
                    }
                case 'P' :
                    {
                        for ( c = 0; c < strlen(ps->sdev_id); c++ ) {
                            cmd_line[b] = ps->sdev_id[c];
                            b++;
                        }
                        break;
                    }
                default :
                    for ( c = 0; c < 12; c++ ) {
                        cmd_line[b] = filenam[c];
                        b++;
                    }
                    d = 12;
                    for ( c = 6; c < strlen(ps->sdev_id); c++ ) {
                        cmd_line[b] = ps->sdev_id[c];
                        filenam[d] = ps->sdev_id[c];
                        b++;
                        d++;
                    }
                    flag = 1;
                }
            } else {
                b++;
                cmd_line[b] = pr->cmd_list[a];
                b++;
            }
        } else
            b++;
    }
    if ( ps->run_type[0] == 'O' || pr->messages[0] == 'D' || pr->messages[0] == 'Y' ) {
        sprintf(msg, "Command to be Executed > \n %s\n", cmd_line);
        user_msg(ps, 0, INFO, msg);
    }
    if ( (rc = system(cmd_line)) != 0 ) {
        if ( (filedes = open(filenam, O_RDONLY)) == -1 ) {
            sprintf(msg, "Command FAILED rc = %d > \n No Error Information " "returned from command:\n %s\n",
                rc, cmd_line);
            user_msg(ps, 950, HARD, msg);
        } else {
            sprintf(msg, "COMMAND: %s FAILED\n with the Following Error " "Information:\n", cmd_line);
            memset(tmsg, '\0', 600);
            read(filedes, tmsg, 600);
            strcat(msg, tmsg);
            while(close(filedes) == -1);
            sprintf(tmsg, "rm %s", filenam);
            system(tmsg);
            user_msg(ps, 950, HARD, msg);
        }
    } else if ( flag == 1 ) {
        sprintf(tmsg, "rm %s", filenam);
        system(tmsg);
    }
    return(rc);
}

/* function eeh_open */
/* to handle eeh in one place for open system call */
int eeh_open(struct htx_data *ps, const char *pathname, int flags)
{
	int rc = -1;

	rc = open(pathname, flags);

	if ( (rc == -1) && eeh_enabled )
	{
		unsigned int temp_retries = eeh_retries;

		while( (rc == -1) && temp_retries )
		{
			sleep(5);
			sprintf(msg, "EEH retry by hxehd exerciser, on open system call failure  retry number =  %d \n",(eeh_retries - (temp_retries - 1)));
			user_msg(ps, errno, INFO, msg);
			/* retry */
			rc = open(pathname, flags);
			temp_retries--;
		}

		if( rc >= 0 )
		{
			sprintf(msg, "EEH retry succesfull by hxehd exerciser, for open call retry number  %d \n",(eeh_retries - temp_retries));
			user_msg(ps, 0, INFO, msg);
		}
		if ( (rc == -1) && !temp_retries )
		{
			sprintf(msg, "\n Exerciser failed on max retries for EEH, open call failed \n\n");
			user_msg(ps, errno, HARD, msg);
		}
	}

	return rc;
}

/* function eeh_close */
/* to handle eeh in one place for close system call */
int eeh_close(struct htx_data *ps, int filedescr)
{
	int rc = -1;

	rc = close(filedescr);

	if ( (rc == -1) && eeh_enabled )
	{
		unsigned int temp_retries = eeh_retries;
		while( (rc == -1) && temp_retries )
		{
			sleep(5);
			sprintf(msg, "EEH retry by hxehd exerciser, on close system call failure  retry number =  %d \n",(eeh_retries - (temp_retries - 1)));
			user_msg(ps, errno, INFO, msg);
			/* retry */
			rc = close(filedescr);
			temp_retries--;
		}

		if( rc >= 0 )
		{
			sprintf(msg, "EEH retry succesfull by hxehd exerciser, for close call retry number  %d \n",(eeh_retries - temp_retries));
			user_msg(ps, 0, INFO, msg);
		}
		if ( (rc == -1) && !temp_retries )
		{
			sprintf(msg, "Exerciser failed on max retries for EEH, close call failed \n");
			user_msg(ps, errno, HARD, msg);
		}
	}

	return rc;
}

/* function eeh_ioctl */
/* to handle eeh in one place for ioctl system call */
int eeh_ioctl(struct htx_data *ps, int filedescr, int command, void *arg)
{
	int rc = -1;

	rc = ioctl(filedescr, command, arg);

	if ( (rc == -1) && eeh_enabled )
	{
		unsigned int temp_retries = eeh_retries;

		while( (rc == -1) && temp_retries )
		{
			sleep(5);
			sprintf(msg, "EEH retry by hxehd exerciser, on ioctl system call failure  retry number =  %d \n",(eeh_retries - (temp_retries - 1)));
			user_msg(ps, errno, INFO, msg);
			/* retry */
			rc = ioctl(filedescr, command, arg);
			temp_retries--;
		}

		if( rc >= 0 )
		{
			sprintf(msg, "EEH retry succesfull by hxehd exerciser, for ioctl call retry number  %d \n",(eeh_retries - temp_retries));
			user_msg(ps, 0, INFO, msg);
		}
		if ( (rc == -1) && !temp_retries )
		{
			sprintf(msg, "\n Exerciser failed on max retries for EEH, ioctl call failed \n\n");
			user_msg(ps, errno, HARD, msg);
		}
	}

	return rc;
}

/* function eeh_read */
/* to handle eeh in one place for read system call */
int eeh_read(struct htx_data *ps, int filedescr, void *buf, unsigned int nbytes)
{
	int rc = -1;

	rc = read(filedescr, buf, nbytes);

	if ( (rc == -1) && eeh_enabled )
	{
		unsigned int temp_retries = eeh_retries;

		while( (rc == -1) && temp_retries )
		{
			sleep(5);
			sprintf(msg, "EEH retry by hxehd exerciser, on read system call failure  retry number =  %d \n",(eeh_retries - (temp_retries - 1)));
			user_msg(ps, errno, INFO, msg);
			/* retry */
			rc = read(filedescr, buf, nbytes);
			temp_retries--;
		}

		if( rc >= 0 )
		{
			sprintf(msg, "EEH retry succesfull by hxehd exerciser, for read call retry number  %d \n",(eeh_retries - temp_retries));
			user_msg(ps, 0, INFO, msg);
		}
		if ( (rc == -1) && !temp_retries )
		{
			sprintf(msg, "\n Exerciser failed on max retries for EEH, read call failed \n\n");
			user_msg(ps, errno, HARD, msg);
		}
	}

	return rc;
}

/* function eeh_write */
/* to handle eeh in one place for write system call */
int eeh_write(struct htx_data *ps, int filedescr, void *buf, unsigned int nbytes)
{
	int rc = -1;

	rc = write(filedescr, buf, nbytes);

	if ( (rc == -1) && eeh_enabled )
	{
		unsigned int temp_retries = eeh_retries;

		while( (rc == -1) && temp_retries )
		{
			sleep(5);
			sprintf(msg, "EEH retry by hxehd exerciser, on write system call failure  retry number =  %d \n",(eeh_retries - (temp_retries - 1)));
			user_msg(ps, errno, INFO, msg);
			/* retry */
			rc = write(filedescr, buf, nbytes);
			temp_retries--;
		}

		if( rc >= 0 )
		{
			sprintf(msg, "EEH retry succesfull by hxehd exerciser, for write call retry number  %d \n",(eeh_retries - temp_retries));
			user_msg(ps, 0, INFO, msg);
		}
		if ( (rc == -1) && !temp_retries )
		{
			sprintf(msg, "\n Exerciser failed on max retries for EEH, write call failed \n\n");
			user_msg(ps, errno, HARD, msg);
		}
	}

	return rc;
}

