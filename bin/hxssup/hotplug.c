
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/sysinfo.h>
#include <sched.h>

#include "hxssup.h"
#include "htxsyscfg64.h"
#include "cfgclibdef.h"
#include "scr_info.h"
#include "hxiipc.h"


#define MONITOR_WAIT_TIME 1


extern int hotplug_signal_delay;       /* delay time to send signal after hotplug */
extern int hotplug_restart_delay;      /* delay time to restart exercisers after hotplug */


typedef struct
{
	int cpu;
	int mem;
	int io;
} hotplug_mode;

extern union shm_pointers shm_addr;
extern  int   MAX_ADDED_DEVICES;
extern  int   semhe_id;
extern unsigned int max_wait_tm;
extern tecg_struct *ecg_info;
extern int cur_ecg_pos;
extern volatile int system_call;
extern volatile int    refresh_screen_5;

extern int load_exerciser(struct htxshm_HE *, struct sigaction *);
extern int process_mdt_to_shm(char *, int, union shm_pointers, char **, CFG__SFT *);

int			hotplug_run = 0;
struct sigaction	sigvector_hotplug;


void hotplug_debug_log(char *debug_text)
{
	char msg_str[1024];
	sprintf(msg_str, "hotplug : %s", debug_text);
	send_message(msg_str, 0, HTX_SYS_INFO, HTX_SYS_MSG);
}



