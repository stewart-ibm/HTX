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
static char sccsid[] = "@(#)65	1.10  src/htx/usr/lpp/htx/bin/hxerng/hxerng.c, exer_rng, htxubuntu 6/3/14 07:12:14";

#include "hxerng.h"


#define		PTHREAD_WAIT(a,b)		{ \
										if ( exit_flag == FALSE ) { \
											pthread_cond_wait(a,b); \
										} \
									}



struct sigaction sigvector;
struct htx_data hd;

/* global data structure for each 1Mb slot status */
short status[MAX_SLOTS]; /* Global array to keep status of any slot in big buffer : 0=Not Populated, 1=Populated but in bits, 2=converted to bytes, 3-17=so many tests done */
short num_th[MAX_SLOTS]; /* Global array to keep track of how many tester threads are already created */
short conv_buf_no, test_buf_no;

/* Pthread mutex and conditionals variables */
pthread_mutex_t status_mutex1, status_mutex2, status_mutex3;
pthread_cond_t update_cond1, update_cond2, update_cond3;

/* global pointers to buffers that contain random numbers. */
char *rnd_bit_ptr;
char *rnd_byte_ptr[3];

unsigned int pvr, shifted_pvr_hw, shifted_pvr_os;
int core_num, dev_fd;
short exit_flag = FALSE;
short no_write = TRUE;
FILE *dlog;

#if 0

/* #1
 * All the #1 parts are just for testing.
 * Those are to be removed once library interface for reading rng in AIX comes up.
 * Till then using libc call to generate rng and test those.
 */
struct drand48_data rnd_buf;
#endif

pthread_t tid[NUM_TESTS + EXTRA_THS];
pthread_attr_t attr;

int num_active_rules;
struct rule_parameters rule;
struct rule_parameters rule_list[MAX_NUM_RULES];

#ifdef __HTX_OPEN_SOURCE__
worker client_func[NUM_TESTS + EXTRA_THS] = {
	&read_rn_func,
	&convert_bit_2_byte,
	&freq,
	&block_freq,
	&runs,
	&longest_run_of_ones,
	&rank,
	&dft,
	&non_overlapping_tm,
	&overlapping_tm,
	&universal,
	&approx_entropy,
	&cusum,
	&random_excursion,
	&random_excursion_var,
	&serial,
	&linear_complexity,
};
#endif /* __HTX_OPEN_SOURCE__ */


int main(int argc, char *argv[])
{
	int i, rc;
#ifdef __HTX_OPEN_SOURCE__
	int th_no[NUM_TESTS + EXTRA_THS];
#endif /*__HTX_OPEN_SOURCE__*/

	/* Register SIGTERM handler */
	sigemptyset((&sigvector.sa_mask));
	sigvector.sa_flags = 0;
	sigvector.sa_handler = (void (*)(int)) sigterm_hdl;
	sigaction(SIGTERM, &sigvector, (struct sigaction *) NULL);

	/* Register SIGUSR1 handler */
	sigvector.sa_handler = (void (*)(int)) sigusr1_hdl;
	sigaction(SIGUSR1, &sigvector, (struct sigaction *) NULL);

	strcpy(dinfo.run_type, "OTH");

#ifndef AWAN
#ifdef DEBUG
	dlog = fopen("/tmp/rng_log","w");
	if ( dlog == NULL ) {
		sprintf(msg,"Erro opening debug log /tmp/rng_log");
		hxfmsg(&hd, -1, HTX_HE_HARD_ERROR, msg);
	}
#endif /* DEBUG */

	/* Parse command-line arguments. */
	if ( argv[1] ) strcpy(dinfo.device_name, argv[1]);
	if ( argv[2] ) strcpy(dinfo.run_type, argv[2]);
	if ( argv[3] ) strcpy(dinfo.rule_file_name, argv[3]);

	/* Set htx_data structure parameters */
	if ( argv[0] ) strcpy(hd.HE_name, argv[0]);
	if ( argv[1] ) strcpy(hd.sdev_id, argv[1]);
	if ( argv[2] ) strcpy(hd.run_type, argv[2]);

	if ( argv[1] ) {
		char *ptr = (char *)&(argv[1]);
		core_num = i = 0;
		while ( i < strlen(ptr) ) {
			if ( *(ptr+i) >= '0' || *(ptr+i) <= '9' ) {
				core_num = core_num*10 + *(ptr+i) - '0';
			}
			i++;
		}
	}
#endif /* AWAN */

	DPRINT(dlog, "1st print. Before calling hxfupdate with START.\n");
	FFLUSH(dlog);
	hxfupdate(START, &hd);

	if ( exit_flag ) {
		exit(0);
	}


#if 0
	/* #1
 	 * All the #1 parts are just for testing.
 	 * Those are to be removed once library interface for reading rng in AIX comes up.
 	 * Till then using libc call to generate rng and test those.
 	 */
	set_rng_gen();
#endif

	DPRINT(dlog, "Calling get_PVR.\n");
	FFLUSH(dlog);

	if ( get_PVR() ) {
		sprintf(msg, "get_PVR() returned P7 or less. Unsupported Architecture!! Exiting...\n");
		hxfmsg(&hd, -1, HTX_HE_HARD_ERROR, msg);
		exit(1);
	}
	DPRINT(dlog, "after PVR check. PVR=%x, H/W PVR=%x, OS PVR=%x\n", pvr, shifted_pvr_hw, shifted_pvr_os);
	FFLUSH(dlog);

	rc = check_rng_available();
	DPRINT(dlog, "rng available  = %d\n", rc);
	FFLUSH(dlog);

	if ( rc ) {
		sprintf(msg, "Random Number Generator unit not available. Exiting...\n");
		hxfmsg(&hd, -1, HTX_HE_HARD_ERROR, msg);
		exit(1);
	}

	if ( allocate_mem() ) {
		sprintf(msg, "Error in allocating shared memory. Exiting...\n");
		hxfmsg(&hd, -1, HTX_HE_HARD_ERROR, msg);
		exit(1);
	}
	DPRINT(dlog, "Mem allocated successfully.\n");
	FFLUSH(dlog);

	if ( read_rf() ) {
		sprintf(msg, "Error reading rule-file. Exiting...\n");
		hxfmsg(&hd, -1, HTX_HE_HARD_ERROR, msg);
		exit(1);
	}
	DPRINT(dlog, "Rulefile reading done.\n");
	FFLUSH(dlog);


	/* Setting thread attribute, mutex and conditional variables */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	pthread_mutex_init(&status_mutex1, NULL);
	pthread_mutex_init(&status_mutex2, NULL);
	pthread_mutex_init(&status_mutex3, NULL);
	pthread_cond_init(&update_cond1, NULL);
	pthread_cond_init(&update_cond2, NULL);
	pthread_cond_init(&update_cond3, NULL);

	/*  Loop forever for REG mode. Exit on SIGTERM only. */
	do {
		int rule_no;

		DPRINT(dlog, "Num of active rules = %d, total threads = %d\n", num_active_rules, (NUM_TESTS + EXTRA_THS));
		FFLUSH(dlog);

		for ( rule_no = 0; rule_no < num_active_rules; rule_no++ ) {
			DPRINT(dlog, "rule no#%d out of %d\n", rule_no, num_active_rules);
			FFLUSH(dlog);
			/* Copy the active rule from rule_list array to global rule variable. */
			memcpy((void *)&rule, (const void *)&rule_list[rule_no], sizeof(struct rule_parameters));
			memset(&(status[0]), 0, sizeof(status[MAX_SLOTS]));
			memset(&(num_th[0]), 0, sizeof(num_th[MAX_SLOTS]));
			conv_buf_no = test_buf_no = 0;

#ifdef __HTX_OPEN_SOURCE__
			for ( i = 0; i < (NUM_TESTS + EXTRA_THS); i++ ) {
				/* Initialize pthread arguments */
				th_no[i] = i;

				DPRINT(dlog, "Creating th#%d\n", i);
				FFLUSH(dlog);
				/* Create the pthreads. */
				if ( pthread_create(&(tid[i]), &attr, (void *(*)(void *))client_func[i], (void *)&th_no[i]) ) {
					int j;
					sprintf(msg, "pthread_create failed for thread:%d with error(%d):%s\n",
							i, errno, strerror(errno));
					hxfmsg(&hd, errno, HTX_HE_HARD_ERROR, msg);
					for( j = 0; j < i; j++ ) {
						pthread_cancel(tid[j]);
						DPRINT(dlog, "Error while creating th#%d, cancelling th#%d\n", i, j);
						FFLUSH(dlog);
					}

					clean_up();
					exit(1);
				}
			}
			DPRINT(dlog, "Threads created successfully.\n");
			FFLUSH(dlog);

			for ( i = 0; i < (NUM_TESTS + EXTRA_THS); i++ ) {
				DPRINT(dlog, "Joining to th#%d\n", i);
				FFLUSH(dlog);

				if ( pthread_join(tid[i], NULL) ) {
					sprintf(msg, "pthread_join failed for thread:%d with error(%d):%s",
							i, errno, strerror(errno));
					hxfmsg(&hd, errno, HTX_HE_HARD_ERROR, msg);
				}
			}
			DPRINT(dlog, "Threads joined successfully.\n");
			FFLUSH(dlog);

#else
			/* This part will be invoked for standalone code (No open source NIST algo).
			 * In this code path, we just need to keep reading rng from hardware and discarding it.
			 */

			long loop_cnt=0;
			while ( (loop_cnt < (rule.num_oper * 100000)) && exit_flag == 0 ) {
				read_rn();
				loop_cnt++;
			}
			DPRINT(dlog, "generated %d random numbers. No testing", loop_cnt);
			FFLUSH(dlog);
#endif /* __HTX_OPEN_SOURCE__ */
			if ( exit_flag ) {
				clean_up();
				exit(0);
			}

			DPRINT(dlog, "Num_oper#%d done out of total#%d.\n", rule_no, num_active_rules);
			FFLUSH(dlog);

			hd.good_others = rule.num_oper;
			hd.num_instructions = rule.num_oper * STREAM_SIZE;
			hd.test_id = rule_no;
			hxfupdate(UPDATE, &hd);
		}
		hxfupdate(FINISH, &hd);
		DPRINT(dlog, "Rulefile completed.\n");
		FFLUSH(dlog);

	}while ( strcmp(dinfo.run_type, "REG") == 0 );

	clean_up();
	exit(0);
}


