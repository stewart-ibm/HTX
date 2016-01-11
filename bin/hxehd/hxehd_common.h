/* @(#)38       1.29  src/htx/usr/lpp/htx/bin/hxehd/hxehd.h, exer_hd, htx61N 7/21/10 01:49:22 */

#include <pthread.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <stdlib.h>
#include <limits.h>
#ifdef __HTX_LINUX__
    /* Linux Specific header files */
	#include "devinfo.h"
	#include <unistd.h>
#else
    /* AIX Specific header files */
    #include <sys/devinfo.h>
    #include <sys/scsi.h>
    #include <sys/scdisk.h>
    #include <cf.h>
    #include <sys/cfgodm.h>
    #include <odmi.h>
    #include <sys/cfgdb.h>
    #include <lvm.h>
    #include <sys/scsi_buf.h>
    #include <sys/scsi.h>
    #include <sys/scdisk.h>
	#include <sys/mdio.h>
	#endif
#ifdef __64BIT__
	#include "hxihtx64.h"
	#include "hxiipc64.h"
#else
	#include "hxihtx.h"
	#include "hxiipc.h"
#endif
#include "hxihtxmp.h"

#define HARD 1
#define MISCOMPARE 2
#define SOFT 4
#define INFO 7
#define PATLIB_PATH "/usr/lpp/htx/pattern/"
#define MAX_THREADS 512
#define MAX_STANZA 1024
#define MAX_NUMBER_OF_THREAD 10
#define SAVE_SEEDS_FLAG  0x1	/* LJR - add rule_options */
#define RESTORE_SEEDS_FLAG 0x2	/* LJR - add rule_options */
#define USER_SEEDS_FLAG 0x4     /* DGS - add rule_options */
#define VALIDATION  0x11
#define PERFORMANCE 0x12

#ifdef __HTX_LINUX__
	extern __const char * __const sys_errlist[]; /* system defined err msgs, indexed by errno      */
#else 
	extern char *  sys_errlist[]; 
#endif 
/*****************************************************************************
** Following pragma allows us to create some inline code that will checkstop
** the machine without having to go into a kernel extension.  This trap is
** invoked in the cmpbuf routine when a miscompare is detected and if attn
** is requested through "turn_attention_on" flag in the rules file. If attn
** instruction is not supported, then the exerciser will give signal 4, that is,
** illegal instruction.
******************************************************************************/
#ifndef __HTX_LINUX__

#pragma mc_func attn { "00000200" }
#pragma reg_killed_by attn
#pragma mc_func trap { "7c810808" }
#pragma reg_killed_by trap

#else
void __attn(unsigned int a,
			unsigned int b,
			unsigned int c,
			unsigned int d,
			unsigned int e,
			unsigned int f,
			unsigned int g);


#endif

extern int sys_nerr;        /* max value for errno                            */

struct operation {
	char oper[16];
	int num_reads;
	int num_writes;
	int compare_enabled;
};

struct ruleinfo {
	struct ruleinfo *next_ruleptr;
	char  rule_id[16];        			 	/* Rule Id                                        */
	char  pattern_id[9];      				/* pattern library id                             */
	char  messages[4];        				/* YES, NO                                        */
	char  addr_type[7];       				/* SEQ, RANDOM                                    */
	int   num_oper;           				/* number of operations to be performed           */
	char  oper[16];            				/* WRC,RWRC,R,W,RC,S                              */
	char  starting_block[9];  				/* n,cc/hh/ss,TOP,MID,BOT      *                  */
	char  direction[5];       				/* UP, DOWN, IN, OUT           * seq oper only    */
	int   increment;          				/* number of blocks to skip    *                  */
	char  type_length[7];     				/* FIXED, RANDOM                                  */
	unsigned int   num_blks;  				/* length of data to read/write in blocks         */
	int   fildes;             				/* file descriptor for device to be exercised     */
	int adapter; 							/* File descriptor for adapter for passthru 	  */
	unsigned long long first_block;			/* starting block converted to numeric            */
	unsigned int dlen;		  				/* length of data to read/write in bytes          */
	short bytpsec;            				/* Number of bytes per sector                     */
	unsigned long long tot_blks;			/* Total number of blocks on volume               */
	unsigned long long min_blkno;  			/* lowest block to be used                        */
	unsigned long long max_blkno;  			/* highest block to be used                       */
	long  min_blklen;         				/* random_dlen returns >= min_blklen (RDT)        */
	int   op_rate;            				/* number of read/write ops per second (RDT)      */
	int   rule_time;          				/* time till this rule will run (RDT)             */
	int   stanza_time;        				/* time till this rule will run (accoustincs)     */
	unsigned int sleep;       				/* number of seconds to sleep                     */
	int   pat_cnt;            				/* number of user supplied pattern forms          */
	unsigned short form[8];   				/* user supplied pattern form                     */
	int   align;              				/* Alignment for IO buffers requested             */
	int   lba_align;						/* user supplied IOP alignement					  */ 
	int   rule_offset;        				/* 1 ==> offset set 0 ==> offset not set          */
	int   offset;             				/* offset into a word boundary                    */
	int   loop_on_offset;     				/* YES or NO                                      */
	int   no_mallocs;         				/* number of mallocs before freeing               */
	int   stanza_msg;         				/* stanza_msg = yes then logs stanza messages     */
	int   max_number_of_threads; 			/* max number of threads allowed                  */
	char  start;              				/* switch for pipe processing                     */
	char  timed;              				/* switch for timing purposes or not              */
	char  cmd_list[250];      				/* string to hold shell command line              */
	float percent;            				/* percentage to increase num oper                */
	int   repeat_pos;         				/* number of times to skip stanza                 */
	int   repeat_neg;         				/* number of times to skip stanza                 */
	unsigned int rule_options; 				/* add rule_options                               */
	char  dev_name[20];      				/* device special file name                       */
	int   om_source;         				/* source id of media                             */
	int   om_target;         				/* target id of media                             */
	int   om_target1;        				/* target id of media second slot                 */
	int   om_invert;         				/* is media to be inverted                        */
	int   om_invert1;        				/* is second media to be inverted                 */
	int   om_prevent;        				/* is media allowed to be removed                 */
	int   om_maxslots;       				/* maximum number of media slots if chgr          */
	unsigned short seed[7];  				/* program value for number generator             */
	long  seed_lba;          				/* program seed value for max lba                 */
	unsigned short useed[7]; 				/* user defined value for number generator        */
    struct operation opertn; 				/* If user specifies oper of format w[M]r[N]c     */
	long  useed_lba;         				/* user defined seed value for max lba            */
    char * rand_buf; 		 				/* Random buffer 								  */
	unsigned int size_rand_buf; 			/* size of random buffer 						  */
	unsigned long long rand_index;   				/* random index into rand_buf 			  */
	unsigned long long and_mask, or_mask;	/* and. or mask for pattern 8,9,A				  */
	unsigned int beging_dword; 				/* beggining dword								  */
	signed long long trailing_dword;		/* Trailing dword where to write pattern 		  */
	unsigned int hotness;					/* Number of IO to be done to make a band HOT	  */ 
} *current_ruleptr;

struct random_seed_t {
	unsigned short xsubi[3];
	struct drand48_data drand_struct;
};



struct rule_stats {
	struct ruleinfo *rule;
	struct htx_data *stats;
};

struct cache_chk  {
    int          filedes;
    unsigned int dlen;
    char         *buff1;
    char         *buff2;
    struct htx_data *ts;
};
struct c_thread {
    unsigned int dlen;
    char         *buff;
    int          count;
    struct htx_data *ts;
};

typedef struct rule_stats rule_stats_t;