void hotplug_log_cpu_mask(cpu_set_t *previous_mask, cpu_set_t *current_mask, int mask_size)
{
	char mask_log_str[2048];
	char mask_str[128];
	unsigned long long * ptr_pre;
	unsigned long long * prt_cur;
	int no_mask_field;
	int mask_field_count;


	sprintf(mask_log_str, "start dumping cpu mask: cpu mask size = <%d> ############################", mask_size);
	send_message(mask_log_str, 0, HTX_SYS_INFO, HTX_SYS_MSG);

	ptr_pre = (unsigned long long*) previous_mask;
	prt_cur = (unsigned long long*) current_mask;

	no_mask_field = mask_size / sizeof(unsigned long long);

	sprintf(mask_str, "\nprev mask :");
	strcpy(mask_log_str,mask_str);
	for (mask_field_count = 0; mask_field_count < no_mask_field; mask_field_count++) {
		if(*ptr_pre == 0) {
			strcat(mask_log_str, "<>");
		} else {
			sprintf(mask_str, "<%llX>", *ptr_pre);
			strcat(mask_log_str, mask_str);
		}
		ptr_pre++;
	}

	send_message(mask_log_str, 0, HTX_SYS_INFO, HTX_SYS_MSG);

	sprintf(mask_str, "\ncurr mask :" );
	strcpy(mask_log_str, mask_str);
	for (mask_field_count = 0; mask_field_count < no_mask_field; mask_field_count++) {
		if(*prt_cur == 0) {
			strcat(mask_log_str, "<>");
		} else {
			sprintf(mask_str, "<%llX>", *prt_cur);
			strcat(mask_log_str, mask_str);
		}
		prt_cur++;
	}
	send_message(mask_log_str, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	
		
	sprintf(mask_log_str, "end dumping cpu mask: dumped mask size = <%zd>  ############################", no_mask_field * sizeof(unsigned long long));
	send_message(mask_log_str, 0, HTX_SYS_INFO, HTX_SYS_MSG);

	if( mask_size != (no_mask_field * sizeof(unsigned long long)) ) {
		sprintf(mask_log_str, "all mask bits are not dumped, actual mask size = <%d>, dumped mask size = <%zd>", mask_size, (no_mask_field * sizeof(unsigned long long) ) );
		send_message(mask_log_str, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	}
}



/* supervisor SIGUSR2 handler to restart mdt and start new added devices after hotplug */
void hotplug_reconfig_restart(int sig, int code, struct sigcontext *scp)
{
	CFG__SFT *mdt_fd = NULL;
	char msg_text[MSG_TEXT_SIZE];
	char  stanza[4096];
	int stanza_counter = 0;
	char  *p_default_snz;
	struct htxshm_HE tmp_HE_entry;
	extern char *default_mdt_snz;
	int cfg_return_code;
	char  tag[40];
	int entry_count;
	int exer_max_count;
	union shm_pointers shm_pointers_itr;
	struct htxshm_HE  *p_shm_HE_base;
	struct htxshm_HE *p_added_shm_HE; 
	int semval;
	char cmd_str[1024], buf[1024];
	char msg[81];
	int workint;
	FILE *fp;
	int no_of_inst;
	int rc;
	char  reg_expr[256];
	boolean_t confirm_errmsg;
	int prev_max_wait_tm;
	int prev_exer_max_count;
	char mdt_path[250];
	union semun hotplug_semctl_arg;


	send_message("SIGUSER2 is received from hotplug child", 0, HTX_SYS_INFO, HTX_SYS_MSG);

	shm_pointers_itr.hdr_addr = shm_addr.hdr_addr;
	if(shm_pointers_itr.hdr_addr->hotplug_process_flag == 0) {
		PRTMSG(MSGLINE, 0,("hotplug operation detected and start processing") ); 
		send_message("SIGUSER2 is received from hotplug child", 0, HTX_SYS_INFO, HTX_SYS_MSG);
		return;
	}

	if(shm_pointers_itr.hdr_addr->hotplug_process_flag != 1) {
		send_message("SIGUSER2 is received from unknown source", 0, HTX_SYS_INFO, HTX_SYS_MSG);
		return;
	}
	refresh_screen_5 = 1;

	send_message("repopulate mdts", 0, HTX_SYS_INFO, HTX_SYS_MSG);

	/* set default file descriptors close on exec */
	fcntl(fileno(stdin), F_SETFD, 1);
	fcntl(fileno(stdout), F_SETFD, 1);
	fcntl(fileno(stderr), F_SETFD, 1);

	sigfillset(&(sigvector_hotplug.sa_mask));
	sigprocmask(SIG_UNBLOCK, &(sigvector_hotplug.sa_mask), NULL);
	sigvector_hotplug.sa_flags = 0;
	sigvector_hotplug.sa_handler = SIG_DFL;

	prev_exer_max_count = exer_max_count = shm_pointers_itr.hdr_addr->max_entries;

	(shm_pointers_itr.hdr_addr)++;  /* skipping shm header */
	p_shm_HE_base = shm_pointers_itr.HE_addr;

	sprintf(mdt_path, "%s/mdt/%s", (char *)getenv("HTXPATH"), (char *)getenv("MDTFILE") );

	sprintf(msg_text, "cp %s /usr/lpp/htx/mdt/mdt", mdt_path);
	system_call = true;
	system(msg_text);
	system_call = false;

	/* process hotplug mdt */
	mdt_fd = cfgcopsf(mdt_path);
	if (mdt_fd == (CFG__SFT *) NULL){
		sprintf(msg_text, "Unable to open MDT file %s", mdt_path);
		send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
		return;
	}

	p_default_snz = (char *) &default_mdt_snz;

	do {
		cfg_return_code = process_mdt_to_shm(stanza, sizeof(stanza), (union shm_pointers) &tmp_HE_entry, &p_default_snz, mdt_fd);
		if (cfg_return_code != CFG_SUCC) {
			if (cfg_return_code == CFG_EOF) {
				continue;
			} else {
				switch (cfg_return_code)
				{
					case CFG_EOF:
						sprintf(tag, "CFG_EOF");
						break;
					case CFG_SZNF:
						sprintf(tag, "CFG_SZNF");
						break;
					case CFG_SZBF:
						sprintf(tag, "CFG_SZBF");
						break;
					case CFG_UNIO:
						sprintf(tag, "CFG_UNIO");
						break;
					default:
						sprintf(tag, "CFG_????");
						break;
				}
			}
		} else {
			stanza_counter++;
		}

		for (entry_count = 0; entry_count < exer_max_count; entry_count++) {
			if (0 == (strcmp(tmp_HE_entry.sdev_id, (p_shm_HE_base + entry_count)->sdev_id))) {
				break;
			}
		}

		if ( (entry_count < exer_max_count) && (shm_addr.hdr_addr->started == 0)) {
			sprintf(msg_text,"HTX not started. Device %s already there(%d, %d)\n",
				tmp_HE_entry.sdev_id, shm_addr.hdr_addr->num_entries, shm_addr.hdr_addr->max_entries);
			send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);
			continue;
		}

		p_added_shm_HE = (p_shm_HE_base + entry_count); /* point to entry for new dev */

		if (entry_count >= exer_max_count) {
			if ( exer_max_count < shm_addr.hdr_addr->pseudo_entry_0) {
				strcpy(EXER_NAME(exer_max_count),tmp_HE_entry.sdev_id);
				ECGEXER_SHMKEY(exer_max_count) = HTXSHMKEY;
				ECGEXER_SEMKEY(exer_max_count) = SEMHEKEY;
				prev_exer_max_count = exer_max_count;
				exer_max_count++;
				NUM_EXERS = exer_max_count; 
			} else {
				sprintf(msg_text, "All %d entries in shared memory allocated for new devices are used.\nIncrease\"max_added_devices\" in /usr/lpp/htx/.htx_profile and  restart HTX", MAX_ADDED_DEVICES);
				send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);
				sprintf(msg_text, "No room for new devices, see /tmp/htxmsg"); 
				send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
				continue;
			}
		} else {
			if (p_added_shm_HE->PID != 0) {
				sprintf(msg_text, "hotplug: Attempt to restart active device %s", tmp_HE_entry.sdev_id);
				send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);
				continue;
			}
		}

		bcopy(&tmp_HE_entry, (char *) p_added_shm_HE, sizeof(tmp_HE_entry));

		hotplug_semctl_arg.val = 0;
		semctl(semhe_id, ( (entry_count * SEM_PER_EXER) + 6), SETVAL, hotplug_semctl_arg);

		sprintf(msg_text,"hotplug: Adding %s by operator request for hotplug.", tmp_HE_entry.sdev_id);
		send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);

		if (shm_addr.hdr_addr->started != 0) {
			if (strncmp(p_added_shm_HE->HE_name, "hxesct", 6) == 0) {
				sprintf(cmd_str, "ps -ef | grep hxesct | grep -v grep | wc -l");
			} else {
				sprintf(cmd_str, "ps -ef | grep %s | grep -v grep | wc -l", p_added_shm_HE->HE_name);
			}

			fp = popen(cmd_str, "r");
			if (fp  == NULL) {
				sprintf(msg_text,"popen error in reconfig_restart");
				send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
			} 

			if (fgets(buf,1024,fp) == NULL) {
				sprintf(msg_text,"fgets error in reconfig_restart");
				send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
			}
			
			sscanf(buf, "%d", &no_of_inst);

			system_call = TRUE;
			pclose(fp);
			system_call = FALSE;

			if(no_of_inst == 0) {
				strcpy(reg_expr,"^");
				strcat(reg_expr,p_added_shm_HE->HE_name);
				strcat(reg_expr, ".*setup[\t ]*$");
				rc = exec_HE_script(reg_expr, p_added_shm_HE->sdev_id, &confirm_errmsg);
				if (rc < 0) {
					sprintf(msg_text, "WARNING: Failed running setup script(s) for %s",tmp_HE_entry.sdev_id);
					send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);
				}			

			}
			prev_max_wait_tm = max_wait_tm;
			workint = (p_added_shm_HE->max_run_tm * 2) + p_added_shm_HE->idle_time;
			if (workint > max_wait_tm) {
				max_wait_tm = workint;
			}

			shm_addr.hdr_addr->num_entries++;
			shm_addr.hdr_addr->max_entries = exer_max_count;

			p_added_shm_HE->tm_last_upd = -1;

			rc = load_exerciser(p_added_shm_HE, &sigvector_hotplug);
			if (rc != 0) {
				max_wait_tm = prev_max_wait_tm;
				shm_addr.hdr_addr->num_entries--;
				shm_addr.hdr_addr->max_entries = exer_max_count = prev_exer_max_count;

				sprintf(msg_text, "ERROR: Failed to start %s. ", p_added_shm_HE->HE_name);
				send_message(msg_text, 0, HTX_SYS_SOFT_ERROR, HTX_SYS_MSG);

				continue;
			}
			
		} else {
			prev_max_wait_tm = max_wait_tm;
			workint = (p_added_shm_HE->max_run_tm * 2) + p_added_shm_HE->idle_time;
			if (workint > max_wait_tm) {
				max_wait_tm = workint;
			}

			shm_addr.hdr_addr->num_entries++;
			shm_addr.hdr_addr->max_entries = exer_max_count;

			p_added_shm_HE->tm_last_upd = -1;
		}

		sprintf(msg, "Started /dev/%s",p_added_shm_HE->sdev_id);
		sprintf(msg_text, "Addition or replacement of exerciser for %s completed sucessfully.", tmp_HE_entry.sdev_id);
		send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);		

	} while(cfg_return_code == CFG_SUCC);

	if (cfgcclsf(mdt_fd) != CFG_SUCC) {
		sprintf(msg_text, "Unable to close MDT file in hotplug.");
		send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	}

	/* set default file descriptors reopen on exec */
	fcntl(fileno(stdin), F_SETFD, 0);
	fcntl(fileno(stdout), F_SETFD, 0);
	fcntl(fileno(stderr), F_SETFD, 0);
	send_message("hotplug operation processing completed", 0, HTX_SYS_INFO, HTX_SYS_MSG);
	PRTMSG1(MSGLINE, 0, ("hotplug operation processing completed"));
}



