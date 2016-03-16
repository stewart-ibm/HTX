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
/* @(#)18       1.32  src/htx/usr/lpp/htx/bin/hxecache/hxecache_new.c, exer_cache, htxubuntu 11/3/11 00:51:40  */

#include "hxecache_new.h"


/* Global data structures */
struct ruleinfo          h_r[MAX_TC];
volatile struct ruleinfo *current_rule_stanza_pointer;

#ifdef __HTX_LINUX__
cpu_set_t default_cpu_affinity_mask;
#endif

/* crash_on_mis_global will be controlled by htxkdblevel shell variable.
 * This will have precedence over crash_on_misc specified in rulefile.
 */
int  crash_on_misc_global;
char *htxkdblevel;

/*
 * Value of prefetch_on variable will determine if prefetch irritator will
 * run along side hxecache.
 * It takes values 0 - Dont run prefetch irritator
 *                 1 - Run prefetch irritator
 * Reads the value provided in rule file.
 */

int                   shm_id,cache_page_req=0,prefetch_page_req=0;
int                   test_l2=0, test_l3=0, num_testcases;
int                   gang_size = GANG_SIZE;
char                  msg[1500];
unsigned int          cache_rollover_testcase, cache_bounce_testcase;
unsigned int          shm_size;
pthread_attr_t        thread_attr;
struct htx_data       h_d;
struct thread_context th_array[MAX_CPUS];

#if defined(_DR_HTX_) && !defined(__HTX_LINUX__)
    struct sigaction sigvector_dr;
    sigset_t mask;
#endif

struct memory_set mem;
struct sys_info   system_information;                                                                   /* Structure to store all system config related  info          */
struct sigaction  sigvector;

int                        start_of_prefetch_memory_index;                                              /* Index into the contiguous memory array, where               */
                                                                                                        /* prefetch memory starts                                      */
int                        instance_number;
int                        num_cache_threads, num_prefetch_threads, total_cache_threads;                /* Total no of cache threads & prefetch threads                */
char                       rule_file_name[100], device_name[50], run_type[3];
long long                  worst_case_memory_required, worst_case_prefetch_memory_required;             /* Largest (i.e worst case) memory required                    */
                                                                                                        /* for running all of the rule stanzas sequentially            */
unsigned int               worst_case_contig_mem;
volatile int               exit_flag = 0;                                                               /* flag to indicate if SIGTERM received                        */
volatile int               cleanup_done=0;                                                              /* Flag to indicate if mem cleanup is done                     */
volatile unsigned long int sync_word1[MAXSYNCWORDS],sync_word2[MAXSYNCWORDS],sync_word3[MAXSYNCWORDS];  /* Sync words for synchronising start and end of cache threads */
pthread_mutex_t mutex      = PTHREAD_MUTEX_INITIALIZER;

/*
 * Suggestions from Brad.
 *
 * 1. pattern length 8 bytes                - done
 * 2. Write at random offset in line            - done
 * 3. write/read/compare while bound to same cpu    - done
 * 4. down the line use different physical pages for memory block - later
 * 5. in the inner loop addr needs to generated first.    - done
 */

void init_sync_words(int shift_count) {                   /* This function is used to initialise all the sync words at once in the beginning */
    int i = 0;
    for( ; i<MAXSYNCWORDS ; i++ ) {
         set_sync_word(&sync_word1[i],shift_count,0,0);
         set_sync_word(&sync_word2[i],shift_count,0,0);
         set_sync_word(&sync_word3[i],shift_count,0,0);
    }
}


int main(int argc, char ** argv)
{
    int              rc, i, th_rc, dr_rc, j = 0, index;
    int              instance_number_units , instance_number_tens ,instance_number_hunds;
    char             cmd[100];
    void             *tresult = (void *)th_rc ;
    FILE             *fp;
	char			 current_rule_id[MAX_RULEID_LEN];
    htxsyscfg_smt_t  system_smt_information;
    htxsyscfg_cpus_t system_cpu_information;

    for(i=0 ; i< MAX_CPUS ; i++) {
        th_array[i].tid = -1;
        th_array[i].thread_type = ALL;
    }

    /*  Register SIGTERM handler */
    sigemptyset((&sigvector.sa_mask));
    sigvector.sa_flags   = 0;
    sigvector.sa_handler = (void (*)(int)) SIGTERM_hdl;
    sigaction(SIGTERM, &sigvector, (struct sigaction *) NULL);

    /*  Register DR handler */
    #if defined(_DR_HTX_) && !defined(__HTX_LINUX__)
        sigprocmask(SIG_UNBLOCK, NULL, &mask);
        sigaddset(&mask, SIGRECONFIG);
        sigprocmask (SIG_UNBLOCK, &mask , NULL);

        sigemptyset(&(sigvector_dr.sa_mask));
        sigvector_dr.sa_flags   = 0;
        sigvector_dr.sa_handler = (void (*)(int))DR_handler;
        dr_rc = sigaction(SIGRECONFIG, &sigvector_dr, (struct sigaction *) NULL);
        if (dr_rc != 0) {
            sprintf(msg,"sigaction failed(%d) for SIGRECONFIG\n",errno);
            hxfmsg(&h_d, 0, HTX_HE_HARD_ERROR, msg);
            exit(1);
        }
        sigprocmask(SIG_UNBLOCK, NULL, &mask);

        if (sigismember(&mask , SIGRECONFIG) == 1) {
            sprintf(msg," SIGRECONFIG signal is blocked\n");
            hxfmsg(&h_d, 0, HTX_HE_HARD_ERROR, msg);
            exit(1);
        }
    #endif

    /*  Parse command line arguments */
    #if defined(__HTX_LINUX__) && defined(AWAN)
        strcpy(device_name, "/dev/cache0");
        strcpy(run_type, "OTH");
        strcpy(rule_file_name, "../rules/reg/hxecache/default");
    #else
        strcpy(device_name, argv[1]);
        strcpy(run_type, argv[2]);
        strcpy(rule_file_name, argv[3]);
    #endif

    /*  Set the program behaviour in case of miscompare */
    htxkdblevel = getenv("HTXKDBLEVEL");
    if (htxkdblevel) {
        crash_on_misc_global = atoi(htxkdblevel);
    }
    else {
        crash_on_misc_global = 0;
    }

    /*  Set htx_data structure parameters */
    #if defined(__HTX_LINUX__) && defined(AWAN)
        strcpy(h_d.HE_name, "./hxecache");
        strcpy(h_d.sdev_id, "/dev/cache0");
        strcpy(h_d.run_type, "OTH");
    #else
        if(argv[0]) strcpy(h_d.HE_name, argv[0]);
        if(argv[1]) strcpy(h_d.sdev_id, argv[1]);
        if(argv[2]) strcpy(h_d.run_type, argv[2]);
    #endif

    hxfupdate(START, &h_d);
    sprintf(msg,"[%d] cycle no = %d\n",__LINE__,h_d.test_id);
    hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
    if(exit_flag ) {
		sprintf(msg,"[%d] Recieved exit_flag, exitting\n",__LINE__);
		hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
		if (exit_flag == DR_EXIT)
			hxfupdate(RECONFIG, &h_d);
		exit(0);
	}
   
    /* Get smt status & threads  */
    #if defined(__HTX_LINUX__) && defined(AWAN)
        system_information.smt_threads = 4;
    #else
        get_smt_details(&system_smt_information);

        /* Check if smt value returned is valid. If not then exit. */
        if(system_smt_information.smt_threads >= 1) {
            system_information.smt_threads = system_smt_information.smt_threads;
        }
        else {
            sprintf(msg,"Incorrect value of smt_threads obtained : %d, Exiting\n",system_smt_information.smt_threads);
            hxfmsg(&h_d, rc, HTX_HE_HARD_ERROR, msg);
            exit(1);
        }
    #endif

    #if defined(__HTX_LINUX__)
        CPU_ZERO(&default_cpu_affinity_mask) ;  /* Clear the mask */
        /* Save the default/original cpu affinity */
        rc = sched_getaffinity(syscall(SYS_gettid) , sizeof(default_cpu_affinity_mask), &default_cpu_affinity_mask ) ;
        if(rc == -1) {
            sprintf(msg, "sched_getaffinity failed  with errno: %d \n",errno);
            hxfmsg(&h_d, errno, HTX_HE_SOFT_ERROR, msg);
        }
    #endif

    DEBUGON("\n"); /* Newline before first print from exer */
    DEBUGON("system_information.smt_threads : %d\n",system_information.smt_threads);

    /* Get pvr information */
    #if defined(__HTX_LINUX__) && defined(AWAN)
        system_information.pvr = 0x4A;    /*Hardcode p7 value for AWAN*/
    #elif defined(__HTX_LINUX__) && !defined(AWAN)
        read_pvr(&system_information.pvr);
        system_information.pvr = (system_information.pvr)>>16 ;
    #else
        system_information.pvr = (unsigned int ) getPvr();
        system_information.pvr = (system_information.pvr)>>16 ;

    #endif

    DEBUGON("\n pvr : %x",system_information.pvr);

    /*
     * Collect cache releated information like:
     * Total L2/L3 cache size, L2/L3 line size, L2/L3 associativity
     * and fill cache_info data structure for L2 and L3
     */
    find_cache_details();

    /* Get the number of logical cpu's */
    #if defined(__HTX_LINUX__) && defined(AWAN)
        system_information.number_of_logical_cpus = 4;
    #else
        phy_logical_virt_cpus(&system_cpu_information);
        system_information.number_of_logical_cpus = system_cpu_information.l_cpus;
    #endif

    DEBUGON("system_information.number_of_logical_cpus are %d\n",system_information.number_of_logical_cpus) ;

    /* Read rule file information */
    rc = read_rule_file();
    if(rc != 0) exit(1);

    /* Find the instance of cache running
     * 0th instance runs on processor 0
     * 1st instance runs on processor 8 [smt is off]
     *            processor 16 [smt = 2   ]
     *            processor 32 [smt = 4    ]
     * so on in multiplies of 8 or 16 or 32 depending SMT
     */

    if(h_d.sdev_id) {
        char *ptr = h_d.sdev_id;
        i = 0;
        instance_number = 0;
        while (i < strlen(h_d.sdev_id)) {
            if(*(ptr + i) >= '0' && *(ptr + i) <= '9') {
                instance_number = instance_number*10 + *(ptr + i) - '0';
            }
            i++;
        }
    }
    else instance_number = 0 ; /*By default bind to first proc */
    
    if ( ((instance_number + 1) * gang_size) <= (system_information.number_of_logical_cpus / system_information.smt_threads) ) {
        num_cache_threads = gang_size;
    }
    else {    /* If its the last */
        num_cache_threads = (system_information.number_of_logical_cpus / system_information.smt_threads) % gang_size;
    }
    
    if(gang_size > 1) {
        system_information.start_cpu_number = instance_number * gang_size * system_information.smt_threads;
        system_information.end_cpu_number   = system_information.start_cpu_number + num_cache_threads * system_information.smt_threads - 1;
    }
    else { /* If gang size = 1, For Equaliser support */
        system_information.start_cpu_number = instance_number;
        system_information.end_cpu_number   = instance_number;
    }

    DEBUGON("Instance_Number                = %d \n" ,instance_number);
    DEBUGON("Total number of cpus           : %d \n", system_information.number_of_logical_cpus);
    DEBUGON("system_information.smt_threads : %d \n",system_information.smt_threads);
    DEBUGON("num_cache_threads              : %d \n",num_cache_threads);
    DEBUGON("Start CPU number               : %d \n",system_information.start_cpu_number);
    DEBUGON("End CPU number                 : %d \n",system_information.end_cpu_number);

    /* Memory allocation: Parse thru the rule file parameters to find the various test cases to be executed.
     * Allocate worst case memory required by the exerciser. This is one time job.
     */
    do{
        /* Find out the worst case memory requirement for this instance of hxecache */
        worst_case_memory_required = find_worst_case_memory_required();

        /* Find out the worst case prefetch memory requirement for this instance of hxecache */
        worst_case_prefetch_memory_required = find_worst_case_prefetch_memory_required();

        DEBUGON("worst_case_memory_required          = %llx \n", worst_case_memory_required);
        DEBUGON("worst_case_contig_mem               = %llx \n", worst_case_contig_mem);
        DEBUGON("worst_case_prefetch_memory_required = %llx \n", worst_case_prefetch_memory_required);

        /* Request for that much memory */
        rc = get_cont_phy_mem(worst_case_memory_required + worst_case_prefetch_memory_required);

        /* If no continguous memory found (i.e rc = -2) AND if any test case is L3 */
        if(rc == -2 && test_l3) {
            sprintf(msg,"Could not acquire 0x%x (cache) Bytes physically contig mem. This is not an error.  " \
                    "In place of L3, L2 test will run.\n", worst_case_contig_mem);
            hxfmsg(&h_d, rc, HTX_HE_INFO, msg);

            /* Setting all L3 tests to L2 tests.  */
            for(i = 0; i < num_testcases; i++) {
                h_r[i].test = L2;
            }
        }

        /* If no continguous memory found (i.e rc = -2) AND if all testcases are L2 */
        else if (rc == -2 && test_l2){
            sprintf(msg,"Could not acquire 0x%x (cache) Bytes physically contig mem. This is not an error." \
                    " Exerciser will Exit.\n", worst_case_contig_mem);
            hxfmsg(&h_d, rc, HTX_HE_HARD_ERROR, msg);
            exit(1);
        }

        /* If memory allocation itself fails */
        else if(rc == -1) {
            sprintf(msg, "Memory allocation failures. Exiting !! \n");
            hxfmsg(&h_d, rc, HTX_HE_HARD_ERROR, msg);
            exit(1);
        }

    }while (rc != 0);

    do {
        int tc, rc,old_prefetch_configuration=0,k;

        for(tc = 0; tc < num_testcases; tc++) {
            int index;
            mem.threads_created = 0;

            /* Set the current rule stanza pointer */
            current_rule_stanza_pointer = &h_r[tc];

            DEBUGON("Running Testcase number %d ***\n",tc+1);
            DEBUGON("Rule Parameters ***************\n");
            DEBUGON("Rule Id %s \n",current_rule_stanza_pointer->rule_id);
            DEBUGON("Test %d \n",current_rule_stanza_pointer->test);
            DEBUGON("Bound %d \n",current_rule_stanza_pointer->bound);
            DEBUGON("Compare %d \n",current_rule_stanza_pointer->compare);
            DEBUGON("Data width %d \n",current_rule_stanza_pointer->data_width);
            DEBUGON("Target Set %d \n",current_rule_stanza_pointer->target_set);
            DEBUGON("Crash on misc %d \n",current_rule_stanza_pointer->crash_on_misc);
            DEBUGON("Num oper %d \n",current_rule_stanza_pointer->num_oper);
            DEBUGON("Prefetch irritator %d \n",current_rule_stanza_pointer->prefetch_irritator);
            DEBUGON("Prefetch nstride %d \n",current_rule_stanza_pointer->prefetch_nstride);
            DEBUGON("Prefetch partial %d \n",current_rule_stanza_pointer->prefetch_partial);
            DEBUGON("Prefetch transient %d \n",current_rule_stanza_pointer->prefetch_transient);
            DEBUGON("Prefetch configuration %x \n",current_rule_stanza_pointer->prefetch_configuration);
            DEBUGON("Seed %d \n",current_rule_stanza_pointer->seed);
            DEBUGON("Testcase Type %d \n",current_rule_stanza_pointer->testcase_type);
            DEBUGON("Gang Size %d \n",current_rule_stanza_pointer->gang_size);
            DEBUGON("*******************************\n");

			memcpy(current_rule_id,current_rule_stanza_pointer->rule_id,MAX_RULEID_LEN);

            /* Create prefetch irritator threads here if required */
            /* Prefetch threads need to be created from scratch if:
             * 1. The prefetch_configuration has changed from the last rule stanza i.e change in combinations of
             *    prefetch algorithms. ==AND==
             * 2. prefetch_configuration not being set to disable prefetch threads
             */

            if( current_rule_stanza_pointer->prefetch_configuration != old_prefetch_configuration &&
                current_rule_stanza_pointer->prefetch_configuration != 0 ) {

                for(k=0;k<mem.num_pages;k++) {
                    if (mem.index_pages[k] != CACHE_PAGE)
                        mem.index_pages[k] = PAGE_FREE;
                }

                /* Calculate number of Prefetch threads to create */
                if(current_rule_stanza_pointer->testcase_type == CACHE_TEST_OFF) {
                    num_prefetch_threads = num_cache_threads * system_information.smt_threads;
                }
                else {
                    num_prefetch_threads = num_cache_threads * (system_information.smt_threads - 1);
                }

                /* Setup thread context for prefetch threads */
                setup_thread_context_array(PREFETCH);

                /* If create threads fails, exerciser cannot continue, therefore exit */
                if(create_threads(PREFETCH) == -1) {
                    exit(1);
                }
            }
    
            mem.threads_to_create = num_cache_threads;

            /* Create Cache threads here, if required
             * Only if cache testcase is disabled (CACHE_TEST_OFF) then we skip creation
             * of cache threads.
             */
            if( current_rule_stanza_pointer->testcase_type != CACHE_TEST_OFF ) {
		printf("test_case = %d\n",current_rule_stanza_pointer->testcase_type);
                setup_thread_context_array(CACHE);
                /* If create threads fails, exerciser cannot continue, therefore exit */
                if(create_threads(CACHE) == -1) {
                    exit(1);
                }
            }

            /* Initialise read/write stats */
            /* Moved the stanza updation logic to between thread creation and thread join */
            h_d.good_reads  = 0;
            h_d.good_writes = 0;
            /*h_d.test_id     = (tc + 1);*/
			h_d.test_id     = get_test_id(current_rule_id);
            hxfupdate(UPDATE, &h_d);
           
            sprintf(msg,"[%d] cycle no = %d\n",__LINE__,h_d.test_id);
            hxfmsg(&h_d, 0, HTX_HE_INFO, msg);

            /* If Cache testcase is disabled ,then there is no need to clean up cache threads
             * as they were not created in this rule stanza pass.
             */
            if( current_rule_stanza_pointer->testcase_type != CACHE_TEST_OFF ) {

                /* Cleanup Cache threads here */
                printf("[%d] calling cleanup cache \n",__LINE__);
                cleanup_threads(CACHE);
                cleanup_thread_context_array(CACHE);
            }

            /* Cleanup Prefetch threads here if required*/

            /* If prefetch threads are currently running */
            if(current_rule_stanza_pointer->prefetch_configuration != 0 ) {

                /* Check if the next stanza has the same prefetch configuration */
                if( /* If not the last rule stanza AND If next stanza prefetch configuration is not the same */
                   (tc != num_testcases-1
                    &&    current_rule_stanza_pointer->prefetch_configuration != h_r[tc+1].prefetch_configuration)
                   || /* OR If it is the last rule stanza*/
                   (tc == num_testcases-1)
                   ||/* OR */
                   (tc != num_testcases-1 /* If not last rule stanza AND If testcase type is changed in the next stanza */
                    && current_rule_stanza_pointer->testcase_type != h_r[tc+1].testcase_type)) {

                    /* Then cleanup */
                    sprintf(msg,"[%d] cleaning up prefetch threads\n",__LINE__);
                    hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
                    cleanup_threads(PREFETCH);
                    cleanup_thread_context_array(PREFETCH);
                }
            }

            /* Save the prefetch configuration */
            old_prefetch_configuration = current_rule_stanza_pointer->prefetch_configuration;

            /* Update HTX stats here. */
            if( current_rule_stanza_pointer->compare) {

                /* Good reads and writes */
                h_d.good_reads  = current_rule_stanza_pointer->num_oper;
                h_d.good_writes = current_rule_stanza_pointer->num_oper;

                if(current_rule_stanza_pointer->data_width != 4) {

                    int data_width_in_bytes,number_of_writes_per_num_oper,number_of_reads_per_num_oper;

                    switch(current_rule_stanza_pointer->data_width ) {

                    case 0:
                        /* byte */
                        data_width_in_bytes = sizeof(char);
                        break;

                    case 1:
                        /* short */
                        data_width_in_bytes = sizeof(short);
                        break;

                    case 2:
                        /* int */
                        data_width_in_bytes = sizeof(int);
                        break;

                    case 3:
                        /* long */
                        data_width_in_bytes = sizeof(long);
                        break;

                    default:
                        break;

                    }

                    if (current_rule_stanza_pointer->testcase_type == CACHE_BOUNCE) {
                        number_of_writes_per_num_oper = current_rule_stanza_pointer->gang_size *            /* number of writes per line x number of lines     */
                            (2 * system_information.cinfo[current_rule_stanza_pointer->test].cache_size/
                             system_information.cinfo[current_rule_stanza_pointer->test].line_size);
                        number_of_reads_per_num_oper = current_rule_stanza_pointer->gang_size *                /* number of reads per line x number of lines     */
                            (2* system_information.cinfo[current_rule_stanza_pointer->test].cache_size/
                             system_information.cinfo[current_rule_stanza_pointer->test].line_size);

                    }
                    else if (current_rule_stanza_pointer->testcase_type == CACHE_ROLLOVER) {                /* For rollover, only one write location is     */
                                                                                                            /* touched per cache line                         */
                        number_of_writes_per_num_oper = 1 *                                                    /* number of writes per line * number of lines     */
                            (2 * system_information.cinfo[current_rule_stanza_pointer->test].cache_size/
                             system_information.cinfo[current_rule_stanza_pointer->test].line_size);
                        number_of_reads_per_num_oper =  1 *                                                    /* number of reads per line * number of lines     */
                            (2* system_information.cinfo[current_rule_stanza_pointer->test].cache_size/
                             system_information.cinfo[current_rule_stanza_pointer->test].line_size);
                    }

                    h_d.bytes_writ = h_d.good_reads * number_of_writes_per_num_oper * data_width_in_bytes ;
                    h_d.bytes_read = h_d.good_reads * number_of_writes_per_num_oper * data_width_in_bytes ;
                }
            }
            else {
                h_d.good_writes = current_rule_stanza_pointer->num_oper;                                    /* Good writes only */
            }
            hxfupdate(UPDATE, &h_d);
            sprintf(msg,"[%d] cycle no = %d\n",__LINE__,h_d.test_id);
            hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
        }
        if(exit_flag) {
			sprintf(msg,"[%d] Recieved exit_flag, exitting\n",__LINE__);
			hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
			if (exit_flag == DR_EXIT)
				hxfupdate(RECONFIG, &h_d);
            break;
		}

		h_d.test_id = 0;
        hxfupdate(FINISH, &h_d);                                                                            /* hxfupdate call with FINISH arg indicates one */
                                                                                                            /* more rule file pass done                     */
    } while( (rc = strcmp(run_type, "REG") == 0) || (rc = strcmp(run_type, "EMC") == 0) );

    if(exit_flag){
        cleanup_threads(ALL);
    }

    hxfmsg(&h_d,rc ,  HTX_HE_INFO  , "Exiting\n");
    memory_set_cleanup();
    hxfmsg(&h_d, rc , HTX_HE_INFO  , "Memory Cleanup\n");
    return 0;
}

