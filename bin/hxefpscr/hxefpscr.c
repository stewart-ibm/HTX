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

/* @(#)46       1.18  src/htx/usr/lpp/htx/bin/hxefpscr/hxefpscr.c, exer_fpscr, htxubuntu 8/6/14 04:36:46 */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <signal.h>
#include <hxihtx.h>
#include <sys/prctl.h>
#include <getrule.h>
#include <ucontext.h>
#include <linux/prctl.h>
#include <linux/shm.h>
#include <sys/stat.h>
#include <sched.h>

typedef unsigned int	uInt32;
extern uInt32 Gprs[64];
extern uInt32 Fprs[64];
extern void fp_handler(void);
extern char *stdata;

#define fpscr1 0x28
#define fpscr2 0x30
#define stream	0x338
#define Pass1_FPR 0x138
#define Pass2_FPR 0x238
#define initial_data 0x38
#define no_of_inst 0x6
#define GANG_SIZE 32

/*************************************************************************************
 *                                                                                   *
 * Following inline code allows us to checkstop the machine in case of a miscompare. *
 * This code will work only on the architectures where 'attn' is available.          *
 *                                 !!! USE CAREFULLY !!!                             *
 *************************************************************************************/
void __attn (unsigned int a, unsigned int b, unsigned int c, unsigned int d){
	__asm__ volatile (".long 0x00000200":);
}

char msgbuf[4096];
char fpscrbuf[4096];
char workstr[256];
char rules_file_name[100];
char crash_on_mis[4];
char turn_attn_on[5];
char *pstdata ;
int  crash_flag;
int  attn_flag;
int  shm_id;
int  cpu;
int  no_of_cpu;
int  prnum;
char rf_seed[15];
uInt32 *inst_ptr;
cpu_set_t online_cpus_mask;

struct htx_data stats;
struct sigaction sigvector;

void  SIGUSR2_hdl (int sig, int code, struct sigcontext *scp);
void  SIGTERM_hdl (int sig, int code, struct sigcontext *scp);

/****************************************************************************/
/***  Beginning of Executable Code	*****************************************/
/****************************************************************************/