/* send sigal to all registered process */
int hotplug_monitor_send_signal(hotplug_mode* p_mode_state)
{
	union shm_pointers shm_pointers_itr;
	int exer_position;
	int exer_max_count;
	char msg_text[512];


	shm_pointers_itr.hdr_addr = shm_addr.hdr_addr;	
	exer_max_count = shm_pointers_itr.hdr_addr->max_entries;

	(shm_pointers_itr.hdr_addr)++;  /* skipping shm header */

	for(exer_position = 0; exer_position < exer_max_count; exer_position++) {
		if(shm_pointers_itr.HE_addr->PID != 0) {
			if( (shm_pointers_itr.HE_addr->hotplug_cpu == 1) && (p_mode_state->cpu == 1) ){
				kill(shm_pointers_itr.HE_addr->PID, SIGUSR2);
				sprintf(msg_text, "hotplug: SIGUSR2 signal has been sent to device <%s> with process ID [%d]", shm_pointers_itr.HE_addr->sdev_id, shm_pointers_itr.HE_addr->PID);
				send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);
			}

			if( (shm_pointers_itr.HE_addr->hotplug_mem == 1) && (p_mode_state->mem == 1) && (p_mode_state->cpu != 1) ){
				kill(shm_pointers_itr.HE_addr->PID, SIGUSR2);
				sprintf(msg_text, "hotplug: SIGUSR2 signal has been sent to device <%s> with process ID [%d]", shm_pointers_itr.HE_addr->sdev_id, shm_pointers_itr.HE_addr->PID);
				send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);
			}
	
			if( (shm_pointers_itr.HE_addr->hotplug_io == 1) && (p_mode_state->io == 1) && (p_mode_state->cpu != 1) && (p_mode_state->mem != 1)){
				kill(shm_pointers_itr.HE_addr->PID, SIGUSR2);
				sprintf(msg_text, "hotplug: SIGUSR2 signal has been sent to device <%s> with process ID [%d]", shm_pointers_itr.HE_addr->sdev_id, shm_pointers_itr.HE_addr->PID);
				send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);
			}
		}

		(shm_pointers_itr.HE_addr)++;
	}
	
	return 0;
}