void sigterm_hdl()
{
	exit_flag = TRUE;
}

void sigusr1_hdl()
{
	no_write = FALSE;
}

#ifdef __HTX_LINUX__
int check_rng_available()
{
	if ( (dev_fd = open("/dev/hwrng", O_RDONLY) ) == -1 ) {
		return(-1);
	}

	return(0);
}
#else
int check_rng_available()
{
	return(0);
}
#endif /* __HTX_LINUX__ */

#if 0
int allocate_mem()
{
	int rc, i;
	key_t key;
	unsigned int shm_flags;
	struct shmid_ds sbuf = { 0 };

#ifndef __HTX_LINUX__
	struct vminfo_psize vminfo = { 0 };
	sbuf.shm_pagesize = 4096;
#endif

	shm_flags = ( IPC_CREAT | IPC_EXCL | S_IRWXU| S_IRWXG | S_IRWXO );

	key = (BIT_SHM_KEY | ((0xfff & core_num) << 4) | (0xf & i));
	if ( (rnd_bit_shm_id = shmget(key, (STREAM_SIZE * 16 / 8), shm_flags)) == -1 ) {
			sprintf(msg, "shmget failed for ID:%lx, error\(%d\):%s\n", key, errno, strerror(errno));
			hxfmsg(&hd, rc, HTX_HE_HARD_ERROR, msg);
			return(rc);
		}

#ifndef __HTX_LINUX__
		if ( (rc = shmctl(rnd_bit_shm_id[i], SHM_PAGESIZE, &sbuf)) == -1 ) {
			sprintf(msg, "shmctl failed for ID:%lx, error\(%d\):%s\n", key, errno, strerror(errno));
			hxfmsg(&hd, rc, HTX_HE_HARD_ERROR, msg);
			return(rc);
		}
#endif

		if ( (rnd_bit_shm_ptr[i] = shmat(rnd_bit_shm_id[i], (char *)0, 0)) == NULL ) {
			sprintf(msg, "shmat failed for ID:%lx, error\(%d\):%s\n", key, errno, strerror(errno));
			hxfmsg(&hd, -1, HTX_HE_HARD_ERROR, msg);
			return(-1);
		}

		if ( (mlock(rnd_bit_shm_ptr[i], STREAM_SIZE)) == -1 ) {
			sprintf(msg, "mlock failed for ID:%lx, error\(%d\):%s\n" key, errno, strerror(errno));
			hxfmsg(&hd, -1, HTX_HE_HARD_ERROR, msg);
			return(-1);
		}
	}

	for ( i=0; i<3; i++ ) {
		key = (BYTE_SHM_KEY | ((0xfff & core_num) << 4) | (0xf & i));
		if ( (rnd_byte_shm_id[i] = shmget(key, (STREAM_SIZE), shm_flags)) == -1 ) {
			sprintf(msg, "shmget failed for ID:%lx, error\(%d\):%s\n", key, errno, strerror(errno));
			hxfmsg(&hd, rc, HTX_HE_HARD_ERROR, msg);
			return(rc);
		}

#ifndef __HTX_LINUX__
		if ( (rc = shmctl(rnd_byte_shm_id[i], SHM_PAGESIZE, &sbuf)) == -1 ) {
			sprintf(msg, "shmctl failed for ID:%lx, error\(%d\):%s\n", key, errno, strerror(errno));
			hxfmsg(&hd, rc, HTX_HE_HARD_ERROR, msg);
			return(rc);
		}
#endif

		if ( (rnd_byte_shm_ptr[i] = shmat(rnd_byte_shm_id[i], (char *)0, 0)) == NULL ) {
			sprintf(msg, "shmat failed for ID:%lx, error\(%d\):%s\n", key, errno, strerror(errno));
			hxfmsg(&hd, -1, HTX_HE_HARD_ERROR, msg);
			return(-1);
		}

		if ( (mlock(rnd_bit_shm_ptr[i], STREAM_SIZE)) == -1 ) {
			sprintf(msg, "mlock failed for ID:%lx, error\(%d\):%s\n" key, errno, strerror(errno));
			hxfmsg(&hd, -1, HTX_HE_HARD_ERROR, msg);
			return(-1);
		}
	}

	return(0);
}
#endif

