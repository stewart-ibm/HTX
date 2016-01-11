/* @(#)40	1.1  src/htx/usr/lpp/htx/bin/htxd/htxd_dr.c, htxd, htxubuntu 7/17/13 08:42:32 */



#include "htxd_instance.h"
#include "htxd_ipc.h"
#include "htxd_ecg.h"


#ifndef __HTX_LINUX__

int htxd_dr_restart_devices(char *dr_ecg_name)
{

	int return_code = 0;

/*	return_code = htxd_set_shm_exercisers_values_for_dr_restart(dr_ecg_name); */




	return return_code;	
}




/* start DR child process */
int htxd_start_DR_child(void)
{

	pid_t dr_child_pid;
	int dr_shm_id = -1;
	char msg_text[256];


	dr_shm_id = htxd_create_dr_sem(); 
	if(dr_shm_id== -1)  {
		strcpy(msg_text,"Unable to get the DR semaphore...No point forking DR child\n");
		htxd_send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);
		return -1;
	}

	htxd_set_dr_sem_value(0);

	dr_child_pid = htxd_create_child_process();		
	switch(dr_child_pid)
	{
	case 0:
		strcpy(msg_text,"Inside DR child...will wait here\n");
		htxd_send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);

		htxd_init_dr_child_signal_handler();

		while(1) {
			sleep(10000);
		}
		exit(0);

	case -1:
		printf("DEBUG: htxd_start_DR_child() failed while creating child process with errno = <%d>", errno); fflush(stdout);

		return -1;

	default:
		sprintf(msg_text,"DR child started..pid(%d)\n", dr_child_pid);
		htxd_send_message(msg_text, 0, HTX_SYS_INFO, HTX_SYS_MSG);

		htxd_set_dr_child_pid(dr_child_pid);

		break;	
	}

	return 0;
}

#endif