int main (int argc, char *argv[])
{
	long     fpscr_count=0;
	int      retcod, cpu_limit;
	int      run_type_OTH = FALSE;
	char     fail_test[20],htxkdblevel[4];
	int      rc_gr, rc, i;
	char     rule_id[80], file_name[40], mega_buf[7000], fpscr_temp[512];
	unsigned int pvr;
	volatile int slow_count;
	FILE     *fp_cpu;
	unsigned int seed;

	static struct	rule_def_struct rule_def_table[] =
	{
		{"RULE_ID", RSTRING_TYPE, 80, ".-z"},
		{"CRASH_ON_MIS", RSTRING_TYPE, 3, ".-z"},
		{"TURN_ATTN_ON", RSTRING_TYPE, 4, ".-z"},
		{"SEED", RSTRING_TYPE, 10, ".-z"},
		{"",0,0,""}
	};

	(void) strcpy(stats.HE_name, argv[0]);	 /* pass exerciser parameters */
	(void) strcpy(stats.sdev_id, argv[1]);
	(void) strcpy(stats.run_type, argv[2]);
	(void) strcpy(rules_file_name, argv[3]);

	if (strcmp(stats.run_type,"OTH") == 0)
	run_type_OTH = TRUE;

	/* Register for hotplug signals from supervisor. */
	stats.hotplug_cpu = 1;

	(void) hxfupdate(START, &stats);
	(void) sprintf(workstr,"%s %s %s %s\n",stats.HE_name,stats.sdev_id,stats.run_type, rules_file_name);
	(void) hxfmsg(&stats, 0, 7, workstr);

#if 0
	/* Not needed. And anyway it not doing anything in current design. */
	s.sa_flags = SA_SIGINFO ;
	s.sa_sigaction = fp_handler;
	sigaction(SIGFPE,&s,NULL);
#endif

	/* Register SIGTERM handler */
	sigemptyset((&sigvector.sa_mask));
	sigvector.sa_flags = 0;
	sigvector.sa_handler = (void (*)(int)) SIGTERM_hdl;
	sigaction(SIGTERM, &sigvector, (struct sigaction *) NULL);

	/* Register SIGUSR2 handler for hotplug */
	sigvector.sa_handler = (void (*)(int, int, struct sigcontext *))SIGUSR2_hdl;
	sigaction(SIGUSR2, &sigvector, (struct sigaction *) NULL);


	/* Set the prctl so that exception are not enabled
	 * as in AIX , this should be done through a lib call
	 * currently it is done directly
	 */
#if defined(PR_FP_EXC_DISABLED) /* defined in kernel headers after ver 2.4.21 */
	prctl(PR_SET_FPEXC,PR_FP_EXC_DISABLED,0,0,0);
#endif
	if (strncmp(stats.sdev_id,"/dev/fpscr",10) != 0) {
		(void) sprintf(stats.msg_text,"%s not allowed \n",stats.sdev_id);
		hxfmsg(&stats,0,1,stats.msg_text);
		return(1);
	} /* endif */

	no_of_cpu = get_nprocs();

	prnum = atoi(&(stats.sdev_id[10]));

	if(NULL == getenv("HTXKDBLEVEL"))
		strcpy(htxkdblevel,"0");
	else
		strcpy(htxkdblevel,getenv("HTXKDBLEVEL"));

	rc_gr = open_rf(&stats,rule_def_table,rules_file_name);
	if(rc_gr!=0)
		hxfmsg(&stats,0,1,"Error while opening the Rule file\n");

	rc_gr = read_rf(&stats,rule_def_table,rule_id,crash_on_mis,turn_attn_on, rf_seed);
	if(rc_gr!=0)
		hxfmsg(&stats,0,1,"Error while reading the Rule file\n");

	close_rf();


	seed = (unsigned long)strtol(rf_seed, (char**) NULL, 16);
	if (seed == 0) {
		seed = 0xabcdef01;
	}

	crash_flag = FALSE; /* Initialize crash_flag = false */
	attn_flag  = FALSE; /* Initialize attn_flag = false */

	if(strcmp(crash_on_mis,"yes") && strcmp(crash_on_mis,"YES") && strcmp(crash_on_mis,"no") && strcmp(crash_on_mis,"NO"))
		hxfmsg(&stats,0,1,"Incorrect value of crash_on_mis in the rules file. Value should be YES or NO. crash_on_mis will be treated as NO\n");

	/* if crash_on_mis in the rules file is not yes and no, report the error*/
	if ( (strcmp(crash_on_mis,"yes") ==0) || (strcmp(crash_on_mis,"YES") ==0) || (strcmp(htxkdblevel,"0") !=0) )
		crash_flag = TRUE;

	/* if turn_attn_on in the rules file is not yes and no, report the error*/
	if(strcmp(turn_attn_on,"yes") && strcmp(turn_attn_on,"YES") && strcmp(turn_attn_on,"no") && strcmp(turn_attn_on,"NO"))
		hxfmsg(&stats,0,1,"Incorrect value of turn_attn_on in the rules file. Value should be YES or NO. turn_attn_on will be treated as NO\n");


	if ( (strcmp(turn_attn_on,"yes") ==0) || (strcmp(turn_attn_on,"YES") ==0) )
		attn_flag = TRUE;

	if ((attn_flag == TRUE) && (crash_flag == TRUE))
		crash_flag = FALSE; /* Priority to attention, if set */


	/* Get PVR Value */
	rc_gr = get_true_cpu_version();
	if ( rc_gr != -1) {
		pvr = (unsigned int ) rc_gr;
		pvr = pvr >> 16;
	}
	else {
		sprintf(fpscr_temp, "get_true_cpu_version failed  %d \n", rc_gr);
		(void) hxfmsg(&stats, 0, 1, fpscr_temp);
	}

	shm_id = shmget(IPC_PRIVATE, (no_of_inst * sizeof(int)), IPC_CREAT | S_IRWXU);
	if(shm_id == -1) {
		sprintf(workstr, "Error: Problem in getting shared memory for instruction stream. Errno: %d\n", errno);
		hxfmsg(&stats, 0, 1, workstr);
		exit(1);
	}

	inst_ptr = (uInt32 *) shmat(shm_id, (char *) 0, SHM_EXEC);
	if(inst_ptr == -1) {
		hxfmsg(&stats, 0, 1, "Error: Problem in attaching to shared memory for instruction stream\n");
		exit(1);
	}

	stats.test_id = 1;
	hxfupdate(UPDATE, &stats);

	do
	{
		cpu = prnum * GANG_SIZE;
		cpu_limit = cpu + GANG_SIZE;

		/* Check is there are any CPUs in this gang to run on. Otherwise exit with DT. */
		if ( cpu >= no_of_cpu ) {
			sprintf(workstr, "No CPUs available in this gang (%d-%d). Total available CPUs =%d\n",
				prnum*GANG_SIZE, cpu_limit, no_of_cpu);
			sprintf(workstr, "%sThis device should not get created. Expected when running hotplug with custome MDT. Exiting this instance.\n", workstr);
			hxfmsg(&stats, 0, HTX_HE_INFO, workstr);
			hxfupdate(RECONFIG, &stats);
			exit(0);
		}

		for (; cpu < cpu_limit && cpu < no_of_cpu; cpu++) {
			/* Only pass logical CPU and not physical CPU because hxefpscr is not very tightly bound to core/chip.
			 * it just runs in a gang of 32 (might cross chips)
			sprintf(workstr, "binding to logical cpu : %d. range = %d - %d. PID = %d\n", cpu, prnum * GANG_SIZE, ( (cpu_limit < no_of_cpu) ? cpu_limit : no_of_cpu) - 1, getpid());
			hxfmsg(&stats, 0, HTX_HE_INFO, workstr);
			 */
			rc = htx_bind_thread(cpu, -1);
			if ( rc == -1 || rc == -2 ) { /* Failure due to hotplug */
				no_of_cpu = get_nprocs();
				if ( no_of_cpu <= prnum * GANG_SIZE ) {
					sprintf(workstr, "Bindprocessor : No CPUs in this gang (%d-%d) are available.\nTotal available CPUs =%d\n",
						prnum*GANG_SIZE, cpu_limit, no_of_cpu);
					sprintf(workstr, "%sExiting this instance.\n", workstr);
					hxfmsg(&stats, 0, HTX_HE_INFO, workstr);
					hxfupdate(RECONFIG, &stats);
					exit(0);
				}
				sprintf(workstr, "htx_bind_thread : reducing number of online CPUs to %d\n", no_of_cpu);
				hxfmsg(&stats, 0, HTX_HE_INFO, workstr);
				continue;
			}
			else if (rc < 0) {
				(void) sprintf(workstr, "Error binding to processor %d. rc = %d.\n",cpu, rc);
				(void) hxfmsg(&stats,rc,1,workstr);
			}
			pstdata=&stdata;
			retcod = fpscr_bust(Gprs, inst_ptr, &seed, pvr);
			if(retcod == -1)
			{
				strcpy(fail_test,"FPSCR_BUST");
				if (crash_flag == TRUE)
					do_trap_htx64(0xBEEFDEAD,pstdata,fail_test,retcod);
				if (attn_flag == TRUE)
					__attn(0xBEEFDEAD,pstdata,fail_test,retcod);

				stats.bad_others = stats.bad_others + 1;
				sprintf(stats.msg_text,"\n");

				sprintf(fpscr_temp, "\nInstructions Executed: %08lx, %08lx, %08lx, %08lx, %08lx, %08lx\n",
							*(inst_ptr), *(inst_ptr+1), *(inst_ptr+2), *(inst_ptr+3), *(inst_ptr+4), *(inst_ptr+5));
				strcpy(fpscrbuf, fpscr_temp);

#ifdef __HTX_LE__
				sprintf(fpscr_temp,"Pass 1 FPSCR = 0x%08x%08x \n", *(unsigned int*) (pstdata+fpscr1+4), *(unsigned int*) (pstdata+fpscr1));
				strcat(fpscrbuf, fpscr_temp);

				sprintf(fpscr_temp,"Pass 2 FPSCR = 0x%08x%08x \n", *(unsigned int*) (pstdata+fpscr2+4), *(unsigned int*) (pstdata+fpscr2));
				strcat(fpscrbuf, fpscr_temp);

				sprintf(fpscr_temp,"Failed in loopcount = %u:%u	 \n",fpscr_count, *(unsigned long*) (pstdata));
				strcat(fpscrbuf, fpscr_temp);
				strcpy(mega_buf, fpscrbuf);
				strcat(mega_buf,"\n");
				for(i=0; i<64; i+=2){
					sprintf(fpscr_temp,"Input data    FPR[%0.2d] =	 0x%08x%08x \n", (i/2),
								*(unsigned int *) (pstdata+initial_data+4+(4*i)),
								*(unsigned int *) (pstdata+initial_data+(4*i)));
					strcat(mega_buf, fpscr_temp);

					sprintf(fpscr_temp,"Pass 1 Output FPR[%0.2d] =	 0x%08x%08x \n", (i/2),
								*(unsigned int *) (pstdata+Pass1_FPR+4+(4*i)),
								*(unsigned int *) (pstdata+Pass1_FPR+(4*i)));
					strcat(mega_buf, fpscr_temp);

					sprintf(fpscr_temp,"Pass 2 Output FPR[%0.2d] =	 0x%08x%08x \n", (i/2),
								*(unsigned int *) (pstdata+Pass2_FPR+4+(4*i)),
								*(unsigned int *) (pstdata+Pass2_FPR+(4*i)));
					strcat(mega_buf, fpscr_temp);
				}
#else
				sprintf(fpscr_temp,"Pass 1 FPSCR =	 0x%08x   %08x \n", *(unsigned int*) (pstdata+fpscr1), *(unsigned int*) (pstdata+fpscr1+4));
				strcat(fpscrbuf, fpscr_temp);

				sprintf(fpscr_temp,"Pass 2 FPSCR =	 0x%08x	  %08x \n", *(unsigned int*) (pstdata+fpscr2), *(unsigned int*) (pstdata+fpscr2+4));
				strcat(fpscrbuf, fpscr_temp);

				sprintf(fpscr_temp,"Failed in loopcount = %u:%u	 \n",fpscr_count, *(unsigned long*) (pstdata));
				strcat(fpscrbuf, fpscr_temp);
				strcpy(mega_buf, fpscrbuf);
				strcat(mega_buf,"\n");
				for(i=0; i<64; i+=2){
					sprintf(fpscr_temp,"Input data    FPR[%0.2d] =	 0x%08x%08x \n", (i/2),
								*(unsigned int *) (pstdata+initial_data+(4*i)),
								*(unsigned int *) (pstdata+initial_data+4+(4*i)));
					strcat(mega_buf, fpscr_temp);

					sprintf(fpscr_temp,"Pass 1 Output FPR[%0.2d] =	 0x%08x%08x \n", (i/2),
								*(unsigned int *) (pstdata+Pass1_FPR+(4*i)),
								*(unsigned int *) (pstdata+Pass1_FPR+4+(4*i)));
					strcat(mega_buf, fpscr_temp);

					sprintf(fpscr_temp,"Pass 2 Output FPR[%0.2d] =	 0x%08x%08x \n", (i/2),
								*(unsigned int *) (pstdata+Pass2_FPR+(4*i)),
								*(unsigned int *) (pstdata+Pass2_FPR+4+(4*i)));
					strcat(mega_buf, fpscr_temp);
				}
#endif /* __HTX_LE__ */
				strcat(mega_buf,"\n");
				print_fpscr_regs(msgbuf);
				strcat(mega_buf, msgbuf);
				sprintf(file_name,"/tmp/fpscr_bust%d.txt",prnum);
				fp_cpu = fopen(file_name, "w");
				if (fp_cpu == NULL) {
					sprintf(fpscr_temp, "Unable to open file %s for writing fpscr bust error msg \n", file_name);
					strcat(stats.msg_text,fpscr_temp);
					(void) hxfmsg(&stats, 0, HTX_HE_MISCOMPARE, stats.msg_text);
				}
				else {
					fprintf(fp_cpu,"FPSCR BUSTER TEST CASE FAILED : \nFailed for cpu(logical cpu no.): %d\n %s", cpu, mega_buf);
					sprintf(fpscr_temp, "FPSCR BUSTER TEST CASE FAILED : See file %s \n", file_name);
					fsync(fp_cpu);
					fclose(fp_cpu);
					strcat(stats.msg_text,fpscr_temp);
					(void) hxfmsg(&stats, 0, HTX_HE_MISCOMPARE, stats.msg_text);
					exit(55);
				}
				fpscr_count = 0;
			}
			else {
				fpscr_count ++;
				stats.good_others = stats.good_others + 1;
				stats.num_instructions += 10000000;
				if (run_type_OTH == TRUE) {
					(void) hxfmsg(&stats, 0, 7, "good fpscr_bust update \n");
				}
				(void) hxfupdate(UPDATE, &stats);
			}

			rc = htx_unbind_thread();
			if (rc != 0) {
				(void) sprintf(workstr, "Error unbinding to processor %d.",cpu);
				(void) hxfmsg(&stats,rc,1,workstr);
			}
		}
		(void) hxfupdate(FINISH, &stats);			/* send HTX we're done */

	} while((strcmp(stats.run_type, "REG") == 0) || (strcmp(stats.run_type, "EMC") == 0));	/* enddo */

	if(shmdt(inst_ptr) == -1) {
		sprintf(workstr, "Error: Unable to detach share memory for instruction stream. Errno: %d\n", errno);
		(void) hxfmsg(&stats, 0, 1, workstr);
	}
	if(shmctl(shm_id, IPC_RMID, 0)	== -1) {
		hxfmsg(&stats, 0, 1, "Error: shmctl - IPC_RMID	failed	for instruction stream memory\n");
	}

	exit(0);
}		/* end main */