int allocate_mem()
{
	int i;
	/* We need 64(MAX_SLOTS)  times 1 Mb(million bits)
	 * Total = (64 * 1024 * 1024)/8.
	 * Total = (8 * 1024 * 1024) bytes.
	 * Total = (8 * STREAM_SIZE) bytes = 8MB.
	 */
	rnd_bit_ptr = malloc((MAX_SLOTS*STREAM_SIZE)/8);

	DPRINT(dlog, "rnd_bit_ptr allocated. %llx\n", rnd_bit_ptr);
	FFLUSH(dlog);

	if ( rnd_bit_ptr == NULL ) {
		sprintf(msg, "malloc of bit stream failed with error(%d):%s\n", errno, strerror(errno));
		hxfmsg(&hd, -1, HTX_HE_HARD_ERROR, msg);
		return(-1);
	}

	/* We also need 3 separate 1MB (megabytes) memory */
	for ( i=0; i<3; i++ ) {
		rnd_byte_ptr[i] = malloc(STREAM_SIZE);

		DPRINT(dlog, "rnd_byte_ptr[%d] allocated. %llx\n", i, rnd_byte_ptr[i]);
		FFLUSH(dlog);

		if ( rnd_byte_ptr[i] == NULL ) {
			sprintf(msg, "malloc of byte stream failed with error(%d):%s\n", errno, strerror(errno));
			hxfmsg(&hd, -1, HTX_HE_HARD_ERROR, msg);
			return(-1);
		}
	}

	return(0);
}

