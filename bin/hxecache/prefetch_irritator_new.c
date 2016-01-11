/* @(#)19       1.7.1.9  src/htx/usr/lpp/htx/bin/hxecache/prefetch_irritator_new.c, exer_cache, htxubuntu 8/2/12 02:40:51  */

#include "hxecache_new.h"
#define BYTES_EXC 16    /* Last few bytes of segment where it loads from has to be within segemnt */
                        /* ld => 16 bytes  */
/* Layout of thread specific data segment
 * address:            .long 0,0
 * endaddress:        .long 0,0
 * time_seed:        .long   0x0
 * mask:            .long 0x3ffffff0  # GO/STOP and STREAM IDS are set to 0 [only in prefetch.s]
 * direction:        .long 0x0
 * proc_no:            .long proc_no
 * 64 bytes
 */

extern struct htx_data          h_d;
extern char                     msg[500];
extern struct memory_set        mem;
extern struct thread_context    th_array[MAX_CPUS];
extern volatile int             exit_flag;
extern void                     prefetch(unsigned long long , unsigned long long , unsigned long long ,unsigned long long*);
extern void                     n_stride(unsigned long long , unsigned long long , unsigned long long ,unsigned long long*);
extern void                     transient_dcbt(unsigned long long , unsigned long long , unsigned long long ,unsigned long long*);
extern void                     partial_dcbt(unsigned long long , unsigned long long , unsigned long long ,unsigned long long*);
extern struct sys_info          system_information;
extern volatile struct ruleinfo *current_rule_stanza_pointer;
unsigned long int               get_random_number_perf(int );

#ifdef __HTX_LINUX__
    extern cpu_set_t default_cpu_affinity_mask;
#endif