/* Put out message to htx error log containing the fpscr and floating point registers. */
print_fpscr_regs(char *msg)
{
	int i, *fpscr_ptr;
	unsigned long long fpscr;
	char msg_work[512];


	save_regs();
	strcpy(msg,"GENERAL PURPOSE REGISTERS:\n");
	for (i = 0; i < 64; i+=2) {
#ifdef __HTX_LE__
		/* For Little Endian, word ordering is reverse. So, print later word first. */
		sprintf(msg_work,"Gpr[%0.2d] = 0x%0.8x%0.8x\n", i/2,Gprs[i+1],Gprs[i]);
#else
		sprintf(msg_work,"Gpr[%0.2d] = 0x%0.8x%0.8x\n", i/2,Gprs[i],Gprs[i+1]);
#endif
		strcat(msg, msg_work);
	}

	strcat(msg,"\nFLOATING POINT REGISTERS:\n");
	for (i = 0; i < 64; i+=2) {
#ifdef __HTX_LE__
		/* For Little Endian, word ordering is reverse. So, print later word first. */
		sprintf(msg_work,"Fpr[%0.2d] = 0x%0.8x%0.8x\n", i/2,Fprs[i+1],Fprs[i]);
#else
		sprintf(msg_work,"Fpr[%0.2d] = 0x%0.8x%0.8x\n", i/2,Fprs[i],Fprs[i+1]);
#endif
		strcat(msg, msg_work);
	}

	getfpscr(&fpscr);
	fpscr_ptr = &fpscr;

#ifdef __HTX_LE__
	sprintf(msg_work,"\nFPSCR = 0x%0.8x%0.8x\n",*(fpscr_ptr+1), *(fpscr_ptr));
#else
	sprintf(msg_work,"\nFPSCR = 0x%0.8x%0.8x\n",*(fpscr_ptr), *(fpscr_ptr+1));
#endif
	strcat(msg,msg_work);
} /* End print_fpscr_regs */