/* populates mdt.hotplug */
int hotplug_monitor_create_mdt(void)
{
	char mdt_path[250];
	char hotplug_str[1024];
	char debug_msg[1024];
	int rc = 0;

	sprintf(hotplug_str, "start recreating mdt files");
	hotplug_debug_log(hotplug_str);

	/* populate the mdt path */
	sprintf(mdt_path, "%s/mdt/%s", (char *)getenv("HTXPATH"), (char *)getenv("MDTFILE") );	

	sprintf(hotplug_str, "mdt_path = [%s]", mdt_path);
	hotplug_debug_log(hotplug_str);

	sprintf(hotplug_str, "cd /usr/lpp/htx/etc/scripts ; /usr/lpp/htx/bin/show_syscfg > /tmp/htx_syscfg; devconfig > /tmp/hotplug_devconfig_out 2>&1");
	system_call = TRUE;
	system(hotplug_str);
	system_call = FALSE;

	return 0;	
}



int hotplug_monitor_process(hotplug_mode* p_mode_state)
{
	char hotplug_str[200];
	union shm_pointers shm_pointers_itr;

	shm_pointers_itr.hdr_addr = shm_addr.hdr_addr;


	hotplug_monitor_send_signal(p_mode_state); 

	sleep(hotplug_restart_delay);

	/* need a restart operation */	
	hotplug_monitor_create_mdt();
	shm_pointers_itr.hdr_addr->hotplug_process_flag = 1;

	sprintf(hotplug_str, "ready to restart MDT, sending SIGUSR2 to supervisor pid <%d>", getppid());
	hotplug_debug_log(hotplug_str);
	kill(getppid(), SIGUSR2);
	
	return 0;
}