void SIGTERM_hdl(int sig)
{
    int i , rc ;

    hxfmsg(&h_d, 0, HTX_HE_INFO, "sigterm received\n");

    exit_flag = NORMAL_EXIT ;

	sprintf(msg,"[%d] Exitting %s\n",__LINE__,__FUNCTION__);
	hxfmsg(&h_d,0,HTX_HE_INFO,msg);

}

int find_cache_details()
{

#if defined(__HTX_LINUX__) && defined(AWAN)                                                    /* Hardcoding p7+ values for AWAN */

    /* Assign L2 cache information */
    system_information.cinfo[L2].line_size  = 128;
    system_information.cinfo[L2].asc        = 8;
    system_information.cinfo[L2].cache_size = 256*1024;

    /* Assign L3 cache information */
    system_information.cinfo[L3].line_size  = 128;
    system_information.cinfo[L3].asc        = 8;
    system_information.cinfo[L3].cache_size = 10*1024*1024;

#elif !defined(AWAN)

    htxsyscfg_cache_t cpu_cache_information;

    /* Fetch cache information */
#    ifdef __HTX_LINUX__
        L2L3cache( &cpu_cache_information);
#    else
        L2cache( &cpu_cache_information);
        L3cache( &cpu_cache_information);
#    endif

    /* Assign L2 cache information */
    system_information.cinfo[L2].line_size  =  cpu_cache_information.L2_line;
    system_information.cinfo[L2].asc        =  cpu_cache_information.L2_asc;
    system_information.cinfo[L2].cache_size =  cpu_cache_information.L2_size;

    /* Assign L3 cache information */
    system_information.cinfo[L3].line_size  =  cpu_cache_information.L3_line;
    system_information.cinfo[L3].asc        =  cpu_cache_information.L3_asc;
    system_information.cinfo[L3].cache_size =  cpu_cache_information.L3_size;

#endif

    sprintf(msg, "CACHE Details: \t L2 \t\t L3    \
        \nLine size \t %d \t\t %d         \
        \nAssociativity \t %d \t\t %d        \
        \nCache size \t %lx \t\t %lx\n"
            , system_information.cinfo[0].line_size, system_information.cinfo[1].line_size,
            system_information.cinfo[0].asc, system_information.cinfo[1].asc,
            system_information.cinfo[0].cache_size, system_information.cinfo[1].cache_size);
    hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
}

/* This function is meant to setup contiguous memory pointers for use by threads to
 * read/write to later on.
 * It does 2 things depending on the pvr:
 * 1. If P7 then this function will just setup the mem.contig_mem[] pointers to 16M
 * page boundaries.
 * 2. If P6/P7+ then it will sort the memory set in physically contiguous order.
 */
void setup_memory_set()
{
    int                i, j, min_index = -1;
    char               *min_ea;
    unsigned long long min, tmp;

    if(system_information.pvr == 0x3E || system_information.pvr == 0x4A) {
        /* If P6/p7+, then threads need to operate on memory spanning 4/2 physically contiguous
         * 16M pages. Therefore an actual sorting of these address w.r.t the real addresses
         * needs to be done.
         */
        for(i = 0; i < (mem.num_pages - 1); i++) {
            min = mem.real_addr[i];
            min_index = -1;
            for(j = i + 1; j < mem.num_pages; j++) {
                if(mem.real_addr[j] < min) {
                    min       = mem.real_addr[j];
                    min_ea    = mem.ea[j];
                    min_index = j;
                    DEBUGON("swapping %d with %d\n",i,min_index);
                }
            }
            if(min_index != -1) {
                tmp                      = mem.real_addr[i];                                            /* swap real_addr array */
                mem.real_addr[i]         = min;
                mem.real_addr[min_index] = tmp;

                DEBUGON("swapping EA[%d]=%x with EA[%d]=%x\n",i,mem.ea[i],min_index,mem.ea[min_index]);

                tmp               = (unsigned long long)mem.ea[i];                                        /* swap effective addr array */
                mem.ea[i]         = min_ea;
                mem.ea[min_index] = (char *)tmp;
            }
        }
    }
    else if(system_information.pvr == 0x3F) {
        /* If P7, then threads need to operate within 16M page boundaries,
         * therefore an actual sorting w.r.t real addresses is not required.
         * We only need to set the mem.contig_mem[] pointers appropriately,
         * such that each smaller gang of cache threads (i.e that which are
         * operating on the same physical cache instance), read/write
         * in their respective area's
         */
        for(i=0; i<(mem.memory_set_size/(16*M)); i++) {
            mem.contig_mem[i][0] = mem.seg_addr[0] + i*16*M;
        }
    }
}

/* This is called for p6 and p7+ only */
char * find_required_contig_mem(unsigned int mem_req, unsigned int index)
{
    int          save_index = -1, j, k;
    unsigned int i, contig_mem_size = 16*M;

    DEBUGON("%s: mem_req = %lx\n",__FUNCTION__, mem_req);
    if(mem_req <= 16*M) {
        mem.contig_mem[0][0] = mem.ea[0];
        return(mem.ea[0]);
    }
    for(i = 0; i < (mem.num_pages - 1); i++) {
        if (((mem.real_addr[i + 1] - mem.real_addr[i]) == 16*M) && ((mem.index_pages[i] == PAGE_FREE) && (mem.index_pages[i+1] == PAGE_FREE)))  {
            contig_mem_size += 16*M;
            if(contig_mem_size == 32*M) save_index = i;
            if(contig_mem_size >= mem_req) {
                for(j = 0, k = save_index; k <= (i+1) ; j++, k++) {
                    mem.contig_mem[index][j] = mem.ea[k];
                    mem.index_pages[k]       = CACHE_PAGE;
                    DEBUGON("%s: %lx\n", __FUNCTION__, mem.contig_mem[index][j]);
                }
                DEBUGON("%s: got memory ! EA = %x \n", __FUNCTION__, mem.ea[save_index]);
                #ifdef DEBUG
                for ( j = 0; j<mem.num_pages - 1;j++)
                    printf("\n ea: %x index_pages :%x",mem.ea[j],mem.index_pages[j]);
                #endif
                return(mem.ea[save_index]);
            }
        }
        else {
            contig_mem_size = 16*M;
            save_index      = -1;
        }
    }
    return(NULL);
}


unsigned char* find_prefetch_mem()
{

    /* mem.index_pages array is initialised to zero.
     * If allocated to cache   value  is 1
     * If 8MB is allocated to prefetch value is 2
     * If whole page is allocated value would be 3
     */

    int      i, j, k;
    int      index    = 0;
    unsigned ret_page = 0;

    #ifdef DEBUG
    for (j =0 ;j< mem.num_pages;j++) {
        printf("mem_addr:index:%d addr:%llx index: %d",j,mem.ea[j],mem.index_pages[j]);
    }
    #endif

    for (i = index; i < mem.num_pages; i++) {
	printf("num_pages = %d\n",mem.num_pages);
	printf("page [%d] status = %d\n",i,mem.index_pages[i]);
        switch ( mem.index_pages[i])
        {
            case PAGE_FREE: mem.index_pages[i] = PAGE_HALF_FREE;
                            DEBUGON("\n  %d %d %llx ",i,mem.index_pages[i],mem.ea[i]);
                            return mem.ea[i];

            case PAGE_HALF_FREE: mem.index_pages[i] = PREFETCH_PAGE;
                                 DEBUGON("\n 1. %d %d %llx ",i,mem.index_pages[i],mem.ea[i]);
                                 return (mem.ea[i] + PREFETCH_MEM_SZ);

            default:break;
        }
    }

    /* No pages available then pass the address of the first prefetch page */
    /* can this happen */
}