void prefetch_irritator(void *arg)
{
    int i, rc, no_of_pages, tid , thread_no, tc, oper , number_of_operations;
    unsigned long long  saved_seed, random_no , starting_address , memory_fetch_size;

    struct thread_context *th = (struct thread_context *)arg;

    /*
     * char *contig_mem[NUM_SEGS*SEG_SIZE/(16*M)]; Physically contiguous
     * memory pointer. memory_set_size variable  gives total memory
     * allocated both are variables of global structure.
     */

    thread_no = th->thread_no ;
    tid       = th->bind_to_cpu;    /* Bind to the processor */

    if (current_rule_stanza_pointer->testcase_type != CACHE_TEST_OFF) {
        /* Set Thread Cancel Type as ASYNCHRONOUS */
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    }

    #ifdef __HTX_LINUX__
        int physical_cpu;
        cpu_set_t cpu_affinity_mask;
        CPU_ZERO(&cpu_affinity_mask);
    
        physical_cpu = get_logical_to_physical_cpu(tid);
        if(physical_cpu!=-1){
            CPU_SET(physical_cpu, &cpu_affinity_mask);
        }
        rc = sched_setaffinity(syscall(SYS_gettid), sizeof(cpu_affinity_mask), &cpu_affinity_mask) ;
    #else
        rc = bindprocessor(BINDTHREAD, thread_self(), tid);
    #endif
        if(rc == -1) {
            #ifdef __HTX_LINUX__  /*Debug printf being added for more info on cpu mask, if cpu bind fails */
            {
                unsigned long i;
                unsigned long *ptr;
                ptr=&default_cpu_affinity_mask;
                sprintf(msg,"::Start of Printing Default cpu mask::\n");
                hxfmsg(&h_d, errno, HTX_HE_SOFT_ERROR, msg);

                for(i=0;i < sizeof(default_cpu_affinity_mask); i=i+sizeof(unsigned long) ){
                    /*Cpu mask is implemented as an array of unsigned long's */
                    sprintf(msg,"default_cpu_affinity_mask[%3d]:%16lx\n", (i/sizeof(unsigned long)), *ptr);
                    hxfmsg(&h_d, errno, HTX_HE_SOFT_ERROR, msg);
                    ptr++;
                }
                sprintf(msg,"::End of Printing Default cpu mask::\n");
                hxfmsg(&h_d, errno, HTX_HE_SOFT_ERROR, msg);
            }
            /*Debug printf being added for more info on cpu mask, if cpu bind fails */
            {
                unsigned long i;
                unsigned long *ptr;
                ptr=&cpu_affinity_mask;
                sprintf(msg,"::Start of Printing cpu mask::\n");
                hxfmsg(&h_d, errno, HTX_HE_SOFT_ERROR, msg);

                for(i=0;i < sizeof(cpu_affinity_mask); i=i+sizeof(unsigned long) ){
                    /*Cpu mask is implemented as an array of unsigned long's */

                    sprintf(msg,"cpu_affinity_mask[%3d]:%16lx\n", (i/sizeof(unsigned long)), *ptr);
                    hxfmsg(&h_d, errno, HTX_HE_SOFT_ERROR, msg);
                    ptr++;
                }
                sprintf(msg,"::End of Printing cpu mask::\n");
                hxfmsg(&h_d, errno, HTX_HE_SOFT_ERROR, msg);
            }

            /* Set exit_flag here to terminate testcase */
            /*exit_flag = 1;*/
            exit_flag = ERR_EXIT;
            #endif

            sprintf(msg, "%d: Bindprocessor for prefetch irritator on cpu:%d failed with %d\n", __LINE__, tid, errno);
            hxfmsg(&h_d, errno, HTX_HE_HARD_ERROR, msg);
        } /* End of if */
        else {
            #ifdef DEBUG
            sprintf(msg,"Bindprocessor success [prefetch]! cpu_no : %d \n",tid);
            hxfmsg(&h_d, errno, HTX_HE_INFO, msg);
            #endif
        }

        th->seedval = time(NULL);
        srand48_r(th->seedval,&th->buffer);

        /* Interpret num_oper of rule file, only if running only prefetch threads */
        if (current_rule_stanza_pointer->testcase_type == CACHE_TEST_OFF) {
            number_of_operations = current_rule_stanza_pointer->num_oper;
        }
        else {
            number_of_operations = 0;
        }

       for (oper = 0; oper < number_of_operations || number_of_operations == 0  ; oper++) {

        /* For CACHE_TEST_OFF,check exit flag is set i.e if SIGTERM was received. If yes, Exit */
        if(current_rule_stanza_pointer->testcase_type == CACHE_TEST_OFF && exit_flag != 0) {
            break;
        }

        /*random_no = get_random_number_perf(thread_no);
        random_no = (unsigned long long)(random_no<<32) | (random_no);*/
		random_no = 0xaabbccdd;
        th_array[thread_no].prev_seed = random_no;

        if (th_array[thread_no].prefetch_algorithm == ROUND_ROBIN_ALL_ENABLED_PREFETCH_ALGORITHMS) {
            /* Run all the enabled prefetch variants in round robin method */

            starting_address = (unsigned long long)(th_array[thread_no].start_of_contiguous_memory);
            memory_fetch_size = (8*M) - BYTES_EXC ;

            /* If prefetch nstride is set in the current prefetch configuration */
            if (PREFETCH_NSTRIDE & current_rule_stanza_pointer->prefetch_configuration == PREFETCH_NSTRIDE ) {
                n_stride(starting_address,memory_fetch_size,random_no,&th_array[thread_no].prefetch_scratch_mem[0]);
            }

            /* For CACHE_TEST_OFF,check exit flag is set i.e if SIGTERM was received. If yes, Exit */
            if (current_rule_stanza_pointer->testcase_type == CACHE_TEST_OFF && exit_flag != 0) {
                break;
            }

            /* If prefetch partial is set in the current prefetch configuration */
            if (PREFETCH_PARTIAL & current_rule_stanza_pointer->prefetch_configuration == PREFETCH_PARTIAL ) {
                partial_dcbt(starting_address,memory_fetch_size,random_no,&th_array[thread_no].prefetch_scratch_mem[0]);
            }

            /* For CACHE_TEST_OFF,check exit flag is set i.e if SIGTERM was received. If yes, Exit */
            if (current_rule_stanza_pointer->testcase_type == CACHE_TEST_OFF && exit_flag != 0) {
                break;
            }

            if ( PREFETCH_IRRITATOR & current_rule_stanza_pointer->prefetch_configuration == PREFETCH_IRRITATOR ) {
                prefetch(starting_address,memory_fetch_size,random_no,&th_array[thread_no].prefetch_scratch_mem[0]);
            }

            /* For CACHE_TEST_OFF,check exit flag is set i.e if SIGTERM was received. If yes, Exit */
            if (current_rule_stanza_pointer->testcase_type == CACHE_TEST_OFF && exit_flag != 0) {
                break;
            }

            if( PREFETCH_TRANSIENT & current_rule_stanza_pointer->prefetch_configuration == PREFETCH_TRANSIENT ) {
                transient_dcbt(starting_address, memory_fetch_size,random_no, &th_array[thread_no].prefetch_scratch_mem[0]);
            }

            /* For CACHE_TEST_OFF,check exit flag is set i.e if SIGTERM was received. If yes, Exit */
            if (current_rule_stanza_pointer->testcase_type == CACHE_TEST_OFF && exit_flag != 0) {
                break;
            }
        }
        else { /* Else Run only the specified algorithm */
            starting_address = (unsigned long long)(th_array[thread_no].start_of_contiguous_memory);
            memory_fetch_size = (8*M) - BYTES_EXC;

            if(th_array[thread_no].prefetch_algorithm == PREFETCH_NSTRIDE) {
                n_stride(starting_address, memory_fetch_size, random_no, &th_array[thread_no].prefetch_scratch_mem[0]);
            }
            else if(th_array[thread_no].prefetch_algorithm == PREFETCH_PARTIAL) {
                partial_dcbt(starting_address, memory_fetch_size, random_no, &th_array[thread_no].prefetch_scratch_mem[0]);
            }
            else if(th_array[thread_no].prefetch_algorithm == PREFETCH_TRANSIENT) {
                transient_dcbt(starting_address, memory_fetch_size, random_no, &th_array[thread_no].prefetch_scratch_mem[0]);
            }
            else if(th_array[thread_no].prefetch_algorithm == PREFETCH_IRRITATOR) {
                prefetch(starting_address, memory_fetch_size, random_no, &th_array[thread_no].prefetch_scratch_mem[0]);
            }

            /* For CACHE_TEST_OFF,check exit flag is set i.e if SIGTERM was received. If yes, Exit */
            if(current_rule_stanza_pointer->testcase_type == CACHE_TEST_OFF && exit_flag != 0) {
                break;
            }
        }
    } /* End of for loop */
}

unsigned long int get_random_number_perf(int index)
{
    unsigned long int val_8 ;

    /*lrand48_r(&th_array[index].buffer, &th_array[index].random_pattern);*/
    val_8 = (th_array[index].random_pattern <<16)| th_array[index].random_pattern ;

    return val_8 ;
}