void SIGTERM_hdl (int sig, int code, struct sigcontext *scp)
{
	(void) hxfmsg(&stats, 0, HTX_HE_INFO, "SIGTERM received, terminating");

	if(shmdt(inst_ptr) == -1) {
		sprintf(workstr, "Error: Unable to detach share memory for instruction stream. Errno: %d\n", errno);
		(void) hxfmsg(&stats, 0, 1, workstr);
	}
	if(shmctl(shm_id, IPC_RMID, 0)	== -1) {
		hxfmsg(&stats, 0, 1, "Error: shmctl - IPC_RMID	failed	for instruction stream memory\n");
	}

	exit(0);
} /* end of SIGTERM_hdl */


void SIGUSR2_hdl (int sig, int code, struct sigcontext *scp)
{
	sprintf(workstr, "HOTPLUG: received SIGUSR2. Total CPUs earlier = %d\n", no_of_cpu);
	no_of_cpu = get_nprocs();
	if ( no_of_cpu <= prnum * GANG_SIZE ) {
		sprintf(workstr, "%sNo CPUs available in this gang (%d-%d). Exiting this instance.\n", workstr, prnum*GANG_SIZE, prnum*(GANG_SIZE+1));
		hxfmsg(&stats, 0, HTX_HE_INFO, workstr);
		hxfupdate(RECONFIG, &stats);
		exit(0);
	}
	sprintf(workstr, "%sTotal CPUs after hotplug = %d\n", workstr, no_of_cpu);
	hxfmsg(&stats, 0, HTX_HE_INFO, workstr);
}