int memory_set_cleanup()
{
    int rc, i;

    sprintf(msg,"[%d] %s called \n",__LINE__,__FUNCTION__);
    hxfmsg(&h_d, rc, HTX_HE_INFO, msg);
    if( cleanup_done == 1 ) {
        sprintf(msg,"[%d] %s : Cleanup already done. Exitting \n",__LINE__,__FUNCTION__);
        hxfmsg(&h_d, rc, HTX_HE_INFO, msg);
        return 0;
    }   
    for(i = 0; i < NUM_SEGS; i++) {
        if( NULL != mem.seg_addr[i]) {
            rc = shmdt(mem.seg_addr[i]);
            if(rc != 0) {
                sprintf(msg,"%s: shmdt failed with %d\n",__FUNCTION__, errno);
                hxfmsg(&h_d, rc, HTX_HE_HARD_ERROR, msg);
                return(-1);
            }

#ifdef __HTX_LINUX__
            rc = shmctl(mem.shm_id[i], SHM_UNLOCK, (struct shmid_ds *) NULL);
            if(rc != 0) {
                sprintf(msg,"%s: shmctl for SHM_UNLOCK failed with %d\n",__FUNCTION__, errno);
                hxfmsg(&h_d, rc, HTX_HE_HARD_ERROR, msg);
                return(-1);
            }
#endif
            rc = shmctl(mem.shm_id[i], IPC_RMID, (struct shmid_ds *) NULL);
            if(rc != 0) {
                sprintf(msg,"%s: shmctl for IPC_RMID failed with %d\n",__FUNCTION__, errno);
                hxfmsg(&h_d, rc, HTX_HE_HARD_ERROR, msg);
                return(-1);
            }
        }
    }
    cleanup_done = 1;
    return(0);
}

