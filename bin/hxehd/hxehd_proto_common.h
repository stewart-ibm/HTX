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

/* @(#)19       1.6  src/htx/usr/lpp/htx/bin/hxehd/hxehd_proto.h, exer_hd, htx61V 2/3/12 05:59:01 */


#ifdef __64BIT__
#include "hxihtx64.h"
#else
#include "hxihtx.h"
#endif

int info_msg(struct htx_data *ps, struct ruleinfo *pr, int loop,
		 unsigned long long *blkno, char *msg_text);

int prt_msg(struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long *blkno,
		int err, int sev, char *text);

int user_msg(struct htx_data *ps, int err, int sev, char *text);

void seg_lock(struct htx_data *ps, unsigned long long flba, unsigned long long llba);
void seg_unlock(struct htx_data *ps, struct ruleinfo *pr, unsigned long long flba, unsigned long long llba);
void init_seed(struct random_seed_t *seed);

void random_dlen(short bytpsec, unsigned long long max_blkno, struct random_seed_t *seed,
			struct ruleinfo *pr);
void random_blkno(unsigned long long *blkno, unsigned int len, short bytpsec, unsigned long long max_blkno,
			 struct random_seed_t *seed, unsigned long long min_blkno, unsigned int blk_align);

void init_blkno(struct htx_data *ps,struct ruleinfo *pr, unsigned long long  *blkno);
unsigned long long set_first_blk(struct ruleinfo *pr);
void set_blkno(unsigned long long *blkno, char *direction, int increment, unsigned int num_blks, unsigned int blk_align);
int wrap(struct ruleinfo *pr, unsigned long long blkno[]);
void clrbuf(char buf[], unsigned int dlen);
int bldbuf(unsigned short *wbuf, struct random_seed_t *seed, unsigned long long *blkno,
	   struct htx_data *ps, struct ruleinfo *pr, char algorithm, unsigned short nwrites);
char cmpbuf( struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long *blkno,
			char wbuf[], char rbuf[], unsigned int nwrites );
int do_compare( struct htx_data *ps, struct ruleinfo *pr, char wbuf[],
				char rbuf[], int crash_flag, int *badsig, int *cksum,
								int loop, unsigned long long blkno, unsigned int nwrites );
int verify_disk_wrapper(struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long *blkno,char *wbuf); 
void run_timer(struct htx_data *ps, struct ruleinfo *pr);
void do_boundary_checks(struct ruleinfo *pr, struct htx_data *ps, unsigned long long *blkno,
				int * saved_dir_flag, int * do_partial);
int set_addr(struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long *blkno);
int write_disk(struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long *blkno,
		   char *wbuf);

int read_disk(struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long *blkno,
		  char *rbuf);
static int parse_keywds( char s[], struct operation * oper );
int read_cache(struct cache_chk *ck);
int write_mem(struct c_thread *ctab);
int read_mem(struct c_thread *ctab);
int read_cache_disk(struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long *blkno,
				char *rbuf, char *wbuf);
int write_cache(struct cache_chk *ck);
int write_cache_disk(struct htx_data *ps, struct ruleinfo *pr, int loop, unsigned long long *blkno,
				 char *wbuf, char *rbuf);
int compare_cache(struct htx_data *ps, struct ruleinfo *pr, int loop,
			  char rbuf[], char wbuf[], unsigned long long *blkno);
int ominfo(struct htx_data s, struct ruleinfo * r);
int ominqry(struct htx_data s, struct ruleinfo * r);
int omprevent(struct htx_data s, struct ruleinfo * r);
int ominit(struct htx_data s, struct ruleinfo *r);
int ommove(struct htx_data s, struct ruleinfo * r);
int omexchge(struct htx_data s, struct ruleinfo * r);
int ominvtry(struct htx_data s, struct ruleinfo * r);
int do_cmd(struct htx_data *ps, struct ruleinfo *pr);
int eeh_open(struct htx_data *ps, const char *pathname, int flags);
int eeh_close(struct htx_data *ps, int filedescr);
int eeh_ioctl(struct htx_data *ps, int filedescr, int command, void *arg);
int eeh_read(struct htx_data *ps, int filedescr, void *buf, unsigned int nbytes);
int eeh_write(struct htx_data *ps, int filedescr, void *buf, unsigned int nbytes);
int get_rule(struct htx_data *ps, char rules_file_name[100], unsigned long long maxblk, unsigned long blksize);
unsigned int get_random_number(struct random_seed_t *seed);
void sig_function(int sig, int code, struct sigcontext *scp);
void * zip_thread (void * rule_stats_p);
void start_msg(struct htx_data *ps, struct ruleinfo *pr, char *msg_text);
char * getsysvol(char *buf, int bufsize);
int proc_rule(struct htx_data s, struct ruleinfo r);
void freebufs(int *pmallo, char *wptr[], char *rptr[]);
int isVG_member(unsigned char *diskdev, char *sysmsg, int msg_length);
int get_lun(struct htx_data *ps, unsigned char *dev_name);
int e_notation(double d, char *s);
int generate_pattern7( char * wbuf, unsigned int bufsize, unsigned int line_length, struct htx_data * htx);
int generate_pattern8( char * wbuf, unsigned int bufsize, unsigned int line_length, struct ruleinfo * pr, struct htx_data * htx);
int generate_pattern9( char * wbuf, unsigned int bufsize, unsigned int length, struct htx_data * htx, struct ruleinfo * pr);
int generate_patternA( char * wbuf, unsigned int bufsize, unsigned int line_length, struct htx_data * htx, struct ruleinfo * pr);
int generate_pattern( char * buf, unsigned int bufsize, char pattern_id, unsigned int line_length, struct random_seed_t *seed, struct htx_data * htx, struct ruleinfo * pr);

/* htxmp lib protos */
int hxfmsg_tsafe(struct htx_data *p, int err, enum sev_code sev, char * text);
int hxfupdate_tsafe(char type,struct htx_data *arg);
int hxfpat_tsafe(char *filename, char *pattern_buf, size_t num_chars);
int hxfsbuf_tsafe(char *buf, size_t len, char *fname, struct htx_data *ps);
int hxfcbuf_tsafe(struct htx_data *ps, char *wbuf, char *rbuf, size_t len, char *msg);
int hxfinqcpu_state(int* cpustate, int * count_active_cpus, int *arch);
int hxfbindto_a_cpu(int *cpustate, int tid, short * cpunum);
int hxfbindto_the_cpu(int cpu, int tid);

