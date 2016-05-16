/* IBM_PROLOG_BEGIN_TAG */
/*
 * Copyright 2003,2016 IBM International Business Machines Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* IBM_PROLOG_END_TAG */


#include "hxiipc64.h" 
#include "hxihtx64.h"

/* Global htxmp data-structure */
   typedef struct {
        /* local_htx_ds keeps track of thread specific stats */
        struct htx_data local_htx_ds;
        /* Read/Write lock for updating stats per thread basis */
        pthread_mutex_t mutex_lock;
   } mp_struct;

mp_struct * global_mp_struct = NULL;  
unsigned int num_threads = 0;
struct htx_data * global_mp_htx_ds ; 

/* This flag lets us choose b/w prev version of mp 
   and the new version
 */ 
   int new_mp = 0; 
/* This flag tells whether mp_instialization was 
	successful or not */ 
	int new_mp_initialize = 0;
/* This thread will make stats update thread wait,
   until exer had made first call to hxfupdate. 
*/ 
	int exer_stats_updated = 0; 
/* Thread index dictated by htxmp library when each thread
   call mp_start() 
 */ 
	int th_index = -1; 	
/* Exit stats for mp stats thread */ 
   int exit_update_td = 0; 
 
/* Stats thread thread attribute */ 
	pthread_attr_t stats_th_attr; 
	pthread_t stats_th_tid; 		

/* mp_start needs a global mutex lock to generate, 
   	unique number each time. */ 
   	pthread_mutex_t mutex_start; 	
	pthread_mutex_t create_thread_mutex;
	pthread_cond_t create_thread_cond;
	pthread_cond_t start_thread_cond;
	pthread_once_t mpstart_onceblock = PTHREAD_ONCE_INIT;


pthread_mutex_t hxfupdate_mutex; 
pthread_mutexattr_t   mta;

pthread_mutex_t hxfpat_mutex;
pthread_once_t hxfpat_onceblock = PTHREAD_ONCE_INIT;

pthread_once_t hxfsbuf_onceblock = PTHREAD_ONCE_INIT;
pthread_mutex_t hxfsbuf_mutex;

pthread_once_t hxfcbuf_onceblock = PTHREAD_ONCE_INIT;
pthread_mutex_t hxfcbuf_mutex;

pthread_once_t hxfbindto_a_cpu_onceblock = PTHREAD_ONCE_INIT;
pthread_mutex_t hxfbindto_a_cpu_mutex;


	
/* function prtotypes */
/* below prototypes if changed here should also be updated in hxihtxmp.h(aix) as they are duplicated */
/* to get rid of funcn prototype warnings when hxihtxmp.h is included anywhere */

int hxfmsg_tsafe(struct htx_data *p,int err, int sev, char * text);
int hxfupdate_tsafe(char type,struct htx_data * arg);
int hxfpat_tsafe(char *filename, char *pattern_buf, int num_chars);
int hxfsbuf_tsafe(char *buf, size_t len, char *fname, struct htx_data *ps);
int hxfcbuf_tsafe(struct htx_data *ps, char *wbuf, char *rbuf, size_t len, char *msg);

int hxfupdate_tunsafe(char call, struct htx_data *data);
int hxfpat_tefficient(char *filename, char *pattern_buf, int num_chars);
int hxfsbuf_tefficient(char *buf, size_t len, char *fname, struct htx_data *ps);
int hxfcbuf_calling_hxfsbuf_tsafe(struct htx_data *ps, char *wbuf, char *rbuf, size_t len,
	char *msg);
void update_stats(void * args); 
