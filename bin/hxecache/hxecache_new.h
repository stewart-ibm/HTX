/* @(#)20       1.5.1.11  src/htx/usr/lpp/htx/bin/hxecache/hxecache_new.h, exer_cache, htx61S, htx61S_691 5/22/12 06:03:38  */

#include <pthread.h>
#include "htxsyscfg64_new.h"

#include <sys/shm.h>
#include <sys/mman.h>

#ifdef __HTX_LINUX__
#    include <sys/stat.h>
#    include <sys/ioctl.h>
#    include <linux/unistd.h>
#    include <fcntl.h>
#endif
#include <sys/types.h>
#include <sys/ipc.h>

#ifndef __HTX_LINUX__
#    include <sys/systemcfg.h>
#    include <sys/vminfo.h>
#    include <sys/processor.h>
#endif

#include <signal.h>
#include <stdlib.h>
#include <strings.h>
#include <hxihtx64.h>

#include <stdio.h>
#include <stdlib.h>

#ifndef __HTX_LINUX__
#    include <sys/thread.h>
#    include <memory.h>
#endif

#ifdef __HTX_LINUX__
#    include <stdint.h>
#    include <strings.h>
#    include <string.h>
#    include <stdarg.h>
#    include <unistd.h>
#    include <sched.h>

/*
 * Since glibc has no wrapper for gettid
 * we need to use the syscall() function call it
 * indirectly. We need an argument of pid_t
 * to be given to sched_setaffinity when binding
 * a thread to a processor, pthread_self() returns
 * a thread id of type pthread_t AND getpid() returns
 * the pid of the parent thread.
 * The following include is req for syscall()
 */
#include <sys/syscall.h>
#endif

#include <errno.h>

#if defined(_DR_HTX_) && !defined(__HTX_LINUX__)
#    include <sys/types.h>
#    include <sys/dr.h>
#    include <sys/procfs.h>
#endif

#if 0
#ifndef __HTX_LINUX__
#    pragma mc_func trap { "7c810808" }
#endif
#endif

#ifdef DEBUG
#    define DEBUGON printf
#    define FPRINTF(a,x,...)
#else
#    define DEBUGON
#    define DEBUGOFF(x, ...)
#    define FPRINTF(a,x,...)
#endif

#define  B     1
#define  K     1024*B
#define  M     1024*K

#define L2 0
#define L3 1

/* Thread types */
#define ALL         0    /* For an uninitialised thread */
#define CACHE       1
#define PREFETCH    2

/* Testcase types */
#define CACHE_BOUNCE    0
#define CACHE_ROLLOVER    1
#define CACHE_TEST_OFF    2

/* Generic ON/OFF states used for flags*/
#define OFF            0
#define ON            1

/* Prefetch Algorithms */
#define PREFETCH_IRRITATOR            1
#define PREFETCH_NSTRIDE            2
#define PREFETCH_PARTIAL            4
#define PREFETCH_TRANSIENT            8

#define ROUND_ROBIN_ALL_ENABLED_PREFETCH_ALGORITHMS 16

#define GANG_SIZE 8
#define MAX_CONTIG_PAGES 4
#define MAX_RULEID_LEN	40		/* Maximum characters in Rule id string */
/*
 * For now create one 512 MB seg and see if you can get required physical
 * contiguous pages.
 */

#define NUM_SEGS 1
#define SEG_SIZE 256*M
#define MAX_TC 12
#define MAX_DEVS 1024
#if 0
#define MAX_CPUS        1024
#endif
#define LOGFILE_W       "wlog"
#define LOGFILE_R       "rlog"

#define PAGE_FREE    0
#define CACHE_PAGE    1
#define PAGE_HALF_FREE 2
#define PREFETCH_PAGE  3
#define PREFETCH_MEM_SZ 8*M

/* Exit flag codes */
#define ERR_EXIT 	1
#define NORMAL_EXIT	2
#define DR_EXIT 	3
#define CPUFAIL_EXIT	4

struct cache_info {
    int line_size;    /* Cache line size */
    int asc;        /* cache associativity */
    int num_sets;    /* Number of sets */
    int cache_size;    /* Total cache size */
};

struct thread_context {
    int thread_no;
    pthread_t tid;
    pthread_attr_t  thread_attrs_detached;
    int bind_to_cpu;
    int tc_id;
    struct drand48_data buffer;
    long int random_pattern;
    long int seedval;
    unsigned int  prev_seed;
    unsigned int saved_seed;
    char dev_name[50];
    unsigned long long pattern;
    unsigned int current_oper;
    unsigned long long prefetch_scratch_mem[64/sizeof(unsigned long long)];
    int thread_type;                        /* Possible values PREFETCH, CACHE, ALL */
    int prefetch_algorithm;                    /* Used by prefetch threads to determine which prefetch algorithm to run*/

    int start_class;
    int end_class;
    int walk_class_jump;
    int offset_within_cache_line;
    int cache_instance_under_test;            /* The physical instance of cpu cache being tested by this thread*/

    int memory_starting_address_offset;        /* Offset of the starting address in memory segment that maps to
                                             * the cache instance being tested by this thread
                                             */

    char* start_of_contiguous_memory;        /* This is a pointer to the start address of contiguous memory that maps
                                             * the cache. This will be used as a starting point upon which offsets will
                                             * be added and memory address to read/write will be calculated
                                             */
    unsigned long long int sync_mask;
};