/* hotplug monitor functionality */
void* hotplug_monitor(void)
{
	hotplug_mode mode_state;
	cpu_set_t *previous_online_cpus_mask	= NULL;	
	cpu_set_t *current_online_cpus_mask	= NULL;	
	cpu_set_t *original_online_cpus_mask	= NULL;	
	size_t cpu_mask_size;
	int number_config_processor;
	int previous_cpu_count;
	int current_cpu_count;
	char debug_msg[1024];
	int hotplug_wait_counter = 0;
	union shm_pointers shm_pointers_itr;
	int return_code;
	int rc =0;
	int semval;
	struct semid_ds sembuffer;
	union semun hotplug_semctl_arg;
	

	hotplug_run = 1;
	shm_pointers_itr.hdr_addr = shm_addr.hdr_addr;

	previous_cpu_count = get_nprocs();

	number_config_processor = sysconf(_SC_NPROCESSORS_CONF);
	if(number_config_processor == -1) {
		sprintf(debug_msg, "ERROR: sysconf() returns <%d>", number_config_processor);
		send_message(debug_msg, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	}

	cpu_mask_size = CPU_ALLOC_SIZE(number_config_processor);

	previous_online_cpus_mask = CPU_ALLOC(number_config_processor);
	if(previous_online_cpus_mask == NULL) {
		sprintf(debug_msg, "ERROR: CPU_ALLOC for number_config_processor <%d>", number_config_processor);	
		send_message(debug_msg, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	}

	CPU_ZERO_S(cpu_mask_size, previous_online_cpus_mask);
	return_code = sched_getaffinity(getpid(), cpu_mask_size, previous_online_cpus_mask);
	if(return_code != 0) {
		sprintf(debug_msg, "ERROR: sched_getaffinity() returned with return code <%d>, and errno as <%d>", return_code, errno);
		send_message(debug_msg, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	}

	original_online_cpus_mask = CPU_ALLOC(number_config_processor);
	if(original_online_cpus_mask == NULL) {
		sprintf(debug_msg, "ERROR: CPU_ALLOC for number_config_processor <%d>", number_config_processor);	
		send_message(debug_msg, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	}

	CPU_ZERO_S(cpu_mask_size, original_online_cpus_mask);
	return_code = sched_getaffinity(getpid(), cpu_mask_size, original_online_cpus_mask);
	if(return_code != 0) {
		sprintf(debug_msg, "ERROR: sched_getaffinity() returned with return code <%d>, and errno as <%d>", return_code, errno);
		send_message(debug_msg, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	}

	current_online_cpus_mask = CPU_ALLOC(number_config_processor);
	if(current_online_cpus_mask == NULL) {
		sprintf(debug_msg, "ERROR: CPU_ALLOC for number_config_processor <%d>, current_online_cpus_mask is NULL", number_config_processor);	
		send_message(debug_msg, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	}

	CPU_ZERO_S(cpu_mask_size, current_online_cpus_mask);
	return_code = sched_getaffinity(getpid(), cpu_mask_size, current_online_cpus_mask);
	if(return_code != 0) {
		sprintf(debug_msg, "ERROR: for current_online_cpus_mask sched_getaffinity() returned with return code <%d>, and errno as <%d>", return_code, errno);
		send_message(debug_msg, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	}

	mode_state.cpu = mode_state.mem = mode_state.io = 0;
	while(hotplug_run == 1) {  /* main loop */
		if( CPU_EQUAL_S( cpu_mask_size, previous_online_cpus_mask, current_online_cpus_mask ) == 0) {
			current_cpu_count = get_nprocs();
			sprintf(debug_msg, "found change in online cpu mask: previous cpu count is <%d>, current cpu count is <%d>", previous_cpu_count, current_cpu_count);
			send_message(debug_msg, 0, HTX_SYS_INFO, HTX_SYS_MSG);
			hotplug_log_cpu_mask(previous_online_cpus_mask, current_online_cpus_mask, cpu_mask_size);

			if(hotplug_wait_counter != 0) {
				sprintf(debug_msg, "found another change in online cpu mask, resetting the hotplug wait counter");
				send_message(debug_msg, 0, HTX_SYS_INFO, HTX_SYS_MSG);
			}
			hotplug_wait_counter = 0;
			shm_pointers_itr.hdr_addr->hotplug_process_flag = 0;

			sprintf(debug_msg, "sending SIGUSR2 signal to supervisor pid <%d>", getppid());
			hotplug_debug_log(debug_msg);
			kill( getppid(), SIGUSR2); 
			mode_state.cpu = 1;
			memcpy( previous_online_cpus_mask, current_online_cpus_mask, cpu_mask_size );
			previous_cpu_count = current_cpu_count;
			rc = 0;
	        hotplug_semctl_arg.val = 1;
	        rc = semctl(semhe_id, SEM_POSITION_SYSCFG, SETVAL, hotplug_semctl_arg);
	        if(rc == -1) {
	            sprintf(debug_msg, "semctl is failed with return code <%d>, errno = <%d>", rc, errno);
	            send_message(debug_msg, 0, HTX_SYS_INFO, HTX_SYS_MSG);
	        }
			rc = update_syscfg();
		    if(rc != 0) {
				sprintf(debug_msg, "ERROR :update_syscfg unsuccessfull inisde hotplug with %d \n",rc);
		        send_message(debug_msg, 0, HTX_SYS_INFO, HTX_SYS_MSG);
		    }

			hotplug_semctl_arg.val = 0;
		    rc = semctl(semhe_id, SEM_POSITION_SYSCFG, SETVAL, hotplug_semctl_arg);

		} 

		if (mode_state.cpu == 1 ||  mode_state.mem == 1 || mode_state.io == 1) {
			if(hotplug_wait_counter < hotplug_signal_delay) {
				hotplug_wait_counter++;
			} else {
				sprintf(debug_msg, "hotplug: start processing hotplug");
				hotplug_debug_log(debug_msg);
				hotplug_monitor_process(&mode_state); 
				mode_state.cpu = mode_state.mem = mode_state.io = 0;
				hotplug_wait_counter = 0;
			}
		}

		sleep(1);


		CPU_ZERO_S(cpu_mask_size, current_online_cpus_mask);
		return_code = sched_getaffinity(getppid(), cpu_mask_size, current_online_cpus_mask);
		if(return_code != 0) {
			sprintf(debug_msg, "ERROR: sched_getaffinity() returned with return code <%d>, and errno as <%d>", return_code, errno);
			send_message(debug_msg, 0, HTX_SYS_INFO, HTX_SYS_MSG);
		}
	}

	if(previous_online_cpus_mask != NULL) {
		CPU_FREE(previous_online_cpus_mask);
	}
	if(current_online_cpus_mask != NULL) {
		CPU_FREE(current_online_cpus_mask);
	}

	return NULL;
}



/* hotplug process launcher forks new porcess for hotplug handling*/
int start_hotplug_monitor(void)
{
	int return_code = 0;
	char msg_str[200];	
	struct sigaction sigvector;


	return_code = fork();
	if(return_code == 0){
		sprintf(msg_str, "%s with pid [%d]", "hotplug monitor process is started", getpid() );
		send_message(msg_str, 0, HTX_SYS_INFO, HTX_SYS_MSG);

		sigvector.sa_handler = SIG_IGN;
		sigvector.sa_flags = 0;
		sigaction(SIGCHLD, &sigvector, (struct sigaction *) NULL);

		hotplug_monitor();
		exit(0);
	}

	return return_code;
}



int stop_hotplug_monitor(void)
{
	int return_code = 0;

	hotplug_run = 0;


	return return_code;
}