int read_rf()
{
	int i, j, rc, line_number = 0;
	struct rule_parameters *r;
	FILE *rule_file;

	for( i = 0; i < MAX_NUM_RULES; i++ ) {
		r = &rule_list[i];
		sprintf(r->rule_name, "test%d", i);
		r->stream_len = 1048576;
		r->blockFrequency_block_len = 128;
		r->nonOverlappingTemplate_block_len = 9;
		r->overlappingTemplate_block_len = 9;
		r->approxEntropy_block_len = 10;
		r->serial_block_len = 16;
		r->linearComplexity_sequence_len = 500;
		r->num_oper = 50;

		for ( j=0; j<NUM_TESTS; j++ ) {
			r->tests_to_run[j] = 1;
		}
	}

#ifdef AWAN
	num_active_rules = 1;
#else
	rule_file = fopen(dinfo.rule_file_name, "r");
	if ( rule_file == NULL ) {
		sprintf(msg, "Error opening rule file : %s\n", dinfo.rule_file_name);
		hxfmsg(&hd, -1, HTX_HE_HARD_ERROR, msg);
		return(-1);
	}

	num_active_rules = 0;

	r = &rule_list[num_active_rules];

	while ( num_active_rules < MAX_NUM_RULES ) {
		rc = get_rule(&line_number, rule_file, r);
		DPRINT(dlog, "get_rule() return value = %d\n", rc);
		FFLUSH(dlog);

		if ( rc == -1 ) {
			fclose(rule_file);
			return(rc);
		}

		else if ( rc == -2 ) { /* EOF received */
			break;
		}

		else if ( rc > 0 ) {  /* End of stanza so move stanza ptr to next */
			num_active_rules++;
			r++;
		}

		/* just a new line in rule file so continue */
	}

	if ( num_active_rules >= MAX_NUM_RULES ) {
		sprintf(msg, "Number of stanzas used:(%d) is more than supported:(%d)\n", num_active_rules, MAX_NUM_RULES);
		hxfmsg(&hd, 0, HTX_HE_INFO, msg);
	}
	fclose(rule_file);
#endif

	return(0);
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

int get_line( char s[], int lim, FILE *fp, char delimiter, int *line)
{
	int c = 0, i = 0, rc;

	while (1) {
		c = fgetc(fp);
		if (c != EOF && c != '\n' && c != delimiter && i < lim) {
			if (c == '=') {
				s[i++] = ' ';
			}
			else {
				s[i++] = c;
			}
		}
		else {
			if (c == delimiter || c == EOF) break;
			else if (c == '\n' && i < lim) {
				*line = *line + 1;
				continue;
			}
			else if (i >= lim) {
				sprintf(msg, "line#: %d. Too big line to fit in !!", *line);
				hxfmsg(&hd, -1, HTX_HE_HARD_ERROR, msg);
				return(-3);
			}
		}
	}
	if(c == EOF && feof(fp)) {
		if(c == EOF && i == 0) return(-2);
		s[i] = '\0';
	}
	else if(c == EOF && ferror(fp)) {
		return(-1);
	}
	else if(c == delimiter) {
		s[i++] = c;
		s[i] = '\0';
	}
	else if (i >= lim) {
		s[i-1] = '\0';
	}
	rc = parse_line(s);
	return(rc);
}

int get_rule(int *line, FILE *fp, struct rule_parameters *r)
{
	char s[MAX_RULE_LINE_SIZE], keywd[80];
	int rc, keywd_count=0;

	/* Loop will break if EOF is received. */
	while (1) {
		rc = get_line(s, MAX_RULE_LINE_SIZE, fp, '\n', line);
		/*
		 * rc =  0 indicates comment in the rulefile.
		 * rc =  1 indicates only '\n' (newline) in the line i.e. change in stanza.
		 * rc >  1 more character. Check for valid parameter name.
		 * rc = -1 error reading the rulefile.
		 * rc = -2 end of file.
		 */

		if ( rc == 0 ) {
			*line = *line + 1;
			continue;
		}

		else if ( rc == 1 ) return(keywd_count);
		else if ( rc == -1 || rc == -3 ) return(-1);
		else if ( rc == -2 && keywd_count > 0 ) return(keywd_count);
		else if ( rc == -2 && keywd_count == 0 ) return(-2);

		*line = *line +1;

		sscanf(s,"%s",keywd);

		if ( strcasecmp(keywd,"rule_name") == 0) {
			sscanf(s, "%*s %s", r->rule_name);
			if ( strlen(r->rule_name) > 16 ) {
				sprintf(msg,"line#%d : %s = %s must be 16 characters or less\n", *line, keywd, r->rule_name);
				hxfmsg(&hd, -1, HTX_HE_HARD_ERROR, msg);
				return(-1);
			}
			DPRINT(dlog, "Param name = %s, Val = %s \n", keywd, r->rule_name);
			FFLUSH(dlog);
		}

		else if ( strcasecmp(keywd, "stream_size") == 0 ) {
			sscanf(s,"%*s %d", &r->stream_len);
			if ( r->stream_len < 0 || r->stream_len > 1048576 ) {
				sprintf(msg, "line#%d : %s = %d must be in the range of 0 < stream_size < 1048576\n",
						*line, keywd, r->stream_len);
				hxfmsg(&hd, -1, HTX_HE_HARD_ERROR, msg);
				return(-1);
			}
			DPRINT(dlog, "Param name = %s, Val = %d \n", keywd, r->stream_len);
			FFLUSH(dlog);
		}

		else if ( strcasecmp(keywd, "num_oper") == 0 ) {
			sscanf(s,"%*s %d", &r->num_oper);
			if ( r->num_oper < 0 ) {
				sprintf(msg, "line# %d %s = %d must be positive integer or 0", *line, keywd, r->num_oper);
				hxfmsg(&hd, -1, HTX_HE_HARD_ERROR, msg);
				return(-1);
			}
			else if(r->num_oper == 0) {
				sprintf(msg, "Warning ! num_oper = 0. stanza will run forever.");
				hxfmsg(&hd, 0, HTX_HE_INFO, msg);
			}
			DPRINT(dlog, "Param name = %s Val = %d\n", keywd, r->num_oper);
			FFLUSH(dlog);
		}

		else {
			sprintf(msg, "line# %d Unsupported keyword: %s", *line, keywd);
			hxfmsg(&hd, -1, HTX_HE_HARD_ERROR, msg);
			return(-1);
		}

		keywd_count++;
	}
	return(0);
}

int get_PVR()
{
   /*
    * In this function we will call AIX getPvr() function loaded in kernel extension.
	* This function will give us the deatil of actual hardware that the code is running on.
    */

#ifdef __HTX_LINUX__
	__asm __volatile ("mfspr %0, 287" : "=r" (pvr));
	shifted_pvr_hw = pvr >> 16;
	shifted_pvr_os = shifted_pvr_hw;
#else
	pvr = (unsigned int) getPvr();
	shifted_pvr_hw = pvr >> 16;
	shifted_pvr_os = shifted_pvr_hw;

	if ( shifted_pvr_hw == 0x3f && (_system_configuration.implementation == POWER_6) ) {
		shifted_pvr_os = 0x3e;
	}
#endif
	/* if ( shifted_pvr_hw >= 0x3f ) { */
	if ( shifted_pvr_hw >= 0x3e ) {
		return(0);
	}
	else {
		return(-1);
	}
}



int read_rn_func(void *tno) /* th0 function. Reads random number from hardware */
{
	unsigned long long rnd_no, *buf_ptr;
	int slot_cnt, i, cnt=0;
	int client_no = *((int *)tno);

	DPRINT(dlog, "%s:thread#%d\n", __FUNCTION__, client_no);
	FFLUSH(dlog);

	/* start reading random numbers till
	 * (1) all the buffer slots are populated - Wait for th1 to complete then
	 * (2) rule.num_open is over.
	 * (3) SIGTERM is received.
	 */
	DPRINT(dlog, "\n num_oper = %d, exit flag = %d false = %d\n", rule.num_oper, exit_flag, FALSE);
	FFLUSH(dlog);

	/* Initialize buffer pointer */
	buf_ptr = (unsigned long long *)rnd_bit_ptr;
	slot_cnt = 0;

	while ( slot_cnt < MAX_SLOTS && exit_flag == FALSE ) {
		for ( i=0; i<((STREAM_SIZE/8)/8); i++ ) {
			rnd_no = read_rn();
#if 0
			DPRINT(dlog, "rno = %llx, slot=%d, count=%d\n", rnd_no, slot_cnt, i);
			FFLUSH(dlog);
#endif
			*buf_ptr = rnd_no;
			buf_ptr++;
		}
		/* Increment the status */
		pthread_mutex_lock(&status_mutex1);
		status[slot_cnt] = 1;
		pthread_mutex_unlock(&status_mutex1);

		/* Slot populated. Signal th1 to start converting now. */
		pthread_cond_signal(&update_cond1);

		slot_cnt++;
	}
	cnt++;

	if ( slot_cnt == MAX_SLOTS && exit_flag == FALSE ) {
		while ( no_write == TRUE && exit_flag == FALSE ) {
			read_rn();
		}
	}

	return(0);
}

#ifdef __HTX_LINUX__
unsigned long long read_rn()
{
	unsigned long long temp;

	read(dev_fd, (void *)&temp, sizeof(unsigned long long));

	return(temp);
}
#else

#if 0
void set_rng_gen()
{
	/* #1
 	 * All the #1 parts are just for testing.
 	 * Those are to be removed once library interface for reading rng in AIX comes up.
 	 * Till then using libc call to generate rng and test those.
 	 */
	long int time_val;

	time_val = time(NULL);

	srand48_r(time_val, &rnd_buf);
}

unsigned long long read_rn()
{
	/* #1
 	 * All the #1 parts are just for testing.
 	 * Those are to be removed once library interface for reading rng in AIX comes up.
 	 * Till then using libc call to generate rng and test those.
 	 */

	unsigned long long temp_rng;
	unsigned long temp;

	mrand48_r(&rnd_buf, (long *)&temp);
	temp_rng = (temp << 32);
	mrand48_r(&rnd_buf, (long *)&temp);
	temp_rng |= temp;

	return(temp_rng);
}
#else /* if 0 */
unsigned long long read_rn()
{
	int rc, cnt;
	struct hcall_in_params in;
	struct h_cop_op_out out;

	in.r3 = H_RANDOM;

#if 0
	/* This function is called many number of times.
	 * These many checks will bring down performance considerably.
	 * So, writing another one without all these checks which will
	 * consider that everything should run fine.
	 */
	do {
		rc = h_cop_random_k(&in, &out);

		if ( rc == 0 ) {
			break;
		}
		else {
			cnt++;
		}
	} while ( (rc !=0) && (cnt<100) );


	if ( cnt == 100 ) {
		return(-1);
	}
	else {
		if ( out.rc == H_Success ) {
			return(out.rand_no);
		}
		else {
			return(-1);
		}
	}
#endif

	h_cop_random_k((char *)&in, (char *)&out);
	if ( out.rc == H_Hardware ) {
		char info_msg[1024];
		sprintf(info_msg, "Error while reading RNG. hcall() Returned H_Hardware. Exiting.\n");
		hxfmsg(&hd, 1, HTX_HE_HARD_ERROR, info_msg);
		clean_up();
		exit(1);
	}
	else {
		return(out.rand_no);
	}
}

#endif /* if 0 */
#endif /* __HTX_LINUX__ */


#ifdef __HTX_OPEN_SOURCE__
int convert_bit_2_byte(void *tno)
{
	char *bit_ptr, *byte_ptr;
	char bit_rnd_stream;
	int i, j;

	DPRINT(dlog, "%s:thread#%d\n", __FUNCTION__, *((int *)tno));
	FFLUSH(dlog);

	/* Wait till that location is populated by th0 */
	pthread_mutex_lock(&status_mutex1);
	if ( status[conv_buf_no] == 0 ) {
		DPRINT(dlog,"%s:waiting for read_rn to populate buffer.conv_buf_no=%d[%d],test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
		FFLUSH(dlog);
		PTHREAD_WAIT(&update_cond1, &status_mutex1);
	}

	pthread_mutex_lock(&status_mutex2);
	if ( (test_buf_no%3) == (conv_buf_no%3)) {
		/* Wait till this location is in use by any tester thread th2-th17 */
		if ( status[test_buf_no] > 1 && status[test_buf_no] < 17 ) {
			DPRINT(dlog, "%s:waiting for tests to complete.conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}

	bit_ptr = (char *)rnd_bit_ptr;
	bit_ptr += (conv_buf_no * (STREAM_SIZE/8));

	byte_ptr = (char *)rnd_byte_ptr[conv_buf_no%3];

	for ( i=0; i<(STREAM_SIZE/8); i++) {
		bit_rnd_stream = *(bit_ptr+i);
		for ( j=0; j<8; j++ ) {
			if ( bit_rnd_stream & 0x80 ) {
				*(byte_ptr + j) = 0x1;
				bit_rnd_stream <<= 1;
			}
			else {
				*(byte_ptr + j) = 0x0;
				bit_rnd_stream <<= 1;
			}
		}
	}

	/* Send signal to all tester threads (th2-th17) to start
	 * working on currently populated buffer.
	 * All these threads are waiting on update_cond2 condition variable.
	 */
	/* wait till all the threads are created */
	pthread_mutex_lock(&status_mutex3);

	if ( num_th[conv_buf_no] < 15 ) {
		DPRINT(dlog,"%s:waiting for all threads to get created. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
		FFLUSH(dlog);

		PTHREAD_WAIT(&update_cond3, &status_mutex3);
	}
	pthread_mutex_unlock(&status_mutex3);

	status[conv_buf_no] = 2;
	conv_buf_no++;


	/* Send SIGUSR1 to th1 to stop (which will internally start th0)
	 * when all the slots have been converted.
	 */
	/* if ( conv_buf_no == MAX_SLOTS ) { */
		kill(getpid(), SIGUSR1);
	/* } */

	/* Unlock both locked mutexes */
	pthread_mutex_unlock(&status_mutex1);
	pthread_mutex_unlock(&status_mutex2);

	DPRINT(dlog,"%s:sending broadcast to all worker threads. update_cond2", __FUNCTION__);
	FFLUSH(dlog);
	pthread_cond_broadcast(&update_cond2);

	/* write the looping code later */
	return(0);
}


int freq(void *tno)
{
	double rc;
	char *stream_ptr, file_name[50];
	FILE *dump_file;

	DPRINT(dlog, "%s:thread#%d\n", __FUNCTION__, *((int *)tno));
	FFLUSH(dlog);

	/* All threads except last thread
	 * Checking for NUM_TESTS-1 (rather than NUM_TESTS) because its getting increments now.
	 */
	if ( num_th[conv_buf_no] < NUM_TESTS-1 ) {
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}
	else { /* Last worker thread */
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;

		DPRINT(dlog, "%s: sending broadcast to update_cond3. All threads created now\n", __FUNCTION__);
		FFLUSH(dlog);
		pthread_cond_broadcast(&update_cond3);
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}

	/* Release the mutex lock so that other tester threads can get in and start operation */
	pthread_mutex_unlock(&status_mutex2);

	/* Point to the currect test buffer. */
	stream_ptr = rnd_byte_ptr[test_buf_no%3];

	rc = Frequency(stream_ptr, STREAM_SIZE);

	if ( rc ) {
		/* Log error */
#ifndef AWAN
		sprintf(file_name,"/tmp/hxerng_stream_dump.%d.%d", getpid(), FREQUENCY);
		dump_file = fopen(file_name, "w");
		fwrite(stream_ptr, sizeof(char), STREAM_SIZE, dump_file);
		fclose(dump_file);
#endif
		printf("\n Stream below threashold for freq(%lf). Stream dumped in %s\n", rc, file_name);
	}

	pthread_mutex_lock(&status_mutex2);
	status[test_buf_no] += 1;

	if ( status[test_buf_no] >= 17 ) {
		/* This buffer has been tested completely. Send signal to th1 to start populating it. */
		test_buf_no++;
		pthread_cond_signal(&update_cond2);
	}

	pthread_mutex_unlock(&status_mutex2);

	return(0);
}

int block_freq(void *tno)
{
	double rc;
	char *stream_ptr, file_name[50];
	FILE *dump_file;

	DPRINT(dlog, "%s:thread#%d\n", __FUNCTION__, *((int *)tno));
	FFLUSH(dlog);

	/* All threads except last thread
	 * Checking for NUM_TESTS-1 (rather than NUM_TESTS) because its getting increments now.
	 */
	if ( num_th[conv_buf_no] < NUM_TESTS-1 ) {
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}
	else { /* Last worker thread */
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;

		DPRINT(dlog, "%s: sending broadcast to update_cond3. All threads created now\n", __FUNCTION__);
		FFLUSH(dlog);
		pthread_cond_broadcast(&update_cond3);
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}

	/* Release the mutex lock so that other tester threads can get in and start operation */
	pthread_mutex_unlock(&status_mutex2);

	/* Point to the currect test buffer. */
	stream_ptr = rnd_byte_ptr[test_buf_no%3];

	rc = BlockFrequency(stream_ptr, STREAM_SIZE, rule.blockFrequency_block_len);

	if ( rc ) {
		/* Log error */
#ifndef AWAN
		sprintf(file_name,"/tmp/hxerng_stream_dump.%d.%d", getpid(), BLOCK_FREQUENCY);
		dump_file = fopen(file_name, "w");
		fwrite(stream_ptr, sizeof(char), STREAM_SIZE, dump_file);
		fclose(dump_file);
#endif
		printf("\n Stream below threashold for block_freq(%lf). Stream dumped in %s\n", rc, file_name);
	}

	pthread_mutex_lock(&status_mutex2);
	status[test_buf_no] += 1;

	if ( status[test_buf_no] >= 17 ) {
		/* This buffer has been tested completely. Send signal to th1 to start populating it. */
		test_buf_no++;
		pthread_cond_signal(&update_cond2);
	}

	pthread_mutex_unlock(&status_mutex2);

	return(0);
}

int runs(void *tno)
{
	double rc;
	char *stream_ptr, file_name[50];
	FILE *dump_file;

	DPRINT(dlog, "%s:thread#%d\n", __FUNCTION__, *((int *)tno));
	FFLUSH(dlog);

	/* All threads except last thread
	 * Checking for NUM_TESTS-1 (rather than NUM_TESTS) because its getting increments now.
	 */
	if ( num_th[conv_buf_no] < NUM_TESTS-1 ) {
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}
	else { /* Last worker thread */
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;

		DPRINT(dlog, "%s: sending broadcast to update_cond3. All threads created now\n", __FUNCTION__);
		FFLUSH(dlog);
		pthread_cond_broadcast(&update_cond3);
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}

	/* Release the mutex lock so that other tester threads can get in and start operation */
	pthread_mutex_unlock(&status_mutex2);

	/* Point to the currect test buffer. */
	stream_ptr = rnd_byte_ptr[test_buf_no%3];

	rc = Runs(stream_ptr, STREAM_SIZE);

	if ( rc ) {
		/* Log error */
#ifndef AWAN
		sprintf(file_name,"/tmp/hxerng_stream_dump.%d.%d", getpid(), RUNS);
		dump_file = fopen(file_name, "w");
		fwrite(stream_ptr, sizeof(char), STREAM_SIZE, dump_file);
		fclose(dump_file);
#endif
		printf("\n Stream below threashold for runs(%lf). Stream dumped in %s\n", rc, file_name);
	}

	pthread_mutex_lock(&status_mutex2);
	status[test_buf_no] += 1;

	if ( status[test_buf_no] >= 17 ) {
		/* This buffer has been tested completely. Send signal to th1 to start populating it. */
		test_buf_no++;
		pthread_cond_signal(&update_cond2);
	}

	pthread_mutex_unlock(&status_mutex2);

	return(0);
}

int longest_run_of_ones(void *tno)
{
	double rc;
	char *stream_ptr, file_name[50];
	FILE *dump_file;

	DPRINT(dlog, "%s:thread#%d\n", __FUNCTION__, *((int *)tno));
	FFLUSH(dlog);

	/* All threads except last thread
	 * Checking for NUM_TESTS-1 (rather than NUM_TESTS) because its getting increments now.
	 */
	if ( num_th[conv_buf_no] < NUM_TESTS-1 ) {
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}
	else { /* Last worker thread */
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;

		DPRINT(dlog, "%s: sending broadcast to update_cond3. All threads created now\n", __FUNCTION__);
		FFLUSH(dlog);
		pthread_cond_broadcast(&update_cond3);
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}

	/* Release the mutex lock so that other tester threads can get in and start operation */
	pthread_mutex_unlock(&status_mutex2);

	/* Point to the currect test buffer. */
	stream_ptr = rnd_byte_ptr[test_buf_no%3];

	rc = LongestRunOfOnes(stream_ptr, STREAM_SIZE);

	if ( rc ) {
		/* Log error */
#ifndef AWAN
		sprintf(file_name,"/tmp/hxerng_stream_dump.%d.%d", getpid(), LONGEST_RUN_OF_ONES);
		dump_file = fopen(file_name, "w");
		fwrite(stream_ptr, sizeof(char), STREAM_SIZE, dump_file);
		fclose(dump_file);
#endif
		printf("\n Stream below threashold for longest_run_of_ones(%lf). Stream dumped in %s\n", rc, file_name);
	}

	pthread_mutex_lock(&status_mutex2);
	status[test_buf_no] += 1;

	if ( status[test_buf_no] >= 17 ) {
		/* This buffer has been tested completely. Send signal to th1 to start populating it. */
		test_buf_no++;
		pthread_cond_signal(&update_cond2);
	}

	pthread_mutex_unlock(&status_mutex2);

	return(0);
}

int rank(void *tno)
{
	double rc;
	char *stream_ptr, file_name[50];
	FILE *dump_file;

	DPRINT(dlog, "%s:thread#%d\n", __FUNCTION__, *((int *)tno));
	FFLUSH(dlog);

	/* All threads except last thread
	 * Checking for NUM_TESTS-1 (rather than NUM_TESTS) because its getting increments now.
	 */
	if ( num_th[conv_buf_no] < NUM_TESTS-1 ) {
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}
	else { /* Last worker thread */
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;

		DPRINT(dlog, "%s: sending broadcast to update_cond3. All threads created now\n", __FUNCTION__);
		FFLUSH(dlog);
		pthread_cond_broadcast(&update_cond3);
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}

	/* Release the mutex lock so that other tester threads can get in and start operation */
	pthread_mutex_unlock(&status_mutex2);

	/* Point to the currect test buffer. */
	stream_ptr = rnd_byte_ptr[test_buf_no%3];

	rc = Rank(stream_ptr, STREAM_SIZE);

	if ( rc ) {
		/* Log error */
#ifndef AWAN
		sprintf(file_name,"/tmp/hxerng_stream_dump.%d.%d", getpid(), MATRIX_RANK);
		dump_file = fopen(file_name, "w");
		fwrite(stream_ptr, sizeof(char), STREAM_SIZE, dump_file);
		fclose(dump_file);
#endif
		printf("\n Stream below threashold for rank(%lf). Stream dumped in %s\n", rc, file_name);
	}

	pthread_mutex_lock(&status_mutex2);
	status[test_buf_no] += 1;

	if ( status[test_buf_no] >= 17 ) {
		/* This buffer has been tested completely. Send signal to th1 to start populating it. */
		test_buf_no++;
		pthread_cond_signal(&update_cond2);
	}

	pthread_mutex_unlock(&status_mutex2);

	return(0);
}

int dft(void *tno)
{
	double rc;
	char *stream_ptr, file_name[50];
	FILE *dump_file;

	DPRINT(dlog, "%s:thread#%d\n", __FUNCTION__, *((int *)tno));
	FFLUSH(dlog);

	/* All threads except last thread
	 * Checking for NUM_TESTS-1 (rather than NUM_TESTS) because its getting increments now.
	 */
	if ( num_th[conv_buf_no] < NUM_TESTS-1 ) {
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}
	else { /* Last worker thread */
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;

		DPRINT(dlog, "%s: sending broadcast to update_cond3. All threads created now\n", __FUNCTION__);
		FFLUSH(dlog);
		pthread_cond_broadcast(&update_cond3);
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}

	/* Release the mutex lock so that other tester threads can get in and start operation */
	pthread_mutex_unlock(&status_mutex2);

	/* Point to the currect test buffer. */
	stream_ptr = rnd_byte_ptr[test_buf_no%3];

	rc = DiscreteFourierTransform(stream_ptr, STREAM_SIZE);

	if ( rc ) {
		/* Log error */
#ifndef AWAN
		sprintf(file_name,"/tmp/hxerng_stream_dump.%d.%d", getpid(), DISCRETE_FOURIER_TRANSFORM);
		dump_file = fopen(file_name, "w");
		fwrite(stream_ptr, sizeof(char), STREAM_SIZE, dump_file);
		fclose(dump_file);
#endif
		printf("\n Stream below threashold for dft(%lf). Stream dumped in %s\n", rc, file_name);
	}

	pthread_mutex_lock(&status_mutex2);
	status[test_buf_no] += 1;

	if ( status[test_buf_no] >= 17 ) {
		/* This buffer has been tested completely. Send signal to th1 to start populating it. */
		test_buf_no++;
		pthread_cond_signal(&update_cond2);
	}

	pthread_mutex_unlock(&status_mutex2);

	return(0);
}

int non_overlapping_tm(void *tno)
{
	double rc;
	char *stream_ptr, file_name[50];
	FILE *dump_file;

	DPRINT(dlog, "%s:thread#%d\n", __FUNCTION__, *((int *)tno));
	FFLUSH(dlog);

	/* All threads except last thread
	 * Checking for NUM_TESTS-1 (rather than NUM_TESTS) because its getting increments now.
	 */
	if ( num_th[conv_buf_no] < NUM_TESTS-1 ) {
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}
	else { /* Last worker thread */
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;

		DPRINT(dlog, "%s: sending broadcast to update_cond3. All threads created now\n", __FUNCTION__);
		FFLUSH(dlog);
		pthread_cond_broadcast(&update_cond3);
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}

	/* Release the mutex lock so that other tester threads can get in and start operation */
	pthread_mutex_unlock(&status_mutex2);

	/* Point to the currect test buffer. */
	stream_ptr = rnd_byte_ptr[test_buf_no%3];

	rc = NonOverlappingTemplateMatchings(stream_ptr, STREAM_SIZE, rule.nonOverlappingTemplate_block_len);

	if ( rc ) {
		/* Log error */
#ifndef AWAN
		sprintf(file_name,"/tmp/hxerng_stream_dump.%d.%d", getpid(), NON_OVERLAPPING_TEMPLATE_MATCHING);
		dump_file = fopen(file_name, "w");
		fwrite(stream_ptr, sizeof(char), STREAM_SIZE, dump_file);
		fclose(dump_file);
#endif
		printf("\n Stream below threashold for non_overlapping_tm(%lf). Stream dumped in %s\n", rc, file_name);
	}

	pthread_mutex_lock(&status_mutex2);
	status[test_buf_no] += 1;

	if ( status[test_buf_no] >= 17 ) {
		/* This buffer has been tested completely. Send signal to th1 to start populating it. */
		test_buf_no++;
		pthread_cond_signal(&update_cond2);
	}

	pthread_mutex_unlock(&status_mutex2);

	return(0);
}

int overlapping_tm(void *tno)
{
	double rc;
	char *stream_ptr, file_name[50];
	FILE *dump_file;

	DPRINT(dlog, "%s:thread#%d\n", __FUNCTION__, *((int *)tno));
	FFLUSH(dlog);

	/* All threads except last thread
	 * Checking for NUM_TESTS-1 (rather than NUM_TESTS) because its getting increments now.
	 */
	if ( num_th[conv_buf_no] < NUM_TESTS-1 ) {
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}
	else { /* Last worker thread */
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;

		DPRINT(dlog, "%s: sending broadcast to update_cond3. All threads created now\n", __FUNCTION__);
		FFLUSH(dlog);
		pthread_cond_broadcast(&update_cond3);
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}

	/* Release the mutex lock so that other tester threads can get in and start operation */
	pthread_mutex_unlock(&status_mutex2);

	/* Point to the currect test buffer. */
	stream_ptr = rnd_byte_ptr[test_buf_no%3];

	rc = OverlappingTemplateMatchings(stream_ptr, STREAM_SIZE, rule.overlappingTemplate_block_len);

	if ( rc ) {
		/* Log error */
#ifndef AWAN
		sprintf(file_name,"/tmp/hxerng_stream_dump.%d.%d", getpid(), OVERLAPPING_TEMPLATE_MATCHING);
		dump_file = fopen(file_name, "w");
		fwrite(stream_ptr, sizeof(char), STREAM_SIZE, dump_file);
		fclose(dump_file);
#endif
		printf("\n Stream below threashold for overlapping_tm(%lf). Stream dumped in %s\n", rc, file_name);
	}

	pthread_mutex_lock(&status_mutex2);
	status[test_buf_no] += 1;

	if ( status[test_buf_no] >= 17 ) {
		/* This buffer has been tested completely. Send signal to th1 to start populating it. */
		test_buf_no++;
		pthread_cond_signal(&update_cond2);
	}

	pthread_mutex_unlock(&status_mutex2);

	return(0);
}

int universal(void *tno)
{
	double rc;
	char *stream_ptr, file_name[50];
	FILE *dump_file;

	DPRINT(dlog, "%s:thread#%d\n", __FUNCTION__, *((int *)tno));
	FFLUSH(dlog);

	/* All threads except last thread
	 * Checking for NUM_TESTS-1 (rather than NUM_TESTS) because its getting increments now.
	 */
	if ( num_th[conv_buf_no] < NUM_TESTS-1 ) {
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}
	else { /* Last worker thread */
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;

		DPRINT(dlog, "%s: sending broadcast to update_cond3. All threads created now\n", __FUNCTION__);
		FFLUSH(dlog);
		pthread_cond_broadcast(&update_cond3);
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}

	/* Release the mutex lock so that other tester threads can get in and start operation */
	pthread_mutex_unlock(&status_mutex2);

	/* Point to the currect test buffer. */
	stream_ptr = rnd_byte_ptr[test_buf_no%3];

	rc = Universal(stream_ptr, STREAM_SIZE);

	if ( rc ) {
		/* Log error */
#ifndef AWAN
		sprintf(file_name,"/tmp/hxerng_stream_dump.%d.%d", getpid(), UNIVERSAL);
		dump_file = fopen(file_name, "w");
		fwrite(stream_ptr, sizeof(char), STREAM_SIZE, dump_file);
		fclose(dump_file);
#endif
		printf("\n Stream below threashold for universal(%lf). Stream dumped in %s\n", rc, file_name);
	}

	pthread_mutex_lock(&status_mutex2);
	status[test_buf_no] += 1;

	if ( status[test_buf_no] >= 17 ) {
		/* This buffer has been tested completely. Send signal to th1 to start populating it. */
		test_buf_no++;
		pthread_cond_signal(&update_cond2);
	}

	pthread_mutex_unlock(&status_mutex2);

	return(0);
}

int approx_entropy(void *tno)
{
	double rc;
	char *stream_ptr, file_name[50];
	FILE *dump_file;

	DPRINT(dlog, "%s:thread#%d\n", __FUNCTION__, *((int *)tno));
	FFLUSH(dlog);

	/* All threads except last thread
	 * Checking for NUM_TESTS-1 (rather than NUM_TESTS) because its getting increments now.
	 */
	if ( num_th[conv_buf_no] < NUM_TESTS-1 ) {
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}
	else { /* Last worker thread */
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;

		DPRINT(dlog, "%s: sending broadcast to update_cond3. All threads created now\n", __FUNCTION__);
		FFLUSH(dlog);
		pthread_cond_broadcast(&update_cond3);
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}

	/* Release the mutex lock so that other tester threads can get in and start operation */
	pthread_mutex_unlock(&status_mutex2);

	/* Point to the currect test buffer. */
	stream_ptr = rnd_byte_ptr[test_buf_no%3];

	rc = ApproximateEntropy(stream_ptr, STREAM_SIZE, rule.approxEntropy_block_len);

	if ( rc ) {
		/* Log error */
#ifndef AWAN
		sprintf(file_name,"/tmp/hxerng_stream_dump.%d.%d", getpid(), APPROXIMATE_ENTROPY);
		dump_file = fopen(file_name, "w");
		fwrite(stream_ptr, sizeof(char), STREAM_SIZE, dump_file);
		fclose(dump_file);
#endif
		printf("\n Stream below threashold for approx_entropy(%lf). Stream dumped in %s\n", rc, file_name);
	}

	pthread_mutex_lock(&status_mutex2);
	status[test_buf_no] += 1;

	if ( status[test_buf_no] >= 17 ) {
		/* This buffer has been tested completely. Send signal to th1 to start populating it. */
		test_buf_no++;
		pthread_cond_signal(&update_cond2);
	}

	pthread_mutex_unlock(&status_mutex2);

	return(0);
}

int cusum(void *tno)
{
	double rc;
	char *stream_ptr, file_name[50];
	FILE *dump_file;

	DPRINT(dlog, "%s:thread#%d\n", __FUNCTION__, *((int *)tno));
	FFLUSH(dlog);

	/* All threads except last thread
	 * Checking for NUM_TESTS-1 (rather than NUM_TESTS) because its getting increments now.
	 */
	if ( num_th[conv_buf_no] < NUM_TESTS-1 ) {
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}
	else { /* Last worker thread */
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;

		DPRINT(dlog, "%s: sending broadcast to update_cond3. All threads created now\n", __FUNCTION__);
		FFLUSH(dlog);
		pthread_cond_broadcast(&update_cond3);
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}

	/* Release the mutex lock so that other tester threads can get in and start operation */
	pthread_mutex_unlock(&status_mutex2);

	/* Point to the currect test buffer. */
	stream_ptr = rnd_byte_ptr[test_buf_no%3];

	rc = CumulativeSums(stream_ptr, STREAM_SIZE);

	if ( rc ) {
		/* Log error */
#ifndef AWAN
		sprintf(file_name,"/tmp/hxerng_stream_dump.%d.%d", getpid(), CUMULATIVE_SUM);
		dump_file = fopen(file_name, "w");
		fwrite(stream_ptr, sizeof(char), STREAM_SIZE, dump_file);
		fclose(dump_file);
#endif
		printf("\n Stream below threashold for cusum(%lf). Stream dumped in %s\n", rc, file_name);
	}

	pthread_mutex_lock(&status_mutex2);
	status[test_buf_no] += 1;

	if ( status[test_buf_no] >= 17 ) {
		/* This buffer has been tested completely. Send signal to th1 to start populating it. */
		test_buf_no++;
		pthread_cond_signal(&update_cond2);
	}

	pthread_mutex_unlock(&status_mutex2);

	return(0);
}

int random_excursion(void *tno)
{
	double rc;
	char *stream_ptr, file_name[50];
	FILE *dump_file;

	DPRINT(dlog, "%s:thread#%d\n", __FUNCTION__, *((int *)tno));
	FFLUSH(dlog);

	/* All threads except last thread
	 * Checking for NUM_TESTS-1 (rather than NUM_TESTS) because its getting increments now.
	 */
	if ( num_th[conv_buf_no] < NUM_TESTS-1 ) {
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}
	else { /* Last worker thread */
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;

		DPRINT(dlog, "%s: sending broadcast to update_cond3. All threads created now\n", __FUNCTION__);
		FFLUSH(dlog);
		pthread_cond_broadcast(&update_cond3);
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}

	/* Release the mutex lock so that other tester threads can get in and start operation */
	pthread_mutex_unlock(&status_mutex2);

	/* Point to the currect test buffer. */
	stream_ptr = rnd_byte_ptr[test_buf_no%3];

	rc = RandomExcursions(stream_ptr, STREAM_SIZE);

	if ( rc ) {
		/* Log error */
#ifndef AWAN
		sprintf(file_name,"/tmp/hxerng_stream_dump.%d.%d", getpid(), RANDOM_EXCURSIONS);
		dump_file = fopen(file_name, "w");
		fwrite(stream_ptr, sizeof(char), STREAM_SIZE, dump_file);
		fclose(dump_file);
#endif
		printf("\n Stream below threashold for random_excursion(%lf). Stream dumped in %s\n", rc, file_name);
	}

	pthread_mutex_lock(&status_mutex2);
	status[test_buf_no] += 1;

	if ( status[test_buf_no] >= 17 ) {
		/* This buffer has been tested completely. Send signal to th1 to start populating it. */
		test_buf_no++;
		pthread_cond_signal(&update_cond2);
	}

	pthread_mutex_unlock(&status_mutex2);

	return(0);
}

int random_excursion_var(void *tno)
{
	double rc;
	char *stream_ptr, file_name[50];
	FILE *dump_file;

	DPRINT(dlog, "%s:thread#%d\n", __FUNCTION__, *((int *)tno));
	FFLUSH(dlog);

	/* All threads except last thread
	 * Checking for NUM_TESTS-1 (rather than NUM_TESTS) because its getting increments now.
	 */
	if ( num_th[conv_buf_no] < NUM_TESTS-1 ) {
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}
	else { /* Last worker thread */
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;

		DPRINT(dlog, "%s: sending broadcast to update_cond3. All threads created now\n", __FUNCTION__);
		FFLUSH(dlog);
		pthread_cond_broadcast(&update_cond3);
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}

	/* Release the mutex lock so that other tester threads can get in and start operation */
	pthread_mutex_unlock(&status_mutex2);

	/* Point to the currect test buffer. */
	stream_ptr = rnd_byte_ptr[test_buf_no%3];

	rc = RandomExcursionsVariant(stream_ptr, STREAM_SIZE);

	if ( rc ) {
		/* Log error */
#ifndef AWAN
		sprintf(file_name,"/tmp/hxerng_stream_dump.%d.%d", getpid(), RANDOM_EXCURSIONS_VARIANT);
		dump_file = fopen(file_name, "w");
		fwrite(stream_ptr, sizeof(char), STREAM_SIZE, dump_file);
		fclose(dump_file);
#endif
		printf("\n Stream below threashold for random_excursion_var(%lf). Stream dumped in %s\n", rc, file_name);
	}

	pthread_mutex_lock(&status_mutex2);
	status[test_buf_no] += 1;

	if ( status[test_buf_no] >= 17 ) {
		/* This buffer has been tested completely. Send signal to th1 to start populating it. */
		test_buf_no++;
		pthread_cond_signal(&update_cond2);
	}

	pthread_mutex_unlock(&status_mutex2);

	return(0);
}

int serial(void *tno)
{
	double rc;
	char *stream_ptr, file_name[50];
	FILE *dump_file;

	DPRINT(dlog, "%s:thread#%d\n", __FUNCTION__, *((int *)tno));
	FFLUSH(dlog);

	/* All threads except last thread
	 * Checking for NUM_TESTS-1 (rather than NUM_TESTS) because its getting increments now.
	 */
	if ( num_th[conv_buf_no] < NUM_TESTS-1 ) {
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}
	else { /* Last worker thread */
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;

		DPRINT(dlog, "%s: sending broadcast to update_cond3. All threads created now\n", __FUNCTION__);
		FFLUSH(dlog);
		pthread_cond_broadcast(&update_cond3);
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}

	/* Release the mutex lock so that other tester threads can get in and start operation */
	pthread_mutex_unlock(&status_mutex2);

	/* Point to the currect test buffer. */
	stream_ptr = rnd_byte_ptr[test_buf_no%3];

	rc = Serial(stream_ptr, STREAM_SIZE, rule.serial_block_len);

	if ( rc ) {
		/* Log error */
#ifndef AWAN
		sprintf(file_name,"/tmp/hxerng_stream_dump.%d.%d", getpid(), SERIAL);
		dump_file = fopen(file_name, "w");
		fwrite(stream_ptr, sizeof(char), STREAM_SIZE, dump_file);
		fclose(dump_file);
#endif
		printf("\n Stream below threashold for serial(%lf). Stream dumped in %s\n", rc, file_name);
	}

	pthread_mutex_lock(&status_mutex2);
	status[test_buf_no] += 1;

	if ( status[test_buf_no] >= 17 ) {
		/* This buffer has been tested completely. Send signal to th1 to start populating it. */
		test_buf_no++;
		pthread_cond_signal(&update_cond2);
	}

	pthread_mutex_unlock(&status_mutex2);

	return(0);
}

int linear_complexity(void *tno)
{
	double rc;
	char *stream_ptr, file_name[50];
	FILE *dump_file;

	DPRINT(dlog, "%s:thread#%d\n", __FUNCTION__, *((int *)tno));
	FFLUSH(dlog);

	/* All threads except last thread
	 * Checking for NUM_TESTS-1 (rather than NUM_TESTS) because its getting increments now.
	 */
	if ( num_th[conv_buf_no] < NUM_TESTS-1 ) {
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}
	else { /* Last worker thread */
		pthread_mutex_lock(&status_mutex3);
		num_th[conv_buf_no] += 1;

		DPRINT(dlog, "%s: sending broadcast to update_cond3. All threads created now\n", __FUNCTION__);
		FFLUSH(dlog);
		pthread_cond_broadcast(&update_cond3);
		pthread_mutex_unlock(&status_mutex3);

		/* Wait till this slot is available in test buffers */
		pthread_mutex_lock(&status_mutex2);
		if ( status[test_buf_no] < 2 ) {
			DPRINT(dlog, "%s: waiting for update_cond2 signal. conv_buf_no=%d[%d], test_buf_no=%d[%d]\n", __FUNCTION__, conv_buf_no, status[conv_buf_no], test_buf_no, status[test_buf_no]);
			FFLUSH(dlog);
			PTHREAD_WAIT(&update_cond2, &status_mutex2);
		}
	}

	/* Release the mutex lock so that other tester threads can get in and start operation */
	pthread_mutex_unlock(&status_mutex2);

	/* Point to the currect test buffer. */
	stream_ptr = rnd_byte_ptr[test_buf_no%3];

	rc = LinearComplexity(stream_ptr, STREAM_SIZE, rule.linearComplexity_sequence_len);

	if ( rc ) {
		/* Log error */
#ifndef AWAN
		sprintf(file_name,"/tmp/hxerng_stream_dump.%d.%d", getpid(), LINEAR_COMPLEXITY);
		dump_file = fopen(file_name, "w");
		fwrite(stream_ptr, sizeof(char), STREAM_SIZE, dump_file);
		fclose(dump_file);
#endif
		printf("\n Stream below threashold for linear_complexity(%lf). Stream dumped in %s\n", rc, file_name);
	}

	pthread_mutex_lock(&status_mutex2);
	status[test_buf_no] += 1;

	if ( status[test_buf_no] >= 17 ) {
		/* This buffer has been tested completely. Send signal to th1 to start populating it. */
		test_buf_no++;
		pthread_cond_signal(&update_cond2);
	}

	pthread_mutex_unlock(&status_mutex2);

	return(0);
}
#endif /* __HTX_OPEN_SOURCE__ */

void clean_up()
{
	int i;

	if ( dev_fd ) {
		close(dev_fd);
	}

	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&status_mutex1);
	pthread_mutex_destroy(&status_mutex2);
	pthread_mutex_destroy(&status_mutex3);
	pthread_cond_destroy(&update_cond1);
	pthread_cond_destroy(&update_cond2);
	pthread_cond_destroy(&update_cond3);
}