struct memory_set {
    char *seg_addr[NUM_SEGS];               /* shared memory EA */
    int shm_id[NUM_SEGS];                   /* shared memory ids arrary */
    int key[NUM_SEGS];                      /* shared memory keys array */
    unsigned long long real_addr[(SEG_SIZE*4)/(16*M)];
    /* shared memory RA arrary */
    char *ea[(SEG_SIZE*4)/(16*M)];         /* shared memory RA arrary  Maximum mem required in case of p7+ rollover, 128MB/core in worst case*/
    int num_pages;                        /* Num of 16M pages allocated */

    char *contig_mem[(SEG_SIZE*4)/(16*M)][MAX_CONTIG_PAGES];         /* Physically contiguous memory pointer */
    unsigned long long page_size;
    unsigned int threads_to_create;            /* holds number of threads to be created */
    unsigned int threads_created;
    unsigned int prefetch_threads_created;
    unsigned int contiguous_memory_size;     /* Size of conitguous memory */
    unsigned int memory_set_size;            /* Size of entire memory set */
    unsigned int index_pages[1024];            /* MAx pages */
};

struct sys_info {
    unsigned int      pvr;
    int               smt_threads;
    int               number_of_logical_cpus;
    int               start_cpu_number;                      /* Start cpu number for this instance       */
    int               end_cpu_number;                        /* End cpu number for this instance         */
    int               instance_number;                       /* Instance number                          */
    int               cores_in_instance[MAX_CORES_PER_CHIP]; /* List of core number available in chip    */
    int               num_cores;                             /* Number of cores in the instance          */
    SYS_STAT          sys_stat;                              /* Hardware statistics                      */
    CPU_MAP           cpu_in_chip_map;                       /* Map containing range of CPUS in instance */
    struct cache_info cinfo[2];                              /* Contains cache info for L2 and L3        */
};


struct ruleinfo {
    char rule_id[MAX_RULEID_LEN+1];         /* Test case name                                             */
    int test;                               /* L2 - 0, L3 - 1                                             */
    int bound;                              /* Yes - 1, No - 0                                            */
    int compare;                            /* Yes - 1, No - 0                                            */
    int data_width;                         /* 0-byte, 1-short, 2-word, 3-double, 4-randomize             */
    int target_set;                         /* Which set you want to target. -1 means all                 */
    int crash_on_misc;                      /* 0 - No, 1 - Yes, i.e. Crash to KDB on Miscompare           */
    int num_oper;                           /* 0 - infinite, +ve integer                                  */
    int seed;                               /* seed                                                       */
    int testcase_type;                      /* 0 - Cache Rollover, 1 - Cache Bounce                       */
    int gang_size;                          /* default = 8, for Equaliser = 1                             */
    int 		thread_sync;				/* Default = TRUE, used to enable/disable sync between threads	*/
    unsigned int prefetch_irritator;        /* Prefetch irritator on/off                                  */
    unsigned int prefetch_nstride;          /* Prefetch n-stride  on/off                                  */
    unsigned int prefetch_partial;          /* Prefetch partial  on/off                                   */
    unsigned int prefetch_transient;        /* Prefetch transient  on/off                                 */
    unsigned int prefetch_configuration;    /* Prefetch configuration, a unique state of a combination of */
                                            /* the above prefetch options on/off                          */
};

extern int Logical_cpus[MAX_THREADS];       /* Array of available logical processors                      */


#ifdef __HTX_LINUX__
#    include <linux/version.h>
#    include <asm/ioctl.h>
#    if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#        define IOCTL_NUM_IOWR
#    else
#        define IOCTL_NUM_IOWR_BAD
#    endif
#endif

#define MAXSYNCWORDS 20        /* This defines the size of the sync word array                 */
#define DISTANCE     10        /* This defines the distance between sync words to be updated   */
#ifndef GANGSIZE
#define GANGSIZE      8
#endif

/* Function declarations */
int               memory_set_cleanup();
int               get_cont_phy_mem(long long size);
int               write_mem(int index , int tid);
int               read_and_compare_mem(int index , int tid);
int               read_rule_file();
int               get_line(char *, FILE *);
int               parse_line(char *);
int               find_cache_details();
int               get_logical_to_physical_cpu(int);
int               create_threads(int thread_type);
int               derive_prefetch_algorithm_to_use(int index);
char *            find_required_contig_mem(unsigned int mem_req , unsigned int index);
void              setup_memory_set();
void              SIGTERM_hdl(int );
void              set_defaults();
void              write_read_and_compare(void *);
void              setup_thread_context_array(int thread_type);
void              cleanup_thread_context_array(int thread_type);
void              cleanup_threads(int thread_type);
long long         find_worst_case_memory_required();
long long         find_worst_case_prefetch_memory_required();
extern void       prefetch_irritator(void * arg);
int               get_next_cpu(int thread_type, int test_case, int thread_no, int core_no);

/* Assembly routines for the thread sync  */
extern void       set_sync_word(volatile unsigned long int *,int,int , int);                 /*     For setting the sync word to all 1s    */
extern void       wait_for(volatile unsigned long int *,int,int);                             /*      Wait till the sync word becomes 0      */
extern void       change(volatile unsigned long int *,unsigned long long int, int,int);        /*   Turn off thread's bit in the sync word */

unsigned char *   find_prefetch_mem();
unsigned long int get_random_number(int seg);
int 			  get_test_id(char *rule_id);
void			  synchronize_threads(int);

#ifndef __HTX_LINUX__
void				SIGCPUFAIL_handler(int);
#endif

#if defined(_DR_HTX_) && !defined(__HTX_LINUX__)
    void DR_handler(int sig, int code, struct sigcontext *scp);
#endif