int get_cont_phy_mem(long long size_requested)
{
    int                    i=0, rc, seg, j, k,ret;
    char                   *p, *addr;
    key_t                  key;
    uint32_t               memflg;
    struct shmid_ds        shm_buf = { 0 };
    unsigned long long     current_shmmax=0,new_shmmax = 0;
    char                   shmval[50];
    FILE                   *fp = 0,*testfp=0;

#ifdef SET_PAGESIZE
    struct vminfo_psize vminfo_64k = { 0 };
    psize_t             psize      = 16*M;
#else
    unsigned long long psize = 16*M;
#endif

    /* Only ONE 16M page required in case we're running on P7 */
    /* Set shm_size based on whether P6 or P7 or P7+ */
    if(size_requested < 16*M) {
        /* If required size is < one 16M page ,then get one 16M page*/
        shm_size = 16*M;
    }
    else {
        /* Calculate required number of 16M pages to span this memory requirement
         * and request for that much memory
         */
        shm_size =  size_requested;

        if (size_requested%(16*M) != 0) {
            shm_size += ( (16*M) - (size_requested%(16*M)) ) ;  /* round off to the next highest 16M page */
        }
    }


    if ((system_information.pvr == 0x3F ) || ( system_information.pvr == 0x4A && test_l2 )) {
        /* If P7 or P7+ L2 test case  then contiguous memory requirements falls within 16M page boundaries. Therefore
         * here we dont need the entire "size" to be truly contiguous (in physical memory), only that
         * it be allocated in distinct 16M pages, so that threads operating on each physical cache
         * instance can operate within the page boundary assigned to them.
         */

        /* Set index number for start of exclusive prefetch memory */
        start_of_prefetch_memory_index = 1;
        /* Mark pages as used */
        for (k=0; k < start_of_prefetch_memory_index; k++)
            mem.index_pages[k] = CACHE_PAGE;
    }
    else {
        /* If P6, then contiguous memory requirements are beyond one 16M page boundaries. Therefore
         * here we require "size" to be truly contiguous (in physical memory). Therefore we allocate
         * a large enough segment (SEG_SIZE), within which a contiguous block of "size" can be found.
         */

        /* Set index number for start of exclusive prefetch memory */
        start_of_prefetch_memory_index = 0;
    }

    memflg = (IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

#if defined(__HTX_LINUX__)
    memflg |= (SHM_HUGETLB );
#endif

    #ifdef __HTX_LINUX__
   
    fp = popen("cat /proc/sys/kernel/shmmax","r");
    if ( (fp == NULL) || (fp == -1) ) {
        sprintf(msg,"[%d] Failed opening /proc/sys/kernel/shmmax with %d\n ",__LINE__,errno);
        hxfmsg(&h_d, rc, HTX_HE_HARD_ERROR, msg);
        return (-1);
    }
    else {
        fscanf(fp,"%s",shmval);
        current_shmmax = strtoull(shmval,NULL,10);
        sprintf(msg,"Current SHMMAX val = %llu and shm_size = %llu\n",current_shmmax,shm_size);
        hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
        pclose(fp);

        if(current_shmmax < shm_size) {
            sprintf(msg,"[%d] Increasing SHMMAX value to %llu\n ",__LINE__,(shm_size+current_shmmax));
            hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
            fp = fopen("/proc/sys/kernel/shmmax","w");
            if ( (fp == NULL) || (fp == -1) ) {
                sprintf(msg,"[%d] Failed opening /proc/sys/kernel/shmmax with %d\n ",__LINE__,errno);
                hxfmsg(&h_d, rc, HTX_HE_HARD_ERROR, msg);
                return (-1);
            }
            else {
                new_shmmax = (current_shmmax + shm_size);
                fprintf(fp,"%llu",new_shmmax);
                fclose(fp);
            }
      
            /* Verify SHM size has changed */ 
            fp = popen("cat /proc/sys/kernel/shmmax","r");
            if ( (fp == NULL) || (fp == -1) ) {
                sprintf(msg,"[%d] Failed opening /proc/sys/kernel/shmmax with %d\n ",__LINE__,errno);
                hxfmsg(&h_d, rc, HTX_HE_HARD_ERROR, msg);
                return (-1);
            }
            else {
                new_shmmax = 0;
                fscanf(fp,"%s",&shmval);
                new_shmmax = strtoull(shmval,NULL,10);   
                sprintf(msg,"New SHMMAX val = %llu and shm_size = %llu\n",new_shmmax,shm_size);
                hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
                pclose(fp);
                if( new_shmmax != (shm_size+current_shmmax)) {
                    sprintf(msg,"[%d] Failed to change SHMMAX value\n ",__LINE__);
                    hxfmsg(&h_d, 1, HTX_HE_HARD_ERROR, msg);
                    return -1;
                }
            }
        }
    }
    #endif

    for(seg = 0; seg < NUM_SEGS; seg ++) {
        mem.key[seg] = ftok(rule_file_name, instance_number);
        /*        putenv("MEMORY_AFFINITY=MCM");*/
        /* The above env var gets set in src/htx/usr/lpp/htx/.htxrc */
        mem.shm_id[seg] = shmget(mem.key[seg]+seg, shm_size, memflg);
        if(mem.shm_id[seg] == -1) {
            memory_set_cleanup();
            sprintf(msg,"[%d] shmget failed with %d while allocating seg size = %lu!\n",__LINE__, errno,shm_size);
            hxfmsg(&h_d, rc, HTX_HE_HARD_ERROR, msg);
            return(-1);
        }

#if !defined(SET_PAGESIZE) && defined(__HTX_LINUX__)
        /* Using SHM_LOCK (Linux Specific) flag to lock the shm area into memory */
        if ((rc = shmctl(mem.shm_id[seg], SHM_LOCK, &shm_buf)) == -1)   {
            memory_set_cleanup();
            sprintf(msg,"shmctl failed to get minimum alignment requirement - %d\n", errno);
            hxfmsg(&h_d, rc, HTX_HE_HARD_ERROR, msg);
            return(-1);
        }
#endif

#if defined(SET_PAGESIZE) && !defined(__HTX_LINUX__)
        if ( shm_size > 256*M) {
            if ((rc = shmctl(mem.shm_id[seg], SHM_GETLBA, &shm_buf)) == -1)   {
                memory_set_cleanup();
                sprintf(msg,"shmctl failed to get minimum alignment requirement - %d\n", errno);
                hxfmsg(&h_d, rc, HTX_HE_HARD_ERROR, msg);
                return(-1);
            }
        }
        shm_buf.shm_pagesize = psize;
        if((rc = shmctl(mem.shm_id[seg], SHM_PAGESIZE, &shm_buf)) == -1) {
            memory_set_cleanup();
            sprintf(msg,"shmctl failed with %d while setting page size.\n",errno);
            hxfmsg(&h_d, rc, HTX_HE_HARD_ERROR, msg);
            return(-1);
        }
#endif

        addr = (char *) shmat(mem.shm_id[seg], 0, 0);
        if(-1 == (int)addr) {
            sprintf(msg,"shmat failed with %d\n",errno);
            hxfmsg(&h_d, -1, HTX_HE_HARD_ERROR, msg);
            memory_set_cleanup();
            return(-1);
        }
        mem.seg_addr[seg] = addr;

#ifdef SET_PAGESIZE
        rc = mlock(addr, shm_size);
        if(rc == -1) {
            sprintf(msg,"mlock failed with %d \n", errno);
            hxfmsg(&h_d, rc, HTX_HE_HARD_ERROR, msg);
            memory_set_cleanup();
            return(-1);
        }
#endif
        i = 0;
        if ((system_information.pvr == 0x3f)) {
            for(p=addr; p < addr+shm_size; p+=16*M) {
                printf("\n p : %llx",p);
                mem.ea[i] = p;
                printf("mem.ea[i] = %llx",mem.ea[i]);
                i++;
            }
            mem.num_pages = i;
        }

        /* Size of the total memory set, i.e size of memory requested from shmget() */
        mem.memory_set_size = shm_size;

        /* Check if its p7+ and l2 test case , then the sorting can be omitted */
        if ((system_information.pvr == 0x3E) || ((system_information.pvr == 0x4A) && test_l3 ))  {
            /* If P6, P7+ with test_l3  only then populate real address fields in mem structure, so that
             * they may be sorted in a physcially contiguous order later on
             */
            i = 0;
            for(p=addr; p < addr+shm_size; p+=16*M) {
                mem.ea[i] = p;
#ifdef __HTX_LINUX__
                get_real_address(p, &mem.real_addr[i]);
#else
                getRealAddress(p, 0, &mem.real_addr[i]);
#endif
                DEBUGON("EA - %x RA - %llx\n", mem.ea[i], mem.real_addr[i]);
                printf("EA - %x RA - %llx\n", mem.ea[i], mem.real_addr[i]);
                i++;
                mem.num_pages = i;
            }
        }

        /* If P7 then this function will just setup the mem.contig_mem[] pointers to 16M page
         * boundaries. If P6 /P7+ then it will sort the memory set in physically contiguous order.
         */
        setup_memory_set();

        if (system_information.pvr == 0x3E ) {
            /* If P6 then find the required physcially contiguous block of memory,
             * within the array of sorted address.
             */
            #if DEBUG
            for(i=0; i< mem.num_pages; i++) {
                DEBUGON("%s: EA = %x RA = %llx \n",__FUNCTION__, mem.ea[i], mem.real_addr[i]);
            }
            #endif

            mem.contig_mem[0][0] = find_required_contig_mem(worst_case_contig_mem, 0);
            DEBUGON("EA of physically contig mem = %lx \n", mem.contig_mem[0][0]);
            if(mem.contig_mem[0][0] == NULL) {
                memory_set_cleanup();
                return(-2);
            }
            mem.contiguous_memory_size = worst_case_contig_mem;
        }
        else if (system_information.pvr == 0x3F) {
            /* If P7, the contiguous memory is only within, 16M page boundaries */
            mem.contiguous_memory_size = 16*M;
        }
        else { /* p7+ */
            if (test_l2)
                mem.contiguous_memory_size = 16*M;
            else {
                /* Add check if p7+ and l2 cache, then no contig mem required
                if l3 and cache bounce, only one set of contig mem is required
                if l3 and cache roll over, gang size contig me required */
                mem.contiguous_memory_size = 32*M;
                for ( i= 0 ; i< num_cache_threads ; i++) {
                    mem.contig_mem[i][0] = find_required_contig_mem(worst_case_contig_mem, i );
                    DEBUGON("EA of physically contig mem = %lx \n", mem.contig_mem[i][0]);

                    if(mem.contig_mem[i] == NULL) {
                        memory_set_cleanup();
                        return(-2);
                    }
                }
            }
        } /* end of else for p7+ */
    }
    return(0);
}

void write_byte(void *addr, int index ,int tid)
{
    char *ptr     = (char *)addr;
    char *pat_ptr = (char *)&th_array[index].pattern;

    *ptr = *(char *)pat_ptr;

#if 0
    DEBUGON("1:%lx:%llx:%d\n",addr,(*((unsigned long long *)addr)), tid);
#endif
}

void write_short(void *addr, int index ,int tid)
{
    char           *pat_ptr = (char *)&th_array[index].pattern;
    unsigned short *ptr     = (unsigned short *)addr;

    *ptr = *(unsigned short *)pat_ptr;

#if 0
    DEBUGON("2:%lx:%llx:%d\n",addr, *(unsigned long long *)addr, tid);
#endif
}

void write_int(void *addr,int index , int tid)
{
    char         *pat_ptr = (char *)&th_array[index].pattern;
    unsigned int *ptr     = (unsigned int *)addr;

    *ptr = *(unsigned int *)pat_ptr;

#if 0
    DEBUGON("4:%lx:%llx:%d\n",addr,*(unsigned long long *)addr, tid);
#endif
}

void write_ull(void *addr,int index , int tid)
{
    char               *pat_ptr = (char *)&th_array[index].pattern;
    unsigned long long *ptr     = (unsigned long long *)addr;

    *ptr = *(unsigned long long *)pat_ptr;

#if 0
    DEBUGON("8:%lx:%llx:%d\n",addr,*(unsigned long long *)addr, tid);
#endif
}

int write_mem(int index , int tid)
{
    int          walk_class, walk_line, memory_per_set, lines_per_set, ct;
    int          data_width,rc;
    char         *addr = mem.contig_mem[0][0];
    void         (*fptr[10]) (void *, int , int);
    unsigned int random_no, mem_per_thread, offset1, offset2;

    fptr[0] = &write_byte;
    fptr[1] = &write_short;
    fptr[2] = &write_int;
    fptr[3] = &write_ull;

    /*
     * cache_type decides whether calling write_mem for L2 or L3.
     */
    ct                = current_rule_stanza_pointer->test;
    data_width     = current_rule_stanza_pointer->data_width;
    memory_per_set = system_information.cinfo[ct].cache_size/system_information.cinfo[ct].asc;
    lines_per_set  = memory_per_set/system_information.cinfo[ct].line_size;

    DEBUGON("memory_per_set = 0x%lx \t lines_per_set = %d\n",
            memory_per_set, lines_per_set);

    if (current_rule_stanza_pointer->target_set != -1) {
        if(current_rule_stanza_pointer->target_set > ((2*system_information.cinfo[ct].asc) - 1)) {
            sprintf(msg,"Specified target class-%d is out of range-%d \n",
                    current_rule_stanza_pointer->target_set,((2*system_information.cinfo[ct].asc) - 1));
                hxfmsg(&h_d, 0, HTX_HE_HARD_ERROR, msg);
            sprintf(msg, "limitting class to %d \n", ((2*system_information.cinfo[ct].asc) - 1));
            hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
            th_array[index].end_class = (2*system_information.cinfo[ct].asc) - 1;
        }
        else {
            th_array[index].start_class = th_array[index].end_class = current_rule_stanza_pointer->target_set;
        }
    }

    /*
     * If data_width == 4 then it will randomize the type of store (i.e. byte,
     * short, int or long long.
     */
    if(data_width == 4) data_width = (get_random_number(index)) % 4;

    /*
     * The external loop targets line in set.
     * Offset1 decides the line in set.
     */
    for(walk_line = 0; walk_line < lines_per_set; walk_line++) {

        #if 0
            DEBUGON("\n walk line : %d", walk_line);
            DEBUGON("\n th_array[index].offset_within_cache_line:%d",th_array[index].offset_within_cache_line);
        #endif

        offset1 =
            walk_line*system_information.cinfo[ct].line_size
            + th_array[index].offset_within_cache_line;

        /*
         * The inner loop here targets specific class and writes onto it twice the size
         * so the class rolls over. offset2 jumps by set.
         */
        for(walk_class = th_array[index].start_class;
            walk_class <= th_array[index].end_class;
            walk_class += th_array[index].walk_class_jump) {

            th_array[index].pattern = (unsigned long long)get_random_number(index);
            offset2                 = walk_class*memory_per_set;

            #if 0
                DEBUGON("offset1: %x offset2:%llx final: %llx",offset1,offset2,(offset1+offset2)/(16*M));
                DEBUGON("\n walk_class: %d",walk_class);
            #endif

            if (system_information.pvr != 0x3F )
                addr =     mem.contig_mem[th_array[index].cache_instance_under_test][(offset1+offset2)/(16*M)]+ ((offset1+offset2)%(16*M));
            else  {
                /* Calculate address to write to */
                addr = th_array[index].start_of_contiguous_memory + (offset1 + offset2);
            }

            #if 0
                DEBUGON("\n start_of_contiguous_memory: %x %x %x", mem.contig_mem[th_array[index].cache_instance_under_test][(offset1+offset2)/(16*M)],\
                         (offset1+offset2)/(16*M), th_array[index].cache_instance_under_test);
                DEBUGON("addr: %llx offset1: %x offset2:%llx",addr,offset1,offset2);
            #endif

            (*fptr[data_width])(addr, index , tid);
        }
    }

    /*
     * Write_mem should have written over entire cache twice and hence would have
     * rolled over the cache.
     */
    return(0);
}

int read_byte(void *addr,int index ,  int tid)
{
    char *ptr            = (char *)addr, *pat_ptr = (char *) &th_array[index].pattern;

#if 0
    DEBUGON("1:%lx:%llx:%d\n",addr,(*((unsigned long long *)addr)), tid);
#endif

    if( *(char *)pat_ptr != *ptr ) {
        int i;

        if(crash_on_misc_global == 1 && current_rule_stanza_pointer->crash_on_misc == 1) {
#ifdef __HTX_LINUX__
            do_trap_htx64((unsigned long)0xBEEFDEAD, (unsigned long) addr, (unsigned long) pat_ptr,
                          (unsigned long) tid, (unsigned long) 1 , (unsigned long) index);
#else
            trap(0xBEEFDEAD, addr, pat_ptr, tid,1 , index);
#endif
        }

        for(i = 0; i < 8; i++) DEBUGON(" %x",*((char *)addr + i));
        sprintf(msg,"RB:TID:%d miscompare!!EA = 0x%lx data = %x pattern = %x thread_index \n",tid,addr, \
                *((char*)addr) ,*((char*)pat_ptr) , index);
        hxfmsg(&h_d, 0, HTX_HE_MISCOMPARE, msg);
        exit(1);
    }
    else {
        return(0);
    }
}

int read_short(void *addr,int index , int tid)
{
    char           *pat_ptr        = (char *) &th_array[index].pattern;
    unsigned short *ptr            = (unsigned short *)addr;

#if 0
    DEBUGON("2:%lx:%llx:%d\n",addr, *(unsigned long long *)addr, tid);
#endif

    if( *(unsigned short *)pat_ptr != *ptr) {
        int i;

        if(crash_on_misc_global == 1 && current_rule_stanza_pointer->crash_on_misc == 1) {
#ifdef __HTX_LINUX__
            do_trap_htx64( (unsigned long) 0xBEEFDEAD, (unsigned long) addr, (unsigned long) pat_ptr,
                           (unsigned long) tid, (unsigned long) 2 , (unsigned long) index);
#else
            trap(0xBEEFDEAD, addr, pat_ptr, tid,2 , index);
#endif
        }

        for(i = 0; i < 8; i++) DEBUGON(" %x",*((char *)addr + i));
        sprintf(msg,"RS: TID: %d  miscompare !! EA = 0x%lx data = %x pattern = %x thread index : %d \n", tid,\
                addr, *((unsigned short *)addr), *((unsigned short *)pat_ptr) ,index);
        hxfmsg(&h_d, 0, HTX_HE_MISCOMPARE, msg);
        exit(1);
    }
    else {
        return(0);
    }
}

int read_int(void *addr, int index ,int tid)
{
    char         *pat_ptr        = (char *) &th_array[index].pattern;
    unsigned int *ptr            = (unsigned int *)addr;

#if 0
    DEBUGON("4:%lx:%llx:%d\n",addr, *(unsigned long long *)addr, tid);
#endif

    if( *(unsigned int *)pat_ptr != *ptr) {
        int i;

        if(crash_on_misc_global == 1 && current_rule_stanza_pointer->crash_on_misc == 1) {
#ifdef __HTX_LINUX__
            do_trap_htx64( (unsigned long) 0xBEEFDEAD, (unsigned long) addr, (unsigned long) pat_ptr,
                           (unsigned long) tid, (unsigned long) 4, (unsigned long) index);
#else
            trap(0xBEEFDEAD, addr, pat_ptr, tid,4,index);
#endif
        }

        for(i = 0; i < 8; i++) DEBUGON(" %x",*((char *)addr + i));
        sprintf(msg,"RI: TID: %d miscompare !! EA = 0x%lx data = %x \
            pattern = %x thread index :%d \n",tid,addr,*((unsigned int *)addr),*((unsigned int *)pat_ptr),index);
        hxfmsg(&h_d, 0, HTX_HE_MISCOMPARE, msg);
        exit(1);
    }
    else {
        return(0);
    }
}

int read_ull(void *addr,int index , int tid)
{
    unsigned long long *ptr = (unsigned long long *)addr;
    char *pat_ptr = (char *) &th_array[index].pattern;

#if 0
    DEBUGON("8:%lx:%llx:%d\n",addr, *(unsigned long long *)addr, tid);
#endif

    if( *(unsigned long long *)pat_ptr != *ptr ) {
        int i;

        if(crash_on_misc_global == 1 && current_rule_stanza_pointer->crash_on_misc == 1) {
#ifdef __HTX_LINUX__
            do_trap_htx64( (unsigned long) 0xBEEFDEAD, (unsigned long) addr, (unsigned long) pat_ptr,
                           (unsigned long) tid, (unsigned long) 8 , (unsigned long) index);
#else
            trap(0xBEEFDEAD, addr, pat_ptr, tid,8 ,index);
#endif
        }

        for(i = 0; i < 8; i++) DEBUGON(" %x",*((char *)addr + i));

        sprintf(msg,"RL: TID: %d miscompare !! addr = 0x%lx data = %llx pattern = %llx thread index :%d \n", tid, \
                addr, *((unsigned long long *)addr), *((unsigned long long *)pat_ptr),index);
        hxfmsg(&h_d, 0, HTX_HE_MISCOMPARE, msg);
        exit(1);
    }
    else {
        return(0);
    }
}

int read_and_compare_mem(int index , int tid)
{
    int walk_class, walk_line, memory_per_set, lines_per_set, ct;
    int data_width, ncpus, rc;
    unsigned int random_no, mem_per_thread, offset1, offset2;
    char *addr = mem.contig_mem[0][0];
    int (*fptr[10]) (void *,int , int);


    fptr[0] = &read_byte;
    fptr[1] = &read_short;
    fptr[2] = &read_int;
    fptr[3] = &read_ull;

    /*
     * cache_type decides whether calling for L2 or L3.
     */
    ct = current_rule_stanza_pointer->test;
    data_width = current_rule_stanza_pointer->data_width;
    memory_per_set = system_information.cinfo[ct].cache_size/system_information.cinfo[ct].asc;
    lines_per_set = memory_per_set/system_information.cinfo[ct].line_size;

    /*
     * If target_set is specified in rule file.
     */
    if (current_rule_stanza_pointer->target_set != -1) {
        if(current_rule_stanza_pointer->target_set > ((2*system_information.cinfo[ct].asc) - 1)) {
            sprintf(msg,"Specified target class-%d is out of range-%d \n",
                    current_rule_stanza_pointer->target_set,((2*system_information.cinfo[ct].asc) - 1));
            hxfmsg(&h_d, 0, HTX_HE_HARD_ERROR, msg);
            sprintf(msg, "limitting class to %d \n", ((2*system_information.cinfo[ct].asc) - 1));
            hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
            th_array[index].end_class = (2*system_information.cinfo[ct].asc) - 1;
        }
        else {
            th_array[index].start_class =th_array[index].end_class = current_rule_stanza_pointer->target_set;
        }
    }

    /*
     * If data_width == 4 then it will randomize the type of load (i.e. byte,
     * short, int or long long.
     */
    if(data_width == 4) data_width = (get_random_number(index))%4;

    /*
     * The external loop targets line in set.
     */
    for(walk_line = 0; walk_line < lines_per_set; walk_line++) {

        offset1 =
            walk_line*system_information.cinfo[ct].line_size
            + th_array[index].offset_within_cache_line;

        /*
           * The inner loop here targets specific class and read/compare onto it.
           */
        for(walk_class = th_array[index].start_class;
            walk_class <= th_array[index].end_class;
            walk_class += th_array[index].walk_class_jump) {

            th_array[index].pattern = (unsigned long long)get_random_number(index);
            offset2 = walk_class*memory_per_set;

            if (system_information.pvr != 0x3F )
                addr =     mem.contig_mem[th_array[index].cache_instance_under_test][(offset1+offset2)/(16*M)]+ ((offset1+offset2)%(16*M));
            else  {
                /* Calculate address to read/compare from */
                addr = th_array[index].start_of_contiguous_memory + (offset1 + offset2);
            }

            (*fptr[data_width])(addr,index , tid);
        }
    }
    DEBUGON("Caches compared okay ! \n");
    return(0);
}

void set_defaults()
{
    /*
     * This function sets default values for every test case in rule file.
     * Following are the defaults :
     *
     * rule_id - NULL string
     * test - 0        i.e. L2 testing
     * compare - 1        i.e. compare the results
     * bound - -1        i.e. No binding
     * data_width - 4    i.e. random data_width values
     * target_set - -1    i.e. all classes
     * crash_on_misc - 1    i.e. drop to KDB incase of miscompare
     */
    int i;

    for(i = 0; i < MAX_TC; i+=3) {
        /*
         * h_r being a global, everthing defaults to zero.
         */
        h_r[i].compare = 1;
        h_r[i].bound = -1;
        h_r[i].data_width = 4;
        h_r[i].target_set = -1;
        h_r[i].crash_on_misc = 1;
        h_r[i].prefetch_irritator = OFF;
        h_r[i].prefetch_nstride = OFF;
        h_r[i].prefetch_partial = OFF;
        h_r[i].prefetch_transient = OFF;
        h_r[i].testcase_type = CACHE_ROLLOVER;
        h_r[i].gang_size = GANG_SIZE;
        h_r[i].prefetch_configuration =  0x0;
#if defined(__HTX_LINUX__) && defined(AWAN)
        h_r[i].test = L3;
        h_r[i].num_oper = 15;
#endif


        h_r[i+1].compare = 1;
        h_r[i+1].bound = -1;
        h_r[i+1].data_width = 3;
        h_r[i+1].target_set = -1;
        h_r[i+1].crash_on_misc = 1;
        h_r[i+1].prefetch_irritator = OFF;
        h_r[i+1].prefetch_nstride = ON;
        h_r[i+1].prefetch_partial = ON;
        h_r[i+1].prefetch_transient = ON;
        h_r[i+1].testcase_type = CACHE_ROLLOVER;
        h_r[i+1].gang_size = GANG_SIZE;
        h_r[i+1].prefetch_configuration =  0x7;

#if defined(__HTX_LINUX__) && defined(AWAN)
        h_r[i+1].test = L3;
        h_r[i+1].num_oper = 15;
#endif


        h_r[i+2].compare = 1;
        h_r[i+2].bound = -1;
        h_r[i+2].data_width = 3;
        h_r[i+2].target_set = -1;
        h_r[i+2].crash_on_misc = 1;
        h_r[i+2].prefetch_irritator = OFF;
        h_r[i+2].prefetch_nstride = ON;
        h_r[i+2].prefetch_partial = ON;
        h_r[i+2].prefetch_transient = ON;
        h_r[i+2].testcase_type = CACHE_BOUNCE;
        h_r[i+2].gang_size = GANG_SIZE;
        h_r[i+2].prefetch_configuration =  0x7;

#if defined(__HTX_LINUX__) && defined(AWAN)
        h_r[i+2].test = L3;
        h_r[i+2].num_oper = 15;
#endif

    }
}

int get_line( char s[], FILE *fp)
{
     int c,i;

     i=0;
     while (((c = fgetc(fp)) != EOF) && c != '\n') {
        s[i++] = c;
     }
     if (c == '\n')
        s[i++] = c;
    s[i] = '\0';
     return(i);
}

int parse_line(char s[])
{
    int len, i = 0, j = 0;

    while(s[i] == ' ' || s[i] == '\t') {
        i++;
    }
    if(s[i] == '*') {
        return(0);
    }
    len = strlen(s);
    for(; i < len && s[i] != '\0'; i++) {
        s[j++] = s[i];
    }
    s[j] = '\0';
    return((s[0] == '\n')? 1 : j);
}

int read_rule_file()
{

#if defined(__HTX_LINUX__) && defined(AWAN)
    set_defaults();
    num_testcases = 3;
#else

    FILE *fptr;
    char line[200], keywd[200];
    int eof_flag = 0, num_tc = 0, keyword_match, rc, change_tc = 1;
    struct ruleinfo *current_ruleptr = &h_r[0];

    /* Set defaults is called below to fill in the rule file parameter structure just in case user makes mistakes.
     * No prefetch would run, also see line 1534  .. very crucial
     */

    set_defaults();
    errno = 0;
    if ( (fptr = fopen(rule_file_name, "r")) == NULL ) {
        sprintf(msg,"error open %s \n",rule_file_name);
        hxfmsg(&h_d, errno, HTX_HE_HARD_ERROR, msg);
        return(-1);
    }
    do {
        rc = get_line(line, fptr);
        /*
         * rc = 0 indicates End of File.
         * rc = 1 indicates only '\n' (newline char) on the line.
         * rc > 1 more characters.
         */
        if(rc == 0) {
            eof_flag = 1;
            break;
        }
        DEBUGON("%s: line - %s rc - %d \n",__FUNCTION__, line, rc);
        /*
         * rc = 1 indicates a newline which means end of current testcase.
          */
        if(rc == 1) {
            change_tc = 1;
            continue;
        }
        /*
          * rc = 0 indicates comment line in rule file.
          * rc = 1 indicates some white spaces and newline.
          * rc > 1 indicates there may be valid test case parameter.
          */
        rc = parse_line(line);
        DEBUGON("%s: line - %s rc - %d \n",__FUNCTION__, line, rc);
        if(rc == 0 || rc == 1) {
            if(rc == 1) change_tc = 1;
            continue;
        }
        else if(rc > 1 && change_tc == 1) {

            current_ruleptr = &h_r[num_tc];
            current_ruleptr->prefetch_configuration = 0 ;
            if(num_tc >= MAX_TC) {
                sprintf(msg,"Max num of test cases allowed are %d \n", MAX_TC);
                hxfmsg(&h_d, 0, HTX_HE_HARD_ERROR, msg);
                return(-1);
            }
            num_tc++;
            DEBUGON("Next rule - %d \n", num_tc);
            change_tc = 0;
        }

        sscanf(line, "%s",keywd);
        DEBUGON("%s: keywd - %s\n",__FUNCTION__, keywd);
        if ( (strcmp(keywd, "rule_id")) == 0 ) {
            char tmp[30];
            sscanf(line, "%*s %s", tmp);
            if (((strlen(tmp)) < 1) || ((strlen(tmp)) > 19) ) {
                sprintf(msg, "rule_id string (%s) length should be in the range"
                        " 1 < length < 19 \n", current_ruleptr->rule_id);
                hxfmsg(&h_d, 0, HTX_HE_SOFT_ERROR, msg);
                strncpy(current_ruleptr->rule_id, tmp, 19);
            }
            else {
                strcpy(current_ruleptr->rule_id, tmp);
                DEBUGON("rule_id - %s \n", current_ruleptr->rule_id);
            }
        }
        else if ((strcmp(keywd, "test")) == 0 ) {
            sscanf(line, "%*s %d", &current_ruleptr->test);
            if (current_ruleptr->test != 0 && current_ruleptr->test != 1) {
                sprintf(msg, "for 'test' possible values are 0 or 1 \n");
                hxfmsg(&h_d, 0, HTX_HE_HARD_ERROR, msg);
                return(-1);
            }
            else {
                DEBUGON("test - %d \n", current_ruleptr->test);
            }
        }
        else if ((strcmp(keywd, "compare")) == 0 ) {
            sscanf(line, "%*s %d", &current_ruleptr->compare);
            DEBUGON("compare - %d \n", current_ruleptr->compare);
            if (current_ruleptr->compare != 0 && current_ruleptr->compare != 1) {
                sprintf(msg, "for 'compare' possible values are 0 or 1 \n");
                hxfmsg(&h_d, 0, HTX_HE_HARD_ERROR, msg);
                return(-1);
            }
            else {
                DEBUGON("compare - %d \n", current_ruleptr->compare);
            }
        }
        else if ((strcmp(keywd, "bound")) == 0 ) {
            sscanf(line, "%*s %d", &current_ruleptr->bound);
            if (current_ruleptr->bound < 0 ) {
                sprintf(msg, "For 'bound' possible values are valid proc nos \n");
                hxfmsg(&h_d, 0, HTX_HE_HARD_ERROR, msg);
                return(-1);
            }
            else {
                DEBUGON("bound - %d \n", current_ruleptr->bound);
            }
        }
        else if ((strcmp(keywd, "data_width")) == 0 ) {
            sscanf(line, "%*s %d", &current_ruleptr->data_width);
            DEBUGON("data_width - %d \n", current_ruleptr->data_width);
            if (current_ruleptr->data_width < 0 || current_ruleptr->data_width > 4) {
                sprintf(msg, "For 'data_width' possible values are 0 to 4 \n");
                hxfmsg(&h_d, 0, HTX_HE_HARD_ERROR, msg);
                return(-1);
            }
            else {
                DEBUGON("data_width - %d \n", current_ruleptr->data_width);
            }
        }
        else if ((strcmp(keywd, "target_set")) == 0 ) {
            sscanf(line, "%*s %d", &current_ruleptr->target_set);
            DEBUGON("data_width - %d \n", current_ruleptr->target_set);
            if (current_ruleptr->target_set < -1) {
                sprintf(msg, "target_set can not be less than -1 \n");
                hxfmsg(&h_d, 0, HTX_HE_HARD_ERROR, msg);
                return(-1);
            }
            else {
                DEBUGON("target_set - %d \n", current_ruleptr->target_set);
            }
        }
        else if ((strcmp(keywd, "crash_on_misc")) == 0 ) {
            sscanf(line, "%*s %d", &current_ruleptr->crash_on_misc);
            DEBUGON("crash_on_misc - %d \n", current_ruleptr->crash_on_misc);
            if (current_ruleptr->crash_on_misc < -1) {
                sprintf(msg, "crash_on_misc must be either 0 or 1 \n");
                hxfmsg(&h_d, 0, HTX_HE_HARD_ERROR, msg);
                return(-1);
            }
            else {
                DEBUGON("crash_on_misc - %d \n", current_ruleptr->crash_on_misc);
            }
        }
        else if ((strcmp(keywd, "num_oper")) == 0 ) {
            sscanf(line, "%*s %d", &current_ruleptr->num_oper);
            DEBUGON("num_oper - %d \n", current_ruleptr->num_oper);
            if (current_ruleptr->num_oper < -1) {
                sprintf(msg, "num_oper must be either 0 or +ve integer \n");
                hxfmsg(&h_d, 0, HTX_HE_HARD_ERROR, msg);
                return(-1);
            }
            else {
                DEBUGON("num_oper - %d \n", current_ruleptr->num_oper);
            }
        }
        else if ((strcmp(keywd, "prefetch_irritator")) == 0 ) {
            sscanf(line, "%*s %d", &current_ruleptr->prefetch_irritator);

            DEBUGON("prefetch_irritator =%d \n",current_ruleptr->prefetch_irritator);
            if ( current_ruleptr->prefetch_irritator != 0 && current_ruleptr->prefetch_irritator != 1) {
                sprintf(msg, "prefetch_irritator should have value of either 0 or 1 \n");
                hxfmsg(&h_d, 0, HTX_HE_HARD_ERROR, msg );
                return(-1);
            }
            else {
                DEBUGON("prefetch_irritator - %d \n",current_ruleptr->prefetch_irritator);
            }

            /* Set the appropriate field in prefetch_configuration */
            current_ruleptr->prefetch_configuration +=  current_ruleptr->prefetch_irritator * PREFETCH_IRRITATOR;
        }
        else if ((strcmp(keywd, "prefetch_nstride")) == 0 ) {
            sscanf(line, "%*s %d", &current_ruleptr->prefetch_nstride);

            DEBUGON("prefetch_nstride =%d \n",current_ruleptr->prefetch_nstride);
            if ( current_ruleptr->prefetch_nstride != 0 && current_ruleptr->prefetch_nstride != 1) {
                sprintf(msg, "prefetch_nstride should have value of either 0 or 1 \n");
                hxfmsg(&h_d, 0, HTX_HE_HARD_ERROR, msg );
                return(-1);
            }
            else {
                DEBUGON("prefetch_nstride - %d \n",current_ruleptr->prefetch_nstride);
            }

            if ( current_ruleptr->prefetch_nstride == ON && system_information.pvr == 0x3E) {
                /* Check if prefetch algorithm is supported on P6 */
                sprintf(msg, "prefetch n-stride not supported for P6\n");
                hxfmsg(&h_d, 0, HTX_HE_INFO, msg );
                current_ruleptr->prefetch_nstride = OFF;
            }

            /* Set the appropriate field in prefetch_configuration */
            current_ruleptr->prefetch_configuration += current_ruleptr->prefetch_nstride * PREFETCH_NSTRIDE;
        }
        else if ((strcmp(keywd, "prefetch_partial")) == 0 ) {
            sscanf(line, "%*s %d", &current_ruleptr->prefetch_partial);

            DEBUGON("prefetch_partial =%d \n",current_ruleptr->prefetch_partial);
            if ( current_ruleptr->prefetch_partial != 0 && current_ruleptr->prefetch_partial != 1) {
                sprintf(msg, "prefetch_partial should have value of either 0 or 1 \n");
                hxfmsg(&h_d, 0, HTX_HE_HARD_ERROR, msg );
                return(-1);
            }
            else {
                DEBUGON("prefetch_partial - %d \n",current_ruleptr->prefetch_partial);
            }

            if ( current_ruleptr->prefetch_partial == ON && system_information.pvr == 0x3E) {
                /* Check if prefetch algorithm is supported on P6 */
                sprintf(msg, "prefetch partial not supported for P6\n");
                hxfmsg(&h_d, 0, HTX_HE_INFO, msg );
                current_ruleptr->prefetch_partial = OFF;
            }

            /* Set the appropriate field in prefetch_configuration */
            current_ruleptr->prefetch_configuration +=  current_ruleptr->prefetch_partial * PREFETCH_PARTIAL;
        }
        else if ((strcmp(keywd, "prefetch_transient")) == 0 ) {
            sscanf(line, "%*s %d", &current_ruleptr->prefetch_transient);

            DEBUGON("prefetch_transient =%d \n",current_ruleptr->prefetch_transient);
            if ( current_ruleptr->prefetch_transient != 0 && current_ruleptr->prefetch_transient != 1) {
                sprintf(msg, "prefetch_transient should have value of either 0 or 1 \n");
                hxfmsg(&h_d, 0, HTX_HE_HARD_ERROR, msg );
                return(-1);
            }
            else {
                DEBUGON("prefetch_transient - %d \n",current_ruleptr->prefetch_transient);
            }

            if ( current_ruleptr->prefetch_transient == ON && system_information.pvr == 0x3E) {
                /* Check if prefetch algorithm is supported on P6 */
                sprintf(msg, "prefetch transient not supported for P6\n");
                hxfmsg(&h_d, 0, HTX_HE_INFO, msg );
                current_ruleptr->prefetch_transient = OFF;
            }

            /* Set the appropriate field in prefetch_configuration */
            current_ruleptr->prefetch_configuration += current_ruleptr->prefetch_transient * PREFETCH_TRANSIENT;
        }
        else if ((strcmp(keywd, "seed")) == 0 ) {
            sscanf(line, "%*s %x", &current_ruleptr->seed);
            DEBUGON("Seed - %x  \n", current_ruleptr->seed);
        }
        else if ((strcmp(keywd, "testcase_type")) == 0 ) {
            sscanf(line, "%*s %x", &current_ruleptr->testcase_type);
            DEBUGON("Seed - %x \n", current_ruleptr->testcase_type);

            if ( current_ruleptr->testcase_type == CACHE_ROLLOVER &&
                 system_information.pvr == 0x3E) {
                /* If Testcase type is CACHE_ROLLOVER and P6 */
                sprintf(msg, "testcase_type CACHE_ROLLOVER supported"
                        " only on P7.Will run CACHE_BOUNCE testcase instead \n");
                hxfmsg(&h_d, 0, HTX_HE_INFO, msg );
                current_ruleptr->testcase_type = CACHE_BOUNCE;
            }

            if ( current_ruleptr->testcase_type != CACHE_BOUNCE &&
                 current_ruleptr->testcase_type != CACHE_ROLLOVER &&
                 current_ruleptr->testcase_type != CACHE_TEST_OFF) {
                /* If Testcase type is not CACHE_BOUNCE/CACHE_ROLLOVER/CACHE_TEST_OFF,
                 * report error and exit
                 */
                sprintf(msg, "testcase_type should have value of either 0,1 or 2 \n");
                hxfmsg(&h_d, 0, HTX_HE_HARD_ERROR, msg );
                return(-1);
            }
                sprintf(msg, "[%d] testcase_type  = %d\n",__LINE__,current_ruleptr->testcase_type);
                hxfmsg(&h_d, 0, HTX_HE_INFO, msg );
        }
        else if ((strcmp(keywd, "gang_size")) == 0 ) {
            sscanf(line, "%*s %d", &gang_size);
            current_ruleptr->gang_size = gang_size;
            if( gang_size != 1 && gang_size != 16 && gang_size != 8) {
                /* If gang size is not 1 (for requaliser) AND
                 * If gang size is not 16 (default value ) , then the gang size is
                 * configured incorrectly. Print an error message and set gang size to
                 * default value
                 */
                sprintf(msg,"Gang Size is incorrectly configured to %d, should be either 1, 8 or 16 \n",gang_size);
                hxfmsg(&h_d, 0, HTX_HE_HARD_ERROR, msg );
                return(-1);
            }
            DEBUGON("Gang Size - %x \n", &gang_size);
        }
        else {
            sprintf(msg, "Wrong keyword - %s specified in rule file. Exiting !! \n", keywd);
            hxfmsg(&h_d, 0, HTX_HE_HARD_ERROR, msg);
            return(-1);
        }
    } while(eof_flag == 0);
    num_testcases = num_tc;
#endif

    return(0);
}

void write_read_and_compare(void *arg)
{
    struct thread_context  *t        = (struct thread_context *)arg;
    int                    index     = t->thread_no;
    int                    tc        = t->tc_id;
    int                    cpu       = t->bind_to_cpu;
    int                    tid       = t->tid ;
    int                    rc , oper;
    int                    i=0,j=0,s=0;                           /* Counters for syncwords   */
    unsigned long long int mask      = t->sync_mask;              /* Sync mask for the thread */
    pthread_t current_tid;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

#ifdef __HTX_LINUX__
    cpu_set_t cpu_affinity_mask;
    CPU_ZERO(&cpu_affinity_mask);                        /* Clear the mask */

    int physical_cpu;
    physical_cpu = get_logical_to_physical_cpu(cpu);

    if(physical_cpu!=-1){
    CPU_SET(physical_cpu, &cpu_affinity_mask);
    }

    /* Using sched_setaffinity() */
    rc = sched_setaffinity(syscall(SYS_gettid) , sizeof(cpu_affinity_mask), &cpu_affinity_mask ) ;
#else
    rc = bindprocessor(BINDTHREAD, thread_self(), cpu);
#endif
    if(rc == -1) {
        sprintf(msg, "Binding to cpu:%d  failed with errno: %d \n",cpu,errno);
        hxfmsg(&h_d, errno, HTX_HE_SOFT_ERROR, msg);

#ifdef __HTX_LINUX__
        /*Debug printf being added for more info on cpu mask, if cpu bind fails */
        {
            unsigned long i;
            unsigned long *ptr;
            ptr=&default_cpu_affinity_mask;
            sprintf(msg,"::Start of Printing Default cpu mask::\n");
            hxfmsg(&h_d, errno, HTX_HE_SOFT_ERROR, msg);
            for(i=0;i < sizeof(default_cpu_affinity_mask); i=i+sizeof(unsigned long) ){
                /*Cpu mask is implemented as an array of unsigned long's */

                sprintf(msg,"default_cpu_affinity_mask[%3d]:%16lx\n",
                        (i/sizeof(unsigned long)), *ptr);
                hxfmsg(&h_d, errno, HTX_HE_INFO, msg);
                ptr++;
            }
            sprintf(msg,"::End of Printing Default cpu mask::\n");
            hxfmsg(&h_d, errno, HTX_HE_INFO, msg);
        }


        /*Debug printf being added for more info on cpu mask, if cpu bind fails */
        {
            unsigned long i;
            unsigned long *ptr;
            ptr=&cpu_affinity_mask;
            sprintf(msg,"::Start of Printing cpu mask::\n");
            hxfmsg(&h_d, errno, HTX_HE_INFO, msg);

            for(i=0;i < sizeof(cpu_affinity_mask); i=i+sizeof(unsigned long) ){
                /*Cpu mask is implemented as an array of unsigned long's */

                sprintf(msg,"cpu_affinity_mask[%3d]:%16lx\n",
                        (i/sizeof(unsigned long)), *ptr);
                hxfmsg(&h_d, errno, HTX_HE_INFO, msg);
                ptr++;
            }
             sprintf(msg,"::End of Printing cpu mask::\n");
            hxfmsg(&h_d, errno, HTX_HE_INFO, msg);
        }

        /* Set exit_flag here to terminate testcase */
        exit_flag = ERR_EXIT;
        pthread_exit(NULL);
#endif

    }
    else {
    DEBUGON("Binding to cpu:%d by thread successful ! \n",cpu );
    }

    /*
     * If num_oper is 0 then run infinitely else num_oper times.
     */

    int shift_count = total_cache_threads - 1;            /* Number of bits the syncword has to be left shifted */

    for(oper = 0,s=0; oper < current_rule_stanza_pointer->num_oper || current_rule_stanza_pointer->num_oper == 0; oper++,s=(s+1)%MAXSYNCWORDS) {
        if (t->seedval == 0 )
        t->seedval = time(NULL);

    srand48_r(t->seedval,&t->buffer);

    th_array[index].current_oper = oper;

    /* Sync up before starting the pass */
    DEBUGON("loop = %d, tid = %d Syncing before pass %d\n",oper,index,oper);

     change(&sync_word1[s],mask,0,0);                        /* Turn off the thread's bit in the sync word1 */
    wait_for(&sync_word1[s],0,0);                            /* Wait till all other threads have turned off their bit in sync word 1 */

    DEBUGON("Loop = %d, tid = %d, sync_word1 = %d, sync_word2 = %d, sync_word3 = %d\n",oper,index,sync_word1[s],sync_word2[s],sync_word3[s]);
    DEBUGON("loop = %d,TID : %d  Before write : srand48 seedval  : %d \n",oper,index ,t->seedval);
    DEBUGON("loop = %d,srand48 buffer : %d \n ",oper, t->buffer);

           write_mem(index ,tid);

        /* Sync in between so that all the threads finish writing before starting to read */

    DEBUGON("loop = %d, tid = %d,Syncing in between \n",oper,index);
    change(&sync_word2[s] ,mask,0,0);
        wait_for(&sync_word2[s] ,0,0);

    DEBUGON("Loop = %d, tid = %d, sync_word1 = %d, sync_word2 = %d, sync_word3 = %d\n",oper,index,sync_word1[s],sync_word2[s],sync_word3[s]);

    if (current_rule_stanza_pointer->compare == 1 ) {
        srand48_r(t->seedval,&t->buffer);
        DEBUGON("loop = %d,TID : %d Before read : srand48 seedval  : %d \n",oper,index , t->seedval);
        DEBUGON("loop = %d,srand48 buffer  : %d \n ",oper,t->buffer);
        read_and_compare_mem(index , tid);
    }

#if defined(__HTX_LINUX__) && defined(AWAN)
    printf("write_read_and_compare: Completed pass number %d\n",oper);
#endif

    /* Sync up so that all threads wait for each other to complete before proceeding to next pass */

    DEBUGON("loop = %d, tid = %d,Syncing after pass %d\n",oper,index,oper);
    change(&sync_word3[s] ,mask,0,0);
        wait_for(&sync_word3[s] ,0,0);
    DEBUGON("Loop = %d, tid = %d, sync_word1 = %d, sync_word2 = %d, sync_word3 = %d\n",oper,index,sync_word1[s],sync_word2[s],sync_word3[s]);

    j=(s + DISTANCE)%MAXSYNCWORDS;                /* Maintain a distance of DISTANCE. And be withtin the limit of MAXSYNCWORDS.   */
        set_sync_word(&sync_word1[j] ,shift_count,0,0);        /* When threads are executing i'th loop, set the sync words              */
        set_sync_word(&sync_word2[j] ,shift_count,0,0);        /* for i+DISTANCE'th loop. This will ensure that race condition            */
        set_sync_word(&sync_word3[j] ,shift_count,0,0);        /* never occurs.                                                      */
    }

#ifdef __HTX_LINUX__
    /* Restore original/default CPU affinity so that it binds to ANY available processor */

    rc = sched_setaffinity(syscall(SYS_gettid) , sizeof(default_cpu_affinity_mask), &default_cpu_affinity_mask ) ;
#else
    rc = bindprocessor(BINDTHREAD, thread_self(), PROCESSOR_CLASS_ANY);
#endif
    if(rc == -1) {
        sprintf(msg, "%d: Unbinding from cpu:%d failed with errno %d \n",__LINE__, cpu, errno);
    hxfmsg(&h_d, errno, HTX_HE_SOFT_ERROR, msg);
    }
    return ;
}

#ifndef __HTX_LINUX__
void DR_handler(int sig, int code, struct sigcontext *scp)
{
    
    int       	rc = 0,i;
    pid64_t   	pid ;
    char      	tmp[128], workstr[80];
    dr_info_t 	DRinfo;
    pthread_t 	current_tid ;
	int 		check_flag = 0, pre_flag = 0, post_flag = 0, max_cpus;

    pid = getpid();
	current_tid = thread_self();
    do {
        rc = dr_reconfig(DR_QUERY, &DRinfo);
    } while ((rc < 0) && (errno == EINTR));
    if( (rc < 0) && (errno == ENXIO) ) {
        return;
    }
    else if (rc < 0) {
        sprintf(msg, "DR: %s for %s: Error in DR_QUERY.  \n\
        dr_reconfig system call returned error(%d).\n",
                h_d.HE_name, h_d.sdev_id, errno);
        hxfmsg(&h_d, 0, HTX_HE_HARD_ERROR, msg);
        return;
       }

	if (exit_flag != DR_EXIT) {
  		sprintf(msg,"DR: SIGRECONFIG  signal recieved !!\n") ;
   		hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
     
    	if (DRinfo.mem == 1 || DRinfo.ent_cap == 1 || DRinfo.hibernate == 1){
   			sprintf(msg, "DR: DLPAR Details - \n"\
						"Phase - Check:  %d, Pre: %d, Post: %d, Post Error: %d\n"\
						"Type - Mem add: %d remove: %d, ent_cap = %d, hibernate = %d "\
						"Action - No action taken. Doing DR_RECONFIG_DONE \n", \
            			DRinfo.check, DRinfo.pre, DRinfo.post, DRinfo.posterror, DRinfo.add, DRinfo.rem, DRinfo.ent_cap, DRinfo.hibernate);
    		hxfmsg(&h_d,0,HTX_HE_INFO,msg);

       		if(dr_reconfig(DR_RECONFIG_DONE,&DRinfo)){
            	sprintf(msg,"dr_reconfig(DR_RECONFIG_DONE) failed with errno %d \n", errno);
            	hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
            	return;
        	}

			sprintf(msg,"DR: DR_RECONFIG_DONE Success!!\n");
			hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
    	}

    /* cpu remove or add operation */

    	if (DRinfo.cpu == 1 && (DRinfo.rem || DRinfo.add) ) {

			/* Check if a cpu in this instance is removed. If it is, then shutdown this instance. */
			/* If it is not, then no need to kill this instance. Reply with DR_RECONFIG done.	  */

		
			if	(DRinfo.check == 1) {
				for( i = system_information.start_cpu_number; i <= system_information.end_cpu_number ; i++ ) {
		
					if ( DRinfo.bcpu == i || (DRinfo.add && ((num_prefetch_threads+total_cache_threads) < MAX_CPUS_PER_CHIP))) {
					
						sprintf(msg, "hxecache: DR in check phase. It will exit soon !\n");
       					hxfmsg(&h_d, 0, HTX_HE_INFO, msg);

        				rc = pthread_mutex_lock( &mutex );
        				if(rc != 0) {
            				sprintf(msg,"[%d] %s: Could not acquire lock\n",__LINE__,__FUNCTION__);
            				hxfmsg(&h_d,errno,HTX_HE_SOFT_ERROR,msg);
        				}

        				exit_flag = DR_EXIT ;

        /* Release the mutex lock */

        rc = pthread_mutex_unlock( &mutex );           
        if(rc != 0) {
            sprintf(msg,"[%d] %s: Could not release lock\n",__LINE__,__FUNCTION__);
            hxfmsg(&h_d,errno,HTX_HE_SOFT_ERROR,msg);
        }

	   					sprintf(msg, "DR: DLPAR Details - \n"\
								"Phase - Check:  %d, Pre: %d, Post: %d, Post Error: %d\n"\
								"Type - CPU remove : %d, add : %d bcpu = %d thread_id = 0x%x\n"\
								"Action - cleaning up and exit \n", \
            					DRinfo.check, DRinfo.pre, DRinfo.post, DRinfo.posterror, DRinfo.rem, DRinfo.add, DRinfo.bcpu, current_tid);
    					hxfmsg(&h_d,0,HTX_HE_INFO,msg);
	
 			       		hxfupdate(RECONFIG, &h_d);

						return;
					}
				} 
   			}

 	  		sprintf(msg, "DR: DLPAR Details - \n"\
						"Phase - Check: %d, Pre: %d, Post: %d, Posterr: %d\n"\
						"Type - CPU remove : %d, add : %d bcpu = %d thread_id = 0x%x\n",\
						DRinfo.check, DRinfo.pre, DRinfo.post, DRinfo.posterror,\
						DRinfo.rem, DRinfo.add,DRinfo.bcpu,current_tid);

			if(DRinfo.check == 1)
				strcat(msg, "Action - Ignoring\n");
 			else if(DRinfo.pre || DRinfo.post || DRinfo.posterror) 
				strcat(msg, "Action - Action taken in check phase \n");

 			hxfmsg(&h_d,0,HTX_HE_INFO,msg);

 	    	if(dr_reconfig(DR_RECONFIG_DONE,&DRinfo)) {
    	   		sprintf(msg,"dr_reconfig(DR_RECONFIG_DONE) failed.  error no %d \n", errno);
          		hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
				return;
       		} else {
	       		sprintf(msg,"DR: DR_RECONFIG_DONE Success!!\n");
				hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
			}

 	   } /* rem / add cpu */
	}
	return;
}
#endif

unsigned long int get_random_number(int seg)
{
    unsigned long int val_8 ;

    /* lrand48_r returns 4 byte random number . Generate one random number shift it by 4bytes and OR it
     * 4 bytes random number generated by second lrand48_r call
     */
    lrand48_r(&th_array[seg].buffer, &th_array[seg].random_pattern);
    val_8 = (unsigned long)th_array[seg].random_pattern << 32 ;
    lrand48_r(&th_array[seg].buffer, &th_array[seg].random_pattern);
    val_8 = val_8 | th_array[seg].random_pattern ;
    return val_8 ;
}

long long find_worst_case_memory_required()
{
    int i, number_of_physical_cache_instances;
    long long worst_case_memory_required;
    int rollover_testcase_flag = 0;             /* Flag determines if rollover test case is to run */
    int prefetch_only_flag = 0;             /* Flag determines if only standalone prefetch case is to run */

    /* Check to see if ROLLOVER testcase is enabled in any of the rule stanza's */
    for(i = 0; i < num_testcases; i++) {

        /* If all rule stanza's run a standalone prefetch testcase, the when we emerge from this loop
         * the prefetch_only_flag will be set
         */
        if( h_r[i].testcase_type == CACHE_TEST_OFF ) {
            prefetch_only_flag=ON;
        }

        /* If any one of the rule stanzas run a ROLLOVER testcase, then we set the rollover flag,
         * turn off the prefetch_only_flag and exit this loop .
         */
        if( h_r[i].testcase_type == CACHE_ROLLOVER ) {
            rollover_testcase_flag = CACHE_ROLLOVER;
            prefetch_only_flag=OFF;
            break;
        }
    }

    /* If neither Rollover testcase flag nor Standalone prefetch flag is enabled then all stanzas
     * are running Bounce testcase. Therefore we set the Bounce flag.
     */
    if(rollover_testcase_flag != CACHE_ROLLOVER  &&  prefetch_only_flag==OFF) {
        rollover_testcase_flag = CACHE_BOUNCE;
    }

    /* If all stanzas are running Standalone prefetch testcases, then memory for cache threads is not required.
     * Exclusive prefetch memory will be allocated for prefetch threads.
     */
    if(prefetch_only_flag == ON) {
        worst_case_memory_required =0;
        return worst_case_memory_required;
    }

    /* Reset the global variables */
    test_l2 = 0;
    test_l3 = 0;

    /*
     *  Check whether we need to test L2 or L3. test_l2 and test_l3 variables are
     *  set to decide whether memory allocation needs to be be done 2*L2 or 2*L3.
     *  If any of the test cases have L3 test then the same memory will be used for
     *  L2 testing.
     */
    for(i = 0; i < num_testcases; i++) {
        if( h_r[i].test == L3 ) {
            test_l3 = 1;
            break;
        }
    }
    if (test_l3 == 0)
        test_l2 = 1;

    if (test_l3) {
        worst_case_contig_mem = 2*system_information.cinfo[L3].cache_size;
    }
    else {
        worst_case_contig_mem = 2*system_information.cinfo[L2].cache_size;
    }

    if (system_information.pvr == 0x3E) {
        worst_case_memory_required = SEG_SIZE; /* 256MB */
        cache_page_req = (worst_case_memory_required/(16*M));
        }
    else if  ((system_information.pvr == 0x3F) && (rollover_testcase_flag == CACHE_BOUNCE)) {
        worst_case_memory_required =  16*M;
        cache_page_req = (worst_case_memory_required/(16*M));
    }
    else if ((system_information.pvr == 0x3F) && (rollover_testcase_flag == CACHE_ROLLOVER)) {
        /* If P7 AND If testcase type is CACHE_ROLLOVER   Calculate the number of physical instances of cache */
        number_of_physical_cache_instances =
            (system_information.end_cpu_number - system_information.start_cpu_number + 1)/system_information.smt_threads;
        worst_case_memory_required = (16*M) * number_of_physical_cache_instances;
        cache_page_req = (worst_case_memory_required/(16*M));
    }
    else if ((system_information.pvr == 0x4A) && (rollover_testcase_flag == CACHE_BOUNCE)) {
        worst_case_memory_required = 128*M;
        cache_page_req = (worst_case_memory_required/(16*M));
        sprintf(msg,"[%d] cache_mem reqd = 128M = %d 16M pages\n",__LINE__,cache_page_req);
        hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
        cache_bounce_testcase = 1;
    }
    else /* its p7+ with cache rollover */ {
        number_of_physical_cache_instances =
            (system_information.end_cpu_number - system_information.start_cpu_number + 1)/system_information.smt_threads;
        worst_case_memory_required = (128*M) * number_of_physical_cache_instances;
        cache_page_req = (worst_case_memory_required/(16*M));
        sprintf(msg,"[%d] cache_mem reqd = %d 16M pages\n",__LINE__,cache_page_req);
        hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
        
        cache_rollover_testcase = 1;
    }

    sprintf(msg,"[%d] cache_mem reqd = %d 16M pages\n",__LINE__,cache_page_req);
    hxfmsg(&h_d, 0, HTX_HE_INFO, msg);

    return worst_case_memory_required;
}

long long find_worst_case_prefetch_memory_required() {

    int i, number_of_physical_cache_instances=0,number_of_pages_required;
    long long worst_case_prefetch_memory_required;
    int total_number_of_8MB_segments_required=0,testcase_type=0;

    /* Calculate number of physical instances of cache covered by this gang */
    number_of_physical_cache_instances =
            (system_information.end_cpu_number - system_information.start_cpu_number + 1)/system_information.smt_threads;

    if (system_information.pvr == 0x3E ) {
        /* If P6 , then we are already going to allocate a 256MB segment, from which a part can
         * be used for prefetch memory. No extra pages are required for prefetch memory
         */
        number_of_pages_required = 0;
        worst_case_prefetch_memory_required = 0;
    }
    else if (system_information.pvr == 0x4A && cache_rollover_testcase && test_l3 == 1 ) {
        /* If cache_rollover_testcase is set then no need to set aside seperate mem for prefetch in case of p7+ */
        worst_case_prefetch_memory_required = 0;
        prefetch_page_req = 0;
        sprintf(msg,"[%d] prefetch_mem reqd = %d 16M pages\n",__LINE__,prefetch_page_req);
        hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
    }
    else {
        /* If P7 /P7+, then we need to allocate exclusive memory for prefetch threads to work on */
        /* Logic behind using the Lookup-Table : Enables faster calculation of number of prefetch algos set.
         * Prefetch configuration contains values upto/within the 4bit range.Each bit set represents
         * a prefetch algorithm enabled from a rule stanza. The Lookup table contains the number of bits
         * set from numbers 0x0-0xF (4bit range).
         */

        int lookup_table[]={ 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 } ;
        /*  Figure out the maximum number of algorithms enabled at once in all the rule stanza's */
        int max_number_of_prefetch_algorithms_enabled=0, number_of_prefetch_algorithms_enabled=0;
        for(i = 0; i < num_testcases; i++) {
            number_of_prefetch_algorithms_enabled = lookup_table[(h_r[i].prefetch_configuration & 0xF)];
            if(number_of_prefetch_algorithms_enabled > max_number_of_prefetch_algorithms_enabled) {
                max_number_of_prefetch_algorithms_enabled = number_of_prefetch_algorithms_enabled;
            }
            if( number_of_prefetch_algorithms_enabled >= 0 ) {
                if( h_r[i].testcase_type == CACHE_TEST_OFF ) {
                    testcase_type = h_r[i].testcase_type;
                }
            }
            
        }

        if ( max_number_of_prefetch_algorithms_enabled ) {
            /* We need 8MB of memory,per prefetch algorithm,per physical cache instance .
              * Therefore , calculate total number of 8MB segments required for the whole gang
              */
            if (h_r[i].testcase_type == CACHE_TEST_OFF)
                total_number_of_8MB_segments_required = system_information.end_cpu_number - system_information.start_cpu_number + 1;
            else
                total_number_of_8MB_segments_required = number_of_physical_cache_instances * (system_information.smt_threads-1);

            /* Calculate total number of pages required. Round off to the next highest page */
            number_of_pages_required = (total_number_of_8MB_segments_required * (8*M))/(16*M)
                + ( (total_number_of_8MB_segments_required * (8*M))%(16*M) == 0 ? 0:1 ) ;
 
            number_of_pages_required += 1;

            /* Calculate memory required */
            worst_case_prefetch_memory_required = (number_of_pages_required) * (16*M);
            prefetch_page_req = number_of_pages_required;
            sprintf(msg,"[%d] Testcase_type = %d prefetch_mem reqd = %d 16M pages\n",__LINE__,testcase_type,prefetch_page_req);
            hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
        }
        else /* Prefetch is not enabled */
            worst_case_prefetch_memory_required = 0;
    }

    return worst_case_prefetch_memory_required;
}

/*
 * This function setup the array of thread_context's so that thread specific infomation
 * can be initialised, prior to creation of the threads themselves.
 */

void setup_thread_context_array(int thread_type)
{
    int i, j=0, index, mem_per_thread, cache_type,index_increment;

    cache_type = current_rule_stanza_pointer->test;

    /* Initialise Prefetch threads here ,if required */
    if(thread_type == PREFETCH) {

        if(current_rule_stanza_pointer->testcase_type == CACHE_TEST_OFF) {

            for(index = 0;
                index < num_prefetch_threads;
                index++) {

                th_array[index].bind_to_cpu = system_information.start_cpu_number + index;
                th_array[index].thread_no   = index;
                th_array[index].thread_type = PREFETCH;

                /* Set the prefetch algorithm for prefetch thread to run */
                th_array[index].prefetch_algorithm         = derive_prefetch_algorithm_to_use(index);
                th_array[index].start_of_contiguous_memory = (char *)find_prefetch_mem();
                    sprintf(msg,"\n Prefetch mem: %llx",th_array[index].start_of_contiguous_memory);
                    hxfmsg(&h_d,0,HTX_HE_INFO,msg);
                if ( th_array[index].start_of_contiguous_memory == NULL ){

                    sprintf(msg,"[%d]Not enough prefetch memory. Exitting !!\n",__LINE__);
                    hxfmsg(&h_d,-1,HTX_HE_HARD_ERROR,msg);
                    exit(-1);
                }

                pthread_attr_init(&th_array[index].thread_attrs_detached);
                pthread_attr_setdetachstate(&th_array[index].thread_attrs_detached, PTHREAD_CREATE_JOINABLE);

            } /* end of for loop */
        }
        else {
            int prefetch_thread_number=0;
            for(i = 0; i < num_cache_threads; i++) {
                for(j = 1; j < system_information.smt_threads; j++) {
                    index = (i * system_information.smt_threads) + j;

                    th_array[index].bind_to_cpu = system_information.start_cpu_number + index;
                    th_array[index].thread_no   = index;
                    th_array[index].thread_type = PREFETCH;
                    
                    /* Set the prefetch algorithm for prefetch thread to run */
                    th_array[index].prefetch_algorithm = derive_prefetch_algorithm_to_use(index);

                    th_array[index].start_of_contiguous_memory = (char *)find_prefetch_mem();
                    sprintf(msg,"\n Prefetch mem: %llx",th_array[index].start_of_contiguous_memory);
                    hxfmsg(&h_d,0,HTX_HE_INFO,msg);
                if ( th_array[index].start_of_contiguous_memory == NULL ){

                    sprintf(msg,"[%d]Not enough prefetch memory. Exitting !!\n",__LINE__);
                    hxfmsg(&h_d,-1,HTX_HE_HARD_ERROR,msg);
                    exit(-1);
                }

                    pthread_attr_init(&th_array[index].thread_attrs_detached);
                    pthread_attr_setdetachstate(&th_array[index].thread_attrs_detached, PTHREAD_CREATE_JOINABLE);

                    DEBUGON("prefetch thread no %d addr: %llx",index,th_array[index].start_of_contiguous_memory);
                    prefetch_thread_number++;

                } /* end of for loop */
            }
        }
    }
    /* Initialise Cache threads here ,if required */
    if(thread_type == CACHE) {


          if( current_rule_stanza_pointer->prefetch_configuration != 0 ||
            (current_rule_stanza_pointer->prefetch_configuration == 0 && current_rule_stanza_pointer->testcase_type == CACHE_BOUNCE)
            ) {
            /* If Prefetch is on OR
             * If Prefetch is off AND Testcase type is CACHE_BOUNCE
             * then increment index by smt_threads value
             */

            index_increment = system_information.smt_threads;
        }
        else if (current_rule_stanza_pointer->prefetch_configuration == 0 &&
                 current_rule_stanza_pointer->testcase_type == CACHE_ROLLOVER ) {
            /* Else if Prefetch is off AND If Testcase type is CACHE_ROLLOVER,
             * then increment index by 1
             */
            index_increment = 1;
        }

        /* Create a sync mask for threads, in which only 1 bit is set for each thread */
        for(index = 0,j=0;
            index < (num_cache_threads*system_information.smt_threads);
            index += index_increment,j++) {
            th_array[index].sync_mask = 0x1 << j;  /* turn on my bit */
        }

        if(current_rule_stanza_pointer->testcase_type == CACHE_BOUNCE) {

            /* A cache line width is split amongst the Cache threads */
            mem_per_thread = system_information.cinfo[cache_type].line_size/mem.threads_to_create;

            for(index = 0;
                index < (num_cache_threads*system_information.smt_threads);
                index += system_information.smt_threads) {

                th_array[index].bind_to_cpu = system_information.start_cpu_number + index;
                th_array[index].thread_no   = index;
                th_array[index].seedval     = current_rule_stanza_pointer->seed;
                th_array[index].thread_type = CACHE;


                th_array[index].start_class     = 0;
                th_array[index].end_class       = (2*system_information.cinfo[cache_type].asc) - 1;
                th_array[index].walk_class_jump = 1;

                /* This is the physical instance of cache that will be tested by this thread */
                th_array[index].cache_instance_under_test = 0;

                /* This is the starting address of the memory segment that will map to this physical instance of cache
                 * that will be tested by this thread
                */
                th_array[index].memory_starting_address_offset = 0;


                /* This is the start of the contiguous memory block for this thread to operate on */
                th_array[index].start_of_contiguous_memory = mem.contig_mem[0][0];

                /* This is the offset within each cache line where this thread will write/read */
                th_array[index].offset_within_cache_line =    mem_per_thread*(index/system_information.smt_threads);

                pthread_attr_init(&th_array[index].thread_attrs_detached);
                pthread_attr_setdetachstate(&th_array[index].thread_attrs_detached, PTHREAD_CREATE_JOINABLE);

            }
        }
        else if(current_rule_stanza_pointer->testcase_type == CACHE_ROLLOVER /* Roll over supported for P7 and above only */
                && current_rule_stanza_pointer->prefetch_configuration != 0 ) {

            for(index = 0;
                index < (num_cache_threads*system_information.smt_threads);
                index += system_information.smt_threads) {

                th_array[index].bind_to_cpu = system_information.start_cpu_number + index;
                th_array[index].thread_no   = index;
                th_array[index].seedval     = current_rule_stanza_pointer->seed;
                th_array[index].thread_type = CACHE;
 
                th_array[index].start_class     = 0;
                th_array[index].end_class       = (2*system_information.cinfo[cache_type].asc) - 1;
                th_array[index].walk_class_jump = 1;

                /* This is the physical instance of cache that will be tested by this thread */
                th_array[index].cache_instance_under_test = index/system_information.smt_threads;

                /* This is the starting address of the memory segment that will map to this physical instance of cache
                 * that will be tested by this thread
                 */
                th_array[index].memory_starting_address_offset =
                    th_array[index].cache_instance_under_test * 2*system_information.cinfo[cache_type].cache_size ;

                if (system_information.pvr == 0x3F) {

                    /* This is the start of the contiguous memory block for this thread to operate on */
                    th_array[index].start_of_contiguous_memory =
                        mem.contig_mem[ th_array[index].memory_starting_address_offset/(16*M) ][0]
                    + th_array[index].memory_starting_address_offset%(16*M);
                    sprintf(msg,"[%d] th_array[%d].start_of_contiguous_memory = %llx\n",__LINE__,index,th_array[index].start_of_contiguous_memory);
					 hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
                }
                else /* p7+ */ {
                    th_array[index].start_of_contiguous_memory = mem.contig_mem[th_array[index].cache_instance_under_test][0];

                }

                /* This is the offset within each cache line where this thread will write/read */
                th_array[index].offset_within_cache_line =    0;

                pthread_attr_init(&th_array[index].thread_attrs_detached);
                pthread_attr_setdetachstate(&th_array[index].thread_attrs_detached, PTHREAD_CREATE_JOINABLE);
            }
        }
        else if(current_rule_stanza_pointer->testcase_type == CACHE_ROLLOVER
                && current_rule_stanza_pointer->prefetch_configuration == 0 ) {

            for(index = 0;
                index < (num_cache_threads*system_information.smt_threads);
                index += 1) {
                
                th_array[index].bind_to_cpu = system_information.start_cpu_number + index;
                th_array[index].thread_no   = index;
                th_array[index].seedval     = current_rule_stanza_pointer->seed;
                th_array[index].thread_type = CACHE;


                th_array[index].start_class = index % system_information.smt_threads;
                th_array[index].end_class =
                    (2*system_information.cinfo[cache_type].asc) - system_information.smt_threads + th_array[index].start_class;
                th_array[index].walk_class_jump = system_information.smt_threads;

                /* This is the physical instance of cache that will be tested by this thread */
                th_array[index].cache_instance_under_test = index/system_information.smt_threads;

                /* This is the starting address of the memory segment that will map to this physical instance of cache
                 * that will be tested by this thread
                 */
                th_array[index].memory_starting_address_offset =
                    th_array[index].cache_instance_under_test * 2*system_information.cinfo[cache_type].cache_size ;

                if (system_information.pvr == 0x3F) {
                /* This is the start of the contiguous memory block for this thread to operate on */
                th_array[index].start_of_contiguous_memory =
                    mem.contig_mem[ th_array[index].memory_starting_address_offset/(16*M) ][0]
                    + th_array[index].memory_starting_address_offset%(16*M);
                }
                else {
                    th_array[index].start_of_contiguous_memory =
                        mem.contig_mem[th_array[index].cache_instance_under_test/system_information.smt_threads][0];

                }

                /* This is the offset within each cache line where this thread will write/read */
                th_array[index].offset_within_cache_line =    0;

                pthread_attr_init(&th_array[index].thread_attrs_detached);
                pthread_attr_setdetachstate(&th_array[index].thread_attrs_detached, PTHREAD_CREATE_JOINABLE);
            }
        }
    }
}

void cleanup_thread_context_array(int thread_type)
{
    int i, j, index,rc;

    sprintf(msg,"%s: called with thread_type = %d\n",__FUNCTION__,thread_type);
    hxfmsg(&h_d, 0, HTX_HE_INFO,msg);

    rc = pthread_mutex_lock( &mutex );
    if(rc != 0) {
        sprintf(msg,"[%d] %s: Could not acquire lock\n",__LINE__,__FUNCTION__);
        hxfmsg(&h_d,errno,HTX_HE_SOFT_ERROR,msg);
    }

    /* Cleanup Prefetch threads here ,if required */
    if(thread_type == PREFETCH) {

        for(i = 0; i < num_cache_threads; i++) {
            for(j = 1; j < system_information.smt_threads; j++) {
                index = (i * system_information.smt_threads) + j;

                    memset(&th_array[index], 0, sizeof(th_array[index]));

            } /* end of for loop */
             }
        } 
    /* Cleanup Cache threads here ,if required */
    if(thread_type == CACHE) {

            for(index = 0;
            index < (num_cache_threads*system_information.smt_threads);
            index +=system_information.smt_threads) {
 
                memset(&th_array[index], 0, sizeof(th_array[index]));
            }
}
    rc = pthread_mutex_unlock( &mutex );
    if(rc != 0) {
        sprintf(msg,"[%d] %s: Could not release lock\n",__LINE__,__FUNCTION__);
        hxfmsg(&h_d,errno,HTX_HE_SOFT_ERROR,msg);
    }
}

int create_threads(int thread_type)
{

    int i, j, index, index_increment,rc;

    if(thread_type == PREFETCH) {

        if(current_rule_stanza_pointer->testcase_type == CACHE_TEST_OFF) {
           	sprintf(msg,"[%d] Test case = CACHE_TEST_OFF, Prefetch threads created = %d",__LINE__,num_prefetch_threads);
           	hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
        	if( !exit_flag ) {
            	for(index = 0;
                	index < num_prefetch_threads;
                	index++) {

                	if(pthread_create(&th_array[index].tid, &th_array[index].thread_attrs_detached,
                    	              (void *(*)(void *))prefetch_irritator, (void *)&th_array[index]) ) {

                    /* If create threads fails, perform cleanup and return fail(-1) to calling function */
                    sprintf(msg, "Prefetch pthread_create call failed with %d \n", errno);
                    hxfmsg(&h_d, errno, HTX_HE_HARD_ERROR, msg);
                    sprintf(msg,"[%d] calling cleanup prefetch \n",__LINE__);
                    hxfmsg(&h_d, 0, HTX_HE_INFO, msg); 
                    cleanup_threads(PREFETCH);
                    memory_set_cleanup();
                    return (-1);
                } /* end of if */
                else {
                    mem.prefetch_threads_created++;
                } /* end of else */

            	}
            	return 0;
        	}
		}
        else {
            sprintf(msg,"Prefetch threads created = %d",num_cache_threads*(system_information.smt_threads - 1));
            hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
			if( !exit_flag ) {
            	for(i = 0; i < num_cache_threads; i++) {
                	for(j = 1; j < system_information.smt_threads; j++) {

                    	index = (i * system_information.smt_threads) + j;
                    	if(pthread_create(&th_array[index].tid, &th_array[index].thread_attrs_detached,
                                      	(void *(*)(void *))prefetch_irritator, (void *)&th_array[index]) ) {

                        	/* If create threads fails, perform cleanup and return fail(-1) to calling function */
                        	sprintf(msg, "Prefetch pthread_create call failed with %d \n", errno);
                        	hxfmsg(&h_d, errno, HTX_HE_HARD_ERROR, msg);
                        	sprintf(msg,"[%d] calling cleanup prefetch \n",__LINE__);
                        	hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
                        	cleanup_threads(PREFETCH);
                        	memory_set_cleanup();
                        	return (-1);
                    	}                                        /* end of if             */
                    	else {
                        	mem.prefetch_threads_created++;
                    	}                                        /* end of else           */

                	} /* end of for loop */
				}
            }
            return 0;

        }
    }
    else if(thread_type == CACHE) {

        if( current_rule_stanza_pointer->prefetch_configuration != 0 ||
            (current_rule_stanza_pointer->prefetch_configuration == 0 && current_rule_stanza_pointer->testcase_type == CACHE_BOUNCE)
            ) {
            /* If Prefetch is on OR
             * If Prefetch is off AND Testcase type is CACHE_BOUNCE
             * then increment index by smt_threads value
             */
          
            index_increment = system_information.smt_threads;
                }
        else if (current_rule_stanza_pointer->prefetch_configuration == 0 &&
                 current_rule_stanza_pointer->testcase_type == CACHE_ROLLOVER ) {
            /* Else if Prefetch is off AND If Testcase type is CACHE_ROLLOVER,
             * then increment index by 1
             */
            index_increment = 1;
        }
        total_cache_threads = ((num_cache_threads*system_information.smt_threads)/index_increment);
        init_sync_words((total_cache_threads-1));

		if( !exit_flag ) {
        	sprintf(msg,"Total cache threads being created = %d\n",((num_cache_threads*system_information.smt_threads)/index_increment));
            hxfmsg(&h_d, 0, HTX_HE_INFO, msg);
            for(index = 0;
            	index < (num_cache_threads*system_information.smt_threads);
            	index += index_increment) {


                if(pthread_create(&th_array[index].tid, &th_array[index].thread_attrs_detached,
                                  (void *(*)(void *))write_read_and_compare, (void *)&th_array[index]) ) {
                    /* If create threads fails, perform cleanup and return fail(-1) to calling function */
                    sprintf(msg, "pthread_create failed with %d \n", errno);
                    hxfmsg(&h_d, errno, HTX_HE_HARD_ERROR, msg);
                    cleanup_threads(ALL);
                    memory_set_cleanup();
                    return (-1);
                }
                else {
                    mem.threads_created++;
                }
 			} /* End of for */

        }	/* End of outer if */
        return 0;
    }
}

void cleanup_threads(int thread_type)
{
    int thread_no , rc, th_rc;
    void *tresult = (void *)th_rc ;

    switch(thread_type) {

    case PREFETCH:
        if( num_prefetch_threads == 0) {
            sprintf(msg,"[%d] No prefetch threads to be cleaned up\n",__LINE__);
            hxfmsg(&h_d, 0, HTX_HE_INFO , msg);
        }
        /*for(thread_no = 0; thread_no< MAX_CPUS; thread_no++) {*/
        for(thread_no = 0; thread_no< (num_prefetch_threads+total_cache_threads); thread_no++) {

            if(th_array[thread_no].thread_type != PREFETCH) {
                continue;
            }

            if (current_rule_stanza_pointer->testcase_type == CACHE_TEST_OFF) {
                /* If cache test is off,i.e we are in prefetch only mode , therefore
                 * wait on join for prefetch threads to complete num_oper iterations
                 * and join back */

                rc = pthread_join(th_array[thread_no].tid, &tresult);
                if (rc != 0) {
                    sprintf(msg,"[%d]cleanup_threads:pthread_join failed ! errno %d :(%d): tnum=%d\n",
                            __LINE__,errno, rc, th_array[thread_no].tid);
                    hxfmsg(&h_d, HTX_HE_HARD_ERROR , rc , msg);
                }
                else {
                    th_array[thread_no].tid = -1;
                    th_array[thread_no].thread_type = ALL;

                    sprintf(msg,"[%d] Prefetch Thread %d Just Joined\n",__LINE__,thread_no);
                    hxfmsg(&h_d, 0, HTX_HE_INFO , msg);
                }
            }
            else {
                /* If any other type of test, prefetch threads will run infinitely, therefore
                 * cancel each thread, and then join
                 */
                rc = pthread_cancel(th_array[thread_no].tid);
                if( (rc != 0) && (rc != ESRCH) ){
                    sprintf(msg, "[%d] cleanup_threads: tnum:%d tid = %d pthread_cancel returned %d\n",__LINE__, thread_no,th_array[thread_no].tid, rc);
                    hxfmsg(&h_d, HTX_HE_HARD_ERROR , rc , msg);
                }
                else {
                    DEBUGON("Prefetch Thread %d Just Cancelled\n",thread_no);
                    rc = pthread_join(th_array[thread_no].tid, &tresult);
                    if (rc != 0) {
                        sprintf(msg,"[%d]cleanup_threads:pthread_join failed ! errno %d :(%d): tnum=%d\n",
                                __LINE__,errno, rc, th_array[thread_no].tid);
                        hxfmsg(&h_d, HTX_HE_HARD_ERROR , rc , msg);
                    }
                    else {
                        th_array[thread_no].tid = -1;
                        th_array[thread_no].thread_type = ALL;
                        sprintf(msg,"[%d] Prefetch Thread %d Just Joined\n",__LINE__,thread_no);
                        hxfmsg(&h_d, 0, HTX_HE_INFO , msg);
                    }
                }
            }
        }

        mem.prefetch_threads_created=0;
        break;

    case CACHE:
        for(thread_no = 0; thread_no< (num_prefetch_threads+total_cache_threads); thread_no++){

            if(th_array[thread_no].thread_type != CACHE){
                continue;
            }
            else {
                rc = pthread_join(th_array[thread_no].tid, &tresult);
                if ((rc != 0)&& (rc != ESRCH) ) {
                    sprintf(msg,"[%d]cleanup_threads:pthread_join failed ! errno %d :(%d): tnum=%d\n",
                            __LINE__,errno, rc, th_array[thread_no].tid);
                    hxfmsg(&h_d, HTX_HE_HARD_ERROR , rc , msg);
                }
                else {
                    th_array[thread_no].tid = -1;
                    th_array[thread_no].thread_type = ALL;
                    sprintf(msg,"[%d] Cache Thread %d Just Joined\n",__LINE__,thread_no);
                    hxfmsg(&h_d, 0, HTX_HE_INFO , msg);
                }
            }
         }

        mem.threads_created=0;
        break;

    case ALL:
        sprintf(msg,"[%d] calling cleanup prefetch \n",__LINE__);
        hxfmsg(&h_d, 0, HTX_HE_INFO , msg);
        cleanup_threads(PREFETCH);
        sprintf(msg,"[%d] calling cleanup cache \n",__LINE__);
        hxfmsg(&h_d, 0, HTX_HE_INFO , msg);
        cleanup_threads(CACHE);
        break;
    }
}

int derive_prefetch_algorithm_to_use(int index) {

    #ifndef __HTX_LINUX__
        if((system_information.pvr == 0x3E) || ((system_information.pvr == 0x3F) && (_system_configuration.implementation == POWER_6))) {
            return PREFETCH_IRRITATOR;
        }
    #else
        if(system_information.pvr == 0x3E) {
            return PREFETCH_IRRITATOR;
        }
    #endif
    else if(system_information.pvr == 0x3F || system_information.pvr == 0x4A) {

        if(system_information.smt_threads == 2) {
            return ROUND_ROBIN_ALL_ENABLED_PREFETCH_ALGORITHMS;
        }
        else if(system_information.smt_threads == 4) {

            switch(index%4) {
            case 0:
                if(PREFETCH_IRRITATOR & current_rule_stanza_pointer->prefetch_configuration == PREFETCH_IRRITATOR) {
                    return PREFETCH_IRRITATOR;
                }
                else if(PREFETCH_NSTRIDE & current_rule_stanza_pointer->prefetch_configuration == PREFETCH_NSTRIDE) {
                    return PREFETCH_NSTRIDE;
                }
                else if(PREFETCH_TRANSIENT & current_rule_stanza_pointer->prefetch_configuration == PREFETCH_TRANSIENT) {
                    return PREFETCH_TRANSIENT;
                }
                else if(PREFETCH_PARTIAL & current_rule_stanza_pointer->prefetch_configuration == PREFETCH_PARTIAL) {
                    return PREFETCH_PARTIAL;
                }
                break;

            case 1:
                if(PREFETCH_NSTRIDE & current_rule_stanza_pointer->prefetch_configuration == PREFETCH_NSTRIDE) {
                    return PREFETCH_NSTRIDE;
                }
                else if(PREFETCH_TRANSIENT & current_rule_stanza_pointer->prefetch_configuration == PREFETCH_TRANSIENT) {
                    return PREFETCH_TRANSIENT;
                }
                else if(PREFETCH_PARTIAL & current_rule_stanza_pointer->prefetch_configuration == PREFETCH_PARTIAL) {
                    return PREFETCH_PARTIAL;
                }
                else if(PREFETCH_IRRITATOR & current_rule_stanza_pointer->prefetch_configuration == PREFETCH_IRRITATOR) {
                    return PREFETCH_IRRITATOR;
                }
                break;

            case 2:
                if(PREFETCH_TRANSIENT & current_rule_stanza_pointer->prefetch_configuration == PREFETCH_TRANSIENT) {
                    return PREFETCH_TRANSIENT;
                }
                else if(PREFETCH_PARTIAL & current_rule_stanza_pointer->prefetch_configuration == PREFETCH_PARTIAL) {
                    return PREFETCH_PARTIAL;
                }
                else if(PREFETCH_IRRITATOR & current_rule_stanza_pointer->prefetch_configuration == PREFETCH_IRRITATOR) {
                    return PREFETCH_IRRITATOR;
                }
                else if(PREFETCH_NSTRIDE & current_rule_stanza_pointer->prefetch_configuration == PREFETCH_NSTRIDE) {
                    return PREFETCH_NSTRIDE;
                }
                break;

            case 3:
                if(PREFETCH_PARTIAL & current_rule_stanza_pointer->prefetch_configuration == PREFETCH_PARTIAL) {
                    return PREFETCH_PARTIAL;
                }
                else if(PREFETCH_IRRITATOR & current_rule_stanza_pointer->prefetch_configuration == PREFETCH_IRRITATOR) {
                    return PREFETCH_IRRITATOR;
                }
                else if(PREFETCH_NSTRIDE & current_rule_stanza_pointer->prefetch_configuration == PREFETCH_NSTRIDE) {
                    return PREFETCH_NSTRIDE;
                }
                else if(PREFETCH_TRANSIENT & current_rule_stanza_pointer->prefetch_configuration == PREFETCH_TRANSIENT) {
                    return PREFETCH_TRANSIENT;
                }
                break;

            default:
                DEBUGON("%s:Invalid index number passed %d\n", __FUNCTION__, index);
                return -1;
                break;
            } /* End of switch case */

        } /* End of else if */

    } /* End of else if */
}

#if defined (__HTX_LINUX__)
int get_logical_to_physical_cpu(int cpu)
{

    static int entry=0;
    unsigned int *ptr;
    int count = 0,i;

    ptr = &default_cpu_affinity_mask;

    for(i=0; i < sizeof(default_cpu_affinity_mask)/4; i++) {
        DEBUGON("%08x\t",*ptr); 
        ptr++;
    }
    DEBUGON("Find map for:%d\n",cpu); 

    for(i=0; i < sizeof(default_cpu_affinity_mask) * 8; i++) {
        if(CPU_ISSET(i, &default_cpu_affinity_mask)) {
            DEBUGON("Found:%d i:%d\n",count,i); 
            count++;
        }
        if((count-1) == cpu)
            break;
    }
    if((count-1) == cpu)
        return i;
    else
        return -1;
}
#endif

/* Function to parse rule_id which ends with a stanza number 	*/
/* and returns the stanza number 								*/
int get_test_id(char *rule_id) {

	char *id;
	int stanza_num;
	
	strsep(&rule_id,"_");
	stanza_num = atoi(rule_id);

	return (stanza_num);
	
}
